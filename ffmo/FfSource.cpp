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
#include "ffsource.h" 

#include <rvb\img.h>
#include "AvLib.h"

#include "..\davsdk\includes\mox.h"

#include "waveconverter.h"
#include "Diagnose.h"


#define MAX_QUEUE_SIZE (10  * 1024 ) //(15 * 1024 * 1024)
#define MIN_FRAMES      15

#define MIN_AUDIO_FRAMES  3
#define MIN_VIDEO_FRAMES  1

#define VIDEO_PICTURE_QUEUE_SIZE   5
#define SUBPICTURE_QUEUE_SIZE      4








inline void copy_data_to_image(char* pDestData, int destPitch, char* pSrcData, int srcPitch, int w, int h)
{
	int   j;
	int lineSize = RV_MIN(destPitch, srcPitch);

	for (j = 0; j < h; j++)
	{
		memcpy(pDestData, pSrcData, lineSize);

		pDestData += destPitch;
		pSrcData += srcPitch;
	}
}


inline AVPixelFormat ConvertToPixelFormat(int depth) {
	if (depth == 8) {
		return AV_PIX_FMT_GRAY8;
	}
	else if (depth == 24) {
		return  AV_PIX_FMT_BGR24;
	}
	else if (depth == 32) {
		return  AV_PIX_FMT_BGRA;
	}
	else {
		RV_ASSERT(0);
	}

	return AV_PIX_FMT_NONE;

}





// static int decode_interrupt_cb(void *ctx)
//{
//    CFfSource *pFs = (CFfSource *)ctx;
//	return pFs->m_bAbortRequest ;
//}





//
//
//void CALLBACK sdl_refresh_timer_cb(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
//{
//
//	VideoState *is =(VideoState *) dwUser;
//
//	//RV_ASSERT(is)
//
//	//::PostMessage(is->hWnd, FF_REFRESH_EVENT, (WPARAM)is, 0);
//}


#define WORKAROUND_BUGS   1
#define LOWRES            0
#define IDCT              FF_IDCT_AUTO
#define ERROR_CONCEALMENT  3
#define FAST               0


//
//void  CFfSource::CloseStreamComponent( int stream_index)
//{
//	/*AVFormatContext *ic = this->m_pFmtCxt;
//    AVCodecContext *avctx;*/
//
//	MX_ASSERT(0);
//   /* if (stream_index < 0 || stream_index >= (int)ic->nb_streams)
//        return;
//
//    avctx = ic->streams[stream_index]->codec;
//	 
//    ic->streams[stream_index]->discard = AVDISCARD_ALL;
//    avcodec_close(avctx);
//
//#if CONFIG_AVFILTER
//    free_buffer_pool(&is->buffer_pool);
//#endif 
//	*/
//
//}
//
// 

