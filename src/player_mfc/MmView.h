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

// MmView.h : interface of the MmView class
//


#pragma once


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
};

#ifndef _DEBUG  // debug version in MmView.cpp
inline MmDoc* MmView::GetDocument() const
   { return reinterpret_cast<MmDoc*>(m_pDocument); }
#endif

