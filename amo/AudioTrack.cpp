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
#include "AudioTrack.h"

#include <rvb\dsm.h>

#define MAX_SAMPLE_COUNT    1024


CAudioTrack::CAudioTrack(void)
{

	 
	m_pAudioBuffer = NULL;
	
	m_pSrcDesc = m_pDstDesc = NULL;

	m_bStarted = FALSE;

	m_nScalar = 1.0f;

	m_hBufferFullEvent =::CreateEvent(NULL,TRUE, FALSE, NULL);
	MX_ASSERT(m_hBufferFullEvent);
	
	m_hBufferEmptyEvent = ::CreateEvent(NULL,TRUE, FALSE, NULL);
	MX_ASSERT(m_hBufferEmptyEvent);
	
	m_arrSample = rvCreateSequence(NULL);
	MX_ASSERT(m_arrSample);

	

	::InitializeCriticalSection(&m_csSample);
	::InitializeCriticalSection(&m_csBuffer);
	

	m_hProcThread = NULL;

}

CAudioTrack::~CAudioTrack(void)
{
	 MX_SAFE_CLOSE_HANDLE(m_hBufferFullEvent);
	 MX_SAFE_CLOSE_HANDLE(m_hBufferEmptyEvent);
	
	 if (m_arrSample){
		 
		 RemoveAllSample();

		 rvDestroySequence(m_arrSample);
		 m_arrSample = NULL;
	 }

	 ::DeleteCriticalSection(&m_csSample);
	 ::DeleteCriticalSection(&m_csBuffer);

}


BOOL  CAudioTrack::AddSample(MX_HANDLE hSample)
{
	CUtAutoLock lock(&m_csSample);

	if (m_bStarted ==FALSE ) return FALSE;

	if (MAX_SAMPLE_COUNT <= rsqGetCount(m_arrSample) /*m_arrSample.GetCount()*/) return FALSE;

	//m_arrSample.AddTail(hSample);
	rsqAddLast_(m_arrSample, hSample);

	MX_ASSERT(m_hSemaSamples);
	::ReleaseSemaphore(m_hSemaSamples, 1, NULL);

	return TRUE;
}


MX_HANDLE CAudioTrack::GetSample()
{
	CUtAutoLock lock(&m_csSample);

	MX_HANDLE hSample=NULL;

	MX_HANDLE* pTmp;

	if ( rsqGetCount(m_arrSample) /*m_arrSample.GetCount()*/>0){
		pTmp = (MX_HANDLE*)rsqGetFirst_(m_arrSample);//m_arrSample.GetHead();
		MX_ASSERT(pTmp);

		hSample =  *pTmp;

		//m_arrSample.RemoveHead();
		rsqRemoveFirst_(m_arrSample);
	}

	return hSample;
}

DWORD WINAPI ThreadAudioTrackProc(  LPVOID lpParameter)
{
	CAudioTrack* pAt = (CAudioTrack* )lpParameter;
	
	pAt->HandleSample();

	 
	return 0;
}




BOOL  CAudioTrack::Start(MxAudioDesc* pSrcDesc, MxAudioDesc* pDestDesc )
{
	if (m_bStarted) return FALSE;

	MX_ASSERT(pSrcDesc && pDestDesc);
 
	m_nBufferSize = max(pDestDesc->dataSize, pSrcDesc->dataSize) * 2;
	MX_ASSERT(m_nBufferSize>0);

	m_pAudioBuffer = new char[m_nBufferSize];

	MX_ASSERT(m_pAudioBuffer);

	if (NULL ==	m_pAudioBuffer) return FALSE;

	m_pSrcDesc = pSrcDesc;
	m_pDstDesc = pDestDesc;

	BOOL ret = m_waveConverter.Create( pSrcDesc->channelLayout, pSrcDesc->sampleRate,  GetAudioSampleFormat(pSrcDesc->sampleFormat), \
		                             pDestDesc->channelLayout, pDestDesc->sampleRate, GetAudioSampleFormat(pDestDesc->sampleFormat));

	if (!ret){
		delete[] m_pAudioBuffer;
		m_pAudioBuffer = NULL;
	}

	m_bStarted =TRUE;
	m_nCurPos =0;

	::SetEvent(this->m_hBufferEmptyEvent);

	m_hSemaSamples = ::CreateSemaphore(NULL, 0, MAX_SAMPLE_COUNT, NULL);
	MX_ASSERT(m_hSemaSamples);
	
	
	m_hProcThread = ::CreateThread(NULL, 0, ThreadAudioTrackProc, this, 0,NULL);
	MX_ASSERT(m_hProcThread);



	return ret;

}

void  CAudioTrack::Stop()
{
	if (!m_bStarted) return  ;

	m_bStarted =FALSE;

	if (m_hProcThread){

		::WaitForSingleObject(m_hProcThread, INFINITE);

		::CloseHandle(m_hProcThread );
		m_hProcThread = NULL;
	}

	if (m_pAudioBuffer){
		delete[] m_pAudioBuffer;
		m_pAudioBuffer = NULL;
	}

	m_waveConverter.Destroy();


	RemoveAllSample();

	 MX_SAFE_CLOSE_HANDLE(m_hSemaSamples);
	
	//MX_ASSERT(0);
	//还要释放链表中的数据


	

}

