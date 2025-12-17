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
#include "ffmo.h"
#include "AvLib.h"

#include "Diagnose.h"
#include<string>  
#include<vector>  

#include <rvb\img.h>
#include "..\davsdk\includes\mox.h"

using namespace std;


//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif



//int64_t             m_sws_flags = SWS_BICUBIC;
//AVCodecContext*     m_avcodec_opts[AVMEDIA_TYPE_NB]={0};
//AVFormatContext*    m_avformat_opts=NULL;
//struct SwsContext*  m_sws_opts = NULL;
 

//HBITMAP m_hBitmap=NULL;
//BITMAPINFOHEADER m_bmpInfoHeader;








static int debug_mv = 0;
static int debug = 0;
static int workaround_bugs = 1;
static int lowres = 0;
static int idct = FF_IDCT_AUTO;



 

AVChannelLayout GetFfmpegLayout(int layout)
{
	switch (layout) {
	case FF_LAYOUT_MONO:  return  AV_CHANNEL_LAYOUT_MONO;
	case FF_LAYOUT_STEREO:  return AV_CHANNEL_LAYOUT_STEREO;
	case FF_LAYOUT_2POINT1:return AV_CHANNEL_LAYOUT_2POINT1;
	case FF_LAYOUT_2_1:  return AV_CHANNEL_LAYOUT_2_1;
	case FF_LAYOUT_SURROUND:  return AV_CHANNEL_LAYOUT_SURROUND;
	case FF_LAYOUT_3POINT1:  return AV_CHANNEL_LAYOUT_3POINT1;
	case FF_LAYOUT_4POINT0:  return AV_CHANNEL_LAYOUT_4POINT0;
	case FF_LAYOUT_4POINT1:  return AV_CHANNEL_LAYOUT_4POINT1;
	case FF_LAYOUT_2_2:  return AV_CHANNEL_LAYOUT_2_2;
	case FF_LAYOUT_QUAD:  return AV_CHANNEL_LAYOUT_QUAD;
	case FF_LAYOUT_5POINT0:  return AV_CHANNEL_LAYOUT_5POINT0;
	case FF_LAYOUT_5POINT1:  return AV_CHANNEL_LAYOUT_5POINT1;
	case FF_LAYOUT_5POINT0_BACK:  return AV_CHANNEL_LAYOUT_5POINT0_BACK;
	case FF_LAYOUT_5POINT1_BACK:  return AV_CHANNEL_LAYOUT_5POINT1_BACK;
	case FF_LAYOUT_6POINT0:  return AV_CHANNEL_LAYOUT_6POINT0;
	case FF_LAYOUT_6POINT0_FRONT:  return AV_CHANNEL_LAYOUT_6POINT0_FRONT;
	case FF_LAYOUT_HEXAGONAL:  return AV_CHANNEL_LAYOUT_HEXAGONAL;
	case FF_LAYOUT_6POINT1:  return AV_CHANNEL_LAYOUT_6POINT1;
	case FF_LAYOUT_6POINT1_BACK:  return AV_CHANNEL_LAYOUT_6POINT1_BACK;
	case FF_LAYOUT_6POINT1_FRONT:  return AV_CHANNEL_LAYOUT_6POINT1_FRONT;
	case FF_LAYOUT_7POINT0:  return AV_CHANNEL_LAYOUT_7POINT0;
	case FF_LAYOUT_7POINT0_FRONT:  return AV_CHANNEL_LAYOUT_7POINT0_FRONT;
	case FF_LAYOUT_7POINT1:  return AV_CHANNEL_LAYOUT_7POINT1;
	case FF_LAYOUT_7POINT1_WIDE:  return AV_CHANNEL_LAYOUT_7POINT1_WIDE;
	case FF_LAYOUT_7POINT1_WIDE_BACK:  return AV_CHANNEL_LAYOUT_7POINT1_WIDE_BACK;
	case FF_LAYOUT_OCTAGONAL:  return AV_CHANNEL_LAYOUT_OCTAGONAL;
	case FF_LAYOUT_HEXADECAGONAL:  return AV_CHANNEL_LAYOUT_HEXADECAGONAL;
	case FF_LAYOUT_STEREO_DOWNMIX:  return AV_CHANNEL_LAYOUT_STEREO_DOWNMIX;
	case FF_LAYOUT_22POINT2:  return AV_CHANNEL_LAYOUT_22POINT2;
	case FF_LAYOUT_AMBISONIC_FIRST_ORDER:  return AV_CHANNEL_LAYOUT_AMBISONIC_FIRST_ORDER;
	}

	return AV_CHANNEL_LAYOUT_STEREO;

}




int GetDsavChannelLayout(AVChannelLayout layout)
{ 
	switch (layout.u.mask)
	{
	case AV_CH_LAYOUT_MONO:       return FF_LAYOUT_MONO;
	case AV_CH_LAYOUT_STEREO:return  FF_LAYOUT_STEREO;
	case AV_CH_LAYOUT_2POINT1: return  FF_LAYOUT_2POINT1;
	case AV_CH_LAYOUT_2_1:    return FF_LAYOUT_2_1;
	case AV_CH_LAYOUT_SURROUND: return  FF_LAYOUT_SURROUND;
	case AV_CH_LAYOUT_3POINT1:         return  FF_LAYOUT_3POINT1;
	case AV_CH_LAYOUT_4POINT0:    return  FF_LAYOUT_4POINT0;
	case AV_CH_LAYOUT_4POINT1:   return  FF_LAYOUT_4POINT1;
	case AV_CH_LAYOUT_2_2:  return  FF_LAYOUT_2_2;
	case AV_CH_LAYOUT_QUAD:    return FF_LAYOUT_QUAD;
	case AV_CH_LAYOUT_5POINT0:     return  FF_LAYOUT_5POINT0;
	case AV_CH_LAYOUT_5POINT1:  return  FF_LAYOUT_5POINT1;
	case AV_CH_LAYOUT_5POINT0_BACK: return FF_LAYOUT_5POINT0_BACK;
	case AV_CH_LAYOUT_5POINT1_BACK: return FF_LAYOUT_5POINT1_BACK;
	case AV_CH_LAYOUT_6POINT0: return FF_LAYOUT_6POINT0;
	case AV_CH_LAYOUT_6POINT0_FRONT: return  FF_LAYOUT_6POINT0_FRONT;
	case AV_CH_LAYOUT_HEXAGONAL: return  FF_LAYOUT_HEXAGONAL;
	case AV_CH_LAYOUT_6POINT1:  return FF_LAYOUT_6POINT1;
	case AV_CH_LAYOUT_6POINT1_BACK:  return FF_LAYOUT_6POINT1_BACK;
	case AV_CH_LAYOUT_6POINT1_FRONT: return  FF_LAYOUT_6POINT1_FRONT;
	case AV_CH_LAYOUT_7POINT0:   return  FF_LAYOUT_7POINT0;
	case AV_CH_LAYOUT_7POINT0_FRONT: return  FF_LAYOUT_7POINT0_FRONT;
	case AV_CH_LAYOUT_7POINT1: return  FF_LAYOUT_7POINT1;
	case AV_CH_LAYOUT_7POINT1_WIDE: return FF_LAYOUT_7POINT1_WIDE;
	case AV_CH_LAYOUT_7POINT1_WIDE_BACK: return  FF_LAYOUT_7POINT1_WIDE_BACK;
	case AV_CH_LAYOUT_OCTAGONAL:  return  FF_LAYOUT_OCTAGONAL;
	case AV_CH_LAYOUT_HEXADECAGONAL: return FF_LAYOUT_HEXADECAGONAL;
	case AV_CH_LAYOUT_STEREO_DOWNMIX: return FF_LAYOUT_STEREO_DOWNMIX;
	case AV_CH_LAYOUT_22POINT2: return FF_LAYOUT_22POINT2;
	default: return FF_LAYOUT_AMBISONIC_FIRST_ORDER;
	}


	return 0;

}





void CopyImageData(char* pData, AVFrame* pFrame, int pitch, int h, int w)
{

	char* pDestLine = pData;
	int i, j;

	char* pSrcLine = (char*)pFrame->data[0];

	char* pDestTemp, * pSrcTemp;

	for (i = 0; i < h; i++)
	{
		pDestTemp = pDestLine;
		pSrcTemp = pSrcLine;

		for (j = 0; j < w; j++) {
			pDestTemp[2] = pSrcTemp[0];
			pDestTemp[1] = pSrcTemp[1];
			pDestTemp[0] = pSrcTemp[2];

			pDestTemp += 4;
			pSrcTemp += 3;
		}

		pDestLine += pitch;
		pSrcLine += pFrame->linesize[0];
	}

	/*return pData;*/
}








    




 


   

 


