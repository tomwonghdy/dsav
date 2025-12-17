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
#ifndef __MX_LIVESOURCE_H_INCLUDED_
#define __MX_LIVESOURCE_H_INCLUDED_

#include "ffmo.h"
#include "ffsource.h" 
  
#include "..\davsdk\includes\Source.h"
#include "..\davsdk\includes\OutputPad.h"
 
 

class FFMO_API CLiveSource:	public CFfSource
{
public:
	CLiveSource(void);
	virtual ~CLiveSource(void);

	enum {
		LS_TRANS_AUTO =0,
		LS_TRANS_TCP ,
		LS_TRANS_UDP ,
	};

	virtual M_RESULT Play(const char* strUrl ) ;
	//no pause support
	virtual void Pause(BOOL bResume) { return; };
	virtual void Stop() ;

	//can not seek
	virtual void Seek(int64_t pos, int64_t rel, BOOL bByBytes) { return; }

	//work on active mode, can not be requested by   downstream media objects
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode);

public:
	 
	int  m_nBufferSize;  //// 设置缓存大小
	int  m_nStimeout;    // 设置超时时间 (秒)
	int  m_nMaxDelay;    // 设置最大延时(ms)
	int  m_transportType;  // 设置打开方式 tcp/udp
   
	   
};







#endif