//
//该线程从资源里面（文件，网络流,或其他硬件读出媒体数据包如视频包，音频包，字幕包）
//工作在被动模式
DWORD WINAPI read_package_thread(LPVOID lpParam)
{
	CFfSource* pFs = (CFfSource*)lpParam;
	RV_ASSERT(pFs);

	FfDecodeContext* pCxt = (FfDecodeContext*)pFs->GetContext();
	MX_ASSERT(pCxt);
	MX_ASSERT(pCxt->lstPicture && pCxt->lstWave);

	RV_ASSERT(pFs->GetMode() == CMediaObject::WM_PASSIVE);

	int ret = 0;
	int  frameAudiothres = RV_MAX(7, pFs->m_nAudioFrameThres);
	int  frameVideothres = RV_MAX(7, pFs->m_nVideoFrameThres);


	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		/*fprintf(stderr, "Could not allocate frame\n");
		ret = AVERROR(ENOMEM);
		goto end;*/
		return AVERROR(ENOMEM);
	}

	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		/*fprintf(stderr, "Could not allocate packet\n");
		ret = AVERROR(ENOMEM);
		goto end;*/
		av_frame_free(&frame);
		return AVERROR(ENOMEM);
	}

	BOOL  bNeedVideo = ((pFs->m_nOutputType & FF_STREAM_VIDEO) == FF_STREAM_VIDEO);
	BOOL  bNeedAudio = ((pFs->m_nOutputType & FF_STREAM_AUDIO) == FF_STREAM_AUDIO);

	BOOL  bManualStop = FALSE;
	/* read frames from the file */
	for (;;) {
		int state = pFs->GetCurState();
		if (state == MX_STOPPED) {
			bManualStop = TRUE;
			break;
		}

		//判断是否需要解码一些数据
		if (bNeedVideo && bNeedAudio) {
			if (rlsGetCount(pCxt->lstPicture) >= frameVideothres && rlsGetCount(pCxt->lstWave) >= frameAudiothres) {
				Sleep(10);
				continue;
			}
		}
		else if (bNeedVideo) {
			if (rlsGetCount(pCxt->lstPicture) >= frameVideothres) {
				Sleep(10);
				continue;
			}
		}
		else if (bNeedAudio) {
			if (rlsGetCount(pCxt->lstWave) >= frameAudiothres) {
				Sleep(10);
				continue;
			}
		}

		ret = av_read_frame(pCxt->pFormatContext, pkt);
		if (ret >= 0) {
			// check if the packet belongs to a stream we are interested in, otherwise
			// skip it
			if ((pkt->stream_index == pCxt->videoIndex) && bNeedVideo)
				ret = decode_packet(pCxt, pCxt->pVideoCodecContext, pkt, frame);
			else if ((pkt->stream_index == pCxt->audioIndex) && bNeedAudio) {
				//validate_ac3_packet(pkt);
				//AC3解码会因为找不到同步字而失败
				//if (pCxt->pAudioStream->codecpar->codec_id == AV_CODEC_ID_AC3) {
				//	ret = decode_ac3_with_sync_fix(pCxt->pAudioCodecContext, pkt);
				//	if (ret < 0) {
				//		break;
				//	}
				//}
				ret = decode_packet(pCxt, pCxt->pAudioCodecContext, pkt, frame);
			}
				

			av_packet_unref(pkt);

			if (ret < 0)
				break;
		}
		else {
			break;
		}
	}


	/* flush the decoders */
	if (ret == 0 || AVERROR_EOF == ret) {
		if (pCxt->pVideoCodecContext && bNeedVideo)
			ret = decode_packet(pCxt, pCxt->pVideoCodecContext, NULL, frame);
	}
	if (ret == 0 || AVERROR_EOF == ret ) {
		if (pCxt->pAudioCodecContext && bNeedAudio)
			ret = decode_packet(pCxt, pCxt->pAudioCodecContext, NULL, frame);
	}

	av_packet_free(&pkt);
	av_frame_free(&frame);

	if (ret == 0 || AVERROR_EOF == ret) {
		//等待下游接收完成
		while (pFs->GetCurState() != MX_STOPPED)
		{
			if (bNeedVideo  && pFs->GetVideoPad()->IsConnected()) {
				if (rlsGetCount(pCxt->lstPicture) > 0) {
					Sleep(10);	continue;
				}
			}

			if (bNeedAudio && pFs->GetAudioPad()->IsConnected()) {
				if (rlsGetCount(pCxt->lstWave) > 0) {
					Sleep(10);		continue;
				}
			}

			break;
		}

		if (!bManualStop) {
			pFs->RaiseStatusReport(MS_COMPLETE);

		}
	}
	else {
		if (!bManualStop) {
			pFs->RaiseErrorReport(ret);
		}
	}

	return 0;
}
//
//
// 
CFfSource::CFfSource(void)
{
	m_nAudioFrameThres = 7*5;
	m_nVideoFrameThres = 17*5;

	m_nOutputType = FF_STREAM_VIDEO | FF_STREAM_AUDIO /* | FF_STREAM_SUBTITLE*/;

	m_nWorkMode = WM_PASSIVE;

	m_audioPad.m_pOwner = this;
	m_videoPad.m_pOwner = this;
	// m_subtitlePad.m_pOwner = this;
	m_audioPad.SetCaptureType(CPad::CT_PULL);
	m_videoPad.SetCaptureType(CPad::CT_PULL);
	m_audioPad.SetMediaType(MX_MT_AUDIO);
	m_videoPad.SetMediaType(MX_MT_VIDEO);

	//m_subtitlePad.SetCaptureType(CPad::CT_PULL);

	m_hFfContext = NULL;
	
	m_bRawVideoData = FALSE;
	m_bRawAudioData = FALSE;

	m_hReadThread = NULL;


	//m_nVideoSize = VS_DEFAULT;
	//m_nAudioOutput =AO_DEFAULT;

	//设置默认的音视频输出格式
	//SetAudeoFormat(AV_DEFAULT_AUDIO_FORMAT, AV_DEFAULT_SAMPLE_RATE, (int)GetAudioChannelLayout(AV_DEFAULT_CHANNELS));
	//SetVideoFormat(AV_DEFAULT_VIDEO_FORMAT, AV_DEFAULT_VIDEO_WIDTH, AV_DEFAULT_VIDEO_HEIGHT);

	::InitializeCriticalSection(&m_csLock);

	// ::InitializeCriticalSection(&m_csReadPackage);



}


