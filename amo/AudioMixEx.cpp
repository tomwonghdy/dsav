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
#include "AudioMixEx.h"


#include "..\davsdk\includes\ffmo.h"

//
//#define DEFAUL_RATE         44100
//#define DEFAULT_CHANNELS    2
//#define DEFAULT_FRAMES      AV_AUDIO_BUFFER_SIZE
//

#define DEFAULT_FRAME_COUNT   1132


CAudioMixEx::CAudioMixEx(void)
{

	m_outputPad.m_pOwner = this;
	m_outputPad.SetCaptureType(CPad::CT_PULL);
	m_outputPad.SetMediaType(MX_MT_AUDIO);
	MxAudioDesc* pOutDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();

	ResetAudioDescEx(pOutDesc, GetAudioSampleFormat(MX_AF_S16), AV_DEFAULT_SAMPLE_RATE, GetAudioChannelLayout(AV_DEFAULT_CHANNELS));

	for (int i=0; i< AME_AUDIO_IN_COUNT; i++){
		m_inputPads[i].m_pOwner = this;
		m_inputPads[i].SetCaptureType(CPad::CT_PULL);
		m_inputPads[i].SetMediaType(MX_MT_AUDIO);

		MxAudioDesc* pInDesc = (MxAudioDesc*)m_inputPads[i].GetFormatDescriptor();
		*pInDesc = *pOutDesc;  
	}

	 
	
	
	m_pTargetBuffer =  NULL; 
	m_nInputSwitches  = AME_INPUT_ALL; 
	 
	m_nWorkMode = WM_ACTIVE;
	 
	for (int i=0; i< AME_AUDIO_IN_COUNT; i++)
	{
		 
		m_audioTracks[i].scalar = 1.0f;
		m_audioTracks[i].curSize = 0 ;
		m_audioTracks[i].total  = 0;
		m_audioTracks[i].frameCount = 0;
		m_audioTracks[i].pBuffer = NULL;
	}

	::InitializeCriticalSection(&m_cs);
	
}

CAudioMixEx::~CAudioMixEx(void)
{
	for (int i=0; i < AME_AUDIO_IN_COUNT; i++)
	{
		if (m_audioTracks[i].pBuffer){
			 delete[] m_audioTracks[i].pBuffer;
		}
	}


	::DeleteCriticalSection(&m_cs);
}


void  CAudioMixEx::SetAmplifier(int index, float ratio)
{
	if (index >=0 && index < AME_AUDIO_IN_COUNT)
	{
		if (ratio < 0.f) ratio =0.f;
		if (ratio > 10.f) ratio =10.0f;

		m_audioTracks[index].scalar =ratio;
		// m_audioTracks[index].m_nScalar = ratio;
	}
}

float CAudioMixEx::GetAmplifier(int index)
{
	if (index >=0 && index < AME_AUDIO_IN_COUNT)
	{
		return m_audioTracks[index].scalar;//m_audioTracks[index].m_nScalar  ;
	}
	return 0;

}

CInputPad*  CAudioMixEx::GetInputPad(int index)
{
	if (index < 0 || index >= AME_AUDIO_IN_COUNT) return NULL;

	return (m_inputPads + index);

}

COutputPad* CAudioMixEx::GetOutputPad( ){
	
	return (&m_outputPad);
}

//
//DWORD WINAPI ThreadProcPlay(LPVOID lpParameter)
//{
//	CAudioMixEx* pDa = (CAudioMixEx* )lpParameter;
//
//	//HANDLE hWaitEvents[MA_STREAME_COUNT +1] ;
//	/*hWaitEvents[0] =  pDa->m_hExitEvent;
//	
//	for (int i=1; i<=MA_STREAME_COUNT; i++){
//	    hWaitEvents[i] = pDa->m_hDataReceipts[i-1]; 
//	}
//	 
//	while(1)
//	{
//		if ((pDa->m_hExitEvent, 0) == WAIT_OBJECT_0) break;
//
//		if (::WaitForMultipleObjects(MA_STREAME_COUNT, hWaitEvents, TRUE, 10) == WAIT_TIMEOUT) continue;
//
//	}
//
//	*/
//
//
//	return 0;
//
//}


