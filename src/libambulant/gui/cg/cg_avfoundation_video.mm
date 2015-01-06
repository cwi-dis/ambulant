// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
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

#ifdef WITH_AVFOUNDATION

#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_avfoundation_video.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/smil2/test_attrs.h"

//#define NS_DBG
#ifndef NS_DBG
#define NS_DBG if(0)
#endif

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// this is basically a re-implementation of the C++ std::pair in ObjC
@interface pair : NSObject
{
	NSObject* first;
	NSObject* second;
}

- (pair*) initWithFirst:(NSObject*) first second: (NSObject*) second;

@property (nonatomic, retain) NSObject* first;
@property (nonatomic, retain) NSObject* second;

@end

@implementation pair
@synthesize first, second;
- (pair*) initWithFirst:(NSObject*) afirst second: (NSObject*) asecond {
	[super init];
	self.first = afirst;
	self.second = asecond;
	return self;
}

@end


@implementation CGVideoAVPlayerManager

// XXXJACK Do these really need to be properties?
@synthesize timeObserver, duration, durationIsKnown, mNSURL;

- (AVPlayer*) mAVPlayer {
	return mAVPlayer;
}

- (void) setMAVPlayer:(AVPlayer*) x {
	return; // setter is no-op
}

- (void) removeTimeObserver {
	NS_DBG NSLog(@"removeTimeObserver(%p)", self);
	if (timeObserver != nil) {
		[mAVPlayer removeTimeObserver:timeObserver];
		timeObserver = nil;
	}
}

- (void) updatePlayState {
	if (mWantToPlay) {
		[mAVPlayer play];
	} else {
		[self pause];
	}		
}

- (void) handleDurationDidChange {
	duration = mAVPlayer.currentItem.asset.duration;
	NS_DBG { NSLog(@"handleDurationDidChange(%p) duration changed to:", self); CMTimeShow(duration); }
	[self updatePlayState];
}

- (void) handlePlayerStatusDidChange {
	AVPlayerStatus playerStatus = mAVPlayer.status;
	NS_DBG NSLog(@"handlePlayerStatusDidChange(%p) status changed to: %d", self, playerStatus);
	mStatus = playerStatus;	
	[self updatePlayState];
}

- (void) handlePlayerCurrentItemAssetDidChange {
	AVPlayerStatus playerStatus = mAVPlayer.status;
	NS_DBG NSLog(@"handlePlayerCurrentItemAssetDidChange(%p) currentItem.asset changed to: %p", self, mAVPlayer.currentItem.asset);
	if (mAVPlayer.currentItem.asset != NULL) {
		[mAVPlayer addObserver:self forKeyPath:@"currentItem.asset.duration" options:0 context:nil];
		mDurationObserver = true;
		mStatus = playerStatus;
		[self updatePlayState];
	}
}

- (void) handlePlayerError {
	AVPlayerStatus playerStatus = mAVPlayer.status;
	NS_DBG NSLog(@"handlePlayerError(%p) Error: status changed to: %d", self, playerStatus);
	NSError* error = self.mAVPlayer.currentItem.error;
	if (error != NULL) {
		NS_DBG NSLog(@"Error is0: %@", error);
	}
}

- (void) handlePlayerItemDidReachEnd:(NSNotification*) notification {
	NS_DBG NSLog(@"handlePlayerItemDidReachEnd(%p): player.status=%d", self, self.mAVPlayer.status);
	if (eod_fun) {
		eod_fun(fun_arg);
	}
	[mAVPlayer pause];
}

- (void) addTimeObserver {
	NS_DBG NSLog(@"addTimeObserver(%p)", self);
	[self removeTimeObserver];
	timeObserver = [mAVPlayer addPeriodicTimeObserverForInterval:CMTimeMakeWithSeconds(1, NSEC_PER_SEC) queue:nil usingBlock:^(CMTime time) {
//TBD	[checkIfAtEndOfMovie];
	}];
	
}

- (CGVideoAVPlayerManager*) initWithURL:(NSURL*) nsurl parent: (void*)arg endOfData: (void*(*)(void*))fun {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	mAVPlayer = [[[AVPlayer alloc] initWithURL:nsurl] retain];
	NS_DBG NSLog(@"CGVideoAVPlayerManager.initWithURL(%p) nsurl=%@ self.retainCount=%d mAVPlayer=%p [mAVPlayer retainCount]=%d", self, nsurl, [self retainCount], mAVPlayer, [mAVPlayer retainCount]);
	fun_arg = arg;
	eod_fun = fun;
	[mAVPlayer addObserver:self forKeyPath:@"status" options:0 context:nil];	
	[mAVPlayer addObserver:self forKeyPath:@"currentItem.asset" options:0 context:nil];
	[mAVPlayer addObserver:self forKeyPath:@"currentItem.error" options:0 context:nil];
	mAssetObserver = mErrorObserver = mStatusObserver = true;
	mDurationObserver = false;
	mWantToPlay = false;
	mAVPlayer.actionAtItemEnd = AVPlayerActionAtItemEndPause;

	// prepare to react on end of media data 
	[[NSNotificationCenter defaultCenter]
		addObserver:self
		selector:@selector(handlePlayerItemDidReachEnd:)
		name:AVPlayerItemDidPlayToEndTimeNotification
		object: [[self mAVPlayer] currentItem]];
	
	mStatus = [[self mAVPlayer] status];
	mNSURL = nsurl;
	
	[pool release];
	return self;
}

