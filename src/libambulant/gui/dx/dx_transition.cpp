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
 
#include "ambulant/gui/dx/dx_transition.h"
#include <map>

using namespace ambulant;
using namespace smil2;

/////////////////////////////////
// Module private definitions

struct transition_factory {
	virtual gui::dx::dx_transition *new_transition(common::playable *playable, lib::timer *timer) = 0;
};

template <class T>
struct trfact : public transition_factory {
	gui::dx::dx_transition *new_transition(common::playable *playable, lib::timer *timer) 
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
	
	add(lib::bowTieWipe, new trfact<default_class>, bt_default);
	
	add(lib::miscDiagonalWipe, new trfact<transition_engine_diagonalwipe>, bt_poly);
	
	add(lib::veeWipe, new trfact<default_class>, bt_default);
	add(lib::barnVeeWipe, new trfact<default_class>, bt_default);
	add(lib::zigZagWipe, new trfact<default_class>, bt_default);
	add(lib::barnZigZagWipe, new trfact<default_class>, bt_default);
	add(lib::irisWipe, new trfact<default_class>, bt_default);
	add(lib::arrowHeadWipe, new trfact<default_class>, bt_default);
	add(lib::pentagonWipe, new trfact<default_class>, bt_default);
	add(lib::hexagonWipe, new trfact<default_class>, bt_default);
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
	
	add(lib::waterfallWipe, new trfact<transition_engine_waterfallwipe>, bt_rectlist);
	
	add(lib::pushWipe, new trfact<default_class>, bt_default);
	add(lib::slideWipe, new trfact<default_class>, bt_default);
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
	common::playable *playable, lib::timer *timer) {
	std::map<lib::transition_type, transition_factory*>& m = 
		transition_factories_inst.trfactmap;
	std::map<lib::transition_type, transition_factory*>::iterator it = m.find(id);
	if(it == m.end())
		return m[lib::clockWipe]->new_transition(playable, timer);
	return (*it).second->new_transition(playable, timer);
}

smil2::blitter_type gui::dx::get_transition_blitter_type(lib::transition_type id) {
	std::map<lib::transition_type, smil2::blitter_type>& m = 
		transition_factories_inst.btmap;
	std::map<lib::transition_type, smil2::blitter_type>::iterator it = m.find(id);
	if(it == m.end()) return m[lib::clockWipe];
	return (*it).second;
}

/////////////////////////////////

// Hack to make rect public
class rect_adapter : public transition_blitclass_rect {
  public:
	lib::screen_rect<int>& get_rect() { return m_newrect;}
};

// Hack to make rectlist public
class rectlist_adapter : public transition_blitclass_rectlist {
  public:
	std::vector< lib::screen_rect<int> >& get_rectlist() { return m_newrectlist;}
};

// Hack to make points public
class poly_adapter : public transition_blitclass_poly {
  public:
	std::vector<lib::point>& get_points() { return m_newpolygon;}
};

/////////////////////////////////
// Create appropriate entities for blitting

HRGN create_rect_region(gui::dx::dx_transition *tr) {
	smil2::transition_blitclass_rect *p = tr->get_as_rect_blitter();
	if(!p) return 0;
	rect_adapter *dummy = (rect_adapter*)p;
	lib::screen_rect<int>& rect = dummy->get_rect();
	return CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom());
}

HRGN create_rectlist_region(gui::dx::dx_transition *tr) {
	smil2::transition_blitclass_rectlist *p = tr->get_as_rectlist_blitter();
	if(!p) return 0;
	rectlist_adapter *dummy = (rectlist_adapter*)p;
	std::vector< lib::screen_rect<int> >& v = dummy->get_rectlist();
	if(v.empty()) return 0;
	HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
	std::vector< lib::screen_rect<int> >::iterator it;
	for(it = v.begin();it!=v.end();it++) {
		lib::screen_rect<int>& rect = *it;
		HRGN next = CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom());
		CombineRgn(hrgn, hrgn, next, RGN_OR);
		DeleteObject((HGDIOBJ)next); 
	}
	return hrgn;
}

HRGN create_poly_region(gui::dx::dx_transition *tr) {
	smil2::transition_blitclass_poly *p = tr->get_as_poly_blitter();
	if(!p) return 0;
	poly_adapter *dummy = (poly_adapter*)p;
	std::vector<lib::point>& v = dummy->get_points();
	if(v.size()<3) return 0;
	POINT *ppt = new POINT[v.size()];
	std::vector<lib::point>::iterator it;
	int i = 0;
	for(it = v.begin();it!=v.end();it++,i++) 
		{ppt[i].x = (*it).x; ppt[i].y = (*it).y;}
	HRGN hrgn = CreatePolygonRgn(ppt, int(v.size()), ALTERNATE);
	delete[] ppt;	
	return hrgn;
}