//
//DWORD WINAPI ThreadAudioMixExProc(  LPVOID lpParameter)
//{
//	CAudioMixEx* pAm = (CAudioMixEx* )lpParameter;
//	
//
//
//	pAm->HandleMix();
//	
//
//	return 0;
//}


M_RESULT CAudioMixEx::Play(const char* strUrl )
{
	if (m_nState == MS_PLAYING) return M_FAILED;

	CUtAutoLock lock(&m_cs);

	MX_ASSERT(NULL == m_pTargetBuffer);
	    
	MxAudioDesc* pOutDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();

	UINT maxDataSize  =  GetAudioBufferSize(pOutDesc->sampleFormat, pOutDesc->channelCount, AME_MAX_FRAME_COUNT);  ;

	m_pTargetBuffer =new char[maxDataSize];
	if (NULL == m_pTargetBuffer) return M_MEM_ALLOC;
	 
	m_nCurrentPos = 0;
	//memset( m_audioTracks, 0, sizeof(m_audioTracks));
	 
	for (int i=0; i< AME_AUDIO_IN_COUNT; i++)
	{
		MxAudioDesc* pInDesc = (MxAudioDesc*)m_inputPads[i].GetFormatDescriptor();
		BOOL ret = m_waveConverters[i].Create(pInDesc->channelLayout, pInDesc->sampleRate,  GetAudioSampleFormat(pInDesc->sampleFormat), \
			pOutDesc->channelLayout, pOutDesc->sampleRate, GetAudioSampleFormat(pOutDesc->sampleFormat));
		MX_ASSERT(ret);
		 
		m_audioTracks[i].curSize = 0 ;
		m_audioTracks[i].total  = 0;
		m_audioTracks[i].frameCount = 0;
		MX_ASSERT(m_audioTracks[i].pBuffer == NULL);
		 
	}
	 

	m_nState = MS_PLAYING;

	//m_hThreadMix = ::CreateThread(NULL, 0, ThreadAudioMixExProc, this, 0,NULL);
	//MX_ASSERT(m_hThreadMix);


	return M_OK;
}

void CAudioMixEx::Pause(BOOL bResume){
	m_nState = MS_PAUSED;
}

