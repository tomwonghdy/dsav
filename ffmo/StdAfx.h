// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__30514511_26BC_467B_8EB4_D34C8C84124B__INCLUDED_)
#define AFX_STDAFX_H__30514511_26BC_467B_8EB4_D34C8C84124B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define _WIN32_WINNT   0x0400 

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <rvb\img.h>
#include <rvb\dsm.h>
#include <rvb\mpm.h>
#include <sst\ucut.h>


#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h> 
#include <libavutil/fifo.h>
#include <libavutil/error.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/frame.h>
}
 
 // 回调函数的参数，用了时间
typedef struct {
	time_t lasttime;
	int    timeout;
}TimeoutMonitor;

#pragma comment(lib, "../davsdk/x64/mox_d")
 

#pragma comment(lib, "Winmm.lib")

#pragma comment(lib, "avcodec") 
#pragma comment(lib, "avformat")
#pragma comment(lib, "avutil")
#pragma comment(lib, "swresample")
#pragma comment(lib, "swscale")
#pragma comment(lib, "avdevice")
#pragma comment(lib, "avfilter")

#pragma comment(lib, "ucut_d")

 

 

#pragma comment(lib, "xgui_d")
#pragma comment(lib, "dsm_d")
#pragma comment(lib, "img_d")
#pragma comment(lib, "mpm_d")


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__30514511_26BC_467B_8EB4_D34C8C84124B__INCLUDED_)
