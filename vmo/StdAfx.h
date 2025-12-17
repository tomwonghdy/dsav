// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C86F2F77_10EE_4A42_805B_4FC9F66203AD__INCLUDED_)
#define AFX_STDAFX_H__C86F2F77_10EE_4A42_805B_4FC9F66203AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT   0x0400 

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdlib.h>

#include <rvb\img.h>
#include <rvb\imio.h>
#include <rvb\dsmsp.h>
#include <rvb\rvdefs.h>
#include <sst\ucut.h>




#include <rvb\pprs.h>
#include <rvb\xgui.h>

#include "..\davsdk\includes\mox.h"

//#include <sst\ucut.h"

#pragma comment(lib, "../davsdk/x64/mox_d")
#pragma comment(lib, "../davsdk/x64/ffmo_d")
#pragma comment(lib, "xgui_d")
#pragma comment(lib, "img_d")
#pragma comment(lib, "dsm_d")
#pragma comment(lib, "pprs_d")
#pragma comment(lib, "imio_d")
#pragma comment(lib, "ucut_d")
 
#pragma comment(lib, "../cblibs/x64/CamBridge_d")
 

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C86F2F77_10EE_4A42_805B_4FC9F66203AD__INCLUDED_)
