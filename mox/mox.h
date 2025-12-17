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
#ifndef  __MOX_H_INCLUDED_
#define  __MOX_H_INCLUDED_


#ifdef MOX_EXPORTS
#define MOX_API __declspec(dllexport)
#else
#define MOX_API __declspec(dllimport)
#endif





#include <rvb\rvtypes.h>


#include <Mmsystem.h>
#include <assert.h>

 
  
//视频幁率， 每秒X帧
#define MX_DEFAULT_VIDEO_FPS    25
#define MX_MAX_VIDEO_FPS        60
#define MX_MIN_VIDEO_FPS        5

#define  MX_INFINITE  0x7FFFFFFF

#define MX_CLOSE_HANDLE(hdl)  if (hdl){CloseHandle(hdl); hdl=NULL;} 

#define MX_IMAGE_ALIGN_1    1
#define MX_IMAGE_ALIGN_4    4

#define AV_DEFAULT_AUDIO_FORMAT   MX_AF_S16

#define AV_DEFAULT_AUDIO_BITRATE   (64000*3)
#define AV_DEFAULT_VIDEO_BITRATE   400000

#define AV_DEFAULT_VIDEO_WIDTH     640
#define AV_DEFAULT_VIDEO_HEIGHT    480
#define AV_DEFAULT_VIDEO_FORMAT    MX_VF_BGR

#if _DEBUG
  #define  MX_ASSERT(expr)   assert(expr)
#else
  #define  MX_ASSERT(expr)
#endif

typedef enum __mx_media_type_{
	MX_MT_UNKNOWN    = 0 ,
	MX_MT_AUDIO      = 1 ,	   //音频
	MX_MT_VIDEO      = (1<<1)  ,	//视频
	MX_MT_SUBTITLE   = (1<<2)  ,  //字幕
	MX_MT_ACTIVEPAGE = (1<<3),    //活动页
}MX_MEDIA_TYPE;

//STATUS REPORT
enum M_STATUS{
   MS_NORMAL    =   0,
   MS_COMPLETE  =   0,
   MS_INPROCESS  =    1, //正在处理
   MS_NOSAMPLE   =    2, //没有样本数据
   MS_FILEFULL   =    3, //文件已经写满
};



//error
enum M_RESULT
{
	M_OK               = 0,
	M_FAILED           = -1,
	M_FILE_NONEXIST    = -2,
	M_RESOURCE_MISSING = -3,
	M_OUT_OF_RANGE     = -4,
    M_CREATE_THREAD    = -5,
	M_CODEC_ERROR      = -6,
	M_MEM_ALLOC        = -7,
	M_INVALID_OBJECT   = -8,
	M_FILE_ERROR       = -9,
	M_OCCUPIED         = -10,
	M_BUFFER_FULL      = -10,
	M_UNKNOWN_TYPE     = -11,
	M_DISCONNECTED     = -12,
	M_DATA_REJECTED    = -13,
	M_BUFFER_EMPTY     = -14,	
	M_TERMINATED       = -15,
	M_CLOSED           = -15,
	M_STOPPED          = -15,
	M_DATA_UNAVAILABLE     = -16, //当前无数据
	M_STREAM_UNAVAILABLE   = -17, //
	M_UNSUPPORT            = -17, //
	M_FILE_FULL            =-18, //文件满
	M_FORMAT_MISMATCH      =-19,
	M_UNSUPPORT_PAD       =-20,
};


typedef struct __mx_media_msg_{
	UINT  code;
	UINT  wParam, lParam;
}MX_MEDIA_MSG;


#define MX_HANDLE   HANDLE

//typedef enum _mx_subtitle_type{
//	MX_ST_IMAGE=0,
//	MX_ST_TEXT,
//}MX_SUBTITLE_TYPE;

#define MX_ET_DEFAULT_TEXT   0
#define MX_ET_DEFAULT_IMAGE  1


#define MX_STOPPED  0 
#define	MX_PLAYING  1 
#define	MX_PAUSED   2 


//datasize 根据媒体的内容不同，有不同的数据大小，该数据仅包含实际媒体的数据
//不包括头文件格式等信息
#define MX_DESCRIPTOR_FIELDS    MX_MEDIA_TYPE  type;  \
	                            UINT        dataSize;


typedef struct __mx_descriptor_
{
	MX_DESCRIPTOR_FIELDS
}MxDescriptor;



 

