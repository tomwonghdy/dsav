// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__86BFF92C_E838_4B09_9281_670D78334CB2__INCLUDED_)
#define AFX_STDAFX_H__86BFF92C_E838_4B09_9281_670D78334CB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdlib.h>
#include <rvb\dsmsp.h>
#include <rvb\rvdefs.h>
#include <sst\ucut.h> 


#include "..\davsdk\includes\mox.h"

#pragma comment(lib,"Winmm")

#pragma comment(lib, "../davsdk/x64/mox_d")
#pragma comment(lib, "../davsdk/x64/ffmo_d")
#pragma comment(lib, "dsm_d")
#pragma comment(lib, "ucut_d")


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__86BFF92C_E838_4B09_9281_670D78334CB2__INCLUDED_)
