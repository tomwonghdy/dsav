// Copyright (c) 2025 Tom Wong  
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
#include "stdafx.h"
#include "StreamDivider.h"

CStreamDivider::CStreamDivider(void)
{
	
	m_pDescriptor =NULL;
	m_bSyncOutput =FALSE;

 
	m_inputPad.m_pOwner = this;
	m_inputPad.SetCaptureType(CPad::CT_ANY);

	m_nOutputCount =2;
	m_hSampleThread  = NULL;

	//m_nMediaType = MX_MT_UNKNOWN;

	m_pPullPad =NULL;

	for (int i=0; i<  MX_SD_OUTPUT_COUNT ; i++)
	{
		m_outputPad[i].m_pOwner = this;
		m_outputPad[i].SetCaptureType(CPad::CT_ANY);

	    m_lstSamples[i]    = NULL;
		m_hSampleEvents[i] = NULL;
	}

	m_nWorkMode = WM_PASSIVE;

	::InitializeCriticalSection(&m_cs);

}


CStreamDivider::~CStreamDivider(void)
{
	MX_ASSERT(m_hSampleThread == NULL);

	if (m_pDescriptor){
		mxDestroyDescriptor(m_pDescriptor );
		m_pDescriptor = NULL; 
	}

	for (int i=0; i<  MX_SD_OUTPUT_COUNT ; i++)
	{
		if(m_hSampleEvents[i])
		{
			::CloseHandle(m_hSampleEvents[i]);
			m_hSampleEvents[i]= NULL;
		}

	    if(m_lstSamples[i]){
			RemoveAllSamples(m_lstSamples[i]);
			rvDestroyList(m_lstSamples[i]);
			m_lstSamples[i]=NULL;
		}
	}

	::DeleteCriticalSection(&m_cs);

 
}


COutputPad* CStreamDivider::GetOutputPad(int index) 
{
	if (index >= 0 && index < RV_MIN(m_nOutputCount,MX_SD_OUTPUT_COUNT)){
		return &m_outputPad[index];
	}

	return NULL;

}


struct __stream_thread_data_
{
	HANDLE          hThread;
	CStreamDivider* pSd;
	int             index;
};


DWORD WINAPI ThreadSubStreamDivider(  LPVOID lpParameter)
{
	struct __stream_thread_data_* pStd =(struct __stream_thread_data_*) lpParameter;
	MX_ASSERT(pStd);

	MX_ASSERT(pStd->pSd);

	pStd->pSd->HandleSamples(pStd->index);

	return 0;
}

DWORD WINAPI ThreadStreamDivider(  LPVOID lpParameter)
{
	CStreamDivider* pSd = (CStreamDivider* )lpParameter;
	 
	struct __stream_thread_data_ stds[MX_SD_OUTPUT_COUNT];

	int cnt =pSd->GetStreamCount();
	MX_ASSERT(cnt <=MX_SD_OUTPUT_COUNT);

	for (int i=0; i <cnt; i++){
		stds[i].index=i;
		stds[i].pSd = pSd;
		stds[i].hThread = ::CreateThread(NULL,0, ThreadSubStreamDivider, &stds[i], 0,NULL);
		MX_ASSERT(stds[i].hThread);
	}

	//等待结束
	for (int i=0; i <cnt; i++){
		if (::WaitForSingleObject(stds[i].hThread, INFINITE) == WAIT_OBJECT_0){
			::CloseHandle(stds[i].hThread);
		}
		else{
		   MX_ASSERT(0);
		   ::CloseHandle(stds[i].hThread);
		}
	}
	 

	return 0;

}

