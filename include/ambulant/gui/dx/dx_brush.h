/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
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

#ifndef AMBULANT_GUI_DX_BRUSH_H
#define AMBULANT_GUI_DX_BRUSH_H

#include "ambulant/config/config.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/colors.h"
#include "ambulant/gui/dx/dx_playable.h"

namespace ambulant {

namespace gui {

namespace dx {

class dx_brush : public dx_renderer_playable {
  public:
	dx_brush(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::gui_window *window, 
		dx_playables_context *dxplayer);
	~dx_brush();
	void start(double t);
	void stop();
	void seek(double t) {}
	void user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
  private:
	lib::color_t m_color;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_BRUSH_H
