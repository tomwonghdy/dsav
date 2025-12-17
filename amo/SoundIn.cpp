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
#include "SoundIn.h"


CSoundIn::CSoundIn(void)
{
	m_bRun = FALSE;
	m_hThread=NULL;
	m_hWaveIn=NULL;
	//ZeroMemory(&m_stWFEX,sizeof(WAVEFORMATEX));
	m_nWantedDataSize =0;

	ZeroMemory(m_stWHDR, SI_MAX_BUFFERS * sizeof(WAVEHDR));

	//m_nAvgBytesPerSec = 0;
	m_pDataFillDoneFunc = NULL;
	m_pUserData = NULL;

	::InitializeCriticalSection(&m_cs);

}

CSoundIn::~CSoundIn(void)
{
	::DeleteCriticalSection(&m_cs);
}




void CSoundIn::StartRecording()
{	
	MMRESULT mRes;
	//SetStatus("Recording...");

	try
	{
		//OpenDevice();

		PrepareBuffers();

		mRes=waveInStart(m_hWaveIn);
		if(mRes!=0)
		{
			//StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
			throw "waveInStart error";
		}

		while(m_bRun)
		{
			SleepEx(100,FALSE);
		}


	//	UnPrepareBuffers();
	

	}
	catch(PCHAR pErrorMsg)
	{
		MessageBox(NULL, pErrorMsg, "´íÎó",0);
		;//AfxMessageBox(pErrorMsg);
	}
	

//	CloseDevice();
		
	if(m_hWaveIn)
	{
		UnPrepareBuffers();
		mRes=waveInClose(m_hWaveIn);

		m_hWaveIn = NULL;
	}

	CloseHandle(m_hThread);
	m_hThread=NULL;
	
	 

}

//void CSoundIn::OnCbnSelchangeDevices()
//{
//	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_DEVICES);
//	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_FORMATS);
//	int nSel;
//	WAVEINCAPS stWIC={0};
//	MMRESULT mRes;
//
//
//	SetStatus("Querying device informations...");
//	pFormats->ResetContent();
//	nSel=pDevices->GetCurSel();
//
//	if(nSel!=-1)
//	{
//		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
//		mRes=waveInGetDevCaps(nSel,&stWIC,sizeof(WAVEINCAPS));
//		if(mRes==0)
//		{
//			if(WAVE_FORMAT_1M08==(stWIC.dwFormats&WAVE_FORMAT_1M08))
//				pFormats->SetItemData(pFormats->AddString("11.025 kHz, mono, 8-bit"),WAVE_FORMAT_1M08);
//			if(WAVE_FORMAT_1M16==(stWIC.dwFormats&WAVE_FORMAT_1M16))
//				pFormats->SetItemData(pFormats->AddString("11.025 kHz, mono, 16-bit"),WAVE_FORMAT_1M16);
//			if(WAVE_FORMAT_1S08==(stWIC.dwFormats&WAVE_FORMAT_1S08))
//				pFormats->SetItemData(pFormats->AddString("11.025 kHz, stereo, 8-bit"),WAVE_FORMAT_1S08);
//			if(WAVE_FORMAT_1S16==(stWIC.dwFormats&WAVE_FORMAT_1S16))
//				pFormats->SetItemData(pFormats->AddString("11.025 kHz, stereo, 16-bit"),WAVE_FORMAT_1S16);
//			if(WAVE_FORMAT_2M08==(stWIC.dwFormats&WAVE_FORMAT_2M08))
//				pFormats->SetItemData(pFormats->AddString("22.05 kHz, mono, 8-bit"),WAVE_FORMAT_2M08);
//			if(WAVE_FORMAT_2M16==(stWIC.dwFormats&WAVE_FORMAT_2M16))
//				pFormats->SetItemData(pFormats->AddString("22.05 kHz, mono, 16-bit"),WAVE_FORMAT_2M16);
//			if(WAVE_FORMAT_2S08==(stWIC.dwFormats&WAVE_FORMAT_2S08))
//				pFormats->SetItemData(pFormats->AddString("22.05 kHz, stereo, 8-bit"),WAVE_FORMAT_2S08);
//			if(WAVE_FORMAT_2S16==(stWIC.dwFormats&WAVE_FORMAT_2S16))
//				pFormats->SetItemData(pFormats->AddString("22.05 kHz, stereo, 16-bit"),WAVE_FORMAT_2S16);
//			if(WAVE_FORMAT_4M08==(stWIC.dwFormats&WAVE_FORMAT_4M08))
//				pFormats->SetItemData(pFormats->AddString("44.1 kHz, mono, 8-bit"),WAVE_FORMAT_4M08);
//			if(WAVE_FORMAT_4M16==(stWIC.dwFormats&WAVE_FORMAT_4M16))
//				pFormats->SetItemData(pFormats->AddString("44.1 kHz, mono, 16-bit"),WAVE_FORMAT_4M16);
//			if(WAVE_FORMAT_4S08==(stWIC.dwFormats&WAVE_FORMAT_4S08))
//				pFormats->SetItemData(pFormats->AddString("44.1 kHz, stereo, 8-bit"),WAVE_FORMAT_4S08);
//			if(WAVE_FORMAT_4S16==(stWIC.dwFormats&WAVE_FORMAT_4S16))
//				pFormats->SetItemData(pFormats->AddString("44.1 kHz, stereo, 16-bit"),WAVE_FORMAT_4S16);
//			if(WAVE_FORMAT_96M08==(stWIC.dwFormats&WAVE_FORMAT_96M08))
//				pFormats->SetItemData(pFormats->AddString("96 kHz, mono, 8-bit"),WAVE_FORMAT_96M08);
//			if(WAVE_FORMAT_96S08==(stWIC.dwFormats&WAVE_FORMAT_96S08))
//				pFormats->SetItemData(pFormats->AddString("96 kHz, stereo, 8-bit"),WAVE_FORMAT_96S08);
//			if(WAVE_FORMAT_96M16==(stWIC.dwFormats&WAVE_FORMAT_96M16))
//				pFormats->SetItemData(pFormats->AddString("96 kHz, mono, 16-bit"),WAVE_FORMAT_96M16);
//			if(WAVE_FORMAT_96S16==(stWIC.dwFormats&WAVE_FORMAT_96S16))
//				pFormats->SetItemData(pFormats->AddString("96 kHz, stereo, 16-bit"),WAVE_FORMAT_96S16);
//
//			if(pFormats->GetCount())
//				pFormats->SetCurSel(0);
//
//		}
//		else
//			StoreError(mRes,TRUE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
//	}
//
//	SetStatus("Waiting to start...");
//
//
//}


