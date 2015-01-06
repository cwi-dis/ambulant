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

// MmView.h : interface of the MmView class
//

#pragma once

#include <string>

class CLogWindow;

class MmView : public CView
{
protected: // create from serialization only
	MmView();
	DECLARE_DYNCREATE(MmView)

// Attributes
public:
	MmDoc* GetDocument() const;

// Operations
public:
	void SetMMDocument(LPCTSTR lpszPathName, bool autostart);
	bool LocateWelcomeDoc(LPCTSTR rpath);
	UINT_PTR m_timer_id;
	CString m_curDocFilename;
	CString m_curFilter;
	UINT m_cursor_id;
	bool m_autoplay;
	CString m_welcomeDocFilename;
#ifndef WITHOUT_LOG_WINDOW
	CLogWindow *m_logwindow;
#endif
// Overrides
  public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~MmView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnDestroy();
	virtual void OnInitialUpdate();
	afx_msg void OnFilePlay();
	afx_msg void OnUpdateFilePlay(CCmdUI *pCmdUI);
	afx_msg void OnFilePause();
	afx_msg void OnUpdateFilePause(CCmdUI *pCmdUI);
	afx_msg void OnFileStop();
	afx_msg void OnUpdateFileStop(CCmdUI *pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnFocusAdvance();
	afx_msg void OnFocusActivate();
	afx_msg void OnViewSource();
	afx_msg void OnUpdateViewSource(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewLog(CCmdUI *pCmdUI);
	afx_msg void OnViewLog();
	afx_msg LRESULT OnSetClientRect(WPARAM wParam, LPARAM lParam);
	afx_msg void OnOpenFilter();
	afx_msg void OnViewFilter();
	afx_msg void OnUpdateViewFilter(CCmdUI *pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnUpdateOpenFilter(CCmdUI *pCmdUI);
	afx_msg void OnViewAutoplay();
	void OnUpdateViewAutoplay(CCmdUI *pCmdUI);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg void OnHelpWelcome();
	afx_msg void OnUpdateHelpWelcome(CCmdUI *pCmdUI);
	afx_msg LRESULT OnReplaceDoc(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMyEraseBkgnd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnViewNormalSize();
};

#ifndef _DEBUG  // debug version in MmView.cpp
inline MmDoc* MmView::GetDocument() const { return reinterpret_cast<MmDoc*>(m_pDocument); }
#endif
