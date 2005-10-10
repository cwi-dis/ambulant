/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
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

#ifndef AMBULANT_GUI_DG_IMAGE_RENDERER_H
#define AMBULANT_GUI_DG_IMAGE_RENDERER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/colors.h"
#include "ambulant/net/url.h"

#include "ambulant/gui/dg/dg_surface.h"
#include "ambulant/gui/dg/dg_dib_surface.h"

#include <string>

namespace ambulant {

namespace gui {

namespace dg {

class viewport;

class image_renderer {
  public:
	image_renderer(const net::url& u, viewport* v);
	~image_renderer();
	
	bool can_play() const { return m_dibsurf != 0;}
	bool is_transparent() const { return m_transparent;}
	const lib::size& get_size() const { return m_size;}
	dib_surface_t *get_dibsurf() { return m_dibsurf;}
	lib::color_t get_transp_color() const { return m_transp_color;}
	
  private:
	void open(const net::url& u, viewport* v);
	net::url m_url;
	dib_surface_t *m_dibsurf;
	lib::size m_size;
	bool m_transparent;
	lib::color_t m_transp_color;
};

} // namespace dg

} // namespace gui

} // namespace ambulant 

#endif // AMBULANT_GUI_DG_IMAGE_RENDERER_H

