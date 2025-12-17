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
#ifndef  __FFMO_H_INCLUDED_
#define  __FFMO_H_INCLUDED_


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FFMO_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FFMO_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef FFMO_EXPORTS
#define FFMO_API __declspec(dllexport)
#else
#define FFMO_API __declspec(dllimport)
#endif
 
#include "..\davsdk\includes\mox.h"
#include "..\davsdk\includes\mediaobject.h"

#include <stdint.h>

#define FF_STREAM_AUDIO      MX_MT_AUDIO
#define FF_STREAM_VIDEO      MX_MT_VIDEO
#define FF_STREAM_SUBTITLE   MX_MT_SUBTITLE

#define FF_STREAM_ALL        (FF_STREAM_AUDIO | FF_STREAM_VIDEO | FF_STREAM_SUBTITLE)

#define FF_MAX_STREAM_COUNT    3

//变量名称
#define FF_MAX_VIDEO_FRAME_DURATION   0
#define FF_VIDEO_ALIVE                1
#define FF_WAVE_ALIVE                2
#define FF_SUBTITLE_ALIVE             3
#define FF_VIDEO_FRAME_RATE           10
#define FF_WAVE_FRAME_RATE           11




#ifndef INT64_MIN
#define INT64_MIN       (-0x7fffffffffffffffLL-1)
#endif

#ifndef INT64_MAX
#define INT64_MAX INT64_C(9223372036854775807)
#endif


/* SDL audio buffer size, in samples. Should be small to have precise
A/V sync as SDL does not have hardware buffer fullness info. */
#define AV_AUDIO_BUFFER_SIZE    1204
#define AV_DEFAULT_SAMPLE_RATE  44100
#define AV_DEFAULT_CHANNELS     2


//MX_AF_S16 defined in mox.h, its value is identical with audiot sample
//formats defined in FFMPEG
#define AV_DEFAULT_AUDIO_FORMAT   MX_AF_S16

#define AV_DEFAULT_AUDIO_BITRATE   (64000*3)
#define AV_DEFAULT_VIDEO_BITRATE   400000

#define AV_DEFAULT_VIDEO_WIDTH     640
#define AV_DEFAULT_VIDEO_HEIGHT    480
#define AV_DEFAULT_VIDEO_FORMAT    MX_VF_BGR


//视频时间戳计算方法
#define AV_PTS_AUTO        0  //自动计算时间戳
#define AV_PTS_REORDER     1  //重新计算时间戳
#define AV_PTS_DEFAULT    -1   //默认计算时间戳



#define ALPHA_BLEND(a, oldp, newp, s)\
((((oldp << s) * (255 - (a))) + (newp * (a))) / (255 << s))

#define RGBA_IN(r, g, b, a, s)\
{\
    unsigned int v = ((const uint32_t *)(s))[0];\
    a = (v >> 24) & 0xff;\
    r = (v >> 16) & 0xff;\
    g = (v >> 8) & 0xff;\
    b = v & 0xff;\
}

#define YUVA_IN(y, u, v, a, s, pal)\
{\
    unsigned int val = ((const uint32_t *)(pal))[*(const uint8_t*)(s)];\
    a = (val >> 24) & 0xff;\
    y = (val >> 16) & 0xff;\
    u = (val >> 8) & 0xff;\
    v = val & 0xff;\
}

#define YUVA_OUT(d, y, u, v, a)\
{\
    ((uint32_t *)(d))[0] = (a << 24) | (y << 16) | (u << 8) | v;\
}



/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20


////视频时间戳计算方法
//#define FF_PTS_AUTO        0  //自动计算时间戳
//#define FF_PTS_REORDER     1  //重新计算时间戳
//#define FF_PTS_DEFAULT    -1   //默认计算时间戳

//播放到了结尾
#define FF_SOURCE_EOF      0

//#define __STDC_CONSTANT_MACROS

//extern "C" {
//#include <libavutil/avassert.h>
//#include <libavutil/channel_layout.h>
//#include <libavutil/opt.h>
//#include <libavutil/mathematics.h>
//#include <libavutil/timestamp.h>
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>
//#include <libswresample/swresample.h>
//}

