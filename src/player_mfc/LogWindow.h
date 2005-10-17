#pragma once
#include "afxcmn.h"
#include "ambulant/lib/amstream.h"


// Implementation of lib::ostream that sends output to the
// log window.
class logwindow_ostream : public ambulant::lib::ostream {
	bool is_open() const {return true;}
	void close() {}
	int write(const unsigned char *buffer, int nbytes) {return write("ostream use of buffer, size not implemented for MFC");}
	int write(const char *cstr);
	void write(ambulant::lib::byte_buffer& bb) {write("ostream use of byte_buffer not implemented for MFC");}
	void flush() {}
};


// LogWindow dialog

class CLogWindow : public CDialog
{
	DECLARE_DYNAMIC(CLogWindow)

private:
	CLogWindow(CWnd* pParent = NULL);   // standard constructor
	static CLogWindow *s_singleton;
public:
	static CLogWindow *GetLogWindowSingleton();
	virtual ~CLogWindow();

	void AppendText(const char *data);
// Dialog Data
	enum { IDD = IDD_LOG_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_richedit;
	afx_msg void OnEnChangeRichedit21();
};
