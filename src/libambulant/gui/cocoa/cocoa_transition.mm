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

#include "ambulant/gui/cocoa/cocoa_transition.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/lib/logger.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

namespace cocoa {

// Helper functions to setup and finalize transitions
static NSImage *
setup_transition_bitblit(bool outtrans, AmbulantView *view)
{
	if (outtrans) {
		NSImage *rv = [view getTransitionOldSource];
        [rv setFlipped: NO];
		[[view getTransitionSurface] lockFocus];
		return rv;
	} else {
		return [view getTransitionNewSource];
	}
}

static void
finalize_transition_bitblit(bool outtrans, common::surface *dst)
{
	if (outtrans) {
		cocoa_window *window = (cocoa_window *)dst->get_gui_window();
		AmbulantView *view = (AmbulantView *)window->view();
		[[view getTransitionSurface] unlockFocus];

		const lib::rect& dstrect_whole = dst->get_clipped_screen_rect();
		NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
		[[view getTransitionNewSource] drawInRect: cocoa_dstrect_whole
			fromRect: cocoa_dstrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	}
}


void
cocoa_transition_blitclass_fade::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	NSImage *newsrc = setup_transition_bitblit(m_outtrans, view);
	AM_DBG lib::logger::get_logger()->debug("cocoa_transition_blitclass_fade::update(%f)", m_progress);
	const lib::rect& dstrect_whole = m_dst->get_clipped_screen_rect();
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	[newsrc drawInRect: cocoa_dstrect_whole
		fromRect: cocoa_dstrect_whole
		operation: NSCompositeSourceOver
		fraction: (float)m_progress];
	finalize_transition_bitblit(m_outtrans, m_dst);
}

void
cocoa_transition_blitclass_rect::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	NSImage *newsrc = setup_transition_bitblit(m_outtrans, view);
	AM_DBG lib::logger::get_logger()->debug("cocoa_transition_blitclass_rect::update(%f)", m_progress);
	lib::rect newrect_whole = m_newrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	newrect_whole &= m_dst->get_clipped_screen_rect();
	NSRect cocoa_newrect_whole = [view NSRectForAmbulantRect: &newrect_whole];

	[newsrc drawInRect: cocoa_newrect_whole
		fromRect: cocoa_newrect_whole
		operation: NSCompositeSourceOver
		fraction: 1.0f];
	finalize_transition_bitblit(m_outtrans, m_dst);
}

void
cocoa_transition_blitclass_r1r2r3r4::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();

	NSImage *oldsrc = [view getTransitionOldSource];
	NSImage *newsrc = [view getTransitionNewSource];
	AM_DBG lib::logger::get_logger()->debug("cocoa_transition_blitclass_r1r2r3r4::update(%f)", m_progress);
	lib::rect oldsrcrect_whole = m_oldsrcrect;
	lib::rect olddstrect_whole = m_olddstrect;
	lib::rect newsrcrect_whole = m_newsrcrect;
	lib::rect newdstrect_whole = m_newdstrect;
	oldsrcrect_whole.translate(m_dst->get_global_topleft());
	oldsrcrect_whole &= m_dst->get_clipped_screen_rect();
	olddstrect_whole.translate(m_dst->get_global_topleft());
	olddstrect_whole &= m_dst->get_clipped_screen_rect();
	newsrcrect_whole.translate(m_dst->get_global_topleft());
	newsrcrect_whole &= m_dst->get_clipped_screen_rect();
	newdstrect_whole.translate(m_dst->get_global_topleft());
	newdstrect_whole &= m_dst->get_clipped_screen_rect();
	NSRect cocoa_oldsrcrect_whole = [view NSRectForAmbulantRect: &oldsrcrect_whole];
	NSRect cocoa_olddstrect_whole = [view NSRectForAmbulantRect: &olddstrect_whole];
	NSRect cocoa_newsrcrect_whole = [view NSRectForAmbulantRect: &newsrcrect_whole];
	NSRect cocoa_newdstrect_whole = [view NSRectForAmbulantRect: &newdstrect_whole];
	if (m_outtrans) {
		[newsrc drawInRect: cocoa_olddstrect_whole
			fromRect: cocoa_oldsrcrect_whole
			operation: NSCompositeCopy
			fraction: 1.0f];

		[oldsrc drawInRect: cocoa_newdstrect_whole
			fromRect: cocoa_newsrcrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	} else {
		[oldsrc drawInRect: cocoa_olddstrect_whole
			fromRect: cocoa_oldsrcrect_whole
			operation: NSCompositeCopy
			fraction: 1.0f];

		[newsrc drawInRect: cocoa_newdstrect_whole
			fromRect: cocoa_newsrcrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	}
}

void
cocoa_transition_blitclass_rectlist::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	NSImage *newsrc = setup_transition_bitblit(m_outtrans, view);
	AM_DBG lib::logger::get_logger()->debug("cocoa_transition_blitclass_rectlist::update(%f)", m_progress);
	std::vector< lib::rect >::iterator newrect;
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		lib::rect newrect_whole = *newrect;
		newrect_whole.translate(m_dst->get_global_topleft());
		newrect_whole &= m_dst->get_clipped_screen_rect();
		NSRect cocoa_newrect_whole = [view NSRectForAmbulantRect: &newrect_whole];

		[newsrc drawInRect: cocoa_newrect_whole
			fromRect: cocoa_newrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	}
	finalize_transition_bitblit(m_outtrans, m_dst);
}

