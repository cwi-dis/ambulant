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

class dx_smiltext_run : 
	public smil2::smiltext_run
{
public:
	dx_smiltext_run(smiltext_run);
	~dx_smiltext_run();
	void dx_smiltext_run_set_attr();
	void dx_smiltext_run_get_extent();
//private:
	// left,top of final destination
	LONG m_left;
	LONG m_top;
	// text extent
	LONG m_width;
//	LONG m_height;
	// Font metrics
	LONG m_ascent;
	LONG m_descent;
	// Direct X stuff: fonts etc.
	HFONT m_dx_font;
};
typedef std::list<dx_smiltext_run> dx_smiltext_runs;
typedef dx_smiltext_runs::iterator dx_smiltext_runs_itr;

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
	void smiltext_changed(bool);
	void horizontal_layout(dx_smiltext_runs* runs, lib::rect* r);
	void vertical_layout(dx_smiltext_runs* runs, lib::rect* r);
	void dx_smiltext_render(dx_smiltext_run* run);
	text_renderer *m_text;
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
