/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

// MmView.cpp : implementation of the MmView class
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "MainFrm.h"

#include "MmDoc.h"
#include "MmView.h"
#include "LogWindow.h"

#include <fstream>
#include <string>

// DX Player
#include "ambulant/gui/dx/dx_player.h"
#include "ambulant/gui/dx/dx_wmuser.h"

// DG Player
#include "ambulant/gui/dg/dg_player.h"
#include "ambulant/gui/dg/dg_wmuser.h"

#include "ambulant/common/preferences.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/net/url.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/version.h"

#include ".\mmview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef WITHOUT_LOG_WINDOW
const TCHAR log_name[] = "amlog.txt";

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
	text_strcat(buf, fn);
	TCHAR *p1 = text_strrchr(buf,'\\');
	if(p1 != NULL) *p1='\0';
	return buf;
}

#ifdef WITHOUT_LOG_WINDOW
std::ofstream 
log_os(get_log_filename().c_str());
#endif

using namespace ambulant;
//#define AM_PLAYER_DG

#ifdef AM_PLAYER_DG
typedef gui::dg::dg_player gui_player;
typedef gui::dg::dg_player_callbacks gui_callbacks;
#pragma comment (lib,"mp3lib.lib")
#pragma comment (lib,"libpng13.lib")
#else 
typedef gui::dx::dx_player gui_player;
typedef gui::dx::dx_player_callbacks gui_callbacks;
#endif

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


static gui_player* 
create_player_instance(const net::url& u) {
	return new gui_player(s_player_callbacks, u);
}

static gui_player *player = 0;
static needs_done_redraw = false;

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
END_MESSAGE_MAP()


// MmView construction/destruction

MmView::MmView()
:	m_logwindow(NULL)
{
	// TODO: add construction code here
	m_timer_id = 0;
	m_cursor_id = 0;
	m_autoplay = true;
	lib::logger::get_logger()->set_show_message(lib::win32::show_message);
#ifdef WITHOUT_LOG_WINDOW
	lib::logger::get_logger()->set_std_ostream(log_os);
#else
	lib::logger::get_logger()->set_ostream(new logwindow_ostream());
#endif // WITHOUT_LOG_WINDOW
	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Windows/MFC"), __DATE__);
#if USE_NLS
	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif
#ifdef AMBULANT_USE_DLL
	lib::logger::get_logger()->debug(gettext("Ambulant Player: using AmbulantPlayer in DLL"));
#endif
#ifdef AM_PLAYER_DG
	lib::logger::get_logger()->debug("Ambulant Player: using DG Player");
#else
	lib::logger::get_logger()->debug("Ambulant Player: using DX Player");
#endif
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

void MmView::OnDraw(CDC* pDC)
{
	MmDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	if(player)
		player->redraw(m_hWnd, pDC->m_hDC);
	
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
	if(LocateWelcomeDoc(TEXT("..\\..\\Extras\\Welcome\\Welcome.smil")) ||
		LocateWelcomeDoc(TEXT("Extras\\Welcome\\Welcome.smil")) ||
		LocateWelcomeDoc(TEXT("Welcome.smil"))){;}
	

	PostMessage(WM_SET_CLIENT_RECT, 
		common::default_layout_width, ambulant::common::default_layout_height);

	CWinApp* pApp = AfxGetApp();
	CString val = pApp->GetProfileString("Settings", "Welcome");
	if(val.IsEmpty()) {
		// first time; write the string and play welcome
		pApp->WriteProfileString("Settings", "Welcome",  "1");
		if(!m_welcomeDocFilename.IsEmpty())
			PostMessage(WM_COMMAND, ID_HELP_WELCOME);
	}

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
	gui_player *dummy = player;
	player = 0;
	if(dummy) {
		dummy->stop();
		delete dummy;
	}
	net::url u(lpszPathName);
	dummy = create_player_instance(u);
	m_curDocFilename = u.get_url().c_str();
	player = dummy;
	if(autostart || m_autoplay)
		PostMessage(WM_COMMAND, ID_FILE_PLAY);
}

void MmView::OnFilePlay()
{
	if(player) {
		player->start();
		needs_done_redraw = true;
		InvalidateRect(NULL);
	}
}

void MmView::OnUpdateFilePlay(CCmdUI *pCmdUI)
{	
	bool enable = player && !player->is_playing();
	pCmdUI->Enable(enable?TRUE:FALSE);
}

void MmView::OnFilePause()
{
	if(player) player->pause();
}

void MmView::OnUpdateFilePause(CCmdUI *pCmdUI)
{
	bool enable = player && (player->is_playing() || player->is_pausing());
	pCmdUI->Enable(enable?TRUE:FALSE);
	if(enable) pCmdUI->SetCheck(player->is_pausing()?1:0);
}

void MmView::OnFileStop()
{

	if(player) {
		net::url u = player->get_url();
		gui_player *dummy = player;
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

void MmView::OnUpdateFileStop(CCmdUI *pCmdUI)
{
	bool enable = player && (player->is_playing() || player->is_pausing());
	pCmdUI->Enable(enable?TRUE:FALSE);
}

void MmView::OnTimer(UINT nIDEvent)
{
	CView::OnTimer(nIDEvent);
	if(player && needs_done_redraw && player->is_done()) {
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

void MmView::OnViewSource() {
	CString cmd = TEXT("Notepad.exe ");
	std::string ustr = LPCTSTR(m_curDocFilename);
	net::url u(ustr);
	cmd += u.get_file().c_str();
	WinExec(cmd, SW_SHOW);	
}

void MmView::OnUpdateViewSource(CCmdUI *pCmdUI) {
	bool b = player && !m_curDocFilename.IsEmpty() && net::url(LPCTSTR(m_curDocFilename)).is_local_file();
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
	WinExec(cmd, SW_SHOW);
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
	if(lParam == 0) return 0;
	std::string *purlstr = (std::string *)lParam;
	SetMMDocument(purlstr->c_str(), wParam?true:false);
	delete purlstr;
	return 0;
}

void MmView::OnOpenFilter() {
	BOOL bOpenFileDialog = TRUE;
	TCHAR lpszDefExt[] = TEXT("*.xml");
	LPCTSTR lpszFileName = NULL; // no initial fn
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	TCHAR lpszFilter[] = TEXT("Settings Files (*.xml)|*.xml|All Files (*.*)|*.*||");
	CWnd* pParentWnd = this;
	CFileDialog dlg(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd);
	dlg.m_ofn.lpstrTitle = TEXT("Select settings file");
	if(!m_curDocFilename.IsEmpty()) {
		net::url u( (LPCTSTR) m_curDocFilename);
		if(u.is_local_file())
			dlg.m_ofn.lpstrInitialDir = get_directory(u.get_file().c_str());
	}
	if(dlg.DoModal()==IDOK) {
		CString str = dlg.GetPathName();
		m_curFilter = str;
		smil2::test_attrs::load_test_attrs(lib::textptr(LPCTSTR(str)).c_str());
		if(player) player->restart();
	}	
}

void MmView::OnUpdateOpenFilter(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void MmView::OnViewFilter()
{
	if(!m_curFilter.IsEmpty()) {
		CString cmd = TEXT("Notepad.exe ");
		cmd += m_curFilter;
		WinExec((LPCSTR)cmd, SW_SHOW);
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
		strTipText.Format("Anchor: %s", str.c_str());
		if(pNMHDR->code == TTN_NEEDTEXTA)
			lstrcpyn(pTTTA->szText, str.c_str(), sizeof(pTTTA->szText));
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
	text_strcat(buf, rpath);
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
