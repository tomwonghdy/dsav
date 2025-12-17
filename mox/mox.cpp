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
#include "mox.h"

#include "mediaobject.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


//
//
//// This is an example of an exported variable
//MOX_API int nMox=0;
//
//// This is an example of an exported function.
//MOX_API int fnMox(void)
//{
//	return 42;
//}
//
//// This is the constructor of a class that has been exported.
//// see mox.h for the class definition
//CMox::CMox()
//{ 
//	return; 
//}
//



typedef struct __mx_media_sample_
{

	__int64 pts;
	double  timeBase;
	__int64 serial;
	double  duration;
	UINT   nDescriptorSize;
	UINT   nDataSize;
	void*  pSampleBuffer;
}MxMediaSample;
 

MOX_API MX_HANDLE    mxCreateSample(MxDescriptor* pFormatDescriptor, UINT nDescriptorSize, void* pDataBuffer, UINT nDataSize  )
{
	MxMediaSample* pSample =(MxMediaSample*) mxCreateSampleEx(pFormatDescriptor, nDescriptorSize, nDataSize);

	if (NULL == pSample) return NULL;
	 
	if (nDataSize > 0){
		RV_ASSERT(pDataBuffer);
		memcpy((char*)pSample->pSampleBuffer + nDescriptorSize , pDataBuffer,  nDataSize); 
	}

	return pSample;

}