void CAudioMixEx::Stop()
{
	if (m_nState == MS_STOPPED) return;

	CUtAutoLock lock(&m_cs);

	m_nState = MS_STOPPED;

	/*if (m_hThreadMix){
	::WaitForSingleObject(m_hThreadMix, INFINITE);
	::CloseHandle(m_hThreadMix );
	m_hThreadMix = NULL;
	}*/
	
	for (int i=0; i < AME_AUDIO_IN_COUNT; i++)
	{
		 m_waveConverters[i].Destroy();

		if (m_audioTracks[i].pBuffer){
			delete[] m_audioTracks[i].pBuffer;

			m_audioTracks[i].pBuffer = NULL;
		}

	}


	if (m_pTargetBuffer){
		delete []m_pTargetBuffer;
		m_pTargetBuffer = NULL;
	}

}
//
//void CAudioMixEx::HandleMix()
//{
//
//	MX_ASSERT(m_pTargetBuffer);
//
//
//	/*HANDLE bufferFullEvents[AME_AUDIO_IN_COUNT] ;
//	int    eventCount =0;
//
//	for (int i=0; i< AME_AUDIO_IN_COUNT; i++){
//
//	if (m_inputPads[i].IsConnected()){
//
//	bufferFullEvents[i] = m_audioTracks[i].GetBufferFullEvent();
//	MX_ASSERT(bufferFullEvents[i]);
//
//	eventCount++;
//	}
//
//	}*/
//
//	MX_HANDLE hSample=NULL;
//	while(m_nState == MS_PLAYING){
//	//	if (::WaitForMultipleObjects(eventCount, bufferFullEvents, TRUE, 50) != WAIT_OBJECT_0)  continue;
//
//		memset(m_pTargetBuffer, pOutDesc->silence, this->pOutDesc->dataSize);
//
//		//读取每个输入流的数据，里面自动添加。
//		for (int i=0; i< AME_AUDIO_IN_COUNT; i++){
//			if (m_inputPads[i].IsConnected()){
//
//				//::ResetEvent(bufferFullEvents[i]);		
//
//				//m_audioTracks[i].Read(m_pTargetBuffer, pOutDesc->dataSize, FALSE);						 
//			}
//		}	
//
//		hSample =  mxCreateSample((MxDescriptor*)&m_audioOutDesc, sizeof(m_audioOutDesc), m_pTargetBuffer, pOutDesc->dataSize);
//		MX_ASSERT(hSample);
//
//		__int64 pts = av_gettime();
//		mxSetSampleOptions(hSample, &pts, NULL, NULL );
//
//		/*if (this->m_pSyncTimer){
//			mxSetSamplePTS(hSample, m_pSyncTimer->GetCurrentClock(m_nState == MS_PAUSED) );
//		}*/
//
//		//通过输出PAD进行输出
//		for (int i=0; i< AME_AUDIO_OUT_COUNT; i++){
//			if (m_outputPads[i].IsConnected()){			
//
//				int nErrCode;
//				if (m_outputPads[i].Pass(hSample, &nErrCode) ==TRUE){
//
//					hSample =  mxCreateSample((MxDescriptor*)&m_audioOutDesc, sizeof(m_audioOutDesc), m_pTargetBuffer, pOutDesc->dataSize);
//		            MX_ASSERT(hSample);
//
//					//pts = av_gettime();
//					mxSetSampleOptions(hSample, &pts, NULL, NULL );
//
//					/*if (this->m_pSyncTimer){
//						mxSetSamplePTS(hSample, m_pSyncTimer->GetCurrentClock(m_nState == MS_PAUSED));
//					}*/
//				}
//			}			
//		}
//
//		mxDestroySample(hSample);
//
//	}
//
//}


 //BOOL CAudioMixEx::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
 //{
	// for (int i=0; i< AME_AUDIO_IN_COUNT; i++)
	// {
	//	 if (pInputPad == &m_inputPads[i]){
	//		 if (type == MX_MT_AUDIO) return TRUE;
	//	 }
	// }

	// return FALSE;

 //}

 //
 //MX_MEDIA_TYPE CAudioMixEx::GetMediaType(CPad* pOutputPad)
 //{
	// 
	// 
	// if (pOutputPad == &m_outputPad ) return MX_MT_AUDIO;
	// 

	// return MX_MT_UNKNOWN;

 //}

BOOL CAudioMixEx::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{

	MX_ASSERT(pInputPad && pDescriptor);

	MxAudioDesc * pDesc =(MxAudioDesc *) pDescriptor;

	MX_ASSERT(pDesc->type == MX_MT_AUDIO);
	if (pDesc->type != MX_MT_AUDIO) return FALSE;


	for (int i=0; i< AME_AUDIO_IN_COUNT; i++)
	{
		if (pInputPad == &m_inputPads[i]){
			MxAudioDesc* pInDesc = (MxAudioDesc*)m_inputPads[i].GetFormatDescriptor();
			*pInDesc = *pDesc;
			//m_audioInDescs[i] = *pDesc;		  
		}
	}
	
	return TRUE;

}

//
//BOOL CAudioMixEx::OnSampleReceived( CPad* pInputPad, MX_HANDLE  hSample, int* pErrCode) 
//{
//
//	
//
//}



BOOL CAudioMixEx::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	 
	if (pOutputPad == &m_outputPad  ){	
		MxAudioDesc* pOutDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();
		return pOutputPad->NegotiateDescriptor((MxDescriptor* )pOutDesc);
	}
	 

	for (int i=0; i< AME_AUDIO_IN_COUNT; i++){
		if (pInputPad == &m_inputPads[i] ){	 
			if (i == 0)      return ((AME_INPUT_1 & m_nInputSwitches) != 0);
			else if (i == 1) return ((AME_INPUT_2 & m_nInputSwitches) != 0);
			else if (i == 2) return ((AME_INPUT_3 & m_nInputSwitches) != 0);
			else if (i == 3) return ((AME_INPUT_4 & m_nInputSwitches) != 0);
		}
	}
	
	return TRUE;

} 

