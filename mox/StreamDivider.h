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
#ifndef  __STREAM_DIVIDER_H_INCLUDED_
#define  __STREAM_DIVIDER_H_INCLUDED_


#include "mox.h"
#include "transform.h"
#include "InputPad.h"
#include "OutputPad.h"

#define MX_SD_OUTPUT_COUNT  4

//该类仅将媒体样品进行分发（复制），不进行格式转换
//只运行在被动模式
//下游的对象缓冲区满的话，会等待
//该类可以经过下游的对象请求样本数据（从一个输出pad）
//下游请求的情况下，只能工作在异步模式
//仅允许一个下游请求数据,其它下游可以分享数据包
//，这样就不会发生冲突。
class MOX_API CStreamDivider :	public CTransform
{
public:
	CStreamDivider(void);
	virtual ~CStreamDivider(void);

	virtual M_RESULT Play(const char* strUrl );
	virtual void     Pause(BOOL bResume);
	virtual void     Stop();


	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad) ;
	virtual void OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad) ;

	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode);

  
	//当接收到一个SAMPLE的时候,产生该事件。
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode);

	//是否接受输入PAD的流媒体格式描述符
	//具体格式协商的时候用，一般输入PIN所在的MO实现。
	virtual BOOL Accept(CPad* pInputPad, MxDescriptor* pDescriptor) ;

	CInputPad*  GetInputPad() { return &m_inputPad; };
	COutputPad* GetOutputPad(int index) ;

	//仅支持被动模式
	virtual void SetMode(CMediaObject::WORK_MODE mode){};

	void SetStreamCount(int count);
	int  GetStreamCount(){ return m_nOutputCount;};

	void SetSyncMode(BOOL flag);
	BOOL IsInSyncMode(){ return m_bSyncOutput; };

	//设置拉流的输出pad
	void SetPullPad(int index);
	void RemovePullPad();
	COutputPad* GetPullPad();

	 
public:
 
	void HandleSamples(int index);


private:
	//必须保证输出的两个流同时接收SAMPLE.
	BOOL m_bSyncOutput;
	int  m_nOutputCount;

	void  SetDescriptor( MxDescriptor* pDesc);
	void  RemoveAllSamples(RvList list);

	int DeliverMultiSamples(MX_HANDLE hSample);
	int SendSampleInSyncMode(MX_HANDLE hSample);

	//由输入pad决定
	MxDescriptor* m_pDescriptor;
 

	CInputPad    m_inputPad;
	COutputPad   m_outputPad[MX_SD_OUTPUT_COUNT];

	COutputPad*  m_pPullPad;
	//线程处理
	CRITICAL_SECTION m_cs;
	HANDLE       m_hSampleThread;
	HANDLE       m_lstSamples[MX_SD_OUTPUT_COUNT];
	HANDLE       m_hSampleEvents[MX_SD_OUTPUT_COUNT];
};






#endif