MOX_API  MX_HANDLE  mxCreateSampleEx(MxDescriptor* pFormatDescriptor, UINT nDescriptorSize,  UINT nDataSize)
{
	if ( (nDescriptorSize + nDataSize ) == 0) return NULL;

	MxMediaSample* pSample = MPM_ALLOC_OBJ(MxMediaSample);
	RV_ASSERT(pSample);
	
	if (NULL == pSample) return NULL;

	
	pSample->pts = 0;
	pSample->timeBase = 1;
	pSample->serial = 0;
	pSample->duration = 0;
	pSample->nDescriptorSize = nDescriptorSize;
	pSample->nDataSize = nDataSize;

	pSample->pSampleBuffer = MPM_ALLOC(nDescriptorSize + nDataSize);

	RV_ASSERT(pSample->pSampleBuffer);

	if (NULL == pSample->pSampleBuffer){
		MPM_FREE(pSample);
		return NULL;
	}

	if (nDescriptorSize>0){
		RV_ASSERT(pFormatDescriptor);
		memcpy(pSample->pSampleBuffer, pFormatDescriptor, nDescriptorSize); 
	}

	return pSample;

}


 MOX_API  void mxDestroySample(MX_HANDLE hSample)
{
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		MPM_FREE(pSample->pSampleBuffer);
		MPM_FREE(pSample);
	}
	
}


 MOX_API MX_HANDLE mxDuplicateSample(MX_HANDLE hSrc )
{
	RV_ASSERT(hSrc);

	MxMediaSample* pSrc =( MxMediaSample*) hSrc;

	MxMediaSample* pDest= (MxMediaSample*) mxCreateSample((MxDescriptor*)pSrc->pSampleBuffer, pSrc->nDescriptorSize, ((char*)pSrc->pSampleBuffer + pSrc->nDescriptorSize), pSrc->nDataSize); 

	RV_ASSERT(pDest);

	pDest->serial = pSrc->serial;
	pDest->duration = pSrc->duration;
	pDest->pts = pSrc->pts;
	pDest->timeBase = pSrc->timeBase;
	
	return pDest;
}



  MOX_API  UINT  mxGetSampleDataSize(MX_HANDLE hSample)
{
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		return pSample->nDataSize;
	}

	return 0;

}


 MOX_API void*   mxGetSampleData(MX_HANDLE hSample)
{
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		return  ((char*)pSample->pSampleBuffer + pSample->nDescriptorSize);
	}

	return 0;

}

 MOX_API UINT    mxGetSampleDescriptorSize(MX_HANDLE hSample)
{
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		
		return pSample->nDescriptorSize ;
	}

	return 0;

}

 MOX_API  MxDescriptor*  mxGetSampleDescriptor(MX_HANDLE hSample)
{
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		if (0 == pSample->nDescriptorSize) return NULL;

		return  (MxDescriptor*)pSample->pSampleBuffer  ;
	}

	return NULL;

}


 MOX_API __int64   mxGetSamplePTS(MX_HANDLE hSample)
{
	
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		
		return pSample->pts ;
	}

	return 0;

}

 MOX_API  double  mxGetSampleTimeBase(MX_HANDLE hSample)
{
	
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		
		return pSample->timeBase ;
	}

	return 0;

}


 MOX_API void   mxSetSamplePTS(MX_HANDLE hSample, __int64 val)
{
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){ 
		  pSample->pts  = val;
	}

	 
}
 MOX_API void   mxSetSampleTimeBase(MX_HANDLE hSample, double val)
{
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		  pSample->timeBase =val ;
	} 
}

 MOX_API void   mxSetSampleSerial(MX_HANDLE hSample, __int64 val)
 {
	 MxMediaSample* pSample =( MxMediaSample*) hSample;

	 if (pSample){
		 pSample->serial =val ;
	 } 
 }

 MOX_API __int64   mxGetSampleSerial(MX_HANDLE hSample )
 {
	 MxMediaSample* pSample =( MxMediaSample*) hSample;

	 if (pSample){
		 return  pSample->serial   ;
	 } 

	 return 0;
 }

 MOX_API void     mxSetSampleDuration(MX_HANDLE hSample, double val)
 {
	 MxMediaSample* pSample = (MxMediaSample*)hSample;

	 if (pSample) {
		 pSample->duration = val;
	 }
 }
 
 MOX_API double  mxGetSampleDuration(MX_HANDLE hSample)
 {
	 MxMediaSample* pSample = (MxMediaSample*)hSample;

	 if (pSample) {
		 return  pSample->duration;
	 }

	 return 0;
 }


 MOX_API void  mxSetSampleOptions(MX_HANDLE hSample, __int64 pts, double timeBase, __int64 serial, double duration)
{
	MxMediaSample* pSample =( MxMediaSample*) hSample;

	if (pSample){
		  pSample->pts = pts;
		  pSample->timeBase = timeBase;
		  pSample->serial = serial;
		  pSample->duration = duration;
	} 
}

 MOX_API void  mxGetSampleOptions(MX_HANDLE hSample, __int64* pPts, double* pTimeBase, __int64* pSerial, double* pDuration)
 {
	 MxMediaSample* pSample =( MxMediaSample*) hSample;

	 if (pSample){
		 if (pPts) *pPts = pSample->pts  ;
		 if (pTimeBase) *pTimeBase  = pSample->timeBase ;
		 if (pSerial) *pSerial = pSample->serial ;
		 if (pDuration) *pDuration = pSample->duration;
	 }

 }
  
 MOX_API void mxResetAudioDesc(MxAudioDesc* pDesc, int sampleFormat, int rate, int channelCount, int sampleCount)
{
	RV_ASSERT(pDesc);
	pDesc->sampleFormat =(int) sampleFormat;
	pDesc->sampleRate  = rate;	
	pDesc->channelCount = channelCount;
 	pDesc->channelLayout = -1;
	pDesc->depth = mxGetSampleBytes((MX_AUDIO_FORMAT)sampleFormat);
	pDesc->sampleCount = sampleCount;

	pDesc->type = MX_MT_AUDIO;
	pDesc->alignBytes = 1;
	pDesc->bitRate = AV_DEFAULT_AUDIO_BITRATE;
	pDesc->dataSize =0;	 

  
}


 MOX_API   void mxResetVideoDesc(MxVideoDesc* pDesc, int format,  int width, int height, int depth)
  {
	  RV_ASSERT(pDesc);
	  pDesc->format = format;
	  pDesc->depth = depth;
	  pDesc->type =  MX_MT_VIDEO;
	  pDesc->width = width;
	  pDesc->height = height;
	  pDesc->alignBytes = MX_IMAGE_ALIGN_4;
	  pDesc->bitRate = AV_DEFAULT_VIDEO_BITRATE;
	  pDesc->fps = MX_DEFAULT_VIDEO_FPS;

	  //CMediaObject子类可能会修改这个值
	  pDesc->dataSize = CMediaObject::CalcBitmapBufferSize(depth, width, height, MX_IMAGE_ALIGN_4);

  }


 MOX_API   void mxResetActivePageDesc(MxActivePageDesc* pDesc, int ucCount,    int duration  )
  {
	  RV_ASSERT(pDesc);
 

	  pDesc->type = MX_MT_ACTIVEPAGE;
	  pDesc->dataSize = 0;
	  pDesc->duration = duration;
	  pDesc->ucCount= ucCount;
	 

  }

 void  mxResetSubTitleDesc(MxSubtitleDesc  * pDesc,  MX_SUBTITLE_FORMAT format,  int duration   )
{
	RV_ASSERT(pDesc);
	 
	pDesc->type = MX_MT_SUBTITLE;
	pDesc->dataSize = 0;
	pDesc->duration = duration;
	pDesc->format= format;
	 
 }

 MOX_API  int mxGetPixelDepth(int format)
 {
	 if (format == MX_VF_U8) return 8;
	 else if (format == MX_VF_BGR) return 24;
	 else if (format == MX_VF_BGRA) return 32;
	 
	 return 0;
 }


 MOX_API MxDescriptor* mxCreateDescriptor(MX_MEDIA_TYPE type)
{
	  MxDescriptor* pDest = NULL;

	  if (type == MX_MT_VIDEO){
		  pDest =(MxDescriptor*) MPM_ALLOC(sizeof(MxVideoDesc));

		  if (pDest){
			  mxResetVideoDesc((MxVideoDesc *)pDest, MX_VF_BGR, 512,384, 24);
		  }
		  else{
			  RV_ASSERT(0);
		  }

	  }
	  else if (type == MX_MT_AUDIO){
		  pDest =(MxDescriptor*) MPM_ALLOC(sizeof(MxAudioDesc));

		  if (pDest){
			  mxResetAudioDesc((MxAudioDesc *)pDest, MX_AF_S16, 44100, 2, 1024);
		  }
		  else{
			  RV_ASSERT(0);
		  }

	  }
	  else if (type == MX_MT_ACTIVEPAGE){
		  pDest = (MxDescriptor*)MPM_ALLOC(sizeof(MxActivePageDesc));

		  if (pDest){
			  mxResetActivePageDesc((MxActivePageDesc *)pDest, 0);
		  }
		  else{
			  RV_ASSERT(0);
		  }

	  }
	  else  if (type == MX_MT_SUBTITLE){
		  pDest = (MxDescriptor*)MPM_ALLOC(sizeof(MxSubtitleDesc  ));

		  if (pDest){
			  mxResetSubTitleDesc ((MxSubtitleDesc *)pDest, MX_STF_BITMAP);
		  }
		  else{
			  RV_ASSERT(0);
		  }

	  }

	  return pDest;
}


 MOX_API void mxDestroyDescriptor(MxDescriptor* pDescriptor )
{

	if (pDescriptor){
		MPM_FREE(pDescriptor);
	}

}

 MOX_API  int  mxGetSampleBytes(MX_AUDIO_FORMAT format)
 {
	 
	 if (format == MX_AF_U8) return 1*8; 
	 else if (format == MX_AF_S16) return 2*8;
	 else if (format == MX_AF_S32) return 4 * 8;
	 else if (format == MX_AF_FLT) return 4 * 8;
	 else if (format == MX_AF_DBL) return 8 * 8;
	 else if (format == MX_AF_U8P) return 1 * 8;
	 else if (format == MX_AF_S16P) return 2 * 8;
	 else if (format == MX_AF_S32P) return 4 * 8;
	 else if (format == MX_AF_FLTP) return 4 * 8;
	 else if (format == MX_AF_DBLP) return 8 * 8;

	 return 0;
 }


  MOX_API MxDescriptor* mxCopyDescriptor(MxDescriptor* pSrc,  MxDescriptor* pDest)
{
	RV_ASSERT(pSrc);

	if (NULL == pDest){
		pDest = mxCreateDescriptor(pSrc->type);
		if (NULL == pDest) return NULL;
	}
	else{
		RV_ASSERT(pSrc->type == pDest->type);

		if (pSrc->type != pDest->type) return pDest;
	}

	if (pSrc->type == MX_MT_AUDIO){
		*((MxAudioDesc*)pDest) = *((MxAudioDesc*)pSrc);
	}
	else if (pSrc->type == MX_MT_VIDEO){
		*((MxVideoDesc*)pDest) = *((MxVideoDesc*)pSrc);
	}
	else if (pSrc->type == MX_MT_ACTIVEPAGE){
		RV_ASSERT(0);

		*((MxActivePageDesc*)pDest) = *((MxActivePageDesc*)pSrc);
	}
	else{
		RV_ASSERT(0);
	}

	return pDest;
}



  
MOX_API  __int64 mxGetCurrentTime()
  {
	  LARGE_INTEGER litmp; 
	  //LONGLONG ticks;
	  //LONGLONG lTimeEnd;
	  //	double dMinus;
	  double dFreq;
	  double ticks;

	  QueryPerformanceFrequency(&litmp);	// 获得机器内部定时器的时钟频率
	  dFreq = (double)litmp.QuadPart;		// 获得计数器的时钟频率

	  QueryPerformanceCounter(&litmp);
	  ticks = (double)litmp.QuadPart;		// 获得时间起始值

	  return (__int64)((ticks  / dFreq) * 1000000);  

  };
