// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/logger.h"

#include <map>

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using ambulant::lib::win32::win_report_last_error;

using namespace ambulant;
using namespace smil2;


/////////////////////////////////
// Module private definitions

struct transition_factory {
	virtual gui::dx::dx_transition *new_transition(common::playable *playable, lib::timer_control *timer) = 0;
};

template <class T>
struct trfact : public transition_factory {
	gui::dx::dx_transition *new_transition(common::playable *playable, lib::timer_control *timer)
	{ return new gui::dx::dx_transition_engine<T>(playable, timer);}
};

// create a map of factories
struct transition_factories {
	transition_factories() {

	// Default values
	typedef transition_engine_fourboxwipe default_class;
	blitter_type bt_default = bt_rectlist;

	///////////////////////////////////////////////////////////////
	// Entries: transition type, transition class, blitter type
	///////////////////////////////////////////////////////////////

	add(lib::barWipe, new trfact<transition_engine_barwipe>, bt_rect);
	add(lib::boxWipe, new trfact<transition_engine_boxwipe>, bt_rect);
	add(lib::fourBoxWipe, new trfact<transition_engine_fourboxwipe>, bt_rectlist);
	add(lib::barnDoorWipe, new trfact<transition_engine_barndoorwipe>, bt_rect);

	add(lib::bowTieWipe, new trfact<transition_engine_bowtiewipe>, bt_polylist);

	add(lib::diagonalWipe, new trfact<transition_engine_diagonalwipe>, bt_poly);
	add(lib::miscDiagonalWipe, new trfact<transition_engine_diagonalwipe>, bt_poly);

	add(lib::veeWipe, new trfact<default_class>, bt_default);
	add(lib::barnVeeWipe, new trfact<default_class>, bt_default);
	add(lib::zigZagWipe, new trfact<default_class>, bt_default);
	add(lib::barnZigZagWipe, new trfact<default_class>, bt_default);
	add(lib::irisWipe, new trfact<transition_engine_iriswipe>, bt_poly);
	add(lib::triangleWipe, new trfact<transition_engine_trianglewipe>, bt_poly);
	add(lib::arrowHeadWipe, new trfact<transition_engine_arrowheadwipe>, bt_poly);
	add(lib::pentagonWipe, new trfact<transition_engine_pentagonwipe>, bt_poly);
	add(lib::hexagonWipe, new trfact<transition_engine_hexagonwipe>, bt_poly);
	add(lib::ellipseWipe, new trfact<default_class>, bt_default);
	add(lib::eyeWipe, new trfact<default_class>, bt_default);
	add(lib::roundRectWipe, new trfact<default_class>, bt_default);
	add(lib::starWipe, new trfact<default_class>, bt_default);
	add(lib::miscShapeWipe, new trfact<default_class>, bt_default);

	add(lib::clockWipe, new trfact<transition_engine_clockwipe>, bt_poly);

	add(lib::pinWheelWipe, new trfact<default_class>, bt_default);
	add(lib::singleSweepWipe, new trfact<default_class>, bt_default);
	add(lib::fanWipe, new trfact<default_class>, bt_default);
	add(lib::doubleFanWipe, new trfact<default_class>, bt_default);
	add(lib::doubleSweepWipe, new trfact<default_class>, bt_default);
	add(lib::saloonDoorWipe, new trfact<default_class>, bt_default);
	add(lib::windshieldWipe, new trfact<default_class>, bt_default);

	add(lib::snakeWipe, new trfact<transition_engine_snakewipe>, bt_rectlist);

	add(lib::spiralWipe, new trfact<default_class>, bt_default);
	add(lib::parallelSnakesWipe, new trfact<default_class>, bt_default);
	add(lib::boxSnakesWipe, new trfact<default_class>, bt_default);
	add(lib::waterfallWipe, new trfact<transition_engine_waterfallwipe>, bt_rectlist);

	add(lib::pushWipe, new trfact<transition_engine_pushwipe>, bt_r1r2r3r4);
	add(lib::slideWipe, new trfact<transition_engine_slidewipe>, bt_r1r2r3r4);

	add(lib::audioVisualFade, new trfact<transition_engine_fade>, bt_fade);
	add(lib::fade, new trfact<transition_engine_fade>, bt_fade);
	}

	~transition_factories() {
		std::map<lib::transition_type, transition_factory*>::iterator it;
		for(it=trfactmap.begin();it!=trfactmap.end();it++)
			delete (*it).second;
	}

	std::map<lib::transition_type, transition_factory*> trfactmap;
	std::map<lib::transition_type, smil2::blitter_type> btmap;

	void add(lib::transition_type id, transition_factory* f, blitter_type bt) {
		trfactmap[id] = f;
		btmap[id] = bt;
	}

};

static transition_factories transition_factories_inst;

/////////////////////////////////
// Public interface

gui::dx::dx_transition *gui::dx::make_transition(lib::transition_type id,
	common::playable *playable, lib::timer_control *timer) {
	std::map<lib::transition_type, transition_factory*>& m =
		transition_factories_inst.trfactmap;
	std::map<lib::transition_type, transition_factory*>::iterator it = m.find(id);
	if(it == m.end()) {
		lib::logger::get_logger()->trace("Missing transition factory for type %d. Returning default", id);
		return m[lib::clockWipe]->new_transition(playable, timer);
	}
	return (*it).second->new_transition(playable, timer);
}

smil2::blitter_type gui::dx::get_transition_blitter_type(lib::transition_type id) {
	std::map<lib::transition_type, smil2::blitter_type>& m =
		transition_factories_inst.btmap;
	std::map<lib::transition_type, smil2::blitter_type>::iterator it = m.find(id);
	if(it == m.end()) {
		return m[lib::clockWipe];
	}
	return (*it).second;
}

