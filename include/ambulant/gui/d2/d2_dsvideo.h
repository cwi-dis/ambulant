/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
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

#ifndef AMBULANT_GUI_D2_D2_DSVIDEO_H
#define AMBULANT_GUI_D2_D2_DSVIDEO_H

#include "ambulant/config/config.h"
#include "ambulant/common/video_renderer.h"
#include "ambulant/lib/mtsync.h"
//#include "ambulant/gui/d2/d2_transition.h"
#include "ambulant/gui/d2/d2_renderer.h"

interface ID2D1Bitmap;

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace d2 {


class d2_dsvideo_renderer :
	public d2_renderer<common::video_renderer> {
  public:
	typedef lib::color_trible ColorType;

	d2_dsvideo_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp);
	~d2_dsvideo_renderer();

	net::pixel_order pixel_layout();
	void redraw_body(const rect &dirty, gui_window *window);

	void recreate_d2d();
	void discard_d2d();
  protected:
	void _push_frame(char* frame, size_t size);
  private:
	char *m_frame;				// Current frame
	size_t m_frame_size;			// Size of current frame
	ID2D1Bitmap *m_d2bitmap;		// The bitmap in Direct2D form
//JNK	HBITMAP m_bitmap;			// Bitmap to hold the image (if needed)
	char *m_bitmap_dataptr;		// Pointer to actual data inside bitmap (if needed)
//JNK	IDirectDrawSurface *m_ddsurf;	// DD surface, for scaling and such
#ifdef ENABLE_FAST_DDVIDEO
//JNK	IDirectDrawSurface7 *m_ddsurf7;	// Same, for replacing the underlying storage
#endif
};

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_D2_DSVIDEO_H