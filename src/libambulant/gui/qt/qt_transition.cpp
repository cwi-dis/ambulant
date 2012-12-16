// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_transition.h"
#include "ambulant/gui/qt/qt_util.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/logger.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace qt {

void
qt_transition_debug::paint_rect(
	ambulant_qt_window* aqw,
	common::surface * dst,
	color_t color)
{
	const rect& dstrect_whole = dst->get_clipped_screen_rect();
	QPainter paint;
	paint.begin(aqw->get_ambulant_pixmap());
	int L = dstrect_whole.left(),
	T = dstrect_whole.top(),
	W = dstrect_whole.width(),
	H = dstrect_whole.height();
	// XXXX Fill with background color
	AM_DBG logger::get_logger()->debug("qt_transition_debug::paint_rect: %s0x%x,%s(%d,%d,%d,%d)", " clearing to ", (long)color," local_ltwh=",L,T,W,H);
	QColor bgc = QColor(redc(color), greenc(color), bluec(color));
	paint.setBrush(bgc);
	paint.drawRect(L,T,W,H);
	paint.flush();
	paint.end();
}

// Helper functions to setup transitions

static void
setup_transition(bool outtrans, ambulant_qt_window *aqw, QPixmap** oldpxmp, QPixmap** newpxmp)
{
	if (outtrans) {
		if (aqw->m_tmppixmap == NULL) {
			// make a copy
			aqw->m_tmppixmap = new QPixmap(*aqw->get_ambulant_oldpixmap());
		}
		*oldpxmp = aqw->get_ambulant_surface();
		*newpxmp = aqw->m_tmppixmap;
	} else {
		*oldpxmp = aqw->get_ambulant_pixmap();
		*newpxmp = aqw->get_ambulant_surface();
		DUMPPIXMAP(*oldpxmp, "oldpxmp");
		DUMPPIXMAP(*newpxmp, "newpxmp");
	}
}

static void
finalize_transition(bool outtrans, ambulant_qt_window *aqw,	 common::surface *dest)
{
	if (outtrans) {
		// copy the pixels in m_tmppixmap to the on-screen pixmap
		QPixmap* dest_pixmap = aqw->get_ambulant_pixmap();
		QPixmap* temp_pixmap = aqw->get_ambulant_surface();
		const lib::rect& r = dest->get_clipped_screen_rect();
		AM_DBG logger::get_logger()->debug("finalize_transition: dest_pixmap=0x%x: temp_pixmap=0x%x (L,T,W,H)=(%d,%d,%d,%d)", dest_pixmap, temp_pixmap,r.left(),r.top(),r.width(), r.height());
		bitBlt(dest_pixmap,r.left(),r.top(), temp_pixmap,r.left(),r.top(),r.width(), r.height());
	}
}

void
qt_transition_blitclass_fade::update()
{
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_fade::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *npm, *qpm;
	setup_transition(m_outtrans, aqw, &qpm, &npm);
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_fade::update(%f) qpm(%d,%d),npm(%d,%d)", m_progress, qpm->width(),	qpm->height(), npm->width(), npm->height());
	QImage m_old_image;
	QImage m_new_image;
	m_old_image = qpm->convertToImage();
	m_new_image = npm->convertToImage();
	const rect&	 newrect_whole =  m_dst->get_clipped_screen_rect();
	qt_image_blend (
		m_old_image, newrect_whole,
		m_new_image, newrect_whole,
		m_progress, 0.0,
		0x000000, 0xFFFFFF);
	QImage res(m_old_image);
	int L = newrect_whole.left(),
		T = newrect_whole.top(),
		W = newrect_whole.width(),
		H = newrect_whole.height();
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_fade::update(): ltwh=(%d,%d,%d,%d)",L,T,W,H);
	QPixmap rpm(W,H);
	rpm.convertFromImage(res);
	bitBlt(qpm, L, T, &rpm, L, T, W, H);
	finalize_transition(m_outtrans, aqw, m_dst);
}

