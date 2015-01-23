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

#ifndef AMBULANT_GUI_GTK_GTK_FILL_H
#define AMBULANT_GUI_GTK_GTK_FILL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/none/none_gui.h"
#include "gtk_renderer.h"

namespace ambulant {

namespace gui {

namespace gtk {

class gtk_fill_renderer : public gtk_renderer<renderer_playable> {
  public:
	gtk_fill_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
	  :       gtk_renderer<renderer_playable>(context, cookie, node, evp, factory, mdp) {};

	void start(double where);
	void seek(double t) {}
	void redraw_body(const lib::rect &dirty, common::gui_window *window);
};

class gtk_background_renderer : public common::background_renderer {
  public:
	gtk_background_renderer(const common::region_info *src)
	:	common::background_renderer(src),
#if GTK_MAJOR_VERSION >= 3
		m_background_surface(NULL) {}
#else // GTK_MAJOR_VERSION < 3
		m_background_pixmap(NULL) {}
#endif // GTK_MAJOR_VERSION < 3

	void redraw(const lib::rect &dirty, common::gui_window *windo);
	void highlight(gui_window *window);
	void keep_as_background();

  private:
#if GTK_MAJOR_VERSION >= 3
	cairo_surface_t* m_background_surface;
#else // GTK_MAJOR_VERSION < 3
	GdkPixmap *m_background_pixmap;
#endif // GTK_MAJOR_VERSION < 3
};

} // namespace gtk

} // namespace gui

} // namespace ambulant
#endif  /*AMBULANT_GUI_GTK_GTK_FILL_H*/
