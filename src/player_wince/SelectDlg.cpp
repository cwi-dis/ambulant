// SelectDlg.cpp : implementation file
//
#include "stdafx.h"
#include "resource.h"
#include "SelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectDlg dialog


CSelectDlg::CSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dir = TEXT("\\Windows\\Ambulant");
	m_filename = TEXT("");
}


void CSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectDlg)
	ON_LBN_DBLCLK(IDC_FILE_LIST, OnDblclkList1)
	ON_LBN_SELCHANGE(IDC_FILE_LIST, OnSelchangeList1)
	ON_CBN_CLOSEUP(IDC_DIR_COMBO, OnCloseupDirCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectDlg message handlers

void CSelectDlg::OnDblclkList1() 
{
	// TODO: Add your control notification handler code here
	
}

void CSelectDlg::OnSelchangeList1() 
{
	CListBox *pList = (CListBox*) GetDlgItem(IDC_FILE_LIST);
	int nIndex = pList->GetCurSel();
	if(nIndex>=0) {
		LPTSTR  sb = m_filename.GetBuffer(MAX_PATH);
		pList->GetText(nIndex, sb);
		m_filename.ReleaseBuffer(); 
	}
}

BOOL CSelectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CComboBox *pCombo = (CComboBox*) GetDlgItem(IDC_DIR_COMBO);	
	pCombo->AddString(TEXT("Ambulant documents"));
	pCombo->SetCurSel(0);

	FillList(TEXT("*.*"));
	CListBox *pList = (CListBox*) GetDlgItem(IDC_FILE_LIST);
	if(pList->GetCount()>0)
		pList->SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


inline bool endsWith(const CString& s, LPCTSTR p) {
	if(!p) return true;
	LPCTSTR rend = p - 1;
	p += _tcslen(p) - 1;
	for(int i = s.GetLength()-1;i>=0 && p != rend;i--,p--) {
		if(*p != s[i]) break;
	}
	return p == rend;
}

void CSelectDlg::FillList(LPCTSTR ext) {
    WIN32_FIND_DATA findData;
	memset(&findData, 0, sizeof(findData));
    HANDLE fileHandle = FindFirstFile(m_dir + TEXT("\\") +  ext, &findData);
	if(fileHandle == INVALID_HANDLE_VALUE) return;
	CListBox *pList = (CListBox*) GetDlgItem(IDC_FILE_LIST);
	while(FindNextFile(fileHandle, &findData)) {
		if((findData.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY) == 0) {
			CString fn = findData.cFileName;
			if(endsWith(fn, TEXT(".smil")) || 
				endsWith(fn, TEXT(".smi")) || 
				endsWith(fn, TEXT(".grins")))
			pList->AddString(fn);
		}
	}
	FindClose(fileHandle);
}


CString CSelectDlg::GetPathName() {
	if(!m_filename.IsEmpty())
		return m_dir + TEXT("\\") + m_filename;
	return TEXT("");
}

void CSelectDlg::OnCloseupDirCombo() 
{
	CListBox *pList = (CListBox*) GetDlgItem(IDC_FILE_LIST);
	pList->ResetContent();
	FillList(TEXT("*.*"));
	if(pList->GetCount()>0) pList->SetCurSel(0);
	
}