void  CAudioTrack::HandleSample()
{

	MX_HANDLE hSample=NULL;

	MX_ASSERT(m_hBufferEmptyEvent);
	MX_ASSERT(m_hBufferFullEvent);

	while(m_bStarted)
	{
		if (::WaitForSingleObject(m_hSemaSamples, 50) != WAIT_OBJECT_0) continue;

		if (!WaitBufferToEmpty(50)) break;

		hSample = GetSample();
		MX_ASSERT(hSample);

		void* pSampleData = mxGetSampleData(hSample); 
		MxAudioDesc* pAudioDesc =(MxAudioDesc*) mxGetSampleDescriptor(hSample);

		int newsize=0;
		int nSampleCount = m_waveConverter.GetDestSampleCount(pAudioDesc->sampleCount,  &newsize);

		::EnterCriticalSection(&m_csBuffer);

		MX_ASSERT(newsize>0);
		MX_ASSERT(newsize <= (UINT)(m_nBufferSize - m_nCurPos));

		BOOL ret =m_waveConverter.ExcuteEx((uint8_t *)pSampleData, pAudioDesc->dataSize,  pAudioDesc->sampleCount , (uint8_t *)m_pAudioBuffer + m_nCurPos, newsize, nSampleCount);		
		MX_ASSERT(ret);

		m_nCurPos += newsize;

		::LeaveCriticalSection(&m_csBuffer);

		if ((UINT)m_nCurPos >= m_pDstDesc->dataSize){
			::SetEvent(m_hBufferFullEvent);
		}

		mxDestroySample(hSample);

	}

}


BOOL  CAudioTrack::WaitBufferToEmpty(int timeout)
{
	while(1){
		 if (m_bStarted==FALSE)   return FALSE;

		 if (::WaitForSingleObject(m_hBufferEmptyEvent, timeout) == WAIT_OBJECT_0) 
		 {
			 ::ResetEvent(m_hBufferEmptyEvent);
			 return TRUE;
		 }
	}

	return FALSE;

}


void  CAudioTrack::Read(char* pBuffer, int nSize, BOOL bCopyOnly)
{
	 MX_ASSERT(m_nCurPos>=nSize);
	 MX_ASSERT(m_hBufferEmptyEvent);

	 MX_ASSERT(m_pDstDesc);
	 MX_ASSERT(pBuffer);
	 MX_ASSERT(m_pAudioBuffer);
	 
	 CUtAutoLock lock(&m_csBuffer);

	 if (bCopyOnly){
		 memcpy(pBuffer, m_pAudioBuffer, nSize);		
	 }
	 else{
		 if(m_pDstDesc->depth ==16){
			 MX_ASSERT(nSize % sizeof(short) == 0);

			 short* pOutData = (short*)pBuffer;
			 short* pInData = (short*)m_pAudioBuffer;

			 int    nLen = nSize / sizeof(short);
			 int n;

			 for (int i=0; i< nLen; i++){
				 n = pOutData[i] + int( pInData[i] * this->m_nScalar + 0.5f);

				 if(n>0x7fff)      pOutData[i] = 0x7fff;
				 else if(n<-32768) pOutData[i] = -32768;
				 else    		   pOutData[i] = (short)n;
				
			 }
  
		 }
		 else if(m_pDstDesc->depth == 8){
			  unsigned char* pOutData = (unsigned char*)pBuffer;
			  unsigned char* pInData = (unsigned char*)m_pAudioBuffer;
			  int n;
			  int    nLen = nSize;

			  for (int i=0; i< nLen; i++){
				 n = (pOutData[i]-128) +  char( (pInData[i] -128)* this->m_nScalar + 0.5f);

				 if(n>127)      n = 127;
				 else if(n<-128) n =-128;				 
				
				 MX_ASSERT(n+128>=0 && (n+128) < 256);

				 pOutData[i] = n+128;
			 }
 
		 }
		 else{
			 MX_ASSERT(0);
		 }
	 }

      memmove(m_pAudioBuffer, m_pAudioBuffer + nSize, m_nCurPos - nSize);
	  m_nCurPos-=nSize;

	 ::SetEvent(m_hBufferEmptyEvent);

}
	



//

void CAudioTrack::RemoveAllSample(void)
{
	CUtAutoLock lock(&m_csSample);

	MX_HANDLE hSample=NULL;

	MX_HANDLE* pTmp;

	while ( rsqGetCount(m_arrSample) /*m_arrSample.GetCount()*/>0){
		pTmp = (MX_HANDLE*) rsqGetFirst_(m_arrSample);// m_arrSample.GetHead();
		MX_ASSERT(pTmp);

		hSample = *pTmp;
		MX_ASSERT(hSample);

		rsqRemoveFirst_(m_arrSample);
		//m_arrSample.RemoveHead();

		mxDestroySample(hSample);
	}

}
