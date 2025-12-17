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
#include "AudioCap.h"

#include "..\davsdk\includes\ffmo.h"

#define LAODAO_VIRTUAL_INPUT "Laodao Virtual Cable"

#define AC_MAX_BUFFER_COUNT 2


CAudioCap::CAudioCap(void)
{
	 
 	m_nStreamFlags = ST_AUDIO;

	m_outputPad.m_pOwner = this;
	m_outputPad.SetMediaType(MX_MT_AUDIO);
	SetMode(WM_ACTIVE);

	m_nDeviceID = -1;
	m_seqSample = NULL;
	 
	ResetAudioDescEx((MxAudioDesc*) m_outputPad.GetFormatDescriptor(), MX_AF_S16, AV_DEFAULT_SAMPLE_RATE, GetAudioChannelLayout(AV_DEFAULT_CHANNELS), 1032);
	  
	::InitializeCriticalSection(&m_cs);

}

CAudioCap::~CAudioCap(void)
{
	MX_ASSERT(m_seqSample ==	NULL);

	::DeleteCriticalSection(&m_cs);

}

void WINAPI DataArrivedFunc(void* pVoid, void* pAudioData, DWORD dwDataSize){
	CAudioCap* pAc = (CAudioCap*)pVoid;

	pAc->HandleReceivedData(pAudioData, dwDataSize);

}

 
void CAudioCap::HandleReceivedData(void* pAudioData, DWORD dwDataSize)
{

	
	EnterCriticalSection(&m_cs) ;

	if (m_nState != MS_PLAYING)
	{
		::LeaveCriticalSection(&m_cs);
		return;
	}

	MxAudioDesc* pAudioDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();
	RV_ASSERT(pAudioDesc);

	MX_ASSERT(pAudioData);
	MX_ASSERT(dwDataSize == pAudioDesc->dataSize);

	MX_HANDLE hSample = mxCreateSample( (MxDescriptor *)pAudioDesc, sizeof(MxAudioDesc), pAudioData, dwDataSize) ;
	MX_ASSERT( hSample );
	MX_ASSERT(  pAudioDesc->sampleRate > 0);


	 
	double dura = pAudioDesc->sampleCount / (double)(pAudioDesc->sampleRate);
	mxSetSampleOptions(hSample, m_nTotalDuation, 0, m_nCurrentPos++, dura);
	m_nTotalDuation += LONGLONG(dura*1000)/*pAudioDesc->sampleCount*/;

	if (WM_ACTIVE == m_nWorkMode)
	{	
		if (m_fnSamplePass) {
			m_fnSamplePass(this, &m_outputPad, hSample, m_pUserData);
		}

		if (m_outputPad.IsConnected()){
			if (!m_outputPad.Pass(hSample, NULL)){
				mxDestroySample(hSample);
			}
		}
		else{
			mxDestroySample(hSample);
		}
	}
	else
	{
		CUtAutoLock lock(&m_cs);

		if (m_seqSample)
		{
			if (rsqGetCount(m_seqSample) < AC_MAX_BUFFER_COUNT){
				rsqAddLast_(m_seqSample, hSample);			 
			}
			else{
				MX_HANDLE hFirstSample=NULL;

				while(rsqGetCount(m_seqSample) > AC_MAX_BUFFER_COUNT){ // > 2
					hFirstSample = rsqRemoveFirst_(m_seqSample );
					mxDestroySample(hFirstSample);
				}

				rsqAddLast_(m_seqSample,  hSample);
				//TRACE("Audio cap buffer is full, a sample was abandoned.\r\n ");
			}
		}
		else{
			mxDestroySample(hSample);
		}

	}

	::LeaveCriticalSection(&m_cs);
}

M_RESULT CAudioCap::Play(const char* strUrl ) 
{

	if (strUrl){
		SetDeviceID( ParseDeviceID(strUrl) );
		if (m_nDeviceID < 0 ) return M_INVALID_OBJECT;
	}

	m_nCurrentPos = 0;
	m_nTotalDuation = 0;

	MxAudioDesc*  pAudioDesc =(MxAudioDesc*)m_outputPad.GetFormatDescriptor();
	RV_ASSERT(pAudioDesc);

	
	WAVEFORMATEX wf;
	ResetWaveInfo(&wf, pAudioDesc);
	
	if (m_nWorkMode == WM_PASSIVE){
		CUtAutoLock lock(&m_cs);

		m_seqSample = rvCreateSequence(NULL  );
		MX_ASSERT(m_seqSample);
	}

	if (!m_soundIn.Start(m_nDeviceID, &wf, pAudioDesc->dataSize ,  DataArrivedFunc, this))
	{
		return M_FAILED;
	}

	m_nState = MS_PLAYING;


	if (m_nWorkMode == WM_PASSIVE){
		int c=0; //最多等待500毫秒
		while(rsqGetCount(m_seqSample) < 1 && c < 50)
		{
			Sleep(10); c++;
		}

		if (rsqGetCount(m_seqSample) <= 0){
			//MX_ASSERT(0);			
			Stop();
			return M_FAILED;

		}
	}	

	return M_OK;
}

void     CAudioCap::Pause(BOOL bResume) {
	 m_nState = MS_PAUSED;

}

void     CAudioCap::Stop() 
{

	 CUtAutoLock lock(&m_cs);

	 m_nState = MS_STOPPED;

	 m_soundIn.Stop();

	 if (m_seqSample){

		RvSeqReader sr;
		MX_HANDLE hTmp=NULL;

		RV_BEGIN_READ_SEQ(m_seqSample, &sr);

		while(!RV_IS_SEQ_END(&sr)){
			RV_READ_FR_SEQ_(&sr, MX_HANDLE,  hTmp);
			mxDestroySample(hTmp);
		}

		RV_END_READ_SEQ(m_seqSample, &sr);		

		rvDestroySequence(m_seqSample);
		m_seqSample = NULL;
	}

}

