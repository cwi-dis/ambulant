// ShowMessage.cpp : implementation file
//

#include "stdafx.h"
#include "AmbulantPlayer.h"
#include "ShowMessage.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"

void log_show_message(int level, const char *message) {
	CShowMessage msg;
	msg.m_message = (LPCTSTR)ambulant::lib::textptr(message);
	// XXXX How do I find our topmost window??
	if (msg.DoModal() == IDSHOWLOG) 
		PostMessage(NULL, WM_COMMAND, ID_VIEW_LOG, 0);
}

// CShowMessage dialog

IMPLEMENT_DYNAMIC(CShowMessage, CDialog)
CShowMessage::CShowMessage(CWnd* pParent /*=NULL*/)
	: CDialog(CShowMessage::IDD, pParent)
{
}

CShowMessage::~CShowMessage()
{
}

void CShowMessage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MESSAGE, m_message);
}

void CShowMessage::OnBnClickedShowLog()
{
	EndDialog(IDSHOWLOG);
}

BEGIN_MESSAGE_MAP(CShowMessage, CDialog)
	ON_BN_CLICKED(IDSHOWLOG, OnBnClickedShowLog)
END_MESSAGE_MAP()


// CShowMessage message handlers
