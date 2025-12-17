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
#ifndef __MX_PAD_H_INCLUDED_
#define __MX_PAD_H_INCLUDED_


#include "mox.h"
//#include "mediasample.h"
#include "mediaobject.h"
#include <rvb\dsm.h>


class CMediaObject;

class MOX_API CPad
{
public:
	CPad(void);
	virtual ~CPad(void);
	 
	enum CAPTURE_TYPE{
		CT_PULL = 1,
		CT_PUSH = 2,
		CT_ANY  = 4,
	};

	enum STREAM_TYPE
	{
		ST_UNKNOWN = -1,
		ST_OUTPUT  = 0,
		ST_INPUT   = 1,
	};

	void SetCaptureType(CAPTURE_TYPE captureType){	m_nCaptureType = captureType;	}
	CAPTURE_TYPE   GetCaptureType() { return m_nCaptureType; };

	//for inner use only
	MX_HANDLE OnTransferSample(MX_HANDLE hSample, CAPTURE_TYPE type, int* pErrCode);
	
	
	//int    GetSampleCount();

	BOOL IsConnected(){ return m_bConnected; };
	  
	void  SetMediaType(MX_MEDIA_TYPE type);
	MX_MEDIA_TYPE  GetMediaType()  ;
	
	
	//获取OWNER的格式描述符号
	MxDescriptor* GetFormatDescriptor();

	//与对方PAD协商媒体数据格式
	BOOL  NegotiateDescriptor(MxDescriptor* pFormatDescriptor);

	//向对方PAD咨询与之相连的MEDIA OBJECT支持的媒体数据格式。
	MxDescriptor* EnquireDescriptor();

	//查询与媒体流有关的属性.
	BOOL  EnquireProperty(const char* strName, char* strValue, int nValueSize);
	 
	//查询与媒体流有关的变量.
	BOOL  EnquireVariable(int nName, void* pValue);

	STREAM_TYPE GetStreamType() {
		return m_nStreamType;
	}

	__int64 GetFrameCount();

public:
	CMediaObject* m_pOwner;
	

protected:
	BOOL         m_bConnected;
	CPad*        m_pCounterPart;

	//MX_MEDIA_TYPE   m_nMediaType;
	CAPTURE_TYPE  m_nCaptureType;
	STREAM_TYPE m_nStreamType ;

	 CRITICAL_SECTION m_csLock;

	
	 MxDescriptor* m_pMediaDesc;
	
	 __int64 m_nFrameCount; 

};








#endif
