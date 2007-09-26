// AmbulantPlayerView.cpp : implementation of the CAmbulantPlayerView class
//

#include "stdafx.h"
#include "AmbulantPlayer.h"

#include "AmbulantPlayerDoc.h"
#include "AmbulantPlayerView.h"
#include "SelectDlg.h"
#include "PreferencesDlg.h"

// DX player
#include "ambulant/gui/dx/dx_player.h"
#include "ambulant/gui/dx/dx_wmuser.h"

#include "ambulant/common/preferences.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/win32/win32_fstream.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/net/url.h"
#include "ambulant/version.h"

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

text_char *log_name = TEXT("\\Program Files\\Ambulant\\amlog.txt");
#ifdef DEBUG
class logger_trace : public ambulant::lib::ostream {
	bool is_open() const {return true;}
	void close() {}
	int write(const unsigned char *buffer, int nbytes) {return write("ostream use of buffer, size not implemented for trace");}
	int write(const char *cstr) {
		ATLTRACE(atlTraceGeneral, 0, _T("%S"), cstr);
		return 0;
	}
	void write(ambulant::lib::byte_buffer& bb) {write("ostream use of byte_buffer not implemented for trace");}
	void flush() {}
};
#endif

typedef gui::dx::dx_player dg_or_dx_player;
typedef gui::dx::dx_player_callbacks gui_callbacks;

// The handle of the single window instance
static HWND s_hwnd;

// A class with callbacks, also instantiated once
class my_player_callbacks : public gui_callbacks {
  public:
	HWND new_os_window();
	void destroy_os_window(HWND hwnd);
	HWND get_main_window();
	html_browser *new_html_browser(int left, int top, int width, int height);
	SIZE get_default_size();
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

html_browser *
my_player_callbacks::new_html_browser(int left, int top, int width, int height)
{
	return NULL;
}

SIZE
my_player_callbacks::get_default_size() {
	SIZE size;
	// XXXJACK these are stupid values. Need to get from environment.
	size.cx = 640;
	size.cy = 480;
	return size;
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
	ON_COMMAND(ID_FILE_PREFERENCES, OnPreferences)
	ON_COMMAND(ID_FILE_LOADSETTINGS, OnFileLoadSettings)
	ON_COMMAND(ID_VIEW_SOURCE, OnViewSource)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SOURCE, OnUpdateViewSource)
	ON_COMMAND(ID_VIEW_LOG, OnViewLog)
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
	

#ifdef DEBUG
	// Log to the debugger output window in VS2005
	lib::logger::get_logger()->set_ostream(new logger_trace());
#else
	// Log to a file
	lib::win32::fstream *fs = new lib::win32::fstream();
	if(fs->open_for_writing(log_name))
		lib::logger::get_logger()->set_ostream(fs);
#endif
	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for WindowsCE/MFC"), __DATE__);
#if ENABLE_NLS
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
#if 0
	common::preferences *prefs = common::preferences::get_preferences();
	prefs->m_prefer_ffmpeg = true;
#endif
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
#ifdef WITH_SMIL30
	if (player)
		player->stop();
#else
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
#endif // WITH_SMIL30
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
#if 0
	ambulant::net::url wurl = ambulant::net::url::from_filename("Welcome/Welcome.smil");
	std::pair<bool, net::url> absolute_url = wurl.get_local_datafile();
	if (!absolute_url.first) {
		lib::logger::get_logger()->error("Welcome document not found");
		return;
	}
	std::string urlstr = absolute_url.second.get_url();
	SetMMDocument(lib::textptr(urlstr->c_str()).c_wstr());
#else
	CString welcomeFilename("\\Program Files\\Ambulant\\Welcome\\Welcome.smil");
	SetMMDocument(welcomeFilename);
#endif
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

void CAmbulantPlayerView::OnPreferences()
{
	PrefPropertySheet dlg;
	if(dlg.DoModal() != IDOK) return;
}

void CAmbulantPlayerView::OnViewSource() {
	USES_CONVERSION;
	CString cmd = TEXT("-opendoc \"");
	std::string ustr = T2CA(LPCTSTR(m_curDocFilename));
	net::url u = net::url::from_url(ustr);
	if (!u.is_local_file()) {
		lib::logger::get_logger()->error("View Source: only for local files...");
		return;
	}
	assert(u.is_local_file());
	// XXXX Also check OnUpdateViewSource
	cmd += u.get_file().c_str(); // XXXX Incorrect
	cmd += "\"";
	CreateProcess(_T("pword.exe"), cmd, NULL, NULL, false, 0, NULL, NULL, NULL, NULL);	
}

void CAmbulantPlayerView::OnUpdateViewSource(CCmdUI *pCmdUI) {
	USES_CONVERSION;
	bool b = player && !m_curDocFilename.IsEmpty() && net::url::from_url(T2CA(LPCTSTR(m_curDocFilename))).is_local_file();
	pCmdUI->Enable(b?TRUE:FALSE);
}

void CAmbulantPlayerView::OnViewLog() {
	// Logging to file: open the file in notepad.
#if 0
	TCHAR buf[_MAX_PATH];
	GetModuleFileName(NULL, buf, _MAX_PATH);
	TCHAR *p1 = text_strrchr(buf,TCHAR('\\'));
	if(p1 != NULL) *p1= TCHAR('\0');
	text_strcat(buf, TEXT("\\"));
	text_strcat(buf, log_name);
#endif
	CString cmd = TEXT("-opendoc \"");
	cmd += log_name;
	cmd += "\"";
	CreateProcess(_T("pword.exe"), cmd, NULL, NULL, false, 0, NULL, NULL, NULL, NULL);	
}
