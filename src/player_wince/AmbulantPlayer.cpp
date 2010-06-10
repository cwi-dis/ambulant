// AmbulantPlayer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "mypreferences.h"

#include "MainFrm.h"

#include "AmbulantPlayerDoc.h"
#include "AmbulantPlayerView.h"

#include "ambulant/version.h"
#include "ambulant/lib/textptr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerApp

BEGIN_MESSAGE_MAP(CAmbulantPlayerApp, CWinApp)
	//{{AFX_MSG_MAP(CAmbulantPlayerApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerApp construction

CAmbulantPlayerApp::CAmbulantPlayerApp()
	: CWinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CAmbulantPlayerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerApp initialization

BOOL CAmbulantPlayerApp::InitInstance()
{
    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the Windows Mobile specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();

#if 0
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
#endif

	// XXXJACK Desktop version inits a few more things. CE needs that too?

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Ambulant"));
//	LoadStdProfileSettings(4);
	mypreferences::install_singleton();
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CAmbulantPlayerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CAmbulantPlayerView)
		);
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
#if 1
	// This is a workaround for a bug in Windows Mobile MFC: it does not
	// communicate the "open" command correctly if a file is passed on the
	// command line. See <http://www.tech-archive.net/Archive/WindowsCE/microsoft.public.windowsce.embedded.vc/2006-10/msg00191.html>
	if( !cmdInfo.m_strFileName.IsEmpty() )
		cmdInfo.m_nShellCommand=CCommandLineInfo::FileOpen;
#endif

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

static TCHAR executable[MAX_PATH];
static HWND other_ambulant_window;
static BOOL CALLBACK
myFindWindowProc(HWND hwnd, LPARAM lParam)
{
	DWORD      dwProcessID;
	INT        iLen;
	TCHAR      szTempName[MAX_PATH]=TEXT("\0");

	GetWindowThreadProcessId(hwnd,&dwProcessID);
	if (!dwProcessID)
		return TRUE;

	iLen=GetModuleFileName((HMODULE)dwProcessID,szTempName,MAX_PATH);
	if (!iLen)
		return TRUE;

	if (_tcsicmp(szTempName, executable) == 0)
	{
		// This window belongs to the Ambulant executable. Last thing to
		// check is that it is actually the toplevel window.
		GetWindowText(hwnd, szTempName, MAX_PATH);
		if (_tcsicmp(szTempName, _T("AmbulantPlayer")) == 0) {
			other_ambulant_window = hwnd;
			return FALSE;
		}
	}

	return TRUE;

}

BOOL CAmbulantPlayerApp::InitApplication()
{
	// Step 1 - create a named mutex and attempt to lock it. If this wqorks then
	// we're the first copy of Ambulant and we don't have to worry about enumerating the
	// windows.
#if 0
	HANDLE hMutex;
	hMutex = CreateMutex(NULL, FALSE, _T("Global\\AmbulantMutex"));
	if (hMutex == NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
#else
	{
#endif
		// Another copy of Ambulant is already running. Try to locate it.
		GetModuleFileName(NULL, executable, MAX_PATH);
		EnumWindows(myFindWindowProc, NULL);
		if (other_ambulant_window) {
			// Found the window for the other instance of Ambulant. Send it
			// a copy of our command line.
			CCommandLineInfo cmdInfo;
			ParseCommandLine(cmdInfo);
			SetForegroundWindow(other_ambulant_window);
			COPYDATASTRUCT cds;
			cds.dwData = 0;
			cds.cbData = (cmdInfo.m_strFileName.GetLength()+1) * sizeof(TCHAR);
			cds.lpData = (PVOID)((LPCTSTR)cmdInfo.m_strFileName);
			SendMessage(other_ambulant_window, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
			// We're all done, and we don't want to start up. Tell our caller this.
			return FALSE;
		}
		// XXX Funny: the mutex was held, but there is no window...
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	CString m_version;

// Implementation
protected:
#ifdef _DEVICE_RESOLUTION_AWARE
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_AM_VERSION, m_version);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;	// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
#ifdef xxxjack_DEVICE_RESOLUTION_AWARE
	ON_WM_SIZE()
#endif
END_MESSAGE_MAP()

#ifdef xxxjack_DEVICE_RESOLUTION_AWARE
void CAboutDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
    	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(),
			this->m_hWnd,
			DRA::GetDisplayMode() != DRA::Portrait ? MAKEINTRESOURCE(IDD_ABOUTBOX_WIDE) : MAKEINTRESOURCE(IDD_ABOUTBOX));
	}
}
#endif

// App command to run the dialog
void CAmbulantPlayerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	ambulant::lib::textptr tp(ambulant::get_version());
	aboutDlg.m_version = TEXT("Ambulant version ");
	aboutDlg.m_version += tp.c_wstr();
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CAmbulantPlayerApp commands
// Added for WCE apps

