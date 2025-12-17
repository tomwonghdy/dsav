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
#ifndef __MX_MP3_SINK_H_INCLUDED_
#define __MX_MP3_SINK_H_INCLUDED_

//注意：
//该类工作在主动模式,
//如果有音频流，将以音频的时间戳为准，
//但该类并不保证音频和视频数据同步。
//当音频和视频数据都有的时候，如果音频数据
//或视频数据无法从上家获取到的话，将立即
//终止。
//************************************************

#include "ffmo.h"
 

#include "..\davsdk\includes\sink.h"
#include "..\davsdk\includes\InputPad.h"

//designed for mp3 encode only
class FFMO_API CMp3Sink: public CSink
{
public:
	CMp3Sink(void);
	virtual ~CMp3Sink(void);
	

	virtual M_RESULT Play(const char* strUrl );
	virtual void Pause(BOOL bResume);
	virtual void Stop();

	CInputPad* GetPad() {return &m_audioPad;};
 
	//in  seconds
	void SetDuration(double val);
	 
	  

	void HandleMux();

	//具体格式协商。
	virtual BOOL Accept(CPad* pInputPad, MxDescriptor* pDescriptor);

	//媒体格式大体协商。
	virtual BOOL CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) ;

	//如果输入PIN是推模式，需要重载，该函数
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE  hSample, int* pErrCode){ return FALSE;};
	 
	 virtual MxDescriptor* GetFormatDescriptor(CPad* pPad);
protected:
	 
 	int  Mux(void);	
	 
private:
	  
	MxAudioDesc  m_audioDesc;	
	CInputPad m_audioPad;
	  
	//double  m_nDuration; 
	  
	HANDLE m_hOutputThread;	

	struct Pimp;
	Pimp*	m_pimp;

	   
};


#endif
