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
#include "VideoRender.h"

#include "..\davsdk\includes\ffmo.h"
#include "..\davsdk\includes\mox.h"


 
GMold m_stretchMold = {RV_DI_STRETCH, -1, FALSE, FALSE};


CVideoRender::CVideoRender(void)
{
	m_bFixedFormat = FALSE;

	m_dc = NULL;
	m_hCaptureThread =NULL;

	m_nFps = 25;

	m_nFlipType = VR_FLIP_NONE;
		 
	m_bNewArrival = FALSE;
	m_hCurSample = NULL;

	
	//ResetVideoDescEx(&m_videoDesc, MX_VF_BGR, AV_DEFAULT_VIDEO_WIDTH,AV_DEFAULT_VIDEO_HEIGHT );
	

	m_inputPad.m_pOwner = this;
	m_outputPad.m_pOwner = this;
	m_inputPad.SetCaptureType(CPad::CT_PULL);
	m_outputPad.SetCaptureType(CPad::CT_PUSH);
	m_inputPad.SetMediaType(MX_MT_VIDEO);
	m_outputPad.SetMediaType(MX_MT_VIDEO);

	m_nWorkMode = WM_ACTIVE;

	//SetMode(WM_PASSIVE);

	::InitializeCriticalSection(&m_cs);

		
	


}

CVideoRender::~CVideoRender(void)
{
	if (m_dc){
		rvgDestroyContext(m_dc);
		m_dc =NULL;
	}

	if (m_hCurSample){
		mxDestroySample(m_hCurSample);
	    m_hCurSample = NULL;
	}

	::DeleteCriticalSection(&m_cs);

}
 

DWORD WINAPI PlayCaptureRenderProc(void* lpUser)
{

	CVideoRender* pCr= (CVideoRender*)lpUser;
	  
	DWORD n = pCr->GetFrameRate();

	//最多60帧,最小10帧
	if (n < 10) n=25;
	if (n > 60) n=25;

	int64_t  period =(int64_t)( (1000.0/n) * 1000 ); //微秒
	//	HWND hWndToRefresh = pPc->m_display.GetBindWnd();
	//	DWORD start = ::GetTickCount();

	int64_t remaining_time = 0;

	int64_t time= 0;

	for(;;){
		if (pCr->GetCurState() == CMediaObject::MS_STOPPED) break;
		
		if (pCr->GetCurState() == CMediaObject::MS_PAUSED){Sleep(10); continue; }

 
		time= GetAvTime() ;//GetAvTime是以微秒为单位

		pCr->HandleCapture();	
	//	pCr->Refresh();

		remaining_time = GetAvTime()  - time;


		if (remaining_time <= period)
		{
			SleepAv((unsigned)( (period - remaining_time)  ));
		}
		else
		{
			SleepAv(1000);//1毫秒
		}

	}
	 

	return 0;
}

M_RESULT CVideoRender::Play(const char* strUrl)
{
	
	m_nState = MS_PLAYING;

	m_bNewArrival = FALSE;

	if (m_nWorkMode == WM_ACTIVE){
		m_hCaptureThread = ::CreateThread(NULL, 0, PlayCaptureRenderProc, this, 0, NULL);
  	    RV_ASSERT(m_hCaptureThread);
	}


	return M_OK;
}
	
void CVideoRender::Pause(BOOL bResume)
{
	m_nState = MS_PAUSED;
}
	
void CVideoRender::Stop()
{
	m_nState = MS_STOPPED;

	if (m_hCaptureThread){
		RV_ASSERT(m_nWorkMode == WM_ACTIVE);

		::WaitForSingleObject(m_hCaptureThread, INFINITE);

		m_hCaptureThread = NULL;
	}

}

	
BOOL CVideoRender::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	if (pOutputPad == &m_outputPad){
		if (pOutputPad->NegotiateDescriptor((MxDescriptor*)pInputPad->GetFormatDescriptor()) == FALSE)
		{
			return FALSE;
		}
	}
	  
	return TRUE;
}
	
BOOL CVideoRender::OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)  {

	RV_ASSERT(m_nWorkMode ==  WM_PASSIVE);

	RV_ASSERT(pInputPad == &m_inputPad);
	RV_ASSERT(hSample);

	if (m_outputPad.IsConnected())
	{
		MX_HANDLE hTemp = mxDuplicateSample(hSample);
		RV_ASSERT(hTemp);

		if (FALSE == m_outputPad.Pass(hTemp, NULL))
		{
			mxDestroySample(hTemp);
		}
	}


	CUtAutoLock lock(&m_cs);
	//CUtAutoLock _lock(&m_cs);

	if (m_hCurSample){
		mxDestroySample(m_hCurSample);
	} 

	m_hCurSample = hSample;
	m_bNewArrival = TRUE;
	
	if (pErrCode) *pErrCode = M_OK;

	return TRUE;
}

 
BOOL     CVideoRender::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{
	RV_ASSERT(pInputPad == &m_inputPad); 
	RV_ASSERT(pDescriptor); 
	RV_ASSERT(pDescriptor->type == MX_MT_VIDEO); 

	MxVideoDesc* pDesc =  (MxVideoDesc*)pDescriptor;
	MxVideoDesc* pInDesc = (MxVideoDesc*)pInputPad->GetFormatDescriptor();

	RV_ASSERT(pInDesc);
	 
	if (m_bFixedFormat) {
		if (pInDesc->format != pDesc->format || pInDesc->width != pDesc->width || pInDesc->height != pDesc->height)
		{
			return FALSE;
		}

		return TRUE;
	}
	else {
		//if (pDesc->format != pInDesc->format)
		//{
		//	return FALSE;
		//}

		*pInDesc = *pDesc;

		if (m_outputPad.IsConnected())
		{
			if (!m_outputPad.NegotiateDescriptor((MxDescriptor*)pInDesc)) {
				m_outputPad.Disconnect();
			}
		}

		return TRUE;
	}
	

}
	
