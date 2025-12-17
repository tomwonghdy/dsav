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

#include "VcamFeeder.h"
#include "VideoClientPipe.h"

#include "..\davsdk\includes\ffmo.h"

#include <rvb\img.h>
#include <rvb\imio.h>

#include <rvb\pprs.h>
 

 
//该管道名称与LaodaoCam虚拟相机的一样
//#define PIPE_NAME  "\\\\.\\pipe\\LaodaoCamPipe"



//int CVcamFeeder::m_nInstanceCount = 0;
//CVcamFeeder* CVcamFeeder::m_pInstance = NULL;

CVcamFeeder::CVcamFeeder(void)
{
	m_nWorkMode = WM_PASSIVE;
	m_image =NULL;

	m_inputPad.m_pOwner = this;
	m_inputPad.SetCaptureType(CPad::CT_PULL);
	
	ResetVideoDescEx( &m_inDesc, MX_VF_BGR, 320, 240 );
	ResetVideoDescEx( &m_outDesc, MX_VF_BGR, 320, 240 );
	 
	m_nFlipType = FT_NONE;
	 
	m_strPipeName =NULL;
	m_bPipeStarted =FALSE;

	m_pClientPipe = new CVideoClientPipe;
	//m_nFps= 25;

	::InitializeCriticalSection(&m_cs);
}

CVcamFeeder::~CVcamFeeder(void)
{
	 if (m_pClientPipe){
		  CVideoClientPipe* pVc = (CVideoClientPipe*)m_pClientPipe;
		  delete pVc;
		  m_pClientPipe =NULL;
	 }

	 if (m_strPipeName){
	     delete []m_strPipeName;
		 m_strPipeName =NULL;
	 }

	 if (m_image){
	    rvDestroyImage(m_image);
		m_image =NULL;
	 }

	::DeleteCriticalSection(&m_cs);

}

void CVcamFeeder::Reset(int width, int height)
{
	  
	ResetVideoDescEx( &m_outDesc, MX_VF_BGR, width, height );
	  
	if (m_cvter.IsValid()){
		m_cvter.Destroy();
	}

	m_cvter.Create(GetVideoPixelFormat( m_inDesc.format),  m_inDesc.width , m_inDesc.height, \
			GetVideoPixelFormat(m_outDesc.format),  m_outDesc.width, m_outDesc.height );

	if (m_image){
		rviSetSize(m_image, width, height);
	}
	else{
	    m_image = rvCreateImage(RIT_RGB , width, height);
	}

}
//
//CVcamFeeder* GetInstance()
//{
//	if (m_pInstance == NULL)
//	{
//		m_pInstance =new CVcamFeeder;
//		RV_ASSERT(m_pInstance);
//	}
//	
//	++m_nInstanceCount;
//
//	return m_pInstance;    
//}
//
//void CVcamFeeder::ReleaseInstance()
//{
//	if (m_nInstanceCount == 0)
//	{
//		return;
//	}
//
//	--m_nInstanceCount;
//
//	if (0 == m_nInstanceCount)
//	{
//		RV_ASSERT(m_pInstance);
//		delete m_pInstance;
//		m_pInstance =NULL;
//	}    
//}


//
//DWORD WINAPI GetVcamFrameProc(void* lpUser)
//{
//
//	CVcamFeeder* pPc = (CVcamFeeder*)lpUser;
//
//
//	DWORD n = pPc->m_nFps;
//
//	//最多60帧,最小10帧
//	if (n < 10) n=15;
//	if (n > 60) n=25;
//
//	DWORD  period = 1000/n;
// 
//	DWORD start = ::GetTickCount();
//
//
//	for(;;){
//		if (pPc->GetCurState() == CMediaObject::MS_STOPPED) break;		 
//		 
//		 
//		pPc->HandleFrame();
//
//		n =  ::GetTickCount() - start;
//
//		start = ::GetTickCount();
//
//		if (n < period)
//		{
//			::Sleep(period - n);
//		}
//		else
//		{
//			Sleep(1);
//		}
//
//	}
//
//
//	return 0;
//}
//
//void CVcamFeeder::HandleFrame()
//{
//	MX_HANDLE hSample=NULL;
//	 
//	if (m_inputPad.IsConnected()==FALSE) return;
//	
//	int err=0;
//	hSample =m_inputPad.Fetch(&err);
//
//	if (hSample)
//	{
//		MxVideoDesc* pDesc = (MxVideoDesc *)mxGetSampleDescriptor(hSample);
//		RV_ASSERT(pDesc->dataSize == mxGetSampleDataSize(hSample));
//		RV_ASSERT(pDesc->width == m_videoDesc.width);
//		RV_ASSERT(pDesc->height == m_videoDesc.height);
//
//		//图象上下颠倒，需要FLIP
//	 	RvImage img = rvCreateImageEx(RIT_RGB, m_videoDesc.width, m_videoDesc.height, TRUE, mxGetSampleData(hSample), pDesc->dataSize);
//	 	RV_ASSERT(img);
//
//	//	rvSaveImage(img, "myloadaofeed.bmp");
//
//		if (m_nFlipType != FT_NONE)
//		{
//		     rvFlip(img, m_nFlipType);
//		}
//		 
//		 m_frameFeed.SetImageDataEx(GetVideoPixelFormat(m_videoDesc.format),  m_videoDesc.width, m_videoDesc.height, \
//		 	                       (char*)mxGetSampleData(hSample), pDesc->dataSize);
//		 
//
//	 	rvDestroyImage(img);
//		 
//		mxDestroySample(hSample);
//
//	}
//		 
//}


