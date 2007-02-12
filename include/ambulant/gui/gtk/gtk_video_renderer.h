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

#ifndef gtk_VIDEO_RENDERER
#define gtk_VIDEO_RENDERER

#include "ambulant/config/config.h"
#include "ambulant/common/video_renderer.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/layout.h"
#include "ambulant/net/raw_video_datasource.h"
#include "ambulant/common/playable.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

namespace ambulant {

namespace gui {

namespace gtk {
	
	

class gtk_video_renderer :  public gtk_renderer<common::video_renderer>  {
  public:
	gtk_video_renderer(
				 common::playable_notification *context,
				 common::playable_notification::cookie_type cookie,
				 const lib::node *node,
				 lib::event_processor *const evp,
				 common::factories *factory);
	
	~gtk_video_renderer();
   	void show_frame(const char* frame, int size);
	void redraw_body(const lib::rect &r, common::gui_window* w);
private:
 	std::queue< std::pair<int, char*> > m_frames;
 	long int m_img_displayed;
	lib::critical_section m_lock;
	GdkPixbuf*	m_image;
	char* 		m_data;
};


} // namespace gtk

} // namespace gui
 
} // namespace ambulant




#endif /* GTK_VIDEO_RENDERER */
