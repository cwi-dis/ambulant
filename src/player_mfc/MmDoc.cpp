// MmDoc.cpp : implementation of the MmDoc class
//

#include "stdafx.h"
#include "DemoPlayer.h"

#include "MmDoc.h"
#include ".\mmdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// MmDoc

IMPLEMENT_DYNCREATE(MmDoc, CDocument)

BEGIN_MESSAGE_MAP(MmDoc, CDocument)
END_MESSAGE_MAP()


// MmDoc construction/destruction

MmDoc::MmDoc()
{
	// TODO: add one-time construction code here

}

MmDoc::~MmDoc()
{
}

BOOL MmDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// MmDoc serialization

void MmDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// MmDoc diagnostics

#ifdef _DEBUG
void MmDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void MmDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// MmDoc commands

BOOL MmDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  Add your specialized creation code here
	POSITION pos = GetFirstViewPosition();
	if(pos != NULL) {
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		//AfxMessageBox(CString("OnOpenDocument ") + lpszPathName);
	}
	return TRUE;
}
