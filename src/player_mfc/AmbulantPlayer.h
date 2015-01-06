/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

// DemoPlayer.h : main header file for the DemoPlayer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

class CLogWindow;

class CAmCommandLineInfo : public CCommandLineInfo {
  public:
	CAmCommandLineInfo() : m_autostart(false) {}

	void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast) {
		if (bFlag){
			if(_tcsicmp(pszParam, _T("start")) == 0)
				m_autostart = true;
			else
				CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
		} else {
			if (m_strFileName.IsEmpty()) m_strFileName = pszParam;
		}

		if(bLast) {
			m_nShellCommand = m_strFileName.IsEmpty()?FileNew:FileOpen;
		}
	}
	bool m_autostart;
};

// CAmbulantPlayerApp:
// See AmbulantPlayer.cpp for the implementation of this class
//

class CAmbulantPlayerApp : public CWinApp
{
public:
	CAmbulantPlayerApp();


// Overrides
public:
	virtual BOOL InitInstance();
	CSingleDocTemplate* m_pDocTemplate;
	bool m_autostart;
	CString m_recentUrl;

// Implementation
	afx_msg void OnAppAbout();
	afx_msg void OnAppHelp();
	DECLARE_MESSAGE_MAP()
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	afx_msg void OnFileOpen();
	afx_msg BOOL OnOpenRecentFile(UINT nID);
	afx_msg void OnFileOpenurl();
	afx_msg void OnPreferences();
	afx_msg void OnViewLog();
	bool LocateHelpDoc(LPCTSTR rpath);
};

extern CAmbulantPlayerApp theApp;
