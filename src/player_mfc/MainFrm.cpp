// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "afxpriv.h"
#include "AmbulantPlayer.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CMainFrame *theMainFrame;

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
	ON_MESSAGE(WM_APP, OnSetStatusLine)
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullScreen)
	ON_MESSAGE(WM_ERASEBKGND, OnMyEraseBkgnd)

END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,			// status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
:	m_fullScreen(FALSE)
{
	// TODO: add member initialization code here
	theMainFrame = this;
}

CMainFrame::~CMainFrame()
{
	theMainFrame = NULL;
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetStatusLine("No document open");

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;		// fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;		// fail to create
	}
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
#ifdef WITHOUT_DIALOGS
	OnViewFullScreen();
#endif
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// DO NOT ADD DOCUMENT NAME TO TITLE
	cs.style &= ~FWS_ADDTOTITLE;

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//	the CREATESTRUCT cs

	// Set the startup dimensions for MMS
	if (cs.cx <= 0) cs.cx = 640;
	if (cs.cy <= 0) cs.cy = 480;
	//cs.cy = 216 + 108;

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

void
CMainFrame::SetStatusLine(char *message)
{
	USES_CONVERSION;
	m_statusline = message;
	CFrameWnd::OnSetMessageString(0, (LPARAM)A2CT(m_statusline.c_str()));
}

LRESULT
CMainFrame::OnSetStatusLine(WPARAM wParam, LPARAM lParam)
{
	SetStatusLine((char *)lParam);
	return 0;
}

LRESULT
CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	if (wParam == AFX_IDS_IDLEMESSAGE)
		return CFrameWnd::OnSetMessageString(0, (LPARAM)A2CT(m_statusline.c_str()));
	return CFrameWnd::OnSetMessageString(wParam, lParam);
}

afx_msg void
CMainFrame::OnViewFullScreen()
{
	WINDOWPLACEMENT wp;
	wp.showCmd = SW_SHOWNORMAL;
#if 0
	// Disable child window positioning within mainframe, because
	// apparently I (Jack) don't have a clue what I'm doing.
	CView *child = GetActiveView();
	RECT childPos;
	if (child)
		child->GetWindowRect(&childPos);
#endif
	m_fullScreen = !m_fullScreen;

	if (m_fullScreen) {
		// Entering fullscreen mode
		m_wndStatusBar.ShowWindow(SW_HIDE);
		m_wndToolBar.ShowWindow(SW_HIDE);

		// Calculate new frame position
		GetWindowRect(&m_origRect);
		::GetWindowRect(::GetDesktopWindow(), &wp.rcNormalPosition);
		DWORD style = GetStyle();
		DWORD exStyle = GetExStyle();
		m_origStyle = style;
		m_origExStyle = exStyle;
		style = WS_POPUP;
		exStyle = WS_EX_TOPMOST;
		ModifyStyleEx(-1, exStyle, 0);
		ModifyStyle(-1, style, 0);
		::AdjustWindowRectEx(&wp.rcNormalPosition, style, TRUE, exStyle);

	} else {
		// Leaving fullscreen mode
		m_wndStatusBar.ShowWindow(SW_SHOW);
		m_wndToolBar.ShowWindow(SW_SHOW);
		ModifyStyleEx(-1, m_origExStyle, 0);
		ModifyStyle(-1, m_origStyle, 0);
		wp.rcNormalPosition = m_origRect;
	}
	SetWindowPlacement(&wp);
#if 0
	// Center the child document
	if (child) {
		int dx = (wp.rcNormalPosition.right - wp.rcNormalPosition.left) / 2 -
			(childPos.right - childPos.left) / 2;
		int dy = (wp.rcNormalPosition.bottom - wp.rcNormalPosition.top) / 2 -
			(childPos.bottom - childPos.top) / 2;
		child->SetWindowPos(NULL, dx, dy, 0, 0, SWP_NOZORDER);
	}
#endif
	InvalidateRect(NULL);
}

// We don't want MFC to clear our background if we use Direct2D rendering.
afx_msg LRESULT 
CMainFrame::OnMyEraseBkgnd(WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}

void
set_status_line(const char *message)
{
	if (theMainFrame)
		theMainFrame->PostMessage(WM_APP, 0, (LPARAM)message);
}

// CMainFrame message handlers

