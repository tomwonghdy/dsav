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
#ifndef __MX_TRANSFORM_H_INCLUDED_
#define __MX_TRANSFORM_H_INCLUDED_

#include "mediaobject.h"


class MOX_API CTransform :	public CMediaObject
{
public:
	CTransform(void);
	virtual ~CTransform(void);

	////当接收到一个SAMPLE的时候,产生该事件。
	//virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode){if (pErrCode) *pErrCode = M_FAILED; return FALSE; };
	//////当接收方通过PAD接收SAMPLE时,产生该事件。
	//virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, MX_HANDLE hSample, int* pErrCode){ if (pErrCode) *pErrCode = M_FAILED; return NULL;};

	//代替inputpad输送给transform媒体对象一个样本数据
	virtual BOOL Pass(MX_HANDLE hSample, int* pErrCode){ return FALSE; };

	//处理output pad样本数据
	void   SetSamplePassHandler(MO_SAMPLE_PASS_FUNC pfnSamplePass );
	MO_SAMPLE_PASS_FUNC GetSamplePassHandler();

protected:

	MO_SAMPLE_PASS_FUNC  m_fnSamplePass;
protected:

    MxDescriptor* DuplicateDescriptor(MxDescriptor* pSrc, MxDescriptor* pDst=NULL);


};






#endif