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
#include "VideoFilter.h"

#include "..\davsdk\includes\ffmo.h"


CVideoFilter::CVideoFilter(void)
{
	m_nType = VF_NONE;

	m_inputPad.m_pOwner = this;
	m_inputPad.SetCaptureType(CPad::CT_ANY);

	m_outputPad.m_pOwner = this;
	m_outputPad.SetCaptureType(CPad::CT_ANY);

	m_nWorkMode = WM_PASSIVE;

	ResetVideoDescEx(&m_inDesc, MX_VF_BGR, AV_DEFAULT_VIDEO_WIDTH,AV_DEFAULT_VIDEO_HEIGHT );
	ResetVideoDescEx(&m_outDesc, MX_VF_BGR, AV_DEFAULT_VIDEO_WIDTH,AV_DEFAULT_VIDEO_HEIGHT );

	::InitializeCriticalSection(&m_cs );



}

CVideoFilter::~CVideoFilter(void)
{
	::DeleteCriticalSection(&m_cs );
}


M_RESULT CVideoFilter::Play(const char* strUrl)
{
	CUtAutoLock lock(&m_cs);

	m_nState = MS_PLAYING;

	return M_OK;
}
	
void CVideoFilter::Stop()
{
	CUtAutoLock lock(&m_cs);

	m_nState = MS_STOPPED;
}

	
BOOL CVideoFilter::OnPadConnected(CPad* pOutputPad, CPad* pInputPad) 
{
	 if (&m_outputPad == pOutputPad){				 
		 return pOutputPad->NegotiateDescriptor((MxDescriptor *)&m_outDesc);
	 }

	 if (pInputPad == &m_inputPad){
		 MxDescriptor* pDesc = pOutputPad->GetFormatDescriptor();
		 RV_ASSERT(pDesc->type == MX_MT_VIDEO);

	 }


	return TRUE;
}

void CVideoFilter::OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad) 
{
	if (pInputPad == &m_inputPad){
		m_outputPad.Disconnect();
	}
}
//
////输出PIN调用connect的时候被调用，媒体对象的输入PIN必须重载该
////函数，以表示是否支持指定的媒体类型。	
//BOOL CVideoFilter::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) {
//
//	RV_ASSERT(pInputPad);
//	RV_ASSERT(pInputPad == &m_inputPad);
//
//	return (type == MX_MT_VIDEO);
//
//	return TRUE;
//}
//
////获得媒体对象输出PIN的媒体类型
//MX_MEDIA_TYPE CVideoFilter::GetMediaType(CPad* pOutputPad){
//	RV_ASSERT(&m_outputPad == pOutputPad);	 
//
//
//	return MX_MT_VIDEO;
//}
//
//MxDescriptor* CVideoFilter::GetFormatDescriptor(CPad* pPad){
//
//	if (pPad == &m_inputPad){
//		return ((MxDescriptor*)&m_inDesc);
//	}
//	else{
//		RV_ASSERT(pPad == &m_outputPad);
//		return ((MxDescriptor*)&m_outDesc);
//	}
//
//}

//当接收到一个SAMPLE的时候,产生该事件。	
BOOL CVideoFilter::OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode){

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
		 if (m_outputPad.Pass(hNew, NULL) == FALSE)
		 {
			 mxDestroySample(hNew);
		 }
	 }
	 else{
		  mxDestroySample(hNew);
	 }


	 if (pErrCode) *pErrCode = M_DATA_REJECTED;
	 return FALSE;
 
}

////当接收方通过PAD接收SAMPLE时,产生该事件。	
MX_HANDLE CVideoFilter::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode) {
	 CUtAutoLock lock(&m_cs);

	// RV_ASSERT(NULL ==hSample);
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
BOOL CVideoFilter::Accept(CPad* pInputPad, MxDescriptor* pDescriptor) {
	RV_ASSERT(pInputPad && pDescriptor);
	RV_ASSERT(pInputPad  == &m_inputPad);
	RV_ASSERT(pInputPad->IsConnected());

	RV_ASSERT(pDescriptor->type == MX_MT_VIDEO);

	MxVideoDesc* pSrcDesc = (MxVideoDesc* )pDescriptor;
	//DuplicateDescriptor(pDescriptor, &m_inDesc);
	ResetVideoDescEx(&m_inDesc, pSrcDesc->format, pSrcDesc->width, pSrcDesc->height);
	ResetVideoDescEx(&m_outDesc, pSrcDesc->format, pSrcDesc->width, pSrcDesc->height);

	return HandleDescInChanged();

}



MX_HANDLE CVideoFilter::ConvertSample(MX_HANDLE hSrc)
{
	
	  MX_HANDLE hNew = NULL;

	  if (VF_BLUR == m_nType ){

	  }
	  else{
		  hNew = mxDuplicateSample(hSrc);
	  }

	  return hNew; 
}


 BOOL    CVideoFilter::HandleDescInChanged( )
 {
	
	 CUtAutoLock lock(&m_cs);

	 if (m_outputPad.IsConnected()){
		 return m_outputPad.NegotiateDescriptor((MxDescriptor *)&m_outDesc);
	 }
  

	 return TRUE;

 }

 void  CVideoFilter::SetFilterType(CVideoFilter::TYPE type)
 {

	 if (type == m_nType) return;

	 CUtAutoLock lock(&m_cs);

	 m_nType = type;

;
 }