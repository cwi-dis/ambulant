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

#ifndef AMBULANT_GUI_DX_BGRENDERER_H
#define AMBULANT_GUI_DX_BGRENDERER_H

#include "ambulant/config/config.h"

#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/layout.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/gui/dg/dg_dib_surface.h"

namespace ambulant {

namespace gui {

namespace dg {

class dg_bgrenderer : public common::background_renderer {
  public:
	dg_bgrenderer(const common::region_info *src);
	~dg_bgrenderer();
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void highlight(common::gui_window *window);
	void keep_as_background();
private:
	dib_surface_t *m_bg_image;
};

} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_BGRENDERER_H