void
qt_transition_blitclass_rect::update()
{
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_rect::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *npm, *qpm;
	setup_transition(m_outtrans, aqw, &qpm, &npm);
	rect newrect_whole = m_newrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	newrect_whole &= m_dst->get_clipped_screen_rect();
	int L = newrect_whole.left(), T = newrect_whole.top(),
		W = newrect_whole.width(), H = newrect_whole.height();
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_rect: qpm=0x%x, npm=0x%x, (L,T,W,H)=(%d,%d,%d,%d)",qpm,npm,L,T,W,H);
	bitBlt(qpm, L, T, npm, L, T, W, H);
	finalize_transition(m_outtrans, aqw, m_dst);
}
void
qt_transition_blitclass_r1r2r3r4::update()
{
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_r1r2r3r4::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *npm, *qpm;
	setup_transition(m_outtrans, aqw, &qpm, &npm);
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_r1r2r3r4::update(%f) qpm(%d,%d),npm(%d,%d)", m_progress, qpm->width(),	qpm->height(), npm->width(), npm->height());
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_r1r2r3r4::update() qpm=0x%x, npm=0x%x.", qpm, npm);
	rect oldsrcrect_whole = m_oldsrcrect;
	rect olddstrect_whole = m_olddstrect;
	rect newsrcrect_whole = m_newsrcrect;
	rect newdstrect_whole = m_newdstrect;
	oldsrcrect_whole.translate(m_dst->get_global_topleft());
	oldsrcrect_whole &= m_dst->get_clipped_screen_rect();
	olddstrect_whole.translate(m_dst->get_global_topleft());
	olddstrect_whole &= m_dst->get_clipped_screen_rect();
	newsrcrect_whole.translate(m_dst->get_global_topleft());
	newsrcrect_whole &= m_dst->get_clipped_screen_rect();
	newdstrect_whole.translate(m_dst->get_global_topleft());
	newdstrect_whole &= m_dst->get_clipped_screen_rect();
	int Loldsrc = oldsrcrect_whole.left(),
		Toldsrc = oldsrcrect_whole.top(),
		Woldsrc = oldsrcrect_whole.width(),
		Holdsrc = oldsrcrect_whole.height();
//	logger::get_logger()->debug("qt_transition_blitclass_r1r2r3r4: (Loldsrc,Toldsrc,Woldsrc,Holdsrc)=(%d,%d,%d,%d)",Loldsrc,Toldsrc,Woldsrc,Holdsrc);
	int Lolddst = olddstrect_whole.left(),
		Tolddst = olddstrect_whole.top(),
		Wolddst = olddstrect_whole.width(),
		Holddst = olddstrect_whole.height();
//	logger::get_logger()->debug("qt_transition_blitclass_r1r2r3r4: (Lolddst,Tolddst,Wolddst,Holddst)=(%d,%d,%d,%d)",Lolddst,Tolddst,Wolddst,Holddst);
	int Lnewsrc = newsrcrect_whole.left(),
		Tnewsrc = newsrcrect_whole.top(),
		Wnewsrc = newsrcrect_whole.width(),
		Hnewsrc = newsrcrect_whole.height();
//	logger::get_logger()->debug("qt_transition_blitclass_r1r2r3r4: (Lnewsrc,Tnewsrc,Wnewsrc,Hnewsrc)=(%d,%d,%d,%d)",Lnewsrc,Tnewsrc,Wnewsrc,Hnewsrc);
	int Lnewdst = newdstrect_whole.left(),
		Tnewdst = newdstrect_whole.top(),
		Wnewdst = newdstrect_whole.width(),
		Hnewdst = newdstrect_whole.height();
//	logger::get_logger()->debug("qt_transition_blitclass_r1r2r3r4: (Lnewdst,Tnewdst,Wnewdst,Hnewdst)=(%d,%d,%d,%d)",Lnewdst,Tnewdst,Wnewdst,Hnewdst);
	bitBlt(qpm, Lolddst, Tolddst, qpm, Loldsrc, Toldsrc, Woldsrc, Holdsrc);
	bitBlt(qpm, Lnewdst, Tnewdst, npm, Lnewsrc, Tnewsrc, Wnewsrc, Hnewsrc);
	finalize_transition(m_outtrans, aqw, m_dst);
}