BOOL InitFFmpeg()
{

//	av_register_all();
	avformat_network_init();

	return TRUE;
}


void CleanupFFmpeg()
{
	avformat_network_deinit();
}


int check_stream_specifier(AVFormatContext* s, AVStream* st, const char* spec)
{
	int ret = avformat_match_stream_specifier(s, st, spec);
	if (ret < 0)
		av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
	return ret;
}
//
// 


FfDecodeContext* InitDecodeContext(MxDescriptor* pVideoDesc, MxDescriptor* pAudioDesc)
{
	FfDecodeContext* pCxt = new FfDecodeContext();
	MX_ASSERT(pCxt);

	memset(pCxt, 0, sizeof(FfDecodeContext));
	pCxt->audioIndex = -1;
	pCxt->videoIndex = -1;

	if (pAudioDesc) {
		MX_ASSERT(pAudioDesc->type == MX_MT_AUDIO);
	}
	if (pVideoDesc) {
		MX_ASSERT(pVideoDesc->type == MX_MT_VIDEO);
	}


	pCxt->pAudioDesc = (MxAudioDesc*)pAudioDesc;
	pCxt->pVideoDesc = (MxVideoDesc*)pVideoDesc;

	pCxt->lstPicture = rvCreateList();
	pCxt->lstWave = rvCreateList();

	return pCxt;
}

inline void release_media_list(RvList list) {
	if (NULL == list) return;

	while (!rlsIsEmpty(list)) {
		mxDestroySample(rlsRemoveHead(list));
	}
}

void       ReleaseContext(FfDecodeContext* pCxt)
{
	if (pCxt) {

		if (pCxt->pVideoCodecContext) {
			avcodec_free_context(&pCxt->pVideoCodecContext);
		}
		if (pCxt->pAudioCodecContext) {
			avcodec_free_context(&pCxt->pAudioCodecContext);
		}

		if (pCxt->pFormatContext) {
			avformat_close_input(&pCxt->pFormatContext);
		}

		if (pCxt->pSwsContext) {
			sws_freeContext(pCxt->pSwsContext);
		}

		if (pCxt->pSwrCxt) {
			swr_free(&pCxt->pSwrCxt);
		}

		if (pCxt->dst_data) {
			av_freep(&pCxt->dst_data[0]);
		}

		if (pCxt->dstVideoData[0]) {
			av_freep(&pCxt->dstVideoData[0]);
		}

		if (pCxt->options) {
			av_dict_free(&pCxt->options);
		}

		if (pCxt->lstPicture) {
			release_media_list(pCxt->lstPicture);

			rvDestroyList(pCxt->lstPicture);
		}

		if (pCxt->lstWave) {
			release_media_list(pCxt->lstWave);
			rvDestroyList(pCxt->lstWave);
		}


		delete pCxt;

	}
}

BOOL       UpdateSwsContext(FfDecodeContext* pCxt, int srcWid, int srcHei, AVPixelFormat srcFmt, int dstWid, int dstHei, AVPixelFormat dstFmt, int dstBufferAlign)
{


	if (pCxt->pSwsContext)
	{
		if (pCxt->dstVideoData[0]) {
			av_freep(&pCxt->dstVideoData[0]);
			pCxt->dstVideoData[0] = NULL;
		}

		sws_freeContext(pCxt->pSwsContext);
	}

	pCxt->pSwsContext = sws_getContext(srcWid, srcHei, srcFmt, dstWid, dstHei, dstFmt, SWS_BILINEAR, NULL, NULL, NULL);

	if (NULL == pCxt->pSwsContext) {
		pCxt->srcFormat = AV_PIX_FMT_NONE;
		pCxt->dstWidth = 0;
		pCxt->dstHeight = 0;
		pCxt->dstPixelFormat = AV_PIX_FMT_NONE;
		pCxt->dstVideoBuffSize = 0;

		return FALSE;
	}

	int ret = av_image_alloc(pCxt->dstVideoData, pCxt->dstVideoLinesize, dstWid, dstHei, dstFmt, dstBufferAlign);

	if (ret < 0) {
		sws_freeContext(pCxt->pSwsContext);
		pCxt->pSwsContext = NULL;

		pCxt->srcFormat = AV_PIX_FMT_NONE;
		pCxt->dstWidth = 0;
		pCxt->dstHeight = 0;
		pCxt->dstPixelFormat = AV_PIX_FMT_NONE;
		pCxt->dstVideoBuffSize = 0;

		return FALSE;
	}

	pCxt->srcFormat = srcFmt;
	pCxt->dstWidth = dstWid;
	pCxt->dstHeight = dstHei;
	pCxt->dstPixelFormat = dstFmt;
	pCxt->dstVideoBuffSize = ret;

	return TRUE;
}


FfEncodeContext* InitEncodeContext()
{
	FfEncodeContext* pCxt = new FfEncodeContext;
	RV_ASSERT(pCxt);

	memset(pCxt, 0, sizeof(FfEncodeContext));

	return pCxt;
}
void ReleaseEncodeContext(FfEncodeContext* pCxt)
{
	if (pCxt) {

		//if (pCxt->swr_ctx) {
		//	swr_free(&pCxt->swr_ctx);
		//}
	

		if (pCxt->pFmtCxt) {
			RV_ASSERT(pCxt->pOutFmt);
			if (!(pCxt->pOutFmt->flags & AVFMT_NOFILE)) {
				/* Close the output file. */
				avio_closep(&pCxt->pFmtCxt->pb);
			}
			  
			if (pCxt->pOption) {
				av_dict_free(&pCxt->pOption);
			}
			/* free the stream */
			avformat_free_context(pCxt->pFmtCxt);
		}

	 
		
		delete pCxt;
	}

}

 

FFMO_API   int GetAudioChannelLayout(int channels)
{
	AVChannelLayout chlay = AV_CHANNEL_LAYOUT_STEREO;
	av_channel_layout_default(&chlay, channels);

	switch (chlay.u.mask)
	{
	case AV_CH_LAYOUT_MONO:       return FF_LAYOUT_MONO;
	case AV_CH_LAYOUT_STEREO:return  FF_LAYOUT_STEREO;
	case AV_CH_LAYOUT_2POINT1: return  FF_LAYOUT_2POINT1;
	case AV_CH_LAYOUT_2_1:    return FF_LAYOUT_2_1;
	case AV_CH_LAYOUT_SURROUND: return  FF_LAYOUT_SURROUND;
	case AV_CH_LAYOUT_3POINT1:         return  FF_LAYOUT_3POINT1;
	case AV_CH_LAYOUT_4POINT0:    return  FF_LAYOUT_4POINT0;
	case AV_CH_LAYOUT_4POINT1:   return  FF_LAYOUT_4POINT1;
	case AV_CH_LAYOUT_2_2:  return  FF_LAYOUT_2_2;
	case AV_CH_LAYOUT_QUAD:    return FF_LAYOUT_QUAD;
	case AV_CH_LAYOUT_5POINT0:     return  FF_LAYOUT_5POINT0;
	case AV_CH_LAYOUT_5POINT1:  return  FF_LAYOUT_5POINT1;
	case AV_CH_LAYOUT_5POINT0_BACK: return FF_LAYOUT_5POINT0_BACK;
	case AV_CH_LAYOUT_5POINT1_BACK: return FF_LAYOUT_5POINT1_BACK;
	case AV_CH_LAYOUT_6POINT0: return FF_LAYOUT_6POINT0;
	case AV_CH_LAYOUT_6POINT0_FRONT: return  FF_LAYOUT_6POINT0_FRONT;
	case AV_CH_LAYOUT_HEXAGONAL: return  FF_LAYOUT_HEXAGONAL;
	case AV_CH_LAYOUT_6POINT1:  return FF_LAYOUT_6POINT1;
	case AV_CH_LAYOUT_6POINT1_BACK:  return FF_LAYOUT_6POINT1_BACK;
	case AV_CH_LAYOUT_6POINT1_FRONT: return  FF_LAYOUT_6POINT1_FRONT;
	case AV_CH_LAYOUT_7POINT0:   return  FF_LAYOUT_7POINT0;
	case AV_CH_LAYOUT_7POINT0_FRONT: return  FF_LAYOUT_7POINT0_FRONT;
	case AV_CH_LAYOUT_7POINT1: return  FF_LAYOUT_7POINT1;
	case AV_CH_LAYOUT_7POINT1_WIDE: return FF_LAYOUT_7POINT1_WIDE;
	case AV_CH_LAYOUT_7POINT1_WIDE_BACK: return  FF_LAYOUT_7POINT1_WIDE_BACK;
	case AV_CH_LAYOUT_OCTAGONAL:  return  FF_LAYOUT_OCTAGONAL;
	case AV_CH_LAYOUT_HEXADECAGONAL: return FF_LAYOUT_HEXADECAGONAL;
	case AV_CH_LAYOUT_STEREO_DOWNMIX: return FF_LAYOUT_STEREO_DOWNMIX;
	case AV_CH_LAYOUT_22POINT2: return FF_LAYOUT_22POINT2;
	default: return FF_LAYOUT_AMBISONIC_FIRST_ORDER;
	}


	return 0;

}