// Helper function: convert a point list to an NSBezierPath
static NSBezierPath *
polygon2path(const lib::point& origin, std::vector<lib::point> polygon)
{
	NSBezierPath *path = [NSBezierPath bezierPath];
	std::vector<lib::point>::iterator newpoint;
	bool first = true;
	for( newpoint=polygon.begin(); newpoint != polygon.end(); newpoint++) {
		lib::point p = *newpoint + origin;
		AM_DBG lib::logger::get_logger()->debug("polygon2path: point=%d, %d", p.x, p.y);
		NSPoint pc = NSMakePoint(p.x, p.y);
		if (first) {
			[path moveToPoint: pc];
			first = false;
		} else {
			[path lineToPoint: pc];
		}
	}
	[path closePath];
	return path;
}

// Helper function: compositing newsrc onto screen with respect
// to a path
static void
composite_path(AmbulantView *view, lib::rect dstrect_whole, NSBezierPath *path, bool outtrans)
{
	NSImage *newsrc = [view getTransitionNewSource];
	NSImage *tmpsrc = [view getTransitionTmpSurface];
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];

	// First, we fill the temporary bitmap with transparent white
	float oldalpha, newalpha;
	if (outtrans) {
		oldalpha = 1.0F;
		newalpha = 0.0F;
	} else {
		oldalpha = 0.0F;
		newalpha = 1.0F;
	}
	[tmpsrc lockFocus];
	[[NSColor colorWithDeviceWhite: 1.0f alpha: (float)oldalpha] set];
	NSRectFill(cocoa_dstrect_whole);

	// Now we fill draw the path on the temp bitmap, with opaque white
	[[NSColor colorWithDeviceWhite: 1.0f alpha: (float)newalpha] set];
	[path fill];

	// Next we composit the source image onto the temp bitmap, but only where
	// the temp bitmap is opaque (the path we just painted there)
	[newsrc drawInRect: cocoa_dstrect_whole
		fromRect: cocoa_dstrect_whole
		operation: NSCompositeSourceIn
		fraction: 1.0f];

	// Finally we put the opaque bits of the temp image onto the destination
	// image
	[tmpsrc unlockFocus];
	[tmpsrc drawInRect: cocoa_dstrect_whole
		fromRect: cocoa_dstrect_whole
		operation: NSCompositeSourceOver
		fraction: 1.0f];
}

void
cocoa_transition_blitclass_poly::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();

	AM_DBG lib::logger::get_logger()->debug("cocoa_transition_blitclass_poly::update(%f)", m_progress);
	// First we create the path
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	NSBezierPath *path = polygon2path(dst_global_topleft, m_newpolygon);

	// Then we composite it onto the screen
	lib::rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(dst_global_topleft);
	dstrect_whole &= m_dst->get_clipped_screen_rect();
	composite_path(view, dstrect_whole, path, m_outtrans);
}

void
cocoa_transition_blitclass_polylist::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();

	AM_DBG lib::logger::get_logger()->debug("cocoa_transition_blitclass_poly::update(%f)", m_progress);
	// First we create the path
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	NSBezierPath *path = NULL;
	std::vector< std::vector<lib::point> >::iterator partpolygon;
	for (partpolygon=m_newpolygonlist.begin(); partpolygon!=m_newpolygonlist.end(); partpolygon++) {
		NSBezierPath *part_path = polygon2path(dst_global_topleft, *partpolygon);
		if (path == NULL)
			path = part_path;
		else
			[path appendBezierPath: part_path];
	}

	// Then we composite it onto the screen
	lib::rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(dst_global_topleft);
	dstrect_whole &= m_dst->get_clipped_screen_rect();
	composite_path(view, dstrect_whole, path, m_outtrans);
}