CFfSource::~CFfSource(void)
{
	//::DeleteCriticalSection(&m_csPictQueue);

	MX_ASSERT(m_hFfContext == NULL);
	MX_ASSERT(m_hReadThread == NULL);

	::DeleteCriticalSection(&m_csLock);
	// ::DeleteCriticalSection(&m_csReadPackage);



}




//
//DWORD WINAPI deliver_video(LPVOID lpVoid)
//{
//    double remaining_time = 0.0;
//    
//	CFfSource* pFs = (CFfSource*) lpVoid;
//	RV_ASSERT(pFs);
//
//
//	RV_ASSERT(pFs->m_videoManager.m_nIndex >= 0);
//
//    for (;;) {
//
//		/*if (pFs->m_control.m_bStopped) break;
//
//		if (pFs->m_control.m_bPaused){
//			::Sleep(9);
//		}*/
//
//        if (remaining_time > 0.0)
//            av_usleep((int64_t)(remaining_time * 1000000.0));
//
//        remaining_time = REFRESH_RATE;
//
//
//		pFs->VideoOutput( &remaining_time);     
//		
//
//    }
//
//	return  0;
//
//}


inline BOOL check_realtime_source(const char* _strUrl) {
	if (NULL == _strUrl) return FALSE;

	int len = (int)strlen(_strUrl);
	if (len < 5) return FALSE;

	char tmp[12];
	memcpy(tmp, _strUrl, 5);
	tmp[4] = '\0';

	if (_stricmp(tmp, "rtmp") == 0) return TRUE;
	if (_stricmp(tmp, "rtsp") == 0) return TRUE;
	if (_stricmp(tmp, "http") == 0) return TRUE;


	return FALSE;

}

M_RESULT CFfSource::Play(const char* _strUrl)
{
	if (m_nState == MS_PLAYING) {
		return M_FAILED;
	}

	if (NULL == _strUrl) return M_FAILED;

	RV_ASSERT(m_nState == MS_STOPPED);


	CUtString mstr(_strUrl);

	char* strUrl = mstr ;//[MAX_PATH];

	ParseUrl(_strUrl, strUrl, mstr.GetLength()+1);


	m_nState = MS_PLAYING;

	FfDecodeContext* pCxt = InitDecodeContext(this->m_videoPad.GetFormatDescriptor(), m_audioPad.GetFormatDescriptor());
	MX_ASSERT(pCxt);



	this->m_hFfContext = pCxt;

	if (!OpenSource(m_hFfContext, strUrl)) {
		CloseSource();

		m_nState = MS_STOPPED;
		return M_FAILED;
	}

	//stream in
	m_hReadThread = CreateThread(NULL, 0, read_package_thread, this, NULL, NULL);
	RV_ASSERT(m_hReadThread);

	return M_OK;

}

void CFfSource::Pause(BOOL bResume)
{
	m_nState = MS_PAUSED;
}


void CFfSource::Seek(int64_t  position)
{
	;;
}

void CFfSource::Stop()
{

	m_nState = MS_STOPPED;


	if (m_hReadThread) {
		::WaitForSingleObject(m_hReadThread, INFINITE);
		m_hReadThread = NULL;
	}

	CloseSource();

}

void  CFfSource::SetMode(CMediaObject::WORK_MODE mode)
{
	//限制运行在被动模式
	;;
}



double CFfSource::GetFrameRate()
{
	if (m_hFfContext == NULL) return 0.;

	FfDecodeContext* pFc = (FfDecodeContext*)m_hFfContext;

	if (pFc->pVideoStream == NULL) return 0.;

	//有总时长的时候计算帧率（较为准确）
	if (pFc->pVideoStream && pFc->pVideoStream->duration > 0) {
		return pFc->pVideoStream->nb_frames / (pFc->pVideoStream->duration / 10000.0);
	}

	// 没有总时长的时候，使用分子和分母计算
	return  pFc->pVideoStream->avg_frame_rate.num * 1.0f / pFc->pVideoStream->avg_frame_rate.den;
}

