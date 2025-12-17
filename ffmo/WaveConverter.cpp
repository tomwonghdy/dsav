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
#include "WaveConverter.h"

#include "AvLib.h"


CWaveConverter::CWaveConverter(void)
{

      m_hSwrCxt = NULL;

	  m_nSrcChnLayout =0;
	  m_nSrcRate = 0;
	  m_nSrcFormat=  FF_WAVE_FMT_NONE;

	  m_nDstChnLayout =0;
	  m_nDestRate =0;
	  m_nDestFormat =FF_WAVE_FMT_NONE;

	 // m_bOpened =FALSE;
	  m_nAlign = 1;

	  m_ppSrcData  =NULL ;  m_nSrcSampleCount = 0;   m_nSrcBufsize = 0;
	  m_ppDstData = NULL ;  m_nDstSampleCount = 0;  m_nDstBufsize = 0;



	 // m_bFirstFrame=FALSE;


}

CWaveConverter::~CWaveConverter(void)
{
	Destroy();
}

BOOL CWaveConverter::Create(int nSrcLayout, int nSrcRate, FF_WAVE_FORMAT nSrcFormat,  
	                        int nDstLayout, int nDestRate, FF_WAVE_FORMAT nDestFormat, 
	                        int align)
{ 

	if (m_hSwrCxt) return FALSE;

	/* create resampler context */
    m_hSwrCxt = swr_alloc();
    if (!m_hSwrCxt) {
        //TRACE( "Could not allocate resampler context\n");
		return FALSE;
    }
	AVChannelLayout srccl = GetFfmpegLayout(nSrcLayout);

 

	int  nb_planes = av_sample_fmt_is_planar((AVSampleFormat)nSrcFormat) ? srccl.nb_channels/*av_get_channel_layout_nb_channels(nSrcChnLayout)*/ : 1;
	m_ppSrcData  = (uint8_t**) av_mallocz(sizeof(uint8_t *) * nb_planes);

	if (NULL == m_ppSrcData){
		swr_free((SwrContext**)(&m_hSwrCxt));
		m_hSwrCxt = NULL;

		return FALSE;
	}
	AVChannelLayout dstcl = GetFfmpegLayout(nDstLayout);
	nb_planes = av_sample_fmt_is_planar((AVSampleFormat)nDestFormat) ? dstcl.nb_channels/*av_get_channel_layout_nb_channels(nDstChnLayout)*/ : 1;
	m_ppDstData  = (uint8_t**) av_mallocz(sizeof(uint8_t *) * nb_planes);
	
	if (NULL == m_ppDstData){
		swr_free((SwrContext**)&m_hSwrCxt);
		m_hSwrCxt = NULL;

		av_freep(&m_ppSrcData[0]);
		m_ppSrcData =NULL;
		return FALSE;
	}

	

    /* set options */
	
	  
	av_opt_set_chlayout(m_hSwrCxt, "in_chlayout", &srccl, 0);
    av_opt_set_int(m_hSwrCxt, "in_sample_rate",       nSrcRate, 0);
    av_opt_set_sample_fmt(m_hSwrCxt, "in_sample_fmt", (AVSampleFormat)nSrcFormat, 0);

	av_opt_set_chlayout(m_hSwrCxt, "out_chlayout", &dstcl, 0);
    av_opt_set_int(m_hSwrCxt, "out_sample_rate",       nDestRate, 0);
    av_opt_set_sample_fmt(m_hSwrCxt, "out_sample_fmt", (AVSampleFormat)nDestFormat, 0);

	 
	 m_nSrcChnLayout = nSrcLayout;
	 m_nSrcRate  = nSrcRate;
	 m_nSrcFormat =  nSrcFormat;

	 m_nDstChnLayout = nDstLayout;
	 m_nDestRate  = nDestRate;
	 m_nDestFormat= nDestFormat;	

	int ret;
    /* initialize the resampling context */
    if ((ret = swr_init((SwrContext*)m_hSwrCxt)) < 0) {
        //TRACE( "Failed to initialize the resampling context\n");
         
		swr_free((SwrContext**)&m_hSwrCxt);
		m_hSwrCxt = NULL;

		return FALSE;
    }

	//source samples alloc
 
	if (!AllocSampleData(AV_AUDIO_BUFFER_SIZE, TRUE)) {
		//TRACE( "Could not allocate audio buffer\n");	 
		swr_free((SwrContext**)&m_hSwrCxt);
		m_hSwrCxt = NULL;

		return FALSE;
	}

	if (!AllocSampleData(AV_AUDIO_BUFFER_SIZE, FALSE)) {
		//TRACE( "Could not allocate dest audio buffer\n");	 
		swr_free((SwrContext**)&m_hSwrCxt);
		m_hSwrCxt = NULL;

		FreeSampleData(TRUE);

		return FALSE;
	}

	m_nSerialCounter = 0; 
	m_nAlign = align;
	//m_bOpened =TRUE;

	return TRUE;
	  
}

void CWaveConverter::Destroy(){

	if (NULL == m_hSwrCxt) return;

	if (m_hSwrCxt){
		SwrContext* p =(SwrContext*) m_hSwrCxt;
		swr_free((SwrContext**)&p);
		m_hSwrCxt =NULL;

	}

	FreeSampleData(TRUE);
	FreeSampleData(FALSE);

	if (m_ppSrcData){
	    av_freep(&m_ppSrcData[0]);
		m_ppSrcData = NULL;
	}

	if (m_ppDstData){
		av_freep(&m_ppDstData[0]);
		m_ppDstData = NULL;
	}

	//m_bOpened = FALSE;

}

