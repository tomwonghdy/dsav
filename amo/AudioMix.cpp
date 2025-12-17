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
#include "AudioMix.h"


#include "..\davsdk\includes\ffmo.h"

//
//#define DEFAUL_RATE         44100
//#define DEFAULT_CHANNELS    2
//#define DEFAULT_FRAMES      AV_AUDIO_BUFFER_SIZE
//



CAudioMix::CAudioMix(void)
{
	m_pts = 0;

	ResetAudioDescEx( &m_audioOutDesc , GetAudioSampleFormat(MX_AF_S16), AV_DEFAULT_SAMPLE_RATE, GetAudioChannelLayout(AV_DEFAULT_CHANNELS));

	for (int i=0; i< AM_AUDIO_IN_COUNT; i++){
		m_inputPads[i].m_pOwner = this;
		m_inputPads[i].SetCaptureType(CPad::CT_PUSH);

		m_audioInDescs[ i]= m_audioOutDesc;
		
		//::InitializeCriticalSection(&m_csSampleArrays[i]);
		
	}

	for (int i=0; i< AM_AUDIO_OUT_COUNT; i++){
		m_outputPads[i].m_pOwner = this;
		m_outputPads[i].SetCaptureType(CPad::CT_PUSH);
	}
	
	m_pTargetBuffer =  NULL;
  
	m_nOutputType = AM_OUTPUT_ALL ;
	m_nInputType  = AM_INPUT_ALL; 


	m_nWorkMode = WM_HYBRID;

	 
	
}

CAudioMix::~CAudioMix(void)
{
}


void  CAudioMix::SetAmplifier(int index, float ratio)
{
	if (index >=0 && index < AM_AUDIO_IN_COUNT)
	{
		if (ratio < 0.f) ratio =0.f;
		if (ratio > 10.f) ratio =10.0f;

		 m_audioTracks[index].m_nScalar = ratio;
	}
}

float CAudioMix::GetAmplifier(int index)
{
	if (index >=0 && index < AM_AUDIO_IN_COUNT)
	{
		return m_audioTracks[index].m_nScalar  ;
	}
	return 0;

}

CInputPad*  CAudioMix::GetInputPad(int index)
{
	if (index < 0 || index >= AM_AUDIO_IN_COUNT) return NULL;

	return (m_inputPads + index);

}

COutputPad* CAudioMix::GetOutputPad(int index){
	
	if (index < 0 || index >= AM_AUDIO_OUT_COUNT) return NULL;

	return (m_outputPads + index);
}

//
//DWORD WINAPI ThreadProcPlay(LPVOID lpParameter)
//{
//	CAudioMix* pDa = (CAudioMix* )lpParameter;
//
//	//HANDLE hWaitEvents[MA_STREAM_COUNT +1] ;
//	/*hWaitEvents[0] =  pDa->m_hExitEvent;
//	
//	for (int i=1; i<=MA_STREAM_COUNT; i++){
//	    hWaitEvents[i] = pDa->m_hDataReceipts[i-1]; 
//	}
//	 
//	while(1)
//	{
//		if ((pDa->m_hExitEvent, 0) == WAIT_OBJECT_0) break;
//
//		if (::WaitForMultipleObjects(MA_STREAM_COUNT, hWaitEvents, TRUE, 10) == WAIT_TIMEOUT) continue;
//
//	}
//
//	*/
//
//
//	return 0;
//
//}



DWORD WINAPI ThreadAudioMixProc(  LPVOID lpParameter)
{
	CAudioMix* pAm = (CAudioMix* )lpParameter;
	


	pAm->HandleMix();
	

	return 0;
}


