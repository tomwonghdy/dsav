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
#include "VmicFeeder.h"

#include "AudioClientPipe.h"
#include "..\davsdk\includes\ffmo.h"

#define MAX_SAMPLE_COUNT  15

CVmicFeeder::CVmicFeeder(void)
{
	m_nWorkMode = WM_PASSIVE;

	m_inputPad.m_pOwner = this;
	m_inputPad.SetCaptureType(CPad::CT_PUSH);
	m_inputPad.SetMediaType(MX_MT_AUDIO);
	ResetAudioDescEx((MxAudioDesc*)m_inputPad.GetFormatDescriptor(), MX_AF_S16, AV_DEFAULT_SAMPLE_RATE, GetAudioChannelLayout(AV_DEFAULT_CHANNELS) );
	ResetAudioDescEx(&m_outDesc, MX_AF_S16, AV_DEFAULT_SAMPLE_RATE, GetAudioChannelLayout(AV_DEFAULT_CHANNELS) );
	 

	m_strPipeName =NULL;
	m_pClientPipe = new CAudioClientPipe;
	m_bPipeStarted =FALSE;

	::InitializeCriticalSection(&m_cs);
 
}


CVmicFeeder::~CVmicFeeder(void)
{
	if (m_pClientPipe){
		  CAudioClientPipe* pVc = (CAudioClientPipe*)m_pClientPipe;
		  delete pVc;
		  m_pClientPipe =NULL;
	 }

	 if (m_strPipeName){
	     delete []m_strPipeName;
		 m_strPipeName =NULL;
	 }

	::DeleteCriticalSection(&m_cs);	
}
//
//void  CVmicFeeder::FetchWaveData()
//{
//	if (m_nWorkMode == WM_PASSIVE) return ;
//	
//	for(;;){
//		if (this->m_nState == CMediaObject::MS_STOPPED) break;	
//		if (this->m_nState == CMediaObject::MS_PAUSED){
//			Sleep(30);
//			continue; 
//		} 
//
//		if (this->m_waveFeeder.IsListFull()){
//		    Sleep(30); continue;
//		}
//
//		if ( !  m_inputPad.IsConnected())
//		{
//			Sleep(30); continue;
//		}
//		 
//		int nErrCode;
//		MX_HANDLE  hSample =   m_inputPad.Fetch(&nErrCode);
//		 
//		if (hSample){
//			MxAudioDesc* pDesc= (MxAudioDesc*)mxGetSampleDescriptor(hSample);
//
//			MX_ASSERT(mxGetSampleDataSize(hSample) == pDesc->dataSize);
//			 
//			char* pSampleData = (char*)mxGetSampleData(hSample);
//			 
//			m_waveFeeder.AppendWaveData(pSampleData, mxGetSampleDataSize(hSample));
//
//			mxDestroySample(hSample);
//		}
//		else{
//		    Sleep(10);
//		}
//		 
//	}
//}
//
//DWORD WINAPI GetVmicWaveProc(void* lpUser)
//{
//
//	CVmicFeeder* pPc = (CVmicFeeder*)lpUser;
//	
//	pPc->FetchWaveData();
//		 
//	return 0;
//}

M_RESULT CVmicFeeder::Play(const char* strUrl ) 
{
	if (m_nState != MS_STOPPED) return M_FAILED;
	if (NULL == strUrl) return M_FAILED;

	CUtAutoLock lock(&m_cs);

	MX_ASSERT(m_pClientPipe);
	CAudioClientPipe* pVcp = (CAudioClientPipe*)m_pClientPipe;
	m_bPipeStarted =pVcp->Start(strUrl );
	 
	if (m_bPipeStarted){
		Reset(pVcp->GetChannels(), pVcp->GetBitsPerSample(), pVcp->GetSamplesPerSecond() );
	}

	if (this->m_strPipeName){
	    delete []m_strPipeName;
	}
	m_strPipeName =new TCHAR[strlen(strUrl) +1];
	MX_ASSERT(m_strPipeName);
	strcpy_s(m_strPipeName, strlen(strUrl) +1, strUrl);
	 
	m_nState = MS_PLAYING;
 

	return M_OK;
}
	
void     CVmicFeeder::Pause(BOOL bResume) 
{
	m_nState = MS_PAUSED;
}
	
void     CVmicFeeder::Stop() 
{
	if (m_nState == MS_STOPPED) return;

	CUtAutoLock lock(&m_cs);

	m_nState = MS_STOPPED;
	m_bPipeStarted = FALSE;

	MX_ASSERT(m_pClientPipe);
	CAudioClientPipe* pVcp = (CAudioClientPipe*)m_pClientPipe;

	pVcp->Stop();

	m_cvter.Destroy();
}
 


