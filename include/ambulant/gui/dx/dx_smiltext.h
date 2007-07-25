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
		public smil2::smiltext_notification,
		public smil2::smiltext_layout_provider
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
	// Callbacks from the smiltext layout engine
	smil2::smiltext_metrics get_smiltext_metrics(const smil2::smiltext_run& run);
	void render_smiltext(const smil2::smiltext_run& run, const lib::rect& r, unsigned int word_spacing);
	// Callbacks from event procesor
	void user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void set_surface(common::surface *dest);
  private:
	// internal helper functions
	HGDIOBJ _dx_smiltext_set_font(const smil2::smiltext_run run, HDC hdc);
	// DirectX interfacing
	void _dx_smiltext_get_ddsurf(common::gui_window *window);
	// instance variables
	net::datasource_factory *m_df;
	smil2::smiltext_layout_engine m_layout_engine;
	critical_section m_lock;
	// Windows GDI data
	ambulant::lib::size m_size;
	viewport* m_viewport;
	HDC m_hdc;
	HFONT m_font;
	IDirectDrawSurface* m_ddsurf;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_TEXT_H
