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
#ifndef __SOUND_OUT_H_INCLUDED_
#define __SOUND_OUT_H_INCLUDED_

#include "amo.h"
//#include <afxmt.h>
#include <Mmsystem.h>



typedef void (*SO_PLAY_DONE_CALLBACK) (void* lpUser);
 

class AMO_API CSoundOut
{
public:
	CSoundOut(void);
	~CSoundOut(void);

public:
	BOOL Start(PWAVEFORMATEX pWaveformat, SO_PLAY_DONE_CALLBACK pfnPLayDone = NULL, void* pUserData = NULL);
	BOOL PlayAudio(char* buf,unsigned int nSize);
	void Stop();

	UINT          GetDeviceNum();
	WAVEOUTCAPS*  GetDeviceCap();
	
	int           GetBufferNum();
	BOOL          IsQueueFull();

	DWORD  GetVolume();

	//The low-order word contains the left-channel volume setting, and the high-order word contains 
	//the right-channel setting. A value of 0xFFFF represents full volume, and a value 
	//of 0x0000 is silence
	void  SetVolume(DWORD  val);
	 
private:
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	inline void AddBuffer();
	inline void SubBuffer();

	BOOL Open(PWAVEFORMATEX pWaveformat);
	void Close();
	BOOL StartThread();
	void StopThread();

private:

	WAVEOUTCAPS	m_waveCaps;
	BOOL		m_bDevOpen;
	BOOL		m_bStarted;
	HWAVEOUT	m_hWave;
	HANDLE		m_hThread;
	DWORD		m_ThreadID;

	//WAVEFORMATEX m_Waveformat;

	//CCriticalSection	m_Lock;
	CRITICAL_SECTION    m_csLock;

	int			m_BufferQueue;

	SO_PLAY_DONE_CALLBACK m_pPlayDoneFunc ;
	void * m_pUserData;

	

};




#endif