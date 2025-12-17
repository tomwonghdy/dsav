// Copyright (c) 2025 Tom Wong  
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
#include "stdafx.h"
#include "StreamAdapter.h"

#include "ImageConverter.h"
#include "WaveConverter.h"
#include "AvLib.h"


CStreamAdapter::CStreamAdapter(void)
{

	m_pInDesc = m_pOutDesc = NULL;
	m_pConverter = NULL;

	m_inputPad.m_pOwner = this;
	m_inputPad.SetCaptureType(CPad::CT_ANY);

	m_outputPad.m_pOwner = this;
	m_outputPad.SetCaptureType(CPad::CT_ANY);

	m_nWorkMode = WM_PASSIVE;

	::InitializeCriticalSection(&m_cs );


}


CStreamAdapter::~CStreamAdapter(void)
{
	RV_ASSERT(NULL == m_pConverter);

	if (m_pInDesc){
		mxDestroyDescriptor(m_pInDesc );
		m_pInDesc = NULL; 
	}

	if (m_pOutDesc){
		mxDestroyDescriptor(m_pOutDesc );
		m_pOutDesc = NULL; 
	}	

	::DeleteCriticalSection(&m_cs );

}


M_RESULT CStreamAdapter::Play(const char* strUrl )
{
	RV_ASSERT(m_pInDesc && m_pOutDesc);
	RV_ASSERT(m_pInDesc->type  == m_pOutDesc->type);
	
	if (!m_inputPad.IsConnected() || !m_outputPad.IsConnected()){
		return M_DISCONNECTED;
	}

	if (m_pInDesc->type == MX_MT_AUDIO){
		CWaveConverter* pCxter = new  CWaveConverter ;
		RV_ASSERT(pCxter);

		MxAudioDesc* pSrc  =(MxAudioDesc*) m_pInDesc;
		MxAudioDesc* pDst  =(MxAudioDesc*) m_pOutDesc;

		if ( pCxter->Create(pSrc->channelLayout, pSrc->sampleRate, GetAudioSampleFormat(pSrc->sampleFormat) , \
			              pDst->channelLayout, pDst->sampleRate,  GetAudioSampleFormat(pDst->sampleFormat) )){

				m_nState = MS_PLAYING;
				m_pConverter = pCxter;
				return M_OK;
		}

		delete pCxter;

		return M_FAILED;		    

	}
	else if (m_pInDesc->type == MX_MT_VIDEO){
		CImageConverter* pCxter =new  CImageConverter ;
		RV_ASSERT(pCxter);

		MxVideoDesc* pSrc  =(MxVideoDesc*) m_pInDesc;
		MxVideoDesc* pDst  =(MxVideoDesc*) m_pOutDesc;

		if ( pCxter->Create( GetVideoPixelFormat(pSrc->format) , pSrc->width, pSrc->height  , \
			               GetVideoPixelFormat(pDst->format) , pDst->width, pDst->height )){
				
				m_nState = MS_PLAYING;
				m_pConverter = pCxter;
				return M_OK;
		}

		delete pCxter;

		return M_FAILED;

	}

	return M_UNKNOWN_TYPE; 

}

