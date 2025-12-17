/////////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2025 Tom Wong  
// Email:  buffi@163.com
// SPDX-License-Identifier: MIT
// 
// RVB website   : http://www.rvb.net.cn/
// FFMPEG website: https://www.ffmpeg.org/
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PictureSizeFilter.h"

#include "..\davsdk\includes\ffmo.h"

CPictureSizeFilter::CPictureSizeFilter(void)
{
 	m_nType = PS_DEFAULT;

	m_inputPad.m_pOwner = this;
	m_inputPad.SetCaptureType(CPad::CT_ANY);
	m_inputPad.SetMediaType(MX_MT_VIDEO);

	m_outputPad.m_pOwner = this;
	m_outputPad.SetCaptureType(CPad::CT_ANY);
	m_outputPad.SetMediaType(MX_MT_VIDEO);
	 
	m_nWorkMode = WM_PASSIVE;

//	ResetVideoDescEx(&m_inDesc, MX_VF_BGR, AV_DEFAULT_VIDEO_WIDTH,AV_DEFAULT_VIDEO_HEIGHT );
//	ResetVideoDescEx(&m_outDesc, MX_VF_BGR, AV_DEFAULT_VIDEO_WIDTH,AV_DEFAULT_VIDEO_HEIGHT );

	::InitializeCriticalSection(&m_cs );

}


CPictureSizeFilter::~CPictureSizeFilter(void)
{

	::DeleteCriticalSection(&m_cs );
}



M_RESULT CPictureSizeFilter::Play(const char* strUrl )
{
	if (m_nState == MS_PLAYING) return M_OCCUPIED;

	if (!m_outputPad.IsConnected()) return M_DISCONNECTED;
	if (!m_inputPad.IsConnected()) return M_DISCONNECTED;
	 
	CUtAutoLock lock(&m_cs);

	MxVideoDesc* pInDesc =(MxVideoDesc*) m_inputPad.GetFormatDescriptor();
 	MxVideoDesc*  pOutDesc = (MxVideoDesc*)m_outputPad.GetFormatDescriptor();
	 

	if (! m_imageConverter.Create( GetVideoPixelFormat(pInDesc->format) , pInDesc->width, pInDesc->height  , \
			               GetVideoPixelFormat(pOutDesc->format) , pOutDesc->width, pOutDesc->height )){
				
		return M_FAILED;
	}
 
	m_nState = MS_PLAYING;

	return M_OK;
}
	
void CPictureSizeFilter::Stop()
{

	CUtAutoLock lock(&m_cs);

	m_nState =MS_STOPPED;

 	m_imageConverter.Destroy();
	 
}

	
BOOL CPictureSizeFilter::OnPadConnected(CPad* pOutputPad, CPad* pInputPad) {
	
	 if (&m_outputPad == pOutputPad){		
		 RV_ASSERT(pInputPad);

		 MxVideoDesc* pOutDesc = (MxVideoDesc*)m_outputPad.GetFormatDescriptor(); 
		 MxVideoDesc* pNextDesc = (MxVideoDesc*)pInputPad->GetFormatDescriptor();
		 RV_ASSERT(pOutDesc && pNextDesc);

		 *pOutDesc = *pNextDesc;

		 return TRUE;
		// return  HandleDescInChanged();
		  
		 //return pOutputPad->NegotiateDescriptor((MxDescriptor *)pOutDesc);
	 }

	 if (pInputPad == &m_inputPad){
		 MxDescriptor* pDesc = pOutputPad->GetFormatDescriptor();
		 RV_ASSERT(pDesc->type == MX_MT_VIDEO);

	 }

	 return TRUE;

}
	
void CPictureSizeFilter::OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad) {
	//DO nothing
	;
}
// 

	//当接收到一个SAMPLE的时候,产生该事件。
BOOL CPictureSizeFilter::OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)
{
	 RV_ASSERT(pInputPad && hSample);    
	 RV_ASSERT(pInputPad == &m_inputPad);
	  
	 CUtAutoLock lock(&m_cs);

	 if (m_nState != MS_PLAYING)
	 {
		 if (pErrCode) *pErrCode = M_TERMINATED;
		 return FALSE;
	 }
	 
	 MX_HANDLE hNew = NULL;

	 hNew = ConvertSample(hSample); 
	 RV_ASSERT(hNew);

	 if (this->m_outputPad.IsConnected()){
		 if (m_outputPad.Pass(hNew, pErrCode) == FALSE)
		 {
			 mxDestroySample(hNew);
			 return FALSE;
		 }
		 else {
			 mxDestroySample(hSample);			 
			 return TRUE;
		 }
	 }
	 else{
		  mxDestroySample(hNew);
	 }

	 if (pErrCode) *pErrCode = M_DATA_REJECTED;
	 return FALSE;
}

	////当接收方通过PAD接收SAMPLE时,产生该事件。