FFMO_API   int GetAudioBytesPerSample(int format)
{
	return av_get_bytes_per_sample((AVSampleFormat)GetAudioSampleFormat(format));
}

FFMO_API   FF_PIXEL_FORMAT GetVideoPixelFormat(int format)
{
	if (format == MX_VF_U8) return (FF_PIXEL_FORMAT)AV_PIX_FMT_GRAY8;
	else if (format == MX_VF_BGR) return (FF_PIXEL_FORMAT)AV_PIX_FMT_BGR24;
	else if (format == MX_VF_BGRA) return (FF_PIXEL_FORMAT)AV_PIX_FMT_BGRA;
	else if (format == MX_VF_YUV) return (FF_PIXEL_FORMAT)AV_PIX_FMT_YUV420P;
	  
	return (FF_PIXEL_FORMAT)format;
}

//ffmpeg当前版本很多函数是1字节对齐
#define DEFAULT_IMAGE_ALIGN   1

FFMO_API   UINT GetPictureBufferSize(int format, int width, int height, int alignBytes)
{
	AVPixelFormat pixfmt = (AVPixelFormat)GetVideoPixelFormat(format);

	return av_image_get_buffer_size(pixfmt, width, height, alignBytes);
	// return avpicture_get_size(pixfmt, width, height);
}
 

FFMO_API   UINT GetAudioBufferSize(int format, int channels, int frameCount)
{
	AVSampleFormat sampfmt = (AVSampleFormat)GetAudioSampleFormat(format);

	return av_samples_get_buffer_size(NULL, channels, frameCount, sampfmt, 1);


}

FFMO_API   int GetAudioChannelCount(int channelLayout)
{
	AVChannelLayout chlay = GetFfmpegLayout(channelLayout);

	return chlay.nb_channels;
}


FFMO_API   FF_WAVE_FORMAT  GetAudioSampleFormat(int format)
{
	return (FF_WAVE_FORMAT)format;
}



FFMO_API    void ResetAudioDescEx(MxAudioDesc* pDesc, int format, int rate, int layout, int frameCount, int bitRate)
{
	//RV_ASSERT(channelLayout);
	AVChannelLayout chlay = GetFfmpegLayout(layout);

	int channelCount = chlay.nb_channels;// av_get_channel_layout_nb_channels(channelLayout);

	mxResetAudioDesc(pDesc, format, rate, channelCount, frameCount);

	//	pDesc->channelLayout = channelLayout; 
	pDesc->depth = GetAudioBytesPerSample(format) * 8;

	//pDesc->silence = (pDesc->depth == 8 ? 0x80 : 0x0);

	pDesc->bitRate = bitRate;

	//	if ( pDesc->frameCount < (pDesc->sampleRate/4) )
	//		pDesc->frameCount  = ((pDesc->sampleRate/4)+3) & ~3;
		//by hdy 0n 09-07-14 分成1000/20=50毫秒采集周期

	if (pDesc->sampleCount < ((pDesc->sampleRate / 20) / 4))
		pDesc->sampleCount = (((pDesc->sampleRate / 20) / 4) + 3) & ~3;

	pDesc->dataSize = av_samples_get_buffer_size(NULL, channelCount, pDesc->sampleCount, (AVSampleFormat)GetAudioSampleFormat(format), 1);;// CMediaObject::CalcWaveBufferSize(pDesc->depth, channelCount, pDesc->frameCount);
}


//计算方法请参考AVPixelFormat的定义中的注释
FFMO_API   int GetVideoBitsPerPixel(int format)
{
	AVPixelFormat pixfmt = (AVPixelFormat)GetVideoPixelFormat(format);

	switch (pixfmt)
	{
	case AV_PIX_FMT_YUV420P: return 12;
	case AV_PIX_FMT_YUYV422: return 16;
	case AV_PIX_FMT_RGB24: return 24;       ///< packed RGB 8:8:8, 24bpp, RGBRGB...
	case AV_PIX_FMT_BGR24: return 24;      ///< packed RGB 8:8:8, 24bpp, BGRBGR...
	case AV_PIX_FMT_YUV422P: return -1;     ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
	case AV_PIX_FMT_YUV444P: return -1;    ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
	case AV_PIX_FMT_YUV410P: return -1;     ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
	case AV_PIX_FMT_YUV411P: return -1;     ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
	case AV_PIX_FMT_GRAY8:   return 8;      ///<        Y        ,  8bpp
	case AV_PIX_FMT_MONOWHITE: return 1;  ///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
	case AV_PIX_FMT_MONOBLACK: return 1;  ///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
	case AV_PIX_FMT_PAL8:      return -1;      ///< 8 bit with PIX_FMT_RGB32 palette
	case AV_PIX_FMT_YUVJ420P:  return 12;   ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV420P and setting color_range
	case AV_PIX_FMT_YUVJ422P:  return 16;   ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV422P and setting color_range
	case AV_PIX_FMT_YUVJ444P: return 24;   ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV444P and setting color_range
	//case AV_PIX_FMT_XVMC_MPEG2_MC: return -1; ///< XVideo Motion Acceleration via common packet passing
	//case AV_PIX_FMT_XVMC_MPEG2_IDCT : return -1;
	case AV_PIX_FMT_UYVY422: return 16;   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
	case AV_PIX_FMT_UYYVYY411: return 12;///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
	case AV_PIX_FMT_BGR8: return 8;     ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
	case AV_PIX_FMT_BGR4: return 4;     ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
	case AV_PIX_FMT_BGR4_BYTE: return 8; ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
	case AV_PIX_FMT_RGB8: return 8;     ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
	case AV_PIX_FMT_RGB4: return 4;     ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
	case AV_PIX_FMT_RGB4_BYTE: return 8; ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
	case AV_PIX_FMT_NV12: return 12;     ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
	case AV_PIX_FMT_NV21: return -1;     ///< as above, but U and V bytes are swapped

	case AV_PIX_FMT_ARGB: return 32;       ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
	case AV_PIX_FMT_RGBA: return 32;       ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
	case AV_PIX_FMT_ABGR: return 32;       ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
	case AV_PIX_FMT_BGRA: return 32;       ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...

	case AV_PIX_FMT_GRAY16BE: return 16;  ///<        Y        , 16bpp, big-endian
	case AV_PIX_FMT_GRAY16LE: return 16;   ///<        Y        , 16bpp, little-endian
	case AV_PIX_FMT_YUV440P: return -1;   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
	case AV_PIX_FMT_YUVJ440P: return -1;   ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of PIX_FMT_YUV440P and setting color_range
	case AV_PIX_FMT_YUVA420P: return -1;  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
	//case AV_PIX_FMT_VDPAU_H264: return -1; ///< H.264 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
	//case AV_PIX_FMT_VDPAU_MPEG1: return -1; ///< MPEG-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
	//case AV_PIX_FMT_VDPAU_MPEG2: return -1; ///< MPEG-2 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
	//case AV_PIX_FMT_VDPAU_WMV3: return -1; ///< WMV3 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
	//case AV_PIX_FMT_VDPAU_VC1: return -1;  ///< VC-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
	case AV_PIX_FMT_RGB48BE: return 48;   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
	case AV_PIX_FMT_RGB48LE: return 48;    ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian

	case AV_PIX_FMT_RGB565BE: return 16;   ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
	case AV_PIX_FMT_RGB565LE: return 16;   ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
	case AV_PIX_FMT_RGB555BE: return 16;   ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), big-endian, most significant bit to 0
	case AV_PIX_FMT_RGB555LE: return 16;   ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), little-endian, most significant bit to 0

	case AV_PIX_FMT_BGR565BE: return 16;   ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
	case AV_PIX_FMT_BGR565LE: return 16;   ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
	case AV_PIX_FMT_BGR555BE: return 16;  ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), big-endian, most significant bit to 1
	case AV_PIX_FMT_BGR555LE: return 16;   ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), little-endian, most significant bit to 1

	//case AV_PIX_FMT_VAAPI_MOCO: return -1;  ///< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
	//case AV_PIX_FMT_VAAPI_IDCT: return -1;  ///< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
	//case AV_PIX_FMT_VAAPI_VLD: return -1;  ///< HW decoding through VA API, Picture.data[3] contains a vaapi_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers

	case AV_PIX_FMT_YUV420P16LE: return -1;  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	case AV_PIX_FMT_YUV420P16BE: return -1;   ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	case AV_PIX_FMT_YUV422P16LE: return -1;   ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	case AV_PIX_FMT_YUV422P16BE: return -1;   ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	case AV_PIX_FMT_YUV444P16LE: return -1;   ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	case AV_PIX_FMT_YUV444P16BE: return -1;   ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	//case AV_PIX_FMT_VDPAU_MPEG4: return -1;   ///< MPEG4 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
	case AV_PIX_FMT_DXVA2_VLD:   return -1;     ///< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer

	case AV_PIX_FMT_RGB444LE: return 16;   ///< packed RGB 4:4:4, 16bpp, (msb)4A 4R 4G 4B(lsb), little-endian, most significant bits to 0
	case AV_PIX_FMT_RGB444BE: return 16;   ///< packed RGB 4:4:4, 16bpp, (msb)4A 4R 4G 4B(lsb), big-endian, most significant bits to 0
	case AV_PIX_FMT_BGR444LE: return 16;  ///< packed BGR 4:4:4, 16bpp, (msb)4A 4B 4G 4R(lsb), little-endian, most significant bits to 1
	case AV_PIX_FMT_BGR444BE: return 16;   ///< packed BGR 4:4:4, 16bpp, (msb)4A 4B 4G 4R(lsb), big-endian, most significant bits to 1
	case AV_PIX_FMT_GRAY8A:   return 8;     ///< 8bit gray, 8bit alpha
	case AV_PIX_FMT_BGR48BE:  return 48;    ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
	case AV_PIX_FMT_BGR48LE:  return 48;    ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian

		//the following 10 formats have the disadvantage of needing 1 format for each bit depth, thus
		//If you want to support multiple bit depths, then using AV_PIX_FMT_YUV420P16* with the bpp stored separately
		//is better
	case AV_PIX_FMT_YUV420P9BE:   return -1;  ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	case AV_PIX_FMT_YUV420P9LE:   return -1; ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	case AV_PIX_FMT_YUV420P10BE:   return -1; ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	case AV_PIX_FMT_YUV420P10LE:   return  -1; ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	case AV_PIX_FMT_YUV422P10BE:   return  -1; ///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	case AV_PIX_FMT_YUV422P10LE:   return  -1; ///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	case AV_PIX_FMT_YUV444P9BE:   return  -1;  ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	case AV_PIX_FMT_YUV444P9LE:   return  -1;  ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	case AV_PIX_FMT_YUV444P10BE:   return  -1; ///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	case AV_PIX_FMT_YUV444P10LE:   return  -1; ///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	case AV_PIX_FMT_YUV422P9BE:   return  -1;  ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	case AV_PIX_FMT_YUV422P9LE:   return  -1;  ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	//case AV_PIX_FMT_VDA_VLD:      return  -1;   ///< hardware decoding through VDA