//
//MX_MEDIA_TYPE CAudioCap::GetMediaType(CPad* pPad){
//
//	 MX_ASSERT(pPad == &m_outputPad); 
//	 return MX_MT_AUDIO;
//}
//
//
//BOOL CAudioCap::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
//{
//	return (MX_MT_AUDIO == type);
//}


BOOL CAudioCap::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	if (pOutputPad == &m_outputPad){

		MxAudioDesc* pAudioDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();
		RV_ASSERT(pAudioDesc);

		if ( pOutputPad->NegotiateDescriptor((MxDescriptor*)pAudioDesc) ) return TRUE;
		  
	}

	return FALSE;
}


////当接收方通过PAD接收SAMPLE时,产生该事件。
MX_HANDLE CAudioCap::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode)
{

	MX_ASSERT(m_nWorkMode == WM_PASSIVE);
	//MX_ASSERT(NULL == hSample);

	CUtAutoLock lock(&m_cs);

	if (m_seqSample)
	{
		if (rsqGetCount(m_seqSample) > 0){
			MX_HANDLE hFirstSample  =NULL;
			hFirstSample = rsqRemoveFirst_(m_seqSample );
			
			if (m_fnSamplePass) {
				m_fnSamplePass(this, &m_outputPad, hFirstSample, m_pUserData);
			}

			return hFirstSample;
		}
	 
	}
	 

    return NULL;
}
//
//MxDescriptor*      CAudioCap::GetFormatDescriptor(CPad* pPad)
//{
//	return  (MxDescriptor*)(&m_audioDesc);
//}

void  CAudioCap::SetMode(CMediaObject::WORK_MODE mode){
	if (m_nWorkMode == mode) return ;

	m_nWorkMode = mode;

	if (m_nWorkMode == WM_ACTIVE)
	{
		m_outputPad.SetCaptureType(CPad::CT_PUSH);
	}
	else if (m_nWorkMode == WM_PASSIVE)
	{
		m_outputPad.SetCaptureType(CPad::CT_PULL);
	}

}


BOOL CAudioCap::SetWaveFormat(int format, int sampleRate, int channels,  int frameCount)
{ 
	int depth = GetAudioBytesPerSample(format) * 8;

	if (m_soundIn.IsFormatSupported(m_nDeviceID, sampleRate, channels, depth) == FALSE) return FALSE;

	MxAudioDesc* pAudioDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();
	RV_ASSERT(pAudioDesc);
	  
	ResetAudioDescEx(pAudioDesc, format, sampleRate, GetAudioChannelLayout(channels), frameCount);
 

	return TRUE;

}

BOOL CAudioCap::SetDeviceID(DWORD dwID)
{

	if (dwID == -1) return FALSE;

	if (m_nDeviceID == dwID) return  FALSE;

	const char* str = m_soundIn.GetDeviceDescription(dwID);

	if (NULL == str) return  FALSE;
	
	if ( str = strstr( str, LAODAO_VIRTUAL_INPUT)  ) return  FALSE;

	m_nDeviceID = dwID ;

	MxAudioDesc* pAudioDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();
	RV_ASSERT(pAudioDesc);

	if (m_soundIn.IsFormatSupported(m_nDeviceID, pAudioDesc->sampleRate, pAudioDesc->channelCount, pAudioDesc->depth)) {
		return  TRUE;
	}

	int rate_array[] = {44100, 22050, 11025 };
	
	for (int i=0; i< 3; i++){
		if (SetWaveFormat(FF_WAVE_FMT_S16, rate_array[i],  2, pAudioDesc->sampleCount)) {
			ResetAudioDescEx(pAudioDesc, FF_WAVE_FMT_S16, rate_array[i], GetAudioChannelLayout(2), pAudioDesc->sampleCount);
			return  TRUE;
		}
		if (SetWaveFormat(FF_WAVE_FMT_S16, rate_array[i],  1, pAudioDesc->sampleCount)) {
			ResetAudioDescEx(pAudioDesc, FF_WAVE_FMT_S16, rate_array[i], GetAudioChannelLayout(1), pAudioDesc->sampleCount);
			return  TRUE;
		}
		if (SetWaveFormat(FF_WAVE_FMT_U8, rate_array[i],   2, pAudioDesc->sampleCount)) {
			ResetAudioDescEx(pAudioDesc, FF_WAVE_FMT_U8, rate_array[i], GetAudioChannelLayout(2), pAudioDesc->sampleCount);
			return  TRUE;
		}
		if (SetWaveFormat(FF_WAVE_FMT_U8, rate_array[i],  1, pAudioDesc->sampleCount)) {
			ResetAudioDescEx(pAudioDesc, FF_WAVE_FMT_U8, rate_array[i], GetAudioChannelLayout(1), pAudioDesc->sampleCount);
			return  TRUE;
		}
	}

	//MX_ASSERT(0);
	return FALSE;


}

// dev://a:/wavein/0
// or    a:/wavein/0
DWORD CAudioCap::ParseDeviceID(const char* strUrl)
{
	MX_ASSERT(strUrl);
 
	strUrl = strstr(strUrl, "dev://");
	
	if (strUrl) strUrl += 6;
 
	strUrl = strstr(strUrl, "a:/");
	if (NULL == strUrl) return -1;
	strUrl += 3;
 
 
	strUrl = strstr(strUrl, "wavein/");
	
	if (NULL == strUrl) return -1;
	strUrl += 7;

	if (strUrl[0] == '\0') return -1;

	return atoi(strUrl);
  
}
 