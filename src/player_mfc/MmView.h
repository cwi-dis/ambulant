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
	UINT_PTR m_timer_id;
	
// Operations
public:
	void SetMMDocument(LPCTSTR lpszPathName);
	
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
};

#ifndef _DEBUG  // debug version in MmView.cpp
inline MmDoc* MmView::GetDocument() const
   { return reinterpret_cast<MmDoc*>(m_pDocument); }
#endif

