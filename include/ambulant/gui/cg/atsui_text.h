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

#ifndef AMBULANT_GUI_CG_ATSUI_TEXT_H
#define AMBULANT_GUI_CG_ATSUI_TEXT_H

#include "ambulant/gui/cg/cg_renderer.h"
#include "ambulant/lib/mtsync.h"
#include <ApplicationServices/ApplicationServices.h>

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cg {

class atsui_text_renderer : public cg_renderer<renderer_playable_dsall> {
  public:
	atsui_text_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp);
		~atsui_text_renderer();

	void redraw_body(const rect &dirty, gui_window *window);
  private:
	UniChar *m_text_storage;
	UniCharCount m_text_storage_length;
	ATSUStyle m_style;
	ATSUTextLayout m_layout_manager;
	lib::color_t m_text_color;
	critical_section m_lock;
};

} // namespace cg

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_CG_ATSUI_TEXT_H
