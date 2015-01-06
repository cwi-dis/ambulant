// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// MmDoc.cpp : implementation of the MmDoc class
//

#include "stdafx.h"
#include "AmbulantPlayer.h"

#include "MmDoc.h"
#include ".\mmdoc.h"

#include "MmView.h"
#include ".\mmview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// MmDoc

IMPLEMENT_DYNCREATE(MmDoc, CDocument)

BEGIN_MESSAGE_MAP(MmDoc, CDocument)
END_MESSAGE_MAP()


// MmDoc construction/destruction

MmDoc::MmDoc()
{
	// TODO: add one-time construction code here
	m_autostart = false;
}

MmDoc::~MmDoc()
{
}

BOOL MmDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	return TRUE;
}




// MmDoc serialization

void MmDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// MmDoc diagnostics

#ifdef _DEBUG
void MmDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void MmDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// MmDoc commands

BOOL MmDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	//if (!CDocument::OnOpenDocument(lpszPathName))
	//	return FALSE;

	// TODO:  Add your specialized creation code here
	POSITION pos = GetFirstViewPosition();
	if(pos != NULL) {
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		((MmView*)pView)->SetMMDocument(lpszPathName, m_autostart);
	}
	return TRUE;
}

void MmDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU) {
	if(_tcsstr(lpszPathName, _T("://")) != 0) {
		// seems a url
		m_strPathName = "URL";
		m_bEmbedded = FALSE;
		SetTitle(_T("URL"));
	} else {
		CDocument::SetPathName(lpszPathName, bAddToMRU);
	}
}

void MmDoc::StartPlayback() {
	POSITION pos = GetFirstViewPosition();
	if(pos != NULL) {
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		((MmView*)pView)->OnFilePlay();
	}
}
