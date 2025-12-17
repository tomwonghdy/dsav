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
#ifndef __STREAM_PUMP_H_INCLUDED_
#define __STREAM_PUMP_H_INCLUDED_

#include "ffmo.h"


#include "..\davsdk\includes\mox.h"
#include "..\davsdk\includes\transform.h"
#include "..\davsdk\includes\inputpad.h"
#include "..\davsdk\includes\outputpad.h"

  
typedef struct __stream_pump_clock_ {
	double pts;           // 时钟基础
	double pts_drift;     // 时钟漂移
	double last_updated;
	__int64    serial;
} PumpClock;

//WORR MODE
//#define SP_WM_UNKNOWN     -1
//#define SP_WM_POWERED     0
//#define SP_WM_POWERLESS   1

class FFMO_API CStreamPump:	public CTransform
{
public:
	CStreamPump(void);
 	virtual ~CStreamPump(void);

	virtual M_RESULT Play(const char* strUrl );
	virtual void Pause(BOOL bResume);
	virtual void Stop();

	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad) ;
	virtual void OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad) ;
	 
	////当接收到一个SAMPLE的时候,产生该事件。
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode){ return FALSE;};

	//////当接收方通过PAD接收SAMPLE时,产生该事件。
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode) ;
 
	//是否接受输入PAD的流媒体格式描述符
	//具体格式协商的时候用，一般输入PIN所在的MO实现。
	virtual BOOL Accept(CPad* pInputPad, MxDescriptor* pDescriptor) ;
	   
	CInputPad*  GetInputPad(int mediaType) ;
	COutputPad* GetOutputPad(int mediaType)  ;
	 
	void SetSysncMode(BOOL flag);
	BOOL IsSysncMode();

public:
	 
	//void HandlePumpInSyncMode();
	void FetchAndSend(CInputPad* pInPad, COutputPad* pOutPad);

	void FetchVideoAndSend(CInputPad* pInPad, COutputPad* pOutPad);
public:
	double  m_nFrameRate; //帧率
	 
	BOOL    m_bDiscardSampleToDownstream;

private:	

    BOOL    m_bSyncMode;   //同步音视频
	//double     m_nNextAudioPts;
	//double     m_nNextVideoPts;

	PumpClock  m_audioClock;
	PumpClock  m_videoClock;

	CInputPad  m_audioInPad;
	COutputPad m_audioOutPad;

	CInputPad  m_videoInPad;
	COutputPad m_videoOutPad;

	HANDLE m_hPump1Thread;	 
	HANDLE m_hPump2Thread;

	CRITICAL_SECTION m_cs ;


	 
};



#endif