void
qt_transition_blitclass_rectlist::update()
{
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_rectlist::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *npm, *qpm;
	setup_transition(m_outtrans, aqw, &qpm, &npm);
	QImage img1 = qpm->convertToImage();
	QImage img2 = npm->convertToImage();
	const rect& dstrect_whole = m_dst->get_clipped_screen_rect();
	int L = dstrect_whole.left(), T = dstrect_whole.top(),
		W = dstrect_whole.width(), H = dstrect_whole.height();
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_rectlist: (L,T,W,H)=(%d,%d,%d,%d)",L,T,W,H);
	QPainter paint;
	QRegion clip_region;
	paint.begin(qpm);
	paint.drawImage(L,T,img1,L,T,W,H);
	std::vector< rect >::iterator newrect;
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		rect corner_rect = *newrect;
		corner_rect.translate(m_dst->get_global_topleft());
		corner_rect &= m_dst->get_clipped_screen_rect();
		int L = corner_rect.left(), T = corner_rect.top(),
			W = corner_rect.width(), H = corner_rect.height();
		AM_DBG logger::get_logger()->debug("qt_transition_blitclass_rectlist: (L,T,W,H)=(%d,%d,%d,%d)",L,T,W,H);
		QRegion newcorner(L,T,W,H);
		clip_region += newcorner;
	}
	paint.setClipRegion(clip_region);
	paint.drawImage(L,T,img2,L,T,W,H);
	paint.flush();
	paint.end();
	finalize_transition(m_outtrans, aqw, m_dst);
}

void
qt_transition_blitclass_poly::update()
{
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_poly::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *npm, *qpm;
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	setup_transition(m_outtrans, aqw, &qpm, &npm);
//	QImage img1 = qpm->convertToImage();
	QImage img2 = npm->convertToImage();
	QPointArray qpa;
	int idx = 0;
	std::vector<point>::iterator newpoint;
	for( newpoint=m_newpolygon.begin();
		newpoint != m_newpolygon.end(); newpoint++)
	{
		point p = *newpoint + dst_global_topleft;
		qpa.putPoints(idx++, 1, p.x, p.y);
	}
	QRegion qreg(qpa, true);
	const rect& newrect_whole =	 m_dst->get_clipped_screen_rect();
	int L = newrect_whole.left(), T = newrect_whole.top(),
		W = newrect_whole.width(), H = newrect_whole.height();
	QPainter paint;
	paint.begin(qpm);
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_poly::update(): ltwh=(%d,%d,%d,%d)",L,T,W,H);
//	paint.drawImage(L,T,img1,L,T,W,H);
	paint.setClipRegion(qreg);
	paint.drawImage(L,T,img2,L,T,W,H);
	paint.flush();
	paint.end();
	finalize_transition(m_outtrans, aqw, m_dst);
}

void
qt_transition_blitclass_polylist::update()
{
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_polylist::update(%f)", m_progress);
	ambulant_qt_window *aqw = (ambulant_qt_window *)m_dst->get_gui_window();
	QPixmap *npm, *qpm;
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	setup_transition(m_outtrans, aqw, &qpm, &npm);
//	QImage img1 = qpm->convertToImage();
	QImage img2 = npm->convertToImage();
	QRegion clip_region;
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_polylist: m_newpolygonlist.size()=%d", m_newpolygonlist.size());
	std::vector< std::vector<point> >::iterator partpolygon;
	for (partpolygon=m_newpolygonlist.begin();
		partpolygon!=m_newpolygonlist.end(); partpolygon++)
	{
		std::vector<point>::iterator newpoint;
		AM_DBG logger::get_logger()->debug("qt_transition_blitclass_polylist: partpolygon.size()=%d", partpolygon->size());
		QPointArray qpa;
		int idx = 0;
		for( newpoint=partpolygon->begin(); newpoint != partpolygon->end(); newpoint++) {
			point p = *newpoint + dst_global_topleft;
			qpa.putPoints(idx++, 1, p.x, p.y);
			AM_DBG logger::get_logger()->debug("qt_transition_blitclass_polylist: idx=%d, p=(%d,%d)", idx, p.x, p.y);
		}
		QRegion qreg(qpa, true);
		clip_region += qreg;
	}
	const rect& newrect_whole =	 m_dst->get_clipped_screen_rect();
	int L = newrect_whole.left(), T = newrect_whole.top(),
		W = newrect_whole.width(), H = newrect_whole.height();
	AM_DBG logger::get_logger()->debug("qt_transition_blitclass_polylist::update() drawImage npm=0x%x ltwh=(%d,%d,%d,%d)",npm,L,T,W,H);
	QPainter paint;
	paint.begin(qpm);
//	paint.drawImage(L,T,img1,L,T,W,H);
	paint.setClipRegion(clip_region);
	paint.drawImage(L,T,img2,L,T,W,H);
	paint.flush();
	paint.end();
	finalize_transition(m_outtrans, aqw, m_dst);
}

