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
#ifndef __AUDIO_TRACK_H_IMPORTED_
#define __AUDIO_TRACK_H_IMPORTED_

#include "amo.h"
#include "..\davsdk\includes\mox.h"
#include "..\davsdk\includes\WaveConverter.h"

#include <rvb\rvtypes.h>

class AMO_API CAudioTrack
{
public:
	CAudioTrack(void);
	~CAudioTrack(void);

	BOOL  AddSample(MX_HANDLE hSample);

	void  Read(char* pBuffer, int nSize, BOOL bCopyOnly=TRUE);


	BOOL  Start(MxAudioDesc* pSrcDesc, MxAudioDesc* pDestDesc );
	void  Stop();

	void HandleSample();

	MX_HANDLE GetSample();


	HANDLE GetBufferFullEvent() {return m_hBufferFullEvent; };
	HANDLE GetBufferEmptyEvent() {return m_hBufferEmptyEvent; };

	float   m_nScalar;

private:
	BOOL  WaitBufferToEmpty(int timeout);
	
	CWaveConverter m_waveConverter;
 
	
	MxAudioDesc* m_pSrcDesc, *m_pDstDesc;

	BOOL  m_bStarted;

	HANDLE m_hBufferFullEvent;
	HANDLE m_hBufferEmptyEvent;

	char* m_pAudioBuffer;
	int   m_nBufferSize;
	int   m_nCurPos;

	HANDLE            m_hSemaSamples ;
 	CRITICAL_SECTION  m_csSample;

	CRITICAL_SECTION  m_csBuffer;

	RvSequence m_arrSample ; // CList<MX_HANDLE , MX_HANDLE>

 
	HANDLE m_hProcThread;

	void RemoveAllSample(void);
};




#endif