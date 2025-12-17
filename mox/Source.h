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
#ifndef __MX_SOURCE_H_INCLUDED_
#define __MX_SOURCE_H_INCLUDED_

//#include "MediaSample.h"
#include "MediaObject.h"
#include "outputpad.h"


//source type flag 
#define ST_NONE         0
#define ST_VIDEO	    1 
#define ST_AUDIO       (1<<1)
#define ST_SUBTITLE    (1<<2)
#define ST_ACTIVEPAGE  (1<<3)

#define ST_URL_START_DELIMIT      "://"
#define ST_URL_SUB_DELIMIT        ":"
#define ST_URL_PATH_DELIMIT        "/"
#define ST_URL_PARAM_LEADER        "?"
#define ST_URL_AND                 "&"
#define ST_URL_EQUAL               "="


#define ST_URL_CAM_LEADER   "cam://"
#define ST_URL_FILE_LEADER  "file://"
#define ST_URL_MIC_LEADER   "mic://"
#define ST_URL_RTSP_LEADER  "rtsp://"




class MOX_API CSource: public  CMediaObject
{
public:
	CSource(void);
	virtual ~CSource(void);

	
	//当接收到一个SAMPLE的时候,产生该事件。
	//对于SOURCE类来说是不可能出现这种事件的，
	//故必须将其屏蔽（实现CMediaObject的重载函数）
	//CSource子类不应该再实现该事件
	virtual BOOL OnSampleReceived( CPad* pInputPad, MX_HANDLE hSample, int* pErrCode){if (pErrCode) *pErrCode = M_FAILED; return FALSE; };

	//virtual COutputPad* GetVideoPad(){return NULL;}
	//virtual COutputPad* GetAudioPad(){return NULL;}
	//virtual COutputPad* GetSubtitlePad(){return NULL;}
	//virtual COutputPad* GetActivePagePad(){return NULL;}



	BOOL  HasVideoStream();
	BOOL  HasAudioStream();
	BOOL  HasSubtitleStream();
	BOOL  HasActivePageStream();

	//
	//fiLe://d:\video\spy4.avi?startpos=20&duration=200
    //dev://v:/webcam/0
	//parameter name:
	//  startpos
	//  duration 
	//
	char* ParseUrl(const char* strUrl, char* strNew, int nNewSize);

	void   SetSamplePassHandler(MO_SAMPLE_PASS_FUNC pfnSamplePass );
	MO_SAMPLE_PASS_FUNC GetSamplePassHandler();

	static BOOL ParseField(const TCHAR* strText, const TCHAR* strName, TCHAR* strValue, int valueSize);
	static BOOL ParseField(const TCHAR* strText, const TCHAR* strName, int* pValue);
	static BOOL ParseField(const TCHAR* strText, const TCHAR* strName, float* pValue);
	static BOOL ParseField(const TCHAR* strText, const TCHAR* strName, bool* pValue);
	static BOOL ParseField(const TCHAR* strText, const TCHAR* strName, double* pValue);




public:
	//毫秒
	__int64 m_nPlayDuration;
	__int64 m_nStartPos;
	
protected:	
	//下面2函数过时
	void ParseParam(const char* strText);
	//解析名称和值(值也是字符串）等式,如: path=c:\myapppath , age=18
	BOOL ParseParam(const char* strText, const char* strName, char* strValue, int valueSize);

protected:
	MO_SAMPLE_PASS_FUNC  m_fnSamplePass;
	int  m_nStreamFlags ;

	//HANDLE  m_hGameOverEvent;


};



#endif 