BOOL  CSoundIn::IsFormatSupported(int index , int rate, int channels, int depth)
{
	 
	WAVEINCAPS stWIC={0};
	MMRESULT mRes;

	ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
	mRes = waveInGetDevCaps(index,&stWIC,sizeof(WAVEINCAPS));

	if(mRes == MMSYSERR_NOERROR)
	{
		if(WAVE_FORMAT_1M08==(stWIC.dwFormats&WAVE_FORMAT_1M08))
			if ( (rate == 11025) && (channels ==1) && (depth == 8) ) return TRUE;	//("11.025 kHz, mono, 8-bit")
		if(WAVE_FORMAT_1M16==(stWIC.dwFormats&WAVE_FORMAT_1M16))
			if ( (rate == 11025) && (channels ==1) && (depth == 16) ) return TRUE;	//"11.025 kHz, mono, 16-bit"),WAVE_FORMAT_1M16);
		if(WAVE_FORMAT_1S08==(stWIC.dwFormats&WAVE_FORMAT_1S08))
			if ( (rate == 11025) && (channels ==2) && (depth == 8) ) return TRUE;	//"11.025 kHz, stereo, 8-bit"),WAVE_FORMAT_1S08);
		if(WAVE_FORMAT_1S16==(stWIC.dwFormats&WAVE_FORMAT_1S16))
			if ( (rate == 11025) && (channels ==2) && (depth == 16) ) return TRUE;	//"11.025 kHz, stereo, 16-bit"),WAVE_FORMAT_1S16);
		if(WAVE_FORMAT_2M08==(stWIC.dwFormats&WAVE_FORMAT_2M08))
			if ( (rate == 22050) && (channels ==1) && (depth == 8) ) return TRUE;	//"22.05 kHz, mono, 8-bit"),WAVE_FORMAT_2M08);
		if(WAVE_FORMAT_2M16==(stWIC.dwFormats&WAVE_FORMAT_2M16))
			if ( (rate == 22050) && (channels ==1) && (depth == 16) ) return TRUE;	//"22.05 kHz, mono, 16-bit"),WAVE_FORMAT_2M16);
		if(WAVE_FORMAT_2S08==(stWIC.dwFormats&WAVE_FORMAT_2S08))
			if ( (rate == 22050) && (channels ==2) && (depth == 8) ) return TRUE;	//"22.05 kHz, stereo, 8-bit"),WAVE_FORMAT_2S08);
		if(WAVE_FORMAT_2S16==(stWIC.dwFormats&WAVE_FORMAT_2S16))
			if ( (rate == 22050) && (channels ==2) && (depth == 16) ) return TRUE;//"22.05 kHz, stereo, 16-bit"),WAVE_FORMAT_2S16);
		if(WAVE_FORMAT_4M08==(stWIC.dwFormats&WAVE_FORMAT_4M08))
			if ( (rate == 44100) && (channels ==1) && (depth == 8) ) return TRUE;//"44.1 kHz, mono, 8-bit"),WAVE_FORMAT_4M08);
		if(WAVE_FORMAT_4M16==(stWIC.dwFormats&WAVE_FORMAT_4M16))
			if ( (rate == 44100) && (channels ==1) && (depth == 16) ) return TRUE;//"44.1 kHz, mono, 16-bit"),WAVE_FORMAT_4M16);
		if(WAVE_FORMAT_4S08==(stWIC.dwFormats&WAVE_FORMAT_4S08))
			if ( (rate == 44100) && (channels ==2) && (depth == 8) ) return TRUE;//"44.1 kHz, stereo, 8-bit"),WAVE_FORMAT_4S08);
		if(WAVE_FORMAT_4S16==(stWIC.dwFormats&WAVE_FORMAT_4S16))
			if ( (rate == 44100) && (channels ==2) && (depth == 16) ) return TRUE;//"44.1 kHz, stereo, 16-bit"),WAVE_FORMAT_4S16);
		if(WAVE_FORMAT_96M08==(stWIC.dwFormats&WAVE_FORMAT_96M08))
			if ( (rate == 96000) && (channels == 1) && (depth == 8) ) return TRUE;//"96 kHz, mono, 8-bit"),WAVE_FORMAT_96M08);
		if(WAVE_FORMAT_96S08==(stWIC.dwFormats&WAVE_FORMAT_96S08))
			if ( (rate == 96000) && (channels ==2) && (depth == 8) ) return TRUE;//"96 kHz, stereo, 8-bit"),WAVE_FORMAT_96S08);
		if(WAVE_FORMAT_96M16==(stWIC.dwFormats&WAVE_FORMAT_96M16))
			if ( (rate == 96000) && (channels ==1) && (depth == 16) ) return TRUE;//"96 kHz, mono, 16-bit"),WAVE_FORMAT_96M16);
		if(WAVE_FORMAT_96S16==(stWIC.dwFormats&WAVE_FORMAT_96S16))
			if ( (rate == 96000) && (channels ==2) && (depth == 16) ) return TRUE;//"96 kHz, stereo, 16-bit"),WAVE_FORMAT_96S16);
	}

	return FALSE;
		 

}
//
//void CSoundIn::SetStatus(LPCTSTR lpszFormat, ...)
//{
//	CString csT1;
//	va_list args;
//
//	va_start(args, lpszFormat);
//	csT1.FormatV(lpszFormat,args);
//	va_end(args);
//
//	if(IsWindow(m_hWnd) && GetDlgItem(IDC_EDIT_STATUS))
//		GetDlgItem(IDC_EDIT_STATUS)->SetWindowText(csT1);
//}