M_RESULT CStreamDivider::Play(const char* strUrl )
{
	if (m_nState == MS_PLAYING) return M_FAILED;
	if( m_pDescriptor == NULL) return M_UNKNOWN_TYPE;

	CUtAutoLock lock(&m_cs);
	
	if (m_nOutputCount <= 0) m_nOutputCount=0;
	if (m_nOutputCount > MX_SD_OUTPUT_COUNT) m_nOutputCount=MX_SD_OUTPUT_COUNT;
	 
	for (int i=0; i< this->m_nOutputCount ; i++)
	{ 
		MX_ASSERT( m_lstSamples[i] == NULL);
		MX_ASSERT( m_hSampleEvents[i] == NULL);

	    m_lstSamples[i]    = rvCreateList();
		m_hSampleEvents[i] = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	m_nState = MS_PLAYING;

	if (this->m_bSyncOutput==FALSE){
		m_hSampleThread = ::CreateThread(NULL, 0, ThreadStreamDivider, this, 0,NULL);
		MX_ASSERT(m_hSampleThread);
	}
		
	return M_OK;

	//return M_DISCONNECTED; 
} 


void CStreamDivider::Pause(BOOL bResume){
	m_nState =MS_PAUSED;
};
	
void CStreamDivider::Stop(){

	CUtAutoLock lock(&m_cs);

	m_nState = MS_STOPPED;

	if (m_hSampleThread){
		::WaitForSingleObject(m_hSampleThread, INFINITE);
		::CloseHandle(m_hSampleThread);
		m_hSampleThread = NULL;
	}

	for (int i=0; i< MX_SD_OUTPUT_COUNT ; i++)
	{ 
		if(m_hSampleEvents[i])
		{
			::CloseHandle(m_hSampleEvents[i]);
			m_hSampleEvents[i]= NULL;
		}

		if(m_lstSamples[i]){
			RemoveAllSamples(m_lstSamples[i]);

			rvDestroyList(m_lstSamples[i]);
			m_lstSamples[i]=NULL;
		}
	}

} 

//  
////输出PIN调用connect的时候被调用，媒体对象的输入PIN必须重载该
////函数，以表示是否支持指定的媒体类型。
//BOOL CStreamDivider::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
//{
//	return (type != MX_MT_UNKNOWN);
//}
//
//	//获得媒体对象输出PIN的媒体类型
//MX_MEDIA_TYPE CStreamDivider::GetMediaType(CPad* pOutputPad)
//{
//	//RV_ASSERT(&m_outputPad[0] == pOutputPad || &m_outputPad[1] == pOutputPad);
//#if _DEBUG
//	BOOL b=FALSE;
//	for (int i=0; i<this->m_nOutputCount; i++){
//	    if (&m_outputPad[i] == pOutputPad){
//		    b=TRUE;
//			break;
//		}
//	}
//	RV_ASSERT(b);
//#endif
//
//    if (m_pDescriptor) return m_pDescriptor->type;
//
//	return MX_MT_UNKNOWN;
//	 
//}
//
//MxDescriptor* CStreamDivider::GetFormatDescriptor(CPad* pPad){
//	return m_pDescriptor;
//}


MX_HANDLE CStreamDivider::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE _hSample,*/ int* pErrCode)
{
//	RV_ASSERT( hSample );
	RV_ASSERT( m_pDescriptor );
	RV_ASSERT( m_nOutputCount>0 && m_nOutputCount <= MX_SD_OUTPUT_COUNT);

	if (!::TryEnterCriticalSection(&m_cs)){
		if (pErrCode) *pErrCode = M_FAILED;
		return NULL;
	}

	if (this->m_nState== MS_STOPPED){
		::LeaveCriticalSection(&m_cs);

	    if (pErrCode) *pErrCode = M_FAILED;
		return NULL;
	}

	if (m_pPullPad != pOutputPad){
		::LeaveCriticalSection(&m_cs);

	    if (pErrCode) *pErrCode = M_UNSUPPORT_PAD;
		return NULL;
	}

	 
	MX_HANDLE hSample =	m_inputPad.Fetch(pErrCode);

	if (hSample){
		for (int i=0; i <m_nOutputCount ; i++){
			if (&m_outputPad[i] != pOutputPad ){
				MX_ASSERT(m_lstSamples[i]);
				MX_ASSERT(m_hSampleEvents[i]);

				MX_HANDLE hNew = mxDuplicateSample(hSample);
				MX_ASSERT(hNew);
				rlsAddTail(m_lstSamples[i], hNew);

				::SetEvent(m_hSampleEvents[i]);
				
			}
		}

		if (pErrCode) *pErrCode = M_OK;
	}
	else{
	    if (pErrCode) *pErrCode = M_FAILED;
	}
	
	::LeaveCriticalSection(&m_cs);

	return hSample;
}

