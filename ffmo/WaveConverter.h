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
#ifndef __FFMPEG_WAVE_CONVERTER_H_INCLUED_
#define __FFMPEG_WAVE_CONVERTER_H_INCLUED_

 
#include "ffmo.h"
//#include "avlib.h"



#define  WC_DEPTH_U8         FF_WAVE_FMT_U8
#define  WC_DEPTH_S16        FF_WAVE_FMT_S16 

#define  WC_CHANNEL_MONO     0x00000004 //AV_CH_LAYOUT_MONO
#define  WC_CHANNEL_STEREO   (0x00000001 | 0x00000002) // AV_CH_LAYOUT_STEREO
 

class FFMO_API CWaveConverter
{
public:

	CWaveConverter(void);
	~CWaveConverter(void);

	//buffer size alignment (0 = default, 1 = no alignment)
	BOOL Create(int nSrcLayout, int nSrcRate, FF_WAVE_FORMAT nSrcFormat,
	        	int nDstLayout, int nDestRate, FF_WAVE_FORMAT nDestFormat,
		        int align = 1);

	void Destroy();

	BOOL Excute(uint8_t** pSrcData, int nSrcFrameCount, uint8_t** pDestData, int nDestFrameCount);
	BOOL ExcuteEx(uint8_t* pSrcData, int nSrcBufsize,  int nSrcFrameCount, uint8_t* pDstData, int nDstBufsize,  int nDstFrameCount);

	int64_t   GetLastDelay();

	
	int64_t   GetDestSampleCount(int nSrcSampleCount, int* pBufferSize =NULL  );

	//BOOL  AllocSamples( int channels, int depth, int sampleCount, );
	BOOL  IsValid(){return (m_hSwrCxt != NULL);};

private:

	BOOL  AllocSampleData(int sampleCount, BOOL bSrc);
	void  FreeSampleData(BOOL bSrc);

	 void* m_hSwrCxt;

	 //BOOL m_bOpened ;
	 int   m_nAlign;

	 __int64   m_nSerialCounter;

	 uint8_t ** m_ppSrcData  ;
	 int        m_nSrcSampleCount;
	 int        m_nSrcBufsize;

	 uint8_t ** m_ppDstData  ;
	 int        m_nDstSampleCount;  //“Ù∆µ÷° ˝¡ø
	 int        m_nDstBufsize;
	  

	 int           m_nSrcChnLayout;
	 int                 m_nSrcRate;
	 FF_WAVE_FORMAT      m_nSrcFormat;

	 int            m_nDstChnLayout;
	 int                 m_nDestRate;
	 FF_WAVE_FORMAT      m_nDestFormat;
	   
};






#endif
