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
#ifndef __AUDIO_MIX_EX_H_INCLUDED_
#define __AUDIO_MIX_EX_H_INCLUDED_



#include "amo.h"
#include "..\davsdk\includes\Transform.h"
#include "..\davsdk\includes\InputPad.h"
#include "..\davsdk\includes\OutputPad.h"

#include "..\davsdk\includes\WaveConverter.h"

//#include "audiotrack.h"


#define AME_AUDIO_IN_COUNT      4
#define AME_AUDIO_OUT_COUNT     1

#define AME_TIME_LEN        50//输出达到的时间长度,单位：毫秒
#define AME_MAX_BUF_LEN     102400
#define AME_LTIME           0x0fffffffffffffff

#define AME_INPUT_1       1
#define AME_INPUT_2       (1 << 1)
#define AME_INPUT_3       (1 << 2)
#define AME_INPUT_4       (1 << 3)

#define AME_INPUT_ALL     (AME_INPUT_1 | AME_INPUT_2 | AME_INPUT_3 | AME_INPUT_4)
 

#define AME_OUTPUT_ALL  (AME_OUTPUT_1 | AME_OUTPUT_2  )

#define AME_MAX_FRAME_COUNT   2048

typedef struct __ame_audio_data
{
	float scalar;
	UINT   total;
	UINT   curSize;
	int   frameCount;
    char* pBuffer;
}AME_AUDIO_DATA;

class AMO_API CAudioMixEx: public CTransform
{
public:
	CAudioMixEx(void);
	virtual ~CAudioMixEx(void);

	//ratio is between 0 and 10
	//normally, this value should be set such as 0.1, 0.2, 1, 2, etc.
	void  SetAmplifier(int index, float ratio);
	float GetAmplifier(int index);

	virtual M_RESULT Play(const char* strUrl );
	virtual void Pause(BOOL bResume);
	virtual void Stop();

	CInputPad*  GetInputPad(int index);
	COutputPad* GetOutputPad();
 


	//virtual BOOL  CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) ;
	virtual BOOL  Accept(CPad* pInputPad, MxDescriptor* pDescriptor);
	//virtual MX_MEDIA_TYPE GetMediaType(CPad* pOutputPad);

	virtual BOOL  OnSampleReceived( CPad* pInputPad, MX_HANDLE  hSample, int* pErrCode){ if (pErrCode) *pErrCode = M_FAILED; return FALSE;};
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample, */int* pErrCode) ;

	virtual BOOL  OnPadConnected(CPad* pOutputPad, CPad* pInputPad);
//	virtual MxDescriptor*  GetFormatDescriptor(CPad* pPad);	

//	void HandleMix();


	void SetOutputFormat(int format, int sampleRate, int channelCount, int frameCount);

	int  m_nInputSwitches;
	 

private:
	void AddInSample(MX_HANDLE hSample, int index);
	void MergeAudio(int index, int frameCount, char* pBufferOut, UINT nBufferSize);
	
	CRITICAL_SECTION m_cs;
 

	CInputPad   m_inputPads[AME_AUDIO_IN_COUNT];
	COutputPad  m_outputPad;	

	//MxAudioDesc m_audioInDescs[AME_AUDIO_IN_COUNT];

	//输出音频采用统一的输出格式，简化功能实现。	
	//MxAudioDesc	m_audioOutDesc;

	//CAudioTrack m_audioTracks[AME_AUDIO_IN_COUNT];
	 AME_AUDIO_DATA m_audioTracks[AME_AUDIO_IN_COUNT];
	 CWaveConverter m_waveConverters[AME_AUDIO_IN_COUNT];
	 
	 char* m_pTargetBuffer;
	 
	 
	
};






#endif
