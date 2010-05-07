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

#ifndef QT_VIDEO_RENDERER
#define QT_VIDEO_RENDERER

#include "ambulant/config/config.h"
#include "ambulant/common/video_renderer.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/gui/qt/qt_includes.h"

namespace ambulant {

namespace gui {

namespace qt {

class qt_video_renderer : public qt_renderer<common::video_renderer> {
  public:
	qt_video_renderer(
				 common::playable_notification *context,
				 common::playable_notification::cookie_type cookie,
				 const lib::node *node,
				 lib::event_processor *const evp,
				 common::factories *factory,
				 common::playable_factory_machdep *mdp);
	
	~qt_video_renderer();
	void redraw_body(const lib::rect &r, common::gui_window* w);
  protected:
	void _push_frame(char* frame, int size);
 
 private:
	QImage *m_image;
 	uchar* m_data;
	int   m_datasize;
};


} // namespace qt

} // namespace gui
 
} // namespace ambulant




#endif /* QT_VIDEO_RENDERER */
