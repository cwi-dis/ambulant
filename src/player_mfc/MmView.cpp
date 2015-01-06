// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// MmView.cpp : implementation of the MmView class
//

#include "stdafx.h"
#include "ambulant/config/config.h"
#include "ambulant/gui/d2/html_bridge.h"
#include "ambulant/common/player.h"
#include "AmbulantPlayer.h"
#include "MainFrm.h"

#include "MmDoc.h"
#include "MmView.h"
#include "LogWindow.h"
#include "ShowMessage.h"

#include <fstream>
#include <string>


#include "ambulant/common/preferences.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/net/url.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/version.h"

#include "MmView.h"
#include "HtmlView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef WITHOUT_LOG_WINDOW
const TCHAR log_name[] = TEXT("amlog.txt");

static std::string get_log_filename() {
	TCHAR buf[_MAX_PATH];
	GetModuleFileName(NULL, buf, _MAX_PATH);
	TCHAR *p1 = text_strrchr(buf,'\\');
	if(p1 != NULL) *p1='\0';
	text_strcat(buf, TEXT("\\"));
	text_strcat(buf, log_name);
	return std::string(ambulant::lib::textptr(buf).str());
}
#endif // WITHOUT_LOG_WINDOW

static TCHAR *get_directory(const TCHAR *fn) {
	static TCHAR buf[_MAX_PATH];
	buf[0] = 0;
	_tcscat_s(buf, sizeof(buf)/sizeof(buf[0]), fn);
	TCHAR *p1 = text_strrchr(buf,'\\');
	if(p1 != NULL) *p1='\0';
	return buf;
}

#ifdef WITHOUT_LOG_WINDOW
std::ofstream
log_os(get_log_filename().c_str());
#endif

using namespace ambulant;
#include "ambulant/gui/d2/d2_player.h"
#include "ambulant/gui/d2/wmuser.h"

// The handle of the single window instance
static HWND s_hwnd;

class html_browser;

// A class with callbacks, also instantiated once
class my_player_callbacks : public ambulant::gui::d2::d2_player_callbacks {
  public:
	HWND new_os_window();
	void destroy_os_window(HWND hwnd);
	html_browser *new_html_browser(int left, int top, int width, int height);
	SIZE get_default_size();
};

my_player_callbacks s_player_callbacks;

HWND
my_player_callbacks::new_os_window() {
	// Return the handle of the single instance for now
	// This means paint bits of the new window
	// to the single instance
	return s_hwnd;
}

SIZE
my_player_callbacks::get_default_size() {
	SIZE size;
	size.cx = 640;
	size.cy = 480;
	return size;
}

void
my_player_callbacks::destroy_os_window(HWND hwnd) {
	// none for now; keep the single instance
}

html_browser *
my_player_callbacks::new_html_browser(int left, int top, int width, int height)
{
	return ::new_html_browser(left, top, width, height);
}

class my_player_feedback : public common::focus_feedback {
  public:
	void document_loaded(lib::document *doc) {}
	void node_focussed(const lib::node *n) {
		if (n == NULL) {
			set_status_line("");
			return;
		}
		const char *alt = n->get_attribute("alt");
		if (alt) {
			set_status_line(alt);
			return;
		}
		const char *href = n->get_attribute("href");
		if (href) {
			static std::string msg;
			msg = "Go to ";
			msg += href;
			set_status_line(msg.c_str());
			return;
		}
		set_status_line("");
	}

};

my_player_feedback s_player_feedback;

#ifdef WITH_CREATE_PLAYER_HOOK
void create_player_hook(void *player);
#endif

static ambulant::gui::d2::d2_player*
create_player_instance(const net::url& u, common::focus_feedback *feedback) {
	ambulant::gui::d2::d2_player *rv = new ambulant::gui::d2::d2_player(s_player_callbacks, feedback, u);
#ifdef WITH_CREATE_PLAYER_HOOK
	create_player_hook((void*)rv);
#endif
	return rv;
}

static ambulant::gui::d2::d2_player *player = 0;
static bool needs_done_redraw = false;

