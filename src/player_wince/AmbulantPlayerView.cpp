// AmbulantPlayerView.cpp : implementation of the CAmbulantPlayerView class
//

#include "stdafx.h"
#include "AmbulantPlayer.h"

#include "AmbulantPlayerDoc.h"
#include "AmbulantPlayerView.h"
#include "SelectDlg.h"

// DG Player
#include "ambulant/gui/dg/dg_player.h"
#include "ambulant/gui/dg/dg_wmuser.h"
// DX player
//#include "ambulant/gui/dx/dx_player.h"
//#include "ambulant/gui/dx/dx_wmuser.h"

#include "ambulant/common/preferences.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/win32/win32_fstream.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/net/url.h"

#ifdef _DEBUG
#pragma comment (lib,"mp3lib_D.lib")
#else
#pragma comment (lib,"mp3lib.lib")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ambulant;

typedef gui::dg::dg_player dg_or_dx_player;
typedef gui::dg::dg_player_callbacks gui_callbacks;

// The handle of the single window instance
static HWND s_hwnd;

// A class with callbacks, also instantiated once
class my_player_callbacks : public gui_callbacks {
  public:
	HWND new_os_window();
	void destroy_os_window(HWND hwnd);
	HWND get_main_window();
};

my_player_callbacks s_player_callbacks;

HWND my_player_callbacks::new_os_window() {
	// Return the handle of the single instance for now
	// This means paint bits of the new window
	// to the single instance
	return s_hwnd;
}

void my_player_callbacks::destroy_os_window(HWND hwnd) {
	// none for now; keep the single instance
}

HWND my_player_callbacks::get_main_window() {
	return AfxGetMainWnd()->GetSafeHwnd();
}


static dg_or_dx_player* 
create_player_instance(const net::url& u) {
	return new dg_or_dx_player(s_player_callbacks, NULL, u);
}

void lib::win32::show_message(int level, const char *message) {
	unsigned int type = MB_OK|MB_TASKMODAL|MB_ICONERROR;
	if (level == lib::logger::LEVEL_WARN) type |= MB_ICONWARNING;
	if (level == lib::logger::LEVEL_ERROR) type |= MB_ICONERROR;
	if (level == lib::logger::LEVEL_FATAL) type |= MB_ICONERROR;
	HWND top = GetForegroundWindow();
	MessageBox(top, textptr(message), text_str("AmbulantPlayer"), type);
}

static  dg_or_dx_player *player = 0;
static bool needs_done_redraw = false;

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView

IMPLEMENT_DYNCREATE(CAmbulantPlayerView, CView)

