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

/* 
 * @$Id$ 
 */

#include "ambulant/gui/cocoa/cocoa_transition.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/lib/logger.h"

//#define AM_DBG
#define FILL_PURPLE
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

namespace cocoa {

void
cocoa_transition_blitclass_fade::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	NSImage *newsrc = [view getTransitionNewSource];
	AM_DBG lib::logger::get_logger()->trace("cocoa_transition_blitclass_fade::update(%f)", m_progress);
	const lib::screen_rect<int> &r =  m_dst->get_rect();
	lib::screen_rect<int> dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	[newsrc drawInRect: cocoa_dstrect_whole 
		fromRect: cocoa_dstrect_whole
		operation: NSCompositeSourceOver
		fraction: m_progress];
}

void
cocoa_transition_blitclass_rect::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();

	NSImage *newsrc = [view getTransitionNewSource];
	AM_DBG lib::logger::get_logger()->trace("cocoa_transition_blitclass_rect::update(%f)", m_progress);
	lib::screen_rect<int> newrect_whole = m_newrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_newrect_whole = [view NSRectForAmbulantRect: &newrect_whole];

	[newsrc drawInRect: cocoa_newrect_whole 
		fromRect: cocoa_newrect_whole
		operation: NSCompositeSourceOver
		fraction: 1.0];
}

void
cocoa_transition_blitclass_r1r2r3r4::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();

	NSImage *oldsrc = [view getTransitionOldSource];
	NSImage *newsrc = [view getTransitionNewSource];
	AM_DBG lib::logger::get_logger()->trace("cocoa_transition_blitclass_r1r2r3r4::update(%f)", m_progress);
	lib::screen_rect<int> oldsrcrect_whole = m_oldsrcrect;
	lib::screen_rect<int> olddstrect_whole = m_olddstrect;
	lib::screen_rect<int> newsrcrect_whole = m_newsrcrect;
	lib::screen_rect<int> newdstrect_whole = m_newdstrect;
	oldsrcrect_whole.translate(m_dst->get_global_topleft());
	olddstrect_whole.translate(m_dst->get_global_topleft());
	newsrcrect_whole.translate(m_dst->get_global_topleft());
	newdstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_oldsrcrect_whole = [view NSRectForAmbulantRect: &oldsrcrect_whole];
	NSRect cocoa_olddstrect_whole = [view NSRectForAmbulantRect: &olddstrect_whole];
	NSRect cocoa_newsrcrect_whole = [view NSRectForAmbulantRect: &newsrcrect_whole];
	NSRect cocoa_newdstrect_whole = [view NSRectForAmbulantRect: &newdstrect_whole];
#ifdef FILL_PURPLE
	// Debug: fill with purple
	lib::screen_rect<int> dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	[[NSColor purpleColor] set];
	NSRectFill(cocoa_dstrect_whole);
#endif
	[oldsrc drawInRect: cocoa_olddstrect_whole 
		fromRect: cocoa_oldsrcrect_whole
		operation: NSCompositeCopy
		fraction: 1.0];

	[newsrc drawInRect: cocoa_newdstrect_whole 
		fromRect: cocoa_newsrcrect_whole
		operation: NSCompositeSourceOver
		fraction: 1.0];
}

void
cocoa_transition_blitclass_rectlist::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();

	NSImage *newsrc = [view getTransitionNewSource];
	AM_DBG lib::logger::get_logger()->trace("cocoa_transition_blitclass_rectlist::update(%f)", m_progress);
	std::vector< lib::screen_rect<int> >::iterator newrect;
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		lib::screen_rect<int> newrect_whole = *newrect;
		newrect_whole.translate(m_dst->get_global_topleft());
		NSRect cocoa_newrect_whole = [view NSRectForAmbulantRect: &newrect_whole];

		[newsrc drawInRect: cocoa_newrect_whole 
			fromRect: cocoa_newrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0];
	}
}

void
cocoa_transition_blitclass_poly::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();

	NSImage *newsrc = [view getTransitionNewSource];
	NSImage *tmpsrc = [view getTransitionTmpSurface];
	AM_DBG lib::logger::get_logger()->trace("cocoa_transition_blitclass_poly::update(%f)", m_progress);
	lib::logger::get_logger()->trace("cocoa_transition_blitclass_poly: not yet implemented");
	// First we create the path
	NSBezierPath *path = [NSBezierPath bezierPath];
	std::vector<lib::point>::iterator newpoint;
	bool first = true;
	for( newpoint=m_newpolygon.begin(); newpoint != m_newpolygon.end(); newpoint++) {
		lib::point p = *newpoint;
		/*AM_DBG*/ lib::logger::get_logger()->trace("cocoa_transition_blitclass_poly: point=%d, %d", p.x, p.y);
		NSPoint pc = NSMakePoint(p.x, p.y);
		if (first) {
			[path moveToPoint: pc];
			first = false;
		} else {
			[path lineToPoint: pc];
		}
	}
	[path closePath];
	// Next, we fill the temporary bitmap with transparent white
	[tmpsrc lockFocus];
	lib::screen_rect<int> dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	[[NSColor colorWithDeviceWhite: 1.0 alpha: 0.0] set];
	NSRectFill(cocoa_dstrect_whole);
	// Now we fill draw the path on the temp bitmap, with opaque white
	[[NSColor colorWithDeviceWhite: 1.0 alpha: 1.0] set];
	[path fill];
	// Next we composit the source image onto the temp bitmap, but only where
	// the temp bitmap is opaque (the path we just painted there)
	[newsrc drawInRect: cocoa_dstrect_whole 
		fromRect: cocoa_dstrect_whole
		operation: NSCompositeSourceIn
		fraction: 1.0];
	// Finally we put the opaque bits of the temp image onto the destination
	// image
	[tmpsrc unlockFocus];
	[tmpsrc drawInRect: cocoa_dstrect_whole 
		fromRect: cocoa_dstrect_whole
		operation: NSCompositeSourceOver
		fraction: 1.0];
	
}

void
cocoa_transition_blitclass_polylist::update()
{
	cocoa_window *window = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();

	NSImage *newsrc = [view getTransitionNewSource];
	AM_DBG lib::logger::get_logger()->trace("cocoa_transition_blitclass_polylist::update(%f)", m_progress);
	lib::logger::get_logger()->trace("cocoa_transition_blitclass_polylist: not yet implemented");
#ifdef FILL_PURPLE
	// Debug: fill with purple
	lib::screen_rect<int> dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	[[NSColor purpleColor] set];
	NSRectFill(cocoa_dstrect_whole);
#endif
}

smil2::transition_engine *
cocoa_transition_engine(common::surface *dst, bool is_outtrans, lib::transition_info *info)
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
		rv = new cocoa_transition_engine_fade();
		break;
	default:
		lib::logger::get_logger()->warn("cocoa_transition_engine: transition type %s not yet implemented",
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

