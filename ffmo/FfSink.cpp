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
#include "FfSink.h"

#include "AvLib.h"
#include "Diagnose.h"
 
#include <rvb\dsm.h>

#define STREAM_DURATION   20  //default duraion 20 seconds
#define STREAM_FRAME_RATE 25 /* 25 images/s */
//#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))

static int sws_flags = SWS_BICUBIC;

//单位:秒,最多3小时
#define FS_MAX_DURATION   ((60 * 60 ) * 3)



//假设最多每秒支持60帧
#define MAX_VIDEO_WRAPS    ( 60*4 )

#define MAX_AUDIO_WRAPS    ( 10 )


static void log_packet(const AVFormatContext* fmt_ctx, const AVPacket* pkt)
{
	AVRational* time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

	//printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
	//	av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
	//	av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
	//	av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
	//	pkt->stream_index);
}


/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
 //static AVFrame* get_audio_frame(OutputStream* ost)
 //{
 //	AVFrame* frame = ost->tmp_frame;
 //	int j, i, v;
 //	int16_t* q = (int16_t*)frame->data[0];
 //
 //	/* check if we want to generate more frames */
 //	//if (av_compare_ts(ost->next_pts, ost->enc->time_base,
 //	//	STREAM_DURATION, (AVRational) { 1, 1 }) > 0)
 //	//	return NULL;
 //
 //	//for (j = 0; j < frame->nb_samples; j++) {
 //	//	v = (int)(sin(ost->t) * 10000);
 //	//	for (i = 0; i < ost->enc->ch_layout.nb_channels; i++)
 //	//		*q++ = v;
 //	//	ost->t += ost->tincr;
 //	//	ost->tincr += ost->tincr2;
 //	//}
 //
 //	//frame->pts = ost->next_pts;
 //	//ost->next_pts += frame->nb_samples;
 //
 //	return frame;
 //}


//
///*
// * encode one audio frame and send it to the muxer
// * return 1 when encoding is finished, 0 otherwise
// */
//static int write_audio_frame(FfEncodeContext* pCxt, AVFormatContext* oc, FfOutputStream* ost)
//{
//	AVCodecContext* c;
//	AVFrame* srcFrame = ost->tmp_frame;
//	int ret;
//	int dst_nb_samples = 0;
//
//	c = ost->enc;
//
//	//	frame = get_audio_frame(ost);
//	//	if (frame) 
//	//	{
//			/* convert samples from native format to destination codec format, using the resampler */
//			/* compute destination number of samples */
//	
//}

/**************************************************************/
/* video output */

static AVFrame* alloc_picture(enum AVPixelFormat pix_fmt, int width, int height, int align)
{
	AVFrame* picture;
	int ret;

	picture = av_frame_alloc();
	if (!picture)
		return NULL;

	picture->format = pix_fmt;
	picture->width = width;
	picture->height = height;

	/* allocate the buffers for the frame data */
	ret = av_frame_get_buffer(picture, align/*0*/);
	if (ret < 0) {
		//fprintf(stderr, "Could not allocate frame data.\n");
		//exit(1);
		av_frame_free(&picture);
		return NULL;

	}

	return picture;
}

static BOOL open_video(AVFormatContext* oc, const AVCodec* codec, FfOutputStream* ost, AVDictionary* opt_arg)
{
	int ret;
	AVCodecContext* c = ost->encode;
	AVDictionary* opt = NULL;

	av_dict_copy(&opt, opt_arg, 0);

	/* open the codec */
	ret = avcodec_open2(c, codec, &opt);
	av_dict_free(&opt);

	if (ret < 0) {
		//fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
		//exit(1);
		return FALSE;
	}

	/* allocate and init a re-usable frame */
	ost->frame = alloc_picture(c->pix_fmt, c->width, c->height, 0);
	if (!ost->frame) {
		//fprintf(stderr, "Could not allocate video frame\n");
		//exit(1);
		return FALSE;
	}

	///* If the output format is not YUV420P, then a temporary YUV420P
	// * picture is needed too. It is then converted to the required
	// * output format. */
	//ost->tmp_frame = NULL;
	//if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
	//	ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
	//	if (!ost->tmp_frame) {
	//		//fprintf(stderr, "Could not allocate temporary picture\n");
	//		//exit(1);
	//		return FALSE;
	//	}
	//}

	/* copy the stream parameters to the muxer */
	ret = avcodec_parameters_from_context(ost->stream->codecpar, c);
	if (ret < 0) {
		//fprintf(stderr, "Could not copy the stream parameters\n");
		//exit(1);
		return FALSE;
	}



	return TRUE;
}


CFfSink::CFfSink(void)
{

	m_hEncodeContext = NULL;

	ResetAudioDescEx(&m_audioDstDesc, AV_DEFAULT_AUDIO_FORMAT, AV_DEFAULT_SAMPLE_RATE, GetAudioChannelLayout(AV_DEFAULT_CHANNELS));
	mxResetVideoDesc(&m_videoDstDesc, MX_VF_YUV, AV_DEFAULT_VIDEO_WIDTH, AV_DEFAULT_VIDEO_HEIGHT, GetVideoBitsPerPixel(MX_VF_YUV)/*MX_VIDEO_DEPTH(AV_DEFAULT_VIDEO_FORMAT)*/);

	m_videoDstDesc.bitRate = AV_DEFAULT_VIDEO_BITRATE;
	m_videoDstDesc.fps = STREAM_FRAME_RATE;
	//m_nVideoFps  = STREAM_FRAME_RATE;
	m_nGopSize = 12;
	m_nMaxBframes = 2;
	m_nMacroBlockDecision = 2;

	m_audioPad.m_pOwner = this;
	m_audioPad.SetCaptureType(CPad::CT_PUSH);
	m_audioPad.SetMediaType(MX_MT_AUDIO);

	m_videoPad.m_pOwner = this;
	m_videoPad.SetCaptureType(CPad::CT_PUSH);
	m_videoPad.SetMediaType(MX_MT_VIDEO);

	m_nDuration = STREAM_DURATION;
	m_nOutputType = FS_OT_DEFAULT;

	m_nWorkMode = WM_PASSIVE;
	m_hOutputThread = NULL;

	m_lstAudioFrame = rvCreateList();
	RV_ASSERT(m_lstAudioFrame);

	m_nVideoFps = MX_DEFAULT_VIDEO_FPS ;
	
	   m_nAudioEncodeCount =0;
	   m_nVideoEncodeCount =0;


	m_lstVideoFrame = rvCreateList();
	RV_ASSERT(m_lstVideoFrame);

	//m_bEncodeRemains = TRUE;

	::InitializeCriticalSection(&m_csRun);
}

CFfSink::~CFfSink(void)
{

	ResetFrameArrays();

	if (m_lstAudioFrame) {
		rvDestroyList(m_lstAudioFrame);
		m_lstAudioFrame = NULL;
	}

	//if (m_lstAudioFrameOut) {
	//	rvDestroyList(m_lstAudioFrameOut);
	//	m_lstAudioFrameOut = NULL;
	//}

	if (m_lstVideoFrame) {
		rvDestroyList(m_lstVideoFrame);
		m_lstVideoFrame = NULL;
	}

	RV_ASSERT(m_hEncodeContext == NULL);

	::DeleteCriticalSection(&m_csRun);

}



