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

// MainFrm.h : interface of the CMainFrame class
//
#include <string>

void set_status_line(const char *message);

#pragma once
class CMainFrame : public CFrameWnd
{

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	/// Called by Ambulant code to set the status line
	void SetStatusLine(char *message);
	/// Called by Ambulant (maybe from other threads)
	LRESULT OnSetStatusLine(WPARAM wParam, LPARAM lParam);

	/// Called my MFC when it wants to display the status line
	LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);

	afx_msg void OnViewFullScreen();

	afx_msg LRESULT OnMyEraseBkgnd(WPARAM wParam, LPARAM lParam);

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	std::string	m_statusline;

	BOOL m_fullScreen;
	RECT m_origRect;
	DWORD m_origStyle;	// Style of window, kept before going fullscreen
	DWORD m_origExStyle;	// exStyle of window, kept before going fullscreen
// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
};


