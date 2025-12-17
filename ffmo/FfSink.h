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
#ifndef __MX_FILE_SINK_H_INCLUDED_
#define __MX_FILE_SINK_H_INCLUDED_



#include "ffmo.h"
 

#include "..\davsdk\includes\sink.h"
#include "..\davsdk\includes\InputPad.h"
  
#include <rvb\dsm.h>
#include <sst\ucut.h>
 

//output type
#define FS_OT_AUDIO   1
#define FS_OT_VIDEO   (1<<1)
#define FS_OT_DEFAULT (FS_OT_AUDIO | FS_OT_VIDEO)
#define FS_OT_NONE    0

//MUX RESULT
#define MR_FULL        2
#define MR_NODATA      1
#define MR_NORMAL      0
#define MR_EOF         3

//ffsink should not be used to encode mp3 file
//using CMp3Sink instead
class FFMO_API CFfSink: public CSink
{
public:
	CFfSink(void);
	virtual ~CFfSink(void);
	

	virtual M_RESULT Play(const char* strFilePath );
	virtual void Pause(BOOL bResume);
	virtual void Stop();

	M_RESULT Replay(const char* strFilePath);
	CInputPad* GetAudioPad() {return &m_audioPad;};
	CInputPad* GetVideoPad() {return &m_videoPad;};

	void SetDuration(double val); 

	//重新调用play有效。
	void SetImageOutputSize(  int width, int height);
	
	void SetVideoBitRate(int bitRate);
	void SetAudioBitRate(int bitRate);

	void HandleMux();

	//具体格式协商。
	virtual BOOL Accept(CPad* pInputPad, MxDescriptor* pDescriptor);
	 

	//如果输入PIN是推模式，需要重载，该函数
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE  hSample, int* pErrCode) ;


	BOOL  FeedImage(const RvImage image);


public:
	int  m_nOutputType; 

	int  m_nDuration;
	int  m_nGopSize;
	int  m_nMaxBframes;
	int  m_nMacroBlockDecision;

	//音频对齐
	int  m_nAudioAlign;

	 
	int   m_nVideoFps;

protected:

	M_RESULT  StartEncode(const char* strFileName, BOOL bClearFrame);
	
	BOOL ResampleRaw(int sampleWanted, int* pErrCode);

	int Resample(MxAudioDesc* pDesc, BYTE* data, int size);


	BOOL GetAudioData(BOOL bFillRemains, int* pErrCode);
	BOOL AddAudioData(HANDLE hSample , int * pErrCode);
	BOOL AddVideoData(HANDLE hSample,  int* pErrCode );
  
	 
	int WriteAudioData(/*HANDLE hSample*/ /*MX_AUDIO_DATA* pAudioData*/);
	int WriteVideoData(HANDLE hSample/*MX_VIDEO_DATA* pVideoData*/ );
  
 	int  Mux(BOOL bCheckDuration , BOOL bEncodeToEnd, BOOL& bEncodeVideo, BOOL& bEncodeAudio);	
  
	
	BOOL  AddStream(int mediaType, int codecId);
	void    DeleteStream();

	BOOL InitContext(HANDLE hFxCxt, const char* strFormatName, const char* strFileName);
	void ReleaseContext(void);

	BOOL OpenDiskFile(const char* strFileName, int* pErrCode);
	BOOL CloseDiskFile( int* pErrCode);
  
	void ResetFrameArrays();

	virtual BOOL SetVideoCodecOptions(int codecId) { return TRUE; };
	virtual BOOL SetAudioCodecOptions(int codecId) { return TRUE; };
	 
protected: 
	  
	CRITICAL_SECTION  m_csRun;
	 
	MxAudioDesc  m_audioDstDesc;
	MxVideoDesc  m_videoDstDesc;
	 
	CInputPad m_audioPad;
	CInputPad m_videoPad;
	  
	int  m_nAudioEncodeCount;
	int  m_nVideoEncodeCount;
	 
	RvList m_lstAudioFrame ; 
	RvList m_lstVideoFrame ; 

	HANDLE  m_hOutputThread;	 
	HANDLE m_hEncodeContext;
	  
	
};







#endif
