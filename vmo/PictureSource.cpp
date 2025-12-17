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
#include "PictureSource.h"

#include "..\davsdk\includes\ffmo.h"


CPictureSource::CPictureSource(void)
{
	//m_image =NULL;
	m_hSendThread =NULL;
	m_nWorkMode = WM_PASSIVE;
	m_nLastStamp = GetAvTime();
	this->m_outputPad.m_pOwner =this;

	//m_image = rvCreateImage(RIT_RGB,320, 240);
	ResetVideoDescEx( &m_videoOutDesc, MX_VF_BGR, 320, 240 );
	 
	m_seq = rvCreateSequence(NULL);

	m_nCunIndex =-1;
	m_nPeriod = 25; //播放周期


	::InitializeCriticalSection(&m_csSample);

}


CPictureSource::~CPictureSource(void)
{
	/*if (m_image){
	    rvDestroyImage(m_image);
		m_image = NULL;
	}*/
	if (m_seq)
	{
		this->RemoveAll();
		rvDestroySequence(m_seq);
		m_seq=NULL;
	}

		
	::DeleteCriticalSection(&m_csSample);
}

	

DWORD WINAPI DataSendThreadPS(LPVOID lpParameter)
{
	CPictureSource* pFf = (CPictureSource* )lpParameter;
	MX_ASSERT(pFf);
	 
	pFf->HandleSendImage();

	return 0;
}


M_RESULT CPictureSource::Play(const char* _strUrl) 
{
	//CUtAutoLock lock(&m_csSample);

	if (m_nState != MS_STOPPED ) return M_FAILED;

	if (_strUrl){
		char strUrl[MAX_PATH];
	   
	     ParseUrl(_strUrl, strUrl, MAX_PATH);

		RvImage im = rvLoadImage(strUrl);

	    if (NULL == im) return M_FAILED;

		rviCast(im, RIT_RGB);

		if (!SetOutputFormat(MX_VF_BGR, rviGetWidth(im), rviGetHeight(im)) ){
			rvDestroyImage(im);
		    return M_FAILED;
		}

		SetImage(im);

		rvDestroyImage(im);

		if (this->m_nWorkMode == WM_ACTIVE)
		{
			
			m_hSendThread = ::CreateThread(NULL, 0, DataSendThreadPS, this, 0, NULL);
			MX_ASSERT(m_hSendThread);
		}
		 
		m_nState = MS_PLAYING;

		return M_OK;
	}
	 

	return M_FILE_NONEXIST;
}
	
void     CPictureSource::Pause(BOOL bResume) 
{
	m_nState = MS_PAUSED;
}
	
void     CPictureSource::Stop()
{
	m_nState = MS_STOPPED;

	if (m_hSendThread){
		::WaitForSingleObject(m_hSendThread, INFINITE);
		::CloseHandle(m_hSendThread);
		m_hSendThread = NULL;
	}

	
}


MX_MEDIA_TYPE CPictureSource::GetMediaType(CPad* pPad)
{
	RV_ASSERT(pPad == &m_outputPad);
	return MX_MT_VIDEO;
}

BOOL CPictureSource::CheckMediaType(CInputPad* pInputPad, MX_MEDIA_TYPE type) 
{
     return (type == MX_MT_VIDEO);
}

BOOL CPictureSource::OnPadConnected(CPad* pOutputPad, CPad* pInputPad)
{
	if (pOutputPad == &m_outputPad)
	{
		return pOutputPad->NegotiateDescriptor((MxDescriptor*) (&m_videoOutDesc));
	}

	return FALSE;
}


MxDescriptor*     CPictureSource::GetFormatDescriptor(CPad* pPad)
{
	return ((MxDescriptor*)(&m_videoOutDesc));
}

