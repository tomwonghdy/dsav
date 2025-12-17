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
#ifndef __AUDIO_MIX_H_INCLUDED_
#define __AUDIO_MIX_H_INCLUDED_

 
#include "amo.h"
#include "..\davsdk\includes\Transform.h"
#include "..\davsdk\includes\InputPad.h"
#include "..\davsdk\includes\OutputPad.h"

#include "..\davsdk\includes\WaveConverter.h"

#include "audiotrack.h"


#define AM_AUDIO_IN_COUNT      4
#define AM_AUDIO_OUT_COUNT     2

#define AM_TIME_LEN        50//输出达到的时间长度,单位：毫秒
#define AM_MAX_BUF_LEN     102400
#define AM_LTIME           0x0fffffffffffffff

#define AM_INPUT_1       1
#define AM_INPUT_2       (1 << 1)
#define AM_INPUT_3       (1 << 2)
#define AM_INPUT_4       (1 << 3)

#define AM_INPUT_ALL     (AM_INPUT_1 | AM_INPUT_2 | AM_INPUT_3 | AM_INPUT_4)

#define AM_OUTPUT_1        1
#define AM_OUTPUT_2        (1 << 1)

#define AM_OUTPUT_ALL  (AM_OUTPUT_1 | AM_OUTPUT_2  )


class AMO_API CAudioMix: public CTransform
{
public:
	CAudioMix(void);
	virtual ~CAudioMix(void);

	//ratio is between 0 and 10
	//normally, this value should be set such as 0.1, 0.2, 1, 2, etc.
	void  SetAmplifier(int index, float ratio);
	float GetAmplifier(int index);

	virtual M_RESULT Play(const char* strUrl );
	virtual void Pause(BOOL bResume);
	virtual void Stop();

	CInputPad*  GetInputPad(int index);
	COutputPad* GetOutputPad(int index);
 


	//virtual BOOL  CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) ;
	virtual BOOL  Accept(CPad* pInputPad, MxDescriptor* pDescriptor);
	//virtual MX_MEDIA_TYPE GetMediaType(CPad* pOutputPad);

	virtual BOOL  OnSampleReceived( CPad* pInputPad, MX_HANDLE  hSample, int* pErrCode) ;
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode){ if (pErrCode) *pErrCode = M_FAILED; return NULL;};

	virtual BOOL  OnPadConnected(CPad* pOutputPad, CPad* pInputPad);
	//virtual MxDescriptor*  GetFormatDescriptor(CPad* pPad);	

	void HandleMix();

	int  m_nInputType;
	int  m_nOutputType ;

private:

	__int64  m_pts;
	
	HANDLE m_hThreadMix;

	CInputPad   m_inputPads[AM_AUDIO_IN_COUNT];
	COutputPad  m_outputPads[AM_AUDIO_OUT_COUNT];	

	MxAudioDesc m_audioInDescs[AM_AUDIO_IN_COUNT];

	//输出音频采用统一的输出格式，简化功能实现。	
	MxAudioDesc	m_audioOutDesc;

	CAudioTrack m_audioTracks[AM_AUDIO_IN_COUNT];
	
	char* m_pTargetBuffer;

};






#endif
