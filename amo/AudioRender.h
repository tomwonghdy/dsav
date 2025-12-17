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
#ifndef __AUDIO_RENDER_H_INCLUDED_
#define __AUDIO_RENDER_H_INCLUDED_

#include "amo.h"
#include "..\davsdk\includes\Render.h"
#include "..\davsdk\includes\OutputPad.h"
#include "..\davsdk\includes\InputPad.h"
#include "SoundOut.h"


class AMO_API CAudioRender :	public CRender
{
public:
	CAudioRender(void);
	virtual ~CAudioRender(void);

	virtual M_RESULT Play(const char* strUrl ) ;
	virtual void     Pause(BOOL bResume) ;
	virtual void     Stop() ;


	void SetAmplifier(float ratio);
	float GetAmplifier();


	CSoundOut* GetSoundDevice(){ return &m_soundOut;};
	

	CInputPad*   GetInputPad( ){return &m_inputPad;}
	COutputPad*  GetOutputPad( ){return &m_outputPad;}

	//virtual MX_MEDIA_TYPE GetMediaType(CPad* pPad) ;
	//virtual BOOL CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) ;

	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad);

	//当接收到一个SAMPLE的时候,产生该事件。
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode);

//	virtual MxDescriptor*      GetFormatDescriptor(CPad* pPad);	

	virtual void       SetMode(CMediaObject::WORK_MODE mode);

	virtual BOOL Accept(CPad* pInputPad, MxDescriptor* pDescriptor);
 
	void RefillData(void);

protected:
	void AdjustVolume(char* pAudioData, UINT nSize);

	CInputPad   m_inputPad;
	COutputPad  m_outputPad;

	CSoundOut   m_soundOut;

	//MxAudioDesc m_audioInDesc;//该类的输出和输入PAD的音频格式相同,	m_audioOutDesc;

	char* m_pSilenceData;

	CRITICAL_SECTION m_csLock;

private:
	float m_nScalar;

};








#endif