//CString CSoundIn::StoreError(MMRESULT mRes,BOOL bDisplay,LPCTSTR lpszFormat, ...)
//{
//	MMRESULT mRes1=0;
//	char szErrorText[1024]={0};
//	char szT1[2*MAX_PATH]={0};
//	
//	va_list args;
//	va_start(args, lpszFormat);
//	_vsntprintf(szT1, MAX_PATH, lpszFormat, args);
//	va_end(args);
//
//	m_csErrorText.Empty();
//
//	if(m_bRun)
//	{
//		mRes1=waveInGetErrorText(mRes,szErrorText,1024);
//
//		if(mRes1!=0)
//			wsprintf(szErrorText,"Error %d in querying the error string for error code %d",mRes1,mRes);
//
//		m_csErrorText.Format("%s: %s",szT1,szErrorText);
//
//		if(bDisplay)
//			AfxMessageBox(m_csErrorText);
//	}
//
//	return m_csErrorText;
//}


DWORD WINAPI AudioInFunc(LPVOID pDt)
{
	CSoundIn *pOb=(CSoundIn*)pDt;
	
	pOb->StartRecording();

	return 0;
}

void CALLBACK WaveCaptureInProc(HWAVEIN hwi,UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{

	WAVEHDR *pHdr=NULL;
	switch(uMsg)
	{
		case WIM_CLOSE:
			break;

		case WIM_DATA:
			{
				CSoundIn *pDlg=(CSoundIn*)dwInstance;
				pDlg->ProcessHeader((WAVEHDR *)dwParam1);
			}
			break;

		case WIM_OPEN:
			break;

		default:
			break;
	}
}



void CSoundIn::ProcessHeader(WAVEHDR * pHdr)
{
	::EnterCriticalSection(&m_cs) ;

	if (!m_bRun) {		 
		::LeaveCriticalSection(&m_cs);
		return;
	}


	MMRESULT mRes=0;

	//TRACE("%d",pHdr->dwUser);

	if(WHDR_DONE==(WHDR_DONE &pHdr->dwFlags))
	{
		//mmioWrite(m_hOPFile,pHdr->lpData,pHdr->dwBytesRecorded);
		if (m_pDataFillDoneFunc)
		{
			m_pDataFillDoneFunc(m_pUserData, pHdr->lpData,pHdr->dwBytesRecorded);
		}

		mRes = waveInAddBuffer(m_hWaveIn,pHdr,sizeof(WAVEHDR));
		
		if(mRes!=0) {
			throw "waveInAddBuffer error";
		}
//			StoreError(mRes,TRUE,"File: %s ,Line Number:%d",__FILE__,__LINE__);

	}

	::LeaveCriticalSection(&m_cs);

}

BOOL CSoundIn::Start(int index, PWAVEFORMATEX pWaveformat, UINT nWantedDataSize,  SI_DATA_FILL_DONE_CALLBACK pfnDataFillDone  , void* pUserData )
{

	MX_ASSERT(pWaveformat);

	MMRESULT mRes=0;

	//csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	//csT1.Trim();
	//sscanf((PCHAR)(LPCTSTR)csT1,"%d",&m_stWFEX.wBitsPerSample);
	//
	//m_stWFEX.wFormatTag=WAVE_FORMAT_PCM;
	//m_stWFEX.nBlockAlign=m_stWFEX.nChannels*m_stWFEX.wBitsPerSample/8;
	//m_stWFEX.nAvgBytesPerSec=m_stWFEX.nSamplesPerSec*m_stWFEX.nBlockAlign;
	//m_stWFEX.cbSize=sizeof(WAVEFORMATEX);

	mRes=waveInOpen(&m_hWaveIn,index , pWaveformat,(DWORD_PTR)WaveCaptureInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);

	if(mRes !=MMSYSERR_NOERROR)
	{
		//StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
		//throw m_csErrorText;
		return FALSE;
	}


	m_pDataFillDoneFunc = pfnDataFillDone ; 
	m_pUserData = pUserData;

	m_nWantedDataSize =  nWantedDataSize;


	m_bRun =TRUE;
	m_hThread = ::CreateThread(NULL, 0, AudioInFunc, this, 0 , NULL);
	MX_ASSERT(m_hThread);


	return TRUE;

}

 
//
//void CSoundIn::OpenDevice()
//{
//	int nT1=0;
//	CString csT1 ="testrecord.wav";
//
//	double dT1=0.0;
//	MMRESULT mRes=0;
//	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_DEVICES);
//	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_FORMATS);
//
//	nT1=pFormats->GetCurSel();
//	if(nT1==-1)
//		throw "";
//
//	pFormats->GetLBText(nT1,csT1);
//	sscanf((PCHAR)(LPCTSTR)csT1,"%lf",&dT1);
//	dT1=dT1*1000;
//
//	m_stWFEX.nSamplesPerSec=(int)dT1;
//	
//	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
//	csT1.Trim();
//
//	if(csT1.Find("mono")!=-1)
//		m_stWFEX.nChannels=1;
//	if(csT1.Find("stereo")!=-1)
//		m_stWFEX.nChannels=2;
//
//	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
//	csT1.Trim();
//	sscanf((PCHAR)(LPCTSTR)csT1,"%d",&m_stWFEX.wBitsPerSample);
//	
//	m_stWFEX.wFormatTag=WAVE_FORMAT_PCM;
//	m_stWFEX.nBlockAlign=m_stWFEX.nChannels*m_stWFEX.wBitsPerSample/8;
//	m_stWFEX.nAvgBytesPerSec=m_stWFEX.nSamplesPerSec*m_stWFEX.nBlockAlign;
//	m_stWFEX.cbSize=sizeof(WAVEFORMATEX);
//
//	mRes=waveInOpen(&m_hWaveIn,pDevices->GetCurSel(),&m_stWFEX,(DWORD_PTR)WaveCaptureInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
//
//	if(mRes!=MMSYSERR_NOERROR)
//	{
//		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
//		throw m_csErrorText;
//	}
//	
//	//GetDlgItem(IDC_FILENAME)->GetWindowText(csT1);
//
//	/*ZeroMemory(&m_stmmIF,sizeof(MMIOINFO));
//	DeleteFile((PCHAR)(LPCTSTR)csT1);
//	m_hOPFile=mmioOpen((PCHAR)(LPCTSTR)csT1,&m_stmmIF,MMIO_WRITE | MMIO_CREATE);
//	if(m_hOPFile==NULL)
//		throw "Can not open file...";
//
//	ZeroMemory(&m_stckOutRIFF,sizeof(MMCKINFO));
//	m_stckOutRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E'); 
//	mRes=mmioCreateChunk(m_hOPFile, &m_stckOutRIFF, MMIO_CREATERIFF);
//	if(mRes!=MMSYSERR_NOERROR)
//	{
//		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
//		throw m_csErrorText;
//	}
//
//	ZeroMemory(&m_stckOut,sizeof(MMCKINFO));
//	m_stckOut.ckid = mmioFOURCC('f', 'm', 't', ' ');
//	m_stckOut.cksize = sizeof(m_stWFEX);
//	mRes=mmioCreateChunk(m_hOPFile, &m_stckOut, 0);
//	if(mRes!=MMSYSERR_NOERROR)
//	{
//		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
//		throw m_csErrorText;
//	}
//	nT1=mmioWrite(m_hOPFile, (HPSTR) &m_stWFEX, sizeof(m_stWFEX));
//	if(nT1!=sizeof(m_stWFEX))
//	{
//		m_csErrorText.Format("Can not write Wave Header..File: %s ,Line Number:%d",__FILE__,__LINE__);
//		throw m_csErrorText;
//	}
//	mRes=mmioAscend(m_hOPFile, &m_stckOut, 0);
//	if(mRes!=MMSYSERR_NOERROR)
//	{
//		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
//		throw m_csErrorText;
//	}
//
//	m_stckOut.ckid = mmioFOURCC('d', 'a', 't', 'a');
//	mRes=mmioCreateChunk(m_hOPFile, &m_stckOut, 0);
//	if(mRes!=MMSYSERR_NOERROR)
//	{
//		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
//		throw m_csErrorText;
//	}*/
//
//}


void CSoundIn::PrepareBuffers()
{
	::EnterCriticalSection( &m_cs);

	MMRESULT mRes=0;
	int nT1=0;

	for(nT1=0;nT1< SI_MAX_BUFFERS; ++nT1)
	{
		
		MX_ASSERT(m_nWantedDataSize>0);

		m_stWHDR[nT1].lpData=(LPSTR)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY /*8*/, m_nWantedDataSize/*m_stWFEX.nAvgBytesPerSec*/);
		m_stWHDR[nT1].dwBufferLength= m_nWantedDataSize;//m_stWFEX.nAvgBytesPerSec;
		m_stWHDR[nT1].dwUser=nT1;

		mRes=waveInPrepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR)); 

		if(mRes!=0)
		{
			//StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
			::LeaveCriticalSection( &m_cs);
			throw "waveInPrepareHeader error";
			return;

		}
		mRes=waveInAddBuffer(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
		if(mRes!=0)
		{
			//StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
			//throw m_csErrorText;
			::LeaveCriticalSection( &m_cs);

			throw "waveInAddBuffer error";

			return;
		}
	}

	::LeaveCriticalSection( &m_cs);



}