CWnd* topView = NULL;

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
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_COMMAND(ID_PLAY_ADVANCEFOCUS, OnFocusAdvance)
	ON_COMMAND(ID_PLAY_ACTIVATEFOCUS, OnFocusActivate)
	ON_COMMAND(ID_VIEW_SOURCE, OnViewSource)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SOURCE, OnUpdateViewSource)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOG, OnUpdateViewLog)
	ON_COMMAND(ID_VIEW_LOG, OnViewLog)
	ON_MESSAGE(WM_SET_CLIENT_RECT, OnSetClientRect)
	ON_COMMAND(ID_VIEW_TESTS, OnOpenFilter)
	ON_COMMAND(ID_VIEW_FILTER, OnViewFilter)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILTER, OnUpdateViewFilter)
	ON_WM_MOUSEMOVE()
	ON_UPDATE_COMMAND_UI(ID_VIEW_TESTS, OnUpdateOpenFilter)
	ON_COMMAND(ID_VIEW_AUTOPLAY, OnViewAutoplay)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AUTOPLAY, OnUpdateViewAutoplay)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_COMMAND(ID_HELP_WELCOME, OnHelpWelcome)
	ON_UPDATE_COMMAND_UI(ID_HELP_WELCOME, OnUpdateHelpWelcome)
	ON_MESSAGE(WM_REPLACE_DOC, OnReplaceDoc)
	ON_MESSAGE(WM_ERASEBKGND, OnMyEraseBkgnd)
	ON_COMMAND(ID_VIEW_NORMALSIZE, OnViewNormalSize)
END_MESSAGE_MAP()


// MmView construction/destruction

MmView::MmView()
:
#ifndef WITHOUT_LOG_WINDOW
	m_logwindow(NULL),
#endif // WITHOUT_LOG_WINDOW
	m_timer_id(0),
	m_cursor_id(0),
	m_autoplay(true)
{
#ifndef WITHOUT_DIALOGS
	lib::logger::get_logger()->set_show_message(log_show_message);
#endif
#ifdef WITHOUT_LOG_WINDOW
	lib::logger::get_logger()->set_std_ostream(log_os);
#else
	lib::logger::get_logger()->set_ostream(new logwindow_ostream());
#endif // WITHOUT_LOG_WINDOW
	int level = ambulant::common::preferences::get_preferences()->m_log_level;
	ambulant::lib::logger::get_logger()->set_level(level);
	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Windows/MFC with VS%s%s"), __DATE__,
#if _MSC_VER < 1500
		"2005 or earlier"
#elif _MSC_VER < 1600
		"2008"
#elif _MSC_VER < 1700
		"2010"
#else
		" newer than 2010"
#endif
		,
#ifdef WITH_GCD_EVENT_PROCESSOR
		" with GCD"
#else
		""
#endif
		);
#if ENABLE_NLS
	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif
#ifdef AMBULANT_USE_DLL
	lib::logger::get_logger()->debug(gettext("Ambulant Player: using AmbulantPlayer in DLL"));
#endif
	lib::logger::get_logger()->debug("Ambulant Player: using D2D Player");
	topView = this;
}

MmView::~MmView()
{
	topView = NULL;
	ambulant::gui::d2::d2_player::cleanup();
}

BOOL MmView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= WS_CLIPCHILDREN; // reduce flicker
	return CView::PreCreateWindow(cs);
}
// MmView drawing
void MmView::OnDraw(CDC* pDC)
{
	CPaintDC *pPDC = dynamic_cast<CPaintDC*>(pDC);
	RECT *pRect = NULL;
	if (pPDC) pRect = &pPDC->m_ps.rcPaint;
	MmDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	// TODO: add draw code for native data here
	if(player)
		player->redraw(m_hWnd, pDC->m_hDC, pRect);

}
// MmView resizing
void MmView::OnViewNormalSize()
{
	if(player)
		player->on_zoom(1.0, m_hWnd);

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

	m_timer_id = SetTimer(1, 500, 0);

	// Set static handle
	s_hwnd = GetSafeHwnd();
	ModifyStyle(0, WS_CLIPCHILDREN); // reduce flicker
#ifdef WITH_SPLASH_SCREEN
	LocateWelcomeDoc(TEXT(WITH_SPLASH_SCREEN));
#else
	if(LocateWelcomeDoc(TEXT("..\\..\\Extras\\Welcome\\Welcome.smil")) ||
		LocateWelcomeDoc(TEXT("Extras\\Welcome\\Welcome.smil")) ||
		LocateWelcomeDoc(TEXT("Welcome.smil"))){;}
#endif

#ifdef WITH_SPLASH_SCREEN
	PostMessage(WM_COMMAND, ID_HELP_WELCOME);
#else
	CWinApp* pApp = AfxGetApp();
	CString val = pApp->GetProfileString(_T("Settings"), _T("Welcome"));
	if(val.IsEmpty()) {
		// first time; write the string and play welcome
		pApp->WriteProfileString(_T("Settings"), _T("Welcome"),  _T("1"));
		if(!m_welcomeDocFilename.IsEmpty())
			PostMessage(WM_COMMAND, ID_HELP_WELCOME);
	}
#endif
	return 0;
}

