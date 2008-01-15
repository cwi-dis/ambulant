/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
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

#ifndef AMBULANT_GUI_QT_QT_UTIL_H
#define AMBULANT_GUI_QT_QT_UTIL_H

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"

namespace ambulant {

namespace gui {

namespace qt {

// blending: used for media opacity, chromakeying and fading
 
void qt_image_blend (QImage dst, const lib::rect dst_rc, 
		     QImage src, const lib::rect src_rc,
		     double opacity_in, double opacity_out,
		     const lib::color_t chroma_low, 
		     const lib::color_t chroma_high);

// convert QColor <-> color_t
color_t QColor2color_t(QColor c);
QColor color_t2QColor(lib::color_t c);

// image debugging: dump pixmaps at various stages of drawing process.
// useful e.g. for animations, transitions and such.
// Enable qt_pixmap_dump by defining WITH_DUMPPIXMAP
//#define	WITH_DUMPPIXMAP
#ifdef	WITH_DUMPPIXMAP
#define DUMPPIXMAP(pixmap, filename)  qt_pixmap_dump(pixmap, filename);
#else
#define DUMPPIXMAP(pixmap, filename)
#endif//WITH_DUMPPIXMAP

void qt_pixmap_dump(QPixmap* gpm, std::string filename);
	
} // namespace qt

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_QT_QT_UTIL_H
