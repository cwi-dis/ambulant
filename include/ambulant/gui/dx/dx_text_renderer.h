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
	void render(LONG x=0, LONG y=0, HFONT h=NULL);
	bool can_play() const { return m_ddsurf != 0;}
	bool is_transparent() const { return true;}
	const lib::size& get_size() const { return m_size;}
	IDirectDrawSurface *get_ddsurf() { return m_ddsurf;}
	void set_text_color(lib::color_t color);
	void set_text_bgcolor(lib::color_t color);
	void set_text_size(float size);
	void set_text_font(const char *font);
	void set_text_data(const char* data, size_t datalen);
	void free_text_data();

  private:
	lib::size m_size;
	net::url m_url;
	viewport* m_viewport;
	IDirectDrawSurface *m_ddsurf;
	lib::color_t m_text_color;
	lib::color_t m_text_bgcolor;
	lib::color_t m_default_bgcolor;
	float m_text_size;
	const char *m_text_font;
	char* m_text_data;
	size_t m_text_datalen;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_TEXT_RENDERER_H

