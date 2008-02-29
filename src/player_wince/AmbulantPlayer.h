// AmbulantPlayer.h : main header file for the AMBULANTPLAYER application
//

#if !defined(AFX_AMBULANTPLAYER_H__82DC84C7_B57C_45F6_9782_371606145240__INCLUDED_)
#define AFX_AMBULANTPLAYER_H__82DC84C7_B57C_45F6_9782_371606145240__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerApp:
// See AmbulantPlayer.cpp for the implementation of this class
//

class CAmbulantPlayerApp : public CWinApp
{
public:
	CAmbulantPlayerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAmbulantPlayerApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL InitApplication();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAmbulantPlayerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AMBULANTPLAYER_H__82DC84C7_B57C_45F6_9782_371606145240__INCLUDED_)
