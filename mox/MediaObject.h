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
#ifndef __MX_MEDIA_OBJECT_H_INCLUDED_
#define __MX_MEDIA_OBJECT_H_INCLUDED_

 

#include "mox.h"
#include "graph.h"
#include "pad.h" 
//#include "synctimer.h"




//
//
//
////ERROR REPORT
//#define MO_ER_NONE             0
//#define MO_FORMAT_MISMATCH     -1  //输入或输出接口的媒体格式不匹配


//最大的属性名称长度。
#define MO_MAX_PROPERTY_NAME_LEN    128
 

class  CPad;
class  CInputPad;
class  COutputPad;




class MOX_API CMediaObject
{
public:
	enum MEDIA_STATE{
		MS_STOPPED = MX_STOPPED,
		MS_PLAYING = MX_PLAYING,
		MS_PAUSED  = MX_PAUSED,
	};
 
	
	enum WORK_MODE{
		WM_UNKNOWN =-1,
		WM_ACTIVE,
		WM_PASSIVE,
		WM_HYBRID,
	};


	CMediaObject(void);
	virtual ~CMediaObject(void);

	virtual M_RESULT Play(const char* strUrl ){ return M_FAILED;};
	virtual void Pause(BOOL bResume){;};
	virtual void Stop(){;};
  
	virtual LONGLONG GetCurrentPos() { return m_nCurrentPos; };
	LONGLONG         GetTotalDuation() { return m_nTotalDuation; };

	//重载的函数，在默认的情况下，必须返回TRUE;
	//否则，将连接失败。
	virtual BOOL OnPadConnected(CPad* pOutputPad, CPad* pInputPad){   return TRUE;};
	virtual void OnPadDisconnected(CPad* pOutputPad, CPad* pInputPad){};

	//获得媒体对象输出PIN的媒体类型
	//virtual MX_MEDIA_TYPE GetMediaType(CPad* pOutputPad){ RV_ASSERT(0); return MX_MT_UNKNOWN; };

	//virtual MxDescriptor* GetFormatDescriptor(CPad* pPad){ return NULL;};

	//当接收到一个SAMPLE的时候,产生该事件。
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode)=0;

	////当接收方通过PAD接收SAMPLE时,产生该事件。
	virtual MX_HANDLE OnSampleRequest(CPad* pOutputPad,/* MX_HANDLE hSample,*/ int* pErrCode)=0;
	 

	//是否接受输入PAD的流媒体格式描述符
	//具体格式协商的时候用，一般输入PIN所在的MO实现。
	virtual BOOL Accept(CPad* pInputPad, MxDescriptor* pDescriptor){RV_ASSERT(0); return FALSE;};

 	virtual void       SetMode(CMediaObject::WORK_MODE mode);
 	CMediaObject::WORK_MODE GetMode() { return m_nWorkMode; };


	MEDIA_STATE GetCurState(){ return m_nState; } ;
	
	 
	static void ResetBitmapInfo(BITMAPINFO* pInfo, int width, int height, int depth);
	static void ResetBitmapInfo(BITMAPINFO* pInfo, MxVideoDesc* pDesc);
 
	//depth is bits per sample
	static void ResetWaveInfo(WAVEFORMATEX* pInfo, int  depth,  int   sampleRate,  int  channelCount);
	static void ResetWaveInfo(WAVEFORMATEX* pInfo, MxAudioDesc* pDesc);

	static UINT  CalcWaveBufferSize(int depth, int channels, int framecount);
	static UINT  CalcBitmapBufferSize(int depth, int width, int height, int alignBytes);

	void  SetUserData(void* pUserData);
	void  SetExceptionHandler(MO_ERROR_OCCURRED_FUNC pfErrorOccurred );
	void  SetStatusReportHandler( MO_STATUS_REPORT_FUNC pfStatusReport );

	void*  GetUserData(){ return m_pUserData; };

	void RaiseErrorReport(int errCode);
	void RaiseStatusReport(int stateCode);
	 	
	 
	 

//pad related functions for traffic control
public:

	//上游的媒体对象如要通过下游的媒体对象对时间戳，或播放位置进行检验，
	//以确认是否已经过时。
	//媒体对象重载这个方法进行时间戳的检验。
	//一般情况下，不检验可以直接返回TRUE。
	virtual  BOOL  CheckStamp(CInputPad* pInputPad, __int64 stamp, int type ,double* pDifference) { if (pDifference) *pDifference = 0; return TRUE;};

	//当媒体对象收到下游对象发送的流量状况的时候，发生的事件。媒体对象通过
	//重载该时间进行状态接收。
	virtual  void  OnTrafficReported(COutputPad* pOutputPad, int state, double masterClock, __int64 lastPts) { ;};

	//媒体对象向上游或下游的对象查询指定属性的值
	virtual  BOOL OnPropertyEnquired(CPad* pPad, const char* strName, char* strValue, int nValueSize) { return FALSE; };

	//媒体对象向上游或下游的对象查询指定变量的值
	virtual  BOOL OnVariableEnquired(CPad* pPad, int nName, void* pValue) { return FALSE; };

	
public:
				
	__int64 m_pid; //performance id， 每次播放生成的id,可以时时间错，或序列号

protected:

	MO_ERROR_OCCURRED_FUNC m_pfErrorOccurred;
	MO_STATUS_REPORT_FUNC  m_pfStatusReport;
	     
	void*                  m_pUserData;


 	CMediaObject::WORK_MODE            m_nWorkMode;

	MEDIA_STATE     m_nState;


	__int64    m_nTotalDuation;
	__int64    m_nCurrentPos;

};





#endif
