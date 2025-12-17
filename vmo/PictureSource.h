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
#pragma once

#include "vmo.h"

#include "..\davsdk\includes\source.h"

class VMO_API CPictureSource :	public CSource
{
public:
	CPictureSource(void);
	virtual ~CPictureSource(void);
	 
	virtual M_RESULT Play(const char* strUrl) ;
	virtual void     Pause(BOOL bResume) ;
	virtual void     Stop();
  
	virtual COutputPad* GetVideoPad(){return &m_outputPad;}

	virtual MX_MEDIA_TYPE GetMediaType(CPad* pPad) ;
	virtual BOOL CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) ;

	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad);
	 
	virtual MxDescriptor*      GetFormatDescriptor(CPad* pPad);	
	virtual void               SetMode(CMediaObject::WORK_MODE mode); 
	  
	////当接收方通过PAD接收SAMPLE时,产生该事件。
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode) ;
  
	//视频输出格式,只能是BGR格式
	//如果output pad已经连接，将重新协商格式
	//当格式协商不成功，就自动断开连接
	BOOL  SetOutputFormat(int format, int width, int height);

	BOOL  SetImage(const RvImage image);
	BOOL  SetImage(const char* strFilePath);

	int   GetImageCount();

public:
	void RemoveAll();
	void HandleSendImage();
protected:

	void SendImage();

	COutputPad  m_outputPad;
	MxVideoDesc m_videoOutDesc;

	//BOOL    m_bHasData;
	//RvImage m_image;
	RvSequence m_seq;
	int      m_nCunIndex;
	int      m_nPeriod; //播放周期
	__int64  m_nLastStamp;
	HANDLE   m_hSendThread;

	CRITICAL_SECTION m_csSample;
};

