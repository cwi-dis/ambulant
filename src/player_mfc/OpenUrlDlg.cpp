// OpenUrlDlg.cpp : implementation file
//

#include "stdafx.h"
#include <wininet.h>

#include "AmbulantPlayer.h"
#include "OpenUrlDlg.h"
#include ".\openurldlg.h"

#include "ambulant/net/url.h"
#include "ambulant/lib/win32/win32_asb.h"

using namespace ambulant;

// COpenUrlDlg dialog

IMPLEMENT_DYNAMIC(COpenUrlDlg, CDialog)
COpenUrlDlg::COpenUrlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpenUrlDlg::IDD, pParent)
{
}

COpenUrlDlg::~COpenUrlDlg()
{
}

void COpenUrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_URL, m_url);
}


BEGIN_MESSAGE_MAP(COpenUrlDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBnClickedButtonBrowse)
END_MESSAGE_MAP()


// COpenUrlDlg message handlers

void COpenUrlDlg::OnOK()
{
	UpdateData(TRUE);
	
	if(m_url.IsEmpty()) {
		AfxMessageBox("Please enter a URL or select a local file");
		return;
	}
	
	std::string urlstr = (const char *) m_url;
	net::url u(urlstr);
	if(u.is_local_file() && !lib::win32::file_exists(u.get_file())) {
		CString str;
		str.Format("The file specified does not exist");
		AfxMessageBox(str);
		return;
	}
	
	CDialog::OnOK();
}

void COpenUrlDlg::OnBnClickedButtonBrowse()
{
	BOOL bOpenFileDialog = TRUE;
	char lpszDefExt[] = "*.smil";
	LPCTSTR lpszFileName = NULL; // no initial fn
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	char lpszFilter[] = "SMIL Documents (*.smil)|*.smil;*.smi;*.grins|All Files (*.*)|*.*||";
	CWnd* pParentWnd = this;
	CFileDialog dlg(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd);
	dlg.m_ofn.lpstrTitle = "Open SMIL document";
	if(dlg.DoModal()==IDOK) {
		CString s = dlg.GetPathName();
		s.Replace("\\", "/");
		m_url = "file:///"; m_url += s;
		GetDlgItem(IDC_EDIT_URL)->SetWindowText(m_url);
	}	
}

