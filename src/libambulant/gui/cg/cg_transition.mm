// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
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

/*
 * @$Id$
 */
#include <UIKit/UIKit.h>
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

// Helper functions to setup and finalize transitions
static CGLayer*
setup_transition (bool outtrans, AmbulantView *view)
{
	CGLayer* rv = NULL;
	if (outtrans) {
		rv = [view getTransitionTmpSurface];
//		 *rv = NULL; //[view getTransitionOldSource];
//		[[view getTransitionSurface] lockFocus];
		return rv;
	} else {
		rv = [view getTransitionSurface];
//		return [view getTransitionNewSource];
//		rv = UIGraphicsGetImageFromCurrentImageContext().CGImage;
//		CALayer* cal = 
	}
	return rv;
}

static void
finalize_transition(bool outtrans, common::surface *dst)
{
/*XX
	if (outtrans) {
		cg_window *window = (cg_window *)dst->get_gui_window();
		AmbulantView *view = (AmbulantView *)window->view();
//XX	[[view getTransitionSurface] unlockFocus];

		const lib::rect& dstrect_whole = dst->get_clipped_screen_rect();
		CGRect cg_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
		[[view getTransitionNewSource] drawInRect: cg_dstrect_whole
			fromRect: cg_dstrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	}
XX*/
}

	void
cg_transition_blitclass_fade::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_fade::update(%f)", m_progress);
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	fullsrcrect.translate(m_dst->get_global_topleft()); // Translate so the right topleft pixel is in place
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextRef ctx = UIGraphicsGetCurrentContext();
	CGContextSetAlpha (ctx, m_outtrans ? 1.0 - m_progress : m_progress);
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
}

// Helper function: substract rect R2 from rect R1. The result is an array of max. 4 non-empty rects
// possibly overlapping rects. The number of rects is returned.
int substract_rect(lib::rect R1, lib::rect R2, lib::rect AR[4]) {
	int rv = 0;
	int L1 = R1.x, T1 = R1.y, L2 = R2.x, T2 = R2.y;
	unsigned int W1 = R1.w, H1 = R1.h, W2 = R2.w, H2 = R2.h;
	lib::rect r1 = lib::rect(lib::point(L1,	T1),	lib::size(W1, (unsigned int)T2-T1));
	lib::rect r2 = lib::rect(lib::point(L1,	T2+H2),	lib::size(W1, (unsigned int)T1+H1-T2-H2));
	lib::rect r3 = lib::rect(lib::point(L1,	T1),	lib::size((unsigned int)L2-L1, H1));
	lib::rect r4 = lib::rect(lib::point(L2+W2, T1),	lib::size((unsigned int)L1+W1-L2-W2, H1));
	if (r1.w > 0 && r1.h >0) 
		AR[rv++] = r1;
	if (r2.w > 0 && r2.h >0)
		AR[rv++] = r2;
	if (r3.w > 0 && r3.h >0)
		AR[rv++] = r3;
	if (r4.w > 0 && r4.h >0)
		AR[rv++] = r4;
	return rv;
}
	
void
cg_transition_blitclass_rect::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	CGLayerRef cg_layer = setup_transition(false, view);
	lib::rect newrect_whole = m_newrect;
	newrect_whole.translate(m_dst->get_global_topleft());
	newrect_whole &= m_dst->get_clipped_screen_rect();
	lib::point LT = newrect_whole.left_top();
	lib::point RB = newrect_whole.right_bottom();
	CGRect cg_clipped_rect = CGRectFromAmbulantRect(newrect_whole);
	AM_DBG NSLog(@"cg_transition_blitclass_rect::update(%f) newrect_whole=(%d,%d),(%d,%d)",m_progress,LT.x,LT.y,RB.x,RB.y);
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextRef ctx = UIGraphicsGetCurrentContext();
	if (m_outtrans) {
		lib::rect region = m_dst->get_rect();
		lib::rect clip_rects[4];
		int nr = substract_rect(region, newrect_whole, clip_rects);
		if (nr > 0) {
			CGRect cg_clip_rects[4];
			for (int i=0; i < nr; i++) {
				cg_clip_rects[i] = CGRectFromAmbulantRect(clip_rects[i]);
			}
			CGContextClipToRects(ctx,cg_clip_rects, nr);
		}
	} else {
		CGContextClipToRect (ctx, cg_clipped_rect);					
	}
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, cg_layer);
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
	CGContextRef ctx = UIGraphicsGetCurrentContext();
	CGContextSaveGState(ctx);
	if (m_outtrans) {
		dx = m_newdstrect.width() * m_progress;
//		dy = m_newdstrect.height() * m_progress;
	} else {
		dx = m_newdstrect.left() - m_newsrcrect.left();
		dy = m_newdstrect.top() - m_newsrcrect.top();
	}
	CGContextTranslateCTM (ctx, dx, dy);

	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	CGContextRestoreGState(ctx);

/*XX*
	NSImage *oldsrc = [view getTransitionOldSource];
	NSImage *newsrc = [view getTransitionNewSource];
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_r1r2r3r4::update(%f)", m_progress);
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
	NSRect cg_oldsrcrect_whole = [view NSRectForAmbulantRect: &oldsrcrect_whole];
	NSRect cg_olddstrect_whole = [view NSRectForAmbulantRect: &olddstrect_whole];
	NSRect cg_newsrcrect_whole = [view NSRectForAmbulantRect: &newsrcrect_whole];
	NSRect cg_newdstrect_whole = [view NSRectForAmbulantRect: &newdstrect_whole];
	if (m_outtrans) {
		[newsrc drawInRect: cg_olddstrect_whole
			fromRect: cg_oldsrcrect_whole
			operation: NSCompositeCopy
			fraction: 1.0f];

		[oldsrc drawInRect: cg_newdstrect_whole
			fromRect: cg_newsrcrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	} else {
		[oldsrc drawInRect: cg_olddstrect_whole
			fromRect: cg_oldsrcrect_whole
			operation: NSCompositeCopy
			fraction: 1.0f];

		[newsrc drawInRect: cg_newdstrect_whole
			fromRect: cg_newsrcrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	}
	 *XX*/
}

void
cg_transition_blitclass_rectlist::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	fullsrcrect.translate(m_dst->get_global_topleft()); // Translate so the right topleft pixel is in place
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGLayerRef cg_layer = setup_transition(false, view);
	CGContextRef ctx = CGLayerGetContext(cg_layer);
	CGContextSaveGState(ctx);
	CGRect* cg_new_rects = (CGRect*) malloc(sizeof(CGRect));
	int n_rects = 1;
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_rectlist::update(%f)", m_progress);
	std::vector< lib::rect >::iterator newrect;
	
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		lib::rect newrect_whole = *newrect;
		if (newrect_whole.empty()) {
			continue;
		}
		newrect_whole.translate(m_dst->get_global_topleft());
		newrect_whole &= m_dst->get_clipped_screen_rect();
		cg_new_rects[n_rects-1] = CGRectFromAmbulantRect(newrect_whole);
		cg_new_rects = (CGRect*) realloc(cg_new_rects, ++n_rects*sizeof(CGRect));
	}
	CGContextClipToRects(ctx, cg_new_rects, n_rects-1);
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	CGContextRestoreGState(ctx);
	free((void*) cg_new_rects);
/*XX
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	NSImage *newsrc = setup_transition_bitblit(m_outtrans, view);
	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_rectlist::update(%f)", m_progress);
	std::vector< lib::rect >::iterator newrect;
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		lib::rect newrect_whole = *newrect;
		newrect_whole.translate(m_dst->get_global_topleft());
		newrect_whole &= m_dst->get_clipped_screen_rect();
		NSRect cg_newrect_whole = [view NSRectForAmbulantRect: &newrect_whole];

		[newsrc drawInRect: cg_newrect_whole
			fromRect: cg_newrect_whole
			operation: NSCompositeSourceOver
			fraction: 1.0f];
	}
	finalize_transition_bitblit(m_outtrans, m_dst);
*XX*/
}


