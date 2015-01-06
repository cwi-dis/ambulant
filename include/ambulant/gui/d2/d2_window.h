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

#ifndef AMBULANT_GUI_D2_WINDOW_H
#define AMBULANT_GUI_D2_WINDOW_H

#include "ambulant/config/config.h"

#include <string>

#include "ambulant/common/layout.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/common/region.h"

namespace ambulant {

namespace gui {

namespace d2 {

class d2_player;

class d2_window : public common::gui_window {
  public:
	d2_window(const std::string& name,
		lib::size bounds,
		common::gui_events *handler,
		d2_player *player,
		HWND hwnd);
	~d2_window();

	void need_redraw(const lib::rect& r);
	void redraw(const lib::rect& r);
	void redraw();
	void need_redraw();
	void redraw_now();
	void need_events(bool onoff) { /* Always get them on windows */ }
	const std::string& get_name() const { return m_name;}
	common::gui_events *get_gui_events() { return m_handler;}
	const lib::rect& get_rect() { return m_viewrc; }
	void lock_redraw();
	void unlock_redraw();
	d2_player*  get_d2_player() { return m_player; }

  private:
	void _need_redraw(const lib::rect& r);

	common::gui_events *m_handler;	// Ambulant window
	HWND m_hwnd;					// Windows window
	std::string m_name;				// for easy access
	lib::rect m_viewrc;
	d2_player *m_player;			// Link back to our owning player

	// lock/unlock redraw
	lib::critical_section m_redraw_rect_lock;
	int m_locked;
	lib::rect m_redraw_rect;
	bool m_redraw_rect_valid;
};

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_WINDOW_H