//必须与FFMPEG定义的AVSampleFormat一致
enum FF_WAVE_FORMAT {
    FF_WAVE_FMT_NONE = -1,
    FF_WAVE_FMT_U8,          ///< unsigned 8 bits
    FF_WAVE_FMT_S16,         ///< signed 16 bits
    FF_WAVE_FMT_S32,         ///< signed 32 bits
    FF_WAVE_FMT_FLT,         ///< float
    FF_WAVE_FMT_DBL,         ///< double

    FF_WAVE_FMT_U8P,         ///< unsigned 8 bits, planar
    FF_WAVE_FMT_S16P,        ///< signed 16 bits, planar
    FF_WAVE_FMT_S32P,        ///< signed 32 bits, planar
    FF_WAVE_FMT_FLTP,        ///< float, planar
    FF_WAVE_FMT_DBLP,        ///< double, planar
    FF_WAVE_FMT_S64,         ///< signed 64 bits
    FF_WAVE_FMT_S64P,        ///< signed 64 bits, planar

    FF_WAVE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};
 

#ifndef FF_API_VAAPI
#define FF_API_VAAPI                    (LIBAVUTIL_VERSION_MAJOR < 57)
#endif

//必须与FFMPEG定义的AVPixelFormat一致
enum FF_PIXEL_FORMAT {
    FF_PIXEL_FMT_NONE = -1,
    FF_PIXEL_FMT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    FF_PIXEL_FMT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    FF_PIXEL_FMT_RGB24,     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    FF_PIXEL_FMT_BGR24,     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    FF_PIXEL_FMT_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    FF_PIXEL_FMT_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    FF_PIXEL_FMT_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    FF_PIXEL_FMT_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    FF_PIXEL_FMT_GRAY8,     ///<        Y        ,  8bpp
    FF_PIXEL_FMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    FF_PIXEL_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    FF_PIXEL_FMT_PAL8,      ///< 8 bits with FF_PIXEL_FMT_RGB32 palette
    FF_PIXEL_FMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of FF_PIXEL_FMT_YUV420P and setting color_range
    FF_PIXEL_FMT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of FF_PIXEL_FMT_YUV422P and setting color_range
    FF_PIXEL_FMT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of FF_PIXEL_FMT_YUV444P and setting color_range
    FF_PIXEL_FMT_UYVY422,   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    FF_PIXEL_FMT_UYYVYY411, ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    FF_PIXEL_FMT_BGR8,      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    FF_PIXEL_FMT_BGR4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    FF_PIXEL_FMT_BGR4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    FF_PIXEL_FMT_RGB8,      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    FF_PIXEL_FMT_RGB4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    FF_PIXEL_FMT_RGB4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    FF_PIXEL_FMT_NV12,      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    FF_PIXEL_FMT_NV21,      ///< as above, but U and V bytes are swapped

    FF_PIXEL_FMT_ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    FF_PIXEL_FMT_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    FF_PIXEL_FMT_ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    FF_PIXEL_FMT_BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...

    FF_PIXEL_FMT_GRAY16BE,  ///<        Y        , 16bpp, big-endian
    FF_PIXEL_FMT_GRAY16LE,  ///<        Y        , 16bpp, little-endian
    FF_PIXEL_FMT_YUV440P,   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    FF_PIXEL_FMT_YUVJ440P,  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of FF_PIXEL_FMT_YUV440P and setting color_range
    FF_PIXEL_FMT_YUVA420P,  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    FF_PIXEL_FMT_RGB48BE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    FF_PIXEL_FMT_RGB48LE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian

    FF_PIXEL_FMT_RGB565BE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    FF_PIXEL_FMT_RGB565LE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    FF_PIXEL_FMT_RGB555BE,  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined
    FF_PIXEL_FMT_RGB555LE,  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined

    FF_PIXEL_FMT_BGR565BE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    FF_PIXEL_FMT_BGR565LE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    FF_PIXEL_FMT_BGR555BE,  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined
    FF_PIXEL_FMT_BGR555LE,  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined

#if FF_API_VAAPI
    /** @name Deprecated pixel formats */
    /**@{*/
    FF_PIXEL_FMT_VAAPI_MOCO, ///< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
    FF_PIXEL_FMT_VAAPI_IDCT, ///< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
    FF_PIXEL_FMT_VAAPI_VLD,  ///< HW decoding through VA API, Picture.data[3] contains a VASurfaceID
    /**@}*/
    FF_PIXEL_FMT_VAAPI = FF_PIXEL_FMT_VAAPI_VLD,
#else
    /**
     *  Hardware acceleration through VA-API, data[3] contains a
     *  VASurfaceID.
     */
     FF_PIXEL_FMT_VAAPI,
#endif