#ifdef AV_PIX_FMT_ABI_GIT_MASTER
	case AV_PIX_FMT_RGBA64BE:   return 64;  ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
	case  AV_PIX_FMT_RGBA64LE:   return 64;   ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
	case AV_PIX_FMT_BGRA64BE:   return 64;  ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
	case AV_PIX_FMT_BGRA64LE:   return 64;  ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
#endif

	case AV_PIX_FMT_GBRP:   return  24;      ///< planar GBR 4:4:4 24bpp
	case AV_PIX_FMT_GBRP9BE:   return 27;    ///< planar GBR 4:4:4 27bpp, big-endian
	case AV_PIX_FMT_GBRP9LE:   return 27;    ///< planar GBR 4:4:4 27bpp, little-endian
	case AV_PIX_FMT_GBRP10BE:   return 30;   ///< planar GBR 4:4:4 30bpp, big-endian
	case AV_PIX_FMT_GBRP10LE:   return 30;   ///< planar GBR 4:4:4 30bpp, little-endian
	case AV_PIX_FMT_GBRP16BE:   return 48;   ///< planar GBR 4:4:4 48bpp, big-endian
	case AV_PIX_FMT_GBRP16LE:   return 48;   ///< planar GBR 4:4:4 48bpp, little-endian

		/**
		* duplicated pixel formats for compatibility with libav.
		* FFmpeg supports these formats since May 8 2012 and Jan 28 2012 (commits f9ca1ac7 and 143a5c55)
		* Libav added them Oct 12 2012 with incompatible values (commit 6d5600e85)
		*/
		//case  AV_PIX_FMT_YUVA422P_LIBAV:   return -1;   ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
		//case  AV_PIX_FMT_YUVA444P_LIBAV:   return -1;   ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)

	case  AV_PIX_FMT_YUVA420P9BE:   return -1;   ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
	case  AV_PIX_FMT_YUVA420P9LE:   return -1;   ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
	case  AV_PIX_FMT_YUVA422P9BE:   return -1;   ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
	case  AV_PIX_FMT_YUVA422P9LE:   return -1;   ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
	case  AV_PIX_FMT_YUVA444P9BE:   return -1;   ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
	case  AV_PIX_FMT_YUVA444P9LE:   return -1;   ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
	case  AV_PIX_FMT_YUVA420P10BE:   return -1;  ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
	case  AV_PIX_FMT_YUVA420P10LE:   return -1;  ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
	case  AV_PIX_FMT_YUVA422P10BE:   return -1;  ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
	case  AV_PIX_FMT_YUVA422P10LE:   return -1;  ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
	case  AV_PIX_FMT_YUVA444P10BE:   return -1;  ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
	case  AV_PIX_FMT_YUVA444P10LE:   return -1;  ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
	case  AV_PIX_FMT_YUVA420P16BE:   return -1;  ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
	case  AV_PIX_FMT_YUVA420P16LE:   return -1;  ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
	case  AV_PIX_FMT_YUVA422P16BE:   return -1; ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
	case  AV_PIX_FMT_YUVA422P16LE:   return -1;  ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
	case  AV_PIX_FMT_YUVA444P16BE:   return -1;  ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
	case  AV_PIX_FMT_YUVA444P16LE:   return -1;  ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
	case  AV_PIX_FMT_VDPAU:   return -1;     ///< HW acceleration through VDPAU, Picture.data[3] contains a VdpVideoSurface

#ifndef AV_PIX_FMT_ABI_GIT_MASTER
	case  AV_PIX_FMT_RGBA64BE:   return 64;  ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
	case  AV_PIX_FMT_RGBA64LE:   return 64;  ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
	case  AV_PIX_FMT_BGRA64BE:   return 64;   ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
	case  AV_PIX_FMT_BGRA64LE:   return 64;   ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
#endif
	case  AV_PIX_FMT_0RGB:   return 32;       ///< packed RGB 8:8:8, 32bpp, 0RGB0RGB...
	case  AV_PIX_FMT_RGB0:   return 32;      ///< packed RGB 8:8:8, 32bpp, RGB0RGB0...
	case  AV_PIX_FMT_0BGR:   return 32;      ///< packed BGR 8:8:8, 32bpp, 0BGR0BGR...
	case  AV_PIX_FMT_BGR0:   return 32;      ///< packed BGR 8:8:8, 32bpp, BGR0BGR0...
	case  AV_PIX_FMT_YUVA444P:   return -1;  ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
	case  AV_PIX_FMT_YUVA422P:   return -1;  ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)

	case  AV_PIX_FMT_YUV420P12BE:   return -1; ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	case  AV_PIX_FMT_YUV420P12LE:   return -1; ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	case  AV_PIX_FMT_YUV420P14BE:   return -1; ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
	case  AV_PIX_FMT_YUV420P14LE:   return -1; ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
	case  AV_PIX_FMT_YUV422P12BE:   return -1; ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	case  AV_PIX_FMT_YUV422P12LE:   return -1; ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	case  AV_PIX_FMT_YUV422P14BE:   return -1; ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
	case  AV_PIX_FMT_YUV422P14LE:   return -1; ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	case  AV_PIX_FMT_YUV444P12BE:   return -1; ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	case  AV_PIX_FMT_YUV444P12LE:   return -1; ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	case  AV_PIX_FMT_YUV444P14BE:   return -1; ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
	case  AV_PIX_FMT_YUV444P14LE:   return -1; ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
	case  AV_PIX_FMT_GBRP12BE:   return 36;    ///< planar GBR 4:4:4 36bpp, big-endian
	case  AV_PIX_FMT_GBRP12LE:   return 36;    ///< planar GBR 4:4:4 36bpp, little-endian
	case  AV_PIX_FMT_GBRP14BE:   return 42;    ///< planar GBR 4:4:4 42bpp, big-endian
	case  AV_PIX_FMT_GBRP14LE:   return 42;    ///< planar GBR 4:4:4 42bpp, little-endian

	}

	return -1;
}