void MmView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void MmView::OnDestroy()
{
	if(player) {
		player->stop();
		delete player;
		player = 0;
	}
	if(m_timer_id) KillTimer(m_timer_id);
	CView::OnDestroy();
}

void MmView::SetMMDocument(LPCTSTR lpszPathName, bool autostart) {
	USES_CONVERSION;
	ambulant::gui::d2::d2_player *dummy = player;
	player = 0;
	if(dummy) {
		dummy->stop();
		delete dummy;
	}
	// Heuristic check to see whether it's a URL:
	// - Contains a \, or
	// - second char is :, or
	// - contains neither : nor /
	bool is_local_filename = false;

	if (_tcschr(lpszPathName, _T('\\'))) is_local_filename = true;
	if (lpszPathName[0] && lpszPathName[1] == _T(':')) is_local_filename = true;
	if (_tcschr(lpszPathName, _T(':')) == NULL && _tcschr(lpszPathName, _T('/')) == NULL) is_local_filename = true;

	net::url u;
	TCHAR path[_MAX_PATH];
	if (is_local_filename) {
		TCHAR *pFilePart = 0;
		GetFullPathName(lpszPathName, MAX_PATH, path, &pFilePart);
		lib::textptr tppath(path);
		u = net::url::from_filename(tppath);
	} else {
		_tcscpy_s(path, sizeof(path)/sizeof(path[0]), lpszPathName);
		u = net::url::from_url(T2CA(path));
	}

	if (!u.is_absolute()) {
		lib::logger::get_logger()->error("Cannot play from non-absolute pathname: %s", lpszPathName);
		return;
	}
	dummy = create_player_instance(u, &s_player_feedback);
	m_curDocFilename = u.get_url().c_str();
	player = dummy;
	set_status_line("Ready");
#ifndef WITH_REMOTE_SYNC
	if(autostart || m_autoplay)
		PostMessage(WM_COMMAND, ID_FILE_PLAY);
#endif
}

void MmView::OnFilePlay()
{
	if(player) {
		player->play();
		needs_done_redraw = true;
		InvalidateRect(NULL);
	}
}

void MmView::OnUpdateFilePlay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((int)(player && player->is_play_enabled()));
	pCmdUI->SetCheck((int)(player && player->is_play_active()));
}

void MmView::OnFilePause()
{
	if(player) player->pause();
}

void MmView::OnUpdateFilePause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((int)(player && player->is_pause_enabled()));
	pCmdUI->SetCheck((int)(player && player->is_pause_active()));
}

void MmView::OnFileStop()
{
#ifdef WITH_SMIL30
	if (player)
		player->stop();
#else
	if(player) {
		net::url u = player->get_url();
		ambulant::gui::d2::d2_player *dummy = player;
		player = 0;
		if(dummy) {
			dummy->stop();
			delete dummy;
		}
		dummy = create_player_instance(u, &s_player_feedback);
		player = dummy;
		PostMessage(WM_INITMENUPOPUP,0, 0);
		InvalidateRect(NULL);
		needs_done_redraw = false;
	}
#endif
}

void MmView::OnUpdateFileStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((int)(player && player->is_stop_enabled()));
	pCmdUI->SetCheck((int)(player && player->is_stop_active()));
}

void MmView::OnTimer(UINT nIDEvent)
{
	// XXXX Jack: This seems a very funny way to get a final redraw...
	CView::OnTimer(nIDEvent);
	if(player && needs_done_redraw && player->is_stop_active()) {
		player->on_done();
		InvalidateRect(NULL);
		PostMessage(WM_INITMENUPOPUP,0, 0);
		needs_done_redraw = false;
	}
}


void MmView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(player) player->on_click(point.x, point.y, GetSafeHwnd());
	CView::OnLButtonDown(nFlags, point);
}

