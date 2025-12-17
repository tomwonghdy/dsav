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
#ifndef __AUDIO_CAP_H_INCLUDED_
#define __AUDIO_CAP_H_INCLUDED_



#include "amo.h"
#include "..\davsdk\includes\source.h"
#include "..\davsdk\includes\OutputPad.h"
#include "..\davsdk\includes\mox.h"
#include <rvb\dsm.h>

#include "SoundIn.h"



class AMO_API CAudioCap :	public CSource
{
public:
	CAudioCap(void);
	virtual ~CAudioCap(void);


	// dev://a:/wavein/0
    // or    a:/wavein/0
	virtual M_RESULT Play(const char* strUrl ) ;
	virtual void     Pause(BOOL bResume) ;
	virtual void     Stop() ;
 
	 
	virtual COutputPad*  GetOutputPad() {return &m_outputPad;}

	 
	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad);
	
	////当接收方通过PAD接收SAMPLE时,产生该事件。
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode);
 
	//virtual MxDescriptor*      GetFormatDescriptor(CPad* pPad);	
	virtual void               SetMode(CMediaObject::WORK_MODE mode); 

	CSoundIn* GetSoundDevice(){ return (&m_soundIn); };

	BOOL SetWaveFormat(int format, int sampleRate, int channels,  int frameCount);
	BOOL SetDeviceID(DWORD dwID);

	void HandleReceivedData(void* pAudioData, DWORD dwDataSize);

private:
	DWORD ParseDeviceID(const char* strUrl);

	COutputPad  m_outputPad;
	//MxAudioDesc m_audioDesc;

	//__int64  m_nPts;

	int    m_nDeviceID;

	CSoundIn m_soundIn;

	RvSequence m_seqSample;

	CRITICAL_SECTION m_cs;
 
	
};




#endif
