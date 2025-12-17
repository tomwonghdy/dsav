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
#ifndef __MX_FRAME_SINK_H_INCLUDED_
#define __MX_FRAME_SINK_H_INCLUDED_

#include "sink.h"
#include "inputpad.h"

//STATUS CODE
#define AS_SC_COMPLETED  0
#define AS_SC_TIMEOUT   1



class MOX_API CAnySink : 	public CSink
{
public:
	CAnySink(void);
	~CAnySink(void);

	CInputPad*  GetInputPad() { return &m_inputPad; };
	//当接收到一个SAMPLE的时候,产生该事件。
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode) ;

	virtual M_RESULT Play(const char* strUrl );
	virtual void Pause(BOOL bResume);
	virtual void Stop();

	//是否接受输入PAD的流媒体格式描述符
	//具体格式协商的时候用，一般输入PIN所在的MO实现。
	virtual BOOL Accept(CPad* pInputPad, MxDescriptor* pDescriptor) ;

	//输出PIN调用connect的时候被调用，媒体对象的输入PIN必须重载该
	//函数，以表示是否支持指定的媒体类型。
	//virtual BOOL CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) ;
	//virtual MxDescriptor* GetFormatDescriptor(CPad* pPad);

	void SetSamplesWanted(int count){m_nSampleCount = count; };
	int  GetSampleReceived();

	MX_HANDLE GetSampleAt(int index);
	 
public:
	MX_MEDIA_TYPE m_nMediaType;
	int           m_nTimeOut; //单位：毫秒，默认：-1，表示永远不过时

private:
	CInputPad  m_inputPad;

	RvSequence m_seqSample;
	


	union  
	{
		MxAudioDesc audio;
		MxVideoDesc video;
		MxSubtitleDesc subtitle;
		MxActiveDesc   active;
	}m_descriptor;

	int m_nSampleCount ;
	HANDLE m_hPumpThread;

	void ClearSample(void);
public:
	void HandlePump(void);
};







#endif