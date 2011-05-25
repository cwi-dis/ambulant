/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_DX_WINDOW_H
#define AMBULANT_GUI_DX_WINDOW_H

#include "ambulant/config/config.h"

#include <string>

#include "ambulant/common/layout.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/common/region.h"

namespace ambulant {
namespace common {
class passive_region;
}}

typedef ambulant::common::surface_impl region; // XXX Or should it be surface??

namespace ambulant {

namespace gui {

namespace dx {

class viewport;

class dx_window : public common::gui_window {
  public:
	dx_window(const std::string& name,
		lib::size bounds,
		region *rgn,
		common::window_factory *wf,
		viewport* m_viewport);
	~dx_window();

	void need_redraw(const lib::rect& r);
	void redraw(const lib::rect& r);
	void need_redraw();
	void redraw_now();
	void need_events(bool onoff) { /* Always get them on windows */ }
	const std::string& get_name() const { return m_name;}
	region *get_region() { return m_rgn;}
	void lock_redraw();
	void unlock_redraw();

	viewport *get_viewport() { return m_viewport;}
  private:
	void _need_redraw(const lib::rect& r);

	// gui_window:
	// passive_region *m_region;
	region *m_rgn;
	std::string m_name; // for easy access
	lib::rect m_viewrc;
	common::window_factory *m_wf;
	viewport* m_viewport;

	// lock/unlock redraw
	lib::critical_section m_redraw_rect_lock;
	int m_locked;
	lib::rect m_redraw_rect;
	bool m_redraw_rect_valid;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_WINDOW_H
