// AmbulantPlayerView.h : interface of the CAmbulantPlayerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_AMBULANTPLAYERVIEW_H__39E37311_AA72_400D_BB0A_5145222534D7__INCLUDED_)
#define AFX_AMBULANTPLAYERVIEW_H__39E37311_AA72_400D_BB0A_5145222534D7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CAmbulantPlayerView : public CView
{
protected: // create from serialization only
	CAmbulantPlayerView();
	DECLARE_DYNCREATE(CAmbulantPlayerView)

// Attributes
public:
	CAmbulantPlayerDoc* GetDocument();

	void SetMMDocument(LPCTSTR lpszPathName);
	UINT_PTR m_timer_id;
	CString m_curDocFilename;
	CString m_curFilter;
	UINT m_cursor_id;
	bool m_autoplay;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAmbulantPlayerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAmbulantPlayerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAmbulantPlayerView)
	afx_msg void OnPlay();
	afx_msg void OnUpdatePlay(CCmdUI* pCmdUI);
	afx_msg void OnPause();
	afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
	afx_msg void OnStop();
	afx_msg void OnUpdateStop(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHelpWelcome();
	afx_msg void OnFileSelect();
	afx_msg void OnFileLoadSettings();
	afx_msg LPARAM OnReplaceDoc(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in AmbulantPlayerView.cpp
inline CAmbulantPlayerDoc* CAmbulantPlayerView::GetDocument()
   { return (CAmbulantPlayerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AMBULANTPLAYERVIEW_H__39E37311_AA72_400D_BB0A_5145222534D7__INCLUDED_)
