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
#ifndef __MX_GRAPH_H_INCLUDED_
#define __MX_GRAPH_H_INCLUDED_


#include "mox.h"

//通知类型
#define MX_GN_GAME_OVER      1  //play finished
#define MX_GN_START_PLAY     2  //play
#define MX_GN_STOP_PLAY      3  //stop
#define MX_GN_PAUSE_PLAY     4  //pause

#define MX_GN_USER_BASE      100

//自定义消息
#define MX_GM_BASE          1016
#define MX_GM_GAME_OVER    (WM_USER + MX_GM_BASE + MX_GN_GAME_OVER)
#define MX_GM_START        (WM_USER + MX_GM_BASE + MX_GN_START_PLAY)
#define MX_GM_STOP         (WM_USER + MX_GM_BASE + MX_GN_STOP_PLAY)
#define MX_GM_PAUSE        (WM_USER + MX_GM_BASE + MX_GN_PAUSE_PLAY)

#define MX_GM_USER         (WM_USER + MX_GM_BASE + MX_GN_USER_BASE)
//
//class CMediaObject;
// 
class MOX_API CGraph
{
public:
	enum STATE{
		GS_STOPPED = MX_STOPPED,
		GS_PLAYING = MX_PLAYING,
		GS_PAUSED  = MX_PAUSED,
	};
	 

	CGraph(void); 
	virtual ~CGraph(void);

	virtual M_RESULT Create(HWND hNotifyWnd, MO_STATUS_REPORT_FUNC pfStatusReport = NULL, MO_ERROR_OCCURRED_FUNC pfErrorOccurred = NULL, void* pUserData = NULL)
	{
		return M_OK;
	}
	virtual void     Destroy() {};
	 
	virtual BOOL     Start(const TCHAR* strUrl , int* pErrCode) =0;
	virtual void     Pause(){ 
		if (m_nState == GS_PLAYING) m_nState = GS_PAUSED ;  
		else if (m_nState == GS_PAUSED) m_nState = GS_PLAYING ;    	
	}

	virtual void     Stop()=0 ;

	virtual void     Refresh() { ; };
	 
	int   GetCurState(){ return m_nState; };
	//HWND  GetParentWnd() {return m_hParentWnd;};
	  
	 
protected: 
	
	STATE m_nState;	 
	//HWND  m_hParentWnd; 
	//void* m_pUserData;
};




#endif
