// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#include "ambulant/gui/cocoa/cocoa_video.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

#include <Cocoa/Cocoa.h>
#include <QuickTime/QuickTime.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define POLL_INTERVAL 20  /* milliseconds */

#ifndef __LP64__
// Helper class to create QTMovie in main thread
@interface MovieCreator : NSObject
{
	QTMovie *movie;
	ambulant::net::timestamp_t position_wanted;
}
+ (void)removeFromSuperview: (QTMovieView *)view;

- (MovieCreator *)init;
- (void)setPositionWanted: (ambulant::net::timestamp_t)begintime;
- (QTMovie *)movie;
- (void)movieWithURL: (NSURL*)url;
- (void)moviePrepare: (id)sender;
- (void)movieStart: (id)sender;
- (void)movieRelease: (id)sender;
@end

@implementation MovieCreator

+ (void)removeFromSuperview: (QTMovieView *)view
{
	[view retain];
	[view pause: self];
	[view removeFromSuperview];
}

- (MovieCreator *)init
{
	self = [super init];
	movie = NULL;
	return self;
}

- (void)setPositionWanted: (ambulant::net::timestamp_t)begintime
{
	position_wanted = begintime;
}

- (QTMovie *)movie
{
	return movie;
}

- (void)movieWithURL: (NSURL*)url
{
	assert(movie == NULL);
	NSDictionary *attrs = [NSDictionary dictionaryWithObjectsAndKeys:
		(id)url, QTMovieURLAttribute,
		[NSNumber numberWithBool:NO], QTMovieOpenAsyncOKAttribute,
		nil];
	movie = [[QTMovie movieWithAttributes:attrs error:nil] retain];
	SetMovieRate([movie quickTimeMovie], 0);
//	[self moviePrepare: self];
}

- (void)moviePrepare: (id) sender
{
	Movie mov = [movie quickTimeMovie];
	TimeValue movtime;
	if (position_wanted >= 0) {
		TimeScale movscale = GetMovieTimeScale(mov);
		movtime = (TimeValue)(position_wanted*(double)movscale/1000000.0);
		SetMovieTimeValue(mov, movtime);
		Fixed playRate = GetMovieRate(mov);
		if (playRate == 0) {
			playRate = GetMoviePreferredRate(mov);
			PrePrerollMovie(mov, movtime, playRate, nil, nil);
			PrerollMovie(mov, movtime, playRate);
		}
		MoviesTask(mov, 0);
	}
	position_wanted = -1;
}

- (void)movieStart: (id) sender
{
	Movie mov = [movie quickTimeMovie];
	SetMovieRate(mov, GetMoviePreferredRate(mov));
}

- (void)movieRelease: (id) sender
{
	assert(movie);
	[movie release];
	movie = NULL;
}
@end

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

typedef lib::no_arg_callback<cocoa_video_renderer> poll_callback;


extern const char cocoa_video_playable_tag[] = "video";
extern const char cocoa_video_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCocoa");
extern const char cocoa_video_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererQuickTime");
extern const char cocoa_video_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererVideo");

common::playable_factory *
create_cocoa_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCocoa"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererQuickTime"), true);
	return new common::single_playable_factory<
		cocoa_video_renderer,
		cocoa_video_playable_tag,
		cocoa_video_playable_renderer_uri,
		cocoa_video_playable_renderer_uri2,
		cocoa_video_playable_renderer_uri3>(factory, mdp);
}

cocoa_video_renderer::cocoa_video_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	renderer_playable(context, cookie, node, evp, fp, mdp),
	m_url(),
	m_movie(NULL),
	m_movie_view(NULL),
	m_mc(NULL),
	m_paused(false),
	m_previous_clip_position(-1),
	m_renderer_state(rs_created)
{
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::cocoa_video_renderer(0x%x)", this);
//	init_with_node(node);
}

