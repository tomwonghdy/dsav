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
#ifndef __FFMPEG_IMAGE_CONVERTER_H_INCLUED_
#define __FFMPEG_IMAGE_CONVERTER_H_INCLUED_

#include "ffmo.h"

//#include "avlib.h"


class FFMO_API CImageConverter
{
public:
	//定义与ffmpeg一致
	enum {
		 PST_FAST_BILINEAR = 1,
		 PST_BILINEAR = 2,
		 PST_BICUBIC = 4,
		 PST_X = 8,
		 PST_PST_POINT = 0x10,
		 PST_AREA = 0x20,
		 PST_BICUBLIN = 0x40,
		 PST_GAUSS = 0x80,
		 PST_SINC = 0x100,
		 PST_LANCZOS = 0x200,
		 PST_SPLINE = 0x400,
	};

public:
	CImageConverter(void);
	~CImageConverter(void);

	BOOL Create(FF_PIXEL_FORMAT srcFormat, int srcWidth, int srcHeight,
		FF_PIXEL_FORMAT dstFormat, int dstWidth, int dstHeight,
		int flags = PST_BILINEAR, int align = MX_IMAGE_ALIGN_4);

	void Destroy();

	BOOL IsValid() { return (m_hImgConvertContext != NULL); };

	BOOL Excute(uint8_t** pSrcData, int* pSrcLineSizes, uint8_t** pDestData, int* pDstLineSizes);
	BOOL ExcuteEx(uint8_t* pSrcData, int pSrcDataSize, uint8_t* pDestData, int pDstDataSize);

	//converter关闭以后，SrcBufferSize和 DestBufferSiz将为0
	UINT GetSrcBufferSize() { return m_nVideoSrcBufsize; };
	UINT GetDestBufferSize() { return m_nVideoDstBufsize; };

	//下面的函数在converter关闭以后，不变（即可以重用）。
	FF_PIXEL_FORMAT GetSrcFormat() { return m_srcFormat; };
	FF_PIXEL_FORMAT GetDestFormat() { return m_dstFormat; };

	int GetSrcWidth() { return m_srcWidth; };
	int GetDestWidth() { return m_dstWidth; };

	int GetSrcHeight() { return m_srcHeight; };
	int GetDestHeight() { return m_dstHeight; };


private:
	void* m_hImgConvertContext;

	FF_PIXEL_FORMAT m_srcFormat, m_dstFormat;
	int m_srcWidth, m_srcHeight;
	int m_dstWidth, m_dstHeight;

	uint8_t* m_pVideoSrcData[4];
	int       m_nVideoSrcLinesizes[4];
	UINT       m_nVideoSrcBufsize;

	uint8_t* m_pVideoDstData[4];
	int       m_nVideoDstLinesizes[4];
	UINT       m_nVideoDstBufsize;

	//BOOL m_bOpened;
	int  m_nAlign;
	int m_sws_flags;

};



#endif