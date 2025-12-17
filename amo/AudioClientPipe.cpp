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
#include "StdAfx.h"
#include "AudioClientPipe.h"
 
#ifndef ASSERT
#define ASSERT MX_ASSERT
#endif

#define VM_FREQUENCE         44100
#define VM_CHANNELS          2
#define VM_BITS_PER_SAMPLE   16
 
#define MAX_HEIGHT 1500

#define INBUFSIZE   256

#define  OUTBUFSIZE  (VM_FREQUENCE*VM_CHANNELS*VM_BYTES_PER_SAMPLE)

#define MAX_WF_CONNECTION_COUNT   1

//#define PIPE_NAME  "\\\\.\\pipe\\MyLaodaoMicPipe"
#define MAX_RESP_LEN    20



#define  DEFAULT_PIPE_TIMEOUT  10


 
 //保存一秒中的数据
#define MAX_WAVEDATA_CNT     (1000/AUDIO_PACKET_TIME)

#define REQ_FORMAT   1
#define SEND_DATA     2
 
#define RESP_FORMAT (~1)
#define RESP_DATA   (~2)

  

typedef struct __wave_data_{
    char* pData;
	int   size;
}WAVE_DATA;



class  CWfLock
{
public:
	CWfLock(CRITICAL_SECTION* pCs)
	{
		m_pCs = pCs;
		if (pCs){
			::EnterCriticalSection(pCs);
		}
	};
	~CWfLock()
	{
		if (m_pCs){
			::LeaveCriticalSection(m_pCs);
		}
	}

private:
	CRITICAL_SECTION* m_pCs;
};

 
DWORD WINAPI DataSendThread(LPVOID lpParameter)
{
	CAudioClientPipe* pFf = (CAudioClientPipe*)lpParameter;

	ASSERT(pFf);

	pFf->AutoSendData();
	 
	return 0;
}

CAudioClientPipe::CAudioClientPipe(void)
{	 
	 
	m_nChannels = VM_CHANNELS;
	m_nSamplesPerSecond = VM_FREQUENCE;
	m_nBitsPerSample = VM_BITS_PER_SAMPLE ;
	 
	  m_bExitService = TRUE;

	  m_hThread= NULL;

	m_nWaveWrapSize =0;
	m_pWaveWrapData =NULL;
	m_nCurDataSize = 0;

	 
//	m_bConnected =FALSE;

	m_nWaveDataSize = 0;
	m_pWaveData  =NULL;

	ResetFormat(m_nChannels, m_nBitsPerSample, m_nSamplesPerSecond);
	 

	//CreateMyPipe();

	m_list =rvCreateList();
	
	::InitializeCriticalSection(&m_cs);
}


CAudioClientPipe::~CAudioClientPipe(void)
{
	 //DestroyMyPipe();

	if (m_hPipe)
	{
		CloseHandle(m_hPipe);
		m_hPipe =NULL;
	}

	if (m_list) { 
		this->ReleaseAll();
		rvDestroyList(m_list);
		m_list =NULL;
	}

	 
	ReleaseBuffers();

	::DeleteCriticalSection(&m_cs);
}

 

 BOOL CAudioClientPipe::Start(const TCHAR* strPipeName)
{

	 ASSERT(m_bExitService == TRUE);
	ASSERT(strPipeName);
	MX_ASSERT(m_list);
	MX_ASSERT(rlsGetCount(m_list)==0);

	BOOL b = this->Connect(strPipeName);

	if (!b) return FALSE;

	ASSERT(m_hPipe);

	int chn=0, bps=0, sps=0;
	if (QueryFormat(m_hPipe, chn, bps, sps)!=ERR_NONE){
		this->Disconnect();
		return FALSE;
	}
	else{
	     //CWfLock lock(&m_cs);
		if (chn != this->m_nChannels || bps != m_nBitsPerSample || sps != m_nSamplesPerSecond) {
			this->ResetFormat(chn, bps, sps);
		}
		 
	}

	m_bExitService=FALSE;
	DWORD nThread=0;

	m_hThread=::CreateThread(NULL,0,DataSendThread,this,NULL,&nThread);

	if (!m_hThread)
	{
		m_bExitService =TRUE;
		return FALSE;
	}
	  
	return TRUE;
 }

