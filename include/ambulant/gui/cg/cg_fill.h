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

#ifndef AMBULANT_GUI_CG_CG_FILL_H
#define AMBULANT_GUI_CG_CG_FILL_H

#include "ambulant/smil2/transition.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/gui/cg/cg_renderer.h"

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cg {

class cg_fill_renderer : public cg_renderer<renderer_playable> {
  public:
	cg_fill_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp)
	:	cg_renderer<renderer_playable>(context, cookie, node, evp, fp, mdp) {};
	~cg_fill_renderer();

	void start(double where);
	void seek(double t) {}
	void redraw_body(const rect &dirty, gui_window *window);
  private:
	critical_section m_lock;
};

class cg_background_renderer : public background_renderer {
  public:
	cg_background_renderer(const common::region_info *src)
	:   background_renderer(src),
		m_bgimage(NULL) {}
	~cg_background_renderer();
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void keep_as_background();
	void highlight(common::gui_window *window);
  private:
	CGImageRef m_bgimage;
};

} // namespace cg

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_CG_CG_FILL_H
