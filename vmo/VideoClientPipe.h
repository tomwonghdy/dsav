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
#pragma once

#include "..\davsdk\vdevs\PipeClient.h"


#include <rvb\dsmsp.h>

//#define VIDEO_PIPE_NAME  "\\\\.\\pipe\\LaodaoCamPipe"

 //每次数据的语音播放长度:毫秒
//需要与filter一致
//#define AUDIO_PACKET_TIME  40

//#define MAXBUF 1024*1024


class  CVideoClientPipe: public CPipeClient
{
public:
	CVideoClientPipe(void);
	virtual ~CVideoClientPipe(void);

	BOOL Start(const TCHAR* strPipeName);
	void Stop();

	//BOOL IsListFull();
	int GetWidth() {return this->m_nWidth; };
	int GetHeight() {return this->m_nHeight; };
	int GetDepth() { return this->m_nDepth; };

//	BOOL IsRunning(){return !m_bExitService; };

	BOOL SetImageData(const char* pImageData, UINT nDataSize);

	
	//void  ReleaseAll();
	//void  ReleaseBuffers();
public:
	//void SetWaveFormat(int channels, int bitsPerSample, int samplesPerSecond );
	 
	//void AutoSendData();

private:
	 
	 
	//void DestroyMyPipe();
	 



private:
//	BOOL m_bExitService;
//	int m_nClientWidth,m_nClientHeight;
	//int m_fps;
	//HANDLE m_hPipe;

	//RvList m_list;

protected:

	
	CRITICAL_SECTION m_cs;

	//HANDLE m_hThread;
	
	int   m_nWidth, m_nHeight, m_nDepth;


  	UINT  m_nWaveWrapSize;
	char* m_pWaveWrapData;
//	UINT  m_nWaveDataSize; // 音频数据长度
//	UINT  m_nCurDataSize;

//	char*  m_pWaveData;     // 音频数据

//	BOOL  m_bConnected;
		
	//friend DWORD WINAPI DataSendThread(LPVOID lpParameter);


 
	int SendVideoData(HANDLE hPipe, const char* pData , UINT size);
	int QueryFormat(HANDLE hPipe, int& with, int& height, int& depth);
	void ResetFormat(int width, int height, int depth);
};

