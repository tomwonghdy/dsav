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
#include "OutputPad.h"

//#include "publics.h"


COutputPad::COutputPad(void)
{

 	m_nStreamType = ST_OUTPUT;
	m_nRate       = MX_DEFAULT_VIDEO_FPS;
}

COutputPad::~COutputPad(void)
{

}


//MX_HANDLE COutputPad::GetNextFrame()
//{
//	return NULL;
//}


M_RESULT COutputPad::Connect(CPad* pInputPad)
{
	RV_ASSERT(pInputPad);
	 

	CUtAutoLock lock(&m_csLock);


	if (m_bConnected) return M_OCCUPIED;

	if (pInputPad->GetStreamType() != ST_INPUT) {
		return M_FORMAT_MISMATCH;
	}

	M_RESULT ret = ((CInputPad*)pInputPad)->HandleConnection(this);

	RV_ASSERT(m_pOwner);

	if (ret == M_OK){

		m_bConnected = TRUE;
		m_pCounterPart = pInputPad;
		
		if (m_pOwner->OnPadConnected(this, pInputPad)){
			m_nFrameCount = 0;
		   return M_OK;
		}
		else{

			((CInputPad*)pInputPad)->HandleConnection(this, TRUE);

			m_bConnected = FALSE;
		    m_pCounterPart = NULL;

			ret = M_FAILED;
		}
	}


	return ret;
}
	
void  COutputPad::Disconnect()
{
	CUtAutoLock lock(&m_csLock);

	if (m_bConnected == FALSE) return;
	RV_ASSERT(m_pCounterPart);
	RV_ASSERT(m_pOwner);

	
	((CInputPad*)m_pCounterPart)->HandleConnection(this, TRUE);

	m_pOwner->OnPadDisconnected(this, m_pCounterPart);

	m_pCounterPart =NULL;

	m_bConnected = FALSE;
}


//
//M_RESULT COutputPad::Initialize(MX_MEDIA_TYPE mediaType, CAPTURE_TYPE captureType)
//{
//	// m_nMediaType = mediaType;
//	 m_nCaptureType = captureType;
//	 
//	 return M_OK;
//}
//	
//void COutputPad::Finalize()
//{
//}
//

BOOL   COutputPad::Pass(MX_HANDLE pSample, int* pErrCode )
{
	
	RV_ASSERT(this->m_nCaptureType == CT_PUSH || m_nCaptureType == CT_ANY);

	CUtAutoLock lock(&m_csLock);

	if (m_bConnected){
		RV_ASSERT(this->m_pCounterPart);

		if (m_pCounterPart){
			if (NULL == m_pCounterPart->OnTransferSample(pSample, CT_PUSH, pErrCode)) {
				m_nFrameCount++;
				return TRUE;
			}
		}
	}



	return FALSE;

}

BOOL COutputPad::ValidateStamp(__int64 stamp, int type, double* pDifference) 
{
	CUtAutoLock lock(&m_csLock);

	if (m_bConnected){
		RV_ASSERT(this->m_pCounterPart);

		if (m_pCounterPart){
			if (m_pCounterPart->m_pOwner)
			{				
				return m_pCounterPart->m_pOwner->CheckStamp((CInputPad*)m_pCounterPart, stamp, type, pDifference);
			}
		}
	}

	if (pDifference) *pDifference = 0.0;

	//由于运行可能延迟，默认情况下返回TRUE.
	return TRUE;

}