cocoa_video_renderer::~cocoa_video_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::~cocoa_video_renderer(0x%x), m_movie=0x%x", this, m_movie);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	if (m_movie_view) {
		[MovieCreator performSelectorOnMainThread: @selector(removeFromSuperview:) withObject: m_movie_view waitUntilDone: NO];
		m_movie_view = NULL;
		// XXXJACK Should call releaseOverlayWindow here
	}
	if (m_mc) {
		[(MovieCreator *)m_mc performSelectorOnMainThread: @selector(releaseMovie:) withObject: nil waitUntilDone: NO];
		[(MovieCreator *)m_mc release];
		m_mc = NULL;
	}
	[pool release];
	m_lock.leave();

}

void
cocoa_video_renderer::init_with_node(const lib::node *n)
{
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	renderer_playable::init_with_node(n);
	assert(m_renderer_state == rs_created || m_renderer_state == rs_prerolled || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped);
	m_renderer_state = rs_inited;

	m_node = n;
	if (m_movie == NULL) {
		assert(m_mc == NULL);
		assert(m_url.is_empty_path() || m_url.same_document(m_node->get_url("src")));
		// Apparently the first call.
		m_url = m_node->get_url("src");
		NSURL *nsurl = [NSURL URLWithString: [NSString stringWithCString: m_url.get_url().c_str() encoding: NSUTF8StringEncoding]];
		if (!nsurl) {
			lib::logger::get_logger()->error(gettext("%s: cannot convert to URL"), m_url.get_url().c_str());
			goto bad;
		}
		m_mc = (void*)[[[MovieCreator alloc] init] retain];
		[(MovieCreator *)m_mc performSelectorOnMainThread: @selector(movieWithURL:) withObject: nsurl waitUntilDone: YES];
		m_movie = [(MovieCreator *)m_mc movie];
	}
	if (!m_movie) {
		lib::logger::get_logger()->error(gettext("%s: cannot open movie"), m_url.get_url().c_str());
		goto bad;
	}

	_init_clip_begin_end();
	if (m_clip_begin != m_previous_clip_position) {
		[(MovieCreator *)m_mc setPositionWanted: m_clip_begin];
	}
	m_previous_clip_position = m_clip_begin;

	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer(0x%x)::init_with_node, m_movie=0x%x url=%s clipbegin=%d", this, m_movie, m_url.get_url().c_str(), m_clip_begin);

  bad:
	[pool release];
	m_lock.leave();
}

void
cocoa_video_renderer::preroll(double when, double where, double how_much)
{
	m_lock.enter();
	assert(m_renderer_state == rs_inited || m_renderer_state == rs_prerolled || m_renderer_state == rs_stopped);
	m_renderer_state = rs_prerolled;
	[(MovieCreator *)m_mc performSelectorOnMainThread: @selector(moviePrepare:) withObject: nil waitUntilDone: YES];
	m_lock.leave();
}