     FF_PIXEL_FMT_YUV420P16LE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
     FF_PIXEL_FMT_YUV420P16BE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
     FF_PIXEL_FMT_YUV422P16LE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
     FF_PIXEL_FMT_YUV422P16BE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
     FF_PIXEL_FMT_YUV444P16LE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
     FF_PIXEL_FMT_YUV444P16BE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
     FF_PIXEL_FMT_DXVA2_VLD,    ///< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer

     FF_PIXEL_FMT_RGB444LE,  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined
     FF_PIXEL_FMT_RGB444BE,  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined
     FF_PIXEL_FMT_BGR444LE,  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined
     FF_PIXEL_FMT_BGR444BE,  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined
     FF_PIXEL_FMT_YA8,       ///< 8 bits gray, 8 bits alpha

     FF_PIXEL_FMT_Y400A = FF_PIXEL_FMT_YA8, ///< alias for FF_PIXEL_FMT_YA8
     FF_PIXEL_FMT_GRAY8A = FF_PIXEL_FMT_YA8, ///< alias for FF_PIXEL_FMT_YA8

     FF_PIXEL_FMT_BGR48BE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
     FF_PIXEL_FMT_BGR48LE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian

     /**
      * The following 12 formats have the disadvantage of needing 1 format for each bit depth.
      * Notice that each 9/10 bits sample is stored in 16 bits with extra padding.
      * If you want to support multiple bit depths, then using FF_PIXEL_FMT_YUV420P16* with the bpp stored separately is better.
      */
      FF_PIXEL_FMT_YUV420P9BE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
      FF_PIXEL_FMT_YUV420P9LE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
      FF_PIXEL_FMT_YUV420P10BE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
      FF_PIXEL_FMT_YUV420P10LE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
      FF_PIXEL_FMT_YUV422P10BE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
      FF_PIXEL_FMT_YUV422P10LE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
      FF_PIXEL_FMT_YUV444P9BE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
      FF_PIXEL_FMT_YUV444P9LE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
      FF_PIXEL_FMT_YUV444P10BE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
      FF_PIXEL_FMT_YUV444P10LE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
      FF_PIXEL_FMT_YUV422P9BE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
      FF_PIXEL_FMT_YUV422P9LE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
      FF_PIXEL_FMT_GBRP,      ///< planar GBR 4:4:4 24bpp
      FF_PIXEL_FMT_GBR24P = FF_PIXEL_FMT_GBRP, // alias for #FF_PIXEL_FMT_GBRP
      FF_PIXEL_FMT_GBRP9BE,   ///< planar GBR 4:4:4 27bpp, big-endian
      FF_PIXEL_FMT_GBRP9LE,   ///< planar GBR 4:4:4 27bpp, little-endian
      FF_PIXEL_FMT_GBRP10BE,  ///< planar GBR 4:4:4 30bpp, big-endian
      FF_PIXEL_FMT_GBRP10LE,  ///< planar GBR 4:4:4 30bpp, little-endian
      FF_PIXEL_FMT_GBRP16BE,  ///< planar GBR 4:4:4 48bpp, big-endian
      FF_PIXEL_FMT_GBRP16LE,  ///< planar GBR 4:4:4 48bpp, little-endian
      FF_PIXEL_FMT_YUVA422P,  ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
      FF_PIXEL_FMT_YUVA444P,  ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
      FF_PIXEL_FMT_YUVA420P9BE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
      FF_PIXEL_FMT_YUVA420P9LE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
      FF_PIXEL_FMT_YUVA422P9BE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
      FF_PIXEL_FMT_YUVA422P9LE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
      FF_PIXEL_FMT_YUVA444P9BE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
      FF_PIXEL_FMT_YUVA444P9LE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
      FF_PIXEL_FMT_YUVA420P10BE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
      FF_PIXEL_FMT_YUVA420P10LE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
      FF_PIXEL_FMT_YUVA422P10BE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
      FF_PIXEL_FMT_YUVA422P10LE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
      FF_PIXEL_FMT_YUVA444P10BE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
      FF_PIXEL_FMT_YUVA444P10LE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
      FF_PIXEL_FMT_YUVA420P16BE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
      FF_PIXEL_FMT_YUVA420P16LE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
      FF_PIXEL_FMT_YUVA422P16BE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
      FF_PIXEL_FMT_YUVA422P16LE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
      FF_PIXEL_FMT_YUVA444P16BE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
      FF_PIXEL_FMT_YUVA444P16LE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)

      FF_PIXEL_FMT_VDPAU,     ///< HW acceleration through VDPAU, Picture.data[3] contains a VdpVideoSurface

      FF_PIXEL_FMT_XYZ12LE,      ///< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as little-endian, the 4 lower bits are set to 0
      FF_PIXEL_FMT_XYZ12BE,      ///< packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as big-endian, the 4 lower bits are set to 0
      FF_PIXEL_FMT_NV16,         ///< interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
      FF_PIXEL_FMT_NV20LE,       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
      FF_PIXEL_FMT_NV20BE,       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian

      FF_PIXEL_FMT_RGBA64BE,     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
      FF_PIXEL_FMT_RGBA64LE,     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
      FF_PIXEL_FMT_BGRA64BE,     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
      FF_PIXEL_FMT_BGRA64LE,     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian

      FF_PIXEL_FMT_YVYU422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb

      FF_PIXEL_FMT_YA16BE,       ///< 16 bits gray, 16 bits alpha (big-endian)
      FF_PIXEL_FMT_YA16LE,       ///< 16 bits gray, 16 bits alpha (little-endian)

      FF_PIXEL_FMT_GBRAP,        ///< planar GBRA 4:4:4:4 32bpp
      FF_PIXEL_FMT_GBRAP16BE,    ///< planar GBRA 4:4:4:4 64bpp, big-endian
      FF_PIXEL_FMT_GBRAP16LE,    ///< planar GBRA 4:4:4:4 64bpp, little-endian
      /**
       *  HW acceleration through QSV, data[3] contains a pointer to the
       *  mfxFrameSurface1 structure.
       */
       FF_PIXEL_FMT_QSV,
       /**
        * HW acceleration though MMAL, data[3] contains a pointer to the
        * MMAL_BUFFER_HEADER_T structure.
        */
        FF_PIXEL_FMT_MMAL,

        FF_PIXEL_FMT_D3D11VA_VLD,  ///< HW decoding through Direct3D11 via old API, Picture.data[3] contains a ID3D11VideoDecoderOutputView pointer

        /**
         * HW acceleration through CUDA. data[i] contain CUdeviceptr pointers
         * exactly as for system memory frames.
         */
         FF_PIXEL_FMT_CUDA,

         FF_PIXEL_FMT_0RGB,        ///< packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
         FF_PIXEL_FMT_RGB0,        ///< packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
         FF_PIXEL_FMT_0BGR,        ///< packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
         FF_PIXEL_FMT_BGR0,        ///< packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined

         FF_PIXEL_FMT_YUV420P12BE, ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
         FF_PIXEL_FMT_YUV420P12LE, ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
         FF_PIXEL_FMT_YUV420P14BE, ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
         FF_PIXEL_FMT_YUV420P14LE, ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
         FF_PIXEL_FMT_YUV422P12BE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
         FF_PIXEL_FMT_YUV422P12LE, ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
         FF_PIXEL_FMT_YUV422P14BE, ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
         FF_PIXEL_FMT_YUV422P14LE, ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
         FF_PIXEL_FMT_YUV444P12BE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
         FF_PIXEL_FMT_YUV444P12LE, ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
         FF_PIXEL_FMT_YUV444P14BE, ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
         FF_PIXEL_FMT_YUV444P14LE, ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
         FF_PIXEL_FMT_GBRP12BE,    ///< planar GBR 4:4:4 36bpp, big-endian
         FF_PIXEL_FMT_GBRP12LE,    ///< planar GBR 4:4:4 36bpp, little-endian
         FF_PIXEL_FMT_GBRP14BE,    ///< planar GBR 4:4:4 42bpp, big-endian
         FF_PIXEL_FMT_GBRP14LE,    ///< planar GBR 4:4:4 42bpp, little-endian
         FF_PIXEL_FMT_YUVJ411P,    ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) full scale (JPEG), deprecated in favor of FF_PIXEL_FMT_YUV411P and setting color_range

         FF_PIXEL_FMT_BAYER_BGGR8,    ///< bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples */
         FF_PIXEL_FMT_BAYER_RGGB8,    ///< bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples */
         FF_PIXEL_FMT_BAYER_GBRG8,    ///< bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples */
         FF_PIXEL_FMT_BAYER_GRBG8,    ///< bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples */
         FF_PIXEL_FMT_BAYER_BGGR16LE, ///< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, little-endian */
         FF_PIXEL_FMT_BAYER_BGGR16BE, ///< bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, big-endian */
         FF_PIXEL_FMT_BAYER_RGGB16LE, ///< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, little-endian */
         FF_PIXEL_FMT_BAYER_RGGB16BE, ///< bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, big-endian */
         FF_PIXEL_FMT_BAYER_GBRG16LE, ///< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, little-endian */
         FF_PIXEL_FMT_BAYER_GBRG16BE, ///< bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, big-endian */
         FF_PIXEL_FMT_BAYER_GRBG16LE, ///< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, little-endian */
         FF_PIXEL_FMT_BAYER_GRBG16BE, ///< bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, big-endian */

         FF_PIXEL_FMT_XVMC,///< XVideo Motion Acceleration via common packet passing

         FF_PIXEL_FMT_YUV440P10LE, ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
         FF_PIXEL_FMT_YUV440P10BE, ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
         FF_PIXEL_FMT_YUV440P12LE, ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
         FF_PIXEL_FMT_YUV440P12BE, ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
         FF_PIXEL_FMT_AYUV64LE,    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
         FF_PIXEL_FMT_AYUV64BE,    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian

         FF_PIXEL_FMT_VIDEOTOOLBOX, ///< hardware decoding through Videotoolbox

         FF_PIXEL_FMT_P010LE, ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian
         FF_PIXEL_FMT_P010BE, ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian

         FF_PIXEL_FMT_GBRAP12BE,  ///< planar GBR 4:4:4:4 48bpp, big-endian
         FF_PIXEL_FMT_GBRAP12LE,  ///< planar GBR 4:4:4:4 48bpp, little-endian

         FF_PIXEL_FMT_GBRAP10BE,  ///< planar GBR 4:4:4:4 40bpp, big-endian
         FF_PIXEL_FMT_GBRAP10LE,  ///< planar GBR 4:4:4:4 40bpp, little-endian

         FF_PIXEL_FMT_MEDIACODEC, ///< hardware decoding through MediaCodec

         FF_PIXEL_FMT_GRAY12BE,   ///<        Y        , 12bpp, big-endian
         FF_PIXEL_FMT_GRAY12LE,   ///<        Y        , 12bpp, little-endian
         FF_PIXEL_FMT_GRAY10BE,   ///<        Y        , 10bpp, big-endian
         FF_PIXEL_FMT_GRAY10LE,   ///<        Y        , 10bpp, little-endian

         FF_PIXEL_FMT_P016LE, ///< like NV12, with 16bpp per component, little-endian
         FF_PIXEL_FMT_P016BE, ///< like NV12, with 16bpp per component, big-endian

         /**
          * Hardware surfaces for Direct3D11.
          *
          * This is preferred over the legacy FF_PIXEL_FMT_D3D11VA_VLD. The new D3D11
          * hwaccel API and filtering support FF_PIXEL_FMT_D3D11 only.
          *
          * data[0] contains a ID3D11Texture2D pointer, and data[1] contains the
          * texture array index of the frame as intptr_t if the ID3D11Texture2D is
          * an array texture (or always 0 if it's a normal texture).
          */
          FF_PIXEL_FMT_D3D11,

          FF_PIXEL_FMT_GRAY9BE,   ///<        Y        , 9bpp, big-endian
          FF_PIXEL_FMT_GRAY9LE,   ///<        Y        , 9bpp, little-endian

          FF_PIXEL_FMT_GBRPF32BE,  ///< IEEE-754 single precision planar GBR 4:4:4,     96bpp, big-endian
          FF_PIXEL_FMT_GBRPF32LE,  ///< IEEE-754 single precision planar GBR 4:4:4,     96bpp, little-endian
          FF_PIXEL_FMT_GBRAPF32BE, ///< IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, big-endian
          FF_PIXEL_FMT_GBRAPF32LE, ///< IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, little-endian

          /**
           * DRM-managed buffers exposed through PRIME buffer sharing.
           *
           * data[0] points to an AVDRMFrameDescriptor.
           */
           FF_PIXEL_FMT_DRM_PRIME,
           /**
            * Hardware surfaces for OpenCL.
            *
            * data[i] contain 2D image objects (typed in C as cl_mem, used
            * in OpenCL as image2d_t) for each plane of the surface.
            */
            FF_PIXEL_FMT_OPENCL,

            FF_PIXEL_FMT_NB         ///< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions
};


