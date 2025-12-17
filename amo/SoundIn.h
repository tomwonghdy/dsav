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
#ifndef __SOUND_IN_H_INCLUDED_
#define __SOUND_IN_H_INCLUDED_

#include "amo.h"
#include <Mmsystem.h>

#define SI_MAX_BUFFERS  3

typedef void ( WINAPI *SI_DATA_FILL_DONE_CALLBACK) (void* lpUser, void* pAudioData, DWORD dwDataSize);

class AMO_API CSoundIn
{
public:
	CSoundIn(void);
	~CSoundIn(void);


	BOOL Start(int index, PWAVEFORMATEX pWaveformat,  UINT nWantedDataSize, SI_DATA_FILL_DONE_CALLBACK pfnDataFillDone = NULL, void* pUserData = NULL);
	void Stop();

	int GetDeviceCount();
	const char* GetDeviceDescription(int index);

	BOOL IsFormatSupported(int index , int rate, int channels, int depth);

	void StartRecording();
	void ProcessHeader(WAVEHDR * pHdr);

private:
	
	//UINT FillDevices();
	
	//void SetStatus(LPCTSTR lpszFormat, ...);
	//CString StoreError(MMRESULT mRes,BOOL bDisplay,LPCTSTR lpszFormat, ...);

	//void OpenDevice();
	//void CloseDevice();

	void PrepareBuffers();
	void UnPrepareBuffers();

	 
	CRITICAL_SECTION m_cs;
 

	BOOL m_bRun;

	HWAVEIN m_hWaveIn;
	//WAVEFORMATEX m_stWFEX;

	WAVEHDR m_stWHDR[SI_MAX_BUFFERS];
	HANDLE m_hThread;


	SI_DATA_FILL_DONE_CALLBACK m_pDataFillDoneFunc;
	void* m_pUserData;

	UINT m_nWantedDataSize;




};





#endif