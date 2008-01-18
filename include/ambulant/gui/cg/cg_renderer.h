/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_CG_CG_RENDERER_H
#define AMBULANT_GUI_CG_CG_RENDERER_H

#include "ambulant/common/renderer_impl.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/mtsync.h"

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cg {

#if UIKIT_NOT_YET
class cocoa_transition_renderer : public ref_counted_obj {
  public:
	cocoa_transition_renderer(event_processor *evp)
	:	m_event_processor(evp),
		m_transition_dest(NULL),
		m_intransition(NULL),
		m_outtransition(NULL),
		m_trans_engine(NULL) {}
	~cocoa_transition_renderer();
	
	void set_surface(common::surface *dest);
	void start(double where);
	void stop();
	void redraw_pre(gui_window *window);
	void redraw_post(gui_window *window);
	void set_intransition(const lib::transition_info *info);
	void start_outtransition(const lib::transition_info *info);
  private:
	void transition_step();

	event_processor *m_event_processor;
	common::surface *m_transition_dest;
	const lib::transition_info *m_intransition;
	const lib::transition_info *m_outtransition;
	smil2::transition_engine *m_trans_engine;
	bool m_fullscreen;
	critical_section m_lock;
};
#endif

template <class RP_Base>
class cg_renderer : public RP_Base {
  public:
	cg_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory)
	:	RP_Base(context, cookie, node, evp, factory)
		{};
	cg_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp)
	:	RP_Base(context, cookie, node, evp)
		{};
	~cg_renderer() {
	}

	void set_surface(common::surface *dest) {
		RP_Base::set_surface(dest);
	}

	virtual void start(double where) {
		RP_Base::start(where);
	}
	
 	virtual void stop() {
		RP_Base::stop();
	}
	
    void redraw(const rect &dirty, gui_window *window) {
		redraw_body(dirty, window);
		if (RP_Base::m_erase_never) RP_Base::m_dest->keep_as_background();
	}

	void set_intransition(const lib::transition_info *info) {
	}
	
	void start_outtransition(const lib::transition_info *info) {
	}
  protected:
    virtual void redraw_body(const rect &dirty, gui_window *window) = 0;

};

} // namespace cg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_CG_CG_RENDERER_H