//
//MxDescriptor*  CAudioMixEx::GetFormatDescriptor(CPad* pPad)
//{
//	for (int i=0; i < AME_AUDIO_IN_COUNT; i++){
//		if (pPad == &m_inputPads[i]){
//			return (MxDescriptor*)&m_audioInDescs[i];	
//		}
//	}
//
//
//	if (pPad == &m_outputPad ){
//		return (MxDescriptor*)&m_audioOutDesc;	
//	}
//
//
//	return NULL;
//}



MX_HANDLE  CAudioMixEx::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode) 
{
	CUtAutoLock lock(&m_cs);

	if (m_nState != MS_PLAYING){
		if (pErrCode) *pErrCode = M_TERMINATED;
		return NULL;
	}

	MX_ASSERT( pOutputPad == &m_outputPad);

	MX_HANDLE hTemp;
	MxAudioDesc* pOutDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();
	int min_frame_count = AME_MAX_FRAME_COUNT + 1  ;
  
	for (int i=0; i< AME_AUDIO_IN_COUNT; i++)
	{ 
		if ( (m_audioTracks[i].frameCount < AME_MAX_FRAME_COUNT) && m_inputPads[i].IsConnected())
		{
			hTemp = m_inputPads[i].Fetch(NULL);

			if (hTemp)
			{
				AddInSample(hTemp, i);

				mxDestroySample(hTemp);
			}
		}

		if ((m_audioTracks[i].frameCount > 0) && (m_audioTracks[i].frameCount < min_frame_count))
		{
			min_frame_count = m_audioTracks[i].frameCount;
		}
	}


	//如果没有一路输入的输入数据存在帧数据，产生一个静音数据。
	if ((AME_MAX_FRAME_COUNT + 1 ) == min_frame_count)
	{
		min_frame_count = DEFAULT_FRAME_COUNT;
	} 

 
	UINT  dataSize =  GetAudioBufferSize(pOutDesc->sampleFormat, pOutDesc->channelCount, min_frame_count);

	memset(m_pTargetBuffer, GetAudioSilence((FF_WAVE_FORMAT)pOutDesc->sampleFormat)/*pOutDesc->silence*/, dataSize);

	for (int i=0; i< AME_AUDIO_IN_COUNT; i++)
	{ 
		if (m_audioTracks[i].frameCount > 0 )
		{ 
			 MergeAudio(i, min_frame_count, m_pTargetBuffer, dataSize);
		}
	}

	pOutDesc->sampleCount = min_frame_count;
	pOutDesc->dataSize = dataSize;

	hTemp = mxCreateSample((MxDescriptor *)pOutDesc,sizeof(MxAudioDesc), m_pTargetBuffer, dataSize);
	MX_ASSERT(hTemp);

	//__int64 pts = GetAvTime();
	double dura = pOutDesc->sampleCount / (double)(pOutDesc->sampleFormat);
	mxSetSampleOptions(hTemp,   m_nCurrentPos , 0, m_nCurrentPos, dura);
	m_nCurrentPos++;
	 
	if (pErrCode) *pErrCode = M_OK;

	return hTemp;
}

