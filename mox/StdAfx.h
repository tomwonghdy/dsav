// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__38A621F0_CDEF_4B0E_9CCA_111EF6CCD2AC__INCLUDED_)
#define AFX_STDAFX_H__38A621F0_CDEF_4B0E_9CCA_111EF6CCD2AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <rvb\mpm.h>
#include <rvb\dsm.h>
#include <rvb\dsmsp.h>

#include <rvb\xgui.h> 
#include <rvb\rvdefs.h> 
#include <rvb\idsm.h> 
#include <sst\ucut.h> 




#pragma comment(lib, "dsm_d")
#pragma comment(lib, "mpm_d")
#pragma comment(lib, "ucut_d")



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__38A621F0_CDEF_4B0E_9CCA_111EF6CCD2AC__INCLUDED_)