BEGIN_MESSAGE_MAP(CAmbulantPlayerView, CView)
	//{{AFX_MSG_MAP(CAmbulantPlayerView)
	ON_COMMAND(ID_PLAY, OnPlay)
	ON_UPDATE_COMMAND_UI(ID_PLAY, OnUpdatePlay)
	ON_COMMAND(ID_PAUSE, OnPause)
	ON_UPDATE_COMMAND_UI(ID_PAUSE, OnUpdatePause)
	ON_COMMAND(ID_STOP, OnStop)
	ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
	ON_WM_DESTROY()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND(ID_HELP_WELCOME, OnHelpWelcome)
	ON_COMMAND(ID_FILE_SELECT, OnFileSelect)
	ON_COMMAND(ID_FILE_LOADSETTINGS, OnFileLoadSettings)
	ON_MESSAGE(WM_REPLACE_DOC, OnReplaceDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView construction/destruction

CAmbulantPlayerView::CAmbulantPlayerView()
{
	m_timer_id = 0;
	m_cursor_id = 0;
	m_autoplay = true;
	lib::logger::get_logger()->set_show_message(lib::win32::show_message);
}

CAmbulantPlayerView::~CAmbulantPlayerView()
{
}

BOOL CAmbulantPlayerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

int CAmbulantPlayerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_timer_id = SetTimer(1, 1000, 0);
	
	// Set static handle
	s_hwnd = GetSafeHwnd();
	

	lib::win32::fstream *fs = new lib::win32::fstream();
	if(fs->open_for_writing(TEXT("\\Windows\\Ambulant\\amlog.txt")))
		lib::logger::get_logger()->set_ostream(fs);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView drawing

void CAmbulantPlayerView::OnDraw(CDC* pDC)
{
	//CAmbulantPlayerDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);
	if(player)
		player->redraw(m_hWnd, pDC->m_hDC);
}

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView diagnostics

#ifdef _DEBUG
void CAmbulantPlayerView::AssertValid() const
{
	CView::AssertValid();
}

CAmbulantPlayerDoc* CAmbulantPlayerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAmbulantPlayerDoc)));
	return (CAmbulantPlayerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerView message handlers

void CAmbulantPlayerView::SetMMDocument(LPCTSTR lpszPathName) {
	dg_or_dx_player *dummy = player;
	player = 0;
	if(dummy) {
		dummy->stop();
		delete dummy;
	}
	lib::textptr tp(lpszPathName);
	net::url u = net::url::from_filename(tp.c_str());
	dummy = create_player_instance(u);
	m_curDocFilename = lpszPathName;
	player = dummy;
	if(m_autoplay)
		PostMessage(WM_COMMAND, ID_PLAY);
}

void CAmbulantPlayerView::OnPlay() 
{
	if(player) {
		player->play();
		needs_done_redraw = true;
	}
	
}

void CAmbulantPlayerView::OnUpdatePlay(CCmdUI* pCmdUI) 
{
	bool enable = player && player->is_play_enabled();
	pCmdUI->Enable(enable?TRUE:FALSE);
	
}

void CAmbulantPlayerView::OnPause() 
{
	if(player) player->pause();
	
}

void CAmbulantPlayerView::OnUpdatePause(CCmdUI* pCmdUI) 
{
	bool enable = player && player->is_pause_enabled();
	pCmdUI->Enable(enable?TRUE:FALSE);
	if(enable) pCmdUI->SetCheck(player->is_pause_active()?1:0);
	
}

void CAmbulantPlayerView::OnStop() 
{
	if(player) {
		net::url u = player->get_url();
		dg_or_dx_player *dummy = player;
		player = 0;
		if(dummy) {
			dummy->stop();
			delete dummy;
		}
		dummy = create_player_instance(u);
		player = dummy;
		PostMessage(WM_INITMENUPOPUP,0, 0); 
		InvalidateRect(NULL);
		needs_done_redraw = false;
	}
}

void CAmbulantPlayerView::OnUpdateStop(CCmdUI* pCmdUI) 
{
	bool enable = player && (player->is_stop_enabled());
	pCmdUI->Enable(enable?TRUE:FALSE);
	
}

void CAmbulantPlayerView::OnDestroy() 
{
	if(player) {
		player->stop();
		delete player;
		player = 0;
	}
	lib::logger::get_logger()->set_ostream(0);
	if(m_timer_id) KillTimer(m_timer_id);
	CView::OnDestroy();
	
}

void CAmbulantPlayerView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(player) player->on_char(nChar);
	CView::OnChar(nChar, nRepCnt, nFlags);
}

void CAmbulantPlayerView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(player) player->on_click(point.x, point.y, GetSafeHwnd());
	CView::OnLButtonDown(nFlags, point);
}


void CAmbulantPlayerView::OnTimer(UINT nIDEvent) 
{
	if(player && needs_done_redraw && player->is_stop_active()) {
		OnStop();
	}
	
	CView::OnTimer(nIDEvent);
}

void CAmbulantPlayerView::OnHelpWelcome() 
{
	CString welcomeFilename("\\Windows\\Ambulant\\Welcome.smil");
	SetMMDocument(welcomeFilename);
	if(!m_autoplay)
		PostMessage(WM_COMMAND, ID_PLAY);

}

void CAmbulantPlayerView::OnFileSelect() 
{
	CSelectDlg dlg(this);
	if(dlg.DoModal() == IDOK) {
		CString fileName = dlg.GetPathName();
		if(!fileName.IsEmpty()) {
			SetMMDocument(fileName);
			if(!m_autoplay)
				PostMessage(WM_COMMAND, ID_PLAY);
		}
	}
}

void CAmbulantPlayerView::OnFileLoadSettings() 
{
	BOOL bOpenFileDialog = TRUE;
	CString strDefExt = TEXT("*.xml");
	DWORD flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	CString strFilter = TEXT("Settings Files (*.xml)|*.xml|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, strDefExt, NULL, flags, strFilter, this);
	dlg.m_ofn.lpstrTitle = TEXT("Select settings file");
	if(dlg.DoModal() == IDOK) {
		m_curFilter = dlg.GetPathName();
		const char *fn = lib::textptr(LPCTSTR(m_curFilter)).c_str();
		load_test_attrs(fn);
		if(player && !m_curDocFilename.IsEmpty()) {
			SetMMDocument(m_curDocFilename);
		}
	}
}

LPARAM CAmbulantPlayerView::OnReplaceDoc(WPARAM wParam, LPARAM lParam) {
	if(lParam == 0) return 0;
	std::string *purlstr = (std::string *)lParam;
	SetMMDocument(lib::textptr(purlstr->c_str()).c_wstr());
	delete purlstr;
	return 0;
}
