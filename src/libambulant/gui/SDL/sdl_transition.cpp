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

#ifdef  WITH_SDL_IMAGE // TBD: rectlist, poly, polylist

#include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_transition.h"
//TBD #include "ambulant/gui/SDL/sdl_util.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/logger.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace sdl {

// Helper functions to setup transitions

static void
setup_transition(bool outtrans, ambulant_sdl_window *asw, SDL_Surface** old_surf, SDL_Surface** new_surf)
{
	if (outtrans) {
		*old_surf = asw->get_sdl_ambulant_window()->top_sdl_surface();
		*new_surf = asw->get_sdl_ambulant_window()->get_sdl_surface();
	} else {
		*old_surf = asw->get_sdl_ambulant_window()->top_sdl_surface();
		*new_surf = asw->get_sdl_ambulant_window()->get_sdl_surface();
	}
}
static void
finalize_transition(bool outtrans, ambulant_sdl_window *asw,  common::surface *dest)
{
	if (outtrans) {
#ifdef  TBD	// implement outtransitions
		// copy the pixels in m_tmpsurface to the on-screen surface
		SDL_Surface* dest_surface = asw->get_sdl_ambulant_window()->get_ambulant_surface();
		SDL_Surface* temp_surface = asw->get_sdl_ambulant_window()->get_ambulant_surface();
		const lib::rect &r=	 dest->get_clipped_screen_rect();
		AM_DBG logger::get_logger()->debug("finalize_transition: dest_surface=0x%x: temp_surface=0x%x (L,T,W,H)=(%d,%d,%d,%d)", dest_surface, temp_surface,r.left(),r.top(),r.width(), r.height());
		GdkGC *gc = gdk_gc_new (dest_surface);
		gdk_draw_surface(dest_surface, gc, temp_surface, r.left(),r.top(),r.left(),r.top(),r.width(), r.height());
		g_object_unref (G_OBJECT (gc));
#endif//TBD
	}
}

void
sdl_transition_blitclass_fade::update()
{

	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_fade::update(%f)", m_progress);
	ambulant_sdl_window *asw = (ambulant_sdl_window *)m_dst->get_gui_window();
	sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
	SDL_Surface* n_srf = asw->get_sdl_ambulant_window()->get_sdl_surface();
	SDL_Surface* o_srf = asw->get_sdl_ambulant_window()->top_sdl_surface();
	const rect& const_newrect =	 m_dst->get_clipped_screen_rect();
	lib::rect newrect = const_newrect;
//	newrect.translate(m_dst->get_global_topleft());
	int L = newrect.left(),  T = newrect.top(),
		W = newrect.width(), H = newrect.height();
	double d_alpha = m_outtrans ? 1.0 - m_progress : m_progress;
	Uint8 alpha = static_cast<Uint8>(round(255*d_alpha));

	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_fade::update(%f) asw=0x%x, o_srf=0x%x,n_srf0x%x alpha=%u (L,T,W,H)=(%d,%d,%d,%d)", m_progress, asw, o_srf, n_srf, alpha,L,T,W,H);
	SDL_SetSurfaceAlphaMod(n_srf, alpha);
	SDL_SetSurfaceBlendMode(n_srf, SDL_BLENDMODE_BLEND);
	SDL_Rect sdl_rect = { L, T, W, H};
	SDL_BlitSurface(n_srf, &sdl_rect, o_srf, &sdl_rect);
}