M_RESULT CAudioMix::Play(const char* strUrl )
{
	if (m_nState == MS_PLAYING) return M_FAILED;

	MX_ASSERT(NULL == m_pTargetBuffer);
	MX_ASSERT(m_audioOutDesc.dataSize > 0);

	m_pts = 0;
	m_pTargetBuffer =new char[this->m_audioOutDesc.dataSize];
	if (NULL == m_pTargetBuffer) return M_MEM_ALLOC;

	for (int i=0; i< AM_AUDIO_IN_COUNT; i++){

		BOOL ret =m_audioTracks[i].Start(&this->m_audioInDescs[i], &m_audioOutDesc);
		MX_ASSERT(ret);
	}

	m_nState = MS_PLAYING;

	m_hThreadMix = ::CreateThread(NULL, 0, ThreadAudioMixProc, this, 0,NULL);
	MX_ASSERT(m_hThreadMix);


	return M_OK;
}

void CAudioMix::Pause(BOOL bResume){
	m_nState = MS_PAUSED;
}

void CAudioMix::Stop()
{

	m_nState = MS_STOPPED;

	if (m_hThreadMix){
		::WaitForSingleObject(m_hThreadMix, INFINITE);
		::CloseHandle(m_hThreadMix );
		m_hThreadMix = NULL;
	}
	
	for (int i=0; i< AM_AUDIO_IN_COUNT; i++){
		m_audioTracks[i].Stop();
	}


	if (m_pTargetBuffer){
		delete []m_pTargetBuffer;
		m_pTargetBuffer = NULL;
	}

}

void CAudioMix::HandleMix()
{

	MX_ASSERT(m_pTargetBuffer);


	HANDLE bufferFullEvents[AM_AUDIO_IN_COUNT] ;
	int    eventCount =0;

	for (int i=0; i< AM_AUDIO_IN_COUNT; i++){

		if (m_inputPads[i].IsConnected()){

		   bufferFullEvents[i] = m_audioTracks[i].GetBufferFullEvent();
		   MX_ASSERT(bufferFullEvents[i]);

		   eventCount++;
		}

	}

	MX_HANDLE hSample=NULL;
	while(m_nState == MS_PLAYING){
		if (::WaitForMultipleObjects(eventCount, bufferFullEvents, TRUE, 50) != WAIT_OBJECT_0)  continue;

		memset(m_pTargetBuffer, GetAudioSilence((FF_WAVE_FORMAT)m_audioOutDesc.sampleFormat )/*m_audioOutDesc.silence*/, this->m_audioOutDesc.dataSize);

		//读取每个输入流的数据，里面自动添加。
		for (int i=0; i< AM_AUDIO_IN_COUNT; i++){
			if (m_inputPads[i].IsConnected()){

				::ResetEvent(bufferFullEvents[i]);		

				m_audioTracks[i].Read(m_pTargetBuffer, m_audioOutDesc.dataSize, FALSE);						 
			}
		}	

		hSample =  mxCreateSample((MxDescriptor*)&m_audioOutDesc, sizeof(m_audioOutDesc), m_pTargetBuffer, m_audioOutDesc.dataSize);
		MX_ASSERT(hSample);

		//__int64 pts = GetAvTime(); 
		mxSetSampleOptions(hSample, m_pts++, 0, 0 );
		m_pts++;

		/*if (this->m_pSyncTimer){
			mxSetSamplePTS(hSample, m_pSyncTimer->GetCurrentClock(m_nState == MS_PAUSED) );
		}*/

		//通过输出PAD进行输出
		for (int i=0; i< AM_AUDIO_OUT_COUNT; i++){
			if (m_outputPads[i].IsConnected()){			

				int nErrCode;
				if (m_outputPads[i].Pass(hSample, &nErrCode) ==TRUE){

					hSample =  mxCreateSample((MxDescriptor*)&m_audioOutDesc, sizeof(m_audioOutDesc), m_pTargetBuffer, m_audioOutDesc.dataSize);
		            MX_ASSERT(hSample);

					//pts = av_gettime();

					mxSetSampleOptions(hSample, m_pts++, NULL, NULL );

					/*if (this->m_pSyncTimer){
						mxSetSamplePTS(hSample, m_pSyncTimer->GetCurrentClock(m_nState == MS_PAUSED));
					}*/
				}
			}			
		}

		mxDestroySample(hSample);

	}

}


 //BOOL CAudioMix::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
 //{
	// for (int i=0; i< AM_AUDIO_IN_COUNT; i++)
	// {
	//	 if (pInputPad == &m_inputPads[i]){
	//		 if (type == MX_MT_AUDIO) return TRUE;
	//	 }
	// }

	// return FALSE;

 //}

 //
 //MX_MEDIA_TYPE CAudioMix::GetMediaType(CPad* pOutputPad)
 //{
	// for (int i=0; i <AM_AUDIO_OUT_COUNT; i++){
	//     if (pOutputPad == &m_outputPads[i]) return MX_MT_AUDIO;
	// }

	// return MX_MT_UNKNOWN;

 //}

