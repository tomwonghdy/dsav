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
#include "AudioRender.h"

#include "..\davsdk\includes\ffmo.h"
#include "..\davsdk\includes\mox.h"


#define DEFAUL_RATE         44100
#define DEFAULT_CHANNELS    2
#define DEFAULT_FRAMES      AV_AUDIO_BUFFER_SIZE

#define DEFAULT_PREFILL_COUNT  3


void audio_feed_notify_callback(void* opaque)
{

	CAudioRender* pThis = (CAudioRender*)opaque;

	pThis->RefillData();
}



CAudioRender::CAudioRender(void)
{
	m_inputPad.m_pOwner = this;
	m_outputPad.m_pOwner = this;
	m_inputPad.SetMediaType(MX_MT_AUDIO);
	m_outputPad.SetMediaType(MX_MT_AUDIO);

	ResetAudioDescEx((MxAudioDesc*)m_inputPad.GetFormatDescriptor(), MX_AF_S16, DEFAUL_RATE, GetAudioChannelLayout(DEFAULT_CHANNELS), 1032);
	ResetAudioDescEx((MxAudioDesc*)m_outputPad.GetFormatDescriptor(), MX_AF_S16, DEFAUL_RATE, GetAudioChannelLayout(DEFAULT_CHANNELS), 1032);

	m_pSilenceData = NULL;

	m_nWorkMode = WM_ACTIVE;
	 
	m_inputPad.SetCaptureType(CPad::CT_PULL);
	m_outputPad.SetCaptureType(CPad::CT_PUSH);
	 
	::InitializeCriticalSection(&m_csLock);

	m_nScalar = 1;// -1;

}


CAudioRender::~CAudioRender(void)
{
	if (m_pSilenceData) {
		delete[]m_pSilenceData;
		m_pSilenceData = NULL;
	}

	::DeleteCriticalSection(&m_csLock);

}


M_RESULT CAudioRender::Play(const char* strUrl)
{



	WAVEFORMATEX waveFormat;
	MxAudioDesc* pDescIn = (MxAudioDesc*)m_inputPad.GetFormatDescriptor();
	RV_ASSERT(pDescIn);

	ResetWaveInfo(&waveFormat, pDescIn->depth, pDescIn->sampleRate, pDescIn->channelCount);

	m_pSilenceData = new char[pDescIn->dataSize/*m_audioInDesc.dataSize*/];
	MX_ASSERT(m_pSilenceData);
	memset(m_pSilenceData, GetAudioSilence((FF_WAVE_FORMAT)pDescIn->sampleFormat)  , pDescIn->dataSize);

	m_soundOut.Start(&waveFormat, audio_feed_notify_callback, this);


	if (m_nWorkMode == WM_ACTIVE)
	{
		//预先填充3组样本数据
		for (int i = 0; i < DEFAULT_PREFILL_COUNT; i++)
			m_soundOut.PlayAudio(m_pSilenceData, pDescIn->dataSize);

	}

	m_nState = MS_PLAYING;

	return M_OK;

}

void CAudioRender::Pause(BOOL bResume)
{
	;
}

void CAudioRender::Stop()
{
	::EnterCriticalSection(&m_csLock);

	m_nState = MS_STOPPED;
	m_soundOut.Stop();

	::LeaveCriticalSection(&m_csLock);

	if (m_pSilenceData) {
		delete[]m_pSilenceData;
		m_pSilenceData = NULL;
	}

}

//
//MX_MEDIA_TYPE CAudioRender::GetMediaType(CPad* pPad) 
//{
//	
//	if (pPad == &m_inputPad){
//		  return MX_MT_AUDIO;
//	 }
//	 else  if (pPad == &m_outputPad){
//		 return MX_MT_AUDIO;
//	 }
//
//	 return MX_MT_UNKNOWN;
//
//}
//
//
//MxDescriptor*  CAudioRender::GetFormatDescriptor(CPad* pPad)
//{
//	if (pPad == &m_inputPad){
//         return (MxDescriptor*)&m_audioInDesc;	
//	}
//	else if (pPad == &m_outputPad){
//	//	return (MxDescriptor*)&m_audioOutDesc;		
//		return (MxDescriptor*)&m_audioInDesc;	
//	}
//	
//	return NULL;
//}