static DWORD WINAPI ThreadFileSinkProc(LPVOID lpParameter)
{
	CFfSink* pDa = (CFfSink*)lpParameter;

	pDa->HandleMux();

	return 0;
}


static void close_stream(AVFormatContext* oc, FfOutputStream* ost)
{
	if (ost->encode) {
		avcodec_free_context(&ost->encode);
		ost->encode = NULL;
	}
	if (ost->frame) {
		av_frame_free(&ost->frame);
		ost->frame = NULL;
	}
	/*if (ost->tmp_frame) {
		av_frame_free(&ost->tmp_frame);
		ost->tmp_frame = NULL;
	}*/

	if (ost->packet) {
		av_packet_free(&ost->packet);
		ost->packet = NULL;
	}

	if (ost->srcVideoData[0]) {
		av_freep(&ost->srcVideoData[0]);
		ost->srcVideoData[0] = NULL;
	}

	if (ost->sws_ctx) {
		sws_freeContext(ost->sws_ctx);
		ost->sws_ctx = NULL;
	}

	if (ost->swr_ctx) {
		swr_free((SwrContext**)&ost->swr_ctx);
		ost->swr_ctx = NULL;
	}

	if (ost->fifo) {
		av_audio_fifo_free(ost->fifo);
		ost->fifo = NULL;
	}


}

void CFfSink::HandleMux()
{
	FfEncodeContext* pCxt = (FfEncodeContext*)this->m_hEncodeContext;
	RV_ASSERT(pCxt);
	RV_ASSERT(m_lstAudioFrame && m_lstVideoFrame);
	RV_ASSERT(pCxt->pFmtCxt);


	BOOL  bEncodeVideo = (this->m_nOutputType & MX_MT_VIDEO) == MX_MT_VIDEO  && pCxt->bHaveVideo;
	BOOL  bEncodeAudio = (this->m_nOutputType & MX_MT_AUDIO) == MX_MT_AUDIO  && pCxt->bHaveAudio;
	 
	int ret = 0;
	RV_ASSERT((bEncodeVideo || bEncodeAudio));

	while (TRUE)
	{
		ret = Mux(TRUE, FALSE, bEncodeVideo, bEncodeAudio);
		if (MR_FULL == ret)
		{
			RaiseStatusReport(MS_FILEFULL);

			//wait to stop
			//set statement as paused, so the incoming sample will not 
			//be received any more
			m_nState = MS_PAUSED;
			while (m_nState != MS_STOPPED) {
				Sleep(15);
			}

			break;
		}
		else  if (MR_NODATA == ret)
		{
			if (m_nState == MS_STOPPED) {
				break;
			}
			Sleep(15);
		}
		else if (ret < 0) {
			break;
		}
		else {
			Sleep(15);
		}
	}

	if (ret >= 0) {
		RV_ASSERT(m_nState == MS_STOPPED);

		if (MR_FULL != ret) {
			FfOutputStream* ost = &pCxt->audioStream;
			int fifosze =( ost->fifo? av_audio_fifo_size(ost->fifo) : 0);

			while (rlsGetCount(m_lstAudioFrame) > 0 || fifosze>0 ||  rlsGetCount(m_lstVideoFrame) > 0)
			{
			//	PrintToOutput("audio frame left: %d\r\n", rlsGetCount(m_lstAudioFrame));

				ret = Mux(FALSE, TRUE, bEncodeVideo, bEncodeAudio);
				if (ret < 0 || ret == MR_EOF) {
					break;
				}

				if (!(bEncodeVideo || bEncodeAudio)) {
					break;
				}
			}

			//flush
			if ((this->m_nOutputType & MX_MT_VIDEO) == MX_MT_VIDEO)
			{
				FfOutputStream* ost = &pCxt->videoStream;
				ret = write_frame(pCxt->pFmtCxt, ost->encode, ost->stream, NULL, ost->packet);
			}
			if ((this->m_nOutputType & MX_MT_AUDIO) == MX_MT_AUDIO) {
				FfOutputStream* ost = &pCxt->audioStream;
				ret = write_frame(pCxt->pFmtCxt, ost->encode, ost->stream, NULL, ost->packet);
			}
		}

	}
	else {
		RaiseErrorReport(ret);
	}

}

void   CFfSink::DeleteStream()
{
	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;

	if (pCxt == NULL)  return;

	/* Close each codec. */
	//if (pCxt->bHaveVideo)
	close_stream(pCxt->pFmtCxt, &pCxt->videoStream);
	//if (pCxt->bHaveAudio)
	close_stream(pCxt->pFmtCxt, &pCxt->audioStream);

}

AVPixelFormat SelectPixelFormat(const AVPixelFormat* pFormatArr)
{
	if (pFormatArr == NULL) {
		return AV_PIX_FMT_NONE;
	}

	const  enum AVPixelFormat* p = pFormatArr;

	while (p)
	{
		if (*p != AV_PIX_FMT_NONE) return *p;
		p++;
	}

	return  AV_PIX_FMT_NONE;

}


BOOL CheckPixelFormat(const AVPixelFormat* pFormatArr, AVPixelFormat pixelFormat)
{
	RV_ASSERT(pFormatArr);

	const enum AVPixelFormat* p = pFormatArr;

	while (p)
	{
		if (*p == AV_PIX_FMT_NONE) break;

		if (*p == pixelFormat) return TRUE;
		p++;
	}

	return FALSE;

}


