// PreferencesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "LogWindow.h"
#include "ambulant/gui/d2/wmuser.h"

// logwindow_ostream implementation

int
logwindow_ostream::write(const char *cstr)
{
#ifdef _DEBUG
	TRACE("%s", cstr);
#endif
	CLogWindow *lwin = CLogWindow::GetLogWindowSingleton();
	lwin->AppendText(cstr);
	return 1;
}

// CLogWindow dialog

IMPLEMENT_DYNAMIC(CLogWindow, CDialog)
CLogWindow::CLogWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CLogWindow::IDD, pParent)
{
}

CLogWindow *CLogWindow::s_singleton = NULL;

CLogWindow *
CLogWindow::GetLogWindowSingleton()
{
	if (s_singleton == 0) {
		s_singleton = new CLogWindow(NULL);
		s_singleton->Create(IDD);
	}
	return s_singleton;
}

CLogWindow::~CLogWindow()
{
}

void CLogWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_richedit);
}


BEGIN_MESSAGE_MAP(CLogWindow, CDialog)
//	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
//	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
//	ON_BN_CLICKED(IDOK, OnBnClickedOK)
ON_EN_CHANGE(IDC_RICHEDIT21, OnEnChangeRichedit21)
ON_MESSAGE(WM_AMBULANT_MESSAGE, OnAddLoggerLine)
ON_WM_SIZE()
END_MESSAGE_MAP()


// CLogWindow message handlers

void
CLogWindow::AppendText(const char *data)
{
	char *myData = _strdup(data);
	if (m_hWnd == NULL) return;
	PostMessage(WM_AMBULANT_MESSAGE, 0, (LPARAM)myData);
}

LPARAM
CLogWindow::OnAddLoggerLine(WPARAM wp, LPARAM lp)
{
	USES_CONVERSION;
	char *myData = (char *)lp;
	m_richedit.SetSel(-1, -1);
	m_richedit.ReplaceSel(A2CT(myData));
	free(myData);
	return 0;
}
void CLogWindow::OnEnChangeRichedit21()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CLogWindow::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// Move the RichEdit control size to fit the window resized.
	CRect rect;
	GetClientRect(&rect);
	if(IsWindow(m_richedit.GetSafeHwnd())){
		m_richedit.MoveWindow(rect);
	}

}
