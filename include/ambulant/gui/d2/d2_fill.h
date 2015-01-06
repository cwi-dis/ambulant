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

#ifndef AMBULANT_GUI_D2_D2_FILL_H
#define AMBULANT_GUI_D2_D2_FILL_H

#include "ambulant/smil2/transition.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/gui/d2/d2_renderer.h"

interface ID2D1SolidColorBrush;

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace d2 {

common::playable_factory *create_d2_fill_playable_factory(
	common::factories *factory,
	common::playable_factory_machdep *mdp);

class d2_fill_renderer : public d2_renderer<renderer_playable> {
  public:
	d2_fill_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp)
	:	d2_renderer<renderer_playable>(context, cookie, node, evp, fp, mdp),
		m_brush(NULL)
	{};
	~d2_fill_renderer();

	void start(double where);
	void seek(double t) {}

	void redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget*);

	void recreate_d2d();
	void discard_d2d();
  private:
	ID2D1SolidColorBrush *m_brush;
	critical_section m_lock;
};

class d2_background_renderer : public background_renderer, public d2_resources {
  public:
	d2_background_renderer(const common::region_info *src, common::playable_factory_machdep *mdp)
	:   background_renderer(src),
		m_d2player(dynamic_cast<d2_player*>(mdp)),
		m_brush(NULL),
		m_bgimage(NULL),
		m_mustrender(false)
	{
		assert(m_d2player);
		m_d2player->register_resources(this);
	}
	~d2_background_renderer();
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void keep_as_background();
	void highlight(common::gui_window *window);
	
	void recreate_d2d();
	void discard_d2d();
  private:
    d2_player *m_d2player;
	ID2D1SolidColorBrush *m_brush;
	void *m_bgimage;
	bool m_mustrender;
};

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_D2_FILL_H
