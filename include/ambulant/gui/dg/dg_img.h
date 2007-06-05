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

#ifndef AMBULANT_GUI_DG_IMG_H
#define AMBULANT_GUI_DG_IMG_H

#include "ambulant/config/config.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/dg/dg_playable.h"

namespace ambulant {

namespace gui {

namespace dg {

class image_renderer;
class dg_gui_region;

class dg_img_renderer : public dg_renderer_playable {
  public:
	dg_img_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories* factory,
		dg_playables_context *dgplayer);
	~dg_img_renderer();
	void start(double t);
	void stop();
	void seek(double t) {}
	void user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window);
 private:
	image_renderer *m_image;
	lib::rect m_msg_rect;
	dg_gui_region *m_rgn;
	common::factories* m_factory;
};

} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_IMG_H
