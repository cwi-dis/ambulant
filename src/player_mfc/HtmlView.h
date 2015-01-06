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

// HtmlView.h : interface of the HtmlView class, implementation of html_widget
//


#pragma once

#ifdef	WITH_HTML_WIDGET
#include "ambulant/gui/d2/html_bridge.h"

class HtmlView : public CHtmlView
{
protected: // create from serialization only
	HtmlView();

	DECLARE_DYNCREATE(HtmlView)

// Attributes
public:
	HtmlView(const RECT& rect, CWnd* parent);
	void InitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	MmDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~HtmlView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
};

// Bridge class
class html_browser_imp : public html_browser {
public:
	html_browser_imp(int left, int top, int width, int height);
	~html_browser_imp();
	void goto_url(std::string url, ambulant::net::datasource_factory *df);
	void show();
	void hide();
	void redraw();
	bool uses_screen_reader();
private:
	HtmlView *m_browser;
};

inline html_browser *new_html_browser(int left, int top, int width, int height)
{
	return new html_browser_imp(left, top, width, height);
}

#else // WITH_HTML_WIDGET
inline html_browser *new_html_browser(int left, int top, int width, int height)
{
	return NULL;
}
#endif // WITH_HTML_WIDGET
