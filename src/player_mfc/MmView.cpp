// MmView.cpp : implementation of the MmView class
//

#include "stdafx.h"
#include "DemoPlayer.h"

#include "MmDoc.h"
#include "MmView.h"

#include "MmViewPlayer.h"

#include ".\mmview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static MmViewPlayer *player = 0;

//static 
MmViewPlayer* MmViewPlayer::get_instance() {
	return player;
}

// MmView

IMPLEMENT_DYNCREATE(MmView, CView)

BEGIN_MESSAGE_MAP(MmView, CView)
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_PLAY, OnFilePlay)
	ON_UPDATE_COMMAND_UI(ID_FILE_PLAY, OnUpdateFilePlay)
	ON_COMMAND(ID_FILE_PAUSE, OnFilePause)
	ON_UPDATE_COMMAND_UI(ID_FILE_PAUSE, OnUpdateFilePause)
	ON_COMMAND(ID_FILE_STOP, OnFileStop)
	ON_UPDATE_COMMAND_UI(ID_FILE_STOP, OnUpdateFileStop)
	ON_WM_TIMER()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// MmView construction/destruction

MmView::MmView()
{
	// TODO: add construction code here
	m_timer_id = 0;
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
	if(player)
		player->redraw();
	
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
int MmView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	player = new MmViewPlayer(m_hWnd);
	//m_timer_id = SetTimer(1, 500, 0);

	return 0;
}

void MmView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	if(player)
		player->redraw();

}

void MmView::OnDestroy()
{
	delete player;
	player = 0;
	if(m_timer_id) KillTimer(m_timer_id);
	CView::OnDestroy();
	// TODO: Add your message handler code here
}

void MmView::SetMMDocument(LPCTSTR lpszPathName) {
	if(!player) {
		AfxMessageBox("Player is null");
		return;
	}
	player->set_document(lpszPathName);
}

void MmView::OnFilePlay()
{
	player->start();
}

void MmView::OnUpdateFilePlay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((player != 0 && player->can_start())?TRUE:FALSE);
	
}

void MmView::OnFilePause()
{
	player->pause();
}

void MmView::OnUpdateFilePause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((player && player->can_pause())?TRUE:FALSE);
}

void MmView::OnFileStop()
{
	player->stop();
}

void MmView::OnUpdateFileStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((player && player->can_stop())?TRUE:FALSE);
}

void MmView::OnTimer(UINT nIDEvent)
{
	if(player)
		player->update_status();
	CView::OnTimer(nIDEvent);
}

