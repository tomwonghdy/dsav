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
#include "AnySink.h"


CAnySink::CAnySink(void)
{

	 

	m_nSampleCount = 1;

	m_inputPad.m_pOwner = this;
	m_inputPad.SetCaptureType( CPad::CT_ANY);

	m_nWorkMode = WM_ACTIVE ;

	m_seqSample = rvCreateSequence();
	RV_ASSERT(m_seqSample); 


	m_nMediaType = MX_MT_VIDEO;

	m_nTimeOut = -1; //ºÁÃë

}


CAnySink::~CAnySink(void)
{
	ClearSample();

	if (m_seqSample){
		 
		rvDestroySequence(m_seqSample);
		m_seqSample = NULL;
	}

}


BOOL CAnySink::OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)
{
	RV_ASSERT(0);
	return FALSE;
}



DWORD WINAPI ThreadPumpProcAs(  LPVOID lpParameter)
{
	CAnySink* pSp = (CAnySink* )lpParameter;

	pSp->HandlePump();

	return 0;

}

M_RESULT CAnySink::Play(const char* strUrl )
{
	m_nState = MS_PLAYING ;
	 
	ClearSample();

	if (m_nWorkMode == WM_ACTIVE){
	  
		m_hPumpThread = ::CreateThread(NULL, 0, ThreadPumpProcAs, this, 0, 0);
		RV_ASSERT(m_hPumpThread);
		 
	}

	return M_OK;

}

void CAnySink::Pause(BOOL bResume){

	m_nState = MS_PAUSED;

}

void CAnySink::Stop(){
	m_nState = MS_STOPPED ;


	if (m_nWorkMode == WM_ACTIVE){
		RV_ASSERT(m_hPumpThread);

		::WaitForSingleObject( m_hPumpThread, INFINITE);
		m_hPumpThread =NULL;
		 
	} 

	//


}

BOOL CAnySink::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{
	RV_ASSERT(pInputPad == &m_inputPad);
	RV_ASSERT(pDescriptor);

	if (m_nMediaType != pDescriptor->type) return FALSE;

	if (pDescriptor->type == MX_MT_AUDIO)
	{
		m_descriptor.audio = *((MxAudioDesc*)pDescriptor);
	}
	else if (pDescriptor->type == MX_MT_VIDEO)
	{
		m_descriptor.video = *((MxVideoDesc*)pDescriptor);
	}
	else
	{
		RV_ASSERT(0);
	}


	return TRUE;
}

//
//BOOL CAnySink::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
//{
//	return (type == m_nMediaType);
//}
//
//
//MxDescriptor* CAnySink::GetFormatDescriptor(CPad* pPad)
//{
//	if (pPad == &m_inputPad){
//		return (MxDescriptor*)(&m_descriptor);
//	}
//
//	return NULL;
//}

void CAnySink::ClearSample(void)
{

	if (m_seqSample)
	{ 
		RvSeqReader sr;
		MX_HANDLE hTmp=NULL;

		RV_BEGIN_READ_SEQ(m_seqSample, &sr);

		while(!RV_IS_SEQ_END(&sr)){
			RV_READ_FR_SEQ_(&sr, MX_HANDLE,  hTmp);
			mxDestroySample(hTmp);
		}

		RV_END_READ_SEQ(m_seqSample, &sr);	
	}
}


void CAnySink::HandlePump(void)
{
	MX_HANDLE hSample = NULL;
	 
	if (!m_inputPad.IsConnected())
	{
		if (m_pfStatusReport )
		{
			m_pfErrorOccurred(this, M_TERMINATED, m_pUserData);
		}

		return;
	}


	int nErrCode = M_OK;

	int count =0;

	__int64 st = mxGetCurrentTime();

	__int64 timeout = m_nTimeOut * 1000;

	while (m_nState != MS_STOPPED  ) 
	{ 
		if (m_nTimeOut > 0)
		{
			if ((mxGetCurrentTime() - st)  >= timeout)
			{
				if (m_pfStatusReport)
				{
					m_pfStatusReport(this, AS_SC_TIMEOUT, m_pUserData);
				}
				break;
			}
		}

		if (m_nState == MS_PAUSED) {
			::Sleep(10); continue;
		}

		count = rsqGetCount(m_seqSample);
		if (count < m_nSampleCount)
		{ 
			hSample = m_inputPad.Fetch(&nErrCode);
			
			if (NULL == hSample){
				if (m_pfErrorOccurred && nErrCode != M_OK)
				{
					m_pfErrorOccurred(this, nErrCode, m_pUserData);
				}

				::Sleep(10) ;
				continue;
			}

			rsqAddLast_(m_seqSample,  hSample);

		}
		else{
			if (m_pfStatusReport)
			{
				m_pfStatusReport(this, AS_SC_COMPLETED, m_pUserData);
			}

			break;
		}
	}

}

int  CAnySink::GetSampleReceived()
{  
	if (m_seqSample)
	{ 
		return rsqGetCount(m_seqSample);
	}
	return -1;
}


MX_HANDLE CAnySink::GetSampleAt(int index)
{
	if (index >=0 && index < rsqGetCount(m_seqSample))
	{
		MX_HANDLE hSample=NULL;
		hSample = rsqGetItemValue_(m_seqSample, index );

		return hSample;
	}

	return NULL;
}