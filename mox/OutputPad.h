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
#ifndef __MX_OUTPUT_PAD_H_INCLUDED_
#define __MX_OUTPUT_PAD_H_INCLUDED_

#include "pad.h"
#include "inputpad.h"
 
//stamp type
#define OP_VS_PTS   0
#define OP_VS_DTS   1
#define OP_VS_POS   -1
 

class MOX_API COutputPad : public CPad
{
public:
	COutputPad(void);
	virtual  ~COutputPad(void);

	BOOL   Pass(MX_HANDLE hSample, int* pErrCode);

	//媒体对象调用该函数进行当前帧时间或位置进行有效性检测
	//pDifference小于0的话，表示没有过时间点
	//pDifference大于0的话，表示已经过时间点
	//pDifference等于0的话，表示刚好
	//只要pDifference大于0，就返回FALSE, 否者返回TRUE.
	BOOL   ValidateStamp(__int64 stamp, int type = OP_VS_PTS, double* pDifference = NULL) ;

//	virtual  MX_HANDLE GetNextFrame();

	//connect执行过程：
    //1. 首先执行输入的HandleConnection;
    //2. 如果一切OK, 执行所有者（mediaoobject）的OnPadConnected
    //3. 如果一切没有问题，返回M_OK
	virtual M_RESULT Connect(CPad* pInputPad);
	virtual void     Disconnect();

public:
	float m_nRate;


};




#endif

