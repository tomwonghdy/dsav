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
#include "LiveSource.h"

#include "AvLib.h"


#include <rvb\img.h>

#include "..\davsdk\includes\mox.h"

#include "waveconverter.h"


#define MAX_QUEUE_SIZE (10  * 1024 ) //(15 * 1024 * 1024)
#define MIN_FRAMES      15

#define MIN_AUDIO_FRAMES  3
#define MIN_VIDEO_FRAMES  1

#define VIDEO_PICTURE_QUEUE_SIZE   5
#define SUBPICTURE_QUEUE_SIZE      4


////FfSource支持的变量
//#define MAX_VIDEO_FRAME_DURATION   0
//#define VIDEO_ALIVE                1
//#define AUDIO_ALIVE                2
//#define SUBTITLE_ALIVE             3

//
//
//inline void copy_data_to_image(char* pDestData, int destPitch, char* pSrcData, int srcPitch, int w, int h)
//{
//	int   j;
//	int lineSize = RV_MIN(destPitch, srcPitch);
//
//	for (j = 0; j < h; j++)
//	{
//		memcpy(pDestData, pSrcData, lineSize);
//
//		pDestData += destPitch;
//		pSrcData += srcPitch;
//	}
//}
//
//
//inline AVPixelFormat ConvertToPixelFormat(int depth) {
//	if (depth == 8) {
//		return AV_PIX_FMT_GRAY8;
//	}
//	else if (depth == 24) {
//		return  AV_PIX_FMT_BGR24;
//	}
//	else if (depth == 32) {
//		return  AV_PIX_FMT_BGRA;
//	}
//	else {
//		RV_ASSERT(0);
//	}
//
//	return AV_PIX_FMT_NONE;
//
//}





//static int decode_interrupt_cb(void* ctx)
//{
//	CLiveSource* pFs = (CLiveSource*)ctx;
//	return  FALSE;// pFs->m_bAbortRequest ;
//}


   

#define WORKAROUND_BUGS   1
#define LOWRES            0
#define IDCT              FF_IDCT_AUTO
#define ERROR_CONCEALMENT  3
#define FAST               0

 
  

inline void send_media_data_to_downstream(RvList list, COutputPad* pOutPad) {
	 
	while (list && !rlsIsEmpty(list)) {
		HANDLE h = rlsRemoveHead(list);

		if (pOutPad->IsConnected()) {
			if (!pOutPad->Pass(h, NULL)) {
				mxDestroySample(h);
			}
		}
		else {
			mxDestroySample(h);
		}
	}
}


BOOL  IsRealtimeStream(AVFormatContext* s)
{
	if (NULL == s) return FALSE;

	if (!_stricmp(s->iformat->name, "rtp")
		|| !_stricmp(s->iformat->name, "rtsp")
		|| !_stricmp(s->iformat->name, "sdp")
		|| !_stricmp(s->iformat->name, "rtmp")
		)
		return TRUE;

	return FALSE;

}

