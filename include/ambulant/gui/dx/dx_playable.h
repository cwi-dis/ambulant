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

#ifndef AMBULANT_GUI_DX_PLAYABLE_H
#define AMBULANT_GUI_DX_PLAYABLE_H

#include "ambulant/config/config.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/dx/html_bridge.h"
#ifdef WITH_D2D
#error Including dx include file while building for Direct2D
#endif

namespace ambulant {

namespace gui {

namespace dx {

class dx_transition;

class AMBULANTAPI dx_playables_context : public common::playable_factory_machdep {
  public:
	virtual void stopped(common::playable *p) = 0;
	virtual void paused(common::playable *p) = 0;
	virtual void resumed(common::playable *p) = 0;
	virtual void set_intransition(common::playable *p, const lib::transition_info *info) = 0;
	virtual void start_outtransition(common::playable *p, const lib::transition_info *info) = 0;
	virtual dx_transition *get_transition(common::playable *p) = 0;
	virtual html_browser_factory *get_html_browser_factory() { return NULL; }
};

class AMBULANTAPI dx_renderer_playable : public common::renderer_playable {
  public:
	dx_renderer_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *fp,
		dx_playables_context *dxplayer)
	:	common::renderer_playable(context, cookie, node, evp, fp, dxplayer),
		m_dxplayer(dxplayer), m_transitioning(false) {}

	void set_intransition(const lib::transition_info *info) {
		m_transitioning = true;
		m_dxplayer->set_intransition(this, info);
	}
	void start_outtransition(const lib::transition_info *info) {
		m_transitioning = true;
		m_dxplayer->start_outtransition(this, info);
	}
	gui::dx::dx_transition *get_transition() {
		if (!m_transitioning) return NULL;
		gui::dx::dx_transition *tr = m_dxplayer->get_transition(this);
		if (tr) return tr;
		m_transitioning = false;
//		m_dxplayer->stopped(this);
//		m_dest->transition_done();
		return NULL;
	}
  protected:
	dx_playables_context *m_dxplayer;
	bool m_transitioning;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_PLAYABLE_H
