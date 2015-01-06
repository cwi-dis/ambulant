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

#ifndef AMBULANT_GUI_GTK_GTK_UTIL_H
#define AMBULANT_GUI_GTK_GTK_UTIL_H

#include "ambulant/gui/gtk/gtk_renderer.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

namespace ambulant {

namespace gui {

namespace gtk {

// blending: used for media opacity, chromakeying and fading

// blend every pixel within 'src_rc' of 'src' with the corresponding
// one in 'dst_rc' of 'dst' using 'opacity_in'/'opacity_out'  if the
// source pixel color is in/out the range [chroma_low,chroma_high], resp.
// when 'mask_color' is not 0 (black), and the source pixel color equals
// the 'mask_color', the source pixel is not blended.

void gdk_pixbuf_blend (GdkPixbuf* dst, const lib::rect dst_rc,
	GdkPixbuf* src, const lib::rect src_rc,
	double opacity_in, double opacity_out,
	const lib::color_t chroma_low,
	const lib::color_t chroma_high,
	const lib::color_t mask_color=0);

// image debugging: dump images/pixmaps at various stages of drawing process.
// needed for development and maintenace of image drawing, e.g. in the context
// of animations, transitions and such.
// Enable by adding -DWITH_DUMPIMAGES to CXXFLAGS on the ./configure line
// #define	WITH_DUMPIMAGES
#ifdef	WITH_DUMPIMAGES
// dump a GDKPixmap/cairo_surface_t or pixbuf on a file named 'nnnnid' where nnnn is a
// generated 4-digit number from 0000-9999 and id is parameter
// return the 4-digit number for identfication purposes 
int
#if GTK_MAJOR_VERSION >= 3
cairo_surface_dump(cairo_surface_t* srf, std::string id);
#define DUMPSURFACE(surface, id)  cairo_surface_dump(surface, id)
#else
gdk_pixmap_dump(GdkPixmap* gpm, std::string id);
#define DUMPPIXMAP(gdkpixmap, id)  gdk_pixmap_dump(gdkpixmap, id)
#endif // GTK_MAJOR_VERSION
int
gdk_pixbuf_dump(GdkPixbuf* gpb, std::string id);
#define DUMPPIXBUF(gdkpixbuf, id)  gdk_pixbuf_dump(gdkpixbuf, id)

#else// ! WITH_DUMPIMAGES
// dummy macro's
#if GTK_MAJOR_VERSION >= 3
#define DUMPSURFACE(surface, id)
#else
#define DUMPPIXBUF(gdkpixbuf, id)
#endif // GTK_MAJOR_VERSION
#define DUMPPIXMAP(gdkpixmap, id)
#endif// ! WITH_DUMPIMAGES


} // namespace gtk

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_GTK_GTK_UTIL_H
