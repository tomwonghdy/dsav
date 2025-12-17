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
#ifndef __VIDEO_RENDER_H_INCLUDED_
#define __VIDEO_RENDER_H_INCLUDED_

#include "vmo.h"
#include "..\davsdk\includes\Render.h"
#include "..\davsdk\includes\InputPad.h"
#include "..\davsdk\includes\OutputPad.h"

//#include <rvb\xgui.h"

#define VR_WM_ACTIVE    0
#define VR_WM_PASSIVE   1
#define VR_WM_UNKNOWN   -1

#define VR_FLIP_NONE  0
#define VR_FLIP_HORI  1
#define VR_FLIP_VERT  (1<<1)
#define VR_FLIP_BOTH  (VR_FLIP_HORI | VR_FLIP_VERT)

class VMO_API CVideoRender : public CRender
{
public:
	CVideoRender(void);
	virtual  ~CVideoRender(void);
	
	virtual M_RESULT Play(const char* strUrl);
	virtual void Pause(BOOL bResume);
	virtual void Stop();

	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad);
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)  ;

	virtual BOOL     Accept(CPad* pInputPad, MxDescriptor* pDescriptor);
	  
	virtual void       SetMode(CMediaObject::WORK_MODE mode);

	void AttachWin(HWND hWnd);
	void DetachWin();
	void Refresh();
	
	void SetPictureFormat(MX_VIDEO_FORMAT format, int width, int height);

	void SetFrameRate(int rate);
	int  GetFrameRate() { return m_nFps; };

	void SetFlipType(int type);


	CInputPad*  GetInputPad(){ return   &m_inputPad ;  };
	COutputPad* GetOutputPad(){ return  &m_outputPad ; };

	void HandleCapture();

public:
	BOOL  m_bFixedFormat;

private:
	void PaintSample(MX_HANDLE hSample);


	CInputPad   m_inputPad;
	COutputPad  m_outputPad;
	 
	HANDLE      m_hCaptureThread;

 	RV_HANDLE        m_dc; 
	CRITICAL_SECTION m_cs;

	BOOL        m_bNewArrival ;
	MX_HANDLE   m_hCurSample;
	int m_nFps ; 
	int m_nFlipType ;

};






#endif
