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
#ifndef __MX_OUTCOME_H_INCLUDED_
#define __MX_OUTCOME_H_INCLUDED_

#include "mediaobject.h"

class MOX_API CSink:	public CMediaObject
{
public:
	CSink(void);
	virtual ~CSink(void);
	 
	//当接收到一个SAMPLE的时候,产生该事件。
	//对于CSink类来说是不可能出现这种事件的，
	//故必须将其屏蔽（实现CMediaObject的重载函数）
	//sink子类不应该再实现该事件
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode){ if (pErrCode) *pErrCode = M_FAILED; return NULL;};

	//代替inputpad输送给transform媒体对象一个样本数据
	virtual BOOL Pass(MX_HANDLE hSample, int* pErrCode){ return FALSE; };

};




















#endif