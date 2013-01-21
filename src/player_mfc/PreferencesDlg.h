#pragma once


// PreferencesDlg dialog

class PrefLoggingPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(PrefLoggingPropertyPage)

public:
	PrefLoggingPropertyPage();   // standard constructor
	virtual ~PrefLoggingPropertyPage();
	BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_PROPPAGE_LOGGING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	int m_log_level;
public:
	afx_msg void OnOK();
};

class PrefMediaPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(PrefMediaPropertyPage)

public:
	PrefMediaPropertyPage();   // standard constructor
	virtual ~PrefMediaPropertyPage();
	BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_PROPPAGE_MEDIA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	int m_do_ffmpeg;
	int m_do_rtsp_tcp;
public:
	afx_msg void OnOK();
};

class PrefParserPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(PrefParserPropertyPage)

public:
	PrefParserPropertyPage();   // standard constructor
	virtual ~PrefParserPropertyPage();
	BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_PROPPAGE_PARSER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString m_parser_id;
	CString m_validation_scheme;
	int m_do_namespaces;
	int m_do_schema;
//	int m_do_dtd;
	int m_validation_schema_full_checking;
public:
	afx_msg void OnOK();
};

class PrefPluginsPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(PrefPluginsPropertyPage)

public:
	PrefPluginsPropertyPage();   // standard constructor
	virtual ~PrefPluginsPropertyPage();
	BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_PROPPAGE_PLUGINS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	int m_do_plugins;
	CString m_plugin_path;
public:
	afx_msg void OnOK();
};

/////////////////////////////////////////////////////////////////////////////
// CAmisPropertySheet

class PrefPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(PrefPropertySheet)

// Construction
public:


	PrefPropertySheet(CWnd* pWndParent = NULL);

	~PrefPropertySheet(){}

// Attributes
public:
	PrefLoggingPropertyPage m_page1;
	PrefMediaPropertyPage m_page2;
	PrefParserPropertyPage m_page3;
	PrefPluginsPropertyPage m_page4;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAmisPropertySheet)
	public:
//	virtual BOOL OnInitDialog();
//	virtual int DoModal();
	//}}AFX_VIRTUAL

// Implementation
public:

//variables
private:

// Generated message map functions
protected:
	//{{AFX_MSG(CAmisPropertySheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

/////////////////////////////////////////////////////////////////////////////

