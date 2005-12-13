// PreferencesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "PreferencesDlg.h"
#include ".\preferencesdlg.h"
#include "ambulant/common/preferences.h"

#pragma warning( disable: 4800)  // Disable performance warning "forcing value to bool true of false"

// PreferencesDlg dialog

IMPLEMENT_DYNAMIC(PreferencesDlg, CDialog)
PreferencesDlg::PreferencesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(PreferencesDlg::IDD, pParent)
{
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	m_log_level = prefs->m_log_level;
	m_parser_id = prefs->m_parser_id.c_str();
	m_validation_scheme = prefs->m_validation_scheme.c_str();
	m_do_namespaces = prefs->m_do_namespaces;
	m_do_schema = prefs->m_do_schema;
	m_validation_schema_full_checking = prefs->m_validation_schema_full_checking;
}

PreferencesDlg::~PreferencesDlg()
{
}

void PreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_LOG_LEVEL, m_log_level);
	DDX_Text(pDX, IDC_PARSER_ID, m_parser_id);
	DDX_Text(pDX, IDC_VALIDATION_SCHEME, m_validation_scheme);
	DDX_Check(pDX, IDC_DO_NAMESPACES, m_do_namespaces);
	DDX_Radio(pDX, IDC_DO_SCHEMA, m_do_schema);
	DDX_Check(pDX, IDC_VALIDATION_SCHEMA_FULL_CHECKING, m_validation_schema_full_checking);
}


BEGIN_MESSAGE_MAP(PreferencesDlg, CDialog)
//	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, OnBnClickedOK)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
END_MESSAGE_MAP()


// PreferencesDlg message handlers

void PreferencesDlg::OnBnClickedOK()
{
	USES_CONVERSION;
	OnOK();
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_log_level = m_log_level;
	prefs->m_parser_id = T2CA((LPCTSTR)m_parser_id);
	prefs->m_validation_scheme = T2CA((LPCTSTR)m_validation_scheme);
	prefs->m_do_namespaces = (bool)m_do_namespaces;
	prefs->m_do_schema = (bool)m_do_schema;
	prefs->m_validation_schema_full_checking = (bool)m_validation_schema_full_checking;
	prefs->save_preferences();
}

void PreferencesDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void PreferencesDlg::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
}

void PreferencesDlg::OnBnClickedCheck2()
{
	// TODO: Add your control notification handler code here
}
