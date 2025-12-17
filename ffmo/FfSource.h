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
#ifndef __MX_FFSOURCE_H_INCLUDED_
#define __MX_FFSOURCE_H_INCLUDED_

#include "ffmo.h"


#include "..\davsdk\includes\Source.h"
#include "..\davsdk\includes\OutputPad.h"
  
  

class FFMO_API CFfSource:	public CSource
{
public:
	CFfSource(void);
	virtual ~CFfSource(void);

	// enum PICTURE_SIZE
	//{
	//	VS_DEFAULT =0,  //原始格式;
	//	VS_ADAPT,       //自适应
	//	VS_HALF_TIMES,  //原始尺寸的一半
	//	VS_TWO_TIMES,   //原始尺寸的2倍   
	//	VS_RATIO_4_3,
	//	VS_RATIO_16_9,
	//	VS_CUSTOM,       //自定义大小格式
	//};
	//
	//enum WAVE_SAMPLE
	//{
	//	AO_DEFAULT =0, //原始格式
	//	AO_ADAPT,      //自适应
	//	AO_CUSTOM ,    //自定义
	//};

	virtual M_RESULT Play(const char* strUrl ) ;
	virtual void Pause(BOOL bResume) ;
	virtual void Stop() ;

	//UNIT: minisecond
	virtual void Seek(int64_t  position);


	virtual COutputPad* GetVideoPad( ){return &m_videoPad;}
	virtual COutputPad* GetAudioPad( ){return &m_audioPad;}
	virtual COutputPad* GetSubtitlePad( ){ return NULL; /*return &m_subtitlePad;*/}

	virtual void       SetMode(CMediaObject::WORK_MODE mode);


	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode);

	HANDLE GetContext() { return  m_hFfContext; };

	double GetFrameRate();

 public: 

	int          m_nOutputType;           //流数据输出类型，指音频，视频，字幕等
	BOOL         m_bRawVideoData;      //保留原视频数据格式
	BOOL         m_bRawAudioData;      //保留原音频数据格式


	int          m_nAudioFrameThres;
	int          m_nVideoFrameThres;


	 
protected:
	//PICTURE_SIZE    m_nVideoSize;   //图像帧输出分辨率
	//WAVE_SAMPLE  m_nAudioOutput; //音频数据输出格式
	//AVFormatContext * GetContext() { return  m_pFmtCxt ; };
	//CRITICAL_SECTION  m_csReadPackage;
	BOOL OpenSource(HANDLE hFxCxt, const char* strFileName);
	void CloseSource(void);

protected:

	COutputPad  m_audioPad;
	COutputPad  m_videoPad;
	COutputPad  m_subtitlePad;
  
	HANDLE      m_hFfContext;
    
	CRITICAL_SECTION  m_csLock;
	HANDLE            m_hReadThread; 
};







#endif