smil2::transition_engine *
cocoa_transition_engine(common::surface *dst, bool is_outtrans, const lib::transition_info *info)
{
	smil2::transition_engine *rv;

	switch(info->m_type) {
	// Series 1: edge wipes
	case lib::barWipe:
		rv = new cocoa_transition_engine_barwipe();
		break;
	case lib::boxWipe:
		rv = new cocoa_transition_engine_boxwipe();
		break;
	case lib::fourBoxWipe:
		rv = new cocoa_transition_engine_fourboxwipe();
		break;
	case lib::barnDoorWipe:
		rv = new cocoa_transition_engine_barndoorwipe();
		break;
	case lib::diagonalWipe:
		rv = new cocoa_transition_engine_diagonalwipe();
		break;
	case lib::miscDiagonalWipe:
		rv = new cocoa_transition_engine_miscdiagonalwipe();
		break;
	case lib::veeWipe:
		rv = new cocoa_transition_engine_veewipe();
		break;
	case lib::barnVeeWipe:
		rv = new cocoa_transition_engine_barnveewipe();
		break;
	case lib::zigZagWipe:
		rv = new cocoa_transition_engine_zigzagwipe();
		break;
	case lib::barnZigZagWipe:
		rv = new cocoa_transition_engine_barnzigzagwipe();
		break;
	case lib::bowTieWipe:
		rv = new cocoa_transition_engine_bowtiewipe();
		break;
	// series 2: iris wipes
	case lib::irisWipe:
		rv = new cocoa_transition_engine_iriswipe();
		break;
	case lib::pentagonWipe:
		rv = new cocoa_transition_engine_pentagonwipe();
		break;
	case lib::arrowHeadWipe:
		rv = new cocoa_transition_engine_arrowheadwipe();
		break;
	case lib::triangleWipe:
		rv = new cocoa_transition_engine_trianglewipe();
		break;
	case lib::hexagonWipe:
		rv = new cocoa_transition_engine_hexagonwipe();
		break;
	case lib::eyeWipe:
		rv = new cocoa_transition_engine_eyewipe();
		break;
	case lib::roundRectWipe:
		rv = new cocoa_transition_engine_roundrectwipe();
		break;
	case lib::ellipseWipe:
		rv = new cocoa_transition_engine_ellipsewipe();
		break;
	case lib::starWipe:
		rv = new cocoa_transition_engine_starwipe();
		break;
	case lib::miscShapeWipe:
		rv = new cocoa_transition_engine_miscshapewipe();
		break;
	// series 3: clock-type wipes
	case lib::clockWipe:
		rv = new cocoa_transition_engine_clockwipe();
		break;
	case lib::singleSweepWipe:
		rv = new cocoa_transition_engine_singlesweepwipe();
		break;
	case lib::doubleSweepWipe:
		rv = new cocoa_transition_engine_doublesweepwipe();
		break;
	case lib::saloonDoorWipe:
		rv = new cocoa_transition_engine_saloondoorwipe();
		break;
	case lib::windshieldWipe:
		rv = new cocoa_transition_engine_windshieldwipe();
		break;
	case lib::fanWipe:
		rv = new cocoa_transition_engine_fanwipe();
		break;
	case lib::doubleFanWipe:
		rv = new cocoa_transition_engine_doublefanwipe();
		break;
	case lib::pinWheelWipe:
		rv = new cocoa_transition_engine_pinwheelwipe();
		break;
	// series 4: matrix wipe types
	case lib::snakeWipe:
		rv = new cocoa_transition_engine_snakewipe();
		break;
	case lib::waterfallWipe:
		rv = new cocoa_transition_engine_waterfallwipe();
		break;
	case lib::spiralWipe:
		rv = new cocoa_transition_engine_spiralwipe();
		break;
	case lib::parallelSnakesWipe:
		rv = new cocoa_transition_engine_parallelsnakeswipe();
		break;
	case lib::boxSnakesWipe:
		rv = new cocoa_transition_engine_boxsnakeswipe();
		break;
	// series 5: SMIL-specific types
	case lib::pushWipe:
		rv = new cocoa_transition_engine_pushwipe();
		break;
	case lib::slideWipe:
		rv = new cocoa_transition_engine_slidewipe();
		break;
	case lib::fade:
	case lib::audioVisualFade:
		rv = new cocoa_transition_engine_fade();
		break;
	default:
		lib::logger::get_logger()->trace("cocoa_transition_engine: transition type %s not yet implemented",
			repr(info->m_type).c_str());
		rv = NULL;
	}
	if (rv)
		rv->init(dst, is_outtrans, info);
	return rv;
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

