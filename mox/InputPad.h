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
#ifndef __MX_INPUT_PAD_H_INCLUDED_
#define __MX_INPUT_PAD_H_INCLUDED_

 

#include "pad.h" 
#include "MediaObject.h"

//report traffic 
#define IP_RT_NORMAL   0
#define IP_RT_JAMMED   1
#define IP_RT_STARVED -1


class MOX_API CInputPad : public CPad
{
public:
	CInputPad(void);
	virtual ~CInputPad(void);

	//HandleConnection执行过程：
    //1. 首先执行所有者（MEDIA OBJECT）的CheckMediaType;
	//   CheckMediaType将调用对应的输出PAD的所有者的GetMediaType;
    //2. 如果一切OK, 执行所有者（mediaoobject）的OnPadConnected
    //3. 如果一切没有问题，返回M_OK
	M_RESULT HandleConnection(CPad* pOutputPad, BOOL bBreak=FALSE);

	//如果PAD里面里面有SAMPLE，则直接传输第一个SAMPLE。否则，
	//从OWNER里面直接获取（调用虚拟函数OnSampleRequest）。
	MX_HANDLE Fetch(int* pErrCode) ;
 
	//下游媒体对象向上游媒体对象报告
	//数据流处理情况
	//state: IP_RT_NORMAL, IP_RT_JAMMED, IP_RT_STARVED
	//masterClock: 主时钟当前时间
	//lastPts: 最近帧的显示时间戳
	void ReportTraffic(int state, double masterClock, __int64 lastPts);

private:


};



#endif