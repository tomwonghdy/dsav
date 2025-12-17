/////////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2025 Tom Wong  
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
//////////////////////////////////////////////////////////////////////////////////////

#if !defined(__AVLIB_H_INCLUDED_)
#define __AVLIB_H_INCLUDED_

 
#include "ffmo.h"

#include <float.h>

#include "..\davsdk\includes\mox.h"
#include "..\davsdk\includes\mediaobject.h"

#include <stdint.h>

#define __STDC_LIMIT_MACROS 


typedef struct __ff_decode_context_
{
    AVFormatContext* pFormatContext  ;          // ffmpeg的全局上下文，所有ffmpeg操作都需要
    int              videoIndex, audioIndex;
    AVStream*          pVideoStream ;            // ffmpeg流信息
    AVStream*          pAudioStream;              // ffmpeg流信息
    AVCodecContext*    pVideoCodecContext ;        // ffmpeg编码上下文
    AVCodecContext*    pAudioCodecContext;         // ffmpeg编码上下文 
    struct SwsContext* pSwsContext ;               // ffmpeg图像编码数据格式转换
    struct SwrContext* pSwrCxt;                   //音频转换
    AVDictionary*      options  ;                 //参数

    MxVideoDesc* pVideoDesc;
    MxAudioDesc* pAudioDesc;

    int64_t     audioSerial;
    int64_t     videoSerial;
     
    //目标视频数据
    uint8_t*      dstVideoData[4] ;
    int           dstVideoLinesize[4];
    int           dstVideoBuffSize;
    AVPixelFormat srcFormat;
    AVPixelFormat dstPixelFormat;
    int           dstWidth, dstHeight;

    //目标音频数据
    int            max_dst_nb_samples;
    uint8_t**      dst_data;
    AVChannelLayout src_ch_layout , dst_ch_layout ;
    int src_rate, dst_rate;
    int dst_line_size;
    enum AVSampleFormat src_sample_fmt , dst_sample_fmt ;
    

    //BOOL   endOfStream;                            //是否已经没有数据
    RvList lstPicture;                             //图像列表
    RvList lstWave;                                //语音列表
}FfDecodeContext;




#define SCALE_FLAGS SWS_BILINEAR

  
 
// a wrapper around a single output AVStream
typedef struct _ff_output_stream_ {
    AVStream*       stream;
    AVCodecContext* encode;
    AVPacket*       packet;

    //video scale
    struct SwsContext* sws_ctx;
    uint8_t*      srcVideoData[4];
    int           srcVideoLinesize[4];
    int           srcVideoBuffSize;
    int           srcHeight, srcWidth;
    AVPixelFormat srcPixelFormat;

    //audio resample
    AVAudioFifo*       fifo;
    struct SwrContext* swr_ctx;
    AVSampleFormat     srcSampleFormat;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int     samples_count;

    AVFrame* frame; 
    //AVFrame* tmp_frame;

    int64_t serial; //序列号，可用于PTS
   
} FfOutputStream;


// a wrapper around a single output AVStream
typedef struct _ff_encode_context_ {
    AVFormatContext* pFmtCxt; 
    BOOL bHaveVideo, bHaveAudio;
    BOOL bSaveToDisk;
    BOOL bFileOpened;

   // int  encodeType;
    const AVCodec* audioCodec, * videoCodec;
    const AVOutputFormat* pOutFmt;
    AVDictionary* pOption;


    
    FfOutputStream audioStream;
    FfOutputStream videoStream;

    //BOOL  bArrangeAudioFrame;
    //AudioEncoderWithFifo* pAudioEncoder;  //解码支持FIFO

}FfEncodeContext;

 
//
///* maximum audio speed change to get correct sync */
 #define SAMPLE_CORRECTION_PERCENT_MAX 10
//
   

#define BITMAP_ALIGN_BYTES   4


//decode 
FfDecodeContext* InitDecodeContext(MxDescriptor* pVideoDesc, MxDescriptor* pAudioDesc);
void       ReleaseContext(FfDecodeContext* pCxt);
BOOL       UpdateSwsContext(FfDecodeContext* pCxt, int srcWid, int srcHei, AVPixelFormat srcFmt, int dstWid, int dstHei, AVPixelFormat dstFmt, int dstBufferAlign);


//encode
FfEncodeContext* InitEncodeContext();
void ReleaseEncodeContext(FfEncodeContext* pCxt);


int output_video_frame(FfDecodeContext* pCxt, MxDescriptor* pVideoDesc, AVFrame* frame);
int decode_packet(  FfDecodeContext* pCxt, AVCodecContext* dec, const AVPacket* pkt, AVFrame* frame);
 
int GetDsavChannelLayout(AVChannelLayout layout);
    
BOOL InitFFmpeg();

void CleanupFFmpeg();


AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,AVFormatContext *s, AVStream *st, AVCodec *codec);

AVDictionary **setup_find_stream_info_opts(AVFormatContext *s, AVDictionary *codec_opts);

AVChannelLayout GetFfmpegLayout(int layout);
 // VideoPicture* WINAPI CreateVideoPicture(int width, int height, int depth);
 //void WINAPI DestroyVideoPicture(void*  pVideoPicture );



//位置单位:position 秒
//timeBase of stream

M_RESULT  SeekMediaPos(AVFormatContext * pFmtCxt, int streamIndex, AVRational timeBase, double position);

int write_frame(AVFormatContext* fmt_ctx, AVCodecContext* c, AVStream* st, AVFrame* frame, AVPacket* pkt);

BOOL IsFrameOverSize(AVCodecContext* enc_ctx/*, const AVFrame* frame*/);

//
//AudioEncoderWithFifo* create_audio_encoder_with_fifo(AVCodecContext* enc_ctx);
//void destroy_audio_encoder_with_fifo(AudioEncoderWithFifo* pEncoder);
//
//int encode_audio_with_frame_adjustment(AudioEncoderWithFifo* encoder,
//    const AVFrame* input_frame,
//    AVPacket* output_packet,
//    void* user_data);
//
//int flush_encoder_with_fifo(AudioEncoderWithFifo* encoder,    AVPacket* output_packet,  void* user_data);

/**
 * 设置静音数据
 */
void set_silence(void* data, int samples, AVSampleFormat fmt);
/**
 * 用静音值初始化帧
 */
void init_frame_with_silence(AVFrame* frame);
//
//AVFrame* create_frame_from_fifo(AudioFifoAdapter* pAudioAdapter, 
//    int target_samples,
//    BOOL fill_silence);

int modify_audio_frame_size(AVFrame* frame, int new_nb_samples);

int pcm_to_mp3(const char* pcm_file_path, const char* mp3_file_path);

double CalculateAudioFrameDuration(const AVFrame* frame, int sampleRate);


#endif // !defined(AFX_AVLIB_H__237A52DA_4C5E_4096_A494_60782261772D__INCLUDED_)
