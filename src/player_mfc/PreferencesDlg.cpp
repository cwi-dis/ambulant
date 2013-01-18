// PreferencesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "PreferencesDlg.h"
#include ".\preferencesdlg.h"
#include "ambulant/common/preferences.h"
#include "ambulant/lib/logger.h"

#pragma warning( disable: 4800)  // Disable performance warning "forcing value to bool true of false"

// Logging property page

BEGIN_MESSAGE_MAP(PrefLoggingPropertyPage, CPropertyPage)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(PrefLoggingPropertyPage, CPropertyPage)
PrefLoggingPropertyPage::PrefLoggingPropertyPage()
	: CPropertyPage(PrefLoggingPropertyPage::IDD)
{
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	m_log_level = prefs->m_log_level;
}

BOOL
PrefLoggingPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	return TRUE;
}

PrefLoggingPropertyPage::~PrefLoggingPropertyPage()
{
}

void PrefLoggingPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_LOG_LEVEL, m_log_level);
}

// PreferencesDlg message handlers

void PrefLoggingPropertyPage::OnOK()
{
	USES_CONVERSION;
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_log_level = m_log_level;
	prefs->save_preferences();
	ambulant::lib::logger::get_logger()->set_level(m_log_level);
	CPropertyPage::OnOK();
}

// Media property page
BEGIN_MESSAGE_MAP(PrefMediaPropertyPage, CPropertyPage)
END_MESSAGE_MAP()


IMPLEMENT_DYNAMIC(PrefMediaPropertyPage, CPropertyPage)
PrefMediaPropertyPage::PrefMediaPropertyPage()
	: CPropertyPage(PrefMediaPropertyPage::IDD)
{
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	m_do_ffmpeg = prefs->m_prefer_ffmpeg;
	m_do_rtsp_tcp = prefs->m_prefer_rtsp_tcp;
}

BOOL
PrefMediaPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	return TRUE;
}

PrefMediaPropertyPage::~PrefMediaPropertyPage()
{
}

void PrefMediaPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_DO_FFMPEG, m_do_ffmpeg);
	DDX_Check(pDX, IDC_DO_RTSP_TCP, m_do_rtsp_tcp);
}

// PreferencesDlg message handlers

void PrefMediaPropertyPage::OnOK()
{
	USES_CONVERSION;
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_prefer_ffmpeg = m_do_ffmpeg;
	prefs->m_prefer_rtsp_tcp = m_do_rtsp_tcp;
	prefs->save_preferences();
	CPropertyPage::OnOK();
}

// Parser property page
BEGIN_MESSAGE_MAP(PrefParserPropertyPage, CPropertyPage)
END_MESSAGE_MAP()


IMPLEMENT_DYNAMIC(PrefParserPropertyPage, CPropertyPage)
PrefParserPropertyPage::PrefParserPropertyPage()
	: CPropertyPage(PrefParserPropertyPage::IDD)
{
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	m_parser_id = prefs->m_parser_id.c_str();
	m_validation_scheme = prefs->m_validation_scheme.c_str();
	m_do_namespaces = prefs->m_do_namespaces;
	m_do_schema = prefs->m_do_schema;
	m_validation_schema_full_checking = prefs->m_validation_schema_full_checking;
}

BOOL
PrefParserPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	return TRUE;
}

PrefParserPropertyPage::~PrefParserPropertyPage()
{
}

void PrefParserPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PARSER_ID, m_parser_id);
	DDX_Text(pDX, IDC_VALIDATION_SCHEME, m_validation_scheme);
	DDX_Check(pDX, IDC_DO_NAMESPACES, m_do_namespaces);
	DDX_Radio(pDX, IDC_DO_SCHEMA, m_do_schema);
	DDX_Check(pDX, IDC_VALIDATION_SCHEMA_FULL_CHECKING, m_validation_schema_full_checking);
}

// PreferencesDlg message handlers

void PrefParserPropertyPage::OnOK()
{
	USES_CONVERSION;
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_parser_id = T2CA((LPCTSTR)m_parser_id);
	prefs->m_validation_scheme = T2CA((LPCTSTR)m_validation_scheme);
	prefs->m_do_namespaces = (bool)m_do_namespaces;
	prefs->m_do_schema = (bool)m_do_schema;
	prefs->m_validation_schema_full_checking = (bool)m_validation_schema_full_checking;
	prefs->save_preferences();
	CPropertyPage::OnOK();
}

// Plugins property page
BEGIN_MESSAGE_MAP(PrefPluginsPropertyPage, CPropertyPage)
END_MESSAGE_MAP()


IMPLEMENT_DYNAMIC(PrefPluginsPropertyPage, CPropertyPage)
PrefPluginsPropertyPage::PrefPluginsPropertyPage()
	: CPropertyPage(PrefPluginsPropertyPage::IDD)
{
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	m_do_plugins = prefs->m_use_plugins;
	m_plugin_path = prefs->m_plugin_path.c_str();
}

BOOL
PrefPluginsPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
#ifdef WITH_WINDOWS_PLUGINS
	CWnd *item = GetDlgItem(IDC_DO_PLUGINS);
	item->EnableWindow();
	item = GetDlgItem(IDC_PLUGIN_DIR);
	item->EnableWindow();
#endif
	return TRUE;
}

PrefPluginsPropertyPage::~PrefPluginsPropertyPage()
{
}

void PrefPluginsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_DO_PLUGINS, m_do_plugins);
	DDX_Text(pDX, IDC_PLUGIN_DIR, m_plugin_path);
}

// PreferencesDlg message handlers

void PrefPluginsPropertyPage::OnOK()
{
	USES_CONVERSION;
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_use_plugins = m_do_plugins;
	prefs->m_plugin_path = T2CA((LPCTSTR)m_plugin_path);
	prefs->save_preferences();
	CPropertyPage::OnOK();
}
BEGIN_MESSAGE_MAP(PrefPropertySheet, CPropertySheet)
END_MESSAGE_MAP()


IMPLEMENT_DYNAMIC(PrefPropertySheet, CPropertySheet)
PrefPropertySheet::PrefPropertySheet(CWnd* pWndParent)
:	CPropertySheet(_T("Preferences"), pWndParent)
{
	AddPage(&m_page1);
	AddPage(&m_page2);
	AddPage(&m_page3);
	AddPage(&m_page4);
}
