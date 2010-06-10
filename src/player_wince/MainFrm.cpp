// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "AmbulantPlayer.h"

#include "MainFrm.h"
#include "AmbulantPlayerDoc.h"
#include "AmbulantPlayerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Array tbSTDButton contains relevant buttons of bitmap IDB_STD_SMALL_COLOR
static TBBUTTON tbButtons[] = {
	{0, 0,				TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0},
	{0, ID_FILE_OPEN,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0, 0,				TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{1, ID_PLAY,	    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{2, ID_PAUSE,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{3, ID_STOP,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{5, 0,				TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{4, ID_APP_ABOUT,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0, 0,				TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}
};
const int nNumButtons = sizeof(tbButtons)/sizeof(TBBUTTON);
const int nNumImages = 5;
const DWORD dwAdornmentFlags = 0; // exit button

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_ACTIVATE()
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here

}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
#if 1 // XXXJACK wm5 may want this:
	if (!m_wndCommandBar.Create(this) ||
	    !m_wndCommandBar.InsertMenuBar(IDR_MAINFRAME) ||
	    !m_wndCommandBar.AddAdornments(dwAdornmentFlags) ||
	    !m_wndCommandBar.LoadToolBar(IDR_MAINFRAME))
#else
	// Add the buttons and adornments to the CommandBar.
	if (!InsertButtons(tbButtons, nNumButtons, IDR_MAINFRAME, nNumImages) ||
	    !AddAdornments(dwAdornmentFlags))
#endif
	{
		TRACE0("Failed to add toolbar buttons\n");
		return -1;
	}

#if 1 // XXXXJACK wm5
	m_wndCommandBar.SetBarStyle(m_wndCommandBar.GetBarStyle() | CBRS_SIZE_FIXED);
#endif
	return 0;
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs


	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

#if 1 // XXXJACK wm5 may not want this
void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	CWnd* otherOwner = pWndOther?pWndOther->GetTopLevelOwner():0;
	if(nState == WA_INACTIVE && otherOwner && otherOwner != this)
		PostMessage(WM_CLOSE);
}
#endif

LRESULT CMainFrame::OnCopyData(WPARAM wParam, LPARAM lParam)
{
   COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;

   // Send the command line to the main ambulant window.
   if (mainAmbulantWindow) {
	   mainAmbulantWindow->SetMMDocument((LPCTSTR)cds->lpData);
   }

   return 0;
}