BOOL CheckSampleFormat(const AVSampleFormat* pFormatArr, AVSampleFormat  format)
{
	RV_ASSERT(pFormatArr);

	const enum AVSampleFormat* p = pFormatArr;

	while (p)
	{
		if (*p == AV_SAMPLE_FMT_NONE) break;

		if (*p == format) return TRUE;
		p++;
	}

	return FALSE;

}
/* Add an output stream. */
BOOL  CFfSink::AddStream(int mediaType, int codecId)
{
	FfEncodeContext* pCxt = (FfEncodeContext*)this->m_hEncodeContext;
	RV_ASSERT(pCxt);

	FfOutputStream* ost = (mediaType == MX_MT_AUDIO ? &pCxt->audioStream : &pCxt->videoStream);
	AVFormatContext* oc = pCxt->pFmtCxt;
	const AVCodec** codec = (mediaType == MX_MT_AUDIO ? &pCxt->audioCodec : &pCxt->videoCodec);
	enum AVCodecID codec_id = (enum AVCodecID)codecId;

	AVCodecContext* c = NULL;
	int i;

	/* find the encoder */
	if (AV_CODEC_ID_MP3 == codec_id) {
		*codec = avcodec_find_encoder_by_name("libmp3lame");
		if (!(*codec)) {
			*codec = avcodec_find_encoder(codec_id);
		}
	}
	else {
		*codec = avcodec_find_encoder(codec_id);
	}

	if (!(*codec)) {
		/*fprintf(stderr, "Could not find encoder for '%s'\n",
			avcodec_get_name(codec_id));
		exit(1);*/
		return FALSE;
	}

	ost->packet = av_packet_alloc();
	if (!ost->packet) {
		//fprintf(stderr, "Could not allocate AVPacket\n");
		//exit(1);
		return FALSE;
	}



	ost->stream = avformat_new_stream(oc, NULL);
	if (!ost->stream) {
		/*fprintf(stderr, "Could not allocate stream\n");
		exit(1);*/
		return FALSE;
	}
	ost->stream->id = oc->nb_streams - 1;
	c = avcodec_alloc_context3(*codec);
	if (!c) {
		/*fprintf(stderr, "Could not alloc an encoding context\n");
		exit(1);*/
		return FALSE;
	}
	ost->encode = c;
	AVRational tb;

	AVChannelLayout chlay = GetFfmpegLayout(m_audioDstDesc.channelLayout);

	switch ((*codec)->type) {
	case AVMEDIA_TYPE_AUDIO:
	{
		//av_channel_layout_default(&c->ch_layout, m_audioDstDesc.channelCount);
		int ret = av_channel_layout_copy(&c->ch_layout, &chlay);
		if (ret < 0) {
			return FALSE;
		}

		c->sample_fmt = (AVSampleFormat)m_audioDstDesc.sampleFormat;
		if (!CheckSampleFormat((*codec)->sample_fmts, c->sample_fmt))
		{
			c->sample_fmt = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
			m_audioDstDesc.sampleFormat = c->sample_fmt;
		}

		// MP3 特定参数
		c->compression_level = 0;
		c->bit_rate = m_audioDstDesc.bitRate;//  64000;
		c->sample_rate = m_audioDstDesc.sampleRate;// 44100;

		if ((*codec)->supported_samplerates) {
			c->sample_rate = (*codec)->supported_samplerates[0];
			for (i = 0; (*codec)->supported_samplerates[i]; i++) {
				if ((*codec)->supported_samplerates[i] == m_audioDstDesc.sampleRate/*44100*/)
					c->sample_rate = m_audioDstDesc.sampleRate/*44100*/;
			}
		}

		//av_channel_layout_copy(&c->ch_layout, &chlay);

		tb = { 1, c->sample_rate };
		ost->stream->time_base = tb;

		int n = c->frame_size;
	}
	break;
	case AVMEDIA_TYPE_VIDEO:
	{
		c->codec_id = codec_id;

		c->bit_rate = m_videoDstDesc.bitRate;   
		/* Resolution must be a multiple of two. */
		c->width = m_videoDstDesc.width;// 352;
		c->height = m_videoDstDesc.height; // 288;
		/* timebase: This is the fundamental unit of time (in seconds) in terms
		 * of which frame timestamps are represented. For fixed-fps content,
		 * timebase should be 1/framerate and timestamp increments should be
		 * identical to 1. */
		tb = { 1, RV_MAX(1,m_nVideoFps) };
		ost->stream->time_base = tb;// (AVRational) { 1, STREAM_FRAME_RATE };
		c->time_base = ost->stream->time_base;

		c->gop_size = m_nGopSize;/* emit one intra frame every twelve frames at most */

		//设置H264参数
		av_opt_set(c->priv_data, "preset", "ultrafast", 0);
		av_opt_set(c->priv_data, "tune", "zerolatency", 0);

		AVPixelFormat bestfmt = (AVPixelFormat)GetVideoPixelFormat(m_videoDstDesc.format);
		if (!CheckPixelFormat((*codec)->pix_fmts, bestfmt))
		{
			bestfmt = SelectPixelFormat((*codec)->pix_fmts);
			if (bestfmt == AV_PIX_FMT_NONE) {
				return FALSE;
			}
		}
		c->pix_fmt = bestfmt;

		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
			/* just for testing, we also add B-frames */
			c->max_b_frames = m_nMaxBframes;// 2;
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			/* Needed to avoid using macroblocks in which some coeffs overflow.
			 * This does not happen with normal video, it just happens here as
			 * the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = m_nMacroBlockDecision;// 2;
		}
	}
	break;

	default:
		return FALSE;
		//break;
	}

	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	return TRUE;
}

/**************************************************************/
/* audio output */
static AVFrame* alloc_audio_frame(enum AVSampleFormat sample_fmt,
	const AVChannelLayout* channel_layout,
	int sample_rate, int nb_samples)
{
	AVFrame* frame = av_frame_alloc();
	int ret;

	if (!frame) {
		//fprintf(stderr, "Error allocating an audio frame\n");
		//exit(1);
		return NULL;
	}

	frame->format = sample_fmt;
	av_channel_layout_copy(&frame->ch_layout, channel_layout);
	frame->sample_rate = sample_rate;
	frame->nb_samples = nb_samples;

	if (nb_samples) {
		ret = av_frame_get_buffer(frame, 1);
		if (ret < 0) {
			//fprintf(stderr, "Error allocating an audio buffer\n");
			//exit(1);
			av_frame_free(&frame);
			return NULL;
		}
	}



	return frame;
}


static BOOL open_audio(FfEncodeContext* pCxt,  /*AVFormatContext* oc,const AVCodec* codec,	FfOutputStream* ost,*/  AVDictionary* opt_arg)
{
	AVCodecContext* c;
	int nb_samples;
	int ret = 0;
	AVDictionary* opt = NULL;

	c = pCxt->audioStream.encode;


	/* open it */
	av_dict_copy(&opt, opt_arg, 0);
	ret = avcodec_open2(c, pCxt->audioCodec, &opt);
	av_dict_free(&opt);

	if (ret < 0) {
		//fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
		//exit(1);
		return FALSE;
	}

	/* init signal generator */
	//pCxt->audioStream.t = 0;
	//pCxt->audioStream.tincr = 2 * M_PI * 110.0 / c->sample_rate;
	///* increment frequency by 110 Hz per second */
	//pCxt->audioStream.tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;
	 

	/* copy the stream parameters to the muxer */
	ret = avcodec_parameters_from_context(pCxt->audioStream.stream->codecpar, c);
	if (ret < 0) {
		//fprintf(stderr, "Could not copy the stream parameters\n");
		//exit(1);
		return FALSE;
	}

	AVFrame* frame = av_frame_alloc();

	if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
		nb_samples = 10000;
	else
		nb_samples = c->frame_size;

	frame->nb_samples = nb_samples;

	av_channel_layout_copy(&frame->ch_layout, &c->ch_layout);
	frame->sample_rate = c->sample_rate;
	frame->format      = c->sample_fmt;

	
	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0) {
		// 错误处理
		av_frame_free(&frame);
		return ret;
	}

	pCxt->audioStream.frame = frame;

	return TRUE;
}


M_RESULT  CFfSink::StartEncode(const char* strFilePath, BOOL bClearFrame)
{
	if (m_nState == MS_PLAYING) return M_FAILED;

	if (NULL == strFilePath) return M_FILE_NONEXIST;

	if (bClearFrame) {
		ResetFrameArrays();
	}
	  
	FfEncodeContext* pCxt = InitEncodeContext();
	RV_ASSERT(pCxt);
	this->m_hEncodeContext = pCxt;

	BOOL ret = InitContext(pCxt, NULL, strFilePath);

	if (!ret) {
		ReleaseContext();
		return M_FAILED;
	}

	pCxt->bSaveToDisk = !(pCxt->pOutFmt->flags & AVFMT_NOFILE);
	if (pCxt->bSaveToDisk) 
	{
		if (!OpenDiskFile(strFilePath, &ret)) {
			ReleaseContext();
			return M_FAILED;
		}
	}
	 
	m_nState = MS_PLAYING;

	m_hOutputThread = CreateThread(NULL, 0, ThreadFileSinkProc, this, NULL, NULL);
	RV_ASSERT(m_hOutputThread);

	return M_OK;
}

M_RESULT CFfSink::Play(const char* strFilePath)
{
	return StartEncode(strFilePath, TRUE);
}

M_RESULT CFfSink::Replay(const char* strFilePath)
{
	return StartEncode(strFilePath, FALSE);
}

void CFfSink::Stop()
{
	if (m_nState == MS_STOPPED) return;

	{
		CUtAutoLock lock(&m_csRun);
		m_nState = MS_STOPPED;
	}

	if (m_hOutputThread) {
		::WaitForSingleObject(m_hOutputThread, INFINITE);
		m_hOutputThread = NULL;
	}

	if (m_hEncodeContext) {
		FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;

		if (pCxt->bFileOpened) 
		{
			CloseDiskFile(NULL);
		}

		DeleteStream();

		ReleaseEncodeContext(pCxt);

		m_hEncodeContext = NULL;
	}
	  
}


//template <class AudioDataType>
//void _DeinterleaveSample(AudioDataType* out, const AudioDataType* in, int channels, int frames)
//{
//	AudioDataType* outp[8];
//	RV_ASSERT(channels<=8);
//
//	for (int i = 0; i < channels; i++)
//	{
//		outp[i] = out + (i * frames);
//	}
//
//	for (int i = 0; i < frames; i++)
//	{
//		for (int j = 0; j < channels; j++)
//		{
//			*(outp[j]++) = *(in++);
//		}
//	}
//}
//
//void DeinterleaveSamples(int format, int channels,
//									   uint8_t* output, const uint8_t* input,
//									   int data_size)
//{
//	int bits = GetAudioBytesPerSample(format) * 8 ; // AudioOutputSettings::FormatToBits(format);
//
//	if (bits == 8)
//	{
//		_DeinterleaveSample((char*)output, (const char*)input, channels,     data_size/sizeof(char)/channels);
//	}
//	else if (bits == 16)
//	{
//		_DeinterleaveSample((short*)output, (const short*)input, channels, data_size/sizeof(short)/channels);
//	}
//	else
//	{
//		_DeinterleaveSample((int*)output, (const int*)input, channels, data_size/sizeof(int)/channels);
//	}
//}

//
///* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
// * 'nb_channels' channels. */
//static AVFrame* get_audio_frame(FfOutputStream* ost)
//{
//	AVFrame* frame = ost->tmp_frame;
//	int j, i, v;
//	int16_t* q = (int16_t*)frame->data[0];
//
//	///* check if we want to generate more frames */
//	//if (av_compare_ts(ost->next_pts, ost->enc->time_base,
//	//	STREAM_DURATION, (AVRational) { 1, 1 }) > 0)
//	//	return NULL;
//
//	//for (j = 0; j < frame->nb_samples; j++) {
//	//	v = (int)(sin(ost->t) * 10000);
//	//	for (i = 0; i < ost->enc->ch_layout.nb_channels; i++)
//	//		*q++ = v;
//	//	ost->t += ost->tincr;
//	//	ost->tincr += ost->tincr2;
//	//}
//
//	frame->pts = ost->next_pts;
//	ost->next_pts += frame->nb_samples;
//
//	return frame;
//}
// 

//static AVFrame* get_video_frame(FfEncodeContext* pCxt,  FfOutputStream* ost)
//{
//	 
//
//}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise, <0 error
 */
//static int write_video_frame(FfEncodeContext* pCxt, HANDLE hSample/* MX_VIDEO_DATA* pVideoData*/, AVFormatContext* oc, FfOutputStream* ost)
//{
//	RV_ASSERT(hSample);
//
//	AVCodecContext* c = ost->encode;
//
//	int ret = 0;
//
//	
//}



int CFfSink::Mux(BOOL bCheckDuration, BOOL bEncodeToEnd, BOOL& bEncodeVideo, BOOL& bEncodeAudio)
{
	RV_ASSERT(m_hEncodeContext);

	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;

	//	RV_ASSERT(m_lstAudioFrameOut);
	RV_ASSERT(m_lstVideoFrame);


	int ret = 0;

	RV_ASSERT(pCxt->bHaveAudio || pCxt->bHaveVideo);

	if (bEncodeVideo || bEncodeAudio)
	{
		int dura = RV_MAX(1, m_nDuration);
		AVRational rat = { 1,1 };

		/* select the stream to encode */
		if (bEncodeVideo &&
			(!bEncodeAudio || av_compare_ts(pCxt->videoStream.next_pts, pCxt->videoStream.encode->time_base,
				pCxt->audioStream.next_pts, pCxt->audioStream.encode->time_base) <= 0)) {

			FfOutputStream* ost = &pCxt->videoStream;
			if (bCheckDuration) {
				/* check if we want to generate more frames */
				if (av_compare_ts(ost->next_pts, ost->encode->time_base, dura/*STREAM_DURATION*/, rat/*(AVRational) { 1, 1 }*/) > 0)
				{				//空frame，会返回1
					int ret = write_frame(pCxt->pFmtCxt, ost->encode, ost->stream, NULL, ost->packet);

					if (ret >= 0) {
						return MR_FULL;
					}
					return ret;
				}
			}

			HANDLE hSample = rlsRemoveHead(m_lstVideoFrame);

			if (hSample) {
				//bEncodeVideo = !write_video_frame(pCxt, pCxt->pFmtCxt, &pCxt->videoStream);
				ret = WriteVideoData(hSample);

				bEncodeVideo = (ret == 0);

				if (ret == 1) {
					ret = MR_EOF;
				}

				mxDestroySample(hSample);

				return ret;
			}
			else {
				if (bEncodeToEnd) {
					bEncodeVideo = FALSE;
				}
			}

		}
		else {

			if (bCheckDuration) {
				FfOutputStream* ost = &pCxt->audioStream;
				/* check if we want to generate more frames */
				if (av_compare_ts(ost->next_pts, ost->encode->time_base, dura/*STREAM_DURATION*/, rat/*(AVRational) { 1, 1 }*/) > 0)
				{

					//空frame，会返回1
					ret = write_frame(pCxt->pFmtCxt, ost->encode, ost->stream, NULL, ost->packet);

					if (ret >= 0)
					{
						return MR_FULL;
					}
					return ret;
				}
			}

			//HANDLE hSample = GetAudioData(bEncodeToEnd);// rlsRemoveHead(m_lstAudioFrameOut);
			//HANDLE pData = GetAudioData();
			//bEncodeAudio = !write_audio_frame(pCxt, pCxt->pFmtCxt, & pCxt->audioStream );
			if (GetAudioData(bEncodeToEnd, &ret)) {

				ret = WriteAudioData(/*hSample*/);
				if (ret == 1) {
					ret = MR_EOF;
				}
				bEncodeAudio = (ret == 0);
				//mxDestroySample(hSample);

				return ret;
			}
			else {
			 
				if (bEncodeToEnd && ret == MR_NODATA) {
					bEncodeAudio = FALSE;
				}
			 
				return ret;
			}
		}
	}

	return MR_NODATA;

}

int CFfSink::Resample(MxAudioDesc* pDesc,  BYTE* data, int size)
{
	RV_ASSERT(m_hEncodeContext && pDesc && data);
	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;
	FfOutputStream* ost   = &pCxt->audioStream;

	int ret = 0;
	//static int s_count = 0;
	//s_count++;

	//int bytesPerSample = GetAudioBytesPerSample(pDesc->sampleFormat);// av_get_bytes_per_sample(srcSampleFormat);
	//int in_samples = size / bytesPerSample / pDesc->channelCount;// ost->encode->ch_layout.nb_channels;
	int in_samples = pDesc->sampleCount ;

	int64_t out_samples = 0;
	if (ost->swr_ctx == NULL)
	{ 
		AVChannelLayout srcch = GetFfmpegLayout(pDesc->channelLayout) ;
		ret = swr_alloc_set_opts2(&ost->swr_ctx,
			& ost->encode->ch_layout , ost->encode->sample_fmt, ost->encode->sample_rate,
			&srcch/*ost->encode->ch_layout*/, (AVSampleFormat)GetAudioSampleFormat( pDesc->sampleFormat), pDesc->sampleRate,
			0, NULL);
		if (ret < 0) {
			return ret;
		}
		RV_ASSERT(ost->swr_ctx);

		ret = swr_init(ost->swr_ctx);
		if (ret < 0) {
			swr_free(&ost->swr_ctx);
			ost->swr_ctx = NULL;
			return ret;
		}

		out_samples =av_rescale_rnd(in_samples/*pDesc->sampleCount*/, ost->encode->sample_rate, pDesc->sampleRate/*m_nSrcRate*/, AV_ROUND_UP);
	}
	else {
		int64_t delay = swr_get_delay(ost->swr_ctx, ost->encode->sample_rate);
		out_samples = av_rescale_rnd(delay + in_samples,ost->encode->sample_rate, pDesc->sampleRate, AV_ROUND_UP);
	}
	  
	 
	uint8_t* outData[AV_NUM_DATA_POINTERS] = { 0 };
	int linesize = 0;
	ret = av_samples_alloc(outData, &linesize, ost->encode->ch_layout.nb_channels, (int)out_samples, ost->encode->sample_fmt, 1);
	RV_ASSERT(ret >= 0);


	ret = swr_convert(ost->swr_ctx, outData, (int)out_samples, (const uint8_t**)&data, in_samples);

	if (ret > 0)
	{
		if (!ost->fifo)
		{
			RV_ASSERT(ost->encode->frame_size > 0);

			ost->fifo = av_audio_fifo_alloc(ost->encode->sample_fmt, ost->encode->ch_layout.nb_channels, ost->encode->frame_size);
			RV_ASSERT(ost->fifo);

		}

		//int samples = size / (m_codec->bits_per_raw_sample / 8);
		ret = av_audio_fifo_write(ost->fifo, (void**)outData, ret);
		//RV_ASSERT(ret > 0);
		//StoreFrame(outData, linesize, ret);
	}

	av_freep(outData);

	return ret;
}

BOOL  CFfSink::AddAudioData(HANDLE hSample, int* pErrCode)
{
	RV_ASSERT(hSample);
	RV_ASSERT(m_hEncodeContext );
	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;

	BOOL  bEncodeAudio = (this->m_nOutputType & MX_MT_AUDIO) == MX_MT_AUDIO;

	//MxAudioDesc* pDesc =(MxAudioDesc*)mxGetSampleDescriptor(hSample); 
	if (MAX_AUDIO_WRAPS <= rlsGetCount(m_lstAudioFrame)) { 
		return FALSE;
	}
	 
	rlsAddTail(m_lstAudioFrame, hSample);

	return TRUE/*(ret >=0)*/;
}

BOOL CFfSink::AddVideoData(HANDLE hSample, int* pErrCode)
{

	RV_ASSERT(hSample);

	BOOL  bEncodeVideo = (this->m_nOutputType & MX_MT_VIDEO) == MX_MT_VIDEO;
	

	if (!bEncodeVideo) {
		if (pErrCode) {
			*pErrCode = M_UNSUPPORT;
		}
		return FALSE;
	}

	if (MAX_VIDEO_WRAPS <= rlsGetCount(m_lstVideoFrame))
	{
		if (pErrCode) {
			*pErrCode = M_BUFFER_FULL;
		}
		return FALSE;
	}

	rlsAddTail(m_lstVideoFrame, hSample);

	return TRUE;

}
//
//int set_swr_context_and_frame(FfEncodeContext* pCxt, MxAudioDesc* pInDesc, FfOutputStream* ost) {
//
//	int ret = 0;
//
//	AVCodecContext* c = ost->enc;
//
//	RV_ASSERT(pCxt->audioStream.tmp_frame == NULL);
//	RV_ASSERT(pCxt->swr_ctx == NULL);
//
//	AVChannelLayout srclay = GetFfmpegLayout(pInDesc->channelLayout);
//	AVSampleFormat srcFmt = (AVSampleFormat)pInDesc->sampleFormat;
//
//
//	/* create resampler context */
//	pCxt->swr_ctx = swr_alloc();
//	if (!pCxt->swr_ctx) {
//		/*fprintf(stderr, "Could not allocate resampler context\n");
//		exit(1);*/
//		return -1;
//	}
//
//	/* set options */
//	av_opt_set_chlayout(pCxt->swr_ctx, "in_chlayout", &srclay, 0);
//	av_opt_set_int(pCxt->swr_ctx, "in_sample_rate", pInDesc->sampleRate, 0);
//	av_opt_set_sample_fmt(pCxt->swr_ctx, "in_sample_fmt", srcFmt, 0);
//
//	av_opt_set_chlayout(pCxt->swr_ctx, "out_chlayout", &c->ch_layout, 0);
//	av_opt_set_int(pCxt->swr_ctx, "out_sample_rate", c->sample_rate, 0);
//	av_opt_set_sample_fmt(pCxt->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);
//
//	/* initialize the resampling context */
//	if ((ret = swr_init(pCxt->swr_ctx)) < 0) {
//		//fprintf(stderr, "Failed to initialize the resampling context\n");
//		//exit(1);
//		return ret;
//	}
//
//	return 0;
//}
//


int CFfSink::WriteAudioData(/*HANDLE hAudioData*/)
{ 
	RV_ASSERT(  m_hEncodeContext);

	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;
	FfOutputStream* ost = &pCxt->audioStream;
	AVCodecContext* c = ost->encode;

	int ret;
	 
	RV_ASSERT(ost->frame);
	ost->next_pts += ost->frame->nb_samples;
 
	return write_frame(pCxt->pFmtCxt, c, ost->stream, ost->frame/*frame*/, ost->packet);

}

int CFfSink::WriteVideoData(HANDLE hSample)
{
	RV_ASSERT(m_hEncodeContext && hSample);
	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;

	FfOutputStream* ost = &pCxt->videoStream;
	AVCodecContext* c = ost->encode;

	MxVideoDesc* pInDesc = (MxVideoDesc*)mxGetSampleDescriptor(hSample);
	RV_ASSERT(pInDesc);
	AVPixelFormat srcfmt = (AVPixelFormat)GetVideoPixelFormat(pInDesc->format);
	 
	int ret = 0;
		/* as we only generate a YUV420P picture, we must convert it
			 * to the codec pixel format if needed */
	if (!ost->sws_ctx)
	{
		RV_ASSERT(ost->srcVideoData[0] == NULL);

		ost->sws_ctx = sws_getContext(pInDesc->width, pInDesc->height, srcfmt,
			c->width, c->height, c->pix_fmt,
			SCALE_FLAGS, NULL, NULL, NULL);

		if (!ost->sws_ctx) {
			return -1;
			//fprintf(stderr,	"Could not initialize the conversion context\n");
			//exit(1);
		}

		int ret = av_image_alloc(ost->srcVideoData, ost->srcVideoLinesize, pInDesc->width, pInDesc->height, srcfmt, pInDesc->alignBytes);
		if (!ret) {
			return ret;
		}
		ost->srcWidth = pInDesc->width;
		ost->srcHeight = pInDesc->height;
		ost->srcPixelFormat = srcfmt;
		 
	}
	else {

		if (pInDesc->width != ost->srcWidth || pInDesc->height != ost->srcHeight || srcfmt != ost->srcPixelFormat) {
			sws_freeContext(ost->sws_ctx);

			ost->sws_ctx = sws_getContext(pInDesc->width, pInDesc->height, srcfmt,
				c->width, c->height, c->pix_fmt,
				SCALE_FLAGS, NULL, NULL, NULL);

			if (!ost->sws_ctx) {
				return -1;
				//fprintf(stderr,	"Could not initialize the conversion context\n");
				//exit(1);
			}

			if (ost->srcVideoData[0]) {
				av_freep(&ost->srcVideoData[0]);
				ost->srcVideoData[0] = NULL;
			}

			int ret = av_image_alloc(ost->srcVideoData, ost->srcVideoLinesize, pInDesc->width, pInDesc->height, srcfmt, pInDesc->alignBytes);
			if (!ret) {
				return ret;
			}
			ost->srcWidth = pInDesc->width;
			ost->srcHeight = pInDesc->height;
			ost->srcPixelFormat = srcfmt;

		}
	}

	/* when we pass a frame to the encoder, it may keep a reference to it
	 * internally; make sure we do not overwrite it here */
	if (av_frame_make_writable(ost->frame) < 0) {
		//exit(1);
		return -1;
	}

	UINT pSrcDataSize = mxGetSampleDataSize(hSample);

	memcpy(ost->srcVideoData[0], mxGetSampleData(hSample), pSrcDataSize);
	//memcpy(ost->tmp_frame->data[0], mxGetSampleData(hSample), pSrcDataSize);

	RV_ASSERT(ost->sws_ctx);

	ret = sws_scale(ost->sws_ctx, (const uint8_t* const*)ost->srcVideoData,
		ost->srcVideoLinesize, 0, ost->srcHeight/*c->height*/, ost->frame->data,
		ost->frame->linesize);
	RV_ASSERT(ret > 0);

	ost->frame->pts = ost->next_pts++;


	return write_frame(pCxt->pFmtCxt, ost->encode, ost->stream, ost->frame, ost->packet);

	//return write_video_frame(pCxt, pVideoData, pCxt->pFmtCxt, &pCxt->videoStream);

}



void CFfSink::Pause(BOOL bResume)
{

}

void CFfSink::SetImageOutputSize(  int width, int height)
{
	if (m_nState != MS_STOPPED) return; 
	if (width < 8 || height < 8) return;
	 
	m_videoDstDesc.width = width;
	m_videoDstDesc.height = height;

}



void CFfSink::SetVideoBitRate(int bitRate)
{
	m_videoDstDesc.bitRate = bitRate;
}


void CFfSink::SetAudioBitRate(int bitRate)
{
	m_audioDstDesc.bitRate = bitRate;
}

void CFfSink::SetDuration(double val)
{
	//	if (val <FS_MAX_DURATION && val >= 1) m_nDuration = val;

	if (val > FS_MAX_DURATION) val = FS_MAX_DURATION;
	if (val < 1) val = 1;

	m_nDuration = val;


}

void CFfSink::ResetFrameArrays()
{
	while (rlsGetCount(m_lstVideoFrame) /*m_arrVideoFrame.GetCount()*/ > 0) {

		HANDLE hSample = rlsRemoveHead(m_lstVideoFrame);

		mxDestroySample(hSample);
	}

	while (rlsGetCount(m_lstAudioFrame) > 0) {
		HANDLE hSample = rlsRemoveHead(m_lstAudioFrame);
		mxDestroySample(hSample);
	}

	//while (rlsGetCount(m_lstAudioFrameOut) > 0) {
	//	HANDLE hSample = rlsRemoveHead(m_lstAudioFrameOut);
	//	mxDestroySample(hSample);
	//}

}


BOOL CFfSink::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{
	RV_ASSERT(pDescriptor);
	RV_ASSERT(m_nState == MX_STOPPED);

	if (&m_audioPad == pInputPad)
	{
		MxAudioDesc* pDesc = (MxAudioDesc*)pDescriptor;

		RV_ASSERT(pDesc->type == MX_MT_AUDIO);
		if (pDesc->type != MX_MT_AUDIO) return FALSE;

		MxAudioDesc* pDstDesc = (MxAudioDesc*)m_audioPad.GetFormatDescriptor();

		if (pDstDesc->channelLayout != pDesc->channelLayout || pDstDesc->sampleRate != pDesc->sampleRate ||
			pDstDesc->sampleFormat != pDesc->sampleFormat) {
			return FALSE;
		}
		else {
			pDstDesc->alignBytes = pDesc->alignBytes;
		}

		return TRUE;

	}
	else if (&m_videoPad == pInputPad)
	{
		MxVideoDesc* pDesc = (MxVideoDesc*)pDescriptor;

		RV_ASSERT(pDesc->type == MX_MT_VIDEO);
		if (pDesc->type != MX_MT_VIDEO) return FALSE;

		MxVideoDesc* pDstDesc = (MxVideoDesc*)m_videoPad.GetFormatDescriptor();

		*pDstDesc = *pDesc;

		m_videoDstDesc = *pDesc;

		return TRUE;

	}

	return FALSE;

}




BOOL CFfSink::OnSampleReceived(CPad* pInputPad, MX_HANDLE  hSample, int* pErrCode)
{

	RV_ASSERT(hSample && pInputPad);

	CUtAutoLock lock(&m_csRun);

	if (m_nState != MS_PLAYING) {
		if (pErrCode) *pErrCode = M_STOPPED;
		return FALSE;
	}

	if (pInputPad == &m_videoPad) {

		if (!AddVideoData(hSample, pErrCode)) {

			if (pErrCode) *pErrCode = M_BUFFER_FULL;
			return FALSE;
		}
		return TRUE;
	}
	else if (pInputPad == &m_audioPad) {

		if (!AddAudioData(hSample, NULL)) {
			if (pErrCode) *pErrCode = M_BUFFER_FULL;
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;

}

BOOL CFfSink::InitContext(HANDLE hFxCxt, const char* strFormatName,  const char* strFileName)
{
	RV_ASSERT(hFxCxt && strFileName);

	FfEncodeContext* pCxt = (FfEncodeContext*)hFxCxt;
	RV_ASSERT(pCxt);

	BOOL bEncodeVidio = (this->m_nOutputType & MX_MT_VIDEO) == MX_MT_VIDEO;
	BOOL bEncodeAudio = (this->m_nOutputType & MX_MT_AUDIO) == MX_MT_AUDIO;


	/* allocate the output media context */
	int ret = avformat_alloc_output_context2(&pCxt->pFmtCxt, NULL, strFormatName, strFileName);
	if (!pCxt->pFmtCxt) {		 
		if (strFormatName == NULL) {
			avformat_alloc_output_context2(&pCxt->pFmtCxt, NULL, "mpeg", strFileName);
		}
	}
	if (!pCxt->pFmtCxt) {
		return FALSE;
	}

	pCxt->pOutFmt = pCxt->pFmtCxt->oformat;
	//	pCxt->encodeType = this->m_nOutputType;

		/* Add the audio and video streams using the default format codecs
		 * and initialize the codecs. */
	if (pCxt->pOutFmt->video_codec != AV_CODEC_ID_NONE && bEncodeVidio) {
		//if (add_stream(&pCxt->videoStream, pCxt->pFmtCxt, &pCxt->videoCodec, pCxt->pOutFmt->video_codec))
		if (AddStream(MX_MT_VIDEO, pCxt->pOutFmt->video_codec))
		{
			if (SetVideoCodecOptions(pCxt->pOutFmt->video_codec)) {
				pCxt->bHaveVideo = TRUE;
			}
		}
	}
	if (pCxt->pOutFmt->audio_codec != AV_CODEC_ID_NONE && bEncodeAudio) {
		//if (add_stream(&pCxt->audioStream, pCxt->pFmtCxt, &pCxt->audioCodec, pCxt->pOutFmt->audio_codec)) 
		if (AddStream(MX_MT_AUDIO, pCxt->pOutFmt->audio_codec))
		{
			if (SetAudioCodecOptions(pCxt->pOutFmt->audio_codec)) {
				pCxt->bHaveAudio = TRUE;
			}
		}
	}

	if (!(pCxt->bHaveAudio || pCxt->bHaveVideo)) {
		return FALSE;
	}

	/* Now that all the parameters are set, we can open the audio and
	 * video codecs and allocate the necessary encode buffers. */
	if (pCxt->bHaveVideo)
	{
		if (!open_video(pCxt->pFmtCxt, pCxt->videoCodec, &pCxt->videoStream, pCxt->pOption)) {
			return FALSE;
		}
	}

	if (pCxt->bHaveAudio) {
		if (!open_audio(pCxt,/*pCxt->pFmtCxt, pCxt->audioCodec, &pCxt->audioStream,*/ pCxt->pOption)) {
			return FALSE;
		}
	}

	return TRUE;
}

void CFfSink::ReleaseContext(void)
{
	if (this->m_hEncodeContext) {
		ReleaseEncodeContext((FfEncodeContext*)m_hEncodeContext);
		m_hEncodeContext = NULL;
	}
}

BOOL  CFfSink::FeedImage(const RvImage image)
{
	if (NULL == image) return FALSE;
	 
	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;
	if (NULL == pCxt) return FALSE;
	 
	MxVideoDesc* pvd = (MxVideoDesc*)m_videoPad.GetFormatDescriptor();

	int pixfmt = (rviGetDepth(image) == 8 ? MX_VF_U8 : MX_VF_BGR);

	ResetVideoDescEx(pvd, pixfmt, rviGetWidth(image), rviGetHeight(image));
	 
	//生成图像帧数据
	MX_HANDLE hFrameData = mxCreateSample((MxDescriptor*)pvd, sizeof(MxVideoDesc), rviGetData(image), rviGetSize(image));
	RV_ASSERT(hFrameData);

	int64_t pts = pCxt->videoStream.serial ;

	//需要计算帧的显示持续时间
	double frame_duration = av_q2d(pCxt->videoStream.encode->time_base);
	  
	// 获取time base（时间基）
	AVRational time_base = pCxt->videoStream.encode->time_base;

	// 转换为秒
	double tb = av_q2d(time_base);

	mxSetSampleOptions(hFrameData, pts, tb, pCxt->videoStream.serial++, frame_duration);

	if (!AddVideoData(hFrameData, NULL)) {  
		mxDestroySample(hFrameData);
		return FALSE;
	}

	return TRUE;
}


 
BOOL CFfSink::ResampleRaw(int sampleWanted, int* pErrCode)
{ 
	RV_ASSERT(m_lstAudioFrame);
	int ret;
	RV_ASSERT(sampleWanted > 0);

	int  c = 0;
	while (TRUE) {
		if (rlsIsEmpty(m_lstAudioFrame)) {
			if (pErrCode) *pErrCode = MR_NODATA;
			return FALSE;
		}

		MX_HANDLE hSample = rlsRemoveHead(m_lstAudioFrame);
		RV_ASSERT(hSample);

		void* pSampleData = mxGetSampleData(hSample);
		int   size = mxGetSampleDataSize(hSample);
		MxAudioDesc* pAd = (MxAudioDesc*)mxGetSampleDescriptor(hSample);

		ret = Resample(pAd, (BYTE*)pSampleData, size);

		c += pAd->sampleCount;

		mxDestroySample(hSample);

		if (ret < 0) {
			if (pErrCode) *pErrCode = ret;
			return FALSE;
		}
		 
		if (c >= sampleWanted) {
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CFfSink::GetAudioData(BOOL bFillRemains, int* pErrCode)
{
	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;
	RV_ASSERT(pCxt);
	 
	FfOutputStream* ost = &pCxt->audioStream;
	int ret = 0;

	//AVFrame* frame = avcodec_alloc_frame();
	ret = av_frame_make_writable(ost->frame);
	if (ret < 0) {
		//exit(1);
		if (pErrCode)*pErrCode = ret;
		return FALSE;
	}

	//固定的frame size
	if (ost->encode->frame_size > 0) 
	{
		if (!ost->fifo || av_audio_fifo_size(ost->fifo) < ost->encode->frame_size)
		{
			if (!ResampleRaw(ost->encode->frame_size, &ret)) {
				if (ret == MR_NODATA && bFillRemains && av_audio_fifo_size(ost->fifo) > 0) {
					ret = av_samples_set_silence( ost->frame->data, 0, ost->frame->nb_samples,
						ost->encode->ch_layout.nb_channels, ost->encode->sample_fmt);
					if (ret < 0) {
						if (pErrCode)*pErrCode = ret;
						return FALSE;
					}
				}
				else {
					if (pErrCode)*pErrCode = ret;
					return FALSE;
				}				
			}	 
		}
	}
	else {
		RV_ASSERT(ost->frame->nb_samples > 0);
		if (!ost->fifo || av_audio_fifo_size(ost->fifo) < ost->frame->nb_samples)
		{
			if (!ResampleRaw(ost->frame->nb_samples, &ret)) {
				if (ret == MR_NODATA && bFillRemains && av_audio_fifo_size(ost->fifo) > 0) {
					ret = av_samples_set_silence( ost->frame->data, 0, ost->frame->nb_samples,
						 ost->encode->ch_layout.nb_channels, ost->encode->sample_fmt);
					if (ret < 0) {
						if (pErrCode)*pErrCode = ret;
						return FALSE;
					}
				}
				else {
					if (pErrCode)*pErrCode = ret;
					return FALSE;
				}
				
			} 
		}	 
	}
	RV_ASSERT(ost->fifo); 	

	int npsize = (ost->encode->frame_size == 0 ? ost->frame->nb_samples : ost->encode->frame_size);
	RV_ASSERT(npsize <= ost->frame->nb_samples);

	ret = av_audio_fifo_read(ost->fifo, (void**)ost->frame->data, npsize);

	if (ret < 0) {
		if (pErrCode) *pErrCode = ret;
		return FALSE;
	}

	ost->frame->pts = ost->samples_count;
	ost->samples_count += ost->frame->nb_samples;
	 
	return TRUE;

}

//
////转换一帧的函数进行
//int CFfSink::ConvertReceivedAudio(int samplesNeeded)
//{
//	RV_ASSERT(m_lstAudioFrame && m_lstAudioFrameOut);
//
//	//FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;
//	//RV_ASSERT(pCxt);
//	//RV_ASSERT(m_pAudioStream);
//
//	//AVCodecContext* pAudioCxt = pCxt->audioStream.enc;
//
//	//int c =0;
//	int sample_count = 0;
//	__int64 pts = 0;
//
//	MxAudioDesc newDesc = m_audioDstDesc;
//
//	while (rlsGetCount(m_lstAudioFrame) > 0) {
//		MX_HANDLE hSample = NULL;
//
//		hSample = rlsRemoveHead(m_lstAudioFrame);
//		RV_ASSERT(hSample);
//
//		mxGetSampleOptions(hSample, &pts, NULL, NULL);
//
//		void* pSampleData = mxGetSampleData(hSample);
//		MxAudioDesc* pSrcDesc = (MxAudioDesc*)mxGetSampleDescriptor(hSample);
//
//		int newsize = 0;
//		int nSampleCount = m_waveConverter.GetDestSampleCount(pSrcDesc->sampleCount, &newsize);
//		RV_ASSERT(nSampleCount > 0 && newsize > 0);
//
//		uint8_t* pBuffer = new uint8_t[newsize];
//		RV_ASSERT(pBuffer);
//		//av_samples_alloc_array_and_samples
//
//		BOOL ret = m_waveConverter.ExcuteEx((uint8_t*)pSampleData, pSrcDesc->dataSize, pSrcDesc->sampleCount, (uint8_t*)pBuffer, newsize, nSampleCount);
//		RV_ASSERT(ret);
//
//		newDesc.sampleCount = nSampleCount;
//		newDesc.dataSize = newsize;
//
//		MX_HANDLE hNew = mxCreateSample((MxDescriptor*)&newDesc, sizeof(MxAudioDesc), pBuffer, newsize);
//		RV_ASSERT(hNew);
//
//		mxSetSampleOptions(hNew, &pts, NULL, NULL);
//
//		rlsAddTail(m_lstAudioFrameOut, hNew);
//
//		mxDestroySample(hSample);
//		delete[] pBuffer;
//
//		sample_count += newDesc.sampleCount;
//		if (sample_count >= samplesNeeded/* pAudioCxt->frame_size*/) {
//			break;
//		}
//	}
//
//	return sample_count;
//
//}


BOOL CFfSink::OpenDiskFile(const char* strFileName, int* pErrCode)
{
	FfEncodeContext* pCxt = (FfEncodeContext*)this->m_hEncodeContext;
	RV_ASSERT(pCxt);

	int ret;

	/* open the output file, if needed */
	if (!(pCxt->pOutFmt->flags & AVFMT_NOFILE)) {
		ret = avio_open(&pCxt->pFmtCxt->pb, strFileName, AVIO_FLAG_WRITE);
		if (ret < 0) {
			//fprintf(stderr, "Could not open '%s': %s\n", filename,
			//	av_err2str(ret));
			//return 1;
			if (pErrCode)*pErrCode = ret;
			return FALSE;
		}
	}
	else {
		return TRUE;
	}

	/* Write the stream header, if any. */
	ret = avformat_write_header(pCxt->pFmtCxt, &pCxt->pOption);
	if (ret < 0) {
		//fprintf(stderr, "Error occurred when opening output file: %s\n",
		//	av_err2str(ret));
		//return 1;

		/* Close the output file. */
		avio_closep(&pCxt->pFmtCxt->pb);

		if (pErrCode)*pErrCode = ret;

		return FALSE;
	}

	pCxt->bFileOpened = TRUE;

	return TRUE;
}


BOOL CFfSink::CloseDiskFile(int* pErrCode)
{
	FfEncodeContext* pCxt = (FfEncodeContext*)m_hEncodeContext;
	RV_ASSERT(pCxt);
	BOOL b = TRUE;
	int ret = 0;

	if (pCxt->bFileOpened)
	{
		ret = av_write_trailer(pCxt->pFmtCxt);
		if (ret < 0)
		{
			b = FALSE;
			if (pErrCode)*pErrCode = ret;
			//return FALSE;
		}

		if (!(pCxt->pOutFmt->flags & AVFMT_NOFILE))
		{	/* Close the output file. */
			if (ret = avio_closep(&pCxt->pFmtCxt->pb) < 0) {
				b = FALSE;
				if (pErrCode)*pErrCode = ret;
			}
		}
	}

	return b;
}

//
//BOOL CFfSink::AddNewDiskFile(int* pErrCode)
//{
//	if (!CloseDiskFile(pErrCode)) {
//		return FALSE;
//	}
//
//	CUtString strNewFile;
//	m_nFileCounter++;
//
//	char buff[MAX_PATH];
//
//	if (!m_strFileName.IsEmpty() && !m_strPathName.IsEmpty() && !m_strFileExt.IsEmpty()) {
//		 
//		strNewFile.Format("%s\\%s%d.%s", (const char*)m_strPathName, (const char*)m_strFileName, m_nFileCounter, (const char*)m_strFileExt);
//		 
//	}
//	else if (!m_strFileName.IsEmpty() && !m_strFileExt.IsEmpty()) {
//		strNewFile.Format("%s%d.%s", (const char*)m_strFileName, m_nFileCounter, (const char*)m_strFileExt);
//	}
//	else if (!m_strFileName.IsEmpty() && !m_strPathName.IsEmpty()) {
//		strNewFile.Format("%s\\%s%d", (const char*)m_strPathName, (const char*)m_strFileName, m_nFileCounter);
//	}
//	else if (!m_strFileName.IsEmpty()) {
//		strNewFile.Format("%s%d", (const char*)m_strFileName, m_nFileCounter);
//	}
//	else {
//		if (pErrCode) *pErrCode = -1;
//		return FALSE;
//	}
//
//	if (!OpenDiskFile(strNewFile, pErrCode)) {
//		return FALSE;
//	}
//
//	return TRUE;
//}