void CAudioClientPipe::Stop()
{
//	TRACE("you are entering stop  DataSendThread  fuction\r\n");
  
	 m_bExitService=TRUE;

	 if (m_hThread)
	 {
		 WaitForSingleObject(m_hThread,  INFINITE);
		 CloseHandle(m_hThread);
		 m_hThread =NULL; 
	 }

	 this->Disconnect();
	 
	 ReleaseAll();
	// ReleaseBuffers();
	 
}


BOOL CAudioClientPipe::AppendWaveData(const char* pWaveData, UINT nDataSize )
{
	if (this->m_bExitService) return FALSE;

	
	MX_ASSERT(pWaveData && (nDataSize >0));
	

	//CWfLock lock(&m_cs);

	//if (rlsGetCount(m_list) >= MAX_WAVEDATA_CNT){
	//	return FALSE;
	//}

	if (this->m_bConnected==FALSE) return FALSE;

	MX_ASSERT(m_list);

	MX_ASSERT(m_pWaveWrapData );
	MX_ASSERT(m_nWaveDataSize>0);

	if ((m_nCurDataSize + nDataSize) >= this->m_nWaveDataSize){
		MX_ASSERT(m_nCurDataSize >=0  );
		int total = m_nCurDataSize + nDataSize;

		char* pTmpBuffer = new char[total];
		MX_ASSERT(pTmpBuffer);


		if (m_nCurDataSize>0){
			memcpy(pTmpBuffer, this->m_pWaveData, m_nCurDataSize);
		}
		memcpy(pTmpBuffer + m_nCurDataSize, pWaveData, nDataSize);

		int cnt = total / m_nWaveDataSize;
		char* strStart = pTmpBuffer;

		for (int i =0; i<cnt; i++){

			WAVE_DATA* pWd =new WAVE_DATA;
			MX_ASSERT(pWd);

			pWd->pData = new char[m_nWaveDataSize];
			pWd->size = m_nWaveDataSize;

			RV_ASSERT(pWd->pData);
			memcpy(pWd->pData, strStart  , m_nWaveDataSize);

			strStart += m_nWaveDataSize;

			rlsAddTail(m_list, pWd);
		}

		int left = total % m_nWaveDataSize;
		if (left >0){
			memcpy(  this->m_pWaveData, strStart, left);
		}
		m_nCurDataSize = left;

		delete[] pTmpBuffer;
	}
	else
	{
		memcpy(   m_pWaveData + m_nCurDataSize, pWaveData, nDataSize);
		m_nCurDataSize += nDataSize;
	}

    return TRUE;
}
//
//int CAudioClientPipe::HandleFormatReq(char* pMsg, int nSize)
//{
////	TRACE("you are entering HandleFormatReq function\r\n");
//	
//	pMsg += (sizeof(UINT) + sizeof(int));
//
//
//	int channels, bitsPerSample, samplePerSecond;
//
//	CopyMemory(&channels, pMsg, sizeof(int) );
//	pMsg +=  sizeof(int) ;
//
//	CopyMemory(&bitsPerSample, pMsg, sizeof(int) );
//	pMsg +=  sizeof(int) ;
//
//	CopyMemory(&samplePerSecond, pMsg, sizeof(int) );
//	pMsg +=  sizeof(int) ;
//
//	SetWaveFormat(channels,bitsPerSample,samplePerSecond);
//
//
//	UINT len;
//	int id=RESPONSE_WAVE_FORMAT;
//	
//	len=sizeof(UINT)+sizeof(int)  ;
//	char buf[50];
//	//MX_ASSERT(buf);
//	
//	CopyMemory(buf,&len,sizeof(UINT));
//	CopyMemory(buf+sizeof(UINT),&id,sizeof(int));
// 
//	
//	DWORD dwWrite=0;
//	
//	if (!WriteFile(m_hPipe,buf,len,&dwWrite,NULL))
//	{
//		return PIPE_WRITE_FAILED;
//	}
//	if (len!=dwWrite)
//	{
//		return PIPE_WRITE_FAILED;
//	}
//	
//	m_bConnected =TRUE;
//
//	//TRACE("you are leaving HandleFormatReq function successly\r\n");
//
//	return PIPE_OK;
//
//}
////
//int CAudioClientPipe::SendImgData()
//{
//	TRACE("you are entering SendImgData function\r\n");
//	UINT len;
//	int id= RESPONSE_IMAGE_DATA;
//	
//	len=sizeof(UINT)+sizeof(int)+m_nImgWrapSize;
//
//	char *buf=new char[len];
//	MX_ASSERT(buf);
//	MX_ASSERT(m_pImgWrapData);
//
//	CopyMemory(buf,&len,sizeof(UINT));
//	CopyMemory(buf+sizeof(UINT),&id,sizeof(int));
//	CopyMemory(buf+sizeof(UINT)+sizeof(int),m_pImgWrapData,m_nImgWrapSize);
//	
//	DWORD dwWrite=0;
//	
//	if (!WriteFile(m_hPipe,buf,len,&dwWrite,NULL))
//	{
//	//	DWORD ret=GetLastError();
//		delete [] buf;
//		return PIPE_WRITE_FAILED;
//	}
//
//	if (len!=dwWrite)
//	{
//		return PIPE_LEN_MISMATCH;
//	}
//
//	delete [] buf;
//	
//	 
//	
//	return PIPE_OK;
//}

