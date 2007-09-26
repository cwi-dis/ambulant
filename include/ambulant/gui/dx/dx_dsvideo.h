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

#ifndef AMBULANT_GUI_DX_DX_DSVIDEO_H
#define AMBULANT_GUI_DX_DX_DSVIDEO_H

#include "ambulant/config/config.h"
#include <objbase.h>
#ifdef AMBULANT_DDRAW_EX
#include <ddrawex.h>
#else
#include <ddraw.h>
#endif
#include "ambulant/common/video_renderer.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/gui/dx/dx_surface.h"

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace dx {


class dx_dsvideo_renderer : 
	public common::video_renderer {
  public:
	typedef lib::color_trible ColorType;

	dx_dsvideo_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
    	common::factories *factory);
	~dx_dsvideo_renderer();

	
//	void user_event(const point &where, int what = 0);
    void show_frame(const char* frame, int size);
	void redraw(const rect &dirty, gui_window *window);
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
	// XXXJACK: need to copy code from dx_renderer_playable
	gui::dx::dx_transition *get_transition() { return NULL; }
  private:
    void _init_bitmap();
	void _init_ddsurf(gui_window *window);
	void _copy_to_ddsurf();
	HBITMAP m_bitmap;			// Bitmap to hold the image
	char *m_bitmap_dataptr;		// Pointer to actual data inside bitmap
	IDirectDrawSurface *m_ddsurf;	// DD surface, for scaling and such
	critical_section m_lock;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_DX_DSVIDEO_H
