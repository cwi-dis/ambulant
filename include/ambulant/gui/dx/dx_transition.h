/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_GUI_DX_TRANSITION_H
#define AMBULANT_GUI_DX_TRANSITION_H

#include "ambulant/config/config.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/layout.h"
#include "ambulant/smil2/transition.h"

#include "ambulant/lib/logger.h"
#ifdef WITH_D2D
#error Including dx include file while building for Direct2D
#endif

namespace ambulant { namespace smil2 {
	enum blitter_type {bt_rect, bt_r1r2r3r4, bt_rectlist, bt_poly, bt_polylist, bt_fade, bt_unknown};
}}

namespace ambulant {

namespace gui {

namespace dx {

class dx_transition {
  public:
	virtual ~dx_transition(){}
	virtual void init(common::surface *dst, bool is_outtrans, const lib::transition_info *info) = 0;
	virtual void first_step() = 0;
	virtual bool next_step() = 0;
	virtual bool next_step(lib::timer::time_type pt) = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual double get_progress() const = 0;
	virtual bool is_outtrans() const = 0;
	virtual bool is_fullscreen() const = 0;
	virtual std::string get_type_str() const = 0;
	virtual std::string get_subtype_str() const = 0;
	virtual smil2::blitter_type get_blitter_type() const = 0;
	virtual smil2::transition_blitclass_rect *get_as_rect_blitter() = 0;
	virtual smil2::transition_blitclass_r1r2r3r4 *get_as_r1r2r3r4_blitter() = 0;
	virtual smil2::transition_blitclass_rectlist *get_as_rectlist_blitter() = 0;
	virtual smil2::transition_blitclass_poly *get_as_poly_blitter() = 0;
	virtual smil2::transition_blitclass_polylist *get_as_polylist_blitter() = 0;
	virtual smil2::transition_blitclass_fade *get_as_fade_blitter() = 0;
};

dx_transition *make_transition(lib::transition_type id,
	common::playable *playable, lib::timer_control *timer);

smil2::blitter_type get_transition_blitter_type(lib::transition_type id);

template<class T>
class transition_engine_adapter : public T {
  public:
	const lib::transition_info *get_info() const { return m_info;}
	double get_progress() const { return m_progress;}
	bool is_outtrans() const { return m_outtrans;}
	bool is_fullscreen() const { return m_info->m_scope == lib::scope_screen; }
  protected:
	virtual void update() {
		if(m_dst) m_dst->need_redraw();
		else lib::logger::get_logger()->show("missing transition dst");
	}
};


template<class T>
class dx_transition_engine : public dx_transition {
  public:
	dx_transition_engine(common::playable *playable, lib::timer_control *timer)
	:	/* m_playable(playable),*/ m_timer(timer) {
		m_engine = new transition_engine_adapter<T>();
	}

	~dx_transition_engine() {
		delete m_engine;
		delete m_timer;
	}

	void first_step() {
		m_engine->begin(m_timer->elapsed());
		m_engine->step(m_timer->elapsed());
		m_timer->resume();
	}

	bool next_step() {
		m_engine->step(m_timer->elapsed());
		return !m_engine->is_done();
	}

	bool next_step(lib::timer::time_type pt) {
		bool no_more = m_engine->is_done();
		m_engine->step(m_timer->elapsed(pt));
		return !no_more;
	}

	void init(common::surface *dst, bool is_outtrans, const lib::transition_info *info)
		{m_engine->init(dst, is_outtrans, info);}

	void pause() {m_timer->pause();}
	void resume() {m_timer->resume();}

	std::string get_type_str() const {return repr(m_engine->get_info()->m_type);}
	std::string get_subtype_str() const {return m_engine->get_info()->m_subtype;}

	double get_progress() const { return m_engine->get_progress();};
	bool is_outtrans() const { return m_engine->is_outtrans();}
	bool is_fullscreen() const { return m_engine->is_fullscreen(); }
	smil2::blitter_type get_blitter_type() const {
		return get_transition_blitter_type(m_engine->get_info()->m_type);
	}

	smil2::transition_blitclass_rect *get_as_rect_blitter() {
		smil2::blitter_type type = get_blitter_type();
		return (type==bt_rect)?(smil2::transition_blitclass_rect*)m_engine:0;
	}

	smil2::transition_blitclass_r1r2r3r4 *get_as_r1r2r3r4_blitter() {
		smil2::blitter_type type = get_blitter_type();
		return (type==bt_r1r2r3r4)?(smil2::transition_blitclass_r1r2r3r4*)m_engine:0;
	}

	smil2::transition_blitclass_rectlist *get_as_rectlist_blitter() {
		smil2::blitter_type type = get_blitter_type();
		return (type==bt_rectlist)?(smil2::transition_blitclass_rectlist*)m_engine:0;
	}

	smil2::transition_blitclass_poly *get_as_poly_blitter() {
		smil2::blitter_type type = get_blitter_type();
		return (type==bt_poly)?(smil2::transition_blitclass_poly*)m_engine:0;
	}

	smil2::transition_blitclass_polylist *get_as_polylist_blitter() {
		smil2::blitter_type type = get_blitter_type();
		return (type==bt_polylist)?(smil2::transition_blitclass_polylist*)m_engine:0;
	}

	smil2::transition_blitclass_fade *get_as_fade_blitter() {
		smil2::blitter_type type = get_blitter_type();
		return (type==bt_fade)?(smil2::transition_blitclass_fade*)m_engine:0;
	}

	transition_engine_adapter<T> *m_engine;
//	common::playable *m_playable;
	lib::timer_control *m_timer;
};

// Hack to make rects public
class r1r2r3r4_adapter : public smil2::transition_blitclass_r1r2r3r4 {
  public:
	lib::rect& get_old_src_rect() { return m_oldsrcrect; }
	lib::rect& get_old_dst_rect() { return m_olddstrect; }
	lib::rect& get_src_rect() { return m_newsrcrect; }
	lib::rect& get_dst_rect() { return m_newdstrect; }
	common::surface* get_dst() { return m_dst; }
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

HRGN create_rect_region(ambulant::gui::dx::dx_transition *tr);
HRGN create_rectlist_region(ambulant::gui::dx::dx_transition *tr);
HRGN create_poly_region(ambulant::gui::dx::dx_transition *tr);
HRGN create_polylist_region(ambulant::gui::dx::dx_transition *tr);
void clipto_r1r2r3r4(ambulant::gui::dx::dx_transition *tr,
	ambulant::lib::rect& src, ambulant::lib::rect& dst);

#endif // AMBULANT_GUI_DX_TRANSITION_H