//需要与新版本兼容,  FFMPEG V5.1
#define FF_LAYOUT_MONO                0    
#define FF_LAYOUT_STEREO              1
#define FF_LAYOUT_2POINT1             2
#define FF_LAYOUT_2_1                 3
#define FF_LAYOUT_SURROUND            4
#define FF_LAYOUT_3POINT1             5
#define FF_LAYOUT_4POINT0             6
#define FF_LAYOUT_4POINT1             7
#define FF_LAYOUT_2_2                 8
#define FF_LAYOUT_QUAD                9
#define FF_LAYOUT_5POINT0             10
#define FF_LAYOUT_5POINT1             11
#define FF_LAYOUT_5POINT0_BACK        12
#define FF_LAYOUT_5POINT1_BACK        13
#define FF_LAYOUT_6POINT0             14
#define FF_LAYOUT_6POINT0_FRONT       15
#define FF_LAYOUT_HEXAGONAL           16
#define FF_LAYOUT_6POINT1             17
#define FF_LAYOUT_6POINT1_BACK        18
#define FF_LAYOUT_6POINT1_FRONT       19
#define FF_LAYOUT_7POINT0             20
#define FF_LAYOUT_7POINT0_FRONT       21
#define FF_LAYOUT_7POINT1             22
#define FF_LAYOUT_7POINT1_WIDE        23
#define FF_LAYOUT_7POINT1_WIDE_BACK   24
#define FF_LAYOUT_OCTAGONAL           25
#define FF_LAYOUT_HEXADECAGONAL       26
#define FF_LAYOUT_STEREO_DOWNMIX      27
#define FF_LAYOUT_22POINT2            28
#define FF_LAYOUT_AMBISONIC_FIRST_ORDER  29 