//
//BOOL    CVideoRender::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type)
//{
//	RV_ASSERT(pInputPad == &m_inputPad);
//
//	return  (MX_MT_VIDEO == type);
//}
//	
//MX_MEDIA_TYPE      CVideoRender::GetMediaType(CPad* pPad) 
//{
//
//	RV_ASSERT(pPad == &m_inputPad || pPad == &m_outputPad);
//
//	return MX_MT_VIDEO;
//}

void CVideoRender::AttachWin(HWND hWnd){

	RV_ASSERT(hWnd);
 

	if (m_dc) return;

	CUtAutoLock lock(&m_cs);

	m_dc = rvgCreateContext(hWnd, -1, -1, RV_CT_GENERIC);

	RV_ASSERT(m_dc);

	if (m_dc){
		rvgSetRvbLogoVisible(m_dc, FALSE);
		rvgSetViewMode(m_dc, RV_VM_STRETCH_KR);
		rvgRealize(m_dc, RV_ALL_LAYER, TRUE);
	}

}

void CVideoRender::DetachWin(){

	CUtAutoLock lock(&m_cs);

	if (m_dc){
		rvgDestroyContext(m_dc);
		m_dc =NULL;
	}
	
}

void CVideoRender::Refresh()
{

	 
	 if (::TryEnterCriticalSection(&m_cs) == FALSE) 
	{
			return;
	} 

 

	if (m_dc){
		if (m_nState == MS_PLAYING){
			   
			if (this->m_bNewArrival){
				rvgClear(m_dc, RV_ALL_GRAPHICS);
 				PaintSample(m_hCurSample);
   		        m_bNewArrival =FALSE;

		
				rvgRealize(m_dc, RV_ALL_LAYER, FALSE);
			
				rvgFlush(m_dc);
			}

	
		}
	}

	::LeaveCriticalSection(&m_cs);
}

//
//MxDescriptor* CVideoRender::GetFormatDescriptor(CPad* pPad)
//{
//	if (pPad) {
//		return pPad->GetFormatDescriptor();
//	}
//
//	return NULL;
//}


void  CVideoRender::SetMode(CMediaObject::WORK_MODE mode)
{
	if (m_nWorkMode == mode) return;

	m_nWorkMode = mode;

	if ( m_nWorkMode == WM_ACTIVE){
		m_inputPad.SetCaptureType(CPad::CT_PULL);
	}
	else if (m_nWorkMode == WM_PASSIVE) {
		m_inputPad.SetCaptureType(CPad::CT_PUSH);
	}

}

 

void CVideoRender::HandleCapture()
{

	if (m_inputPad.IsConnected()){

		MX_HANDLE hSample = m_inputPad.Fetch(NULL);

		if (hSample && m_outputPad.IsConnected())
		{
			MX_HANDLE hTemp = mxDuplicateSample(hSample);
			RV_ASSERT(hTemp);

			if (FALSE == m_outputPad.Pass(hTemp, NULL))
			{
				mxDestroySample(hTemp);
			}
		}

		if (hSample){

			CUtAutoLock lock(&m_cs);

			if (m_hCurSample){
				mxDestroySample(m_hCurSample);
			}

			m_hCurSample = hSample;
			m_bNewArrival = TRUE;

		}

	}

}


void CVideoRender::SetFrameRate(int rate)
{
	if (rate < 5) rate =5;
	if (rate > 60) rate =60;

	m_nFps = rate;
}

void CVideoRender::SetFlipType(int type)
{
	m_nFlipType = type;
}

void CVideoRender::SetPictureFormat(MX_VIDEO_FORMAT format, int width, int height) 
{
	if (m_bFixedFormat == FALSE) return;

	MxVideoDesc* pInDesc = (MxVideoDesc*)m_inputPad.GetFormatDescriptor();
	ResetVideoDescEx(pInDesc, format, width, height);
	 
}

void CVideoRender::PaintSample(MX_HANDLE hSample)
{
	
	RV_ASSERT(hSample);

	UINT sze = mxGetSampleDataSize(hSample);

	//RV_ASSERT(sze == m_videoDesc.dataSize);
	MxVideoDesc* pInDesc = (MxVideoDesc*)mxGetSampleDescriptor(hSample); // this->m_inputPad.GetFormatDescriptor();
	RV_ASSERT(pInDesc);

	 RV_ASSERT(pInDesc->type == MX_MT_VIDEO && pInDesc->depth == 24);

	RvImage img = rvCreateImageEx(RIT_RGB, pInDesc->width, pInDesc->height, TRUE, mxGetSampleData(hSample), sze);
	RV_ASSERT(img);

	if (this->m_nFlipType == VR_FLIP_HORI){
	   rvFlip(img, RV_DIR_HORI);
	}
	else if (this->m_nFlipType == VR_FLIP_VERT){
	   rvFlip(img, RV_DIR_VERT);
	}
	else if (this->m_nFlipType == VR_FLIP_BOTH){
	   rvFlip(img, RV_DIR_BOTH);
	}
	 
	if (m_dc){ 
		GMold* pOldPostModel = rvgSelectMold (m_dc, &m_stretchMold);

		rvgDrawImage(m_dc,  img);

		rvgSelectMold (m_dc, pOldPostModel);
	}	

	rvDestroyImage(img);

}
