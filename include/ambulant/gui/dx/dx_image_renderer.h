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

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_DX_IMAGE_RENDERER_H
#define AMBULANT_GUI_DX_IMAGE_RENDERER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/net/url.h"
#include "ambulant/net/datasource.h"

#include <string>
#ifdef WITH_D2D
#error Including dx include file while building for Direct2D
#endif

struct IDirectDrawSurface;

namespace ambulant {

namespace gui {

namespace dx {

class viewport;

class image_renderer {
  public:
	image_renderer(const net::url& u, net::datasource *src, viewport* v);
	~image_renderer();

	bool can_play() const { return m_ddsurf != 0;}
	bool is_transparent() const { return m_transparent;}
	const lib::size& get_size() const { return m_size;}
	IDirectDrawSurface *get_ddsurf() { return m_ddsurf;}

  private:
	void open(net::datasource *src, viewport* v);
	net::url m_url;
	IDirectDrawSurface *m_ddsurf;
	lib::size m_size;
	bool m_transparent;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_IMAGE_RENDERER_H