FFMO_API   void ResetAudioDescEx(MxAudioDesc* pDesc, int format, int sampleRate, int layout, int frameCount = AV_AUDIO_BUFFER_SIZE, int bitRate = AV_DEFAULT_AUDIO_BITRATE);

FFMO_API   void ResetVideoDescEx(MxVideoDesc* pDesc, int format, int width, int height, int bitRate = AV_DEFAULT_VIDEO_BITRATE, int alignBytes = MX_IMAGE_ALIGN_4);

FFMO_API   int GetVideoBitsPerPixel(int format);

FFMO_API   int GetAudioChannelLayout(int channels);
FFMO_API   int GetAudioBytesPerSample(int format);
FFMO_API   FF_PIXEL_FORMAT  GetVideoPixelFormat(int format);
FFMO_API   UINT          GetPictureBufferSize(int format, int width, int height, int alignBytes);

FFMO_API   UINT  GetAudioBufferSize(int format, int channels, int frameCount);
FFMO_API   int   GetAudioChannelCount(int channelLayout);

FFMO_API   FF_WAVE_FORMAT  GetAudioSampleFormat(int format);

//Get the current time in microseconds（百万分之一秒）
FFMO_API   int64_t  GetAvTime();

//返回0： 成功
FFMO_API   int  SleepAv(unsigned int usec);

FFMO_API   int  GetAudioSilence(FF_WAVE_FORMAT format);
 
 


#endif
