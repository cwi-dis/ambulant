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
};

#ifndef _DEBUG  // debug version in MmView.cpp
inline MmDoc* MmView::GetDocument() const
   { return reinterpret_cast<MmDoc*>(m_pDocument); }
#endif

