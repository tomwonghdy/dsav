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
#ifndef __MX_LIVE_SINK_H_INCLUDED_
#define __MX_LIVE_SINK_H_INCLUDED_


#include "ffmo.h"  
#include "FfSink.h"   
  
class FFMO_API CLiveSink: public CFfSink
{
public:
	enum SUBTYPE
	{
		SST_UNK  = 0,
		SST_RTSP =1,
		SST_RTMP ,
	};

	CLiveSink(void);
	virtual ~CLiveSink(void);
	 
	virtual M_RESULT Play(const char* strUrl );
	virtual void Pause(BOOL bResume);
	virtual void Stop();
	   

public:
	void HandleMux();
protected: 
	int  GetLiveType(const char* strUrl);
	virtual BOOL SetVideoCodecOptions(int codecId)  ;
	virtual BOOL SetAudioCodecOptions(int codecId)  ;

	
};







#endif