BOOL CWaveConverter::Excute(uint8_t** pSrcData, int nSrcCount, uint8_t** pDestData, int nDestCount)
{
//	RV_ASSERT(m_bOpened);
	RV_ASSERT(m_hSwrCxt);  

	if (NULL == m_hSwrCxt) return FALSE;

	/* convert to destination format */
	int ret = swr_convert((SwrContext*)m_hSwrCxt, (uint8_t **)pDestData, nDestCount, (const uint8_t **)pSrcData, nSrcCount);

	if (ret < 0) {
		//TRACE("Error while converting\n");
		return FALSE;
	}

	m_nSerialCounter++;


	return TRUE;
}

BOOL CWaveConverter::ExcuteEx(uint8_t* pSrcData, int nSrcBufsize,  int nSrcFrameCount, uint8_t* pDstData, int nDstBufsize,  int nDstFrameCount)
{
	if (NULL == m_hSwrCxt) return FALSE;
	//RV_ASSERT(m_bOpened);
	RV_ASSERT(pSrcData && pDstData);

	if (!AllocSampleData(nSrcFrameCount, TRUE)) {		 
		return FALSE;
	}
	RV_ASSERT(nSrcBufsize == m_nSrcBufsize);

	if (!AllocSampleData(nDstFrameCount, FALSE)) {		 
		return FALSE;
	}

	RV_ASSERT(nDstBufsize == m_nDstBufsize);

	memcpy(m_ppSrcData[0], pSrcData,  nSrcBufsize);
	 
	BOOL ret  = Excute(this->m_ppSrcData, nSrcFrameCount, this->m_ppDstData, nDstFrameCount);

	if (ret){
		memcpy(pDstData, m_ppDstData[0], nDstBufsize);
	}


	return ret;
}


BOOL  CWaveConverter::AllocSampleData(int sampleCount, BOOL bSrc)
{
	int lineSize;
	int channel;
	int  ret;

	if (bSrc) {
		if (m_nSrcSampleCount == sampleCount) return TRUE;
		 
		FreeSampleData(bSrc);
		m_nSrcSampleCount = sampleCount;

		AVChannelLayout scrcl = GetFfmpegLayout(m_nSrcChnLayout);
		channel = scrcl.nb_channels;// av_get_channel_layout_nb_channels(m_nSrcChnLayout);

		ret = av_samples_alloc(m_ppSrcData, &lineSize, channel, m_nSrcSampleCount, (AVSampleFormat)m_nSrcFormat, m_nAlign);

		if (ret < 0) {
			m_ppSrcData = NULL;
			return FALSE;
		}

		m_nSrcBufsize = av_samples_get_buffer_size(NULL, channel, m_nSrcSampleCount, (AVSampleFormat)m_nSrcFormat, m_nAlign);

		return TRUE;
	}
	else {
		if (m_nDstSampleCount == sampleCount) return TRUE;

		FreeSampleData(bSrc);
		m_nDstSampleCount = sampleCount;

		AVChannelLayout dstcl = GetFfmpegLayout(m_nDstChnLayout);
		channel = dstcl.nb_channels;// av_get_channel_layout_nb_channels(m_nDstChnLayout);

		ret = av_samples_alloc(m_ppDstData, &lineSize, channel, m_nDstSampleCount, (AVSampleFormat)m_nDestFormat, m_nAlign);

		if (ret < 0) {
			m_ppDstData = NULL;
			return FALSE;
		}
		m_nDstBufsize = av_samples_get_buffer_size(NULL, channel, m_nDstSampleCount, (AVSampleFormat)m_nDestFormat, m_nAlign);

		return TRUE;
	}


	return FALSE;



}

int64_t CWaveConverter::GetLastDelay()
{
	if (m_hSwrCxt ==NULL) return 0;

	return (int)swr_get_delay((SwrContext*)m_hSwrCxt, m_nSrcRate);
}

int64_t  CWaveConverter::GetDestSampleCount(int nSrcCount ,   int* pBufferSize)
{ 
	if (m_hSwrCxt == NULL) {
		if (pBufferSize) *pBufferSize = 0;
		return 0;
	}

	int64_t n = 0;

	if (m_nSerialCounter == 0)
		n =   av_rescale_rnd(nSrcCount, m_nDestRate, m_nSrcRate, AV_ROUND_UP);
	else 
		n =   av_rescale_rnd(swr_get_delay((SwrContext*)m_hSwrCxt, m_nSrcRate) + nSrcCount, m_nDestRate, m_nSrcRate, AV_ROUND_UP);

	if (pBufferSize){
		AVChannelLayout dstcl = GetFfmpegLayout(m_nDstChnLayout);

		*pBufferSize = av_samples_get_buffer_size(NULL, dstcl.nb_channels/*av_get_channel_layout_nb_channels(m_nDstChnLayout)*/, n, (AVSampleFormat)m_nDestFormat, m_nAlign/*1*/);
	}

	return n;
}

void  CWaveConverter::FreeSampleData(BOOL bSrc)
{

	if (bSrc ){		 
		av_freep(&m_ppSrcData[0]);    
		m_nSrcSampleCount = 0;
	}
	else{				   
		av_freep(&m_ppDstData[0]);	
		m_nDstSampleCount = 0;
	}

}
 