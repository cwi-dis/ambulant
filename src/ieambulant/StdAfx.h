// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently
#define WINVER 0x0500
#if !defined(AFX_STDAFX_H__D94D5A87_3DD0_4178_89A8_3622AE3ADDB3__INCLUDED_)
#define AFX_STDAFX_H__D94D5A87_3DD0_4178_89A8_3622AE3ADDB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D94D5A87_3DD0_4178_89A8_3622AE3ADDB3__INCLUDED)