//
//void CAudioClientPipe::SetWaveFormat(int channels, int bitsPerSample, int samplesPerSecond )
//{
//
//	//if (channels ==	m_nChannels && bitsPerSample == m_nBitsPerSample && samplesPerSecond == m_nSamplesPerSecond){
//	//	return;
//	//}
//
//	CWfLock lock(&m_cs);
//
//	this->ReleaseAll();
//	this->ReleaseBuffers();
//
//	m_nChannels=channels;
//	m_nBitsPerSample=bitsPerSample;
//	m_nSamplesPerSecond=samplesPerSecond;
//	 
// 	int dataSize =   (m_nChannels * m_nSamplesPerSecond *  m_nBitsPerSample /8  );
//	MX_ASSERT(dataSize >0);
//
//	if (m_pWaveWrapData){
//	    delete[]m_pWaveWrapData;
//	}
//	if (m_pWaveData){
//	    delete[]m_pWaveData;
//	}
//
//	m_nWaveDataSize = dataSize;
//	m_pWaveData = new char[dataSize];
//	MX_ASSERT( m_pWaveData);
//	ZeroMemory(m_pWaveData, m_nWaveDataSize);
//
//	m_nWaveWrapSize = dataSize + sizeof(UINT) + sizeof(int);
//	m_pWaveWrapData = new char[m_nWaveWrapSize];
//	MX_ASSERT( m_pWaveWrapData);
//	ZeroMemory(m_pWaveWrapData, m_nWaveWrapSize);
//
//	char* buf = m_pWaveWrapData;
//	int id = RESPONSE_WAVE_DATA;
//
//	CopyMemory(buf, &m_nWaveWrapSize, sizeof(UINT));
//	buf+=sizeof(UINT);
//
//	CopyMemory(buf, &id, sizeof(int));
//	buf+=sizeof(int);
//
//
//	
//
//}
//
//DWORD  CAudioClientPipe::CreateMyPipe()
//{
//	//TRACE(" you are entering CreateMyPipe.\r\n");
//	m_hPipe=CreateNamedPipe(PIPE_NAME,	PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,0,MAX_WF_CONNECTION_COUNT,OUTBUFSIZE,INBUFSIZE,DEFAULT_PIPE_TIMEOUT,NULL);
//
//	if (INVALID_HANDLE_VALUE==m_hPipe)
//	{
//		
//		m_hPipe=NULL;
//		//DWORD ret= GetLastError();
//		//TRACE("failed to creat pipe%u\r\n",ret);
//		return PIPE_CREATE_FAILED;
//	}
//		
//	return PIPE_OK;
//
//}
//
//
//DWORD CAudioClientPipe::WaitConnect(int timeout)
//{
//	MX_ASSERT(m_hPipe);
//
//	HANDLE hEvent =CreateEvent(NULL,TRUE,FALSE,NULL);
//	MX_ASSERT(hEvent);
//	DWORD err = 0;
//
//	OVERLAPPED ovlap;
//	ZeroMemory(&ovlap,sizeof(OVERLAPPED));
//	ovlap.hEvent=hEvent;
//
//	if (!ConnectNamedPipe(m_hPipe,&ovlap))
//	{
//		DWORD err =  GetLastError();
//
//		if (ERROR_PIPE_CONNECTED == err)
//		{
//			CloseHandle(hEvent);
//			return PIPE_RECONNECTED;
//		}
//		else if (ERROR_IO_PENDING == err)
//		{
//			CloseHandle(hEvent);
//			 
//			return	PIPE_IO_PENDING;
//		}
//
//	}
//
//
//	if (WAIT_FAILED == WaitForSingleObject(hEvent, timeout))
//	{
//		CloseHandle(hEvent);
//		TRACE("failed to wait object\r\n");
//		//TRACE("WaitForObject error %u\r\n",GetLastError());
//		return PIPE_WAIT_FAILED;
//
//	}
//
//	CloseHandle(hEvent);
//
//	TRACE("connected successfully. \r\n");
//
//	return PIPE_OK;
//
//}
//
//void CAudioClientPipe::DestroyMyPipe()
//{
//	
//	
//	if(m_hPipe)
//	{
//		CloseHandle(m_hPipe);
//
//		m_hPipe =NULL;
//		
//	}
//	
//}
//
//int CAudioClientPipe::ReadMyPipe()
//{
//	//TRACE("you are entering ReadMyPipe\r\n");
//	DWORD dwRead = 0;
//	char buf[128];
//
//	if (!ReadFile(m_hPipe,buf,sizeof(buf),&dwRead,NULL))
//	{
//		DWORD err= ::GetLastError();
//
//		
//		return PIPE_READ_FAILED;
//	}
//	  
//	if (dwRead == 0){
//	    return PIPE_NO_DATA;
//	}
//	
//	UINT len;
//	int id;
// 
//	CopyMemory(&len,buf,sizeof(UINT));
//	CopyMemory(&id,buf+sizeof(UINT),sizeof(int));
//	 
//		
//	if (dwRead !=len)
//	{
//		return PIPE_READ_FAILED;
//	}
//	
//
//	 if (id== REQUEST_WAVE_FORMAT)
//	 { 
//		 return   HandleFormatReq(buf, len);
//		 
//	  }
//	 else if(id == REQUEST_WAVE_DATA)
//	 {
//		 return HandleDataReq();  
//	 }
//	 
//	 return PIPE_WRONG_MSG;
//}
//
//DWORD CAudioClientPipe::HandleDataReq()
//{
//	// TRACE("you are entering HandleDataReq\r\n");
//	MX_ASSERT(this->m_bConnected);
//
//	MX_ASSERT(m_pWaveWrapData);
//	MX_ASSERT(m_nWaveWrapSize >0);
//	MX_ASSERT(m_list );
//
//	WAVE_DATA* pWd =(WAVE_DATA*) rlsRemoveHead(m_list);
//	int headsize =sizeof(UINT) + sizeof(int);
//
//	
//	if (pWd){
//		MX_ASSERT(pWd->size == (m_nWaveWrapSize - headsize));
//
//		CopyMemory(m_pWaveWrapData + headsize, pWd->pData, pWd->size);
//		delete[] pWd->pData;
//		delete pWd;
//	}
//	else{
//	    memset(m_pWaveWrapData + headsize, 0, m_nWaveWrapSize - headsize);
//	}
//	
//	DWORD dwWrite=0;
//	  
//	if (!WriteFile(m_hPipe,m_pWaveWrapData, m_nWaveWrapSize  ,&dwWrite,NULL))
//	{ 
//		return PIPE_WRITE_FAILED;
//	}
//
//	if (dwWrite != m_nWaveWrapSize) return PIPE_WRITE_FAILED;
//
//
//	return PIPE_OK;
//}