void MmView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(player) {
		int new_cursor_id = player->get_cursor(point.x, point.y, GetSafeHwnd());
		if(new_cursor_id>0) EnableToolTips(TRUE);
		else CancelToolTips();
		if(new_cursor_id != m_cursor_id) {
			HCURSOR new_cursor = 0;
			if(new_cursor_id == 0) {
				new_cursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
			} else {
				new_cursor = AfxGetApp()->LoadCursor(IDC_CURSOR_HAND);
			}
			SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, HandleToLong(new_cursor));
			m_cursor_id = new_cursor_id;
		}
	}
	CView::OnMouseMove(nFlags, point);
}

void MmView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if(player) player->on_char(nChar);
	CView::OnChar(nChar, nRepCnt, nFlags);
}

void MmView::OnFocusAdvance()
{
	if (player) player->on_focus_advance();
}

void MmView::OnFocusActivate()
{
	if (player) player->on_focus_activate();
}

void MmView::OnViewSource() {
	USES_CONVERSION;
	CString cmd = TEXT("Notepad.exe ");
	std::string ustr = T2CA(LPCTSTR(m_curDocFilename));
	net::url u = net::url::from_url(ustr);
	if (!u.is_local_file()) {
		lib::logger::get_logger()->error("View Source: only for local files...");
		return;
	}
	assert(u.is_local_file());
	// XXXX Also check OnUpdateViewSource
	cmd += u.get_file().c_str(); // XXXX Incorrect
	WinExec(T2CA(cmd), SW_SHOW);
}

void MmView::OnUpdateViewSource(CCmdUI *pCmdUI) {
	USES_CONVERSION;
	bool b = player && !m_curDocFilename.IsEmpty() && net::url::from_url(T2CA(LPCTSTR(m_curDocFilename))).is_local_file();
	pCmdUI->Enable(b?TRUE:FALSE);
}

void MmView::OnViewLog() {
#ifdef WITHOUT_LOG_WINDOW
	// Logging to file: open the file in notepad.
	TCHAR buf[_MAX_PATH];
	GetModuleFileName(NULL, buf, _MAX_PATH);
	TCHAR *p1 = text_strrchr(buf,TCHAR('\\'));
	if(p1 != NULL) *p1= TCHAR('\0');
	text_strcat(buf, TEXT("\\"));
	text_strcat(buf, log_name);
	CString cmd = TEXT("Notepad.exe ");
	cmd += buf;
	WinExec((LPCSTR)(LPCTSTR)cmd, SW_SHOW);
#else
	// Logging to a window: show the window.
	if (m_logwindow == NULL) {
		m_logwindow = CLogWindow::GetLogWindowSingleton();
	}
	m_logwindow->ShowWindow(SW_SHOW);
#endif
}

void MmView::OnUpdateViewLog(CCmdUI *pCmdUI) {
	//pCmdUI->Enable((player && !m_curDocFilename.IsEmpty())?TRUE:FALSE);
	pCmdUI->Enable(TRUE);
}

LPARAM MmView::OnSetClientRect(WPARAM wParam, LPARAM lParam) {
	CFrameWnd *mainWnd = (CFrameWnd*) AfxGetMainWnd();

	POINT pt = {0, 0}; // margins

	CRect rc1;
	mainWnd->GetWindowRect(&rc1);

	CRect rc2;
	GetWindowRect(&rc2);
	int dx = rc1.Width() - rc2.Width();
	int dy = rc1.Height() - rc2.Height();

	CSize size(int(wParam) + (2*pt.x + 4) + dx, int(lParam) + (2*pt.y+4) + dy);

	UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE;
	mainWnd->SetWindowPos(&wndTop, 0, 0, size.cx, size.cy, flags);
	return 0;
}

LPARAM MmView::OnReplaceDoc(WPARAM wParam, LPARAM lParam) {
	USES_CONVERSION;
	if(lParam == 0) return 0;
	std::string *purlstr = (std::string *)lParam;
	SetMMDocument(A2CT(purlstr->c_str()), wParam?true:false);
	delete purlstr;
	return 0;
}