BOOL  CVmicFeeder::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{
	RV_ASSERT(pInputPad == &m_inputPad);
	RV_ASSERT(pDescriptor);

	MxAudioDesc* pDesc = (MxAudioDesc*)pDescriptor;

	if (pDesc->type != MX_MT_AUDIO) return FALSE;

	MxAudioDesc* pInDesc = (MxAudioDesc*)pInputPad->GetFormatDescriptor();

	*pInDesc = *pDesc;
	 

	return TRUE;
}
//
//MX_MEDIA_TYPE CVmicFeeder::GetMediaType(CPad* pPad) 
//{
//	RV_ASSERT( pPad == &m_inputPad );
//
//	return MX_MT_AUDIO;
//}
//
//BOOL CVmicFeeder::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
//{
//	RV_ASSERT( pInputPad == &m_inputPad );
//
//	return (type == MX_MT_AUDIO);
//}


BOOL CVmicFeeder::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	return TRUE;
}


BOOL CVmicFeeder::OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)
{
	 CUtAutoLock lock(&m_cs);

	if (m_nState != MS_PLAYING){ 
	    if (pErrCode) *pErrCode = M_FAILED;
		return FALSE;
	}

	RV_ASSERT(m_pClientPipe);
	CAudioClientPipe* pVcp = (CAudioClientPipe*)m_pClientPipe;
	 
	RV_ASSERT(hSample);
	RV_ASSERT(m_strPipeName);
	
	 
	
	if (!m_bPipeStarted){
		m_bPipeStarted = pVcp->Start(m_strPipeName);
		if (m_bPipeStarted){
			Reset(pVcp->GetChannels(), pVcp->GetBitsPerSample(), pVcp->GetSamplesPerSecond());
		}
	}

	if (pVcp->IsRunning())
	{
		 
		MxAudioDesc* pInDesc = (MxAudioDesc *)mxGetSampleDescriptor(hSample);
		RV_ASSERT(pInDesc);

		//	RV_ASSERT(pInDesc->dataSize == mxGetSampleDataSize(hSample));

		//RV_ASSERT(pInDesc->channelCount== m_inDesc.channelCount);
		//RV_ASSERT(pInDesc->sampleRate == m_inDesc.sampleRate);
		//RV_ASSERT(pInDesc->format == m_inDesc.format);


		if (m_cvter.IsValid()){

			int newsize=0;
			int nSampleCount = m_cvter.GetDestSampleCount(pInDesc->sampleCount,  &newsize);
			void* pSrcData = mxGetSampleData(hSample); 
			RV_ASSERT(pSrcData);

			//output to file.
			char* pDestData =  new char[newsize];
			MX_ASSERT(pDestData);
			
	/*		MX_AUDIO_DATA* pData = new MX_AUDIO_DATA;
			RV_ASSERT(pData);
			pData->pBuffer = new char[newsize];
			RV_ASSERT(pData->pBuffer);
			pData->frameCount = nSampleCount;
			pData->dataSize = newsize;*/
			 
			BOOL ret =m_cvter.ExcuteEx((uint8_t *)pSrcData, pInDesc->dataSize,  pInDesc->sampleCount , (uint8_t *)pDestData, newsize, nSampleCount);
			RV_ASSERT(ret);

			if (!pVcp->IsListFull()){
				pVcp->AppendWaveData(pDestData, newsize);
			}
			 
			delete[]pDestData;
		}
	   
		mxDestroySample(hSample);

		if (pErrCode) *pErrCode =M_OK;

		return TRUE;
		 
	}
	else{
	    if (m_bPipeStarted){
			pVcp->Stop();
			m_cvter.Destroy();
			m_bPipeStarted =FALSE;
		}
	}
	 
	if (pErrCode) *pErrCode =M_DATA_REJECTED;

	return FALSE;
}

//
//MxDescriptor*    CVmicFeeder::GetFormatDescriptor(CPad* pPad)
//{
//	RV_ASSERT(pPad == &m_inputPad);
//
//	return (MxDescriptor*)(&m_inDesc);
//}

void CVmicFeeder::Reset(int channels, int bitsPerSample, int samplesPerSecond)
{
	
	MX_ASSERT(bitsPerSample == 16);

	 
	ResetAudioDescEx(&m_outDesc, MX_AF_S16, samplesPerSecond, GetAudioChannelLayout(channels) );
	 
	  
	if (m_cvter.IsValid()){
		m_cvter.Destroy();
	}
	 
	
	MxAudioDesc* pInDesc =(MxAudioDesc*) m_inputPad.GetFormatDescriptor();
	MX_ASSERT(pInDesc);

	BOOL ret = m_cvter.Create(pInDesc->channelLayout, pInDesc->sampleRate ,GetAudioSampleFormat(pInDesc->sampleFormat),  \
			m_outDesc.channelLayout, m_outDesc.sampleRate, GetAudioSampleFormat( m_outDesc.sampleFormat));


}
//