// return the difference of 2 SDL_Rects A-B for the special case that 3 sides
// coincide and either their width or their hight are equal (A > B)
// In all other cases, return A.
SDL_Rect
SDL_Rect_Substract (SDL_Rect A, SDL_Rect B)
{
	SDL_Rect rv = A;
//	printf("A=(%d,%d,%d,%d),B=(%d,%d,%d,%d)\n",A.x,A.y,A.w,A.h,B.x,B.y,B.w,B.h);
	if (SDL_HasIntersection (&A, &B) ) {
		if (A.h == B.h && A.w > B.w) {
			if (A.x == B.x) { // right edge not coincident
				rv.x += B.w;
				rv.w -= B.w;
			} else { // left edge not coincident
				rv.w -= B.w;
			}
		} else if (A.w == B.w && A.h > B.h) {
			if (A.y == B.y) { // bottom edge not coincident
				rv.y += B.h;
				rv.h -= B.h;
			} else {// top edge not coincident
				rv.h -= B.h;
			}
		}
//		printf("rv=(%d,%d,%d,%d)\n",rv.x,rv.y,rv.w,rv.h);
	}
	return rv;
}

lib::rect null_rect()
{
	return lib::rect(lib::point(0,0),lib::size(0,0));
}

// return the difference of 2 rects A-B for the special case that 3 sides
// coincide and either their width or their hight are equal (A > B)
// In all other cases, return A.
lib::rect
substract (lib::rect A, lib::rect B)
{
	lib::rect rv = A;
//	printf("A=(%d,%d,%d,%d),B=(%d,%d,%d,%d)\n",A.left(),A.top(),A.width(),A.height(),B.left(),B.top(),B.width(),B.height());
	if ((A & B) != null_rect()) {
		if (A.h == B.h && A.w > B.w) {
			if (A.x == B.x) { // right edge not coincident
				rv.x = rv.x + B.w;
				rv.w = rv.w - B.w;
			} else { // left edge not coincident
				rv.w = rv.w -  B.w;
			}
		} else if (A.w == B.w && A.h > B.h) {
			if (A.y == B.y) { // bottom edge not coincident
				rv.y = rv.y + B.h;
				rv.h = rv.h - B.h;
			} else { // top edge not coincident
				rv.h = rv.h - B.h;
			}
		}
 //		printf("rv=(%d,%d,%d,%d)\n",rv.left(),rv.top(),rv.width(),rv.height());
	}
	return rv;
}

lib::rect
get_final_rect (common::surface* dst, const lib::rect rect) {
	lib::rect rv = rect;
	if (dst != NULL) {
		rv.translate(dst->get_global_topleft());
		rv &= dst->get_clipped_screen_rect();
	}
	return rv;
}

// copy all pixels in a rectangle from a SDL_Surface* to another rectangle of a SDL_Surface
void
copy_rect (SDL_Surface* src, lib::rect src_rect,SDL_Surface* dst, lib::rect dst_rect) {
	if (src == NULL || dst == NULL) {
		return;
	}
	SDL_Surface* tmp = NULL, *active_src = src;
	
	if (src == dst && ((src_rect & dst_rect) != lib::rect(lib::point(0,0),lib::size(0,0)))) {
		// overlapping regions
		if (src_rect == dst_rect) {
			// whole region self copy, would change nothing 
			return;
		}
		tmp = SDL_ConvertSurface(src, src->format, src->flags);
		active_src = tmp;
	}
	SDL_Rect SDL_Rect_dst = SDL_Rect_from_ambulant_rect(dst_rect);
	SDL_Rect SDL_Rect_src = SDL_Rect_from_ambulant_rect(src_rect);
	SDL_BlitSurface(active_src, &SDL_Rect_src, dst, &SDL_Rect_dst);  
	if (tmp != NULL) {
		SDL_FreeSurface(tmp);
	}
}