/*
void CFfSource::SetMasterSyncType(CSyncTimer::SYNC_TYPE type) {

	m_nTargetSyncType = type;
}
*/
// 
// void CFfSource::UpdateMasterSyncType( ) {
//
//    
//    if (m_nTargetSyncType == CSyncTimer::ST_VIDEO_MASTER) {
//        if (m_videoManager.m_pStream)
//			 m_syncTimer.m_nSyncType = CSyncTimer::ST_VIDEO_MASTER;
//           // return AV_SYNC_VIDEO_MASTER;
//        else
//			m_syncTimer.m_nSyncType = CSyncTimer::ST_AUDIO_MASTER;
//          //  return AV_SYNC_AUDIO_MASTER;
//    }
//	else if (m_nTargetSyncType == CSyncTimer::ST_AUDIO_MASTER) {
//        
//		if (m_audioManager.m_pStream)
//            m_syncTimer.m_nSyncType = CSyncTimer::ST_AUDIO_MASTER;
//        else
//			 m_syncTimer.m_nSyncType = CSyncTimer::ST_EXTERNAL_CLOCK ;
//
//    }
//	else {
//         m_syncTimer.m_nSyncType =  CSyncTimer::ST_EXTERNAL_CLOCK ;
//    } 
//
//}


//
// 
//static void update_external_clock_pts(VideoState *is, double pts)
//{
//   is->external_clock_time = av_gettime();
//   is->external_clock = pts;
//   is->external_clock_drift = pts - is->external_clock_time / 1000000.0;
//}
//
//static void update_external_clock_speed(VideoState *is, double speed) {
//    update_external_clock_pts(is, get_external_clock(is));
//    is->external_clock_speed = speed;
//}
//
//
//static BOOL IsRealTimeSource(AVFormatContext *s)
//{
//    if(   !strcmp(s->iformat->name, "rtp")
//       || !strcmp(s->iformat->name, "rtsp")
//       || !strcmp(s->iformat->name, "sdp")
//    )
//        return TRUE;
//
//	MX_ASSERT(0);
// ///*   if(s->pb && (   !strncmp(s->filename, "rtp:", 4)
// //                || !strncmp(s->filename, "udp:", 4))
// //      )
// //       return TRUE;*/
//
//    
//	return FALSE;
//
//} 
//


int open_codec_context(int* stream_idx, AVCodecContext** dec_ctx, AVFormatContext* fmt_ctx, /*AVDictionary* options,*/ enum AVMediaType type)
{
	int ret, stream_index;
	AVStream* st = NULL;
	const AVCodec* dec = NULL;

	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0) {
		//fprintf(stderr, "Could not find %s stream in input file '%s'\n",
	   //	 av_get_media_type_string(type), src_filename);
		return ret;
	}
	else {
		stream_index = ret;
		st = fmt_ctx->streams[stream_index];

		//
		if (st->codecpar->codec_id == AV_CODEC_ID_AC3) {
			//liba52 not tested so far
			dec = avcodec_find_decoder_by_name("liba52");
		}
		if (!dec ) {
			/* find decoder for the stream */
			dec = avcodec_find_decoder(st->codecpar->codec_id);
			if (!dec) {
				//fprintf(stderr, "Failed to find %s codec\n",
			   //	 av_get_media_type_string(type));
				return AVERROR(EINVAL);
			}
		} 
		 

		/* Allocate a codec context for the decoder */
		*dec_ctx = avcodec_alloc_context3(dec);
		if (!*dec_ctx) {
			//fprintf(stderr, "Failed to allocate the %s codec context\n",
		   //	 av_get_media_type_string(type));
			return AVERROR(ENOMEM);
		}

		/* Copy codec parameters from input stream to output codec context */
		if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
			//fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
		   //	 av_get_media_type_string(type));
			return ret;
		}

		/* Init the decoders */
		if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
			//fprintf(stderr, "Failed to open %s codec\n",
		   //	 av_get_media_type_string(type));
			return ret;
		}
		*stream_idx = stream_index;
	}

	return 0;
}



