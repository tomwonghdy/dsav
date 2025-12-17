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
#include "MediaObject.h"
 

CMediaObject::CMediaObject(void)
{

	m_pid=0;

	m_nTotalDuation = 0;
	m_nCurrentPos   = -1;

	m_nWorkMode = WM_UNKNOWN;

	  
	m_nState = MS_STOPPED;


	m_pfErrorOccurred = NULL;
	m_pfStatusReport  = NULL;
	m_pUserData       = NULL;

	//m_pSyncTimer =NULL;
}

CMediaObject::~CMediaObject(void)
{ 

}

//void CMediaObject::SetSyncTimer(CSyncTimer* pTimer)
//{
//	m_pSyncTimer = pTimer ;
//}

void CMediaObject::SetMode(CMediaObject::WORK_MODE mode)
{
	//m_nWorkMode = mode;
}

void CMediaObject::ResetBitmapInfo(BITMAPINFO* pInfo, int width, int height, int depth){



	if (pInfo){
		BITMAPINFOHEADER* pHeader = &pInfo->bmiHeader;

		int iLineBytes =( width * depth / 8 +3) / 4 * 4  ;

		pHeader->biWidth  = width ; 
		pHeader->biHeight = height ; 
		pHeader->biPlanes = 1; 
		pHeader->biBitCount    = depth; 
		pHeader->biCompression = 0; 
		pHeader->biSizeImage   = iLineBytes  *  pHeader->biHeight    ; 
		pHeader->biXPelsPerMeter = 0; 
		pHeader->biYPelsPerMeter = 0; 
		pHeader->biClrUsed       = 0; 
		pHeader->biClrImportant  = 0; 

		pHeader->biSize = sizeof(BITMAPINFOHEADER) + pHeader->biSizeImage  ; 		 

	}

}

void CMediaObject::ResetBitmapInfo(BITMAPINFO* pInfo, MxVideoDesc* pDesc)
{
	RV_ASSERT(pDesc && pInfo);

	/*int depth = 24;
	if (pDesc->format  == MX_VF_GRAY) depth = 8;
	else if (pDesc->format  == MX_VF_RGBA) depth = 32;*/

	ResetBitmapInfo(pInfo, pDesc->width, pDesc->height , pDesc->depth);

}


void CMediaObject::ResetWaveInfo(WAVEFORMATEX* pInfo, int  depth,  int   sampleRate,  int  channelCount)
{

	if (pInfo){
		memset(pInfo, 0, sizeof(WAVEFORMATEX));
		pInfo->nChannels   =  channelCount; // m_audio_dec_ctx->channels;
		pInfo->wFormatTag  =  WAVE_FORMAT_PCM;
		pInfo->nSamplesPerSec = sampleRate; // m_audio_dec_ctx->sample_rate;
		pInfo->wBitsPerSample = depth; //
		pInfo->nBlockAlign    = (pInfo->wBitsPerSample * pInfo->nChannels) >> 3;
		pInfo->nAvgBytesPerSec = pInfo->nBlockAlign * pInfo->nSamplesPerSec;
		pInfo->cbSize          = 0;

	}
 
}


void CMediaObject::ResetWaveInfo(WAVEFORMATEX* pInfo, MxAudioDesc* pDesc){
	ResetWaveInfo(pInfo, pDesc->depth, pDesc->sampleRate, pDesc->channelCount);
}


	
UINT  CMediaObject::CalcWaveBufferSize(int depth, int channels, int framecount){

	UINT sze =0;
	sze = depth / 8;
	sze *= channels;
	sze *= framecount;

	return sze;

}

UINT  CMediaObject::CalcBitmapBufferSize(int depth, int width, int height, int alignBytes)
{
	int iLineBytes =( width * depth / 8 +3) / 4 * 4  ;

	if (alignBytes== MX_IMAGE_ALIGN_1){
	    iLineBytes =int(width * depth / 8.f);
	}

	return  (UINT) iLineBytes   *  (UINT)height ; 			
}

void  CMediaObject::SetUserData(void* pUserData)
{
	m_pUserData = pUserData;
}

void   CMediaObject::SetExceptionHandler(MO_ERROR_OCCURRED_FUNC pfErrorOccurred )
{
	m_pfErrorOccurred = pfErrorOccurred;
}

void   CMediaObject::SetStatusReportHandler( MO_STATUS_REPORT_FUNC pfStatusReport )
{
	m_pfStatusReport = pfStatusReport;
}

void CMediaObject::RaiseErrorReport(int errCode)
{
	if (m_pfErrorOccurred)
	{  
		m_pfErrorOccurred(this, errCode, m_pUserData);
	}
}

void CMediaObject::RaiseStatusReport(int stateCode)
{
	if (m_pfStatusReport)
	{  
		m_pfStatusReport(this, stateCode, m_pUserData);
	}
}
 