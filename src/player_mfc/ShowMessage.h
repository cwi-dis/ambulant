#pragma once

// Function that shows a message (in a dialog box) and optionally
// shows the log window
void log_show_message(int level, const char *message);

// CShowMessage dialog

class CShowMessage : public CDialog
{
	DECLARE_DYNAMIC(CShowMessage)

public:
	CShowMessage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowMessage();

// Dialog Data
	enum { IDD = IDD_SHOW_MESSAGE };
	CString m_message;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnBnClickedShowLog();

	DECLARE_MESSAGE_MAP()
};