BOOL CFfSource::OpenSource(HANDLE hFxCxt, const char* strFileName)
{

	RV_ASSERT(strFileName && hFxCxt);

	FfDecodeContext* pCxt = (FfDecodeContext*)hFxCxt;

	/* = InitContext();
	MX_ASSERT(pCxt);

	this->m_hFfContext = pCxt;*/

	int ret = 0;
	// AVFormatContext* fmt_ctx

	 /* open input file, and allocate format context */
	if (avformat_open_input(&pCxt->pFormatContext, strFileName, NULL, (pCxt->options == NULL ? NULL : &pCxt->options)/*NULL*/) < 0) {
		//fprintf(stderr, "Could not open source file %s\n", src_filename);
		//exit(1);

		return FALSE;
	}

	/* retrieve stream information */
	if (avformat_find_stream_info(pCxt->pFormatContext, NULL) < 0) {
		//fprintf(stderr, "Could not find stream information\n");
		//exit(1);

		return FALSE;
	}

	if (open_codec_context(&pCxt->videoIndex, &pCxt->pVideoCodecContext, pCxt->pFormatContext, /*pCxt->options,*/ AVMEDIA_TYPE_VIDEO) >= 0) {
		pCxt->pVideoStream = pCxt->pFormatContext->streams[pCxt->videoIndex];
	}

	if (open_codec_context(&pCxt->audioIndex, &pCxt->pAudioCodecContext, pCxt->pFormatContext, /*pCxt->options, */AVMEDIA_TYPE_AUDIO) >= 0) {
		pCxt->pAudioStream = pCxt->pFormatContext->streams[pCxt->audioIndex];
	}

	if (pCxt->pVideoCodecContext == NULL && pCxt->pAudioCodecContext == NULL) {

		return FALSE;
	}
	//视频转换
	if (pCxt->pVideoCodecContext) {
		int  width = pCxt->pVideoCodecContext->width;
		int  height = pCxt->pVideoCodecContext->height;
		AVPixelFormat pix_fmt = pCxt->pVideoCodecContext->pix_fmt;

		AVPixelFormat dstfmt = (AVPixelFormat)GetVideoPixelFormat(MX_VF_BGR);

		if (!UpdateSwsContext(pCxt, width, height, pix_fmt, width, height, dstfmt, BITMAP_ALIGN_BYTES)) {
			return FALSE;
		}

		ResetVideoDescEx((MxVideoDesc*)m_videoPad.GetFormatDescriptor(), MX_VF_BGR, width, height);
	}
	//音频转换
	if (pCxt->pAudioCodecContext) {
		/* create resampler context */
		pCxt->pSwrCxt = swr_alloc();
		if (!pCxt->pSwrCxt) {
			return FALSE;
			//fprintf(stderr, "Could not allocate resampler context\n");
			//ret = AVERROR(ENOMEM);
			//goto end;
		}

		av_channel_layout_copy(&pCxt->src_ch_layout, &pCxt->pAudioCodecContext->ch_layout);
		pCxt->dst_ch_layout = GetFfmpegLayout(FF_LAYOUT_STEREO);
		pCxt->src_rate = pCxt->pAudioCodecContext->sample_rate;
		pCxt->dst_rate = AV_DEFAULT_SAMPLE_RATE;
		pCxt->src_sample_fmt = pCxt->pAudioCodecContext->sample_fmt;
		pCxt->dst_sample_fmt = (AVSampleFormat)GetAudioSampleFormat(MX_AF_S16);

		ResetAudioDescEx((MxAudioDesc*)m_audioPad.GetFormatDescriptor(), AV_DEFAULT_AUDIO_FORMAT, AV_DEFAULT_SAMPLE_RATE, FF_LAYOUT_STEREO);

		/* set options */
		av_opt_set_chlayout(pCxt->pSwrCxt, "in_chlayout", &pCxt->src_ch_layout, 0);
		av_opt_set_int(pCxt->pSwrCxt, "in_sample_rate", pCxt->src_rate, 0);
		av_opt_set_sample_fmt(pCxt->pSwrCxt, "in_sample_fmt", pCxt->src_sample_fmt, 0);

		av_opt_set_chlayout(pCxt->pSwrCxt, "out_chlayout", &pCxt->dst_ch_layout, 0);
		av_opt_set_int(pCxt->pSwrCxt, "out_sample_rate", pCxt->dst_rate, 0);
		av_opt_set_sample_fmt(pCxt->pSwrCxt, "out_sample_fmt", pCxt->dst_sample_fmt, 0);

		/* initialize the resampling context */
		if ((ret = swr_init(pCxt->pSwrCxt)) < 0) {
			//fprintf(stderr, "Failed to initialize the resampling context\n");
			//goto end;
			return FALSE;
		} 
	}


	return TRUE;


}


void CFfSource::CloseSource(void)
{

	if (m_hFfContext)
	{
		CUtAutoLock lock(&m_csLock);

		ReleaseContext((FfDecodeContext*)m_hFfContext);
		m_hFfContext = NULL;
	}

}