BOOL CAudioMix::Accept(CPad* pInputPad, MxDescriptor* pDescriptor)
{

	MX_ASSERT(pInputPad && pDescriptor);

	MxAudioDesc * pDesc =(MxAudioDesc *) pDescriptor;

	MX_ASSERT(pDesc->type == MX_MT_AUDIO);
	if (pDesc->type != MX_MT_AUDIO) return FALSE;


	for (int i=0; i< AM_AUDIO_IN_COUNT; i++)
	{
		if (pInputPad == &m_inputPads[i]){
			m_audioInDescs[i] = *pDesc;		  
		}
	}
	
	return TRUE;

}


BOOL CAudioMix::OnSampleReceived( CPad* pInputPad, MX_HANDLE  hSample, int* pErrCode) 
{

	if (m_nState != MS_PLAYING){
		if (pErrCode) *pErrCode = M_TERMINATED;
		return FALSE;
	}

	MX_ASSERT(hSample && pInputPad);

	for (int i=0; i< AM_AUDIO_IN_COUNT; i++)
	{
		if (pInputPad == &m_inputPads[i]){
			
			if (this->m_audioTracks[i].AddSample(hSample))
			{
				return TRUE;
			}

			break;
		}
	}
	
	if (pErrCode) *pErrCode = M_DATA_REJECTED;

	return FALSE;

}



BOOL CAudioMix::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	for (int i=0; i< AM_AUDIO_OUT_COUNT; i++){
		if (pOutputPad == &m_outputPads[i] ){	

			if (i == 0 && (AM_OUTPUT_1 & m_nOutputType))  
				return pOutputPad->NegotiateDescriptor((MxDescriptor* )&m_audioOutDesc); 
			else if (i == 1 && (AM_OUTPUT_2 & m_nOutputType))  
				return pOutputPad->NegotiateDescriptor((MxDescriptor* )&m_audioOutDesc); 

		}
	}

	for (int i=0; i< AM_AUDIO_IN_COUNT; i++){
		if (pInputPad == &m_inputPads[i] ){	 
			if (i == 0)      return ((AM_INPUT_1 & m_nInputType) != 0);
			else if (i == 1) return ((AM_INPUT_2 & m_nInputType) != 0);
			else if (i == 2) return ((AM_INPUT_3 & m_nInputType) != 0);
			else if (i == 3) return ((AM_INPUT_4 & m_nInputType) != 0);
		}
	}
	
	return TRUE;

} 

//
//MxDescriptor*  CAudioMix::GetFormatDescriptor(CPad* pPad)
//{
//	for (int i=0; i < AM_AUDIO_IN_COUNT; i++){
//		if (pPad == &m_inputPads[i]){
//			return (MxDescriptor*)&m_audioInDescs[i];	
//		}
//	}
//
//	for (int i=0; i < AM_AUDIO_OUT_COUNT; i++){
//		if (pPad == &m_outputPads[i]){
//			return (MxDescriptor*)&m_audioOutDesc;	
//		}
//	}
// 
//	return NULL;
//}