//当接收到一个SAMPLE的时候,产生该事件。
BOOL CStreamDivider::OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample,int* pErrCode){

	RV_ASSERT( hSample );
	RV_ASSERT( m_pDescriptor );
	
	if (!::TryEnterCriticalSection(&m_cs)){
		if (pErrCode) *pErrCode = M_FAILED;
		return FALSE;
	}

	if (this->m_nState== MS_STOPPED){
		::LeaveCriticalSection(&m_cs);

	    if (pErrCode) *pErrCode = M_FAILED;
		return FALSE;
	}


	int ret = DeliverMultiSamples(hSample);
	  
	mxDestroySample(hSample);

	if (pErrCode) *pErrCode = M_OK;

	::LeaveCriticalSection(&m_cs);

	return TRUE;

}


BOOL CStreamDivider::OnPadConnected(CPad* pOutputPad, CPad* pInputPad) {
	
	for (int i=0; i < RV_MIN(m_nOutputCount,MX_SD_OUTPUT_COUNT) ; i++){
		if ( pOutputPad == &m_outputPad[i] ){
			RV_ASSERT(m_pDescriptor);
			return   pOutputPad->NegotiateDescriptor(m_pDescriptor); 
		}
	}

	
	if (pInputPad == &m_inputPad){
		MxDescriptor* pDesc = pOutputPad->GetFormatDescriptor();
		RV_ASSERT(pDesc);

		SetDescriptor(pDesc);
	}

	return TRUE;
}

void CStreamDivider::OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad) 
{
	 
	if (pInputPad == &m_inputPad)
	{
		if (m_pDescriptor){
			mxDestroyDescriptor(m_pDescriptor );
			m_pDescriptor = NULL; 
		}

		for (int i=0; i< RV_MIN(m_nOutputCount,MX_SD_OUTPUT_COUNT); i++)
		{
			m_outputPad[i].Disconnect();
		}

	}

}

//是否接受输入PAD的流媒体格式描述符
//具体格式协商的时候用，一般输入PIN所在的MO实现。	
BOOL CStreamDivider::Accept(CPad* pInputPad, MxDescriptor* pDescriptor) 
{

	RV_ASSERT(pInputPad && pDescriptor);
	RV_ASSERT(pInputPad  == &m_inputPad);
	RV_ASSERT(pInputPad->IsConnected());
	RV_ASSERT(m_pDescriptor);

	if (m_pDescriptor->type != pDescriptor->type ){
		RV_ASSERT(0);
		return FALSE;
	}

	for (int i=0; i < RV_MIN(m_nOutputCount,MX_SD_OUTPUT_COUNT); i++){
		if (m_outputPad[i].IsConnected()){
			if (m_outputPad[i].NegotiateDescriptor(pDescriptor) == FALSE) return FALSE;			
		}
	}

	SetDescriptor(pDescriptor);
 
	return TRUE;	 
}

int CStreamDivider::DeliverMultiSamples(MX_HANDLE hSample)
{
	MX_ASSERT(m_nState == MX_PLAYING);
	int ret =M_OK;
	if (this->m_bSyncOutput){
		ret = SendSampleInSyncMode(hSample );
	}
	else{
		for (int i=0; i<this->m_nOutputCount; i++){
		    MX_ASSERT(m_lstSamples[i]);
		    MX_ASSERT(m_hSampleEvents[i]);

			MX_HANDLE hNew = mxDuplicateSample(hSample);
			MX_ASSERT(hNew);

			rlsAddTail(m_lstSamples[i], hNew);

			::SetEvent(m_hSampleEvents[i]);
		}
	}

	return ret;
}

int CStreamDivider::SendSampleInSyncMode(MX_HANDLE hSample)
{
	MX_HANDLE hTemp = mxDuplicateSample(hSample);
	MX_ASSERT(hTemp);

	int nErrCode =M_OK;

	for (int i=0; i<  m_nOutputCount ; i++)
	{
		if (hTemp == NULL)
		{
			hTemp = mxDuplicateSample(hSample);
			MX_ASSERT(hTemp);
		}

		if (m_outputPad[i].IsConnected()){
			 
			int c = 0;
			while(c++ < 50) // 可以等待0.5秒
			{
				if (m_nState == MS_STOPPED){
					mxDestroySample(hTemp);                         
					return M_TERMINATED;
				}

				if (m_nState == MS_PAUSED){
					if (c>0) --c;
					::Sleep(10);
					continue;
				}

				if (m_outputPad[i].Pass( hTemp, &nErrCode)){
					hTemp = NULL;
					break;
				}
				else{
					if (nErrCode == M_BUFFER_FULL){
						if (c>0) --c;
						::Sleep(10);
						continue;
					}
					else {
						break;
					}
				}

			}

		}		
		else{
			if (this->m_fnSamplePass){
				m_fnSamplePass(this, &m_outputPad[i], hTemp, this->m_pUserData);
			}
		}
	}

	if (hTemp){
		mxDestroySample(hTemp);
	}	

	return M_OK;
}



