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

#include "ambulant/gui/cg/cg_transition.h"
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/lib/logger.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

namespace cg {
	
// Helper function: add a clockwise defined rectangle to the path of a CGContext
// This is used for out transitions to reverse the effect of the counter-clockwise defined 
// clipping paths by enclosing them in a clockwise defined rectangle for the whole region
// using the non-zero winding rule
void
add_clockwise_rectangle (CGContextRef ctx, CGRect cg_rect)
{	
	CGFloat cg_minX = CGRectGetMinX(cg_rect), cg_maxX = CGRectGetMaxX(cg_rect),
	cg_minY = CGRectGetMinY(cg_rect), cg_maxY = CGRectGetMaxY(cg_rect);
	CGContextMoveToPoint(ctx, cg_minX, cg_minY);
	CGContextAddLineToPoint(ctx, cg_minX, cg_maxY);
	CGContextAddLineToPoint(ctx, cg_maxX, cg_maxY);
	CGContextAddLineToPoint(ctx, cg_maxX, cg_minY);
	CGContextAddLineToPoint(ctx, cg_minX, cg_minY);
}
	
void
cg_transition_blitclass_fade::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_fade::update(%f)", m_progress);
	// the transition surface is a transparent layer over the full screen
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextRef ctx = [view getCGContext];
	CGContextSaveGState(ctx);
	lib::rect r = m_dst->get_rect();
	r.translate(m_dst->get_global_topleft());
	CGContextClipToRect(ctx,CGRectFromAmbulantRect(r));
	CGContextSetAlpha (ctx, m_outtrans ? 1.0 - m_progress : m_progress);
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	CGContextRestoreGState(ctx);
}

void
cg_transition_blitclass_rect::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	lib::rect newrect_whole = m_newrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	newrect_whole &= m_dst->get_clipped_screen_rect();
	lib::point LT = newrect_whole.left_top();
	lib::point RB = newrect_whole.right_bottom();
	CGRect cg_clipped_rect = CGRectFromAmbulantRect(newrect_whole);
	AM_DBG NSLog(@"cg_transition_blitclass_rect::update(%f) newrect_whole=(%d,%d),(%d,%d)",m_progress,LT.x,LT.y,RB.x,RB.y);
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	lib::rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
	dstrect_whole &= m_dst->get_clipped_screen_rect();
	CGRect cg_dstrect_whole = CGRectFromAmbulantRect(dstrect_whole);
	CGContextRef ctx = [view getCGContext];
	CGContextSaveGState(ctx);
	if (m_outtrans) {
		add_clockwise_rectangle (ctx, cg_dstrect_whole);
	}
	CGContextAddRect(ctx, cg_clipped_rect);
	CGContextClip(ctx);
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	CGContextRestoreGState(ctx);
}

void
cg_transition_blitclass_r1r2r3r4::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	lib::rect oldrect = m_dst->get_rect();
	lib::rect newrect_whole = m_newsrcrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	newrect_whole &= m_dst->get_clipped_screen_rect();
	int dx = 0;
	int dy = 0;
	CGContextRef ctx = [view getCGContext];
	CGContextSaveGState(ctx);
	if ( ! m_outtrans) {
		dx = m_newdstrect.left() - m_newsrcrect.left();
		dy = m_newdstrect.top() - m_newsrcrect.top();
	}
	CGContextTranslateCTM (ctx, dx, dy);
	AM_DBG NSLog(@"m_oldsrcrect=(%d,%d,%d,%d)",m_oldsrcrect.left(),m_oldsrcrect.top(),m_oldsrcrect.width(),m_oldsrcrect.height());
	AM_DBG NSLog(@"m_olddstrect=(%d,%d,%d,%d)",m_olddstrect.left(),m_olddstrect.top(),m_olddstrect.width(),m_olddstrect.height());
	AM_DBG NSLog(@"m_newsrcrect=(%d,%d,%d,%d)",m_newsrcrect.left(),m_newsrcrect.top(),m_newsrcrect.width(),m_newsrcrect.height());
	AM_DBG NSLog(@"m_newdstrect=(%d,%d,%d,%d)",m_newdstrect.left(),m_newdstrect.top(),m_newdstrect.width(),m_newdstrect.height());
	AM_DBG NSLog(@"dx=%d, dy=%d",dx,dy);

	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	if (m_outtrans) {
		lib::rect r(m_olddstrect);
		r.translate(m_dst->get_global_topleft());
		CGContextClipToRect(ctx, CGRectFromAmbulantRect(r));
	} else {
		CGContextClipToRect(ctx, CGRectFromAmbulantRect(newrect_whole));
	}

	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	CGContextRestoreGState(ctx);
}

