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

#ifndef AMBULANT_GUI_GTK_GTK_UTIL_H
#define AMBULANT_GUI_GTK_GTK_UTIL_H

#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_renderer.h"

#include <gtk/gtk.h>
namespace ambulant {

namespace gui {

namespace gtk {

void gdk_pixbuf_blend (GdkPixbuf* dst, const lib::rect dst_rc, 
		       GdkPixbuf* src, const lib::rect src_rc,
		       double opacity_in, double opacity_out,
		       const lib::color_t chroma_low, 
		       const lib::color_t chroma_high);

void gdk_pixmap_dump(GdkPixmap* gpm, std::string filename);
	
} // namespace gtk

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_GTK_GTK_UTIL_H