void  CStreamDivider::SetDescriptor( MxDescriptor* pDesc)
{
	
	RV_ASSERT(pDesc);
	if (NULL == m_pDescriptor )
	{
		m_pDescriptor =  mxCopyDescriptor(pDesc, NULL);
		RV_ASSERT(m_pDescriptor);
	}
	else {

		if (m_pDescriptor->type != pDesc->type){
			mxDestroyDescriptor(m_pDescriptor );
			m_pDescriptor =  mxCreateDescriptor(pDesc->type);
			RV_ASSERT(m_pDescriptor);
		}

		mxCopyDescriptor(pDesc, m_pDescriptor);

	}
		 
}

void  CStreamDivider::RemoveAllSamples(RvList list)
{
	MX_ASSERT(list);

	if (rlsGoHead(list)){
	    while(TRUE){
		     MX_HANDLE hSample = rlsGetCur(  list);
			 MX_ASSERT(hSample);

			 mxDestroySample(hSample);
			 if (!rlsStep(list)){
			     break;
			 }
		}

		rlsRemoveAll(list);
	}

}

void CStreamDivider::SetStreamCount(int count)
{
	CUtAutoLock lock(&m_cs);
	if (m_nState != MS_STOPPED){
		return;
	}

	this->m_nOutputCount = count;
}

void CStreamDivider::HandleSamples(int index)
{

	MX_ASSERT(index>=0 && index < MX_SD_OUTPUT_COUNT);
	MX_ASSERT(index>=0 && index < m_nOutputCount);

	MX_ASSERT(m_hSampleEvents[index]);
	MX_ASSERT(m_lstSamples[index]);
	
	//拉输出pad就直接退出算了。
	if (m_pPullPad){
		if (&m_outputPad[index] == m_pPullPad)
		{
			return;
		}
	}

	while(m_nState != MS_STOPPED){
	    if (m_nState == MS_PAUSED){
		    Sleep(10) ;continue;
		}

		if (::WaitForSingleObject(m_hSampleEvents[index], 20) != WAIT_OBJECT_0){
		    continue;
		}
		::ResetEvent(m_hSampleEvents[index]);

		//发送样品
		while(m_nState != MS_STOPPED){
			if (m_nState == MS_PAUSED){
				Sleep(10) ;continue;
			}

			MX_HANDLE hSample = rlsRemoveHead(m_lstSamples[index]);
			if (hSample == NULL){
			    break;
			}

			int nErrCode=0;
			if (m_outputPad[index].IsConnected()){
				if (!m_outputPad[index].Pass( hSample, &nErrCode)){
					if (nErrCode == M_BUFFER_FULL){
						 //重新放回头部
						 rlsAddHead(m_lstSamples[index], hSample);
						 ::Sleep(1);
					}
					else{
						mxDestroySample(hSample);   
					}
				}
			}
			else{
				if (this->m_fnSamplePass){
					m_fnSamplePass(this, &m_outputPad[index], hSample, this->m_pUserData);
				}
				mxDestroySample(hSample); 
			}

		} 
	}



}

void CStreamDivider::SetSyncMode(BOOL flag)
{
	CUtAutoLock lock(&m_cs);
	if (m_nState != MS_STOPPED){
		return;
	}

	this->m_bSyncOutput = flag;
}

void CStreamDivider::SetPullPad(int index)
{
	CUtAutoLock lock(&m_cs);
	if (m_nState != MS_STOPPED){
		return;
	}

	if (index >=0 && index < this->m_nOutputCount ){
	    m_pPullPad = &m_outputPad[index];
	}

}

void CStreamDivider::RemovePullPad()
{
	CUtAutoLock lock(&m_cs);
	if (m_nState != MS_STOPPED){
		return;
	} 
	m_pPullPad = NULL; 
}

	
COutputPad* CStreamDivider::GetPullPad()
{
	return m_pPullPad;
}
//
//MX_MEDIA_TYPE CStreamDivider::GetMediaType()
//{
//	return m_nMediaType;
//}
//	
//void CStreamDivider::SetMediaType(MX_MEDIA_TYPE type)
//{
//	m_nMediaType = type; 
//}