BOOL CAudioClientPipe::IsListFull()
{
	MX_ASSERT(m_list);

	return  (rlsGetCount(m_list) >= MAX_WAVEDATA_CNT);
}

void  CAudioClientPipe::ReleaseAll()
{
	MX_ASSERT(m_list);

	if (rlsGoHead(m_list)){
	    while(TRUE){
		     WAVE_DATA* pwd =  ( WAVE_DATA*)rlsGetCur(m_list);
			 MX_ASSERT(pwd);

			 delete[] pwd->pData;
			 delete pwd;

			 if (!rlsStep(m_list)){
				 break;
			 }
		}

		rlsRemoveAll(m_list);
	}

	

}

//
//void   CAudioClientPipe::SendError(DWORD errCode)
//{
//	
//	if (errCode == ERROR_PIPE_NOT_CONNECTED )
//	{
//		TRACE("No process is on the other end of the pipe\r\n");
//	}
//	else if (errCode == ERROR_NO_DATA )
//	{
//		TRACE("The pipe is being closed\r\n");
//		DisconnectNamedPipe(m_hPipe);		
//	}
//	else if (errCode==SEND_FAIL)
//	{
//		TRACE("failed to send  the number of bytes written  is not equal the specified numbers\r\n");
//		DisconnectNamedPipe(m_hPipe);
//		
//	}
//	
//
//}
//void CAudioClientPipe::  ReadPipeError(DWORD errCode)
//	{
//
//		if (errCode == ERROR_PIPE_NOT_CONNECTED )
//		{
//			TRACE("No process is on the other end of the pipe\r\n");
//		}
//		else if (errCode == ERROR_NO_DATA )
//		{
//			TRACE("The pipe is being closed\r\n");
//			DisconnectNamedPipe(m_hPipe);		
//		}
//		else if (errCode== ERROR_BROKEN_PIPE)
//		{
//			TRACE("The pipe has been ended\r\n");
//		}
//	 
//
//	}

