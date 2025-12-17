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
#include "InputPad.h"
#include "outputpad.h"

CInputPad::CInputPad(void)
{
	//	m_pOwner = NULL;
	m_nStreamType = ST_INPUT;
}

CInputPad::~CInputPad(void)
{

}

//MX_HANDLE CInputPad::Receive(void)
//{
//	RV_ASSERT(this->m_pCounterPart);
//
//	return m_pCounterPart->Fetch();
//
//}
//


MX_HANDLE CInputPad::Fetch(int* pErrCode)
{

	RV_ASSERT(this->m_nCaptureType == CT_PULL || m_nCaptureType == CT_ANY);

//	CUtAutoLock lock(&m_csLock);
	CUtAutoLock lock(&m_csLock);

	if (m_bConnected) {
		RV_ASSERT(m_pCounterPart);

		if (m_pCounterPart) {
			MX_HANDLE hSample =  m_pCounterPart->OnTransferSample(NULL, CT_PULL, pErrCode);
			if (hSample) {
				m_nFrameCount++;
			}
			return hSample;
		}

	}

	return NULL;
}



M_RESULT CInputPad::HandleConnection(CPad* pOutputPad, BOOL bBreak)
{
	RV_ASSERT(pOutputPad);
	//RV_ASSERT( ((COutputPad*)pOutputPad)->GetStreamType() == ST_OUTPUT );

	RV_ASSERT(m_pOwner);

	CUtAutoLock lock(&m_csLock);
	 

	if (bBreak == FALSE) {
		if (m_bConnected) return M_OCCUPIED;

		MxDescriptor* pOutDesc = pOutputPad->GetFormatDescriptor();
		MxDescriptor* pInDesc = this->GetFormatDescriptor();

		if (NULL == pOutDesc || pInDesc == NULL) {
			return M_INVALID_OBJECT;
		}

		if (pOutDesc->type == MX_MT_UNKNOWN) {
			return M_UNKNOWN_TYPE;
		}

		if (pOutDesc->type != pInDesc->type) {
			return M_FORMAT_MISMATCH;
		}
		
		if (pOutputPad->GetCaptureType() != this->GetCaptureType() ) {
			if (pOutputPad->GetCaptureType() != CT_ANY && GetCaptureType() != CT_ANY) {
				return M_FORMAT_MISMATCH;
			}			
		}

		if (m_pOwner->OnPadConnected(pOutputPad, this)) {
			m_bConnected = TRUE;
			m_pCounterPart = pOutputPad;
			return M_OK;
		} 



	}
	else {
		if (m_bConnected) {
			RV_ASSERT(m_pCounterPart == pOutputPad);
			m_pCounterPart = NULL;

			this->m_bConnected = FALSE;

			m_pOwner->OnPadDisconnected(pOutputPad, this);

		}

		return M_OK;
	}

	return M_FAILED;
}

void CInputPad::ReportTraffic(int state, double masterClock, __int64 lastPts)
{
	CUtAutoLock lock(&m_csLock);

	if (m_bConnected) {
		RV_ASSERT(this->m_pCounterPart);

		if (m_pCounterPart) {
			if (m_pCounterPart->m_pOwner)
			{
				return m_pCounterPart->m_pOwner->OnTrafficReported((COutputPad*)m_pCounterPart, state, masterClock, lastPts);
			}
		}
	}
}