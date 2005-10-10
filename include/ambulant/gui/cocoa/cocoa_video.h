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

#ifndef AMBULANT_GUI_COCOA_COCOA_VIDEO_H
#define AMBULANT_GUI_COCOA_COCOA_VIDEO_H

#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/mtsync.h"
#include <Cocoa/Cocoa.h>

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cocoa {

class cocoa_video_renderer : 
	public renderer_playable {
  public:
	cocoa_video_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp);
	~cocoa_video_renderer();

	void start(double where);
//	void freeze() {}
	void stop();
	void seek(double t) {}

	common::duration get_dur();
	
	void redraw(const rect &dirty, gui_window *window);
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
  private:
	void poll_playing();
	net::url m_url;
	NSMovie *m_movie;
	NSMovieView *m_movie_view;
	critical_section m_lock;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_VIDEO_H
