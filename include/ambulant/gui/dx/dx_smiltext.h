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

common::playable_factory *create_dx_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

class dx_smiltext_renderer :
		public dx_renderer_playable,
		public smil2::smiltext_notification,
		public smil2::smiltext_layout_provider
/*
	Operation of dx_smiltext_renderer.

	First, called from redraw(), _dx_smiltext_get_ddsurf() copies the
	background area in m_region_dds. Then redraw() calls m_layout_engine->redraw().
	From this, for each string (word) in a smiltext_run, if textBackgroundColor/
	mediaBackgroundOpacity is set, an aditional textbg_dds is created and filled
	with textBackgroundColor. Similarly, if textColor/mediaOpacity is set, an
	additional text_dds is created. Text is drawn in both text_dds/textbg dds.
	Then first textbg_dds is blended with m_region_dds, next textbg_dds is blended
	with m_region_dds, in both cases only using the text[background]Color with
	the desired opacity for the blend, keeping the original pixels in m_region_dds
	when not blending them.
	Finally, after all smiltext_run items are processed in this way, redraw() blits
	m_region_dds to the viewport area (screen).
*/
{
  public:
	dx_smiltext_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories* factory,
		common::playable_factory_machdep *dxplayer);
	~dx_smiltext_renderer();
	void start(double t);
	//void stop();
	bool stop();
	void seek(double t) {}
	// Callback from the engine
	void smiltext_changed();
	void marker_seen(const char *name);
	// Callbacks from the smiltext layout engine
	smil2::smiltext_metrics get_smiltext_metrics(const smil2::smiltext_run& run);
	const lib::rect& get_rect();
	void render_smiltext(const smil2::smiltext_run& run, const lib::rect& r);
	void smiltext_stopped();
	// Callbacks from event procesor
	bool user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void set_surface(common::surface *dest);
  private:
	// internal helper functions
	HGDIOBJ _dx_smiltext_set_font(const smil2::smiltext_run run, HDC hdc, HFONT* font);
	// DirectX interfacing
	void _dx_smiltext_get_ddsurf(common::gui_window *window);
	// instance variables
	net::datasource_factory *m_df;
	common::playable_notification* m_context;
	smil2::smiltext_layout_engine m_layout_engine;
	lib::critical_section m_lock;
	// Windows GDI data
	ambulant::lib::size m_size;
	viewport* m_viewport;
	double m_bgopacity;
	HDC m_hdc;
	IDirectDrawSurface* m_region_dds;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_TEXT_H