FFMO_API    void ResetVideoDescEx(MxVideoDesc* pDesc, int format, int width, int height, int bitRate, int alignBytes)
{
	RV_ASSERT(pDesc);
	RV_ASSERT(width > 0);
	RV_ASSERT(height > 0);

	//AVPixelFormat pixfmt = (AVPixelFormat)GetVideoPixelFormat(format);

	mxResetVideoDesc(pDesc, format, width, height, GetVideoBitsPerPixel(format));

	pDesc->dataSize = GetPictureBufferSize(format, width, height, alignBytes);
	pDesc->bitRate = bitRate;

}


FFMO_API   int64_t  GetAvTime()
{
	return av_gettime();
}


FFMO_API   int  SleepAv(unsigned int usec)
{
	if (usec < 0) {
		// 处理负值情况
		usec = 0;
	}
	if (usec > 10000000) { // 限制最大睡眠时间，例如10秒
		usec = 10000000;
	}

	return av_usleep(usec);
}

FFMO_API   int  GetAudioSilence(FF_WAVE_FORMAT format)
{
	AVSampleFormat fmt = (AVSampleFormat)format;
	switch (fmt) {
	case AV_SAMPLE_FMT_U8:
	case AV_SAMPLE_FMT_U8P:
		return 0x80; // 无符号8位静音值是128
		break;
	case AV_SAMPLE_FMT_S16:
	case AV_SAMPLE_FMT_S16P:
		return  0;  // 16位静音值是0
		break;
	case AV_SAMPLE_FMT_S32:
	case AV_SAMPLE_FMT_S32P:
	case AV_SAMPLE_FMT_FLT:
	case AV_SAMPLE_FMT_FLTP:
		return  0 ;  // 32位静音值是0
		break;
	case AV_SAMPLE_FMT_DBL:
	case AV_SAMPLE_FMT_DBLP:
		return  0 ;  // 64位静音值是0
		break;
	default:
		return  0 ;  // 默认按32位处理
		break;
	}
}


M_RESULT  SeekMediaPos(AVFormatContext* pFmtCxt, int streamIndex, AVRational timeBase, double _position)
{
	//if (m_nStreamIndex < 0) return M_CLOSED;


	RV_ASSERT(pFmtCxt);
	//RV_ASSERT(m_pStream);

	double n = (double)(_position * 1000);
	__int64 position = __int64(n / 1000.0 * AV_TIME_BASE);

	AVRational time_base_q = { 1, AV_TIME_BASE };

	__int64 seekTime = av_rescale_q(position, time_base_q, timeBase/*m_pStream->time_base*/);

	__int64 seekStreamDuration = pFmtCxt->duration;

	int flags = AVSEEK_FLAG_BACKWARD;

	if (seekTime > 0 && seekTime < seekStreamDuration)
		flags |= AVSEEK_FLAG_ANY; // H.264 I frames don't always register as "keyframes" in FFmpeg

	int ret = av_seek_frame(pFmtCxt, streamIndex, seekTime, flags);

	if (ret >= 0)
	{
		return M_OK;
	}

	return M_FAILED;
}


static int output_audio_frame(FfDecodeContext* pCxt, MxAudioDesc * pAudioDesc,  AVFrame* frame)
{
	AVStream* pStream = pCxt->pAudioStream;

	int64_t src_nb_samples = frame->nb_samples, dst_nb_samples;
	AVChannelLayout src_ch_layout = pCxt->src_ch_layout;
	AVChannelLayout dst_ch_layout = pCxt->dst_ch_layout;
	 
	AVSampleFormat dst_sample_fmt = pCxt->dst_sample_fmt;// (AVSampleFormat)GetAudioSampleFormat(pAudioDesc->format);

	//uint8_t ** dst_data = NULL;
	int  dst_linesize;


	/* buffer is going to be directly written to a rawaudio file, no alignment */
	int  dst_nb_channels = dst_ch_layout.nb_channels;
	int ret = 0;

	if (pCxt->dst_data == NULL) {
		/* compute the number of converted samples: buffering is avoided
	    * ensuring that the output buffer will contain at least all the
	    * converted input samples */
		dst_nb_samples = av_rescale_rnd(src_nb_samples, pAudioDesc->sampleRate /*dst_rate*/, pCxt->pAudioCodecContext->sample_rate/*src_rate*/, AV_ROUND_UP);
		 
		
		ret = av_samples_alloc_array_and_samples(&pCxt->dst_data, &dst_linesize, dst_nb_channels,
			dst_nb_samples, dst_sample_fmt, 0);
		if (ret < 0) {
			// fprintf(stderr, "Could not allocate destination samples\n");
			//goto end;
			return -1;
		}
		pCxt->dst_line_size = dst_linesize;
		pCxt->max_dst_nb_samples = dst_nb_samples;
	}
	else {
		/* compute destination number of samples */
		dst_nb_samples = av_rescale_rnd(swr_get_delay(pCxt->pSwrCxt, pCxt->pAudioCodecContext->sample_rate/*src_rate*/) +
			src_nb_samples, pAudioDesc->sampleRate /*dst_rate*/, pCxt->pAudioCodecContext->sample_rate /*src_rate*/, AV_ROUND_UP);

		if (dst_nb_samples > pCxt->max_dst_nb_samples) {
			av_freep(&pCxt->dst_data[0]);
			// pCxt->dst_data  should not be set to null after be freed by av_afreep
			////pCxt->dst_data = NULL;

			
			ret = av_samples_alloc(pCxt->dst_data, &dst_linesize, dst_nb_channels,
				dst_nb_samples, dst_sample_fmt, 1);

			if (ret < 0) {
				return -1;
			}

			pCxt->dst_line_size = dst_linesize;
			pCxt->max_dst_nb_samples = dst_nb_samples;
		}
	}
	

	/* convert to destination format */
	 ret = swr_convert(pCxt->pSwrCxt, pCxt->dst_data, dst_nb_samples, (const uint8_t**)frame->data , src_nb_samples);
	if (ret < 0) {
		//fprintf(stderr, "Error while converting\n");
		//goto end;
		return -1;
	}
	int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels, ret, dst_sample_fmt, 1);
	if (dst_bufsize < 0) {
		/*fprintf(stderr, "Could not get sample buffer size\n");
		goto end;*/
		
		return -1;
	}
	RV_ASSERT(dst_bufsize > 0);

	pAudioDesc->dataSize = dst_bufsize;
	pAudioDesc->sampleCount = ret;
	pAudioDesc->channelLayout = GetDsavChannelLayout(dst_ch_layout);
	pAudioDesc->channelCount = dst_ch_layout.nb_channels;

	MX_HANDLE hFrameData = mxCreateSample((MxDescriptor*)pAudioDesc, sizeof(MxAudioDesc), pCxt->dst_data[0] , dst_bufsize);
	RV_ASSERT(hFrameData);

	int64_t pts = frame->pts;

	// 获取time base（时间基）
	AVRational time_base = pStream->time_base; 
	// 转换为秒
	double tb = av_q2d(time_base); 
	 
	double dura = CalculateAudioFrameDuration(frame, pAudioDesc->sampleRate);

	mxSetSampleOptions(hFrameData, pts, tb, pCxt->audioSerial++, dura);

	rlsAddTail(pCxt->lstWave, hFrameData);
	 
//	printf("t:%f in:%d out:%d\n", t, src_nb_samples, ret);
	//fwrite(dst_data[0], 1, dst_bufsize, dst_file);


	//size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
	//printf("audio_frame n:%d nb_samples:%d pts:%s\n",
	   // audio_frame_count++, frame->nb_samples,
	   // av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

	/* Write the raw audio data samples of the first plane. This works
	 * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
	 * most audio decoders output planar audio, which uses a separate
	 * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
	 * In other words, this code will write only the first audio channel
	 * in these cases.
	 * You should use libswresample or libavfilter to convert the frame
	 * to packed data. */
	 // fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);


	return 0;
}

