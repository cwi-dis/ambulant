// OpenUrlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "OpenUrlDlg.h"


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
END_MESSAGE_MAP()


// COpenUrlDlg message handlers