//
//MX_MEDIA_TYPE CFfSource::GetMediaType(CPad* pOutputPad)
//{
//	if (pOutputPad == &m_audioPad) {
//		return MX_MT_AUDIO;
//	}
//	else  if (pOutputPad == &m_videoPad) {
//		return MX_MT_VIDEO;
//	}
//
//	return MX_MT_UNKNOWN;
//
//}
//
//
//MxDescriptor* CFfSource::GetFormatDescriptor(CPad* pPad)
//{
//	if (pPad) {
//		return pPad->GetFormatDescriptor();
//	}
//	//if (pPad == &m_audioPad){
//	   //  return  ((MxDescriptor*)(&m_audioDesc)); 
//	//}
//	//else  if (pPad == &m_videoPad){
//	   // return  ((MxDescriptor*)(&m_videoDesc));
//	//}
//
//	return NULL;
//}
//
//
//void CFfSource::UpdateBitmapInfo(BITMAPINFO* pInfo, int width, int height, int depth)
//{
//	 
//
//	if (pInfo){
//		BITMAPINFOHEADER* pHeader = &pInfo->bmiHeader;
//
//		 int iLineBytes =( width * depth / 8 +3) / 4 * 4  ;
//
//		pHeader->biWidth  = width ; 
//		pHeader->biHeight = height ; 
//		pHeader->biPlanes = 1; 
//		pHeader->biBitCount    = depth; 
//		pHeader->biCompression = 0; 
//		pHeader->biSizeImage   = iLineBytes  *  pHeader->biHeight    ; 
//		pHeader->biXPelsPerMeter = 0; 
//		pHeader->biYPelsPerMeter = 0; 
//		pHeader->biClrUsed       = 0; 
//		pHeader->biClrImportant  = 0; 
//
//		pHeader->biSize = sizeof(BITMAPINFOHEADER) + pHeader->biSizeImage  ; 
//
//	}
//
//}

//
//
//	VideoPicture *vp;
//
//	for (int i=0; i<VIDEO_PICTURE_QUEUE_SIZE ; i++){
//		vp = &m_pContext->pictq[i];
//
//	 	vp->timer_id = 0;
//	 
//
//		if (vp->pRgbFrame)	destroy_rgb_frame(vp->pRgbFrame);
//
//		vp->pRgbFrame = create_rgb_frame(width,height);
//		RV_ASSERT(vp->pRgbFrame);
//	} 
//}
//
//void CFfSource::OutputVideo(void)
////{
////	RV_ASSERT(m_pContext);
////	
////	VideoPicture *vp;
////
////	SubPicture *sp, *sp2;
////
////	if (::TryEnterCriticalSection(&m_csPictQueue)== FALSE){ return; }
////
////	if (this->m_pContext->abort_request){
////		::LeaveCriticalSection(&m_csPictQueue);
////	}
////
////	if (m_pContext->video_st) {	
////		
////			/* dequeue the picture */
////			vp = &m_pContext->pictq[m_pContext->pictq_rindex];
////
////			/* update current video pts */
////			m_pContext->video_current_pts = vp->pts;
////			m_pContext->video_current_pts_drift = m_pContext->video_current_pts - av_gettime() / 1000000.0;
////			m_pContext->video_current_pos = vp->pos;
//// 
////			/* display picture */			
////			//video_image_display(m_pContext, pDC);  //by hdy 需要更改成通过PAD输出
////			if (m_videoPad.IsConnected()){
////				MX_HANDLE hSample = mxCreateSample(&m_bmInfo, sizeof(m_bmInfo), vp->pRgbFrame->pBuffer,m_bmInfo.bmiHeader.biSizeImage); //new CMediaSample(MT_RGB, &m_bmInfo, vp->pRgbFrame->pBuffer, 0);
////				RV_ASSERT(hSample);
////
////				if (!m_videoPad.Pass(hSample))
////				{
////					mxDestroySample(hSample);
////					//delete pSample;
////				}
////
////			}
////
////			/* update queue size and signal for next picture */
////			if (++m_pContext->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
////				m_pContext->pictq_rindex = 0;
////	
////			vp->timer_id= 0;
////		
////	} 
////
////
////	::LeaveCriticalSection(&m_csPictQueue);
////	
////
////
////	::ReleaseSemaphore(this->m_pContext->hPicQueSema, 1, NULL);
//
//
//}



