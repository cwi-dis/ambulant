/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
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

#ifndef AMBULANT_GUI_NONE_AREA_H
#define AMBULANT_GUI_NONE_AREA_H

#include "ambulant/config/config.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace gui {

namespace none {

class none_area_renderer : public common::renderer_playable {
  public:
	none_area_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp)
	:   common::renderer_playable(context, cookie, node, evp, fp, mdp),
		m_rgn(NULL) {}
	~none_area_renderer();
	void start(double t);
//	void stop();
	bool stop();
	void seek(double t) {}
	bool user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window) {}
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
  private:
	lib::rect *m_rgn;
};
} // namespace none

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_NONE_AREA_H
