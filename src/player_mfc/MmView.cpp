// MmView.cpp : implementation of the MmView class
//

#include "stdafx.h"
#include "DemoPlayer.h"

#include "MmDoc.h"
#include "MmView.h"

#include "ambulant/gui/dx/dx_viewport.h"
#include ".\mmview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace ambulant;

gui::dx::viewport *viewport = 0;

// MmView

IMPLEMENT_DYNCREATE(MmView, CView)

BEGIN_MESSAGE_MAP(MmView, CView)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// MmView construction/destruction

MmView::MmView()
{
	// TODO: add construction code here

}

MmView::~MmView()
{
}

BOOL MmView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// MmView drawing

void MmView::OnDraw(CDC* /*pDC*/)
{
	MmDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	if(viewport)
		viewport->redraw();
	
}


// MmView diagnostics

#ifdef _DEBUG
void MmView::AssertValid() const
{
	CView::AssertValid();
}

void MmView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

MmDoc* MmView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(MmDoc)));
	return (MmDoc*)m_pDocument;
}
#endif //_DEBUG


// MmView message handlers

void MmView::OnDestroy()
{
	delete viewport;
	CView::OnDestroy();
	// TODO: Add your message handler code here
}

void MmView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	if(!viewport)
		viewport = new gui::dx::viewport(176, 216, m_hWnd);
	viewport->redraw();
}