/* pause or resume the video */
//
//void CFfSource::PauseStream(VideoState *is)
//{
//
//	if (is->paused) {
//		is->frame_timer += av_gettime() / 1000000.0 + is->video_current_pts_drift - is->video_current_pts;
//		if(is->read_pause_return != AVERROR(ENOSYS)){
//			is->video_current_pts = is->video_current_pts_drift + av_gettime() / 1000000.0;
//		}
//
//		is->video_current_pts_drift = is->video_current_pts - av_gettime() / 1000000.0;
//	}
//
//	is->paused = !is->paused;
//	
//
//}


//void CFfSource::SetTargetImageSize(int width, int height)
//{
//	
//
//	m_nImgWidth =( (width % 2 == 0)? width: width-1);
//	m_nImgHeight =( (height % 2 == 0)? height: height-1);
//	
//
//}


/**
*
* @param pts the dts of the pkt / pts of the frame and guessed if not known
*/
//
//int CFfSource::QueuePicture(  AVFrame *src_frame, double pts, int64_t pos, int serial)
//{
//	
//
//	//如果超过一定数量的帧，
//	//需要等待队列释放帧。
//	while(TRUE){
//		if (m_control.m_bStopped) return -1;
//
//		BOOL ret = m_videoManager.IsFrameQueueFull(10);		 
//
//		if (ret) break;
//	}
//
//
//	VideoPicture* pVp =  (VideoPicture* )m_videoManager.TryReadyFrame();
//	
//	if (pVp == NULL) pVp = CreateVideoPicture(m_videoDesc.width ,m_videoDesc.height, m_videoDesc.depth );
//	else{
//		RV_ASSERT(pVp->width == m_videoDesc.width);
//		RV_ASSERT(pVp->height == m_videoDesc.height);		 
//	}
//
//	RV_ASSERT(pVp);
//
//	pVp->pos = pos;
//	pVp->pts = pts;
//	pVp->serial = serial;
//  
//	
//	m_imageConverter.Excute(src_frame->data, src_frame->linesize,  m_pVideoNewData, m_nVideoNewLinesizes);
//
//	RvImage im = pVp->image;
//	RV_ASSERT(im);
//
//	void* pData = rviGetData(im);
//	RV_ASSERT(pData);
//
//	memcpy(pData, m_pVideoNewData[0], m_nVideoNewBuffsize);
//	
//	m_videoManager.PushFrame(pVp);
//
//	return 0;
//}

//
// BOOL CFfSource::QueueSubtitle(SubPicture *sp, double pts)
// {
//	//如果超过一定数量的帧，
//	//需要等待队列释放帧。
//	while(TRUE){
//		if (m_control.m_bStopped) return FALSE  ;
//
//		if (m_subtitleManager.IsFrameQueueFull(10)) break;				
//		 
//	}
//
//	if (sp->sub.pts != AV_NOPTS_VALUE)
//		pts = sp->sub.pts / (double)AV_TIME_BASE;
//
//	sp->pts = pts;
//
//	if ( sp->sub.format == 0){
//
//		int i,j;
//		for (i = 0; i < sp->sub.num_rects; i++)
//		{
//			for (j = 0; j < sp->sub.rects[i]->nb_colors; j++)
//			{
//				/*RGBA_IN(r, g, b, a, (uint32_t*)sp->sub.rects[i]->pict.data[1] + j);
//				y = RGB_TO_Y_CCIR(r, g, b);
//				u = RGB_TO_U_CCIR(r, g, b, 0);
//				v = RGB_TO_V_CCIR(r, g, b, 0);
//				YUVA_OUT((uint32_t*)sp->sub.rects[i]->pict.data[1] + j, y, u, v, a);*/
//			}
//		}
//	
//	}
//	else if (sp->sub.format == 1)
//	{
//		//to do
//	}
//
//	m_subtitleManager.PushFrame(sp);
//
//	return TRUE;
//}
//



