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

#ifndef AMBULANT_GUI_DX_TEXT_RENDERER_H
#define AMBULANT_GUI_DX_TEXT_RENDERER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/colors.h"
#include "ambulant/net/url.h"
#include "ambulant/net/datasource.h"

#include <string>

struct IDirectDrawSurface;

namespace ambulant {

namespace gui {

namespace dx {

class viewport;

class text_renderer {
  public:
	text_renderer(const net::url& u, const lib::size& bounds, viewport* v);
	~text_renderer();
	void open(net::datasource_factory *df);
	bool can_play() const { return m_ddsurf != 0;}
	bool is_transparent() const { return true;}
	const lib::size& get_size() const { return m_size;}
	IDirectDrawSurface *get_ddsurf() { return m_ddsurf;}
	void set_text_color(lib::color_t color);
	void set_text_size(float size);
	void set_text_font(const char *font);
	
  private:
	lib::size m_size;
	net::url m_url;
	viewport* m_viewport;
	IDirectDrawSurface *m_ddsurf;
	lib::color_t m_text_color;
	float m_text_size;
	const char *m_text_font;
};

} // namespace dx

} // namespace gui

} // namespace ambulant 

#endif // AMBULANT_GUI_DX_TEXT_RENDERER_H