void
cg_transition_blitclass_rectlist::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextRef ctx = [view getCGContext];
	CGContextSaveGState(ctx);
	bool is_clipped = false;
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_rectlist::update(%f)", m_progress);
	std::vector< lib::rect >::iterator newrect;
	for (newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		lib::rect newrect_whole = *newrect;
		if (newrect_whole.empty()) {
			continue;
		}
		is_clipped = true;
		newrect_whole.translate(m_dst->get_global_topleft()); // Translate so the right topleft pixel is in place
		newrect_whole &= m_dst->get_clipped_screen_rect();
		CGContextAddRect(ctx, CGRectFromAmbulantRect(newrect_whole));
	}
	if (is_clipped) {
		if (m_outtrans) {
			lib::rect dstrect_whole = m_dst->get_rect();
			dstrect_whole.translate(m_dst->get_global_topleft());
			dstrect_whole &= m_dst->get_clipped_screen_rect();
			CGRect cg_dstrect_whole = CGRectFromAmbulantRect(dstrect_whole);
			add_clockwise_rectangle (ctx, cg_dstrect_whole);
		}		
		CGContextClip(ctx);
		CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	} else if (m_outtrans) {
		CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	}
	
	CGContextRestoreGState(ctx);
}

// Helper function: convert a point list to a CGPath
static CGPathRef
polygon2path(const lib::point& origin, std::vector<lib::point> polygon)
{
	CGMutablePathRef path = CGPathCreateMutable ();
	std::vector<lib::point>::iterator newpoint;
	bool first = true;
	lib::point old_point;
	for( newpoint=polygon.begin(); newpoint != polygon.end(); newpoint++) {
		lib::point p = *newpoint + origin;
		if ( ! first) {
			if (p.x == old_point.x && p.y == old_point.y) {
				continue;
			}
		}
		old_point  = p;
		AM_DBG lib::logger::get_logger()->debug("polygon2path: point=%d, %d", p.x, p.y);
		if (first) {
			CGPathMoveToPoint(path, NULL, p.x, p.y);
			first = false;
		} else {
			CGPathAddLineToPoint(path, NULL, p.x, p.y);
		}
	}
	CGPathCloseSubpath(path);
	return path;
}

void
cg_transition_blitclass_poly::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	
	CGContextRef ctx = [view getCGContext];
	CGContextSaveGState(ctx);

	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_poly::update(%f)", m_progress);
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	lib::rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(dst_global_topleft);
	dstrect_whole &= m_dst->get_clipped_screen_rect();
	if (m_outtrans) {
		add_clockwise_rectangle(ctx, CGRectFromAmbulantRect(dstrect_whole));
	}

	// Define the clipping path
	CGPathRef path = polygon2path(dst_global_topleft, m_newpolygon);
	CGContextAddPath(ctx, path);
	CGContextClip(ctx);
								
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	CGContextRestoreGState(ctx);
	CFRelease(path);
}

