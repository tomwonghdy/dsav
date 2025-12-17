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
#include "stdafx.h"
#include "SoundOut.h"


 


#define MAX_BUFFER_COUNT   5


CSoundOut::CSoundOut()
{
	//ZeroMemory(&m_Waveformat,sizeof(WAVEFORMATEX)); 
	memset(&m_waveCaps,0,sizeof(m_waveCaps));
	m_bDevOpen = FALSE;
	m_bStarted = FALSE;

	m_hWave = 0;
	m_hThread = 0;
	m_ThreadID = 0;
	m_BufferQueue = 0;
	//m_Lock.Init();

	m_pPlayDoneFunc = NULL ;
	m_pUserData = NULL;

	::InitializeCriticalSection(&m_csLock);


}

CSoundOut::~CSoundOut()
{
	Stop();
//	m_Lock.cleanup();
	::DeleteCriticalSection (&m_csLock);

}

UINT CSoundOut::GetDeviceNum()
{
	return waveOutGetNumDevs();
}

WAVEOUTCAPS* CSoundOut::GetDeviceCap()
{
	MMRESULT mRet = waveOutGetDevCaps(WAVE_MAPPER,&m_waveCaps,sizeof(m_waveCaps));
	if( mRet == MMSYSERR_NOERROR )
		return &m_waveCaps;
	return NULL;
}

// 典型参数设置方法 longf120823
// 	_wfx.nSamplesPerSec  = 44100;  /* sample rate */
// 	_wfx.wBitsPerSample  = 16;     /* sample size */
// 	_wfx.nChannels       = 2;      /* channels    */
// 	_wfx.cbSize          = 0;      /* size of _extra_ info */
// 	_wfx.wFormatTag      = WAVE_FORMAT_PCM;
// 	_wfx.nBlockAlign     = (_wfx.wBitsPerSample * _wfx.nChannels) >> 3;
// 	_wfx.nAvgBytesPerSec = _wfx.nBlockAlign * _wfx.nSamplesPerSec;
BOOL CSoundOut::Open(PWAVEFORMATEX pWaveformat)
{
	if( m_bDevOpen )
	{
		return FALSE;
	}
	
	//memcpy(&m_Waveformat,pWaveformat,sizeof(WAVEFORMATEX));
	pWaveformat->nBlockAlign     = (pWaveformat->wBitsPerSample * pWaveformat->nChannels) >> 3;
	pWaveformat->nAvgBytesPerSec = pWaveformat->nBlockAlign * pWaveformat->nSamplesPerSec;


	MMRESULT mRet;
//	WAVEFORMATEX wfx;

	//lphWaveOut: PHWaveOut;   {用于返回设备句柄的指针; 如果 dwFlags=WAVE_FORMAT_QUERY, 这里应是 nil}
	//uDeviceID: UINT;         {设备ID; 可以指定为: WAVE_MAPPER, 这样函数会根据给定的波形格式选择合适的设备}
	//lpFormat: PWaveFormatEx; {TWaveFormat 结构的指针; TWaveFormat 包含要申请的波形格式}
	//dwCallback: DWORD        {回调函数地址或窗口句柄; 若不使用回调机制, 设为 nil}
	//dwInstance: DWORD        {给回调函数的实例数据; 不用于窗口}
	//dwFlags: DWORD           {打开选项}// long120823
	mRet = waveOutOpen(0,WAVE_MAPPER,pWaveformat,0,0,WAVE_FORMAT_QUERY);

	if( mRet != MMSYSERR_NOERROR )
	{
		return FALSE;
	}

	mRet = waveOutOpen(&m_hWave, WAVE_MAPPER, pWaveformat, m_ThreadID, 0, CALLBACK_THREAD);

	if( mRet != MMSYSERR_NOERROR )
	{
		return FALSE;
	}

	m_bDevOpen = TRUE;

	return TRUE;
}

void CSoundOut::Close()
{
	if (!m_bDevOpen)
	{
		return;
	}

	if(!m_hWave)
	{
		return;
	}

	MMRESULT mRet = waveOutClose(m_hWave);
	if( mRet != MMSYSERR_NOERROR )
	{
		return;
	}
	m_hWave = 0;
	m_bDevOpen = FALSE;

}

DWORD WINAPI CSoundOut::ThreadProc(LPVOID lpParameter)
{
	CSoundOut *pWaveOut;
	pWaveOut = (CSoundOut *)lpParameter;

	MSG msg;
	while(GetMessage(&msg,0,0,0))
	{
		switch(msg.message )
		{
		case WOM_OPEN:
			break;
		case WOM_CLOSE:
			break;
		case WOM_DONE:
			{
				WAVEHDR* pWaveHead = (WAVEHDR*)msg.lParam;
				waveOutUnprepareHeader((HWAVEOUT)msg.wParam,pWaveHead,sizeof(WAVEHDR));
				pWaveOut->SubBuffer();
				delete []pWaveHead->lpData;
				delete pWaveHead;

				if (pWaveOut->m_pPlayDoneFunc && pWaveOut->m_bStarted){
					pWaveOut->m_pPlayDoneFunc(pWaveOut->m_pUserData);
				}				
			}
			break;
		/*case WM_QUIT:  
		  //this case will never be happened.  
		  //If the GetMessage retrieves the WM_QUIT message, the return value is zero.
			return msg.wParam;*/
		}
	}

	return msg.wParam;
}