void    CPictureSource::SetMode(CMediaObject::WORK_MODE mode)
{
		
	if (m_nWorkMode == mode) return ;
	if (m_nState != MS_STOPPED) return;


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

 

//视频输出格式,只能是BGR格式
BOOL CPictureSource::SetOutputFormat(int format, int width, int height)
{
	RV_ASSERT(width >0 && height > 0);

	if (MX_VF_BGR != format) return FALSE;
	if (m_nState != MS_STOPPED) return FALSE;

	/*m_bHasData = FALSE;

	if (m_image == NULL){
	    m_image = rvCreateImage(RIT_RGB, width, height);
	}
	else{
	    rviResize(m_image, width, height);
	}*/

	ResetVideoDescEx(&m_videoOutDesc, format,   width,  height);

	if (this->m_outputPad.IsConnected()){
		if (m_outputPad.NegotiateDescriptor((MxDescriptor*)&m_videoOutDesc)== FALSE){
			m_outputPad.Disconnect();
			RaiseErrorReport(M_FORMAT_MISMATCH);

			return FALSE;
		}
	}	

	return TRUE;

}

BOOL CPictureSource::SetImage(const RvImage image)
{
	if (NULL == image) return FALSE;
	MX_ASSERT(m_seq);

	if (rviGetImageType(image) != RIT_RGB)  return FALSE;
	if (rviGetWidth(image) != m_videoOutDesc.width) return FALSE;
	if (rviGetHeight(image) != m_videoOutDesc.height) return FALSE;

	this->RemoveAll();
	{
	    CUtAutoLock lock(&m_csSample);
        RvImage im = rviClone(image);

		rsqAddLast_(m_seq, im);
		m_nCunIndex = 0;
	}

	if (m_nState == MS_PLAYING) 
	{
		this->SendImage();
	} 

	return TRUE;
}

BOOL  CPictureSource::SetImage(const char* strFilePath)
{
	if (NULL == strFilePath) return FALSE;

	RvImage im = rvLoadImage(strFilePath);

	if (NULL == im) return FALSE;
	 
	BOOL b = this->SetImage(im);

	rvDestroyImage(im);

	return b;
}


////当接收方通过PAD接收SAMPLE时,产生该事件。
MX_HANDLE CPictureSource::OnSampleRequest(CPad* pOutputPad, /*MX_HANDLE hSample,*/ int* pErrCode)
{
	
	if (m_nWorkMode != WM_PASSIVE) return NULL;
  
	 //RV_ASSERT(NULL ==hSample);
	 if ( m_nState == MS_STOPPED) 
	 {
		  if (pErrCode) *pErrCode = M_TERMINATED;
		  return NULL;
	 }

	//  if (this->m_bHasData ==FALSE) return NULL;
	 MX_ASSERT(m_seq);
	 
	 CUtAutoLock lock(&m_csSample);

	 if (rsqGetCount(m_seq) <=0 ) return NULL;

	 RvImage im = NULL;

	 LONGLONG now = GetAvTime();
	  
	 if ((now - m_nLastStamp) >= 1000/RV_MAX(1,m_nPeriod)){
		 m_nCunIndex ++ ;
		 m_nLastStamp = now;
	 }
	 
	 m_nCunIndex %= rsqGetCount(m_seq);

	 im = rsqGetItemValue_(m_seq, m_nCunIndex );
	 MX_ASSERT(im);


	 MX_HANDLE hNew = mxCreateSample((MxDescriptor *)&m_videoOutDesc, sizeof(MxVideoDesc), rviGetData(im), rviGetSize(im));
	 RV_ASSERT(hNew);

	 __int64  pts = GetAvTime();
	 mxSetSampleOptions(hNew, pts, NULL, NULL);
	  
	return hNew;
}

int   CPictureSource::GetImageCount()
{
	CUtAutoLock lock(&m_csSample);

	if (m_seq){
	    return rsqGetCount(m_seq);
	}

	return 0;

}

void  CPictureSource::RemoveAll()
{
	 CUtAutoLock lock(&m_csSample);

	if (m_seq){
	    RvSeqReader sr;
		RV_BEGIN_READ_SEQ(m_seq, &sr);

		RvImage im=NULL;
		while(!RV_IS_SEQ_END(&sr)){
		     RV_READ_FR_SEQ_(&sr, RvImage, im);
			 MX_ASSERT(im);

			 rvDestroyImage(im);
		}

		RV_END_READ_SEQ(m_seq, &sr);

		rsqClear(m_seq);
	}
}

void CPictureSource::HandleSendImage()
{
	MX_ASSERT(m_seq);
	int cnt = rsqGetCount(m_seq);
	if (cnt <=0) return; //为0的情况

	this->m_nCunIndex = 0;
	if (cnt == 1){  //只有一个帧的情况
	    SendImage();
		return;
	}

	__int64 st =GetAvTime();
	int delay = 1000 /RV_MAX(1, this->m_nPeriod);
	

	while(m_nState != MS_STOPPED){
		if (m_nState == MS_PAUSED) {
			Sleep(30);
			continue;
		}

		__int64 n =GetAvTime();

		if ((n - st) >= delay)
		{
			SendImage();

			m_nCunIndex ++;
			m_nCunIndex %= cnt;

			st =GetAvTime();
		}

		Sleep(10);
	}

}


void CPictureSource::SendImage()
{
	MX_ASSERT(m_seq);

	if (m_nWorkMode == WM_ACTIVE)
	{
		CUtAutoLock lock(&m_csSample);

		MX_ASSERT(m_nCunIndex>=0 && m_nCunIndex< rsqGetCount(m_seq));

		RvImage im = NULL;		 
		im = rsqGetItemValue_(m_seq, m_nCunIndex );
		MX_ASSERT(im); 

		MX_HANDLE hNew = mxCreateSample((MxDescriptor *)&m_videoOutDesc, sizeof(MxVideoDesc), rviGetData(im), rviGetSize(im));
		RV_ASSERT(hNew);

		__int64  pts = GetAvTime();
		mxSetSampleOptions(hNew, pts, NULL, NULL);

		if (m_outputPad.IsConnected()){
			if (!m_outputPad.Pass(hNew, NULL)){
				mxDestroySample(hNew);					 
			}
		}
		else{
			if (this->m_fnSamplePass){
				m_fnSamplePass(this, &m_outputPad, hNew, this->m_pUserData);
			}

			mxDestroySample(hNew);
		}


	}
}