void CAudioClientPipe::AutoSendData()
{
	ASSERT(m_hPipe);

	while( IsRunning())
	{		
		//if ( IsConnected() ==FALSE){
		//	if (! Connect()){
		//		Sleep(50);
		//		continue;
		//	}
		//}

		RV_ASSERT(m_list);
		RV_ASSERT(this->m_hPipe);

		BOOL  b=TRUE;
		while(IsRunning()){
		    WAVE_DATA* pWd =(WAVE_DATA*) rlsRemoveHead(m_list);
			
			if (pWd){
				int ret = SendAudioData(m_hPipe, pWd->pData, pWd->size);
				
				if (ret == ERR_FULL){
				   //重新放回头部 ，下次使用
				   rlsAddHead(m_list, pWd);

				   Sleep(50); //额外等待一小段时间
				}
				else{
				    delete[]pWd->pData;
				    delete pWd;

					if (!(ret == ERR_NONE  )){
						b=FALSE;
						break;
					}
				}		
			} 

			Sleep(10);
		} 

		if(b==FALSE){
			this->m_bExitService=TRUE;
			break;
		}
	}
}

void  CAudioClientPipe::ReleaseBuffers()
{
	this->m_nWaveDataSize = 0;
	this->m_nCurDataSize=0;
	this->m_nWaveWrapSize =0;

	if (m_pWaveWrapData){
		delete[] m_pWaveWrapData;
	    m_pWaveWrapData =NULL;
	}

	if (m_pWaveData){
		delete[] m_pWaveData;
	    m_pWaveData =NULL;
	}
}