int output_video_frame(FfDecodeContext* pCxt, MxDescriptor* pVideoDesc, AVFrame* frame)
{
	AVStream*   pStream = pCxt->pVideoStream;

	if (frame->width != pCxt->dstWidth || frame->height != pCxt->dstHeight || frame->format != pCxt->srcFormat)
	{
		if (!UpdateSwsContext(pCxt, frame->width, frame->height, (AVPixelFormat)frame->format, frame->width, frame->height, pCxt->dstPixelFormat, BITMAP_ALIGN_BYTES)) {
			return -1;
		}
		RV_ASSERT(pVideoDesc);
		ResetVideoDescEx((MxVideoDesc*)pVideoDesc, MX_VF_BGR, frame->width, frame->height);
	}

	/* convert to destination format */
	if (NULL == pCxt->pSwsContext) return -1;

	int n = sws_scale(pCxt->pSwsContext, (const uint8_t* const*)frame->data,
		frame->linesize, 0, frame->height, pCxt->dstVideoData, pCxt->dstVideoLinesize);

	if (n <= 0) {
		return -1;
	}

	//生成图像帧数据
	MX_HANDLE hFrameData = mxCreateSample(pVideoDesc, sizeof(MxVideoDesc), pCxt->dstVideoData[0], pCxt->dstVideoBuffSize);
	RV_ASSERT(hFrameData);
	 
	int64_t pts =  frame->pts ; 
	   
	//需要计算帧的显示持续时间
	double frame_duration = av_q2d(pCxt->pVideoCodecContext->time_base);
	frame_duration += frame->repeat_pict * (frame_duration * 0.5);

	// 获取time base（时间基）
	AVRational time_base = pStream->time_base;

	// 转换为秒
	double tb =  av_q2d(time_base);

	mxSetSampleOptions(hFrameData, frame->pts, tb, pCxt->videoSerial++, frame_duration);

	rlsAddTail(pCxt->lstPicture, hFrameData);
	  
	return 0;
}


int decode_packet(FfDecodeContext* pCxt, AVCodecContext* dec, const AVPacket* pkt, AVFrame* frame)
{
	int ret = 0;

	// submit the packet to the decoder
	ret = avcodec_send_packet(dec, pkt);
	if (ret < 0) {
		// fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
		return ret;
	}

	// get all the available frames from the decoder
	while (ret >= 0) {
		ret = avcodec_receive_frame(dec, frame);
		if (ret < 0) {
			// those two return values are special and mean there is no output
			// frame available, but there were no errors during decoding
			if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
				return 0;
			}

			//	 fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
			return ret;
		}

		// write the frame data to output file
		if (dec->codec->type == AVMEDIA_TYPE_VIDEO) {
			ret = output_video_frame(pCxt, (MxDescriptor*)pCxt->pVideoDesc, frame);
		}
		else {
			  ret = output_audio_frame(pCxt,  pCxt->pAudioDesc , frame);
		}


		av_frame_unref(frame);
		if (ret < 0)
			return ret;
	}

	return 0;
}

//
//AudioFifoAdapter* create_audio_fifo_adapter(AVSampleFormat sample_fmt,
//	const AVChannelLayout* ch_layout,
//	int sample_rate) {
//	AudioFifoAdapter* adapter = (AudioFifoAdapter*)av_malloc(sizeof(AudioFifoAdapter));
//	if (!adapter) return NULL;
//
//	adapter->sample_fmt = sample_fmt;
//	adapter->ch_layout = *ch_layout;
//	adapter->sample_rate = sample_rate;
//	adapter->channels = ch_layout->nb_channels;
//	adapter->sample_size = av_get_bytes_per_sample(sample_fmt);
//	adapter->pts_offset = 0;
//
//	adapter->fifo = av_fifo_alloc2(1024 * 16, adapter->sample_size * adapter->channels  /*sizeof(uint8_t*)*/, 0);
//	if (!adapter->fifo) {
//		av_free(adapter);
//		return NULL;
//	}
//
//	
//	 
//	 
// //	OutputDebugString("Created audio FIFO adapter:\n");
////	OutputDebugString("  Sample format: %s\n", av_get_sample_fmt_name(sample_fmt));
////	OutputDebugString("  Channels: %d\n", adapter->channels);
////	OutputDebugString("  Sample size: %d bytes\n", adapter->sample_size);
//
//	return adapter;
//}
//
//void free_audio_fifo_adapter(AudioFifoAdapter** adapter) 
//{
//	if (!adapter || !*adapter) return;
//
//	AudioFifoAdapter* a = *adapter;
//
//	if (a->fifo) {
//		// 清理 FIFO 中剩余的数据
//		uint8_t* data;
//		while (av_fifo_read(a->fifo, &data, 1) >= 0) {
//			av_free(data);
//		}
//		av_fifo_freep2(&a->fifo);
//	}
//
//	av_channel_layout_uninit(&a->ch_layout);
//	av_free(a);
//	*adapter = NULL;
//}
//
//
//int feed_frame_to_adapter(AudioFifoAdapter* adapter, const AVFrame* frame) {
//	if (!adapter || !frame) return AVERROR(EINVAL);
//
//	int samples_to_add = frame->nb_samples;
//	int channels = adapter->channels;
//	int sample_size = adapter->sample_size;
//	int is_planar = av_sample_fmt_is_planar(adapter->sample_fmt);
//
//	// 计算每声道需要的数据大小
//	int data_size_per_channel = samples_to_add * sample_size;
//
//	if (is_planar) {
//		// 平面格式：每个声道单独存储
//		for (int ch = 0; ch < channels; ch++) {
//			uint8_t* channel_data =(uint8_t*) av_malloc(data_size_per_channel);
//			if (!channel_data) return AVERROR(ENOMEM);
//
//			memcpy(channel_data, frame->data[ch], data_size_per_channel);
//
//			if (av_fifo_write(adapter->fifo, channel_data, samples_to_add / channels) < 0) {
//				av_free(channel_data);
//				return AVERROR(ENOMEM);
//			}
//		}
//	}
//	else {
//		// 打包格式：所有声道交错存储
//		int total_size = samples_to_add * channels * sample_size;
//		uint8_t* packed_data = (uint8_t*)av_malloc(total_size);
//		if (!packed_data) return AVERROR(ENOMEM);
//
//		memcpy(packed_data, frame->data[0], total_size);
//
//		if (av_fifo_write(adapter->fifo, packed_data, samples_to_add * channels) < 0) {
//			av_free(packed_data);
//			return AVERROR(ENOMEM);
//		}
//	}
//
//	//printf("Added %d samples to FIFO (total: %zu samples)\n",
//	//	samples_to_add, av_fifo_can_read(adapter->fifo) / channels);
//
//	return 0;
//}
//
//AVFrame* get_frame_from_adapter(AudioFifoAdapter* adapter, int required_samples) {
//	if (!adapter || required_samples <= 0) return NULL;
//
//	int available_samples = av_fifo_can_read(adapter->fifo) /*/ adapter->channels*/;
//	if (available_samples < required_samples) {
//		//printf("Not enough samples in FIFO: %d available, %d required\n",
//		//	available_samples, required_samples);
//		return NULL;
//	}
//
//	AVFrame* frame = av_frame_alloc();
//	if (!frame) return NULL;
//
//	frame->format = adapter->sample_fmt;
//	frame->sample_rate = adapter->sample_rate;
//	frame->ch_layout = adapter->ch_layout;
//	frame->nb_samples = required_samples;
//	frame->pts = adapter->pts_offset;
//
//	if (av_frame_get_buffer(frame, 0) < 0) {
//		av_frame_free(&frame);
//		return NULL;
//	}
//
//	int channels = adapter->channels;
//	int sample_size = adapter->sample_size;
//	int is_planar = av_sample_fmt_is_planar(adapter->sample_fmt);
//
//	if (is_planar) {
//		// 平面格式
//		for (int ch = 0; ch < channels; ch++) {
//			uint8_t* src_data = (uint8_t*)av_malloc(sample_size * required_samples);
//
//			int size = required_samples / channels;
//			if (av_fifo_read(adapter->fifo, src_data, required_samples) < 0) {
//				av_frame_free(&frame);
//				return NULL;
//			}
//			 
//			memcpy(frame->data[ch], src_data, size);
//			av_free(src_data);
//		}
//	}
//	else {
//		int copy_size = required_samples * channels * sample_size;
//		
//		// 打包格式
//		uint8_t* src_data;
//		if (av_fifo_read(adapter->fifo, &src_data, required_samples * channels) < 0) {
//			av_frame_free(&frame);
//			return NULL;
//		}
//
//		memcpy(frame->data[0], src_data, copy_size);
//		av_free(src_data);
//	}
//
//	adapter->pts_offset += required_samples;
//
//	//printf("Retrieved %d samples from FIFO (%zu samples remaining)\n",
//	//	required_samples, av_fifo_can_read(adapter->fifo) / channels);
//
//	return frame;
//}

