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

#ifndef AMBULANT_GUI_DX_smiltext_H
#define AMBULANT_GUI_DX_smiltext_H

#include "ambulant/config/config.h"
#include <string>

#include "ambulant/smil2/smiltext.h"
#include "ambulant/gui/dx/dx_playable.h"
#include "ambulant/gui/dx/dx_text_renderer.h"

namespace ambulant {

namespace gui {

namespace dx {

class smiltext_renderer;

class dx_smiltext_renderer : 
		public dx_renderer_playable,
		public smil2::smiltext_notification
 {
  public:
	dx_smiltext_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories* factory, 
		dx_playables_context *dxplayer);
	~dx_smiltext_renderer();
	void start(double t);
	void stop();
	void seek(double t) {}
	// Callback from the engine
	void smiltext_changed();
	void user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void set_surface(common::surface *dest);
  private:
//X LPTSTR m_text_storage;
//X	DWORD m_text_size;
	std::string m_text_storage;
	text_renderer *m_text;
	LONG m_x;
	LONG m_y;
	net::datasource_factory *m_df;
// from cocoa_smiltext.h
	smil2::smiltext_engine m_engine;
	const smil2::smiltext_params& m_params;
	bool m_render_offscreen; // True if m_params does not allows rendering in-place
	lib::timer::time_type m_epoch;
	critical_section m_lock;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_TEXT_H