- (void) dealloc {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NS_DBG NSLog(@"dealloc(%p)", self);
	[[NSNotificationCenter defaultCenter]
		removeObserver:self
		name:AVPlayerItemDidPlayToEndTimeNotification
		object: [[self mAVPlayer] currentItem]];
	if (mStatusObserver) {
		[mAVPlayer removeObserver:self forKeyPath:@"status"];
	}
	if (mAssetObserver) {
		[mAVPlayer removeObserver:self forKeyPath:@"currentItem.asset"];
	}
	if (mDurationObserver) {
		[mAVPlayer removeObserver:self forKeyPath:@"currentItem.asset.duration"];
	}
	if (mErrorObserver) {
		[mAVPlayer removeObserver:self forKeyPath:@"currentItem.error"];
	}
	NS_DBG NSLog(@"mAVPlayer.retainCount=%d",[mAVPlayer retainCount]);
	
	[mAVPlayer release];	
	[mNSURL release];
	
	[pool release];
	[super dealloc];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
	NS_DBG NSLog(@"observeValueForKeyPath(%p) keyPath=%@", self, keyPath);
	if ([keyPath isEqualToString:@"status"] && object == mAVPlayer)
		[self handlePlayerStatusDidChange];
	else if ([keyPath isEqualToString:@"currentItem.asset"] && object == mAVPlayer)
		[self handlePlayerCurrentItemAssetDidChange];
	else if ([keyPath isEqualToString:@"currentItem.asset.duration"] && object == mAVPlayer)
		[self handleDurationDidChange];
	else if ([keyPath isEqualToString:@"currentItem.error"] && object == mAVPlayer)
		[self handlePlayerError];
}

- (void) play {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NS_DBG NSLog(@"play(%p) mStatus=%d", self, mStatus);
	if (mStatus == AVPlayerStatusReadyToPlay) {
		[mAVPlayer play];
	}
	mWantToPlay = true;
	[pool release];
}

- (void) pause {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NS_DBG NSLog(@"pause(%p) mStatus=%d", self, mStatus);
	if (mStatus == AVPlayerStatusReadyToPlay) {
		[mAVPlayer pause];
	}
	mWantToPlay = false;
	[pool release];
}

- (void) removeFromSuperAndRelease:(pair*) objs {
	NS_DBG NSLog(@"removeFromSuper(%p)", self);
	if (objs != NULL) {
		UIView* uiview = (UIView*) objs.first;
		CALayer* uilayer = (CALayer*) objs.second;
		NS_DBG NSLog(@"removeFromSuper(%p): uiview=%p uilayer=%p", self, uiview, uilayer);
		CALayer *superlayer = [uiview layer];
		if (superlayer != NULL) {
			NSMutableArray* sublayers = [NSMutableArray arrayWithArray: superlayer.sublayers];
			NSUInteger idx = [sublayers indexOfObject:uilayer];
			if (idx == NSNotFound) {
				[sublayers removeObjectAtIndex:idx];
				superlayer.sublayers = [NSArray arrayWithArray: sublayers];		
			}
		}
		[objs release];
	}
	[self release];
}

- (void) removeLayer: (CALayer*) layer fromView: (UIView*) uiview {
	pair* objs = [[[pair alloc] init] initWithFirst: uiview second: layer];
	[self performSelectorOnMainThread:@selector(removeFromSuperAndRelease:) withObject: objs waitUntilDone: NO];
}

- (void) setClipBegin:(Float64) beginTime end: (Float64) endTime {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NS_DBG NSLog(@"set_clip_begin(%p):%f end:%f", self, beginTime, endTime);
	if (beginTime >= 0.0) {
		mBeginTime = CMTimeMakeWithSeconds(beginTime, 1);
		mBeginTime.timescale = USEC_PER_SEC;
		[mAVPlayer seekToTime: mBeginTime];
	}
	if (endTime >= 0) {
		mEndTime = CMTimeMakeWithSeconds(endTime, 1);
		mEndTime.timescale = USEC_PER_SEC;
	}
	if (mDurationIsKnown) {
		mAVPlayer.currentItem.forwardPlaybackEndTime = mEndTime;
	}
	[pool release];
}
@end
										  
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
	m_avplayer_manager(NULL),
	m_avplayer_layer(NULL),
	m_avplayer_view(NULL),
	m_visible(false),
	m_previous_clip_position(-1),
	m_renderer_state(rs_created)
{
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(): %p created", (void*)this);
}
	
