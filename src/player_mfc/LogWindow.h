#pragma once
#include "afxcmn.h"
#include "ambulant/lib/amstream.h"

#define THREAD_SAFE_LOG

// Implementation of lib::ostream that sends output to the
// log window.
class logwindow_ostream : public ambulant::lib::ostream {
	bool is_open() const {return true;}
	void close() {}
	int write(const unsigned char *buffer, int nbytes) {return write("ostream use of buffer, size not implemented for MFC");}
	int write(const char *cstr);
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
#ifdef THREAD_SAFE_LOG
	LPARAM OnAmbulantMessage(WPARAM wParam, LPARAM lParam);
#endif
// Dialog Data
	enum { IDD = IDD_LOG_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	LPARAM OnAddLoggerLine(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_richedit;
	afx_msg void OnEnChangeRichedit21();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
