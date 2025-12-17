
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
#ifndef  __VIDEO_FILTER_H_INCLUDED_
#define  __VIDEO_FILTER_H_INCLUDED_

#include "vmo.h"
#include "..\davsdk\includes\transform.h"

#include "..\davsdk\includes\InputPad.h"
#include "..\davsdk\includes\OutputPad.h"


class  VMO_API  CVideoFilter:	public CTransform
{
public:
	enum TYPE{
		VF_NONE = 0,
		VF_BLUR ,   //使变模糊
		VF_GRAY ,   //转成灰度图
	};

	CVideoFilter(void);
	virtual ~CVideoFilter(void);

	virtual M_RESULT Play(const char* strUrl);
	virtual void Stop();

	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad) ;
	virtual void OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad) ;

	 

	//当接收到一个SAMPLE的时候,产生该事件。
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode);

	////当接收方通过PAD接收SAMPLE时,产生该事件。
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode) ;

	//是否接受输入PAD的流媒体格式描述符
	//具体格式协商的时候用，一般输入PIN所在的MO实现。
	virtual BOOL Accept(CPad* pInputPad, MxDescriptor* pDescriptor) ;

	CInputPad*  GetInputPad() { return &m_inputPad; };
	COutputPad* GetOutputPad() { return &m_outputPad; };

	void SetFilterType(CVideoFilter::TYPE type);
	int  GetFilterType() { return m_nType; };
	 
protected:
	
	MX_HANDLE  ConvertSample(MX_HANDLE hSrc);
	BOOL       HandleDescInChanged();
	 
	MxVideoDesc m_inDesc;
	MxVideoDesc m_outDesc;

	CInputPad  m_inputPad;
	COutputPad m_outputPad;

	TYPE  m_nType;

	CRITICAL_SECTION m_cs;
	 
};




#endif
