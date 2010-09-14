// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
 * @$Id$
 */

#ifdef WITH_AVFOUNDATION

#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_avfoundation_video.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// These two constants should match. Moreover, the optimal setting may depend on the
// specific hardware.
// XXXJACK: we should get rid of these, analoguous to what cocoa_dsvideo does:
// Get the information dynamically.
#if 1
#define MY_PIXEL_LAYOUT net::pixel_argb
#define MY_BITMAP_INFO (kCGImageAlphaNoneSkipFirst|kCGBitmapByteOrder32Host)
#define MY_BPP 4
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

extern const char cg_avfoundation_video_playable_tag[] = "video";
	extern const char cg_avfoundation_video_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCoreGraphics");
	extern const char cg_avfoundation_video_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererAVFoundation");
	extern const char cg_avfoundation_video_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererVideo");

common::playable_factory *
create_cg_avfoundation_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{

	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCoreGraphics"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererAVFoundation"), true);
	return new common::single_playable_factory<
		cg_avfoundation_video_renderer,
		cg_avfoundation_video_playable_tag,
		cg_avfoundation_video_playable_renderer_uri,
		cg_avfoundation_video_playable_renderer_uri2,
		cg_avfoundation_video_playable_renderer_uri3>(factory, mdp);
}

cg_avfoundation_video_renderer::cg_avfoundation_video_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	renderer_playable(context, cookie, node, evp, factory, mdp),
	m_url(),
	m_avplayer(NULL),
	m_avplayer_layer(NULL),
	m_avplayer_view(NULL),
	m_mc(NULL),
	m_paused(false),
	m_previous_clip_position(-1),
	m_renderer_state(rs_created)
{
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(): 0x%x created", (void*)this);
}

cg_avfoundation_video_renderer::~cg_avfoundation_video_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_avfoundation_video_renderer(0x%x)", (void *)this);
	if (m_avplayer_layer != NULL) {
		m_avplayer_layer == NULL;
	}
	if (m_avplayer != NULL) {
		[m_avplayer release];
		m_avplayer = NULL;
	}
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::init_with_node(const lib::node *n)
{
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	renderer_playable::init_with_node(n);
	assert(m_renderer_state == rs_created || m_renderer_state == rs_prerolled || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped);
	m_renderer_state = rs_inited;
	CMTime cm_clip_begin;
	
	m_node = n;
	if (m_avplayer == NULL) {
		assert(m_mc == NULL);
		assert(m_url.is_empty_path() || m_url.same_document(m_node->get_url("src")));
		// Apparently the first call.
		m_url = m_node->get_url("src");
		NSURL *nsurl = [NSURL URLWithString: [NSString stringWithCString: m_url.get_url().c_str() encoding: NSUTF8StringEncoding]];
		if (!nsurl) {
			lib::logger::get_logger()->error(gettext("%s: cannot convert to URL"), m_url.get_url().c_str());
			goto bad;
		}
		m_avplayer = [[[AVPlayer alloc] initWithURL:nsurl] autorelease];
		[m_avplayer retain];
#ifdef JNK		
		m_mc = (void*)[[[MovieCreator alloc] init] retain];
		[(MovieCreator *)m_mc performSelectorOnMainThread: @selector(movieWithURL:) withObject: nsurl waitUntilDone: YES];
		m_movie = [(MovieCreator *)m_mc movie];
	}
	if (!m_movie) {
		lib::logger::get_logger()->error(gettext("%s: cannot open movie"), m_url.get_url().c_str());
		goto bad;
#endif//JNK
	}
	_init_clip_begin_end();
	cm_clip_begin = CMTimeMakeWithSeconds((Float64)m_clip_begin, 1);
	cm_clip_begin.timescale = USEC_PER_SEC;
	[m_avplayer seekToTime: cm_clip_begin];
#ifdef JNK			
	if (m_clip_begin != m_previous_clip_position) {
		[(MovieCreator *)m_mc setPositionWanted: m_clip_begin];
	}
	m_previous_clip_position = m_clip_begin;
#endif//JNK
	
	/*AM_DBG*/ lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::init_with_node, m_player=0x%x, url=%s, clipbegin=%d", this, m_avplayer, m_url.get_url().c_str(), m_clip_begin);
	
bad:
	[pool release];
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::start(double where) {
	m_lock.enter();
	/*AM_DBG*/ lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::start, m_player=0x%x, where=%lf", this, m_avplayer, where);

	if (m_avplayer) {
		m_dest->show(this);
		m_renderer_state = rs_started;
//		m_dest->need_redraw();
	}

	m_lock.leave();
}

bool
cg_avfoundation_video_renderer::stop() {
	/*AM_DBG*/ lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::stop, m_player=0x%x", this, m_avplayer);
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	bool rv = true;
	m_renderer_state = rs_fullstopped;
//	[m_avplayer pause];

	m_dest->need_redraw();
	[pool release];
	m_lock.leave();
	return rv;
}
	
void
cg_avfoundation_video_renderer::resume() {
	/*AM_DBG*/ lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::resume, m_player=0x%x, rate=%lf", this, m_avplayer, [m_avplayer rate]);
	m_lock.enter();
	m_renderer_state = rs_started;
	m_dest->need_redraw();
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::pause(pause_display d) {
	m_lock.enter();
	/*AM_DBG*/ lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::pause, m_player=0x%x, rate=%lf, pause_display=%d" , this, m_avplayer, [m_avplayer rate], d);
	m_renderer_state = rs_stopped;
	m_dest->need_redraw();
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	const rect &r = m_dest->get_rect();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	
	/*AM_DBG*/ logger::get_logger()->debug("cg_avfoundation_video_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	assert(m_renderer_state == rs_started || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped);
	if (!m_avplayer) {
		m_context->stopped(m_cookie);
		[pool release];
		m_lock.leave();
		return;
	}
	if (m_avplayer_layer == NULL) {		
		// Determine current position and size.
		CALayer *superlayer = [view layer];
		m_avplayer_layer = [AVPlayerLayer playerLayerWithPlayer:m_avplayer];
		m_avplayer_view = view;
		[superlayer addSublayer:m_avplayer_layer];		
		CGSize cgsize = [m_avplayer_layer preferredFrameSize];
		if (cgsize.width == 0 || cgsize.height == 0) {
			cgsize.width = r.width();
			cgsize.height = r.height();
		}
		m_srcsize = size(int(cgsize.width), int(cgsize.height));
	}
	rect srcrect;
	rect dstrect = m_dest->get_fit_rect(m_srcsize, &srcrect, m_alignment);
	dstrect.translate(m_dest->get_global_topleft());
	CGRect frameRect = [view CGRectForAmbulantRectForLayout: &dstrect];
	m_avplayer_layer.frame = frameRect;

	if (m_renderer_state == rs_started) {
		[m_avplayer play];
	} else if (m_renderer_state == rs_stopped) {
		[m_avplayer pause];
	} else if (m_renderer_state == rs_fullstopped) {
		m_context->stopped(m_cookie);
		if (m_avplayer_view != NULL) {
			CALayer *superlayer = [m_avplayer_view layer];
			NSMutableArray* sublayers = [NSMutableArray arrayWithArray: superlayer.sublayers];
			NSUInteger idx = [sublayers indexOfObject:m_avplayer_layer];
			[sublayers removeObjectAtIndex:idx];
			superlayer.sublayers = [NSArray arrayWithArray: sublayers];
			m_avplayer_layer = NULL;
		}
		[m_avplayer release];
		m_avplayer = NULL;
	}

#ifdef JNK		
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
#endif//JNK
	[pool release];
	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant
#endif // WITH_AVFOUNDATION

