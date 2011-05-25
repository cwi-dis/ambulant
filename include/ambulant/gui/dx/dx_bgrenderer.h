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

#ifndef AMBULANT_GUI_DX_BGRENDERER_H
#define AMBULANT_GUI_DX_BGRENDERER_H

#include "ambulant/config/config.h"

#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/layout.h"
#include "ambulant/lib/gtypes.h"
#ifdef WITH_D2D
#error Including dx include file while building for Direct2D
#endif

struct IDirectDrawSurface;

namespace ambulant {

namespace gui {

namespace dx {

class dx_bgrenderer : public common::background_renderer {
  public:
	dx_bgrenderer(const common::region_info *src);
	~dx_bgrenderer();
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void highlight(common::gui_window *window);
	void keep_as_background();
private:
	IDirectDrawSurface *m_bg_image;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_BGRENDERER_H
