#pragma once


// COpenUrlDlg dialog

class COpenUrlDlg : public CDialog
{
	DECLARE_DYNAMIC(COpenUrlDlg)

public:
	COpenUrlDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COpenUrlDlg();

// Dialog Data
	enum { IDD = IDD_OPEN_URL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// The URL to open
	CString m_url;
protected:
	virtual void OnOK();
public:
	afx_msg void OnBnClickedButtonBrowse();
};
