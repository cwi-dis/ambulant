// AmbulantPlayerDoc.cpp : implementation of the CAmbulantPlayerDoc class
//

#include "stdafx.h"
#include "AmbulantPlayer.h"

#include "AmbulantPlayerDoc.h"
#include "AmbulantPlayerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerDoc

IMPLEMENT_DYNCREATE(CAmbulantPlayerDoc, CDocument)

BEGIN_MESSAGE_MAP(CAmbulantPlayerDoc, CDocument)
	//{{AFX_MSG_MAP(CAmbulantPlayerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerDoc construction/destruction

CAmbulantPlayerDoc::CAmbulantPlayerDoc()
{
	// TODO: add one-time construction code here

}

CAmbulantPlayerDoc::~CAmbulantPlayerDoc()
{
}

BOOL CAmbulantPlayerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerDoc serialization

#ifndef _WIN32_WCE_NO_ARCHIVE_SUPPORT
void CAmbulantPlayerDoc::Serialize(CArchive& ar)
{
	(ar);
}
#endif // !_WIN32_WCE_NO_ARCHIVE_SUPPORT


/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerDoc diagnostics

#ifdef _DEBUG
void CAmbulantPlayerDoc::AssertValid() const
{
	CDocument::AssertValid();
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerDoc commands

BOOL CAmbulantPlayerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	//if (!CDocument::OnOpenDocument(lpszPathName))
	//	return FALSE;

	POSITION pos = GetFirstViewPosition();
	if(pos != NULL) {
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		((CAmbulantPlayerView*)pView)->SetMMDocument(lpszPathName);
	}

	return TRUE;
}
