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

#ifndef AMBULANT_GUI_DG_PLAYABLE_H
#define AMBULANT_GUI_DG_PLAYABLE_H

#include "ambulant/config/config.h"
#include "ambulant/common/renderer_impl.h"

namespace ambulant {

namespace gui {

namespace dg {

class dg_transition;

class dg_playables_context {
  public:
	virtual void stopped(common::playable *p) = 0;
	virtual void paused(common::playable *p) = 0;
	virtual void resumed(common::playable *p) = 0;
	virtual void set_intransition(common::playable *p, const lib::transition_info *info) = 0;
	virtual void start_outtransition(common::playable *p, const lib::transition_info *info) = 0; 
	virtual dg_transition *get_transition(common::playable *p) = 0;
};

class dg_renderer_playable : public common::renderer_playable {
  public:
	dg_renderer_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::gui_window *window, 
		dg_playables_context *dgplayer) 
	:	common::renderer_playable(context, cookie, node, evp), 
		m_window(window), m_dgplayer(dgplayer), m_transitioning(false) {}
	
	void set_intransition(const lib::transition_info *info) {
		//m_transitioning = true; 
		//m_dgplayer->set_intransition(this, info);
	}
	void start_outtransition(const lib::transition_info *info) {  
		//m_transitioning = true; 
		//m_dgplayer->start_outtransition(this, info);
	}
 //	gui::dx::dx_transition *get_transition() {
//		if (!m_transitioning) return NULL;
//		gui::dx::dx_transition *tr = m_dxplayer->get_transition(this);
//		if (tr) return tr;
//		m_transitioning = false;
//		m_dest->transition_done();
//		return NULL;
//	}
 protected:
	common::gui_window *m_window;
	dg_playables_context *m_dgplayer;
	bool m_transitioning;
};

} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_PLAYABLE_H