/**
 * 从 FIFO 读取打包格式数据
 */
int read_packed_data_from_fifo(AVFifo* fifo, AVFrame* frame, int samples_to_read) {
	if (!fifo || !frame) return AVERROR(EINVAL);

	AVSampleFormat fmt = (AVSampleFormat)frame->format;
	int channels = frame->ch_layout.nb_channels;
	int bytes_per_sample = av_get_bytes_per_sample(fmt);
	int read_size = samples_to_read * channels * bytes_per_sample;

	uint8_t* src_data;

	// 从 FIFO 读取打包数据
	if (av_fifo_read(fifo, &src_data, 1) < 0) {
		//fprintf(stderr, "Failed to read packed data from FIFO\n");
		return AVERROR(EIO);
	}

	// 复制数据到帧
	if (frame->data[0]) {
		memcpy(frame->data[0], src_data, read_size);
	}

	// 释放 FIFO 中的数据
	av_free(src_data);

	//printf("Read %d samples (packed) from FIFO\n", samples_to_read);
	return 0;
}
/**
 * 从 FIFO 读取平面格式数据
 */
int read_planar_data_from_fifo(AVFifo* fifo, AVFrame* frame, int samples_to_read) {
	if (!fifo || !frame) return AVERROR(EINVAL);

	AVSampleFormat fmt =(AVSampleFormat) frame->format;
	int channels = frame->ch_layout.nb_channels;
	int bytes_per_sample = av_get_bytes_per_sample(fmt);
	int read_size = samples_to_read * bytes_per_sample;

	for (int ch = 0; ch < channels; ch++) {
		uint8_t* src_data;

		// 从 FIFO 读取一个声道的数据
		if (av_fifo_read(fifo, &src_data, 1) < 0) {
			//fprintf(stderr, "Failed to read channel %d from FIFO\n", ch);
			return AVERROR(EIO);
		}

		// 复制数据到帧
		if (frame->data[ch]) {
			memcpy(frame->data[ch], src_data, read_size);
		}

		// 释放 FIFO 中的数据
		av_free(src_data);
	}

	printf("Read %d samples (planar) from FIFO\n", samples_to_read);
	return 0;
}
/**
 * 从 FIFO 读取数据创建指定长度的 AVFrame
 * @param fifo 音频 FIFO
 * @param sample_fmt 样本格式
 * @param ch_layout 声道布局
 * @param sample_rate 采样率
 * @param target_samples 目标样本数
 * @param fill_silence 是否用静音填充不足的部分
 * @return 创建的 AVFrame，需要调用者释放
 */
//AVFrame* create_frame_from_fifo(AudioFifoAdapter* pAudioAdapter,
//	int target_samples,
//	BOOL fill_silence)
//{
//	AVFifo* fifo = pAudioAdapter->fifo;
//	AVSampleFormat sample_fmt = pAudioAdapter->sample_fmt;	
//	const AVChannelLayout* ch_layout = &pAudioAdapter->ch_layout;
//	int sample_rate = pAudioAdapter->sample_rate;
//
//
//	if (!fifo || target_samples <= 0) {
//		return NULL;
//	}
//
//	int channels = ch_layout->nb_channels;
//	int bytes_per_sample = av_get_bytes_per_sample(sample_fmt);
//	int is_planar = av_sample_fmt_is_planar(sample_fmt);
//
//	// 计算 FIFO 中可用的样本数
//	size_t fifo_size = av_fifo_can_read(fifo);
//	int available_samples;
//
//	if (is_planar) {
//		// 平面格式：每个声道单独存储
//		available_samples = fifo_size / channels;
//	}
//	else {
//		// 打包格式：所有声道交错存储
//		available_samples = fifo_size / (channels * bytes_per_sample);
//	}
//
//	//printf("FIFO status: %zu bytes, %d samples available, %d samples requested\n",
//	//	fifo_size, available_samples, target_samples);
//
//	if (available_samples < target_samples && !fill_silence) {
//		//printf("Not enough data in FIFO and silence filling disabled\n");
//		return NULL;
//	}
//
//	// 创建输出帧
//	AVFrame* frame = av_frame_alloc();
//	if (!frame) {
//		return NULL;
//	}
//
//	frame->format = sample_fmt;
//	frame->sample_rate = sample_rate;
//	frame->ch_layout = *ch_layout;
//	frame->nb_samples = target_samples;
//	frame->pts = pAudioAdapter->pts_offset;
//
//	if (av_frame_get_buffer(frame, 0) < 0) {
//		av_frame_free(&frame);
//		return NULL;
//	}
//
//	// 初始化帧数据为静音（如果需要填充）
//	if (fill_silence && available_samples < target_samples) {
//		init_frame_with_silence(frame);
//	}
//
//	// 从 FIFO 读取数据填充帧
//	int samples_to_read = (available_samples < target_samples) ?
//		available_samples : target_samples;
//
//	if (samples_to_read > 0) {
//		if (is_planar) {
//			read_planar_data_from_fifo(fifo, frame, samples_to_read);
//		}
//		else {
//			read_packed_data_from_fifo(fifo, frame, samples_to_read);
//		}
//	}
//
//	pAudioAdapter->pts_offset += target_samples;
////	printf("Created frame with %d samples (%d from FIFO, %d silence)\n",
//	//	target_samples, samples_to_read, target_samples - samples_to_read);
//
//	return frame;
//}

/**
 * 用静音值初始化帧
 */
void init_frame_with_silence(AVFrame* frame)
{
	if (!frame || !frame->data[0]) return;

	AVSampleFormat fmt = (AVSampleFormat)frame->format;
	int channels = frame->ch_layout.nb_channels;
	int samples = frame->nb_samples;
	int is_planar = av_sample_fmt_is_planar(fmt);

	if (is_planar) {
		// 平面格式
		for (int ch = 0; ch < channels; ch++) {
			if (frame->data[ch]) {
				set_silence(frame->data[ch], samples, fmt);
			}
		}
	}
	else {
		// 打包格式
		if (frame->data[0]) {
			set_silence(frame->data[0], samples * channels, fmt);
		}
	}
}

/**
 * 设置静音数据
 */