int CAudioClientPipe::SendAudioData(HANDLE hPipe,  char* pData , UINT size)
{
	ASSERT(pData && (size >0));
	ASSERT(size == m_nWaveDataSize);
	ASSERT(m_nWaveWrapSize == (m_nWaveDataSize+5*sizeof(int)));
	 
	int retcode=0;
	int pos=0;
	int id=SEND_DATA;

	BYTE* buff =(BYTE*) m_pWaveWrapData;
	ASSERT(buff);

	memcpy(buff + pos, &id, sizeof(int));
	pos += sizeof(int);

	memcpy(buff + pos, &m_nWaveWrapSize, sizeof(int));
	pos += sizeof(int);
	 
	memcpy(buff + pos, &m_nChannels, sizeof(int));
	pos += sizeof(int);

	memcpy(buff + pos, &m_nBitsPerSample, sizeof(int));
	pos += sizeof(int);

	memcpy(buff + pos, &m_nSamplesPerSecond, sizeof(int));
	pos += sizeof(int);
	
	memcpy(buff + pos, pData, size);
	pos+=size;
	 
	retcode =Write(hPipe, buff, pos);
	if (ERR_NONE !=retcode)
	{
		return retcode; //表示消息或管道错误
	}


	BYTE resp[sizeof(int)*3];
	UINT  n=sizeof(resp);
	retcode = Read(hPipe, resp, n);
	if (ERR_NONE !=retcode){
	     return retcode;//表示消息或管道错误
	}

	pos=0;
	int ret=0;
	memcpy(&id, resp + pos, sizeof(int));
    pos += sizeof(int);

	memcpy(&size, resp + pos, sizeof(int));
    pos += sizeof(int);

	memcpy(&ret, resp + pos, sizeof(int));
    pos += sizeof(int);


	if (id != RESP_DATA){
		 return ERR_MSG;
	}

	if (size != sizeof(int) *3){
		 return ERR_MSG;
	}

	 
	return ret;
}


int CAudioClientPipe::QueryFormat(HANDLE hPipe, int& channels, int& bitsPerSample, int& samplesPerSecond)
{
	BYTE msg[MAX_RESP_LEN];
	int pos=0;
	int id = REQ_FORMAT;
	int retcode=0;
	UINT size = sizeof(int) + sizeof(int);

	memcpy(msg + pos, &id, sizeof(int));
	pos += sizeof(int);

	memcpy(msg+ pos, &size, sizeof(int));
	pos += sizeof(int);
	 
	retcode =Write(hPipe, msg, size);
	if (ERR_NONE != retcode){
	    return ERR_PIPE;
	}

	size = MAX_RESP_LEN;
	retcode =Read(hPipe, msg, size);
	if (ERR_NONE != retcode){
	    return ERR_MSG;
	}

	pos=0;
	memcpy(&id, msg + pos,  sizeof(int));
	pos += sizeof(int);

	if (id != RESP_FORMAT){
	    return FALSE;
	}

	memcpy( &size,  msg+ pos,sizeof(int));
	pos += sizeof(int);

	if (size != MAX_RESP_LEN){
	    return ERR_MSG;
	}
	 
	memcpy(&channels, msg+ pos,  sizeof(int));
	pos += sizeof(int);

	memcpy(&bitsPerSample, msg+ pos,  sizeof(int));
	pos += sizeof(int);
	 
	memcpy(&samplesPerSecond, msg+ pos,  sizeof(int));
	pos += sizeof(int);
	 

	 

	return ERR_NONE;
}


void CAudioClientPipe::ResetFormat(int channels, int bitsPerSample, int samplePerSecond)
{
	if (this->m_pWaveData){
		delete[] m_pWaveData;
	}
	m_nWaveDataSize = UINT (m_nChannels * m_nSamplesPerSecond * (m_nBitsPerSample/8.f) * AUDIO_PACKET_TIME/ 1000);
	ASSERT(m_nWaveDataSize>0);
	m_pWaveData = new char[m_nWaveDataSize];


	if (this->m_pWaveWrapData){
		delete[] m_pWaveWrapData;
	}

	m_nWaveWrapSize = m_nWaveDataSize + sizeof(int)*5;
	m_pWaveWrapData =new char[m_nWaveWrapSize];

	this->m_nChannels = channels;
	this->m_nBitsPerSample = bitsPerSample;
	this->m_nSamplesPerSecond = samplePerSecond;

	m_nCurDataSize =0;
}
