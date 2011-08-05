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

#ifndef AMBULANT_GUI_QT_QT_UTIL_H
#define AMBULANT_GUI_QT_QT_UTIL_H

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"

namespace ambulant {

namespace gui {

namespace qt {

// blending: used for media opacity, chromakeying and fading

void
qt_image_blend (QImage dst, const lib::rect dst_rc,
	QImage src, const lib::rect src_rc,
	double opacity_in, double opacity_out,
	const lib::color_t chroma_low,
	const lib::color_t chroma_high);

// convert QColor <-> color_t
color_t QColor2color_t(QColor c);
QColor color_t2QColor(lib::color_t c);

// image debugging: dump images/pixmaps at various stages of drawing process.
// needed for development and maintenace of image drawing, e.g. in the context
// of animations, transitions and such.
// Enable by adding -DWITH_DUMPIMAGES to CXXFLAGS on the ./configure line
// #define	WITH_DUMPIMAGES

// dump a QImage or Qpixbuf on a file named 'nnnnid' where nnnn is a
// generated 4-digit number from 0000-9999 and id is parameter
// return the 4-digit number for identfication purposes 
#ifdef	WITH_DUMPIMAGES
#define DUMPIMAGE(qimage, id)  qt_image_dump(qimage, id)
int
qt_image_dump(QImage* qimage, std::string id);
#define DUMPPIXMAP(qpixmap, id)  qt_pixmap_dump(qpixmap, id)
int
qt_pixmap_dump(QPixmap* qpixmap, std::string id);
#else
#define DUMPIMAGE(qimage, id)
#define DUMPPIXMAP(qpixmap, id)
#endif//WITH_DUMPIMAGES

} // namespace qt

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_QT_QT_UTIL_H
