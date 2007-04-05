// AmbulantPlayerDoc.h : interface of the CAmbulantPlayerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_AMBULANTPLAYERDOC_H__020B0BA9_65B1_4E4D_BA2F_E1EC9A189E7C__INCLUDED_)
#define AFX_AMBULANTPLAYERDOC_H__020B0BA9_65B1_4E4D_BA2F_E1EC9A189E7C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CAmbulantPlayerDoc : public CDocument
{
protected: // create from serialization only
	CAmbulantPlayerDoc();
	DECLARE_DYNCREATE(CAmbulantPlayerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAmbulantPlayerDoc)
	public:
	virtual BOOL OnNewDocument();
#ifndef _WIN32_WCE_NO_ARCHIVE_SUPPORT
	virtual void Serialize(CArchive& ar);
#endif // !_WIN32_WCE_NO_ARCHIVE_SUPPORT
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAmbulantPlayerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAmbulantPlayerDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AMBULANTPLAYERDOC_H__020B0BA9_65B1_4E4D_BA2F_E1EC9A189E7C__INCLUDED_)
