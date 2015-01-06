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

/*
 * $Id$
 */

/* from: renderer.h,v 1.1 2003/09/15 10:36:17 jack Exp */

#ifndef AMBULANT_GUI_GTK_GTK_IMAGE_RENDERER_H
#define AMBULANT_GUI_GTK_GTK_IMAGE_RENDERER_H


#include "ambulant/common/factory.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/gui/none/none_gui.h"

#include "ambulant/gui/gtk/gtk_renderer.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

namespace ambulant {

namespace gui {

using namespace common;
using namespace lib;
using namespace net;

namespace gtk {

class gtk_image_renderer : public gtk_renderer<renderer_playable_dsall> {

  public:
	gtk_image_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const node *node,
		event_processor *const evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
	:	gtk_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
		m_image(NULL),
		m_image_loaded(false)
		{};
	~gtk_image_renderer();

	void redraw_body(const rect &dirty, gui_window *window);

  private:
	GdkPixbuf *m_image;
	bool m_image_loaded;
	critical_section m_lock;
};

} // namespace gtk

} // namespace gui

} // namespace ambulant

#endif/*AMBULANT_GUI_GTK_GTK_IMAGE_RENDERER_H*/