void CStreamAdapter::Stop()
{
	CUtAutoLock lock(&m_cs);

	if (m_nState == MS_STOPPED) return;

	RV_ASSERT(m_pInDesc && m_pOutDesc);
	RV_ASSERT(m_pInDesc->type  == m_pOutDesc->type);
	RV_ASSERT(m_pConverter);


	m_nState = MS_STOPPED;
		 
	if (m_pInDesc->type == MX_MT_AUDIO){
		CWaveConverter* pCxter = (CWaveConverter*)m_pConverter;

		pCxter->Destroy();

		delete pCxter;
		m_pConverter = NULL;

	}
	else if (m_pInDesc->type == MX_MT_VIDEO){
		CImageConverter* pCxter = (CImageConverter*)m_pConverter;
		pCxter->Destroy();

		delete pCxter;
		m_pConverter = NULL;

	}


}

 BOOL CStreamAdapter::OnPadConnected(CPad* pOutputPad, CPad* pInputPad) 
 {
	 if (&m_outputPad == pOutputPad){
		 RV_ASSERT(m_pOutDesc);		 
		 if (!pOutputPad->NegotiateDescriptor(m_pOutDesc)){
			 MxDescriptor* pDesc = pOutputPad->EnquireDescriptor();
			 RV_ASSERT(pDesc);

			 if (pDesc->type != m_pOutDesc->type) return FALSE;

			 m_pOutDesc =  DuplicateDescriptor(pDesc, m_pOutDesc);
		 }
	 }

	 if (pInputPad == &m_inputPad){
		 MxDescriptor* pDesc = pOutputPad->GetFormatDescriptor();
		 RV_ASSERT(pDesc);

		 m_pInDesc = DuplicateDescriptor(pDesc, m_pInDesc);
		 m_pOutDesc =  DuplicateDescriptor(pDesc, m_pOutDesc);
	 }

	 return TRUE;

 }
 


 void CStreamAdapter::OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad)
 {

	 if (pInputPad == &m_inputPad)
	 {
		 if (m_pInDesc){
			 mxDestroyDescriptor(m_pInDesc );
			 m_pInDesc = NULL; 
		 }

		 if (m_pOutDesc){
			 mxDestroyDescriptor(m_pOutDesc );
			 m_pOutDesc = NULL; 
		 }

		 m_outputPad.Disconnect();		 

	 }

 }

 ////输出PIN调用connect的时候被调用，媒体对象的输入PIN必须重载该
 ////函数，以表示是否支持指定的媒体类型。
 //BOOL CStreamAdapter::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
 //{
	//RV_ASSERT(pInputPad == &m_inputPad);

	// return (type != MX_MT_UNKNOWN);
 //}

 ////获得媒体对象输出PIN的媒体类型
 //MX_MEDIA_TYPE CStreamAdapter::GetMediaType(CPad* pOutputPad)
 //{
	// RV_ASSERT(pOutputPad == &m_outputPad);

	// if (NULL == m_pOutDesc) return  MX_MT_UNKNOWN;

	// return m_pOutDesc->type;
 //}

 //MxDescriptor* CStreamAdapter::GetFormatDescriptor(CPad* pPad)
 //{
	// if (pPad == &m_inputPad) return m_pInDesc;
	//
	// if (pPad == &m_outputPad) return m_pOutDesc;

	// return NULL;

 //}

 //当接收到一个SAMPLE的时候,产生该事件。
 BOOL CStreamAdapter::OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)
 { 

	 CUtAutoLock lock(&m_cs);

	 if (m_nState != MS_PLAYING)
	 {
		 if (pErrCode) *pErrCode = M_TERMINATED;
		 return FALSE;
	 }

	  RV_ASSERT(pInputPad && hSample);    
	 RV_ASSERT(pInputPad == &m_inputPad);
	  RV_ASSERT(m_pConverter);


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
 MX_HANDLE CStreamAdapter::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode)
 {

	 CUtAutoLock lock(&m_cs);

	 /*RV_ASSERT(NULL ==hSample);*/
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

 MX_HANDLE  CStreamAdapter::ConvertSample(MX_HANDLE hSrc)
 {	

	  RV_ASSERT(m_pInDesc && m_pOutDesc);
	  RV_ASSERT(m_pInDesc->type  == m_pOutDesc->type);
	
	  MX_HANDLE hNew = NULL;

	 if (m_pInDesc->type  == MX_MT_VIDEO  ){
		RV_ASSERT(m_pConverter);

		void* pSampleData = mxGetSampleData(hSrc); 

		CImageConverter* pCxter = (CImageConverter*)m_pConverter;
		RV_ASSERT(	m_pInDesc->dataSize == pCxter->GetSrcBufferSize());

		UINT nDstSize = pCxter->GetDestBufferSize();	
		RV_ASSERT(	nDstSize == m_pOutDesc->dataSize );

		hNew = mxCreateSampleEx(m_pOutDesc, sizeof(MxVideoDesc), nDstSize);
		RV_ASSERT(hNew);

		BOOL ret = pCxter->ExcuteEx((uint8_t*)pSampleData, m_pInDesc->dataSize, (uint8_t*)mxGetSampleData(hNew) , nDstSize);
		RV_ASSERT(ret);

	//	return TRUE;
		 
	}
	else if (m_pInDesc->type  == MX_MT_AUDIO ){
		RV_ASSERT(m_pConverter);
		CWaveConverter* pCxter = (CWaveConverter*)m_pConverter;

		void* pSampleData = mxGetSampleData(hSrc); 
		MxAudioDesc* pSrcDesc =(MxAudioDesc*) mxGetSampleDescriptor(hSrc);

		MxAudioDesc* pDestDesc =(MxAudioDesc*) m_pOutDesc;

		int newsize=0;

		int nSampleCount = pCxter->GetDestSampleCount(pSrcDesc->sampleCount,  &newsize);
		if (nSampleCount > 0) {
			RV_ASSERT(newsize >0);
		}

		ResetAudioDescEx( (MxAudioDesc*)m_pOutDesc, pDestDesc->sampleFormat, pDestDesc->sampleRate, pDestDesc->channelLayout, nSampleCount); 

		hNew = mxCreateSampleEx(m_pOutDesc, sizeof(MxAudioDesc), newsize);
		RV_ASSERT(hNew);

		////output to file.
		//MX_AUDIO_DATA* pData = new MX_AUDIO_DATA;
		//RV_ASSERT(pData);
		//pData->pBuffer = new char[newsize];
		//RV_ASSERT(pData->pBuffer);
		//pData->frameCount = nSampleCount;
		//pData->dataSize = newsize;

		
		BOOL ret =pCxter->ExcuteEx((uint8_t *)pSampleData, pSrcDesc->dataSize,  pSrcDesc->sampleCount , (uint8_t *)mxGetSampleData(hNew), newsize, nSampleCount);
 	
		RV_ASSERT(ret);

		//return TRUE;

		 

 	}
	else {
		RV_ASSERT(0);

		 
	}

	return hNew;
 }

 //是否接受输入PAD的流媒体格式描述符
 //具体格式协商的时候用，一般输入PIN所在的MO实现。
 BOOL CStreamAdapter::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
 {
	RV_ASSERT(pInputPad && pDescriptor);
	RV_ASSERT(pInputPad  == &m_inputPad);
	RV_ASSERT(pInputPad->IsConnected());
	RV_ASSERT(m_pInDesc);

	if (m_pInDesc->type != pDescriptor->type ){
		 RV_ASSERT(0);
		return FALSE;
	} 
	

	m_pInDesc = DuplicateDescriptor(pDescriptor, m_pInDesc);

	
	return HandleDescInChanged();
		
 
 }


 BOOL    CStreamAdapter::HandleDescInChanged( )
 {
	
	 CUtAutoLock lock(&m_cs);

     if (m_nState == MS_STOPPED) return TRUE;
	  RV_ASSERT(m_pConverter);
	 RV_ASSERT(m_pInDesc && m_pOutDesc );


	 if (m_pInDesc->type == MX_MT_AUDIO){
		 CWaveConverter* pCxter = (CWaveConverter*)m_pConverter;
		 pCxter->Destroy();

		 MxAudioDesc* pSrc  =(MxAudioDesc*) m_pInDesc;
		 MxAudioDesc* pDst  =(MxAudioDesc*) m_pOutDesc;

		 
		 return  pCxter->Create(pSrc->channelLayout, pSrc->sampleRate,  GetAudioSampleFormat(pSrc->sampleFormat) , \
			                  pDst->channelLayout, pDst->sampleRate,  GetAudioSampleFormat(pDst->sampleFormat) ) ;


	 }
	 else if (m_pInDesc->type == MX_MT_VIDEO){
		 CImageConverter* pCxter = (CImageConverter*)m_pConverter;
		 pCxter->Destroy();

		 MxVideoDesc* pSrc  =(MxVideoDesc*) m_pInDesc;
		 MxVideoDesc* pDst  =(MxVideoDesc*) m_pOutDesc;

		 return  pCxter->Create( GetVideoPixelFormat(pSrc->format) , pSrc->width, pSrc->height  , \
			                   GetVideoPixelFormat(pDst->format) , pDst->width, pDst->height );
		 		 
	 }

	 return FALSE;

 }