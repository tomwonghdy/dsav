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
#ifndef __VMIC_FEEDER_H_INCLUDED_
#define __VMIC_FEEDER_H_INCLUDED_

#include "amo.h"


#include "..\davsdk\includes\Sink.h"
#include "..\davsdk\includes\inputpad.h"
#include "..\davsdk\includes\WaveConverter.h"



//将语音数据输出到虚拟Laodao Microphone
class AMO_API CVmicFeeder :	public CSink
{
public:
	CVmicFeeder(void);
	virtual ~CVmicFeeder(void);

	
	virtual M_RESULT Play(const char* strUrl ) ;
	virtual void     Pause(BOOL bResume) ;
	virtual void     Stop() ;
 
	CInputPad*  GetInputPad( ){return &m_inputPad;}

	//仅工作在被动模式
	virtual void       SetMode(CMediaObject::WORK_MODE mode){};

	virtual BOOL  Accept(CPad* pInputPad, MxDescriptor* pDescriptor);
	//virtual MX_MEDIA_TYPE GetMediaType(CPad* pPad) ;
	//virtual BOOL CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) ;

	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad);
	
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode);
 
	//virtual MxDescriptor*      GetFormatDescriptor(CPad* pPad);
public:

	void Reset(int channels, int bitsPerSample, int samplesPerSecond);

	//void  FetchWaveData();
protected:
	
	CInputPad   m_inputPad;
	//MxAudioDesc m_inDesc;
	MxAudioDesc m_outDesc;

	CWaveConverter     m_cvter;

	//CWaveFeed m_waveFeeder;
	//HANDLE m_hThreadVmic ;

	void* m_pClientPipe; 
	TCHAR*  m_strPipeName;
	BOOL  m_bPipeStarted ;

	CRITICAL_SECTION m_cs;
	

};



#endif