void set_silence(void* data, int samples, AVSampleFormat fmt) 
{
	switch (fmt) {
	case AV_SAMPLE_FMT_U8:
	case AV_SAMPLE_FMT_U8P:
		memset(data, 0x80, samples);  // 无符号8位静音值是128
		break;
	case AV_SAMPLE_FMT_S16:
	case AV_SAMPLE_FMT_S16P:
		memset(data, 0, samples * 2);  // 16位静音值是0
		break;
	case AV_SAMPLE_FMT_S32:
	case AV_SAMPLE_FMT_S32P:
	case AV_SAMPLE_FMT_FLT:
	case AV_SAMPLE_FMT_FLTP:
		memset(data, 0, samples * 4);  // 32位静音值是0
		break;
	case AV_SAMPLE_FMT_DBL:
	case AV_SAMPLE_FMT_DBLP:
		memset(data, 0, samples * 8);  // 64位静音值是0
		break;
	default:
		memset(data, 0, samples * 4);  // 默认按32位处理
		break;
	}
}
//
//AudioEncoderWithFifo* create_audio_encoder_with_fifo(AVCodecContext* enc_ctx) 
//{
//	AudioEncoderWithFifo* encoder =(AudioEncoderWithFifo*) av_malloc(sizeof(AudioEncoderWithFifo));
//	if (!encoder) return NULL;
//
//	encoder->enc_ctx = enc_ctx;
//	encoder->frames_encoded = 0;
//
//	encoder->fifo_adapter = create_audio_fifo_adapter(
//		enc_ctx->sample_fmt,
//		&enc_ctx->ch_layout,
//		enc_ctx->sample_rate
//	);
//
//	if (!encoder->fifo_adapter) {
//		av_free(encoder);
//		return NULL;
//	}
//
//	return encoder;
//}
//
//int encode_audio_with_frame_adjustment(AudioEncoderWithFifo* encoder,
//	const AVFrame* input_frame,
//	AVPacket* output_packet, 
//	void* user_data) 
//{
//	if (!encoder || !encoder->fifo_adapter) {
//		return AVERROR(EINVAL);
//	}
//
//	FfEncodeContext* pCxt = (FfEncodeContext*)user_data;
//
//	int ret = 0;
//
//	BOOL bEncodeRemain = (input_frame == NULL);
//
//	// 1. 将输入帧添加到 FIFO
//	if (input_frame) {
//		ret = feed_frame_to_adapter(encoder->fifo_adapter, input_frame);
//		if (ret < 0) return ret;
//	}
//
//	// 2. 从 FIFO 中获取符合编码器要求的帧并编码
//	int required_samples = encoder->enc_ctx->frame_size;
//
//	while (TRUE) {
//		AVFrame* encoded_frame = NULL;
//		
//		if (bEncodeRemain) {
//			encoded_frame = create_frame_from_fifo(encoder->fifo_adapter, required_samples , TRUE);
//		}
//		else {
//			encoded_frame = get_frame_from_adapter(encoder->fifo_adapter, required_samples);
//		}
//
//		if (!encoded_frame) {
//			// 没有足够的数据了
//			//break;
//			return 0;
//		}
//		
//		ret = write_frame(pCxt->pFmtCxt, encoder->enc_ctx, pCxt->audioStream.st, encoded_frame, output_packet);
//	//	// 3. 编码帧
//	//	ret = avcodec_send_frame(encoder->enc_ctx, encoded_frame);
//	//	if (ret < 0) {
//	//		//fprintf(stderr, "Error sending frame: %s\n", av_err2str(ret));
//	//		av_frame_free(&encoded_frame);
//	//		return ret;
//	//	}
//
//	//	// 4. 接收编码包
//	//	while (ret >= 0) {
//	//		ret = avcodec_receive_packet(encoder->enc_ctx, output_packet);
//	//		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
//	//			break;
//	//		}
//	//		else if (ret < 0) {
//	//			//fprintf(stderr, "Error during encoding: %s\n", av_err2str(ret));
//	//			break;
//	//		}
//
//	//		// 5. 处理编码后的包
//	//		if (write_packet_cb) {
//	//			write_packet_cb(output_packet, user_data);
//	//		}
//
//	//		encoder->frames_encoded++;
//	//		av_packet_unref(output_packet);
//	//	}
//
//		av_frame_free(&encoded_frame);
//	}
//
//	return 0;
//}
//
//int flush_encoder_with_fifo(AudioEncoderWithFifo* encoder,	AVPacket* output_packet,  	void* user_data) 
//{
//	if (!encoder) return AVERROR(EINVAL);
//
//	//printf("Flushing encoder with remaining samples...\n");
//
//	// 1. 先处理 FIFO 中剩余的数据
//	int remaining_samples = av_fifo_can_read(encoder->fifo_adapter->fifo) /
//		encoder->fifo_adapter->channels;
//
//	FfEncodeContext* pCxt = (FfEncodeContext*)user_data;
//
//	int ret = 0;
//	if (remaining_samples > 0) {
//		//printf("Encoding remaining %d samples from FIFO\n", remaining_samples);
//
//		//// 对于剩余数据，可能需要填充静音来满足帧大小要求
//		//int required_samples = encoder->enc_ctx->frame_size;
//
//		//if (remaining_samples < required_samples) {
//		//	//printf("Adding %d samples of silence to complete frame\n",
//		//	//	required_samples - remaining_samples);
//		//	// 这里可以添加静音填充逻辑
//
//		//}
//
//		// 编码剩余数据
//		ret = encode_audio_with_frame_adjustment(encoder, NULL, output_packet, user_data);
//		if (ret < 0) {
//			return ret;
//		}
//	}
//
//	ret = write_frame(pCxt->pFmtCxt, encoder->enc_ctx, pCxt->audioStream.st, NULL, output_packet);
//	return ret;
//	//// 2. 刷新编码器
//	//avcodec_send_frame(encoder->enc_ctx, NULL);
//
//	//int ret;
//	//while ((ret = avcodec_receive_packet(encoder->enc_ctx, output_packet)) == 0) {
//	//	if (write_packet_cb) {
//	//		write_packet_cb(output_packet, user_data);
//	//	}
//	//	av_packet_unref(output_packet);
//	//}
//
//	////printf("Encoder flush completed. Total frames encoded: %d\n",
//	////	encoder->frames_encoded);
//
//	return 0;
//}
//
//void destroy_audio_encoder_with_fifo(AudioEncoderWithFifo* pEncoder)
// { 
//	if (!pEncoder) return ;
//
//	if (pEncoder->fifo_adapter) {
//		free_audio_fifo_adapter(&pEncoder->fifo_adapter);		 
//	}
//	  
//	av_free(pEncoder);
//
//}

BOOL IsFrameOverSize(AVCodecContext* enc_ctx/*, const AVFrame* frame*/)
{
	//printf("\n=== Frame Size Diagnosis ===\n");
	//printf("Encoder frame_size: %d\n", enc_ctx->frame_size);
	//printf("Frame nb_samples: %d\n", frame->nb_samples);
	//printf("Sample rate: %d\n", enc_ctx->sample_rate);
	//printf("Channels: %d\n", enc_ctx->ch_layout.nb_channels);

	if (enc_ctx->frame_size <= 0) {
		return FALSE;//printf("Encoder supports variable frame sizes\n");
	}
	//else if (frame->nb_samples == enc_ctx->frame_size) {
	//	return FALSE; // printf("✓ Frame size matches encoder requirements\n");
	//}
	else {
		//printf("✗ Frame size mismatch!\n");
		//printf("Difference: %d samples\n", frame->nb_samples - enc_ctx->frame_size);

		// 特定编码器建议
		switch (enc_ctx->codec_id) {
		case AV_CODEC_ID_AAC:
			return TRUE;//printf("AAC requires exact frame size of %d samples\n", enc_ctx->frame_size);
			break;
		case AV_CODEC_ID_MP3:
			return TRUE; // printf("MP3 typically uses 1152 samples per frame\n");
			break;
		case AV_CODEC_ID_AC3:
			return TRUE;//printf("AC3 requires multiples of 256 samples\n");
			break;
		case AV_CODEC_ID_OPUS:
			return TRUE; // printf("Opus supports 2.5, 5, 10, 20, 40, 60 ms frames\n");
			break;
		default:
			return FALSE;//printf("Check codec-specific frame size requirements\n");
			break;
		}
	}
//	printf("=============================\n");
}



int write_frame(AVFormatContext* fmt_ctx, AVCodecContext* c, AVStream* st, AVFrame* frame, AVPacket* pkt)
{
	int ret;

	if (frame != NULL) {
		//validate_parameters_before_send(c, frame);
	 }
	// 
		// send the frame to the encoder
	ret = avcodec_send_frame(c, frame);
	if (ret < 0) {
		//char buff[1205] = {0};
		//av_make_error_string(buff, sizeof(buff), ret);
  
		//PrintToOutput("Error sending a frame to the encoder: %s\n", (const char*)buff);

		if (AVERROR_EOF == ret) {
			return 1;
		}
		return ret;
	}
	 
	while (ret >= 0) {
		ret = avcodec_receive_packet(c, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			break;
		else if (ret < 0) {
			//fprintf(stderr, "Error encoding a frame: %s\n", av_err2str(ret));
			//exit(1);
			return ret;
		}

		/* rescale output packet timestamp values from codec to stream timebase */
		av_packet_rescale_ts(pkt, c->time_base, st->time_base);
		pkt->stream_index = st->index;

		/* Write the compressed frame to the media file. */
		//log_packet(fmt_ctx, pkt);

		ret = av_interleaved_write_frame(fmt_ctx, pkt);
		/* pkt is now blank (av_interleaved_write_frame() takes ownership of
		 * its contents and resets pkt), so that no unreferencing is necessary.
		 * This would be different if one used av_write_frame(). */
		if (ret < 0) {
			//fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(ret));
			//exit(1);
		//	av_packet_unref(pkt);
			return ret;
		}

		//av_packet_unref(pkt);
	}

	return ret == AVERROR_EOF ? 1 : 0;
}

int modify_audio_frame_size(AVFrame* frame, int new_nb_samples) 
{
	int ret;

	// 取消引用所有缓冲区
	av_frame_unref(frame);

	// 更新样本数量
	frame->nb_samples = new_nb_samples;

	// 重新分配缓冲区
	ret = av_frame_get_buffer(frame, 1);
	if (ret < 0) {
		return ret;
	}

	return 0;
}
/*
 *@return 帧的时长（秒）
 */ 
double CalculateAudioFrameDuration(const AVFrame * frame, int sampleRate) 
{
	if (!frame || sampleRate <= 0) {
		return 0.0;
	}

	// 获取帧中的样本数量
	int nb_samples = frame->nb_samples;
	if (nb_samples <= 0) {
		return 0.0;
	}

	// 计算时长：样本数 / 采样率
	double duration = ((double)nb_samples) / sampleRate;
	return duration;
}