smil2::transition_engine *
qt_transition_engine(common::surface *dst, bool is_outtrans, const transition_info *info)
{
	smil2::transition_engine *rv;
	AM_DBG logger::get_logger()->debug("qt_transition_engine: info=0x%x info->m_type=%d", info, info->m_type);

	switch(info->m_type) {
	
	// Series 1: edge wipes
	case barWipe:
		rv = new qt_transition_engine_barwipe();
		break;
	case boxWipe:
		rv = new qt_transition_engine_boxwipe();
		break;
	case fourBoxWipe:
		rv = new qt_transition_engine_fourboxwipe();
		break;
	case barnDoorWipe:
		rv = new qt_transition_engine_barndoorwipe();
		break;
	case diagonalWipe:
		rv = new qt_transition_engine_diagonalwipe();
		break;
	case miscDiagonalWipe:
		rv = new qt_transition_engine_miscdiagonalwipe();
		break;
	case veeWipe:
		rv = new qt_transition_engine_veewipe();
		break;
	case barnVeeWipe:
		rv = new qt_transition_engine_barnveewipe();
		break;
	case zigZagWipe:
		rv = new qt_transition_engine_zigzagwipe();
		break;
	case barnZigZagWipe:
		rv = new qt_transition_engine_barnzigzagwipe();
		break;
	case bowTieWipe:
		rv = new qt_transition_engine_bowtiewipe();
		break;
	
	// series 2: iris wipes
	case irisWipe:
		rv = new qt_transition_engine_iriswipe();
		break;
	case pentagonWipe:
		rv = new qt_transition_engine_pentagonwipe();
		break;
	case arrowHeadWipe:
		rv = new qt_transition_engine_arrowheadwipe();
		break;
	case triangleWipe:
		rv = new qt_transition_engine_trianglewipe();
		break;
	case hexagonWipe:
		rv = new qt_transition_engine_hexagonwipe();
		break;
	case eyeWipe:
		rv = new qt_transition_engine_eyewipe();
		break;
	case roundRectWipe:
		rv = new qt_transition_engine_roundrectwipe();
		break;
	case ellipseWipe:
		rv = new qt_transition_engine_ellipsewipe();
		break;
	case starWipe:
		rv = new qt_transition_engine_starwipe();
		break;
	case miscShapeWipe:
		rv = new qt_transition_engine_miscshapewipe();
		break;
	
	// series 3: clock-type wipes
	case clockWipe:
		rv = new qt_transition_engine_clockwipe();
		break;
	case singleSweepWipe:
		rv = new qt_transition_engine_singlesweepwipe();
		break;
	case doubleSweepWipe:
		rv = new qt_transition_engine_doublesweepwipe();
		break;
	case saloonDoorWipe:
		rv = new qt_transition_engine_saloondoorwipe();
		break;
	case windshieldWipe:
		rv = new qt_transition_engine_windshieldwipe();
		break;
	case fanWipe:
		rv = new qt_transition_engine_fanwipe();
		break;
	case doubleFanWipe:
		rv = new qt_transition_engine_doublefanwipe();
		break;
	case pinWheelWipe:
		rv = new qt_transition_engine_pinwheelwipe();
		break;
	
	// series 4: matrix wipe types
	case snakeWipe:
		rv = new qt_transition_engine_snakewipe();
		break;
	case waterfallWipe:
		rv = new qt_transition_engine_waterfallwipe();
		break;
	case spiralWipe:
		rv = new qt_transition_engine_spiralwipe();
		break;
	case parallelSnakesWipe:
		rv = new qt_transition_engine_parallelsnakeswipe();
		break;
	case boxSnakesWipe:
		rv = new qt_transition_engine_boxsnakeswipe();
		break;
	
	// series 5: SMIL-specific types
	case pushWipe:
		rv = new qt_transition_engine_pushwipe();
		break;
	case slideWipe:
		rv = new qt_transition_engine_slidewipe();
		break;
	case fade:
	case audioVisualFade:
		rv = new qt_transition_engine_fade();
		break;
	default:
		logger::get_logger()->warn(gettext("%s: transition type %s not yet implemented"), "qt_transition_engine", repr(info->m_type).c_str());
		rv = NULL;
	}
	if (rv)
		rv->init(dst, is_outtrans, info);
	return rv;
}

} // namespace qt

} // namespace gui

} //namespace ambulant