void CSoundIn::UnPrepareBuffers()
{
	::EnterCriticalSection( &m_cs);

	MMRESULT mRes=0;
	int nT1=0;

	if(m_hWaveIn)
	{
		mRes=waveInStop(m_hWaveIn);

		for(nT1=0;nT1<3;++nT1)
		{
			if(m_stWHDR[nT1].lpData)
			{
				mRes = waveInUnprepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
				
				HeapFree(GetProcessHeap(),0,m_stWHDR[nT1].lpData);

				ZeroMemory(&m_stWHDR[nT1],sizeof(WAVEHDR));
			}
		}
	}

	::LeaveCriticalSection( &m_cs);


}


void CSoundIn::Stop()
{
	m_bRun = FALSE;
	
	while(m_hThread)
	{
		SleepEx(100,FALSE);
	}


	/*if (m_hThread){
		::WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread=NULL;

	}*/

	/*
	MMRESULT mRes=0;
	
	if(m_hWaveIn)
	{		
		mRes=waveInClose(m_hWaveIn);
		m_hWaveIn=NULL;
	}		
	*/

}

//
//void CSoundIn::CloseDevice()
//{
//	MMRESULT mRes=0;
//	
//	if(m_hWaveIn)
//	{
//		UnPrepareBuffers();
//		mRes=waveInClose(m_hWaveIn);
//	}
//
//
//	//if(m_hOPFile)
//	//{
//	//	mRes=mmioAscend(m_hOPFile, &m_stckOut, 0);
//	//	if(mRes!=MMSYSERR_NOERROR)
//	//	{
//	//		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
//	//	}
//	//	mRes=mmioAscend(m_hOPFile, &m_stckOutRIFF, 0);
//	//	if(mRes!=MMSYSERR_NOERROR)
//	//	{
//	//		StoreError(mRes,FALSE,"File: %s ,Line Number:%d",__FILE__,__LINE__);
//	//	}
//	//	mmioClose(m_hOPFile,0);
//	//	m_hOPFile=NULL;
//	//}
//
//	m_hWaveIn=NULL;
//}