//
//该线程从资源里面（文件，网络流,或其他硬件读出媒体数据包如视频包，音频包，字幕包）
//
DWORD WINAPI read_live_package_thread(LPVOID lpParam)
{
	CLiveSource* pLs = (CLiveSource*)lpParam;
	RV_ASSERT(pLs);
 

	FfDecodeContext* pCxt = (FfDecodeContext*)pLs->GetContext();
	MX_ASSERT(pCxt);
	MX_ASSERT(pCxt->lstPicture && pCxt->lstWave);

	RV_ASSERT(pLs->GetMode() == CMediaObject::WM_ACTIVE);

	int ret = 0;
	//int  framethres = RV_MAX(7, pLs->m_nFrameThres);

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

	BOOL  bNeedVideo = ((pLs->m_nOutputType & FF_STREAM_VIDEO) == FF_STREAM_VIDEO);
	BOOL  bNeedAudio = ((pLs->m_nOutputType & FF_STREAM_AUDIO) == FF_STREAM_AUDIO);

	COutputPad* pVideoPad = pLs->GetVideoPad();
	COutputPad* pAudioPad = pLs->GetAudioPad();

	/* read frames from the file */
	for (;;) {
		int state = pLs->GetCurState();
		if (state == MX_STOPPED) {
			break;
		}
		 
		ret = av_read_frame(pCxt->pFormatContext, pkt);
		if (ret >= 0) {
			// check if the packet belongs to a stream we are interested in, otherwise
			// skip it
			if ((pkt->stream_index == pCxt->videoIndex) && bNeedVideo) {
				ret = decode_packet(pCxt, pCxt->pVideoCodecContext, pkt, frame);

				if (ret == 0) {
					send_media_data_to_downstream(pCxt->lstPicture, pVideoPad);
				}				
			}				
			else if ((pkt->stream_index == pCxt->audioIndex) && bNeedAudio) {
				ret = decode_packet(pCxt, pCxt->pAudioCodecContext, pkt, frame);
				if (ret == 0) {
					send_media_data_to_downstream(pCxt->lstWave, pAudioPad);
				}
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
	if (ret == 0 && bNeedVideo) {
		if (pCxt->pVideoCodecContext) {
			ret = decode_packet(pCxt, pCxt->pVideoCodecContext, NULL, frame);
			if (ret == 0) {
				send_media_data_to_downstream(pCxt->lstPicture, pVideoPad);
			}
		} 	
	}
	if (ret == 0 && bNeedAudio) {
		if (pCxt->pAudioCodecContext) {
			ret = decode_packet(pCxt, pCxt->pAudioCodecContext, NULL, frame);
			if (ret == 0) {
				send_media_data_to_downstream(pCxt->lstWave, pAudioPad);
			}
		}
	}

	av_packet_free(&pkt);
	av_frame_free(&frame);

	if (ret < 0) {		   
		pLs->RaiseErrorReport(ret);
	}

	return 0;
}
//
//
// 
CLiveSource::CLiveSource(void)
{ 
	m_nOutputType = FF_STREAM_VIDEO | FF_STREAM_AUDIO /* | FF_STREAM_SUBTITLE*/;

	//m_audioPad.m_pOwner = this;
	//m_videoPad.m_pOwner = this;
	//m_subtitlePad.m_pOwner = this;

	m_audioPad.SetCaptureType(CPad::CT_PUSH);
	m_videoPad.SetCaptureType(CPad::CT_PUSH);
	m_subtitlePad.SetCaptureType(CPad::CT_PUSH);
	//m_audioPad.SetMediaType(MX_MT_AUDIO);
	//m_videoPad.SetMediaType(MX_MT_VIDEO);

	m_nWorkMode = WM_ACTIVE;
	 
	m_hFfContext = NULL;
	m_nBufferSize = 1024000 *4;  //// 设置缓存大小
	m_nStimeout = 5;    // 设置超时时间 
	m_nMaxDelay = 500;    // 设置最大延时
	m_transportType = LS_TRANS_AUTO;
	 
}


CLiveSource::~CLiveSource(void)
{ 
}



 

M_RESULT CLiveSource::Play(const char* _strUrl)
{

	if (NULL == _strUrl) return M_FAILED;

	RV_ASSERT(m_nState == MS_STOPPED);

	m_nState = MS_PLAYING;

	CUtString mstr(_strUrl);

	char* strUrl = mstr ;//;

	ParseUrl(_strUrl, strUrl, mstr.GetLength() +1);

	FfDecodeContext* pCxt = InitDecodeContext(m_videoPad.GetFormatDescriptor(), m_audioPad.GetFormatDescriptor());
	MX_ASSERT(pCxt);
	this->m_hFfContext = pCxt;
	 
	char buff[28];

	// 设置超时时间（微秒）
	sprintf_s(buff, "%d", RV_MAX(3,m_nStimeout) *1000*1000);
	av_dict_set(&pCxt->options, "stimeout", buff, 0);  // 5秒超时

	// 设置缓冲区大小
	sprintf_s(buff, "%d", m_nBufferSize);
	av_dict_set(&pCxt->options, "buffer_size", buff, 0);

	// 设置最大延迟
	sprintf_s(buff, "%d", RV_MAX(3, this->m_nMaxDelay)* 1000);
	av_dict_set(&pCxt->options, "max_delay", buff, 0);  // 500ms

	// 设置RTSP传输协议
	if (m_transportType == LS_TRANS_UDP) {
		av_dict_set(&pCxt->options, "rtsp_transport", "udp", 0);
	}
	else if (m_transportType == LS_TRANS_TCP) {
		av_dict_set(&pCxt->options, "rtsp_transport", "tcp", 0);
	}
	else
	{		
		av_dict_set(&pCxt->options, "rtsp_transport", "tcp+udp", 0);
	}
	  
	if (!OpenSource(m_hFfContext, strUrl)) {
		CloseSource();
		m_nState = MS_STOPPED;

		return M_FAILED;
	}

	//判断是否为实时流
	if (!IsRealtimeStream(pCxt->pFormatContext)) {
		CloseSource();
		m_nState = MS_STOPPED;

		return M_FAILED;
	}

	//stream in
	m_hReadThread = CreateThread(NULL, 0, read_live_package_thread, this, NULL, NULL);
	RV_ASSERT(m_hReadThread);

	return M_OK;

}

//void CLiveSource::Pause(BOOL bResume)
//{
//	;// m_nState = MS_PAUSED;
//}

MX_HANDLE CLiveSource::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode)
{
	if (pErrCode) {
		*pErrCode = M_FAILED; 
	}

	return NULL;
}
// 
//void CLiveSource::Seek(int64_t pos, int64_t rel, BOOL bByBytes)
//{
//	//if ( this->m_audioManager.m_nIndex   >= 0) {
//	//	m_audioManager.ClearPacketQueue();
//	//	m_audioManager.PushPacket(CreateMyAvPacket(&m_flush_pkt));
//	//	//packet_queue_flush(&is->audioq);
//	//	//packet_queue_put(&is->audioq, &flush_pkt);
//	//}
//
//	//if ( this->m_subtitleManager.m_nIndex   >= 0) {
//	//	m_subtitleManager.ClearPacketQueue();
//	//	m_subtitleManager.PushPacket(CreateMyAvPacket(&m_flush_pkt));
//	//	//packet_queue_flush(&is->subtitleq);
//	//	//packet_queue_put(&is->subtitleq, &flush_pkt);
//	//}
//
//	//if (m_videoManager.m_nIndex   >= 0) {
//	//	m_videoManager.ClearPacketQueue();
//	//	m_videoManager.PushPacket(CreateMyAvPacket(&m_flush_pkt));
//
//	//	//packet_queue_flush(&is->videoq);
//	//	//packet_queue_put(&is->videoq, &flush_pkt);
//	//}
//
//	//m_control.Seek(pos, rel, bByBytes);
//
//}

void CLiveSource::Stop()
{

	m_nState = MS_STOPPED;
	 
	if (m_hReadThread) {
		::WaitForSingleObject(m_hReadThread, INFINITE);
		m_hReadThread = NULL;
	}

	CloseSource();

}

/*
void CLiveSource::SetMasterSyncType(CSyncTimer::SYNC_TYPE type) {

	m_nTargetSyncType = type;
}
*/
// 
// void CLiveSource::UpdateMasterSyncType( ) {
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

 


/* pause or resume the video */
//
//void CLiveSource::PauseStream(VideoState *is)
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


//void CLiveSource::SetTargetImageSize(int width, int height)
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
//int CLiveSource::QueuePicture(  AVFrame *src_frame, double pts, int64_t pos, int serial)
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
// BOOL CLiveSource::QueueSubtitle(SubPicture *sp, double pts)
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

//
//
////MX_HANDLE CLiveSource::OnSampleRequest(CPad* pOutputPad, MX_HANDLE hSample, int* pErrCode)
//{
//
//	/* if (m_nState != MS_PLAYING)
//	 {
//		 if (pErrCode) *pErrCode = M_TERMINATED;
//		 return NULL;
//	 }
//
//	 MX_HANDLE hTemp ;
//
//	 if (pOutputPad == &m_audioPad){
//
//		 RV_ASSERT(m_audioPad.IsConnected());
//
//		 if (m_audioManager.IsStarted()){
//
//			 hTemp =  m_audioManager.PopFrame();
//
//			 if (hTemp){
//				  if (pErrCode) *pErrCode = M_OK;
//			 }
//			 else
//			 {
//				 if (pErrCode) *pErrCode = M_DATA_UNAVAILABLE;
//			 }
//
//			 return hTemp;
//		 }
//		 else{
//			 if (pErrCode) *pErrCode = M_DATA_UNAVAILABLE;
//
//			 return NULL;
//		 }
//
//	 }
//	 else if (pOutputPad == &m_videoPad  ){
//
//		 RV_ASSERT(m_videoPad.IsConnected());
//
//		 if (m_videoManager.IsStarted()){
//
//			 hTemp = m_videoManager.PopFrame();
//
//			 if (hTemp){
//				 if (pErrCode) *pErrCode = M_OK;
//			 }
//			 else
//			 {
//				 if (pErrCode) *pErrCode = M_DATA_UNAVAILABLE;
//			 }
//
//			 return hTemp;
//		 }
//		 else{
//			 if (pErrCode) *pErrCode = M_TERMINATED;
//
//			 return NULL;
//		 }
//
//	 }
//
//	  if (pErrCode) *pErrCode = M_FAILED;
//*/
//	return NULL;
//
//}




//
//BOOL CLiveSource::OnPropertyEnquired(CPad* pPad, const char* strName, char* strValue, int nValueSize)
//{
//	if (strName == NULL  ) return FALSE;
//
//	char _strName[MO_MAX_PROPERTY_NAME_LEN];
//
//	strcpy_s(_strName, MO_MAX_PROPERTY_NAME_LEN, strName);
//
//	_strlwr_s(_strName, MO_MAX_PROPERTY_NAME_LEN);
//
//	if (!strcmp(_strName, "isrealtime"))
//	{
//		BOOL ret = IsRealtimeStream();
//
//		if (strValue){
//			RV_ASSERT(nValueSize >0);
//
//			if (ret) strcpy_s(strValue, nValueSize, "TRUE");
//			else      strcpy_s(strValue, nValueSize, "FALSE");
//		}
//
//		return TRUE;
//		 
//	} 
//	else if (!strcmp(_strName, "activestreams"))
//	{
//		int n = 0; 
//
//		if (m_videoManager.IsStarted())    n |= FF_STREAM_VIDEO;
//		if (m_audioManager.IsStarted())    n |= FF_STREAM_AUDIO;
//		if (m_subtitleManager.IsStarted()) n |= FF_STREAM_SUBTITLE;
//
//		if (strValue){
//			RV_ASSERT(nValueSize >0);
//
//			sprintf_s(strValue, nValueSize, "%d", n );
//			 
//		}
//
//		return TRUE;
//
//	} 
//
//	
//
//	return CSource::OnPropertyEnquired(pPad, strName, strValue, nValueSize);
//
//}
//
//BOOL CLiveSource::OnVariableEnquired(CPad* pPad, int nName, void* pValue) 
//{
//	RV_ASSERT(pValue);
//	
//	if (nName == MAX_VIDEO_FRAME_DURATION)
//	{
//		RV_ASSERT(pPad == &m_videoPad);
//		RV_ASSERT(MS_STOPPED != m_nState);
//		RV_ASSERT(m_pFmtCxt);
//
//		*((double*)pValue) = (m_pFmtCxt->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0; 
//
//		return TRUE;
//	}
//	else if (nName == VIDEO_ALIVE)
//	{
//		RV_ASSERT(pPad == &m_videoPad);
//		RV_ASSERT(MS_STOPPED != m_nState);
//		RV_ASSERT(m_pFmtCxt);
//
//		*((BOOL*)pValue) =  m_videoManager.IsStarted(); 
//
//		return TRUE;
//	} 
//	else if (nName == AUDIO_ALIVE)
//	{
//		RV_ASSERT(pPad == &m_audioPad  );
//		RV_ASSERT(MS_STOPPED != m_nState);
//		RV_ASSERT(m_pFmtCxt);
//
//		*((BOOL*)pValue) =  m_audioManager.IsStarted(); 
//
//		return TRUE;
//	} 
//	else if (nName == SUBTITLE_ALIVE)
//	{
//		RV_ASSERT(pPad == &m_subtitlePad    );
//		RV_ASSERT(MS_STOPPED != m_nState);
//		RV_ASSERT(m_pFmtCxt);
//
//		*((BOOL*)pValue) =  m_subtitleManager.IsStarted(); 
//
//		return TRUE;
//	} 
//
//
//	return FALSE;
//}
//  

//
//BOOL CLiveSource::WaitForStreamReady(CFfStream* pStreamManager,int frameCount ,int timeout )
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
// BOOL CLiveSource::Get(MX_MEDIA_TYPE stream, const char* name, int& value)
//{
//	if (NULL == name) return FALSE;
//
//	switch(stream)
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
//	}
//		 
//	 
//	 return FALSE;
// }
//	
// BOOL CLiveSource::Get(MX_MEDIA_TYPE stream, const char* name, double& value)
//{
//	if (NULL == name) return FALSE;
//
//
//	return FALSE;
// }
//
	/*
 BOOL CLiveSource::Set(MX_MEDIA_TYPE stream, const char* name, int  value)
{
	return FALSE;
 }

 BOOL CLiveSource::Set(MX_MEDIA_TYPE stream, const char* name, double  value)
{
	return FALSE;
 }*/