cg_avfoundation_video_renderer::~cg_avfoundation_video_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_avfoundation_video_renderer(%p) m_dest=%p [m_avplayer_manager retainCount]=%d", (void *)this, m_dest, m_avplayer_manager == NULL ? -999 : [m_avplayer_manager retainCount]);

	if (m_avplayer_manager != NULL) {
		if (m_visible) {
			[m_avplayer_manager removeLayer: m_avplayer_layer fromView: m_avplayer_view];
		} else {
			[m_avplayer_manager release];
		}
		m_avplayer_layer = NULL; 
		m_avplayer_manager = NULL;
	}
	m_lock.leave();
}

void
cg_avfoundation_video_renderer::init_with_node(const lib::node *n)
{
	m_lock.enter();
	renderer_playable::init_with_node(n);
	// can be re-used (from cache) in any state
	assert(m_renderer_state != rs_unknown);
	if (m_renderer_state == rs_unknown) {
		m_lock.leave();
		return;
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	m_renderer_state = rs_initialized;
	m_node = n;
	if (m_avplayer_manager == NULL) {
		assert(m_url.is_empty_path() || m_url.same_document(m_node->get_url("src")));
		// Apparently the first call.
		m_url = m_node->get_url("src");
		NSURL *nsurl = [[NSURL URLWithString: [NSString stringWithCString: m_url.get_url().c_str() encoding: NSUTF8StringEncoding]] retain];
		if (!nsurl) {
			lib::logger::get_logger()->error(gettext("%s: cannot convert to URL"), m_url.get_url().c_str());
			goto bad;
		}
		m_avplayer_manager = [[CGVideoAVPlayerManager alloc] initWithURL:nsurl parent:this endOfData:  &ambulant::gui::cg::cg_avfoundation_video_renderer::eod_reached];
	}
	_init_clip_begin_end();

	[m_avplayer_manager setClipBegin:(Float64) m_clip_begin end:(Float64) m_clip_end];
	
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::init_with_node, [m_avplayer_manager mAVPlayer]=%p, url=%s, clipbegin=%d", this, [m_avplayer_manager mAVPlayer], m_url.get_url().c_str(), m_clip_begin);
	
bad:
	[pool release];
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::init_with_node, [m_avplayer_manager retainCount]=%d", (void *)this, m_avplayer_manager == NULL ? -999 : [m_avplayer_manager retainCount]);
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::start(double where) {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::start, [m_avplayer_manager mAVPlayer]=%p, [m_avplayer_manager retainCount]=%d where=%lf", this, [m_avplayer_manager mAVPlayer], [m_avplayer_manager retainCount], where);

	if (m_avplayer_manager && m_renderer_state != rs_error_state) {
		m_dest->show(this);
		[m_avplayer_manager setClipBegin: where end: -1];
		[m_avplayer_manager play];
		m_renderer_state = rs_playing;
		m_context->started(m_cookie, where);
	}

	m_lock.leave();
}

bool
cg_avfoundation_video_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::stop, [m_avplayer_manager avplayer]=%p, [m_avplayer_manager retainCount]=%d", this, [m_avplayer_manager mAVPlayer],  [m_avplayer_manager retainCount]);
	m_lock.enter();
	bool rv = true;
	if (m_avplayer_manager && m_renderer_state != rs_error_state) {
		m_context->stopped(m_cookie);
		m_renderer_state = rs_stopping;
	}
	m_lock.leave();
	return rv;
}

void
cg_avfoundation_video_renderer::post_stop() {
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::post_stop, [m_avplayer_manager mAVPlayer]=%p, [m_avplayer_manager retainCount]=%d", this, [m_avplayer_manager mAVPlayer], [m_avplayer_manager retainCount]);
	m_lock.enter();
	m_renderer_state = rs_fullstopped;
	m_context->stopped(m_cookie);
	if (m_dest != NULL) {
		m_dest->renderer_done(this); //already done by smil_player::stop_playable()
	}
	m_lock.leave();
}

void
cg_avfoundation_video_renderer::resume() {
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::resume, [m_avplayer_manager mAVPlayer]=%p, rate=%lf", this, [m_avplayer_manager mAVPlayer], [[m_avplayer_manager mAVPlayer] rate]);
	m_lock.enter();
	if (m_renderer_state == rs_paused) {
		[m_avplayer_manager play];
		m_renderer_state = rs_playing;
		if (m_dest != NULL) {
			m_dest->need_redraw();
		}
	}
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::pause(pause_display d) {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::pause, [m_avplayer_manager mAVPlayer]=%p, rate=%lf, pause_display=%d" , this, [m_avplayer_manager mAVPlayer], [[m_avplayer_manager mAVPlayer] rate], d);
	[m_avplayer_manager pause];
	m_renderer_state = rs_paused;
	if (m_dest != NULL) {
		m_dest->need_redraw();
	}
	m_lock.leave();
}
	
common::duration 
cg_avfoundation_video_renderer::get_dur() {
	common::duration rv = common::duration(false, 0);
	if ([m_avplayer_manager durationIsKnown]) {
		CMTime cm_duration = [m_avplayer_manager duration]; 
		if (cm_duration.flags == kCMTimeFlags_Valid)
			rv = common::duration(true, (double) cm_duration.value * (double) cm_duration.timescale);
	}
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::get_dur returns: %d, %lf " , this, (bool) rv.first, (double) rv.second);
	return rv;
}
	
void*
cg_avfoundation_video_renderer::eod_reached(void* arg) {
	cg_avfoundation_video_renderer* cavr = (cg_avfoundation_video_renderer*) arg;
	if (cavr != NULL) {
		AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(%p)::eod_eached, [m_avplayer_manager avplayer]=%p" , cavr, cavr->m_avplayer_manager.mAVPlayer);
		cavr->stop();
	}
	return NULL;
}
	
void
cg_avfoundation_video_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	if (m_renderer_state == rs_error_state) {
		m_lock.leave();
		return;
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	const rect &r = m_dest->get_rect();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	
	AM_DBG logger::get_logger()->debug("cg_avfoundation_video_renderer.redraw(%p, local_ltrb=(%d,%d,%d,%d)), [m_avplayer_manager retainCount]=%d m_renderer_state=%d", (void *)this, r.left(), r.top(), r.right(), r.bottom(), [m_avplayer_manager retainCount], m_renderer_state);
	
	assert(m_renderer_state == rs_playing ||  m_renderer_state == rs_paused || m_renderer_state == rs_stopping || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped || m_renderer_state == rs_error_state);
	if (! (m_renderer_state == rs_playing ||  m_renderer_state == rs_paused || m_renderer_state == rs_stopping || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped || m_renderer_state == rs_error_state)
		|| m_avplayer_manager == NULL || [[m_avplayer_manager mAVPlayer] error] != NULL) {
		[pool release];
		m_lock.leave();
		if (m_renderer_state != rs_error_state && m_avplayer_manager != NULL) {
			m_renderer_state = rs_error_state;
			NSString* ns_error = [[[m_avplayer_manager mAVPlayer] error] localizedDescription];
			lib::logger::get_logger()->error("cg_video: %s", [ns_error cStringUsingEncoding:NSUTF8StringEncoding]);
		}
		return;
	}
	if (m_avplayer_layer == NULL) {		
		// Determine current position and size.
		CALayer *superlayer = [view layer];
		m_avplayer_layer = [AVPlayerLayer playerLayerWithPlayer:[m_avplayer_manager mAVPlayer]];
		m_avplayer_view = view;
		[superlayer addSublayer:m_avplayer_layer];
		m_visible = true;
		CGSize cgsize = [m_avplayer_layer preferredFrameSize];
		if (cgsize.width == 0 || cgsize.height == 0) {
			cgsize.width = r.width();
			cgsize.height = r.height();
		}
		m_srcsize = size(int(cgsize.width), int(cgsize.height));
	}
	if (m_visible) {
		rect srcrect;
		rect dstrect = m_dest->get_fit_rect(m_srcsize, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());
		CGRect frameRect = CGRectFromAmbulantRect(dstrect);
		m_avplayer_layer.frame = frameRect;
	}
	if (m_renderer_state == rs_stopping) {
		if (m_visible) {
			[m_avplayer_manager pause];
			[m_avplayer_layer removeFromSuperlayer];
			m_avplayer_layer = NULL;
		}
		m_renderer_state = rs_stopped;
		m_visible = false;
		[m_avplayer_manager release];
		m_avplayer_manager = NULL;
	} else if (m_renderer_state == rs_fullstopped) {
		if (m_visible && m_avplayer_layer != NULL) {
			[m_avplayer_layer removeFromSuperlayer];
			m_visible = false;
			m_avplayer_layer = NULL;
			m_renderer_state = rs_error_state;
		}
		[m_avplayer_manager release];
		m_avplayer_manager = NULL;
	}

	[pool release];
	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant
#endif // WITH_AVFOUNDATION