void MmView::OnOpenFilter() {
	USES_CONVERSION;
	BOOL bOpenFileDialog = TRUE;
	TCHAR lpszDefExt[] = TEXT("*.xml");
	LPCTSTR lpszFileName = NULL; // no initial fn
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	TCHAR lpszFilter[] = TEXT("Settings Files (*.xml)|*.xml|All Files (*.*)|*.*||");
	CWnd* pParentWnd = this;
	CFileDialog dlg(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd);
	dlg.m_ofn.lpstrTitle = TEXT("Select settings file");
	if(!m_curDocFilename.IsEmpty()) {
		// XXXX Is this correct? (URL vs. filename)
		net::url u = net::url::from_url( T2CA((LPCTSTR) m_curDocFilename));
		if(u.is_local_file())
			dlg.m_ofn.lpstrInitialDir = get_directory(A2CT(u.get_file().c_str()));
	}
	if(dlg.DoModal()==IDOK) {
		CString str = dlg.GetPathName();
		m_curFilter = str;
		smil2::test_attrs::load_test_attrs(lib::textptr(LPCTSTR(str)).c_str());
	}
}

void MmView::OnUpdateOpenFilter(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void MmView::OnViewFilter()
{
	USES_CONVERSION;
	if(!m_curFilter.IsEmpty()) {
		CString cmd = TEXT("Notepad.exe ");
		cmd += m_curFilter;
		WinExec(T2CA((LPCTSTR)cmd), SW_SHOW);
	}
}

void MmView::OnUpdateViewFilter(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_curFilter.IsEmpty());
}

void MmView::OnViewAutoplay()
{
	m_autoplay = !m_autoplay;
}

void MmView::OnUpdateViewAutoplay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_autoplay?1:0);
}

BOOL MmView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	if(!player) return false;

	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	UINT_PTR nID = pNMHDR->idFrom;
	CString strTipText;
	if(pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)) {
		std::string str = player->get_pointed_node_str();
		strTipText.Format(_T("Anchor: %s"), str.c_str());
		if(pNMHDR->code == TTN_NEEDTEXTA)
			strncpy_s(pTTTA->szText, sizeof(pTTTA->szText), str.c_str(), sizeof(pTTTA->szText));
		else
			::MultiByteToWideChar( CP_ACP , 0, str.c_str(), -1, pTTTW->szText, sizeof(pTTTW->szText) );
		*pResult = 0;
		return TRUE;
	}
	return FALSE;
}

INT_PTR MmView::OnToolHitTest(CPoint point, TOOLINFO* pTI) const {
	if(player) {
		int new_cursor_id = player->get_cursor(point.x, point.y, GetSafeHwnd());
		if(new_cursor_id > 0) {
			INT_PTR nHit = 1; // node id
			if (pTI != NULL) {
				pTI->hwnd = m_hWnd;
				pTI->uId = (UINT_PTR)m_hWnd; // hWndChild;
				pTI->uFlags |= TTF_IDISHWND;
				pTI->lpszText = LPSTR_TEXTCALLBACK;
				pTI->uFlags |= TTF_NOTBUTTON; //|TTF_CENTERTIP;
			}
			return nHit;
		}
	}
	return -1;  // not found
}

bool MmView::LocateWelcomeDoc(LPCTSTR rpath) {
	TCHAR buf[_MAX_PATH];
	GetModuleFileName(NULL, buf, _MAX_PATH);
	TCHAR *p1 = text_strrchr(buf,'\\');
	if(p1) *++p1='\0';
	_tcscat_s(buf, sizeof(buf)/sizeof(buf[0]), rpath);
	TCHAR path[_MAX_PATH];
	TCHAR *pFilePart = 0;
	GetFullPathName(buf, MAX_PATH, path, &pFilePart);
	WIN32_FIND_DATA fd;
	memset(&fd, 0, sizeof(WIN32_FIND_DATA));
	HANDLE hFind = FindFirstFile(path, &fd);
	if(hFind != INVALID_HANDLE_VALUE){
		FindClose(hFind);
		m_welcomeDocFilename = path;
		return true;
	}
	return false;
}

void MmView::OnHelpWelcome()
{
	if(!m_welcomeDocFilename.IsEmpty())
		SetMMDocument(m_welcomeDocFilename, true);
}

void MmView::OnUpdateHelpWelcome(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_welcomeDocFilename.IsEmpty());
}

// We don't want MFC to clear the background for Direct2D rendering
afx_msg LRESULT 
MmView::OnMyEraseBkgnd(WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}
