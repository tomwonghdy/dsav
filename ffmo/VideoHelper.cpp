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
#include "VideoHelper.h"


#define WORKAROUND_BUGS   1
#define LOWRES            0
#define IDCT              FF_IDCT_AUTO
#define ERROR_CONCEALMENT  3
//#define FAST               0


CVideoHelper::CVideoHelper(void)
{
	m_hSource = NULL;
	m_pFmtCxt = NULL;

	m_pFmtOpts = NULL;
	m_pCodecOpts = NULL;


	m_bRequestAbort =FALSE;

	m_pStream = NULL;

	m_nStreamIndex =-1; 

}


CVideoHelper::~CVideoHelper(void)
{
	  
}



 static int video_helper_decode_interrupt_cb(void *ctx)
{
    CVideoHelper *pFs = (CVideoHelper *)ctx;
	return pFs->m_bRequestAbort ;
}



// 回调函数
static int interrupt_callback(void *p) {
	TimeoutMonitor *r = (TimeoutMonitor *)p;
	if (r->lasttime > 0) {
		if (time(NULL) - r->lasttime > r->timeout) {
			// 等待超时则中断
			return   1;
		}
	}
    
	return 0;
}

 

BOOL CVideoHelper::Open(const char* strFilePath, int* pErrCode)
{
	int err, /*i,*/ ret;
  //  int st_index[AVMEDIA_TYPE_NB];
  //  AVPacket pkt1, *pkt = &pkt1;
 //   int eof = 0;
	if (pErrCode) *pErrCode = M_FAILED;
    

    AVDictionaryEntry *t;
    AVDictionary **opts;

    unsigned int orig_nb_streams;
   
   // memset(st_index, -1, sizeof(st_index)); 
   
	m_bRequestAbort =FALSE;

	 m_pFmtCxt = avformat_alloc_context();
	 RV_ASSERT(m_pFmtCxt);

	 if ( NULL == m_pFmtCxt)
	 {
		 if (pErrCode) *pErrCode = M_MEM_ALLOC;
		 return FALSE;
	 }


	((AVFormatContext*) m_pFmtCxt)->interrupt_callback.callback = video_helper_decode_interrupt_cb;
	((AVFormatContext*)m_pFmtCxt)->interrupt_callback.opaque = this;


	 TimeoutMonitor input_runner = {0, 5};
	  
	 ((AVFormatContext*)m_pFmtCxt)->interrupt_callback.callback = interrupt_callback;
	 ((AVFormatContext*)m_pFmtCxt)->interrupt_callback.opaque = &input_runner;
	 input_runner.lasttime = time(NULL);
	 input_runner.timeout = 5; 
	 
	 AVFormatContext* pCxt =(AVFormatContext*) m_pFmtCxt;
	 AVDictionary* pOpts =(AVDictionary*) m_pFmtOpts;
    err = avformat_open_input(&pCxt, strFilePath, NULL, &pOpts);
	
    if (err < 0)
	{      
        if (pErrCode) *pErrCode = err;
        goto FAIL;
    }

    if ((t = av_dict_get((AVDictionary*)m_pFmtOpts, "", NULL, AV_DICT_IGNORE_SUFFIX)))
	{
        goto FAIL;
    }
    

    //if (m_bGenPts)   m_pFmtCxt->flags |= AVFMT_FLAG_GENPTS;

	RV_ASSERT(0);
	opts = NULL;// setup_find_stream_info_opts((AVFormatContext*)m_pFmtCxt, (AVDictionary*)m_pCodecOpts);
    orig_nb_streams = ((AVFormatContext*)m_pFmtCxt)->nb_streams;

    err = avformat_find_stream_info((AVFormatContext*)m_pFmtCxt, opts);
    if (err < 0) {
		if (pErrCode) *pErrCode = err;

		goto FAIL;
    }

    for (unsigned int i = 0; i < orig_nb_streams; i++)
        av_dict_free(&opts[i]);

    av_freep(&opts);

    if (((AVFormatContext*)m_pFmtCxt)->pb)
		((AVFormatContext*)m_pFmtCxt)->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use url_feof() to test for the end
 

    for (unsigned int i = 0; i < ((AVFormatContext*)m_pFmtCxt)->nb_streams; i++)
        ((AVFormatContext*)m_pFmtCxt)->streams[i]->discard = AVDISCARD_ALL;

	 
    m_nStreamIndex = av_find_best_stream(((AVFormatContext*)m_pFmtCxt), AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
 
    if (m_nStreamIndex >= 0) {
        ret = OpenStreamComponent ( m_nStreamIndex );

		if (ret < 0) goto FAIL;
		
    } 
	else{
		goto FAIL;
	}

    
	return TRUE;	


FAIL:

	if (m_pFmtCxt){
		AVFormatContext* p = (AVFormatContext*)m_pFmtCxt;
		avformat_close_input(&p);
		m_pFmtCxt =NULL;
	}


	

	return FALSE;
}



/* open a given stream. Return 0 if OK */
int CVideoHelper::OpenStreamComponent( int stream_index)
{
	//AVFormatContext *ic = (AVFormatContext*) m_pFmtCxt;
	//RV_ASSERT(ic);

 //   AVCodecContext *avctx;
 //   AVCodec *codec;
 //   const char *forced_codec_name = NULL;
 //   AVDictionary *opts;
 //   AVDictionaryEntry *t = NULL;
	//int ret =0;

 //   if (stream_index < 0 || stream_index >= (int)ic->nb_streams)
 //       return -1;

	//RV_ASSERT(0);
 //  // avctx = ic->streams[stream_index]->codec;

 // //  codec = avcodec_find_decoder(avctx->codec_id);
	//  

 //   if (!codec) {      
 //       return -1;
 //   }

 //   avctx->codec_id = codec->id;
 //   avctx->workaround_bugs   = WORKAROUND_BUGS;
 //   avctx->lowres            = LOWRES;
 //   
	//if(avctx->lowres > codec->max_lowres){
 //       av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
 //               codec->max_lowres);
 //       avctx->lowres = codec->max_lowres;
 //   }

 //   avctx->idct_algo         = IDCT;
 //   avctx->skip_frame        = AVDISCARD_DEFAULT;
 //   avctx->skip_idct         = AVDISCARD_DEFAULT;
 //   avctx->skip_loop_filter  = AVDISCARD_DEFAULT;
 //   avctx->error_concealment = ERROR_CONCEALMENT;

	//RV_ASSERT(0);
 // //  if(avctx->lowres) avctx->flags |= CODEC_FLAG_EMU_EDGE;

 //   
 // //  if(codec->capabilities & CODEC_CAP_DR1)
 // //      avctx->flags |= CODEC_FLAG_EMU_EDGE;


 //   opts = filter_codec_opts((AVDictionary*)m_pCodecOpts, avctx->codec_id, ic, ic->streams[stream_index], codec);
 //   
	//if (!av_dict_get(opts, "threads", NULL, 0))
 //       av_dict_set(&opts, "threads", "auto", 0);

 //   if (avcodec_open2(avctx, codec, &opts) < 0)
 //       return -1;

 //   if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
 //       av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
 //       return AVERROR_OPTION_NOT_FOUND;
 //   }
 //
 //   ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;

 //   RV_ASSERT(avctx->codec_type == AVMEDIA_TYPE_VIDEO );
	//
	//m_pStream = ic->streams[stream_index]; 
	//
 //   return 0;
	return -1;
}

void CVideoHelper::Close()
{
	 m_bRequestAbort= TRUE;

	 if (m_nStreamIndex  >= 0)
	 {
		 CloseStreamComponent(m_nStreamIndex);

		 m_nStreamIndex =-1;
	 }
	 
	 if (m_pFmtCxt) {
		 AVFormatContext* p = (AVFormatContext*)m_pFmtCxt;
		 avformat_close_input(&p);
		 m_pFmtCxt= NULL;
	 }

}



void  CVideoHelper::CloseStreamComponent( int stream_index)
{
	RV_ASSERT(m_pFmtCxt);

	MX_ASSERT(0);

	/*AVFormatContext *ic = this->m_pFmtCxt;
    AVCodecContext *avctx;

    if (stream_index < 0 || stream_index >= (int)ic->nb_streams)   return;

    avctx = ic->streams[stream_index]->codec;

    RV_ASSERT  (avctx->codec_type == AVMEDIA_TYPE_VIDEO) ;
    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    avcodec_close(avctx);

#if CONFIG_AVFILTER
    free_buffer_pool(&is->buffer_pool);
#endif*/

}


M_RESULT  CVideoHelper::RetrievePicture(__int64 position,  MxVideoDesc* pDesc, void* pDataBuffer, UINT nDataSize)
{
	if (M_OK == Seek(position)){
		return Read(pDesc, pDataBuffer, nDataSize, TRUE);
	}
	return M_FAILED;
}

M_RESULT    CVideoHelper::Seek(__int64 position)
{
	if (m_nStreamIndex < 0) return M_CLOSED;

	return SeekMediaPos((AVFormatContext*)m_pFmtCxt, m_nStreamIndex, ((AVStream*)m_pStream)->time_base, position/1000.);
	
	//RV_ASSERT(m_pFmtCxt);
	//RV_ASSERT(m_pStream);

	//double n = (double)position;
	//position = __int64 (n / 1000.0 * AV_TIME_BASE);	
	// 
	//AVRational time_base_q = {1, AV_TIME_BASE};

	//__int64 seekTime = av_rescale_q(position, time_base_q, m_pStream->time_base);

	//__int64 seekStreamDuration =m_pFmtCxt->duration;

	//int flags =AVSEEK_FLAG_BACKWARD;

	//if (seekTime> 0 && seekTime < seekStreamDuration)
	//	flags |=AVSEEK_FLAG_ANY; // H.264 I frames don't always register as "keyframes" in FFmpeg

	//int ret =av_seek_frame(m_pFmtCxt, m_nStreamIndex, seekTime, flags);
 // 
	//if (ret>=0) 
	//{
	//	return M_OK;
	//}

	//return M_FAILED;
}

M_RESULT CVideoHelper::Read(MxVideoDesc* pDesc, void* pDataBuffer, UINT nDataSize, BOOL bKeyFrame)
{
	if (m_nStreamIndex < 0) return M_CLOSED;

	MX_ASSERT(0);
////	RV_ASSERT(pDesc && pDataBuffer);
////	RV_ASSERT(m_pStream   );
////	RV_ASSERT(m_pStream  && m_pFmtCxt );
////
////	SwsContext *pSWSCtx;  
////	AVCodecContext *pCodecCtx = m_pStream->codec;  
////	AVFrame *pFrame;  
////	AVFrame *pFrameDest; 
////
////	int     frameFinished;  
//// 
////	static AVPacket packet;   
////
////    pFrame=avcodec_alloc_frame();   
////	pFrameDest = avcodec_alloc_frame();
////
////	RV_ASSERT(pFrame && pFrameDest);
////
////
////	AVPixelFormat dest_fmt = GetVideoPixelFormat(pDesc->format);
////
////	MX_ASSERT(0);
//////	RV_ASSERT(  nDataSize == avpicture_get_size(dest_fmt, pDesc->width,pDesc->height));  
////	
//////	avpicture_fill((AVPicture *)pFrameDest,(const uint8_t *) pDataBuffer, dest_fmt, pDesc->width, pDesc->height);  
////	
////	pSWSCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pDesc->width, pDesc->height, dest_fmt, SWS_BICUBIC, NULL, NULL, NULL);  
////
////	RV_ASSERT(pSWSCtx);
////
////	BOOL bRead =FALSE;
////	while(av_read_frame(m_pFmtCxt,&packet)>=0)  
////	{  
////		if(packet.stream_index == m_nStreamIndex)  
////		{  
////			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);  
////			if(frameFinished)  
////			{  
////				if (bKeyFrame){
////					if(pFrame->key_frame==1)//这里取到关键帧数据  
////					{  
////						sws_scale(pSWSCtx, pFrame->data, pFrame->linesize,0,  pCodecCtx->height, pFrameDest->data, pFrameDest->linesize);  
////						bRead =TRUE;
////						break;
////					}  
////				}
////				else{
////					sws_scale(pSWSCtx, pFrame->data, pFrame->linesize,0,  pCodecCtx->height, pFrameDest->data, pFrameDest->linesize);  
////					bRead =TRUE;
////					break;
////				}
////			}  
////		}  
////
////		av_free_packet(&packet);  
////
////	}  
////
////	av_free(pFrameDest);  
////	av_free(pFrame);  
////	sws_freeContext(pSWSCtx);  
////
////	if (bRead) return M_OK;

	return M_FAILED;
}

__int64 CVideoHelper::GetDuration()
{
	if (m_pFmtCxt){

		__int64 secs, us;
		secs =((AVFormatContext*) m_pFmtCxt)->duration / AV_TIME_BASE;
		us = ((AVFormatContext*)m_pFmtCxt)->duration % AV_TIME_BASE;
		 
		return (secs * 1000 + int(us *  1000.0  / AV_TIME_BASE));

	}
	return 0;
}


float CVideoHelper::GetFrameRate()
{
	if (m_pStream){
	     double n= ((AVStream*) m_pStream)->r_frame_rate.num/(double)((AVStream * )m_pStream)->r_frame_rate.den;

		return float(n);
	}
	 
	return 0;
}


AVPixelFormat CVideoHelper::GetPixelFormat()
{
		MX_ASSERT(0);
	//if (m_nStreamIndex>=0 ){
	//	RV_ASSERT(m_pStream);	 
	//	RV_ASSERT(m_pStream->codec);	 

	//	return m_pStream->codec->pix_fmt;

	//}

	return AV_PIX_FMT_NONE;
}

int   CVideoHelper::GetVideoWidth()
{
	MX_ASSERT(0);
	//if (m_nStreamIndex>=0 ){
	//	RV_ASSERT(m_pStream);	 
	//	RV_ASSERT(m_pStream->codec);	 

	//	return m_pStream->codec->width;

	//}

	return 0;
}

int   CVideoHelper::GetVideoHeight()
{
	MX_ASSERT(0);

	//if (m_nStreamIndex>=0 ){
	//	RV_ASSERT(m_pStream);	 
	//	RV_ASSERT(m_pStream->codec);	 

	//	return m_pStream->codec->height;
	//}

	return 0;
}

const char* CVideoHelper::GetCodecName()
{
		
	MX_ASSERT(0);

	/*if (m_nStreamIndex >= 0 ){
		RV_ASSERT(m_pStream);	 
		RV_ASSERT(m_pStream->codec);	 

		return m_pStream->codec->codec_name;

	}*/

	return NULL;
}