/////////////////////////////////
// Private

// Hack to make rect public
class rect_adapter : public transition_blitclass_rect {
  public:
	lib::rect& get_rect() { return m_newrect;}
};

// Hack to make rectlist public
class rectlist_adapter : public transition_blitclass_rectlist {
  public:
	std::vector< lib::rect >& get_rectlist() { return m_newrectlist;}
};

// Hack to make points public
class poly_adapter : public transition_blitclass_poly {
  public:
	std::vector<lib::point>& get_points() { return m_newpolygon;}
};

// Hack to make points lists public
class polylist_adapter : public transition_blitclass_polylist {
  public:
	std::vector< std::vector<lib::point> >& get_point_lists() { return m_newpolygonlist;}
};


// Empty region used on error conditions
inline HRGN empty_region() {return CreateRectRgn(0, 0, 0, 0);}

/////////////////////////////////
// Public interface
// Create appropriate entities for blitting


HRGN create_rect_region(gui::dx::dx_transition *tr) {
	smil2::transition_blitclass_rect *p = tr->get_as_rect_blitter();
	assert(p);
	rect_adapter *dummy = (rect_adapter*)p;
	lib::rect& rect = dummy->get_rect();
	HRGN hrgn = CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom());
	if(!hrgn) {
		win_report_last_error("CreateRectRgn()");
		return empty_region();
	}
	return hrgn;
}

HRGN create_rectlist_region(gui::dx::dx_transition *tr) {
	smil2::transition_blitclass_rectlist *p = tr->get_as_rectlist_blitter();
	assert(p);
	rectlist_adapter *dummy = (rectlist_adapter*)p;
	std::vector< lib::rect >& v = dummy->get_rectlist();
	if(v.empty()) {
		lib::logger::get_logger()->trace("%s: Returning empty region. Rectlist is empty!",
			tr->get_type_str().c_str());
		return empty_region();
	}
	HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
	std::vector< lib::rect >::iterator it;
	for(it = v.begin();it!=v.end();it++) {
		lib::rect& rect = *it;
		HRGN next = CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom());
		CombineRgn(hrgn, hrgn, next, RGN_OR);
		DeleteObject((HGDIOBJ)next);
		if(!hrgn) {
			win_report_last_error("CombineRgn()");
			lib::logger::get_logger()->trace("%s: Returning empty region due to a fault",
				tr->get_type_str().c_str());
			return empty_region();
		}
	}
	AM_DBG {
		RECT rc;
		GetRgnBox(hrgn, &rc);
		lib::logger::get_logger()->debug("RegionBox: %d %d %d %d", rc.left, rc.top, rc.right, rc.bottom);
		if(rc.left == rc.right || rc.top == rc.bottom)
			lib::logger::get_logger()->debug("Region is empty");
	}
	return hrgn;
}

HRGN create_poly_region(gui::dx::dx_transition *tr) {
	smil2::transition_blitclass_poly *p = tr->get_as_poly_blitter();
	assert(p);
	poly_adapter *dummy = (poly_adapter*)p;
	std::vector<lib::point>& v = dummy->get_points();
	if(v.size()<3) return 0;
	POINT *ppt = new POINT[v.size()];
	std::vector<lib::point>::iterator it;
	int i = 0;
	for(it = v.begin();it!=v.end();it++,i++) {
		ppt[i].x = (*it).x;
		ppt[i].y = (*it).y;
	}
	HRGN hrgn = CreatePolygonRgn(ppt, int(v.size()), ALTERNATE);
	delete[] ppt;
	if(!hrgn) {
		win_report_last_error("CreatePolygonRgn()");
		return empty_region();
	}
	return hrgn;
}

HRGN create_polylist_region(gui::dx::dx_transition *tr) {
	smil2::transition_blitclass_polylist *p = tr->get_as_polylist_blitter();
	assert(p);
	polylist_adapter *dummy = (polylist_adapter*)p;
	std::vector< std::vector<lib::point> > vv = dummy->get_point_lists();
	HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
	std::vector< std::vector<lib::point> >::iterator it;
	for(it = vv.begin();it!=vv.end();it++) {
		std::vector<lib::point>& v = *it; int i = 0;
		POINT *ppt = new POINT[v.size()];
		for(std::vector<lib::point>::iterator pit = v.begin();pit!=v.end();pit++,i++)
			{ppt[i].x = (*pit).x; ppt[i].y = (*pit).y;}
		HRGN next = CreatePolygonRgn(ppt, int(v.size()), ALTERNATE);
		delete[] ppt;
		if(!next) {
			win_report_last_error("CreatePolygonRgn()");
			next = empty_region();
		}
		CombineRgn(hrgn, hrgn, next, RGN_OR);
		DeleteObject((HGDIOBJ)next);
		if(!hrgn) {
			win_report_last_error("CombineRgn()");
			return empty_region();
		}
	}
	return hrgn;
}

void clipto_r1r2r3r4(gui::dx::dx_transition *tr, lib::rect& src, lib::rect& dst) {
	smil2::transition_blitclass_r1r2r3r4 *p = tr->get_as_r1r2r3r4_blitter();
	assert(p);
	gui::dx::r1r2r3r4_adapter *dummy = (gui::dx::r1r2r3r4_adapter*)p;
	lib::rect& r3 = dummy->get_src_rect(); r3.translate(src.left_top());
	lib::rect& r4 = dummy->get_dst_rect(); r4.translate(dst.left_top());
	src &= r3;
	dst &= r4;
	AM_DBG lib::logger::get_logger()->debug("%s -> %s (%s -> %s)",
		repr(r3).c_str(), repr(r4).c_str(), repr(src).c_str(), ::repr(dst).c_str());
}
