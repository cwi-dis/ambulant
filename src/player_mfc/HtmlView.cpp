/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

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
	Navigate2(_T("http://www.google.nl/"),NULL,NULL);
}

void HtmlView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();
	InitialUpdate();
//KB	Navigate2(_T("http://www.ambulantplayer.org"),NULL,NULL);
}

extern CWnd*  topView;
HtmlView* s_browser = NULL;
CWinThread* s_ambulant_thread = NULL;

html_browser::html_browser(int left, int top, int width, int height)
: m_browser(NULL) {
	RECT rect;
	rect.left = left;
	rect.top = top;
	rect.right = left + width;
	rect.bottom = top + height;
	if ( ! s_browser) {
		HtmlView* browser = new HtmlView(rect, topView);
//KB	HtmlView* browser = new HtmlView(rect, CWnd::GetForegroundWindow());
		assert (browser != NULL);
		browser->InitialUpdate();
		s_ambulant_thread = AfxGetThread();
		s_ambulant_thread->SetThreadPriority(THREAD_PRIORITY_LOWEST);
		m_browser = s_browser = browser;
		hide();
	} else m_browser = s_browser;
}

html_browser::~html_browser() {
	lib::logger::get_logger()->debug("html_browser::~html_browser(0x%x)", this);
	HtmlView* browser = (HtmlView*) m_browser;
	ShowWindow(browser->m_hWnd, SW_HIDE);
	delete m_browser;
	s_browser = NULL;
}

void
html_browser::goto_url(std::string url) {
	CString CSurl(url.c_str());
	lib::logger::get_logger()->debug("html_browser::goto_url(0x%x): url=%s)", this, url.c_str());
	HtmlView* browser = (HtmlView*) m_browser;
	browser->Navigate2(CSurl,NULL,_T(""));
}

void
html_browser::hide() {
	lib::logger::get_logger()->debug("html_browser::hide(0x%x)", this);
	HtmlView* browser = (HtmlView*) m_browser;
	ShowWindow(browser->m_hWnd, SW_HIDE);
}

void
html_browser::show() {
	lib::logger::get_logger()->debug("html_browser::show(0x%x)", this);
	HtmlView* browser = (HtmlView*) m_browser;
	ShowWindow(browser->m_hWnd, SW_SHOW);
}
#else	// WITH_HTML_WIDGET
#endif // WITH_HTML_WIDGET
