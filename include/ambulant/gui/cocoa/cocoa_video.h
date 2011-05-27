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

#ifndef AMBULANT_GUI_COCOA_COCOA_VIDEO_H
#define AMBULANT_GUI_COCOA_COCOA_VIDEO_H

#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/mtsync.h"
#include <Cocoa/Cocoa.h>
#include <QTKit/QTKit.h>

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
		event_processor *evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp);
	~cocoa_video_renderer();

	void start(double where);
//	void freeze() {}
//	void stop();
	bool stop();
	void post_stop();
	void init_with_node(const lib::node *n);
	void preroll(double when, double where, double how_much);
	void pause(pause_display d=display_show);
	void resume();
	void seek(double t);

	common::duration get_dur();

	void redraw(const rect &dirty, gui_window *window);
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
  private:
	enum { rs_created, rs_inited, rs_prerolled, rs_started, rs_stopped, rs_fullstopped } m_renderer_state; // Debugging, mainly
	void _poll_playing();
	net::url m_url;             // The URL of the movie we play
	QTMovie *m_movie;           // The movie itself
	QTMovieView *m_movie_view;  // The view displaying the movie
	void *m_mc;                 // Our helper ObjC class to run methods in the main thread
	bool m_paused;
	net::timestamp_t m_previous_clip_position; // Where we are officially positioned
	lib::timer::signed_time_type m_video_epoch;    // Ambulant clock value corresponding to video clock 0.
	void _fix_video_epoch();    // Set m_video_epoch according to current movie time
	void _fix_clock_drift();    // Synchronise movie clock and ambulant clock

	critical_section m_lock;
};

} // namespace cocoa

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_VIDEO_H