void
sdl_transition_blitclass_rect::update()
{
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_rect::update(%f)", m_progress);
	ambulant_sdl_window* asw = (ambulant_sdl_window *)m_dst->get_gui_window();
	sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
	SDL_Surface* n_srf = asw->get_sdl_ambulant_window()->get_sdl_surface();
	SDL_Surface* o_srf = asw->get_sdl_ambulant_window()->top_sdl_surface();
	lib::rect dstrect_whole = get_final_rect (m_dst, m_dst->get_rect());
	lib::rect newrect_whole = get_final_rect(m_dst, m_newrect);
	if (m_outtrans) {
		// create a temporary copy of the 'old' pixels
		SDL_Surface* t_srf = saw->copy_sdl_surface(o_srf);
		// copy all 'new' pixels to the destination
		copy_rect (n_srf, dstrect_whole, o_srf, dstrect_whole);
		// copy the desired part of the 'new' pixels to the destination
		copy_rect(t_srf, newrect_whole, o_srf, newrect_whole);
		SDL_FreeSurface(t_srf);
	} else {
		// copy the desired part of the 'new' pixels to the destination
		copy_rect (n_srf, newrect_whole, o_srf, newrect_whole);
	}
}

void
sdl_transition_blitclass_r1r2r3r4::update()
{
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_r1r2r3r4::update(%f)", m_progress);
	ambulant_sdl_window *asw = (ambulant_sdl_window *)m_dst->get_gui_window();
	sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
	SDL_Surface* n_srf = asw->get_sdl_ambulant_window()->get_sdl_surface();
	SDL_Surface* o_srf = asw->get_sdl_ambulant_window()->top_sdl_surface();
	if (o_srf == NULL || n_srf == NULL) {
		return;
	}
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_r1r2r3r4::update() o_srf=0x%x, n_srf=0x%x.", o_srf, n_srf);
	rect oldsrcrect = get_final_rect(m_dst, m_oldsrcrect);
	rect olddstrect = get_final_rect(m_dst, m_olddstrect);
	rect newsrcrect = get_final_rect(m_dst, m_newsrcrect);
	rect newdstrect = get_final_rect(m_dst, m_newdstrect);
	AM_DBG logger::get_logger()->debug("m_oldsrcrect=(%d,%d,%d,%d) m_olddstrect=(%d,%d,%d,%d)",m_oldsrcrect.left(),m_oldsrcrect.top(),m_oldsrcrect.width(), m_oldsrcrect.height(),m_olddstrect.left(),m_olddstrect.top(),m_olddstrect.width(), m_olddstrect.height());
	AM_DBG logger::get_logger()->debug("m_newsrcrect=(%d,%d,%d,%d) m_newdstrect=(%d,%d,%d,%d)",m_newsrcrect.left(),m_newsrcrect.top(),m_newsrcrect.width(), m_newsrcrect.height(),m_newdstrect.left(),m_newdstrect.top(),m_newdstrect.width(), m_newdstrect.height());
	AM_DBG logger::get_logger()->debug("oldsrcrect=(%d,%d,%d,%d) olddstrect=(%d,%d,%d,%d)",oldsrcrect.left(),oldsrcrect.top(),oldsrcrect.width(), oldsrcrect.height(),olddstrect.left(),olddstrect.top(),olddstrect.width(), olddstrect.height());
	AM_DBG logger::get_logger()->debug("newsrcrect=(%d,%d,%d,%d newdstrect=(%d,%d,%d,%d",newsrcrect.left(),newsrcrect.top(),newsrcrect.width(), newsrcrect.height(),newdstrect.left(),newdstrect.top(),newdstrect.width(), newdstrect.height());

	if (m_outtrans) {
		// For transOut, we do the same as transIn, except that we use the complement of each rect.
		// This has the effect, that the 'movement' during transOut is the reverse of transIn.
	    lib::rect screen_rect = get_final_rect(m_dst,  m_dst->get_rect());
		// copy the old pixels out of the way
		copy_rect(o_srf, substract(screen_rect, olddstrect), o_srf, substract(screen_rect, oldsrcrect));
		// copy the new pixels in place of the old ones
		copy_rect(n_srf, substract(screen_rect, newdstrect), o_srf, substract(screen_rect, newsrcrect));
	} else {
		// copy the old pixels out of the way
		copy_rect(o_srf, oldsrcrect, o_srf, olddstrect);
		// copy the new pixels in place of the old ones
		copy_rect(n_srf, newsrcrect, o_srf, newdstrect);
	}
}

void
sdl_transition_blitclass_rectlist::update()
{
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_rectlist::update(%f)", m_progress);
	ambulant_sdl_window *asw = (ambulant_sdl_window *)m_dst->get_gui_window();
	SDL_Surface *n_srf, *o_srf;
	setup_transition(m_outtrans, asw, &o_srf, &n_srf);
	const rect& dstrect_whole = m_dst->get_clipped_screen_rect();
	int Ldst = dstrect_whole.left(), Tdst = dstrect_whole.top(),
		Wdst = dstrect_whole.width(), Hdst = dstrect_whole.height();
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_rectlist: (L,T,W,H)=(%d,%d,%d,%d)",Ldst,Tdst,Wdst,Hdst);
	std::vector< rect >::iterator newrect;
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
    	lib::rect r = *newrect;
		r.translate(m_dst->get_global_topleft());
		r &= m_dst->get_clipped_screen_rect();
		SDL_Rect sdl_new =  SDL_Rect_from_ambulant_rect(r);
		SDL_BlitSurface(n_srf, &sdl_new, o_srf, &sdl_new);
	}
#ifdef  TBD	// implement rectlist::update()
	GdkGC *gc = gdk_gc_new (o_srf);
	GdkRegion* region = gdk_region_new();
	std::vector< rect >::iterator newrect;
	for(newrect=m_newrectlist.begin(); newrect != m_newrectlist.end(); newrect++) {
		rect corner_rect = *newrect;
		corner_rect.translate(m_dst->get_global_topleft());
		corner_rect &= m_dst->get_clipped_screen_rect();
		int L = corner_rect.left(), T = corner_rect.top(),
			W = corner_rect.width(), H = corner_rect.height();
		GdkRectangle rectangle;
		rectangle.x = L;
		rectangle.y = T;
		rectangle.width = W;
		rectangle.height = H;
		AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_rectlist: (L,T,W,H)=(%d,%d,%d,%d)",L,T,W,H);
		gdk_region_union_with_rect(region, &rectangle);
	}
	gdk_gc_set_clip_region(gc, region);
	gdk_draw_surface(o_srf, gc, n_srf, Ldst, Tdst, Ldst, Tdst, Wdst, Hdst);
	g_object_unref (G_OBJECT (gc)); // clears region as well
#endif//TBD
	finalize_transition(m_outtrans, asw, m_dst);
}

void
sdl_transition_blitclass_poly::update()
{
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_poly::update(%f)", m_progress);
	ambulant_sdl_window *asw = (ambulant_sdl_window *)m_dst->get_gui_window();
	SDL_Surface *n_srf, *o_srf;
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	setup_transition(m_outtrans, asw, &o_srf, &n_srf);
	uint n_points = m_newpolygon.size();
	if (n_points <= 2) { // cannot create polygon, maybe not yet implemented
		return;
	}
#ifdef  TBD	// implement poly_update()
	GdkPoint* points = (GdkPoint*) malloc (n_points*sizeof(GdkPoint));
	uint idx = 0;
	std::vector<point>::iterator newpoint;
	for( newpoint=m_newpolygon.begin(); newpoint != m_newpolygon.end(); newpoint++) {
		point p = *newpoint + dst_global_topleft;
		points[idx].x = p.x;
		points[idx].y = p.y;
		idx++;
	}
	GdkRegion* region = gdk_region_polygon(points, n_points, GDK_WINDING_RULE);
	free(points);
	GdkGC *gc = gdk_gc_new (o_srf);
	gdk_gc_set_clip_region(gc, region);
	const rect& newrect_whole =	 m_dst->get_clipped_screen_rect();
	int Ldst= newrect_whole.left(), Tdst = newrect_whole.top(),
		Wdst = newrect_whole.width(), Hdst = newrect_whole.height();
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_poly::update(): ltwh=(%d,%d,%d,%d)",Ldst,Tdst,Wdst,Hdst);
	gdk_draw_surface(o_srf, gc, n_srf, Ldst, Tdst, Ldst, Tdst, Wdst, Hdst);
	gdk_region_destroy(region);
	g_object_unref (G_OBJECT (gc));
#endif//TBD
	finalize_transition(m_outtrans, asw, m_dst);
}

