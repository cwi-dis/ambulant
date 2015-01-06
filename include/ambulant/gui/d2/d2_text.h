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

#ifndef AMBULANT_GUI_D2_TEXT_H
#define AMBULANT_GUI_D2_TEXT_H

#include "ambulant/config/config.h"
#include "ambulant/gui/d2/d2_renderer.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/textptr.h"

interface IDWriteFactory;
interface IDWriteTextFormat;
interface ID2D1SolidColorBrush;

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace d2 {

common::playable_factory *create_d2_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

class d2_text_renderer : public d2_renderer<renderer_playable_dsall> {
  public:
	d2_text_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp);
	~d2_text_renderer();

	void init_with_node(const lib::node *n);
	void redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget*);

	void recreate_d2d();
	void discard_d2d();
  private:
	lib::color_t m_text_color;

	static IDWriteFactory *s_write_factory;

	IDWriteTextFormat *m_text_format;
	ID2D1SolidColorBrush *m_brush;
	critical_section m_lock;
};

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_TEXT_H