BOOL CAudioRender::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{

	MX_ASSERT(pInputPad == &m_inputPad);

	MxAudioDesc* pDesc = (MxAudioDesc*)pDescriptor;

	MX_ASSERT(pDesc->type == MX_MT_AUDIO);
	if (pDesc->type != MX_MT_AUDIO) return FALSE;

	MxAudioDesc* pThisDesc = (MxAudioDesc*)pInputPad->GetFormatDescriptor();

	if (pDesc->depth == pThisDesc->depth && \
		pDesc->channelCount == pThisDesc->channelCount && \
		pDesc->channelLayout == pThisDesc->channelLayout && \
		pDesc->sampleFormat == pThisDesc->sampleFormat && \
		pDesc->sampleRate == pThisDesc->sampleRate \
		/* && pDesc->frameCount    == m_audioInDesc.sampleCount*/)
	{
		return TRUE;
	}

	return FALSE;

}
void CAudioRender::RefillData(void)
{
	if (m_nWorkMode == WM_PASSIVE) return;

	::EnterCriticalSection(&m_csLock);

	/*if ( m_nState == MS_STOPPED){
		::LeaveCriticalSection(& m_csLock);
		return 	;
	}*/


	MX_HANDLE  hSample = NULL;

	if (m_nState == MS_PLAYING && m_inputPad.IsConnected())
	{
		int nErrCode;
		hSample = m_inputPad.Fetch(&nErrCode);
	}

	if (hSample) {
		MxAudioDesc* pDesc = (MxAudioDesc*)mxGetSampleDescriptor(hSample);

		MX_ASSERT(mxGetSampleDataSize(hSample) == pDesc->dataSize);
		//  MX_ASSERT(  pDesc->dataSize <= m_audioInDesc.dataSize);

		char* pSampleData = (char*)mxGetSampleData(hSample);

		AdjustVolume(pSampleData, pDesc->dataSize);

		m_soundOut.PlayAudio(pSampleData, pDesc->dataSize);
		//memcpy(stream,mxGetSampleData(hSample) , len);

	}
	else {
		MxAudioDesc* pInDesc =(MxAudioDesc*) m_inputPad.GetFormatDescriptor();
		
		MX_ASSERT(m_pSilenceData && pInDesc);
		m_soundOut.PlayAudio(m_pSilenceData, pInDesc->dataSize/*m_audioInDesc.dataSize*/);

		hSample = mxCreateSample((MxDescriptor*)pInDesc /*&m_audioInDesc*/, sizeof(MxAudioDesc), \
			m_pSilenceData, pInDesc->dataSize);

		MX_ASSERT(hSample);

		/*if (this->m_pSyncTimer){
			mxSetSamplePTS(hSample, m_pSyncTimer->GetCurrentClock(m_nState == MS_PAUSED ));
		}*/

	}

	::LeaveCriticalSection(&m_csLock);

	if (m_nState == MS_PLAYING)
	{
		//将流媒体样本发送给与输出PAD连接的媒体对象.
		if (m_outputPad.IsConnected()) {

			int nErrCode;

			if (m_outputPad.Pass(hSample, &nErrCode) == FALSE) {
				mxDestroySample(hSample);
			}

		}
		else {
			mxDestroySample(hSample);
		}

	}
	else {
		mxDestroySample(hSample);
	}


}

void  CAudioRender::SetMode(CMediaObject::WORK_MODE mode)
{
	if (m_nWorkMode == mode) return;

	m_nWorkMode = mode;

	if (m_nWorkMode == WM_ACTIVE) {
		m_inputPad.SetCaptureType(CPad::CT_PULL);
		m_outputPad.SetCaptureType(CPad::CT_PUSH);
	}
	else {
		m_inputPad.SetCaptureType(CPad::CT_PUSH);
		m_outputPad.SetCaptureType(CPad::CT_PUSH);
	}


}

//BOOL CAudioRender::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
//{
//	if (pInputPad == &m_inputPad){
//		if (type == MX_MT_AUDIO) return TRUE;
//	}
//	return FALSE;
//
//}


BOOL CAudioRender::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	if (pOutputPad == &m_outputPad) {
		//return pOutputPad->NegotiateDescriptor((MxDescriptor* )&m_audioOutDesc); 
		MxAudioDesc* pDescIn = (MxAudioDesc*)m_inputPad.GetFormatDescriptor();
		RV_ASSERT(pDescIn);

		return pOutputPad->NegotiateDescriptor((MxDescriptor*)pDescIn);
	}
	else {
		return TRUE;
	}

	return FALSE;
}


BOOL CAudioRender::OnSampleReceived(CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)
{
	MX_ASSERT(hSample);

	if (m_nState == MS_STOPPED) {

		if (pErrCode) *pErrCode = M_TERMINATED;

		//TRACE("EXIT CAudioRender::OnSampleReceived \r\n");
		return FALSE;
	}

	MxAudioDesc* pDesc = (MxAudioDesc*)mxGetSampleDescriptor(hSample);

	MxAudioDesc* pDescIn = (MxAudioDesc*)m_inputPad.GetFormatDescriptor();
	RV_ASSERT(pDescIn);
	
	MX_ASSERT(mxGetSampleDataSize(hSample) == pDesc->dataSize);
	MX_ASSERT(pDesc->dataSize <= pDescIn->dataSize);

	if (m_soundOut.IsQueueFull()) {
		if (pErrCode) *pErrCode = M_BUFFER_FULL;
		return FALSE;
	}

	char* pSampleData = (char*)mxGetSampleData(hSample);

	AdjustVolume(pSampleData, pDesc->dataSize);

	CUtAutoLock lock(&m_csLock);
	m_soundOut.PlayAudio(pSampleData, pDesc->dataSize);
	 
	//将流媒体样本发送给与输出PAD连接的媒体对象.
	if (m_outputPad.IsConnected()) {

		int nErrCode;

		if (m_outputPad.Pass(hSample, &nErrCode)) {
			return TRUE;
		}
	}

	mxDestroySample(hSample);

	return TRUE; 

}

void CAudioRender::SetAmplifier(float ratio)
{
	if (ratio > 10) ratio = 10;

	m_nScalar = ratio;

}

float CAudioRender::GetAmplifier() {
	return m_nScalar;
}

void CAudioRender::AdjustVolume(char* pAudioData, UINT nSize) {
	MxAudioDesc* pDescIn = (MxAudioDesc*)m_inputPad.GetFormatDescriptor();
	RV_ASSERT(pDescIn);

	MX_ASSERT(pDescIn->depth == 16);

	if ((m_nScalar < 0) || (fabs(m_nScalar - 1.0f) < 0.0001f)) return;

	//if(m_pDstDesc->depth ==16){
	MX_ASSERT(nSize % sizeof(short) == 0);
	short* pInData = (short*)pAudioData;

	int  nLen = nSize / sizeof(short);
	int n;

	for (int i = 0; i < nLen; i++) {
		n = int(pInData[i] * m_nScalar + 0.5f);

		if (n > 0x7fff)      pInData[i] = 0x7fff;
		else if (n < -32768) pInData[i] = -32768;
		else    		  pInData[i] = (short)n;

	}

	//}
}