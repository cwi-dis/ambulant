#pragma once


// PreferencesDlg dialog

class PreferencesDlg : public CDialog
{
	DECLARE_DYNAMIC(PreferencesDlg)

public:
	PreferencesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~PreferencesDlg();

// Dialog Data
	enum { IDD = IDD_PREFERENCES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	int m_log_level;
	CString m_parser_id;
	CString m_validation_scheme;
	int m_do_namespaces;
	int m_do_schema;
//	int m_do_dtd;
	int m_do_validation;
	int m_validation_schema_full_checking;
public:
	afx_msg void OnBnClickedOK();
	afx_msg void OnBnClickedCancel();
};