M_RESULT CVcamFeeder::Play(const char* strUrl )
{
	if (m_nState != MS_STOPPED) return M_FAILED;
	if (NULL == strUrl) return M_FAILED;
	
	CUtAutoLock lock(&m_cs);


	MX_ASSERT(m_pClientPipe);
	CVideoClientPipe* pVcp = (CVideoClientPipe*)m_pClientPipe;

    //支持重连,这里不需要判断是否成功启动
	m_bPipeStarted =pVcp->Start(strUrl );
	 
	if (m_bPipeStarted){
		Reset(pVcp->GetWidth(), pVcp->GetHeight());
	}


	if (this->m_strPipeName){
	    delete []m_strPipeName;
	}
	m_strPipeName =new TCHAR[strlen(strUrl) +1];
	MX_ASSERT(m_strPipeName);
	strcpy(m_strPipeName, strUrl);

	m_nState = MS_PLAYING;
	  

	return M_OK;
}

void     CVcamFeeder::Pause(BOOL bResume){
	m_nState = MS_PAUSED;

}

void     CVcamFeeder::Stop() 
{
	if (m_nState == MS_STOPPED) return;

	CUtAutoLock lock(&m_cs);

	m_nState = MS_STOPPED;
	m_bPipeStarted = FALSE;

	MX_ASSERT(m_pClientPipe);
	CVideoClientPipe* pVcp = (CVideoClientPipe*)m_pClientPipe;

	pVcp->Stop();

	m_cvter.Destroy();
	  
}
 

BOOL  CVcamFeeder::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{
	RV_ASSERT(pInputPad == &m_inputPad);
	RV_ASSERT(pDescriptor);

	MxVideoDesc* pDesc = (MxVideoDesc*)pDescriptor;
	RV_ASSERT(pDesc->type == MX_MT_VIDEO);

	m_inDesc = *pDesc; 

	return TRUE;
}
//
//MX_MEDIA_TYPE CVcamFeeder::GetMediaType(CPad* pPad) 
//{
//	RV_ASSERT( pPad == &m_inputPad );
//
//	return MX_MT_VIDEO;
//}
//
//BOOL CVcamFeeder::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
//{
//	RV_ASSERT( pInputPad == &m_inputPad );
//
//	return (type == MX_MT_VIDEO);
//
//
//}


BOOL CVcamFeeder::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	return TRUE;
}


BOOL CVcamFeeder::OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)
{
	CUtAutoLock lock(&m_cs);

	if (m_nState != MS_PLAYING){ 
	    if (pErrCode) *pErrCode = M_FAILED;
		return FALSE;
	}

	RV_ASSERT(m_pClientPipe);
	CVideoClientPipe* pVcp = (CVideoClientPipe*)m_pClientPipe;


	RV_ASSERT(hSample);
	RV_ASSERT(m_strPipeName);
	 
	MxVideoDesc* pDesc = (MxVideoDesc *)mxGetSampleDescriptor(hSample);
	RV_ASSERT(pDesc->dataSize == mxGetSampleDataSize(hSample));
 	RV_ASSERT(pDesc->width == m_inDesc.width);
 	RV_ASSERT(pDesc->height == m_inDesc.height);
	
	if (!m_bPipeStarted){
		m_bPipeStarted = pVcp->Start(m_strPipeName);
		if (m_bPipeStarted){
			Reset(pVcp->GetWidth(), pVcp->GetHeight());
		}
	}

	if (pVcp->IsConnected())
	{
		
	 	 
		MX_ASSERT(m_image);
		 
		UINT size = rviGetSize(m_image);
		const char* pImgData = (const char*)rviGetData(m_image);

		if (m_cvter.IsValid()){
			MX_ASSERT(rviGetSize(m_image) == m_outDesc.dataSize);
			BOOL b=m_cvter.ExcuteEx((uint8_t*)mxGetSampleData(hSample) , mxGetSampleDataSize(hSample), (uint8_t*)pImgData, size);
		    RV_ASSERT(b);		
		}
	 	 //图象上下颠倒，需要FLIP
		if (m_nFlipType != FT_NONE)
		{
			rvFlip(m_image, m_nFlipType);
		}

		pVcp->SetImageData(pImgData, size);
		 
		mxDestroySample(hSample);

		if (pErrCode) *pErrCode =M_OK;

		return TRUE;
		 
	}
	else{
	    if (m_bPipeStarted){
			pVcp->Stop();
			m_cvter.Destroy();
			m_bPipeStarted =FALSE;
		}
	}
	 
	if (pErrCode) *pErrCode =M_DATA_REJECTED;

	return FALSE;
}

void CVcamFeeder::SetImageSize(int width, int height)
{
	ResetVideoDescEx(&m_outDesc, MX_VF_BGR, width, height );
}

//MxDescriptor* CVcamFeeder::GetFormatDescriptor(CPad* pPad)
//{
//	RV_ASSERT(pPad == &m_inputPad);
//
//	return (MxDescriptor*)(&m_inDesc);
//}