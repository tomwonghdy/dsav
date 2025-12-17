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
#include "Pad.h" 

#include <rvb\dsm.h> 


CPad::CPad(void)
{
	 m_pCounterPart=NULL;
	// m_nMediaType = MX_MT_UNKNOWN;
	 m_bConnected = FALSE;
	 m_nFrameCount = 0;

	 m_nCaptureType = CT_PULL;
 	 m_nStreamType = ST_INPUT;
	 m_pMediaDesc  = NULL;
	 m_nStreamType = ST_UNKNOWN;

 	 m_pOwner = NULL;

	 ::InitializeCriticalSection(&m_csLock);


}

CPad::~CPad(void)
{
	if (this->m_pMediaDesc) {
		if (m_pMediaDesc->type == MX_MT_AUDIO) {
			MxAudioDesc* p = (MxAudioDesc*)m_pMediaDesc;
			delete p; 
		}
		else if (m_pMediaDesc->type == MX_MT_VIDEO) {
			MxVideoDesc* p = (MxVideoDesc*)m_pMediaDesc;
			delete p; 
		}
		else {
			MX_ASSERT(0); 
		}

		m_pMediaDesc = NULL;
	}
	
	::DeleteCriticalSection(&m_csLock);
}


MX_HANDLE CPad::OnTransferSample(MX_HANDLE hSample, CAPTURE_TYPE type, int* pErrCode)
{ 
	if (pErrCode)  *pErrCode = M_OK;

	RV_ASSERT(m_pOwner);

	if (type == CT_PULL){
	//	RV_ASSERT( m_nStreamType == ST_OUTPUT );
		if (this->m_pOwner){
			MX_HANDLE hSample = m_pOwner->OnSampleRequest(this,  /*hSample, */pErrCode);
			if (hSample)
			{
				m_nFrameCount++;
			}
			return hSample;
		}
	}
	else if (type == CT_PUSH){
		//RV_ASSERT( m_nStreamType == ST_INPUT );

		//如果该PAD所在的MEDIA OBJECT将SAMPLE接收，
		//则返回NULL；
		//否则，返回输入的hSample。
		if (this->m_pOwner){
			if (m_pOwner->OnSampleReceived(this, hSample, pErrCode)){
				m_nFrameCount++;
				return NULL;
			}
		}

		return hSample;

	}
	else{
		RV_ASSERT(0);
	}

	if (pErrCode)  *pErrCode = M_FAILED;

	return NULL;
}

//
//void CPad::Append(MX_HANDLE pSample )
//{
//	//CUtAutoLock lock(&m_csLock);
//
//	RV_ASSERT(pSample);
//
//	//rlsAddTail(m_arrData, pSample);
//	//m_arrData.AddTail(pSample);
//	 
//}

//int  CPad::GetSampleCount()
//{
//	//CUtAutoLock lock(&m_csLock);
//
//	return rlsGetCount(m_arrData);//.GetCount();
//}


MX_MEDIA_TYPE  CPad::GetMediaType(){
	 
	if (m_pMediaDesc == NULL) {
		return MX_MT_UNKNOWN;
	}

	return m_pMediaDesc->type; 
}

void  CPad::SetMediaType(MX_MEDIA_TYPE type)
{
	if (m_pMediaDesc) {
		if (type == m_pMediaDesc->type) {
			return;
		} 

		if (m_pMediaDesc->type == MX_MT_AUDIO) {
			MxAudioDesc* p = (MxAudioDesc*)m_pMediaDesc;
			delete p;
			m_pMediaDesc = NULL;
		}
		else if (m_pMediaDesc->type == MX_MT_VIDEO) {
			MxVideoDesc* p = (MxVideoDesc*)m_pMediaDesc;
			delete p;
			m_pMediaDesc = NULL;
		}
		else {
			MX_ASSERT(0);
			return;
		}
	}

	if ( type == MX_MT_AUDIO) {
		MxAudioDesc* p = new MxAudioDesc ;
		MX_ASSERT(p);

		mxResetAudioDesc(p, AV_DEFAULT_AUDIO_FORMAT, 44100, 2, 1204);
		m_pMediaDesc = (MxDescriptor*)p;
	}
	else if ( type == MX_MT_VIDEO) {
		MxVideoDesc* p = new  MxVideoDesc ;
		MX_ASSERT(p);
	 
		mxResetVideoDesc(p, AV_DEFAULT_VIDEO_FORMAT, AV_DEFAULT_VIDEO_WIDTH, AV_DEFAULT_VIDEO_HEIGHT, mxGetPixelDepth(AV_DEFAULT_VIDEO_FORMAT));

		m_pMediaDesc = (MxDescriptor*)p;
	}
	else {
		MX_ASSERT(0);
		return;
	} 

}

MxDescriptor* CPad::GetFormatDescriptor()
{
	return m_pMediaDesc;
	//if (NULL == this->m_pOwner){
	//	//TRACE("Owner of this pad(%x) is missing.\r\n", ((DWORD)this));
	//	return NULL;
	//}

	//return m_pOwner->GetFormatDescriptor(this);

}


BOOL CPad::NegotiateDescriptor(MxDescriptor* pDescriptor)
{
	 CUtAutoLock lock(&m_csLock);

	if (!m_bConnected) return FALSE;

	RV_ASSERT(m_pCounterPart) ;
	RV_ASSERT(m_pCounterPart->m_pOwner ) ;

	return m_pCounterPart->m_pOwner->Accept(m_pCounterPart, pDescriptor); 

}

MxDescriptor* CPad::EnquireDescriptor()
{
	 CUtAutoLock lock(&m_csLock);

	if (!m_bConnected) return NULL;

	RV_ASSERT(m_pCounterPart) ;
	RV_ASSERT(m_pCounterPart->m_pOwner ) ;

	return m_pCounterPart->GetFormatDescriptor();

}

BOOL  CPad::EnquireProperty(const char* strName, char* strValue, int nValueSize)
{
	 CUtAutoLock lock(&m_csLock);

	if (!m_bConnected) return FALSE;

	RV_ASSERT(m_pCounterPart) ;
	RV_ASSERT(m_pCounterPart->m_pOwner ) ;

	return m_pCounterPart->m_pOwner->OnPropertyEnquired(m_pCounterPart, strName, strValue, nValueSize); 

}

BOOL   CPad::EnquireVariable(int nName, void* pValue)
{
	CUtAutoLock lock(&m_csLock);

	if (!m_bConnected) return FALSE;

	RV_ASSERT(m_pCounterPart) ;
	RV_ASSERT(m_pCounterPart->m_pOwner ) ;

	return m_pCounterPart->m_pOwner->OnVariableEnquired(m_pCounterPart, nName, pValue); 
}

__int64 CPad::GetFrameCount()
{
	return m_nFrameCount;
}