MX_HANDLE CFfSource::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode)
{

	if (m_nState != MS_PLAYING)
	{
		if (pErrCode) *pErrCode = M_TERMINATED;
		return NULL;
	}

	MX_HANDLE hTemp;

	if (pOutputPad == &m_audioPad) {

		RV_ASSERT(m_audioPad.IsConnected());

		if (::TryEnterCriticalSection(&m_csLock)) {
			if (NULL == m_hFfContext) {
				::LeaveCriticalSection(&m_csLock);
				return NULL;
			}

			FfDecodeContext* pffcxt = (FfDecodeContext*)m_hFfContext;
			RV_ASSERT(pffcxt->lstWave);

			hTemp = rlsRemoveHead(pffcxt->lstWave);

			
			if (rlsGetCount(pffcxt->lstWave) <= 2) {
				m_nAudioFrameThres ++;
			}

			if (hTemp) {
				if (pErrCode) *pErrCode = M_OK;
			}
			else
			{
				if (pErrCode) *pErrCode = M_DATA_UNAVAILABLE;
			}

			::LeaveCriticalSection(&m_csLock);

			return hTemp;
		}
		else {
			if (pErrCode) *pErrCode = M_DATA_UNAVAILABLE;

			return NULL;
		}

	}
	else if (pOutputPad == &m_videoPad) {

		RV_ASSERT(m_videoPad.IsConnected());

		//CUtAutoLock lock(&m_csLock);
		if (::TryEnterCriticalSection(&m_csLock)) {
			if (NULL == m_hFfContext) {
				::LeaveCriticalSection(&m_csLock);
				return NULL;
			}

			FfDecodeContext* pffcxt = (FfDecodeContext*)m_hFfContext;
			RV_ASSERT(pffcxt->lstPicture);

			hTemp = rlsRemoveHead(pffcxt->lstPicture);

			if (rlsGetCount(pffcxt->lstPicture) <= 2) {
				m_nVideoFrameThres++;
			}

			if (hTemp) {
				if (pErrCode) *pErrCode = M_OK;
			}
			else
			{
				if (pErrCode) *pErrCode = M_DATA_UNAVAILABLE;
			}

			::LeaveCriticalSection(&m_csLock);

			return hTemp;
		}

		/*if (m_videoManager.IsStarted()){

			hTemp = m_videoManager.PopFrame();

			if (hTemp){
				if (pErrCode) *pErrCode = M_OK;
			}
			else
			{
				if (pErrCode) *pErrCode = M_DATA_UNAVAILABLE;
			}

			return hTemp;
		}
		else{
			if (pErrCode) *pErrCode = M_TERMINATED;

			return NULL;
		}*/

	}

	if (pErrCode) *pErrCode = M_FAILED;

	return NULL;

}

//
//void CFfSource::SetVideoFormat(int format, int width, int height)
//{
//
//	//	ResetVideoDescEx(&m_videoDesc, format,   width,  height);
//}
//
//void CFfSource::SetAudeoFormat(int format, int sampleRate, int channelLayout)
//{
//	//	ResetAudioDescEx(&m_audioDesc, format, sampleRate, channelLayout);
//}

//
//BOOL CFfSource::WaitForStreamReady(CFfStream* pStreamManager,int frameCount ,int timeout )
//{
//	int delay = 50;
//
//	int c = timeout/delay;
//	RV_ASSERT(frameCount > 0);
//
//	//等待一些音频数据准备好。
//	//if (pStreamManager->IsStarted())
//	{
//		int i=0;
//		while (pStreamManager->GetFrameCount() < frameCount )
//		{
//			Sleep(delay);
//
//			if ( ++i > c)
//			{
//				return FALSE;
//			}
//		}
//
//		return TRUE;
//	}
//
//	return FALSE;
//}

//
// BOOL CFfSource::Get(MX_MEDIA_TYPE stream, const char* name, int& value)
//{
//	if (NULL == name) return FALSE;
//
//	MX_ASSERT(0);
//	/*switch(stream)
//	{
//	case MX_MT_AUDIO:
//		{
//			AVStream* pStream = m_audioManager.GetCurStream();
//
//			if (NULL == pStream) return FALSE;
//			RV_ASSERT(pStream->codec);
//
//			if (_stricmp(name, "bitrate") == 0)
//			{
//				value = pStream->codec->bit_rate;
//				return TRUE;
//			}
//
//			if (_stricmp(name, "maxbr") == 0)
//			{
//				value = pStream->codec->rc_max_rate;
//				return TRUE;
//			}
//
//			if (_stricmp(name, "minbr") == 0)
//			{
//				value = pStream->codec->rc_min_rate;
//				return TRUE;
//			}
//		}
//		break;
//	default:
//		RV_ASSERT(0);
//	}*/
//		 
//	 
//	 return FALSE;
// }
//	
// BOOL CFfSource::Get(MX_MEDIA_TYPE stream, const char* name, double& value)
//{
//	if (NULL == name) return FALSE;
//
//
//	return FALSE;
// }

	/*
 BOOL CFfSource::Set(MX_MEDIA_TYPE stream, const char* name, int  value)
{
	return FALSE;
 }

 BOOL CFfSource::Set(MX_MEDIA_TYPE stream, const char* name, double  value)
{
	return FALSE;
 }*/