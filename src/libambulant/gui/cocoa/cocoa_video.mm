// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

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
		lib::logger::get_logger()->error(gettext("%s: cannot convert to URL"), m_url.get_url().c_str());
		return;
	}
	m_movie = [[NSMovie alloc] initWithURL: nsurl byReference: YES];
	if (!m_movie) {
		lib::logger::get_logger()->error(gettext("%s: cannot open movie"), [[nsurl absoluteString] cString]);
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer: m_movie=0x%x", m_movie);
	
	_init_clip_begin_end();
	if (m_clip_begin) {
		Movie mov = (Movie)[m_movie QTMovie];
		TimeScale movscale = GetMovieTimeScale(mov);
		TimeValue movtime = (TimeValue)(m_clip_begin*(double)movscale/1000000.0);
		SetMovieTimeValue(mov, movtime);
	}

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
		[m_movie_view stop: NULL];
		[m_movie_view removeFromSuperview];
		m_movie_view = NULL;
	}
	[pool release];
	m_lock.leave();

}

common::duration 
cocoa_video_renderer::get_dur()
{
	common::duration rv(false, 0);
	m_lock.enter();
	if (m_movie) {
		Movie mov = (Movie)[m_movie QTMovie];
		AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::get_dur QTMovie is 0x%x", (void *)mov);
		TimeValue movdur = GetMovieDuration(mov);
		TimeScale movscale = GetMovieTimeScale(mov);
		double dur = (double)movdur / (double)movscale;
		if (m_clip_end > 0 && dur > (m_clip_end/1000000.0))
			dur = (m_clip_end/1000000.0);
		if (m_clip_begin > 0)
			dur -= (m_clip_begin/1000000.0);
		AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::get_dur: GetMovieDuration=%f", dur);
		rv = common::duration(true, dur);
	}
	m_lock.leave();
	return rv;
}

void
cocoa_video_renderer::start(double where)
{
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::start()");
	if (!m_dest) {
		lib::logger::get_logger()->debug("cocoa_video_renderer::start: no destination surface");
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
		AM_DBG logger::get_logger()->debug("cocoa_video_renderer.stop: removing m_movie_view 0x%x", (void *)m_movie_view);
		[m_movie_view stop: NULL];
		[m_movie_view removeFromSuperview];
		m_movie_view = NULL;
	}
	if (m_movie) {
		AM_DBG logger::get_logger()->debug("cocoa_video_renderer.stop: release m_movie 0x%x", (void *)m_movie);
		[m_movie release];
		m_movie = NULL;
	}
	[pool release];
	m_lock.leave();
	m_context->stopped(m_cookie);
}

void
cocoa_video_renderer::pause(pause_display d)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::pause(%s)", m_node->get_sig().c_str());
	if (m_movie && m_movie_view) {
		[m_movie_view stop: NULL];
		if (d == display_hide)
			[m_movie_view setHidden: YES];
	}
	m_lock.leave();
}

void
cocoa_video_renderer::resume()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::resume()");
	if (m_movie && m_movie_view) {
		if ([m_movie_view isHidden]) [m_movie_view setHidden: NO];
		[m_movie_view start: NULL];
	}
	m_lock.leave();
}

void
cocoa_video_renderer::seek(double where)
{
	m_lock.enter();
	lib::logger::get_logger()->debug("cocoa_video_renderer::seek(%f): not implemented", where);
	m_lock.leave();
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
	if (!is_stopped && m_clip_end > 0) {
		Movie mov = (Movie)[m_movie QTMovie];
		TimeValue movtime = GetMovieTime(mov, NULL);
		TimeScale movscale = GetMovieTimeScale(mov);
		double curtime = (double)movtime / (double)movscale;
		if ( curtime > (m_clip_end/1000000.0)) {
			is_stopped = true;
			if (m_movie_view) [m_movie_view stop: NULL];
		}
	}
	
	if (!is_stopped) {
		// schedule another call in a while
		ambulant::lib::event *e = new poll_callback(this, &cocoa_video_renderer::poll_playing);
		m_event_processor->add_event(e, POLL_INTERVAL, ambulant::lib::ep_low);
	}
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::poll_playing: is_stopped=%d", is_stopped);
	m_lock.leave();
	if (is_stopped)
		m_context->stopped(m_cookie);
}

void
cocoa_video_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	rect dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	AM_DBG logger::get_logger()->debug("cocoa_video_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	if (m_movie && !m_movie_view) {
		AM_DBG logger::get_logger()->debug("cocoa_video_renderer.redraw: creating movie view");
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
		m_event_processor->add_event(e, POLL_INTERVAL, ambulant::lib::ep_low);
	}
	
	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant
