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
#include "StreamPump.h"
#include "avlib.h"



#define SP_DEF_FRAME_RATE  30

//#define MAX_VIDEO_SAMPLE_COUNT_BUFFERED  5
//#define MAX_AUDIO_SAMPLE_COUNT_BUFFERED  3
#define MIN_BUFFER_COUNT   6


// 初始化时钟
void init_clock(PumpClock* c) {
	c->pts = 0;
	c->pts_drift = 0;
	c->last_updated = 0;
	c->serial = 0;
}

// 设置时钟
void set_clock(PumpClock* c, double pts, int serial) {
	double time = av_gettime_relative() / 1000000.0;
	c->pts = pts;
	c->last_updated = time;
	c->serial = serial;
}

// 获取时钟
double get_clock(PumpClock* c) {
	if (c->serial == 0) return NAN;
	double time = av_gettime_relative() / 1000000.0;
	return c->pts + (time - c->last_updated);
}

CStreamPump::CStreamPump(void)
{
	m_nFrameRate = SP_DEF_FRAME_RATE;

	m_nWorkMode = WM_ACTIVE;

	m_audioInPad.SetCaptureType(CPad::CT_PULL);
	m_audioOutPad.SetCaptureType(CPad::CT_PUSH);
	m_audioInPad.SetMediaType(MX_MT_AUDIO);
	m_audioOutPad.SetMediaType(MX_MT_AUDIO);

	m_videoInPad.SetCaptureType(CPad::CT_PULL);
	m_videoOutPad.SetCaptureType(CPad::CT_PUSH);
	m_videoInPad.SetMediaType(MX_MT_VIDEO);
	m_videoOutPad.SetMediaType(MX_MT_VIDEO);

	m_bSyncMode = FALSE;
	m_bDiscardSampleToDownstream = TRUE;

	// m_pDescriptor = NULL;
	m_hPump1Thread = NULL;
	m_hPump2Thread = NULL;

 

	init_clock(&m_audioClock);
	init_clock(&m_videoClock);


	m_audioInPad.m_pOwner = this;
	m_audioOutPad.m_pOwner = this;

	m_videoInPad.m_pOwner = this;
	m_videoOutPad.m_pOwner = this;

	//	m_hSendThread = NULL;
	//	m_hSampleSemaphor =NULL;

	::InitializeCriticalSection(&m_cs);

}

CStreamPump::~CStreamPump(void)
{
	::DeleteCriticalSection(&m_cs);
}


DWORD WINAPI ThreadPumpAudioProc(LPVOID lpParameter)
{
	CStreamPump* pSp = (CStreamPump*)lpParameter;

	pSp->FetchAndSend(pSp->GetInputPad(MX_MT_AUDIO), pSp->GetOutputPad(MX_MT_AUDIO));

	return 0;

}

DWORD WINAPI ThreadPumpVideoProc(LPVOID lpParameter)
{
	CStreamPump* pSp = (CStreamPump*)lpParameter;

	pSp->FetchAndSend(pSp->GetInputPad(MX_MT_VIDEO), pSp->GetOutputPad(MX_MT_VIDEO));

	return 0;

}
 

DWORD WINAPI ThreadPumpVideoSyncProc(LPVOID lpParameter)
{
	CStreamPump* pSp = (CStreamPump*)lpParameter;

	pSp->FetchVideoAndSend(pSp->GetInputPad(MX_MT_VIDEO), pSp->GetOutputPad(MX_MT_VIDEO));

	return 0;

}
//DWORD WINAPI ThreadPumpSyncoProc(LPVOID lpParameter)
//{
//	CStreamPump* pSp = (CStreamPump*)lpParameter;
//
//	pSp->HandlePumpInSyncMode();
//
//	return 0;
//
//}


void CStreamPump::FetchAndSend(CInputPad* pInPad, COutputPad* pOutPad)
{
	MX_HANDLE hSample = NULL;
	RV_ASSERT(m_nWorkMode == WM_ACTIVE);

	if (!pInPad->IsConnected()) return;

	int nErrCode = M_OK;

	while (m_nState != MS_STOPPED)
	{
		if (m_nState == MS_PAUSED) {
			::Sleep(30); continue;
		}

		if (hSample == NULL) {
			hSample = pInPad->Fetch(&nErrCode);

			if (NULL == hSample) {
				::Sleep(30); continue;
			}
		}

		double dura = mxGetSampleDuration(hSample);
		if (dura < 0.01) {
			dura = (1. / RV_CHECK_RANGE(1., 100., m_nFrameRate));
		}

		if (pOutPad->IsConnected()) {
			if (pOutPad->Pass(hSample, &nErrCode) == FALSE) {
				if (!m_bDiscardSampleToDownstream) {
					if (M_BUFFER_FULL == nErrCode || M_STOPPED == nErrCode) {
						Sleep(10);
						continue;
					}
				}
				mxDestroySample(hSample);
				hSample = NULL;
			}
			else {				
				hSample = NULL;
			}

			SleepAv((UINT)(dura * 1000000));
		}
		else {
			if (this->m_fnSamplePass) {
				m_fnSamplePass(this, pOutPad, hSample, this->m_pUserData);
			}
			
			mxDestroySample(hSample);
			hSample = NULL;

			SleepAv((UINT)(dura * 1000000));
		}
	}

	if (hSample) {
		mxDestroySample(hSample);
	}
}
 

void CStreamPump::FetchVideoAndSend(CInputPad* pInPad, COutputPad* pOutPad)
{
	MX_HANDLE hSample = NULL;
	RV_ASSERT(m_nWorkMode == WM_HYBRID);

	if (!pInPad->IsConnected()) return;

	int nErrCode = M_OK;

	while (m_nState != MS_STOPPED)
	{
		if (m_nState == MS_PAUSED) {
			::Sleep(1); continue;
		}

		if (hSample == NULL) {
			hSample = pInPad->Fetch(&nErrCode);
		}
		 
		if (hSample) { 
			double tb = mxGetSampleTimeBase(hSample);
			__int64 pts = mxGetSamplePTS(hSample);
			double dura = mxGetSampleDuration(hSample);
			__int64 serial =(__int64) mxGetSampleDuration(hSample);
			  
			double video_pts =0., actual_delay=0.;
			if (pts != AV_NOPTS_VALUE) {
				video_pts =  pts * tb;
			}
			  
			// 视频同步到音频
			double audio_time = get_clock(&m_audioClock);
			double diff = video_pts - audio_time;

			// 同步阈值
			double sync_threshold = RV_MAX(0.04, RV_MIN(0.1, dura));

			if (fabs(diff) < sync_threshold || isnan(diff)) {
				// 在同步阈值内
				actual_delay = dura;
			}
			else if (diff > 0) {
				// 视频比音频快，等待
				actual_delay = dura + diff * 0.9;
			}
			else {
				// 视频比音频慢，加速（可能丢帧）
				actual_delay = dura;
			}

			// 限制延迟范围
			actual_delay = FFMAX(0.01, FFMIN(actual_delay, 0.1));

			// 等待合适时间
			av_usleep((int)(actual_delay * 1000000.0));

			// 显示帧
			if (pOutPad->IsConnected()) {
				if (pOutPad->Pass(hSample, &nErrCode) == FALSE) {
					mxDestroySample(hSample);
					hSample = NULL;
				}
				else {
					hSample = NULL;
				}
			}
			else {
				if (this->m_fnSamplePass) {
					m_fnSamplePass(this, pOutPad, hSample, this->m_pUserData);
				}
				mxDestroySample(hSample);
				hSample = NULL;
			}

			// 更新视频时钟
			set_clock(&m_videoClock, video_pts, serial);

			/*if (m_nNextVideoPts <= m_nNextAudioPts)
		    { 
				

				SleepAv(n * 1000*1000);
			}
			else { 
				::Sleep(1); continue;
			}*/

		}
		else {
			::Sleep(1); continue;
		}
		 

	}

	if (hSample) {
		mxDestroySample(hSample);
	}
}


M_RESULT CStreamPump::Play(const char* strUrl)
{
	if (m_nState == MS_PLAYING) return M_OCCUPIED;
	  
	m_nState = MS_PLAYING;

	//m_nNextAudioPts = 0;
	//m_nNextVideoPts = 0;
	init_clock(&m_audioClock);
	init_clock(&m_videoClock);

	if (!m_bSyncMode) {
	 
		m_hPump1Thread = ::CreateThread(NULL, 0, ThreadPumpAudioProc, this, 0, 0);
		RV_ASSERT(m_hPump1Thread);
		m_hPump2Thread = ::CreateThread(NULL, 0, ThreadPumpVideoProc, this, 0, 0);
		RV_ASSERT(m_hPump2Thread);
	}
	else {
		m_hPump1Thread = ::CreateThread(NULL, 0, ThreadPumpVideoSyncProc, this, 0, 0);
		RV_ASSERT(m_hPump1Thread);
	}
	  
	return M_OK;

}

void CStreamPump::Pause(BOOL bResume) {

	m_nState = MS_PAUSED;

}

void CStreamPump::Stop() {

	if (m_nState == MS_STOPPED) return;

	m_nState = MS_STOPPED;
	 
	if (m_hPump1Thread)
	{
		::WaitForSingleObject(m_hPump1Thread, INFINITE);
		::CloseHandle(m_hPump1Thread);
		m_hPump1Thread = NULL;
	}

	if (m_hPump2Thread)
	{
		::WaitForSingleObject(m_hPump2Thread, INFINITE);
		::CloseHandle(m_hPump2Thread);
		m_hPump2Thread = NULL;
	}



}

BOOL CStreamPump::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	//audio pad
	if (pOutputPad == &m_audioOutPad) {
		if (m_audioInPad.IsConnected())
		{
			//RV_ASSERT(m_pDescriptor);
			return   pOutputPad->NegotiateDescriptor(m_audioInPad.GetFormatDescriptor());
		}
	}

	if (pInputPad == &m_audioInPad) {
		MxDescriptor* pDesc = pInputPad->GetFormatDescriptor();

		if (m_audioOutPad.IsConnected())
		{
			return m_audioOutPad.NegotiateDescriptor(pInputPad->GetFormatDescriptor());
		}
		return TRUE;
	}

	//video pad
	if (pOutputPad == &m_videoOutPad) {
		if (m_videoInPad.IsConnected())
		{
			//RV_ASSERT(m_pDescriptor);
			return   pOutputPad->NegotiateDescriptor(m_videoInPad.GetFormatDescriptor());
		}
	}

	if (pInputPad == &m_videoInPad) {
		MxDescriptor* pDesc = pInputPad->GetFormatDescriptor();

		if (m_videoOutPad.IsConnected())
		{
			return m_videoOutPad.NegotiateDescriptor(pInputPad->GetFormatDescriptor());
		}
		return TRUE;
	}

	//默认情况下，一般返回TRUE，以支持连接。
	return FALSE;

}
void CStreamPump::OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad)
{
	//if (pInputPad == &m_inputPad)
	//{ 
	//	m_outputPad.Disconnect();
	//}

}
//   

//是否接受输入PAD的流媒体格式描述符
//具体格式协商的时候用，一般输入PIN所在的MO实现。
BOOL CStreamPump::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{
	RV_ASSERT(pInputPad && pDescriptor);
	//RV_ASSERT(pInputPad == &m_inputPad);
	RV_ASSERT(pInputPad->IsConnected());


	if (pInputPad == &this->m_audioInPad) {
		mxCopyDescriptor(pDescriptor, this->m_audioInPad.GetFormatDescriptor());
		mxCopyDescriptor(pDescriptor, this->m_audioOutPad.GetFormatDescriptor());
		if (m_audioOutPad.IsConnected()) {
			return  m_audioOutPad.NegotiateDescriptor(pDescriptor);
		}

		return TRUE;
	}
	else if (pInputPad == &this->m_videoInPad) {
		mxCopyDescriptor(pDescriptor, this->m_videoInPad.GetFormatDescriptor());
		mxCopyDescriptor(pDescriptor, this->m_videoOutPad.GetFormatDescriptor());
		if (m_videoOutPad.IsConnected()) {
			return  m_videoOutPad.NegotiateDescriptor(pDescriptor);
		}

		return TRUE;
	}


	return FALSE;

}




CInputPad* CStreamPump::GetInputPad(int mediaType)
{
	if (mediaType == MX_MT_AUDIO) return &m_audioInPad;
	else if (mediaType == MX_MT_VIDEO) return &m_videoInPad;
	 

	return NULL;
}

COutputPad* CStreamPump::GetOutputPad(int mediaType)
{
	if (mediaType == MX_MT_AUDIO) return &m_audioOutPad;
	else if (mediaType == MX_MT_VIDEO) return &m_videoOutPad;
	
	return NULL;
}


void  CStreamPump::SetSysncMode(BOOL flag)
{
	if (flag == m_bSyncMode) return;
	if (m_nState != MS_STOPPED) return;

	m_bSyncMode = flag;

	if (m_bSyncMode) {
		m_audioInPad.SetCaptureType(CPad::CT_PULL);
		m_audioOutPad.SetCaptureType(CPad::CT_PULL);

		m_videoInPad.SetCaptureType(CPad::CT_PULL);
		m_videoOutPad.SetCaptureType(CPad::CT_PUSH);

		m_nWorkMode = WM_HYBRID;
	}
	else {
		m_audioInPad.SetCaptureType(CPad::CT_PULL);
		m_audioOutPad.SetCaptureType(CPad::CT_PUSH);

		m_videoInPad.SetCaptureType(CPad::CT_PULL);
		m_videoOutPad.SetCaptureType(CPad::CT_PUSH);

		m_nWorkMode = WM_ACTIVE;
	}


}

BOOL  CStreamPump::IsSysncMode()
{
	return m_bSyncMode;
}

MX_HANDLE CStreamPump::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode)
{
	if (pOutputPad == &m_audioOutPad) {
		if (m_audioInPad.IsConnected()) {
			MX_HANDLE hSample = m_audioInPad.Fetch(pErrCode);
			if (hSample) {
				if (this->m_fnSamplePass) {
					m_fnSamplePass(this, pOutputPad, hSample, this->m_pUserData);
				}

				ULONGLONG pts = mxGetSamplePTS(hSample);
				double    tb = mxGetSampleTimeBase(hSample);
				__int64     serial = mxGetSampleSerial(hSample);
				
				//m_nNextAudioPts +=  (pts* tb );  

				set_clock(&m_audioClock, pts * tb, serial);
			}

			return hSample;
		}
	}

	if (pErrCode) {
		*pErrCode = M_UNSUPPORT_PAD;
	}
	return	NULL;
}