MX_HANDLE CPictureSizeFilter::OnSampleRequest(CPad* pOutputPad,/* MX_HANDLE hSample,*/ int* pErrCode) 
{
	 CUtAutoLock lock(&m_cs);

	 //RV_ASSERT(NULL ==hSample);
	 if ( m_nState == MS_STOPPED) 
	 {
		  if (pErrCode) *pErrCode = M_TERMINATED;
		  return NULL;
	 }
	 
	 if (m_inputPad.IsConnected()){ 

		int nErrCode = M_OK;
		MX_HANDLE hSample = m_inputPad.Fetch(&nErrCode);

		if (hSample){

			MX_HANDLE hNew = NULL;

			hNew = ConvertSample(hSample); 
			RV_ASSERT(hNew);			

			mxDestroySample(hSample);

			return hNew;

		}
		else
		{
			if (pErrCode) *pErrCode = nErrCode;
			return NULL;
		}

	 }


	 if (pErrCode) *pErrCode = M_DISCONNECTED;
	 return NULL;
}

	//是否接受输入PAD的流媒体格式描述符
	//具体格式协商的时候用，一般输入PIN所在的MO实现。
BOOL CPictureSizeFilter::Accept(CPad* pInputPad, MxDescriptor* pDescriptor) 
{
	RV_ASSERT(pInputPad && pDescriptor);
	RV_ASSERT(pInputPad  == &m_inputPad);
	RV_ASSERT(pInputPad->IsConnected());

	RV_ASSERT(pDescriptor->type == MX_MT_VIDEO);

	MxVideoDesc* pSrcDesc = (MxVideoDesc* )pDescriptor;
	MxVideoDesc* pInDesc = (MxVideoDesc*)m_inputPad.GetFormatDescriptor();
	 
	ResetVideoDescEx(pInDesc, pSrcDesc->format, pSrcDesc->width, pSrcDesc->height);

	return TRUE;
	//return HandleDescInChanged();

}

MX_HANDLE CPictureSizeFilter::ConvertSample(MX_HANDLE hSrc)
{
	
	  MX_HANDLE hNew = NULL;

	  MxVideoDesc* pInDesc = (MxVideoDesc*)m_inputPad.GetFormatDescriptor();
	  MxVideoDesc* pOutDesc = (MxVideoDesc*)m_outputPad.GetFormatDescriptor();

	  void* pSampleData = mxGetSampleData(hSrc); 

#if _DEBUG
	  MxVideoDesc* pSrcDesc =(MxVideoDesc*) mxGetSampleDescriptor(hSrc);

	 RV_ASSERT(pInDesc->dataSize == m_imageConverter.GetSrcBufferSize());
	  RV_ASSERT(pInDesc->dataSize == pSrcDesc->dataSize);

#endif


	  UINT nDstSize = m_imageConverter.GetDestBufferSize();	
	  RV_ASSERT(	nDstSize == pOutDesc->dataSize );

	  hNew = mxCreateSampleEx((MxDescriptor *)pOutDesc, sizeof(MxVideoDesc), nDstSize);
	  RV_ASSERT(hNew);

	  BOOL ret = m_imageConverter.ExcuteEx((uint8_t*)pSampleData, pInDesc->dataSize, (uint8_t*)mxGetSampleData(hNew) , nDstSize);
	  RV_ASSERT(ret);

	 return hNew;;
}


 BOOL    CPictureSizeFilter::HandleDescInChanged( )
 {
	
	 CUtAutoLock lock(&m_cs);

     if (m_nState != MS_STOPPED) return FALSE;
	
	 m_imageConverter.Destroy();

	 MxVideoDesc* pInDesc = (MxVideoDesc*)m_inputPad.GetFormatDescriptor();
	 MxVideoDesc* pOutDesc = (MxVideoDesc*)m_outputPad.GetFormatDescriptor();


	 return m_imageConverter.Create( GetVideoPixelFormat(pInDesc->format) , pInDesc->width, pInDesc->height  , \
		 GetVideoPixelFormat(pOutDesc->format) , pOutDesc->width, pOutDesc->height );

	 return TRUE;

 }

void CPictureSizeFilter::SetFormat(int format, int width, int height)
{
	if (m_nType != PS_CUSTOM) return ;

	MxVideoDesc* pOutDesc = (MxVideoDesc*)m_outputPad.GetFormatDescriptor();

	ResetVideoDescEx(pOutDesc, format, width, height );
}

void CPictureSizeFilter::SetFormatEx(MxVideoDesc* pVideoDesc)
{
		
	if (m_nType != PS_CUSTOM) return;
	 
	if (pVideoDesc){	
		RV_ASSERT(pVideoDesc->type == MX_MT_VIDEO);

		MxVideoDesc* pOutDesc = (MxVideoDesc*)m_outputPad.GetFormatDescriptor();

		ResetVideoDescEx(pOutDesc, pVideoDesc->format, pVideoDesc->width, pVideoDesc->height );
	}

}


void CPictureSizeFilter::SetType(CPictureSizeFilter::TYPE type)
{
	m_nType = type;
}

int  CPictureSizeFilter::GetType(){
	return m_nType;
}