#if !defined(AFX_SELECTDLG_H__3A9CAA9D_283B_44DE_A3C9_4E2D9B2E6E24__INCLUDED_)
#define AFX_SELECTDLG_H__3A9CAA9D_283B_44DE_A3C9_4E2D9B2E6E24__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectDlg dialog

class CSelectDlg : public CDialog
{
// Construction
public:
	CSelectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectDlg)
	enum { IDD = IDD_SELECT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	CString m_dir;
	CString m_filename;
	void FillList(LPCTSTR ext);
	CString GetPathName();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectDlg)
	afx_msg void OnDblclkList1();
	afx_msg void OnSelchangeList1();
	virtual BOOL OnInitDialog();
	afx_msg void OnCloseupDirCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTDLG_H__3A9CAA9D_283B_44DE_A3C9_4E2D9B2E6E24__INCLUDED_)
