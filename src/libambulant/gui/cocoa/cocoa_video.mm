/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/gui/cocoa/cocoa_video.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/common/region_info.h"

#include <Cocoa/Cocoa.h>
#include <QuickTime/QuickTime.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define POLL_INTERVAL 200  /* milliseconds */

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

typedef lib::no_arg_callback<cocoa_video_renderer> poll_callback;

cocoa_video_renderer::cocoa_video_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp)
:	renderer_playable(context, cookie, node, evp),
	m_url(node->get_url("src")),
	m_movie(NULL),
	m_movie_view(NULL)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSURL *nsurl = [NSURL URLWithString: [NSString stringWithCString: m_url.get_url().c_str()]];
//	NSURL *nsurl = [NSURL fileURLWithPath: [NSString stringWithCString: m_url.get_url().c_str()]];
	if (!nsurl) {
		lib::logger::get_logger()->error("cocoa_video_renderer: cannot convert to URL: %s", m_url.get_url().c_str());
		return;
	}
	m_movie = [[NSMovie alloc] initWithURL: nsurl byReference: YES];
	if (!m_movie) {
		lib::logger::get_logger()->error("cocoa_video_renderer: cannot open movie: %s", [[nsurl absoluteString] cString]);
		return;
	}
	AM_DBG lib::logger::get_logger()->trace("cocoa_video_renderer: m_movie=0x%x", m_movie);
	[pool release];
}

cocoa_video_renderer::~cocoa_video_renderer()
{
	m_lock.enter();
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (m_movie) {
		[m_movie release];
		m_movie = NULL;
	}
	if (m_movie_view) {
		[m_movie_view removeFromSuperview];
		m_movie_view = NULL;
	}
	[pool release];
	m_lock.leave();

}

std::pair<bool, double> 
cocoa_video_renderer::get_dur()
{
	if (!m_movie)
		return std::pair<bool, double>(false, 0);
	Movie mov = (Movie)[m_movie QTMovie];
	AM_DBG lib::logger::get_logger()->trace("cocoa_video_renderer: QTMovie is 0x%x", (void *)mov);
	return std::pair<bool, double>(true, 7);
}

void
cocoa_video_renderer::start(double where)
{
	AM_DBG lib::logger::get_logger()->trace("cocoa_video_renderer::start()");
	if (!m_dest) {
		lib::logger::get_logger()->trace("cocoa_video_renderer::start: no destination surface");
		m_context->stopped(m_cookie);
		return;
	}
	m_lock.enter();
	m_dest->show(this); // XXX Do we need this?
	m_lock.leave();
}

void
cocoa_video_renderer::stop()
{
	m_lock.enter();
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (m_dest) m_dest->renderer_done(this);
	if (m_movie_view) {
		AM_DBG logger::get_logger()->trace("cocoa_video_renderer.stop: removing m_movie_view 0x%x", (void *)m_movie_view);
		[m_movie_view stop: NULL];
		[m_movie_view removeFromSuperview];
		m_movie_view = NULL;
	}
	if (m_movie) {
		AM_DBG logger::get_logger()->trace("cocoa_video_renderer.stop: release m_movie 0x%x", (void *)m_movie);
		[m_movie release];
		m_movie = NULL;
	}
	[pool release];
	m_lock.leave();
	m_context->stopped(m_cookie);
}

void
cocoa_video_renderer::poll_playing()
{
	m_lock.enter();
	if (m_movie == NULL || m_movie_view == NULL) {
		// Movie is not running. No need to continue polling right now
		m_lock.leave();
		return;
	}
	bool is_stopped = ![m_movie_view isPlaying];
	if (!is_stopped) {
		// schedule another call in a while
		ambulant::lib::event *e = new poll_callback(this, &cocoa_video_renderer::poll_playing);
		m_event_processor->add_event(e, POLL_INTERVAL, ambulant::lib::event_processor::low);
	}
	AM_DBG lib::logger::get_logger()->trace("cocoa_video_renderer::poll_playing: is_stopped=%d", is_stopped);
	m_lock.leave();
	if (is_stopped)
		m_context->stopped(m_cookie);
}

void
cocoa_video_renderer::redraw(const screen_rect<int> &dirty, gui_window *window)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	screen_rect<int> dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	AM_DBG logger::get_logger()->trace("cocoa_video_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	if (m_movie && !m_movie_view) {
		AM_DBG logger::get_logger()->trace("cocoa_video_renderer.redraw: creating movie view");
		// Create the movie view and link it in
		NSRect frameRect = [view NSRectForAmbulantRect: &dstrect];
		
		m_movie_view = [[NSMovieView alloc] initWithFrame: frameRect];
		[m_movie_view showController: NO adjustingSize: NO];
		[view addSubview: m_movie_view];
		// Set the movie playing
		[m_movie_view setMovie: m_movie];
		[m_movie_view start: NULL];
		// And start the poll task
		ambulant::lib::event *e = new poll_callback(this, &cocoa_video_renderer::poll_playing);
		m_event_processor->add_event(e, POLL_INTERVAL, ambulant::lib::event_processor::low);
	}
	
	m_lock.leave();
}

void 
cocoa_video_renderer::user_event(const point &where, int what)
{
	if (what == user_event_click) m_context->clicked(m_cookie, 0);
	else if (what == user_event_mouse_over) m_context->pointed(m_cookie, 0);
	else assert(0);
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant
