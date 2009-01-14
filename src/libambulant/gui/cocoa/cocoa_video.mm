// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
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
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

#include <Cocoa/Cocoa.h>
#include <QuickTime/QuickTime.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define POLL_INTERVAL 200  /* milliseconds */

// Helper class to create QTMovie in main thread
@interface MovieCreator : NSObject
{
	QTMovie *movie;
	ambulant::net::timestamp_t clip_begin;
}
- (MovieCreator *)init: (ambulant::net::timestamp_t)begintime;
- (QTMovie *)movie;
- (void)movieWithURL: (NSURL*)url;
+ (void)removeFromSuperview: (QTMovieView *)view;
+ (void)removeFromSuperview: (QTMovieView *)view;
+ (void)removeFromSuperview: (QTMovieView *)view;
@end
@implementation MovieCreator
- (MovieCreator *)init: (ambulant::net::timestamp_t)begintime
{
	self = [super init];
	clip_begin = begintime;
	return self;
}

- (QTMovie *)movie
{
	return movie;
}

- (void)movieWithURL: (NSURL*)url
{
	movie = NULL;
    NSDictionary *attrs = [NSDictionary dictionaryWithObjectsAndKeys:
        (id)url, QTMovieURLAttribute,
        [NSNumber numberWithBool:NO], QTMovieOpenAsyncOKAttribute,
        nil];
    movie = [[QTMovie movieWithAttributes:attrs error:nil] retain];

	Movie mov = [movie quickTimeMovie];
	TimeValue movtime;
	if (clip_begin) {
		TimeScale movscale = GetMovieTimeScale(mov);
		movtime = (TimeValue)(clip_begin*(double)movscale/1000000.0);
		SetMovieTimeValue(mov, movtime);
	} else {
		movtime = GetMovieTime(mov, nil);
	}
	MoviesTask(mov, 0);
#if 0
	Fixed playRate = GetMoviePreferredRate(mov);
	PrePrerollMovie(mov, movtime, playRate, nil, nil);
	PrerollMovie(mov, movtime, playRate);
	MoviesTask(mov, 0);
#endif
}

+ (void)removeFromSuperview: (QTMovieView *)view
{
	[view pause: self];
	[view removeFromSuperview];
}
@end

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

typedef lib::no_arg_callback<cocoa_video_renderer> poll_callback;

#ifdef OLD_OFFSCREEN_CODE
// Helper routine to forward quicktime redraw notifications to the cocoa_video_renderer
// object. This is installed when needed (when we go to offscreen mode). According
// to the QT docs installing such a redraw handler may slowdown rendering.
static OSErr
movieDidDrawFrame(Movie theMovie, long refCon)
{
	cocoa_video_renderer *r = reinterpret_cast<cocoa_video_renderer*> (refCon);
	r->_qt_did_redraw();
	return 0;
}
#endif

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
	m_url(node->get_url("src")),
	m_movie(NULL),
	m_movie_view(NULL),
	m_offscreen_window(NULL),
	m_offscreen(false),
	m_paused(false)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSURL *nsurl = [NSURL URLWithString: [NSString stringWithCString: m_url.get_url().c_str()]];
//	NSURL *nsurl = [NSURL fileURLWithPath: [NSString stringWithCString: m_url.get_url().c_str()]];
	if (!nsurl) {
		lib::logger::get_logger()->error(gettext("%s: cannot convert to URL"), m_url.get_url().c_str());
		return;
	}
	
	_init_clip_begin_end();
	MovieCreator *mc = [[MovieCreator alloc] init: m_clip_begin];
	[mc performSelectorOnMainThread: @selector(movieWithURL:) withObject: nsurl waitUntilDone: YES];
	m_movie = [mc movie];
	[mc release];
	
	if (!m_movie) {
		lib::logger::get_logger()->error(gettext("%s: cannot open movie"), [[nsurl absoluteString] cString]);
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer: cocoa_video_renderer(0x%x), m_movie=0x%x url=%s clipbegin=%d", this, m_movie, m_url.get_url().c_str(), m_clip_begin);
	
#if 0
	Movie mov = [m_movie quickTimeMovie];
	TimeValue movtime;
	// Calling SetMovieTimeValue here can cause deadlocks, it seems, probably due to
	// a misunderstanding of QT threading intricacies on my side. The workaround
	// is that we call it later, just after we've started playback (in the redraw
	// routine). But: this feels like a hack so I'm not sure this is a real solution
	// and not merely masks the problem.
	if (m_clip_begin) {
		TimeScale movscale = GetMovieTimeScale(mov);
		movtime = (TimeValue)(m_clip_begin*(double)movscale/1000000.0);
		SetMovieTimeValue(mov, movtime);
	} else {
		movtime = GetMovieTime(mov, nil);
	}
	Fixed playRate = GetMoviePreferredRate(mov);
	PrePrerollMovie(mov, movtime, playRate, nil, nil);
	PrerollMovie(mov, movtime, playRate);
	//SetMovieRate(mov, playRate);
#endif
	[pool release];
}

cocoa_video_renderer::~cocoa_video_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer: ~cocoa_video_renderer(0x%x), m_movie=0x%x", this, m_movie);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (m_movie) {
		[m_movie release];
		m_movie = NULL;
	}
	if (m_movie_view) {
		[MovieCreator performSelectorOnMainThread: @selector(removeFromSuperview:) withObject: m_movie_view waitUntilDone: NO];
		m_movie_view = NULL;
		// XXXJACK Should call releaseOverlayWindow here
	}
	if (m_offscreen_window) {
		[m_offscreen_window release];
		m_offscreen_window = NULL;
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
	m_lock.enter();
	if (where > 0) {
		Movie mov = [m_movie quickTimeMovie];
		TimeValue movtime;
		TimeScale movscale = GetMovieTimeScale(mov);
		movtime = (TimeValue)((where+m_clip_begin/1000000.0)*movscale);
		SetMovieTimeValue(mov, movtime);
	}
	m_paused = false;
	m_dest->show(this); // XXX Do we need this?
	m_lock.leave();
}

void
cocoa_video_renderer::stop()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cocoa_video_renderer::stop(0x%x)", this);
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (m_dest) m_dest->renderer_done(this);
	if (m_movie_view) {
		AM_DBG logger::get_logger()->debug("cocoa_video_renderer.stop: removing m_movie_view 0x%x", (void *)m_movie_view);
		[MovieCreator performSelectorOnMainThread: @selector(removeFromSuperview:) withObject: m_movie_view waitUntilDone: NO];
		m_movie_view = NULL;
		// XXXJACK Should call releaseOverlayWindow here
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
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AM_DBG lib::logger::get_logger()->debug("cocoa_video_renderer::resume(0x%x)", this);
	m_paused = false;
	if (m_movie && m_movie_view) {
		if ([m_movie_view isHidden]) [m_movie_view setHidden: NO];
		[m_movie_view play: NULL];
	}
	[pool release];
	m_lock.leave();
}

void
cocoa_video_renderer::seek(double where)
{
	m_lock.enter();
	lib::logger::get_logger()->debug("cocoa_video_renderer::seek(%f)", where);
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
	if (m_movie == NULL || m_movie_view == NULL) {
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
			if (m_movie_view) [m_movie_view pause: NULL];
		}
	}
	
	if (!is_stopped) {
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
	if (!m_movie) {
		m_lock.leave();
		return;
	}
	NSValue *value = [m_movie attributeForKey:QTMovieCurrentSizeAttribute];
	NSSize nssize = [value sizeValue];
	size srcsize = size(int(nssize.width), int(nssize.height));
	rect srcrect;
	rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
	dstrect.translate(m_dest->get_global_topleft());

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	NSRect frameRect = [view NSRectForAmbulantRect: &dstrect];

	if (m_movie && !m_movie_view) {
		// Create the movie view and link it in
		AM_DBG logger::get_logger()->debug("cocoa_video_renderer.redraw: creating movie view");
		m_movie_view = [[QTMovieView alloc] initWithFrame: frameRect];
		[m_movie_view setControllerVisible: NO];
		[view addSubview: m_movie_view];
		[m_movie_view release];
		// Set the movie playing
		[m_movie_view setMovie: m_movie];
		[m_movie_view play: NULL];
		// Create the overlay window
		[view requireOverlayWindow];
		// And start the poll task
		ambulant::lib::event *e = new poll_callback(this, &cocoa_video_renderer::_poll_playing);
		m_event_processor->add_event(e, POLL_INTERVAL, ambulant::lib::ep_low);
	} else {
		//  Need to compare frameRect to current Qt rect and move if needed
        if (!NSEqualRects(frameRect, [m_movie_view frame]) ) {
            [m_movie_view setFrame: frameRect];
        }
	}
	// Set things up so subsequent redraws go to the overlay window
	[view useOverlayWindow];
	
#ifdef OLD_OFFSCREEN_CODE
	_copy_bits(view, frameRect);
#endif
	m_lock.leave();
}

#ifdef OLD_OFFSCREEN_CODE
void
cocoa_video_renderer::_qt_did_redraw()
{
	if (!m_offscreen) return;
	if (!m_dest) return;
	m_dest->need_redraw();
}
	
void
cocoa_video_renderer::_copy_bits(NSView *dst, NSRect& dstrect)
{
	if (!m_offscreen) return;
	AM_DBG NSLog(@"_copy_bits()");
	[m_movie_view lockFocus];
	NSBitmapImageRep *src = [[NSBitmapImageRep alloc] initWithFocusedViewRect: [m_movie_view bounds]];
	NSRect srcrect = [m_movie_view bounds];
	[m_movie_view unlockFocus];
	[dst lockFocus];
	[src drawInRect: dstrect];
	[dst unlockFocus];
	[src release];
}

void cocoa_video_renderer::_go_offscreen()
{
	if (m_offscreen) return;
	m_offscreen = true;
	
	// Set up the redraw-done callback.
    MovieDrawingCompleteUPP myUpp = NewMovieDrawingCompleteUPP (movieDidDrawFrame);
	if (!m_movie) return;
	Movie mov = [m_movie quickTimeMovie];
	if (!mov) return;
	SetMovieDrawingCompleteProc(mov,
		movieDrawingCallWhenChanged,
		myUpp,
		(long)this);
	DisposeMovieDrawingCompleteUPP (myUpp);
	
	// Create the offscreen window, if it isn't there already.
	// For easy coding we make it as big as the normal rect.
	if (!m_offscreen_window) {
		m_onscreen_window = [m_movie_view window];
		NSRect rect = NSMakeRect(0,0,2000,2000); // XXXJACK
		m_offscreen_window = [[NSWindow alloc] initWithContentRect: rect 
			styleMask: NSBorderlessWindowMask 
			backing: NSBackingStoreNonretained
			defer: NO];
	}
	
	// Move the display offscreen
	NSView *contentview = [m_offscreen_window contentView];
	[contentview addSubview: m_movie_view];
}

void
cocoa_video_renderer::_go_onscreen()
{
	if (!m_offscreen) return;
	m_offscreen = false;
	
	// Clear the redraw-done callback
	if (!m_movie) return;
	Movie mov = [m_movie quickTimeMovie];
	if (!mov) return;
	SetMovieDrawingCompleteProc(mov, movieDrawingCallWhenChanged, nil, nil);
	
	// Move the display onscreen.
	assert(m_offscreen_window);
	assert(m_onscreen_window);
	NSView *contentview = [m_onscreen_window contentView];
	[contentview addSubview: m_movie_view];
}
#endif /* OLD_OFFSCREEN_CODE */

} // namespace cocoa

} // namespace gui

} //namespace ambulant