//UINT CSoundIn::FillDevices()
//{
//	CComboBox *pBox=(CComboBox*)GetDlgItem(IDC_DEVICES);
//	UINT nDevices,nC1;
//	WAVEINCAPS stWIC={0};
//	MMRESULT mRes;
//
//	pBox->ResetContent();
//	nDevices=waveInGetNumDevs();
//
//	for(nC1=0;nC1<nDevices;++nC1)
//	{
//		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
//		mRes=waveInGetDevCaps(nC1,&stWIC,sizeof(WAVEINCAPS));
//		if(mRes==0)
//			pBox->AddString(stWIC.szPname);
//		else
//			StoreError(mRes,TRUE,"File: %s ,Line Number:%d", __FILE__, __LINE__);
//	}
//
//	if(pBox->GetCount())
//	{
//		pBox->SetCurSel(0);
//		OnCbnSelchangeDevices();
//	}
//	return nDevices;
//}


int CSoundIn::GetDeviceCount()
{
	return waveInGetNumDevs();
}

const char* CSoundIn::GetDeviceDescription(int index)
{
	WAVEINCAPS stWIC ;
	MMRESULT mRes;
 
	ZeroMemory(&stWIC,sizeof(WAVEINCAPS));

	mRes =waveInGetDevCaps(index,&stWIC,sizeof(WAVEINCAPS));

	if(mRes == MMSYSERR_NOERROR )
	{
		return stWIC.szPname;
	}

	return NULL;	
}