void
cg_transition_blitclass_polylist::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	CGContextRef ctx = [view getCGContext];
	CGContextSaveGState(ctx);

	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_poly::update(%f)", m_progress);
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	lib::rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(dst_global_topleft);
	dstrect_whole &= m_dst->get_clipped_screen_rect();
	if (m_outtrans) {
		add_clockwise_rectangle(ctx, CGRectFromAmbulantRect(dstrect_whole));
	}

	// First we create the path
	std::vector< std::vector<lib::point> >::iterator partpolygon;
	for (partpolygon=m_newpolygonlist.begin(); partpolygon!=m_newpolygonlist.end(); partpolygon++) {
		CGPathRef path = polygon2path(dst_global_topleft, *partpolygon);
		CGContextAddPath(ctx, path);
		CFRelease(path);
	}

	// Then we composite it onto the screen
	CGContextClip(ctx);
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	CGContextRestoreGState(ctx);
}

 
smil2::transition_engine *
cg_transition_engine(common::surface *dst, bool is_outtrans, const lib::transition_info *info)
{
	smil2::transition_engine *rv;

	switch(info->m_type) {
	// Series 1: edge wipes
	case lib::barWipe:
		rv = new cg_transition_engine_barwipe();
		break;
	case lib::boxWipe:
		rv = new cg_transition_engine_boxwipe();
		break;
	case lib::fourBoxWipe:
		rv = new cg_transition_engine_fourboxwipe();
		break;
	case lib::barnDoorWipe:
		rv = new cg_transition_engine_barndoorwipe();
		break;
	case lib::diagonalWipe:
		rv = new cg_transition_engine_diagonalwipe();
		break;
	case lib::miscDiagonalWipe:
		rv = new cg_transition_engine_miscdiagonalwipe();
		break;
	case lib::veeWipe:
		rv = new cg_transition_engine_veewipe();
		break;
	case lib::barnVeeWipe:
		rv = new cg_transition_engine_barnveewipe();
		break;
	case lib::zigZagWipe:
		rv = new cg_transition_engine_zigzagwipe();
		break;
	case lib::barnZigZagWipe:
		rv = new cg_transition_engine_barnzigzagwipe();
		break;
	case lib::bowTieWipe:
		rv = new cg_transition_engine_bowtiewipe();
		break;
	// series 2: iris wipes
	case lib::irisWipe:
		rv = new cg_transition_engine_iriswipe();
		break;
	case lib::pentagonWipe:
		rv = new cg_transition_engine_pentagonwipe();
		break;
	case lib::arrowHeadWipe:
		rv = new cg_transition_engine_arrowheadwipe();
		break;
	case lib::triangleWipe:
		rv = new cg_transition_engine_trianglewipe();
		break;
	case lib::hexagonWipe:
		rv = new cg_transition_engine_hexagonwipe();
		break;
	case lib::eyeWipe:
		rv = new cg_transition_engine_eyewipe();
		break;
	case lib::roundRectWipe:
		rv = new cg_transition_engine_roundrectwipe();
		break;
	case lib::ellipseWipe:
		rv = new cg_transition_engine_ellipsewipe();
		break;
	case lib::starWipe:
		rv = new cg_transition_engine_starwipe();
		break;
	case lib::miscShapeWipe:
		rv = new cg_transition_engine_miscshapewipe();
		break;
	// series 3: clock-type wipes
	case lib::clockWipe:
		rv = new cg_transition_engine_clockwipe();
		break;
	case lib::singleSweepWipe:
		rv = new cg_transition_engine_singlesweepwipe();
		break;
	case lib::doubleSweepWipe:
		rv = new cg_transition_engine_doublesweepwipe();
		break;
	case lib::saloonDoorWipe:
		rv = new cg_transition_engine_saloondoorwipe();
		break;
	case lib::windshieldWipe:
		rv = new cg_transition_engine_windshieldwipe();
		break;
	case lib::fanWipe:
		rv = new cg_transition_engine_fanwipe();
		break;
	case lib::doubleFanWipe:
		rv = new cg_transition_engine_doublefanwipe();
		break;
	case lib::pinWheelWipe:
		rv = new cg_transition_engine_pinwheelwipe();
		break;
	// series 4: matrix wipe types
	case lib::snakeWipe:
		rv = new cg_transition_engine_snakewipe();
		break;
	case lib::waterfallWipe:
		rv = new cg_transition_engine_waterfallwipe();
		break;
	case lib::spiralWipe:
		rv = new cg_transition_engine_spiralwipe();
		break;
	case lib::parallelSnakesWipe:
		rv = new cg_transition_engine_parallelsnakeswipe();
		break;
	case lib::boxSnakesWipe:
		rv = new cg_transition_engine_boxsnakeswipe();
		break;
	// series 5: SMIL-specific types
	case lib::pushWipe:
		rv = new cg_transition_engine_pushwipe();
		break;
	case lib::slideWipe:
		rv = new cg_transition_engine_slidewipe();
		break;
	case lib::fade:
	case lib::audioVisualFade:
		rv = new cg_transition_engine_fade();
		break;
	default:
		lib::logger::get_logger()->trace("cg_transition_engine: transition type %s not yet implemented",
			repr(info->m_type).c_str());
		rv = NULL;
	}
	if (rv)
		rv->init(dst, is_outtrans, info);
	return rv;
}
	
} // namespace cg

} // namespace gui

} //namespace ambulant