void CAudioMixEx::AddInSample(MX_HANDLE hSample, int index)
{
	MX_ASSERT(index >= 0 && index < AME_AUDIO_IN_COUNT);

	MX_ASSERT(hSample);

	void* pSampleData = mxGetSampleData(hSample); 
	MxAudioDesc* pAudioDesc =(MxAudioDesc*) mxGetSampleDescriptor(hSample);

	int newsize= 0;
	int nSampleCount = m_waveConverters[index].GetDestSampleCount(pAudioDesc->sampleCount,  &newsize);
	 
	MX_ASSERT(newsize>0);
 
	if ((m_audioTracks[index].total - m_audioTracks[index].curSize) < newsize) 
	{
		UINT totalSize = newsize + m_audioTracks[index].curSize;
		char* buf = new char[totalSize];
		MX_ASSERT(buf);

		if (m_audioTracks[index].pBuffer){
			MX_ASSERT(m_audioTracks[index].curSize >= 0 && m_audioTracks[index].curSize <= m_audioTracks[index].total );

			if (m_audioTracks[index].curSize>0) memcpy(buf, m_audioTracks[index].pBuffer, m_audioTracks[index].curSize);

			delete[] m_audioTracks[index].pBuffer;
		}

		m_audioTracks[index].pBuffer = buf;
		m_audioTracks[index].total = totalSize;

	}

	MX_ASSERT(m_audioTracks[index].pBuffer);

	BOOL ret =m_waveConverters[index].ExcuteEx((uint8_t *)pSampleData, pAudioDesc->dataSize,  pAudioDesc->sampleCount , (uint8_t *)m_audioTracks[index].pBuffer + m_audioTracks[index].curSize, newsize, nSampleCount);		
	MX_ASSERT(ret);
  
	m_audioTracks[index].curSize += newsize;
	m_audioTracks[index].frameCount += nSampleCount;

	 
}


void CAudioMixEx::MergeAudio(int index, int frameCount, char* pBufferOut, UINT nBufferSize)
{
	MX_ASSERT(index >= 0 && index < AME_AUDIO_IN_COUNT);
	 
	MxAudioDesc* pOutDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();

	CUtAutoLock lock(&m_cs);

	MX_ASSERT(m_audioTracks[index].pBuffer);
	MX_ASSERT(pBufferOut);

	MX_ASSERT(m_audioTracks[index].curSize >= nBufferSize);
	MX_ASSERT(m_audioTracks[index].frameCount >= frameCount);


	if(pOutDesc->depth ==16){
		MX_ASSERT(nBufferSize % sizeof(short) == 0);

		short* pOutData = (short*)pBufferOut;
		short* pInData = (short*)m_audioTracks[index].pBuffer;

		int    nLen = nBufferSize / sizeof(short);
		int n;

		for (int i=0; i< nLen; i++){
			n = pOutData[i] + int( pInData[i] * m_audioTracks[index].scalar + 0.5f);

			if(n>0x7fff)      pOutData[i] = 0x7fff;
			else if(n<-32768) pOutData[i] = -32768;
			else    		   pOutData[i] = (short)n;

		}

	}
	else if(pOutDesc->depth == 8){
		unsigned char* pOutData = (unsigned char*)pBufferOut;
		unsigned char* pInData = (unsigned char*)m_audioTracks[index].pBuffer;
		int n;
		int    nLen = nBufferSize;

		for (int i=0; i< nLen; i++){
			n = (pOutData[i]-128) +  char( (pInData[i] -128)* m_audioTracks[index].scalar + 0.5f);

			if(n>127)      n = 127;
			else if(n<-128) n =-128;				 

			MX_ASSERT(n+128>=0 && (n+128) < 256);

			pOutData[i] = n+128;
		}

	}
	else{
		MX_ASSERT(0);
	}
 
	memmove(m_audioTracks[index].pBuffer, m_audioTracks[index].pBuffer + nBufferSize, m_audioTracks[index].curSize - nBufferSize);

	m_audioTracks[index].curSize -= nBufferSize;
	m_audioTracks[index].frameCount -= frameCount;
  

}


void CAudioMixEx::SetOutputFormat(int format, int sampleRate, int channelCount,   int frameCount)
{
	MX_ASSERT(frameCount > 0);

	MxAudioDesc* pOutDesc = (MxAudioDesc*)m_outputPad.GetFormatDescriptor();

	ResetAudioDescEx(pOutDesc, format, sampleRate, GetAudioChannelLayout(channelCount), frameCount);
}
