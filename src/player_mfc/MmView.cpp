/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

// MmView.cpp : implementation of the MmView class
//

#include "stdafx.h"
#include "DemoPlayer.h"

#include "MmDoc.h"
#include "MmView.h"

#include "MmViewPlayer.h"

#include ".\mmview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static MmViewPlayer *player = 0;

//static 
MmViewPlayer* MmViewPlayer::get_instance() {
	return player;
}

// MmView

IMPLEMENT_DYNCREATE(MmView, CView)

BEGIN_MESSAGE_MAP(MmView, CView)
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_PLAY, OnFilePlay)
	ON_UPDATE_COMMAND_UI(ID_FILE_PLAY, OnUpdateFilePlay)
	ON_COMMAND(ID_FILE_PAUSE, OnFilePause)
	ON_UPDATE_COMMAND_UI(ID_FILE_PAUSE, OnUpdateFilePause)
	ON_COMMAND(ID_FILE_STOP, OnFileStop)
	ON_UPDATE_COMMAND_UI(ID_FILE_STOP, OnUpdateFileStop)
	ON_WM_TIMER()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// MmView construction/destruction

MmView::MmView()
{
	// TODO: add construction code here
	m_timer_id = 0;
}

MmView::~MmView()
{
}

BOOL MmView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// MmView drawing

void MmView::OnDraw(CDC* /*pDC*/)
{
	MmDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	if(player)
		player->redraw();
	
}


// MmView diagnostics

#ifdef _DEBUG
void MmView::AssertValid() const
{
	CView::AssertValid();
}

void MmView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

MmDoc* MmView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(MmDoc)));
	return (MmDoc*)m_pDocument;
}
#endif //_DEBUG


// MmView message handlers
int MmView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	player = new MmViewPlayer(m_hWnd);
	//m_timer_id = SetTimer(1, 500, 0);

	return 0;
}

void MmView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	if(player)
		player->redraw();

}

void MmView::OnDestroy()
{
	delete player;
	player = 0;
	if(m_timer_id) KillTimer(m_timer_id);
	CView::OnDestroy();
	// TODO: Add your message handler code here
}

void MmView::SetMMDocument(LPCTSTR lpszPathName) {
	if(!player) {
		AfxMessageBox("Player is null");
		return;
	}
	player->set_document(lpszPathName);
}

void MmView::OnFilePlay()
{
	player->start();
}

void MmView::OnUpdateFilePlay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((player != 0 && player->can_start())?TRUE:FALSE);
	
}

void MmView::OnFilePause()
{
	player->pause();
}

void MmView::OnUpdateFilePause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((player && player->can_pause())?TRUE:FALSE);
}

void MmView::OnFileStop()
{
	player->stop();
}

void MmView::OnUpdateFileStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((player && player->can_stop())?TRUE:FALSE);
}

void MmView::OnTimer(UINT nIDEvent)
{
	if(player)
		player->update_status();
	CView::OnTimer(nIDEvent);
}

