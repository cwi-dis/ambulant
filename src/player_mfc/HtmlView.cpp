// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

// MmView.cpp : implementation of the MmView class
//

#include "stdafx.h"
#include "ambulant/gui/dx/html_bridge.h"

#include "MmDoc.h"
#include "HtmlView.h"
#include <string>
#include "ambulant/lib/logger.h"

using namespace ambulant;

#ifdef	WITH_HTML_WIDGET

// #define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// HtmlView

IMPLEMENT_DYNCREATE(HtmlView, CHtmlView)

BEGIN_MESSAGE_MAP(HtmlView, CHtmlView)
	// Standard printing commands
//	ON_COMMAND(ID_FILE_PRINT, HtmlView::OnFilePrint)
	ON_COMMAND(AFX_IDC_BROWSER, HtmlView::OnInitialUpdate)
END_MESSAGE_MAP()

// HtmlView construction/destruction

HtmlView::HtmlView()
{
	// TODO: add construction code here
	// JUNK
}

HtmlView::HtmlView(const RECT& rect, CWnd* parent)
{
	// TODO: add construction code here
	Create(NULL,_T("HtmlWidget"),WS_VISIBLE,rect,parent,AFX_IDW_PANE_FIRST);
}

HtmlView::~HtmlView()
{
//	Detach();
//	DestroyWindow();
}

BOOL HtmlView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CHtmlView::PreCreateWindow(cs);
}

void HtmlView::InitialUpdate()
{
//	Navigate2(_T("http://www.google.nl/"),NULL,NULL);
}

void HtmlView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();
	InitialUpdate();
//KB	Navigate2(_T("http://www.ambulantplayer.org"),NULL,NULL);
}

#ifdef _DEBUG
void HtmlView::AssertValid() const
{
	CView::AssertValid();
}

void HtmlView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

extern CWnd*  topView;
//KB HtmlView* s_browser = NULL;
CWinThread* s_ambulant_thread = NULL;

html_browser::html_browser(int left, int top, int width, int height)
: m_browser(NULL) {
	RECT rect;
	rect.left = left;
	rect.top = top;
	rect.right = left + width;
	rect.bottom = top + height;
	if ( ! m_browser) {
		HtmlView* browser = new HtmlView(rect, topView);
//KB	HtmlView* browser = new HtmlView(rect, CWnd::GetForegroundWindow());
		assert (browser != NULL);
		browser->InitialUpdate();
		if ( ! s_ambulant_thread) {
			s_ambulant_thread = AfxGetThread();
			s_ambulant_thread->SetThreadPriority(THREAD_PRIORITY_LOWEST);
		}
		m_browser = browser;
		hide();
	} 
	AM_DBG lib::logger::get_logger()->debug("html_browser::html_browser(0x%x): LTWH=(%d,%d,%d,%d) m_browser=0x%x", this, left, top, width, height, m_browser);
}

html_browser::~html_browser() {
	AM_DBG lib::logger::get_logger()->debug("html_browser::~html_browser(0x%x)", this);
	HtmlView* browser = (HtmlView*) m_browser;
	ShowWindow(browser->m_hWnd, SW_HIDE);
//KB DestroyWindow(browser->m_hWnd);
//KB delete browser;
//KB s_browser = NULL;
}

void
html_browser::goto_url(std::string url) {
	CString CSurl(url.c_str());
	AM_DBG lib::logger::get_logger()->debug("html_browser::goto_url(0x%x): url=%s)", this, url.c_str());
	HtmlView* browser = (HtmlView*) m_browser;
	browser->Navigate2(CSurl,NULL,_T(""));
}

void
html_browser::hide() {
	AM_DBG lib::logger::get_logger()->debug("html_browser::hide(0x%x)", this);
	HtmlView* browser = (HtmlView*) m_browser;
	ShowWindow(browser->m_hWnd, SW_HIDE);
}

void
html_browser::show() {
	AM_DBG lib::logger::get_logger()->debug("html_browser::show(0x%x)", this);
	HtmlView* browser = (HtmlView*) m_browser;
	ShowWindow(browser->m_hWnd, SW_SHOW);
}

void
html_browser::redraw() {
	AM_DBG lib::logger::get_logger()->debug("html_browser::redraw(0x%x)", this);
//	HtmlView* browser = (HtmlView*) m_browser;
//	ShowWindow(browser->m_hWnd, SW_SHOWNA);
}
#else	// WITH_HTML_WIDGET
#endif // WITH_HTML_WIDGET