typedef void  (WINAPI* MO_ERROR_OCCURRED_FUNC)(HANDLE /*hObject*/, int /*errorCode*/, void* /*pUserData*/);
typedef void  (WINAPI* MO_STATUS_REPORT_FUNC) (HANDLE /*hObject*/, int /*status*/, void* /*pUserData*/);

typedef void  (WINAPI* MO_SAMPLE_PASS_FUNC) (HANDLE /*hObject*/, HANDLE /*output pad*/, MX_HANDLE /*hSample*/, void* /*pUserData*/);





//audio format must be coordinance with that in FFMPEG
typedef enum __mx_subtitle_format_
{
	MX_STF_UNKNOWN   =  -1,
	MX_STF_BITMAP    = 0,        //FFMPEG SUBTITLE 
	MX_STF_TEXT        ,         //FFMPEG SUBTITLE 
	MX_STF_ASS         ,         //FFMPEG SUBTITLE 
}MX_SUBTITLE_FORMAT;


typedef struct __mx_subtitle_descriptor_{
	 MX_DESCRIPTOR_FIELDS
	 MX_SUBTITLE_FORMAT  format; 
	 int                 duration;
}MxSubtitleDesc;

typedef struct __mx_active_descriptor_{
	MX_DESCRIPTOR_FIELDS
	int             ucCount;//user control count
	int             duration;
}MxActiveDesc, MxActivePageDesc;


//audio format must be coordinance with that in FFMPEG
typedef enum __mx_audio_format_
{
	MX_AF_UNKNOWN =-1,
	MX_AF_U8    =0,          ///< unsigned 8 bits
    MX_AF_S16     ,         ///< signed 16 bits
    MX_AF_S32     ,         ///< signed 32 bits
    MX_AF_FLT     ,         ///< float
    MX_AF_DBL     ,         ///< double
    MX_AF_U8P     ,         ///< unsigned 8 bits, planar
    MX_AF_S16P    ,        ///< signed 16 bits, planar
    MX_AF_S32P    ,        ///< signed 32 bits, planar
    MX_AF_FLTP    ,        ///< float, planar
    MX_AF_DBLP    ,        ///< double, planar
}MX_AUDIO_FORMAT;


typedef enum __mx_video_format_
{
	MX_VF_UNKNOWN =-1,
	MX_VF_U8      = 0x8000,     //ID VALUEs Less than 0x8000 reverved for FFMPEG PIXEL FORMAT
    MX_VF_BGR     = 0x8001  ,        
    MX_VF_BGRA    = 0x8002  ,   
	MX_VF_YUV     = 0x8010  , 
}MX_VIDEO_FORMAT; 
   

//#define  MX_VIDEO_DEPTH(fmt)   ((fmt) & 0xFF)

//#define  MX_TIME_BASE_INT(num, den)  (((num)<<16) | (den)) 
//#define  MX_TIME_BASE_RAT(rational, val)   { rational.num =(int)( (val) >> 16);  rational.den = int((val) & 0xFFFF); }

////audio wave format
//typedef struct __mx_audio_descriptor_{
//	MX_DESCRIPTOR_FIELDS
//	int     format; //format as sampleformat defined in FFPMEG
//    int     sampleRate;
//    int     channelCount;
//	int     channelLayout; //ffmpeg 5.1版channellayout为结构体
//	int     depth;
//	int     silence;         //静音值,16位为0,8位为
//	int     frameCount;
//	int     bitRate;         //用于流媒体压缩和文件输出
//}MxAudioDesc;
//audio wave format
typedef struct __mx_audio_descriptor_ {
	MX_DESCRIPTOR_FIELDS
	int     sampleFormat; //format as sampleformat defined in FFPMEG
	int     sampleRate;   //sample rate
	int     channelCount; 
	int     channelLayout; //ffmpeg 5.1版channellayout为结构体
	int     depth;          //sample bits
	int     alignBytes;      //字节对齐0-默认，1-不对齐
	int     sampleCount;
	int     bitRate;         //用于流媒体压缩和文件输出
}MxAudioDesc;


typedef struct __mx_video_descriptor_{
	MX_DESCRIPTOR_FIELDS
    int      format;  //图像格式，如RGB, YUV等
	int      depth;   //根据不同图像格式计算出来的值，该值与BITMAP图像格式的bitcount对应
    int      width;
    int      height;	
 	int      alignBytes;  //对齐字节1，4
	int      fps;        //帧率
	int      bitRate;    //用于流媒体压缩和文件输出
}MxVideoDesc;