// Helper function: convert a point list to a CGPath
static CGPathRef
polygon2path(const lib::point& origin, std::vector<lib::point> polygon)
{
	CGMutablePathRef path = CGPathCreateMutable ();
	std::vector<lib::point>::iterator newpoint;
	bool first = true;
	for( newpoint=polygon.begin(); newpoint != polygon.end(); newpoint++) {
		lib::point p = *newpoint + origin;
		AM_DBG lib::logger::get_logger()->debug("polygon2path: point=%d, %d", p.x, p.y);
		CGPoint pc = CGPointMake(p.x, p.y);
		if (first) {
			CGPathMoveToPoint(path, NULL, p.x, p.y);
			first = false;
		} else {
			CGPathAddLineToPoint(path, NULL, p.x, p.y);
		}
	}
	CGPathCloseSubpath(path);
//	[path closePath];
	return path;
}
/*XX
// Helper function: compositing newsrc onto screen with respect
// to a path
static void
composite_path(AmbulantView *view, lib::rect dstrect_whole, NSBezierPath *path, bool outtrans)
{
	NSImage *newsrc = [view getTransitionNewSource];
	NSImage *tmpsrc = [view getTransitionTmpSurface];
	NSRect cg_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];

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
	NSRectFill(cg_dstrect_whole);

	// Now we fill draw the path on the temp bitmap, with opaque white
	[[NSColor colorWithDeviceWhite: 1.0f alpha: (float)newalpha] set];
	[path fill];

	// Next we composit the source image onto the temp bitmap, but only where
	// the temp bitmap is opaque (the path we just painted there)
	[newsrc drawInRect: cg_dstrect_whole
		fromRect: cg_dstrect_whole
		operation: NSCompositeSourceIn
		fraction: 1.0f];

	// Finally we put the opaque bits of the temp image onto the destination
	// image
	[tmpsrc unlockFocus];
	[tmpsrc drawInRect: cg_dstrect_whole
		fromRect: cg_dstrect_whole
		operation: NSCompositeSourceOver
		fraction: 1.0f];
}
XX*/
void
cg_transition_blitclass_poly::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	CGContextRef ctx = UIGraphicsGetCurrentContext();

	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_poly::update(%f)", m_progress);
	// First we create the path
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	std::vector<lib::point>  polygon;
	if (m_outtrans) {
		// reverse the order of the elements to get the correct clipping path for use with the
		// non-zero winding rule when the region is added to the path
		for (std::vector<lib::point>::reverse_iterator ri=m_newpolygon.rbegin(); ri != m_newpolygon.rend(); ri++) {
			polygon.push_back(*ri);
		}
		lib::rect region = m_dst->get_rect();
		CGRect cg_region = CGRectFromAmbulantRect(region);
		CGContextAddRect(ctx, cg_region);
	} else {
		polygon = m_newpolygon;
	}
	CGPathRef path = polygon2path(dst_global_topleft, polygon);

	// Then we composite it onto the screen
	CGContextAddPath(ctx, path);
	CGContextClip(ctx);
	lib::rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(dst_global_topleft);
	dstrect_whole &= m_dst->get_clipped_screen_rect();

	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextSaveGState(ctx);
	CGContextDrawLayerInRect(ctx, cg_fullsrcrect, [view getTransitionSurface]);
	CGContextRestoreGState(ctx);
	CFRelease(path);
}

void
cg_transition_blitclass_polylist::update()
{
	cg_window *window = (cg_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)window->view();
	CGContextRef ctx = UIGraphicsGetCurrentContext();

	AM_DBG lib::logger::get_logger()->debug("cg_transition_blitclass_poly::update(%f)", m_progress);
	// First we create the path
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
//X	NSBezierPath *path = NULL;
	std::vector< std::vector<lib::point> >::iterator partpolygon;
	AM_DBG {
	int n = 0;
	for (partpolygon=m_newpolygonlist.begin(); partpolygon!=m_newpolygonlist.end(); partpolygon++) {
		lib::logger::get_logger()->debug("partpolygon: %d", n++);
			for (std::vector<lib::point>::iterator i=(*partpolygon).begin(); i != (*partpolygon).end(); i++) {
				lib::logger::get_logger()->debug("\tx=%d, y=%d", (*i).x, (*i).y);
			}
	}
	}//AM_DBG
	for (partpolygon=m_newpolygonlist.begin(); partpolygon!=m_newpolygonlist.end(); partpolygon++) {
	//X	NSBezierPath *path = polygon2path(dst_global_topleft, m_newpolygon);
		CGPathRef path = polygon2path(dst_global_topleft, *partpolygon);
		CGContextAddPath(ctx, path);
		CFRelease(path);
	}
	CGContextClip(ctx);
	// Then we composite it onto the screen
	lib::rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(dst_global_topleft);
	dstrect_whole &= m_dst->get_clipped_screen_rect();
//X	composite_path(view, dstrect_whole, path, m_outtrans);
	lib::rect fullsrcrect = lib::rect(lib::point(0, 0), lib::size(view.bounds.size.width,view.bounds.size.height));  // Original image size
	CGRect cg_fullsrcrect = CGRectFromAmbulantRect(fullsrcrect);
	CGContextSaveGState(ctx);
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

//#endif//JNK