common::duration
cocoa_video_renderer::get_dur()
{
	common::duration rv(false, 0);
	m_lock.enter();
	assert(m_renderer_state == rs_inited || m_renderer_state == rs_prerolled || m_renderer_state == rs_started || m_renderer_state == rs_stopped);

	if (m_movie) {
		Movie mov = [m_movie quickTimeMovie];
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
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::start(0x%x)", this);
	if (!m_dest) {
		lib::logger::get_logger()->debug("cocoa_video_renderer::start: no destination surface");
		m_context->stopped(m_cookie);
		return;
	}
	preroll(0, where, 0);
	m_lock.enter();
	assert(m_renderer_state == rs_prerolled);
	m_renderer_state = rs_started;
	if(m_movie == NULL) {
		m_renderer_state = rs_stopped;
		m_context->stopped(m_cookie);
		m_lock.leave();
		return;
	}
	m_paused = false;
	m_dest->show(this); // XXX Do we need this?

	Movie mov = [m_movie quickTimeMovie];
	if (GetMovieRate(mov) == 0) {
		_fix_video_epoch();
	}

	[(MovieCreator *)m_mc performSelectorOnMainThread: @selector(movieStart:) withObject: nil waitUntilDone: YES];
	m_previous_clip_position = -1;
	// And start the poll task
	ambulant::lib::event *e = new poll_callback(this, &cocoa_video_renderer::_poll_playing);
	m_event_processor->add_event(e, POLL_INTERVAL, ambulant::lib::ep_low);
	m_lock.leave();
}

bool
cocoa_video_renderer::stop()
{
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer(0x%x)::stop()", (void*)this);
	assert(m_renderer_state == rs_prerolled || m_renderer_state == rs_started || m_renderer_state == rs_stopped);
	m_renderer_state = rs_stopped;
	m_context->stopped(m_cookie);
	return false;
}

void
cocoa_video_renderer::post_stop()
{
	m_lock.enter();
#if 0
	// This assertion can trigger, but I've only seen it in release builds of the Safari plugin
	// (which is very difficult to debug). Playing videotests example document from the website.
	// Stack shows we're called from destroy_playable_in_cache.
	assert(m_renderer_state == rs_stopped);
#endif
	m_renderer_state = rs_fullstopped;
	AM_DBG logger::get_logger()->debug("cocoa_video_renderer::post_stop(0x%x)", this);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	if (m_movie_view) {
		AM_DBG logger::get_logger()->debug("cocoa_video_renderer.post_stop: removing m_movie_view 0x%x", (void *)m_movie_view);
		[MovieCreator performSelectorOnMainThread: @selector(removeFromSuperview:) withObject: m_movie_view waitUntilDone: NO];
		m_movie_view = NULL;
	}
	[pool release];
	m_lock.leave();
}

void
cocoa_video_renderer::pause(pause_display d)
{
	m_lock.enter();
	assert(m_renderer_state == rs_prerolled || m_renderer_state == rs_started || m_renderer_state == rs_stopped);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::pause(0x%x)", this);
	m_paused = true;
	if (m_movie && m_movie_view) {
		[m_movie_view pause: NULL];
		if (d == display_hide)
			[m_movie_view setHidden: YES];
	}
	[pool release];
	m_lock.leave();
}

void
cocoa_video_renderer::resume()
{
	m_lock.enter();
	assert(m_renderer_state == rs_prerolled || m_renderer_state == rs_started || m_renderer_state == rs_stopped);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::resume(0x%x)", this);
	m_paused = false;
	if (m_movie && m_movie_view) {
		if ([m_movie_view isHidden]) [m_movie_view setHidden: NO];
		[m_movie_view play: NULL];
	}
	_fix_video_epoch();
	[pool release];
	m_lock.leave();
}

void
cocoa_video_renderer::seek(double where)
{
	m_lock.enter();
	lib::logger::get_logger()->debug("cocoa_video_renderer::seek(%f)", where);
	assert( where >= 0);
	Movie mov = [m_movie quickTimeMovie];
	TimeValue movtime;
	TimeScale movscale = GetMovieTimeScale(mov);
	movtime = (TimeValue)((where+m_clip_begin/1000000.0)*(double)movscale);
	SetMovieTimeValue(mov, movtime);

	m_lock.leave();
}

void
cocoa_video_renderer::_poll_playing()
{
	m_lock.enter();
	if (m_movie == NULL) {
		// Movie is not running. No need to continue polling right now
		m_lock.leave();
		return;
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	bool is_stopped = IsMovieDone([m_movie quickTimeMovie]);
	if (!is_stopped && m_clip_end > 0) {
		Movie mov = [m_movie quickTimeMovie];
		TimeValue movtime = GetMovieTime(mov, NULL);
		TimeScale movscale = GetMovieTimeScale(mov);
		double curtime = (double)movtime / (double)movscale;
		if ( curtime > (m_clip_end/1000000.0)) {
			is_stopped = true;
			// if (m_movie_view) [m_movie_view pause: NULL];
			m_previous_clip_position = m_clip_end;
		}
	}

	if (!is_stopped) {
		_fix_clock_drift();
		// schedule another call in a while
		ambulant::lib::event *e = new poll_callback(this, &cocoa_video_renderer::_poll_playing);
		m_event_processor->add_event(e, POLL_INTERVAL, ambulant::lib::ep_low);
	}
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::_poll_playing: is_stopped=%d", is_stopped);
	[pool release];
	m_lock.leave();
	if (is_stopped) {
		AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer(0x%x): calling stopped()", this);
		m_context->stopped(m_cookie);
	}
}

void
cocoa_video_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	assert(m_renderer_state == rs_started || m_renderer_state == rs_stopped);
	if (!m_movie) {
		m_lock.leave();
		return;
	}
	NSValue *value = [m_movie attributeForKey:QTMovieNaturalSizeAttribute];
	NSSize nssize = [value sizeValue];
	size srcsize = size(int(nssize.width), int(nssize.height));
	rect srcrect;
	rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
	dstrect.translate(m_dest->get_global_topleft());

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	NSRect frameRect = [view NSRectForAmbulantRect: &dstrect];

	if (!m_movie_view) {
		// Create the movie view if it doesn't exist already
		AM_DBG logger::get_logger()->debug("cocoa_video_renderer.redraw: creating movie view");
		m_movie_view = [[QTMovieView alloc] initWithFrame: frameRect];
		[m_movie_view setControllerVisible: NO];
		[m_movie_view setMovie: m_movie];
	}

	NSView *parent = [m_movie_view superview];
	if (parent != (NSView *)view) {
		// Put the movie view into the hierarchy, possibly removing it from the old place first.
		if (parent) {
			[m_movie_view retain];
			[m_movie_view removeFromSuperviewWithoutNeedingDisplay];
		}
		[view addSubview: m_movie_view];
		[m_movie_view release];
		[view requireOverlayWindow];
		// Set things up so subsequent redraws go to the overlay window
		[view useOverlayWindow];
	}

	//	Need to compare frameRect to current Qt rect and move if needed
	if (!NSEqualRects(frameRect, [m_movie_view frame]) ) {
		[m_movie_view setFrame: frameRect];
	}

	m_lock.leave();
}

void
cocoa_video_renderer::_fix_video_epoch()
{
	Movie mov = [m_movie quickTimeMovie];
	TimeValue movtime = GetMovieTime(mov, NULL);
	TimeScale movscale = GetMovieTimeScale(mov);
	double curtime = (double)movtime / (double)movscale;
	m_video_epoch = m_event_processor->get_timer()->elapsed() - lib::timer::signed_time_type(curtime*1000.0);
}

void
cocoa_video_renderer::_fix_clock_drift()
{
	Movie mov = [m_movie quickTimeMovie];
	TimeValue movtime = GetMovieTime(mov, NULL);
	TimeScale movscale = GetMovieTimeScale(mov);
	double curtime = (double)movtime / (double)movscale;
	lib::timer::signed_time_type expected_time = m_video_epoch + lib::timer::signed_time_type(curtime*1000.0);
	lib::timer::signed_time_type clock_drift = expected_time - m_event_processor->get_timer()->elapsed();

	// If we have drifted too far we assume something fishy is going on and resync.
	if (clock_drift < -100000 || clock_drift > 100000) {
		lib::logger::get_logger()->trace("cocoa_video: Quicktime clock %dms ahead. Resync.");
		_fix_video_epoch();
		return;
	}
	if (clock_drift < -1 || clock_drift > 1) {
		lib::timer::signed_time_type residual_clock_drift = m_event_processor->get_timer()->set_drift(clock_drift);
		if (residual_clock_drift) {
			lib::logger::get_logger()->debug("cocoa_video_renderer: not implemented: adjusting audio clock by %ld ms", residual_clock_drift);
		}
		// XXX For now, assume residual_clock_drift always zero.
	}
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

#endif // __LP64__