void
sdl_transition_blitclass_polylist::update()
{

	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_polylist::update(%f)", m_progress);
	ambulant_sdl_window *asw = (ambulant_sdl_window *)m_dst->get_gui_window();
	SDL_Surface *n_srf, *o_srf;
	const lib::point& dst_global_topleft = m_dst->get_global_topleft();
	setup_transition(m_outtrans, asw, &o_srf, &n_srf);
#ifdef  TBD	// implement polylist::update()
	GdkRegion* clip_region = gdk_region_new();
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_polylist: m_newpolygonlist.size()=%d", m_newpolygonlist.size());
	std::vector< std::vector<point> >::iterator partpolygon;
	for (partpolygon=m_newpolygonlist.begin(); partpolygon!=m_newpolygonlist.end(); partpolygon++) {
		uint n_points = partpolygon->size();
		if (n_points <= 2) { // cannot create polygon
			logger::get_logger()->warn("sdl_transition_blitclass_polylist: cannot create polygon, partpolygon.size()=%d", n_points);			break;
		}
		GdkPoint* points = (GdkPoint*) malloc (n_points*sizeof(GdkPoint));
		uint idx = 0;
		std::vector<point>::iterator newpoint;
		AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_polylist: partpolygon.size()=%d", partpolygon->size());
		for( newpoint=partpolygon->begin(); newpoint != partpolygon->end(); newpoint++) {
			point p = *newpoint + dst_global_topleft;
			points[idx].x = p.x;
			points[idx].y = p.y;
			idx++;
			AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_polylist: idx=%d, p=(%d,%d)", idx, p.x, p.y);
		}
		GdkRegion* next_region = gdk_region_polygon(points, n_points, GDK_WINDING_RULE);
		free(points);
		gdk_region_union (clip_region, next_region);
		gdk_region_destroy(next_region);
	}
	rect newrect_whole =  m_dst->get_rect();
	newrect_whole.translate(dst_global_topleft);
	newrect_whole &= m_dst->get_clipped_screen_rect();
	int Ldst= newrect_whole.left(), Tdst = newrect_whole.top(),
		Wdst = newrect_whole.width(), Hdst = newrect_whole.height();
	AM_DBG logger::get_logger()->debug("sdl_transition_blitclass_polylist::update(): ltwh=(%d,%d,%d,%d)",Ldst,Tdst,Wdst,Hdst);
	GdkGC *gc = gdk_gc_new (o_srf);
	gdk_gc_set_clip_region(gc, clip_region);
	gdk_draw_surface(o_srf, gc, n_srf, Ldst, Tdst, Ldst, Tdst, Wdst, Hdst);
	gdk_region_destroy(clip_region);
	g_object_unref (G_OBJECT (gc));
#endif//TBD
	finalize_transition(m_outtrans, asw, m_dst);
}

smil2::transition_engine *
sdl_transition_engine(common::surface *dst, bool is_outtrans, const transition_info *info)
{
	smil2::transition_engine *rv;
	AM_DBG logger::get_logger()->debug("sdl_transition_engine: info=0x%x info->m_type=%d", info, info->m_type);

	switch(info->m_type) {
	// Series 1: edge wipes
	case barWipe:
		rv = new sdl_transition_engine_barwipe();
		break;
	case boxWipe:
		rv = new sdl_transition_engine_boxwipe();
		break;
	case fourBoxWipe:
		rv = new sdl_transition_engine_fourboxwipe();
		break;
	case barnDoorWipe:
		rv = new sdl_transition_engine_barndoorwipe();
		break;
	case diagonalWipe:
		rv = new sdl_transition_engine_diagonalwipe();
		break;
	case miscDiagonalWipe:
		rv = new sdl_transition_engine_miscdiagonalwipe();
		break;
	case veeWipe:
		rv = new sdl_transition_engine_veewipe();
		break;
	case barnVeeWipe:
		rv = new sdl_transition_engine_barnveewipe();
		break;
	case zigZagWipe:
		rv = new sdl_transition_engine_zigzagwipe();
		break;
	case barnZigZagWipe:
		rv = new sdl_transition_engine_barnzigzagwipe();
		break;
	case bowTieWipe:
		rv = new sdl_transition_engine_bowtiewipe();
		break;
	// series 2: iris wipes
	case irisWipe:
		rv = new sdl_transition_engine_iriswipe();
		break;
	case pentagonWipe:
		rv = new sdl_transition_engine_pentagonwipe();
		break;
	case arrowHeadWipe:
		rv = new sdl_transition_engine_arrowheadwipe();
		break;
	case triangleWipe:
		rv = new sdl_transition_engine_trianglewipe();
		break;
	case hexagonWipe:
		rv = new sdl_transition_engine_hexagonwipe();
		break;
	case eyeWipe:
		rv = new sdl_transition_engine_eyewipe();
		break;
	case roundRectWipe:
		rv = new sdl_transition_engine_roundrectwipe();
		break;
	case ellipseWipe:
		rv = new sdl_transition_engine_ellipsewipe();
		break;
	case starWipe:
		rv = new sdl_transition_engine_starwipe();
		break;
	case miscShapeWipe:
		rv = new sdl_transition_engine_miscshapewipe();
		break;
	// series 3: clock-type wipes
	case clockWipe:
		rv = new sdl_transition_engine_clockwipe();
		break;
	case singleSweepWipe:
		rv = new sdl_transition_engine_singlesweepwipe();
		break;
	case doubleSweepWipe:
		rv = new sdl_transition_engine_doublesweepwipe();
		break;
	case saloonDoorWipe:
		rv = new sdl_transition_engine_saloondoorwipe();
		break;
	case windshieldWipe:
		rv = new sdl_transition_engine_windshieldwipe();
		break;
	case fanWipe:
		rv = new sdl_transition_engine_fanwipe();
		break;
	case doubleFanWipe:
		rv = new sdl_transition_engine_doublefanwipe();
		break;
	case pinWheelWipe:
		rv = new sdl_transition_engine_pinwheelwipe();
		break;
	// series 4: matrix wipe types
	case snakeWipe:
		rv = new sdl_transition_engine_snakewipe();
		break;
	case waterfallWipe:
		rv = new sdl_transition_engine_waterfallwipe();
		break;
	case spiralWipe:
		rv = new sdl_transition_engine_spiralwipe();
		break;
	case parallelSnakesWipe:
		rv = new sdl_transition_engine_parallelsnakeswipe();
		break;
	case boxSnakesWipe:
		rv = new sdl_transition_engine_boxsnakeswipe();
		break;
	// series 5: SMIL-specific types
	case pushWipe:
		rv = new sdl_transition_engine_pushwipe();
		break;
	case slideWipe:
		rv = new sdl_transition_engine_slidewipe();
		break;
	case fade:
	case audioVisualFade:
		rv = new sdl_transition_engine_fade();
		break;
	default:
		logger::get_logger()->warn(gettext("%s: transition type %s not yet implemented"),"sdl_transition_engine",repr(info->m_type).c_str());
		rv = NULL;
	}
	if (rv)
		rv->init(dst, is_outtrans, info);
	return rv;
}

} // namespace sdl

} // namespace gui

} //namespace ambulant

#endif//WITH_SDL_IMAGE