//typedef struct __mx_audio_data
//{
//	int    frameCount;
//	UINT   dataSize;
//	char*  pBuffer;
//}MX_AUDIO_DATA;
//
//typedef struct __mx_video_data
//{		
//	UINT   dataSize;
//	char*  pBuffer;
//}MX_VIDEO_DATA;

 

#define MX_SAFE_CLOSE_HANDLE(h)   if (h){  ::CloseHandle(h); h = NULL; } 



MOX_API  MX_HANDLE    mxCreateSample(MxDescriptor* pFormatDescriptor, UINT nDescriptorSize, void* pDataBuffer, UINT nDataSize);
MOX_API  MX_HANDLE    mxCreateSampleEx(MxDescriptor* pFormatDescriptor, UINT nDescriptorSize,  UINT nDataSize);

MOX_API  void      mxDestroySample(MX_HANDLE hSample);

MOX_API  MX_HANDLE      mxDuplicateSample(MX_HANDLE hSource) ;


MOX_API  UINT      mxGetSampleDataSize(MX_HANDLE hSample);
MOX_API  void*     mxGetSampleData(MX_HANDLE hSample);

MOX_API  UINT      mxGetSampleDescriptorSize(MX_HANDLE hSample);
MOX_API  MxDescriptor*     mxGetSampleDescriptor(MX_HANDLE hSample);


MOX_API  __int64 mxGetSamplePTS(MX_HANDLE hSample);
MOX_API  double     mxGetSampleTimeBase(MX_HANDLE hSample);

MOX_API  void     mxSetSamplePTS(MX_HANDLE hSample, __int64 val);
MOX_API  void     mxSetSampleTimeBase(MX_HANDLE hSample, double val);

MOX_API void     mxSetSampleSerial(MX_HANDLE hSample, __int64 val);
MOX_API __int64  mxGetSampleSerial(MX_HANDLE hSample );

//图像帧的持续时间
MOX_API void     mxSetSampleDuration(MX_HANDLE hSample, double val);
MOX_API double   mxGetSampleDuration(MX_HANDLE hSample);

MOX_API void  mxSetSampleOptions(MX_HANDLE hSample, __int64 pts, double timeBase, __int64 serial, double duration=0 );
MOX_API void  mxGetSampleOptions(MX_HANDLE hSample, __int64* pPts,double* pTimeBase, __int64* pSerial, double* pDuration= NULL);


MOX_API  MxDescriptor*  mxCreateDescriptor(MX_MEDIA_TYPE type);
MOX_API  void           mxDestroyDescriptor(MxDescriptor* pDescriptor );
MOX_API  MxDescriptor*   mxCopyDescriptor(MxDescriptor* pSrc,  MxDescriptor* pDest =NULL);

MOX_API  int  mxGetSampleBytes(MX_AUDIO_FORMAT format);






MOX_API  void  mxResetAudioDesc(MxAudioDesc* pDesc, int sampleFormat, int rate, int channelCount, int sampleCount);
MOX_API  void  mxResetVideoDesc(MxVideoDesc* pDesc, int format,  int width, int height, int depth);

MOX_API  void  mxResetActivePageDesc(MxActivePageDesc* pDesc,   int ucCount  , int duration = -1  );
MOX_API  void  mxResetSubTitleDesc(MxSubtitleDesc* pDesc,  MX_SUBTITLE_FORMAT format,  int duration =0  );

MOX_API  int mxGetPixelDepth(int format);

//获得当前CPU的时间，单位: 微秒
MOX_API  __int64 mxGetCurrentTime();

//
//class CUtString
//{
//public:
//	CUtString(const char* _strUrl)
//	{
//		m_str=NULL;
//		m_nLength =0;
//		if (_strUrl)
//		{
//			m_nLength = (int)strlen(_strUrl);
//			int len =m_nLength + 1;
//	 
//			m_str = new char[len];
//			MX_ASSERT(m_str);
//			strcpy_s(m_str, len, _strUrl);
//		}
//
//	}
//	~CUtString()
//	{
//		if (m_str){
//		    delete[] m_str;
//			m_str=NULL;
//		}
//	}
//
//	char* GetPtr(){ return m_str;};
//	int   GetLength(){ return m_nLength; };
//	int   GetSize(){  if (NULL == m_str) return 0; else  return (m_nLength +1); };
//private:
//	char* m_str;
//	int   m_nLength;
//
//};




#endif
