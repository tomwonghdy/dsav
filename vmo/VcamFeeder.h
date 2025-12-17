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
#ifndef __VCAM_FEEDER_H_INCLUDED_
#define __VCAM_FEEDER_H_INCLUDED_

#include "vmo.h"
#include <rvb\rvtypes.h>

#include "..\davsdk\includes\sink.h"
#include "..\davsdk\includes\inputpad.h"
#include "..\davsdk\includes\ImageConverter.h"
 




 
class VMO_API CVcamFeeder :	public CSink
{
 
public:
 	enum FLIP_TYPE 
	{
		FT_NONE = -1,
		FT_HORI = RV_DIR_HORI,
		FT_VERT = RV_DIR_VERT,
		FT_BOTH = RV_DIR_BOTH,
	};


	CVcamFeeder();
	virtual ~CVcamFeeder();

	//仅工作在被动模式
	virtual void       SetMode(CMediaObject::WORK_MODE mode){};

		
	virtual M_RESULT Play(const char* strUrl ) ;
	virtual void     Pause(BOOL bResume) ;
	virtual void     Stop() ;
 
	CInputPad*  GetInputPad( ){return &m_inputPad;}

	virtual BOOL  Accept(CPad* pInputPad, MxDescriptor* pDescriptor);
 
	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad);
	
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode);
 
	//virtual MxDescriptor*      GetFormatDescriptor(CPad* pPad);		 
	void    SetImageSize(int width, int height);
	 
public:
	 
	FLIP_TYPE m_nFlipType;
	
	  
private:
	void Reset(int width, int height);
private: 

	CImageConverter     m_cvter;
	CInputPad           m_inputPad;
	MxVideoDesc         m_inDesc;
	MxVideoDesc         m_outDesc;
	
	BOOL                m_bPipeStarted;
	TCHAR*  m_strPipeName;
	RvImage m_image;

	void* m_pClientPipe; 

	CRITICAL_SECTION m_cs;

};






#endif