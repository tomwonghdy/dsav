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
#include "stdafx.h"
#include "ImageConverter.h"

CImageConverter::CImageConverter(void)
{
	m_hImgConvertContext = NULL;
	m_srcFormat =  m_dstFormat =FF_PIXEL_FMT_NONE;
	m_srcWidth = 0;
	m_srcHeight =0;
	m_dstWidth =0 ;
	m_dstHeight =0;

	m_nAlign = MX_IMAGE_ALIGN_4;
	memset( m_pVideoSrcData, 0, sizeof(m_pVideoSrcData))  ;
	memset( m_nVideoSrcLinesizes, 0, sizeof(m_nVideoSrcLinesizes))  ;
    m_nVideoSrcBufsize = 0;

	memset( m_pVideoDstData, 0, sizeof(m_pVideoDstData))  ;
	memset( m_nVideoDstLinesizes, 0, sizeof(m_nVideoDstLinesizes))  ;
    m_nVideoDstBufsize = 0;


	m_sws_flags = SWS_BICUBIC;

	//m_bOpened =FALSE;

}

CImageConverter::~CImageConverter(void)
{
	Destroy();

	RV_ASSERT(m_hImgConvertContext == NULL);

}


BOOL CImageConverter::Create(FF_PIXEL_FORMAT srcFormat, int srcWidth, int srcHeight, 
	                          FF_PIXEL_FORMAT dstFormat, int dstWidth, int dstHeight,
	                          int flags, int align)
{
	if (m_hImgConvertContext ) return FALSE;

	m_srcFormat = srcFormat;
	m_dstFormat = dstFormat;
	m_srcWidth  = srcWidth ;
	m_srcHeight = srcHeight;
	m_dstWidth  = dstWidth; 
	m_dstHeight = dstHeight;

	m_sws_flags = flags;
	m_nAlign = align;

	m_hImgConvertContext = sws_getCachedContext((struct SwsContext*)m_hImgConvertContext,
                                           m_srcWidth, m_srcHeight,(AVPixelFormat)m_srcFormat, m_dstWidth, m_dstHeight,
		(AVPixelFormat)m_dstFormat, m_sws_flags, NULL, NULL, NULL);

	if (NULL == m_hImgConvertContext) return FALSE;

	 
	
	int ret = av_image_alloc(m_pVideoSrcData, m_nVideoSrcLinesizes, srcWidth, srcHeight, (AVPixelFormat)srcFormat, MX_IMAGE_ALIGN_4/*1*/);

	if (ret < 0) {
		if (m_hImgConvertContext){
			sws_freeContext((SwsContext*) m_hImgConvertContext);
			m_hImgConvertContext = NULL;
		}

		/*TRACE("Could not allocate source raw video buffer\n");*/
		return FALSE;
	}
	else{
        m_nVideoSrcBufsize = ret;
	}
 
	
	ret =   av_image_alloc(m_pVideoDstData, m_nVideoDstLinesizes, dstWidth, dstHeight, (AVPixelFormat)dstFormat, m_nAlign /*1*/);

	if (ret < 0) {
		if (m_hImgConvertContext){
			sws_freeContext((SwsContext*) m_hImgConvertContext);
			m_hImgConvertContext = NULL;
		}
		
		av_free(m_pVideoSrcData[0]);
		memset( m_pVideoSrcData, 0, sizeof(m_pVideoSrcData))  ;
		m_nVideoSrcBufsize = 0;

		/*TRACE("Could not allocate dest raw video buffer\n");*/
		return FALSE;
	}
	else{
        m_nVideoDstBufsize = ret;
	}

	//m_bOpened =TRUE;

	return TRUE;


}

void CImageConverter::Destroy()
{
	if (m_hImgConvertContext == NULL) return;

	if (m_hImgConvertContext){
	    sws_freeContext((SwsContext*)m_hImgConvertContext);
		m_hImgConvertContext = NULL;
	}


	av_free(m_pVideoSrcData[0]);
	memset( m_pVideoSrcData, 0, sizeof(m_pVideoSrcData))  ;

	av_free(m_pVideoDstData[0]);
	memset( m_pVideoDstData, 0, sizeof(m_pVideoDstData))  ;

	m_nVideoSrcBufsize = 0;
	m_nVideoDstBufsize =0;

//	m_bOpened=FALSE;

}

BOOL CImageConverter::Excute(uint8_t** pSrcData, int* pSrcLineSizes,   uint8_t** pDestData, int* pDstLineSizes){

 
	if (m_hImgConvertContext == NULL) {
		return FALSE;
	}
	RV_ASSERT(pSrcData && pSrcLineSizes);
	RV_ASSERT(pDestData && pDstLineSizes);

    

    int n = sws_scale((SwsContext*) m_hImgConvertContext, pSrcData, pSrcLineSizes,
                  0,  m_srcHeight,pDestData,pDstLineSizes);

	return (n > 0);
}

BOOL CImageConverter::ExcuteEx(uint8_t* pSrcData, int pSrcDataSize,   uint8_t* pDestData, int pDstDataSize)
{
	// RV_ASSERT(m_bOpened);
	if (m_hImgConvertContext == NULL)  return FALSE;

	 if (pSrcDataSize != m_nVideoSrcBufsize) return FALSE ;
	 if (pDstDataSize != m_nVideoDstBufsize) return FALSE ;

	 RV_ASSERT(m_pVideoSrcData && m_pVideoDstData);
	 RV_ASSERT(pSrcData && pDestData);

	 memcpy( m_pVideoSrcData[0], pSrcData,   pSrcDataSize);
  
	 if (Excute(m_pVideoSrcData, m_nVideoSrcLinesizes, m_pVideoDstData, m_nVideoDstLinesizes)){
         memcpy(pDestData, m_pVideoDstData[0], pDstDataSize);
		 return TRUE;
	 }

	 return FALSE;
}