BOOL CSoundOut::StartThread()
{
	if( m_bStarted )
	{
		return FALSE;
	}

	m_hThread = CreateThread(0,0,ThreadProc,this,0,&m_ThreadID);

	if( !m_hThread )
	{
		return FALSE;
	}

	m_bStarted = TRUE;

	return TRUE;
}

void CSoundOut::StopThread()
{
	//if (!m_bThread)
	//{
	//	return;
	//}

	if(m_hThread)
	{
		//int t=50;
		//DWORD ExitCode;
		//BOOL bEnd=FALSE;

		while(this->GetBufferNum())::Sleep(10);

		PostThreadMessage(m_ThreadID,WM_QUIT,0,0);

		::WaitForSingleObject(m_hThread,INFINITE); 

	/*	while(t)
		{
			GetExitCodeThread(m_hThread,&ExitCode);

			if(ExitCode!= STILL_ACTIVE)
			{
				bEnd=TRUE;
				break;
			}
			else
				Sleep(10);
			t--;
		}

		if(!bEnd)
		{
			TerminateThread(m_hThread,0);
		}*/

		m_hThread = 0;
	}

	m_bStarted =FALSE;
	 
	//pWaveOut = FALSE;

}

BOOL CSoundOut::Start(PWAVEFORMATEX pWaveformat, SO_PLAY_DONE_CALLBACK pfnPLayDone, void* pUserData)
{
	m_pPlayDoneFunc = pfnPLayDone;
	m_pUserData     = pUserData;

	if (NULL==pWaveformat)
	{
		return FALSE;
	}
	if( !StartThread())
	{
		return FALSE;
	}
	if( !Open(pWaveformat))
	{
		StopThread();
		return FALSE;
	}



	return TRUE;
}

BOOL CSoundOut::PlayAudio(char* buf,unsigned int  nSize)
{
	MX_ASSERT(buf);

	if( !m_bDevOpen )
	{
		return FALSE;
	}

	if( GetBufferNum() >= MAX_BUFFER_COUNT)//超过缓冲最大包，不继续播放 
	{
		return FALSE;
	}

	MMRESULT mRet;
	char*	lpData = NULL;
	WAVEHDR* pWaveHead = new WAVEHDR;

	ZeroMemory(pWaveHead,sizeof(WAVEHDR));

	lpData = new char[nSize];
	pWaveHead->dwBufferLength = nSize;
	memcpy(lpData,buf,nSize);
	pWaveHead->lpData = lpData;

	mRet = waveOutPrepareHeader(m_hWave,pWaveHead, sizeof(WAVEHDR));
  	if( mRet != MMSYSERR_NOERROR )
	{
		return FALSE;
	}

	mRet = waveOutWrite(m_hWave,pWaveHead,sizeof(WAVEHDR));
  	if( mRet != MMSYSERR_NOERROR )
	{
		return FALSE;
	}

	AddBuffer();

	return TRUE;
}

void CSoundOut::Stop()
{
	m_bStarted = FALSE;

	StopThread();

	Close();

}


int CSoundOut::GetBufferNum()
{
	int nRet = 5;

	//::EnterCriticalSection(&m_csLock);
	::EnterCriticalSection(&m_csLock);

	nRet = m_BufferQueue;

	::LeaveCriticalSection(&m_csLock);

	return nRet;
}

BOOL  CSoundOut::IsQueueFull()
{
	return  (m_BufferQueue >= MAX_BUFFER_COUNT );
}

void CSoundOut::AddBuffer()
{
	::EnterCriticalSection(&m_csLock);
	
	m_BufferQueue++;
	
	::LeaveCriticalSection(&m_csLock);
}

void CSoundOut::SubBuffer()
{
	::EnterCriticalSection(&m_csLock);

	m_BufferQueue--;
	
	::LeaveCriticalSection(&m_csLock);
}

void CSoundOut::SetVolume(DWORD  val)
{
	if (m_bStarted){
		MX_ASSERT(m_hWave);
	    waveOutSetVolume(m_hWave, val) ;
	}
}

DWORD  CSoundOut::GetVolume(){

	if (m_bStarted){
		MX_ASSERT(m_hWave);

		DWORD n=0;

		if (MMSYSERR_NOERROR ==  waveOutGetVolume(m_hWave, &n) )
		{
			return n;
		}
	}

	return 0;
}
