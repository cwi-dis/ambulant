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

// Used to call a C++ function from Objective C. 
extern "C" void* call_C_function(void* args, void*(*fun)(void*arg)) {
	return fun(args);
};
						
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_avfoundation_video.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/smil2/test_attrs.h"

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef	__OBJC__
//#ifdef JUNK
@interface pair : NSObject
{
	NSObject* first;
	NSObject* second;
}

- (pair*) initWithFirst:(NSObject*) first Second: (NSObject*) second;

@property (nonatomic, retain) NSObject*first;
@property (nonatomic, retain) NSObject*second;

@end

@implementation pair
@synthesize first, second;
- (pair*) initWithFirst:(NSObject*) afirst Second: (NSObject*) asecond {
	[super init];
	self.first = afirst;
	self.second = asecond;
	return self;
}

@end
//#endif //JUNK

@implementation CGVideoAVPlayer

//X @synthesize timeObserver, duration, durationIsKnown, url, playerStatus;
@synthesize duration, durationIsKnown, url, playerStatus;

- (AVPlayer*)
avplayer {
	return avplayer;
}

- (void)
setAvplayer:(AVPlayer*) x {
	return; // setter is no-op
}

#ifdef	JUNK
// we use AVPlayerItemDidPlayToEndTimeNotification instead of checkIfAtEndOfMovie
// however, for an user interface accurately displaying the current stream position,
// using a timeObserver is more appropriate
- (void)
addTimeObserver {
	[self removeTimeObserver];
	timeObserver = [avplayer addPeriodicTimeObserverForInterval:CMTimeMakeWithSeconds(1, NSEC_PER_SEC) queue:nil usingBlock:^(CMTime time) {
		//X		[updateControls];
		//TBD		[checkIfAtEndOfMovie];
	}];
	
}

- (void)
removeTimeObserver {
	if (timeObserver != nil) {
		[avplayer removeTimeObserver:timeObserver];
		timeObserver = nil;
	}
}
#endif //JUNK

- (void)
handleDurationDidChange {
	duration = avplayer.currentItem.asset.duration;
	NSLog(@"duration changed to:"); CMTimeShow(duration);
//X	[self updateControls];
}

- (void)
handlePlayerStatusDidChange {
	AVPlayerStatus avplayerStatus = avplayer.status;
	NSLog(@"status changed to: %d", avplayerStatus);
	if (avplayerStatus == AVPlayerStatusReadyToPlay) {
//		[avplayer play];
	}
}

- (void)
handlePlayerError {
	AVPlayerStatus avplayerStatus = avplayer.status;
	NSLog(@"Error: status changed to: %d", avplayerStatus);
	NSError* error = self.avplayer.currentItem.error;
	if (error != NULL) {
		NSLog(@"Error is0: %@", error);
//		[error release];
	}
}

- (void)
handlePlayerItemDidReachEnd:(NSNotification*) notification {
	NSLog(@"handlePlayerItemDidReachEnd: player.status=%d",self.avplayer.status);
	if (eod_fun) {
		eod_fun(fun_arg);
	}
	[avplayer pause];
	[avplayer seekToTime:kCMTimeZero];	
	
}

-(void) addObservers {
	playerStatus = CGVideoAVPlayerInitialized;
	[avplayer addObserver:self forKeyPath:@"status" options:0 context:nil];
	[avplayer addObserver:self forKeyPath:@"currentItem.asset.duration" options:0 context:nil];
	[avplayer addObserver:self forKeyPath:@"currentItem.error" options:0 context:nil];
	avplayer.actionAtItemEnd = AVPlayerActionAtItemEndPause;
	// prepare to react after keyboard show/hide
	[[NSNotificationCenter defaultCenter]
	 addObserver: self
	 selector: @selector(handlePlayerItemDidReachEnd:)
	 name: AVPlayerItemDidPlayToEndTimeNotification
	 object: [[self avplayer] currentItem]];	
}

- (CGVideoAVPlayer*)
initWithURL:(NSURL*) nsurl //parent: (void*)arg endOfData: (void*(*)(void*))fun {
{
	bool initial = false;
	if (avplayer == NULL) {
		playerStatus = CGVideoAVPlayerNull;
		initial = true;
		avplayer = [[AVPlayer alloc] retain];
		NSLog(@"CGVideoAVPlayerManager.initWithURL(%@) self=0x%x self.retainCount=%d avplayer=0x%x [avplayer retainCount]=%d", nsurl, self, [self retainCount], avplayer, [avplayer retainCount]);
//		fun_arg = arg;
//		eod_fun = fun;
	} else {
		NSLog(@"CGVideoAVPlayerManager.initWithURL(0%x) nsurl=%@:", self,  nsurl);
	}
	if (initial) {
		[avplayer initWithURL: nsurl];
	} else {
		AVPlayerItem* new_item = [[AVPlayerItem alloc] initWithURL: nsurl];
		[avplayer replaceCurrentItemWithPlayerItem:new_item]; 			
	}
	[self addObservers];
	[self handleDurationDidChange];
	[self handlePlayerStatusDidChange];
	url = nsurl;
//	[self addTimeObserver];
	
	return self;
}

- (void)
dealloc {
	NSLog(@"CGVideoAVPlayerManager.dealloc(0x%x): self.retainCount=%d avplayer=0x%x [avplayer retainCount]=%d",  self, [self retainCount], avplayer, [avplayer retainCount]);
//	NSLog(@"avplayer.retainCount=%d",[avplayer retainCount]);
//	[avplayer release];	
	[url release];
	
	[super dealloc];
}

- (void)
observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
	NSLog(@"CGVideoAVPlayerManager.observeValueForKeyPath(0x%x) keyPath=%@ self.retainCount=%d avplayer=0x%x [avplayer retainCount]=%d", self, keyPath, [self retainCount], avplayer, [avplayer retainCount]);
	if ([keyPath isEqualToString:@"status"] && object == avplayer)
		[self handlePlayerStatusDidChange];
	else if ([keyPath isEqualToString:@"currentItem.asset.duration"] && object == avplayer)
		[self handleDurationDidChange];
	else if ([keyPath isEqualToString:@"currentItem.error"] && object == avplayer)
		[self handlePlayerError];
}

- (void)
play {
	NSLog(@"CGVideoAVPlayerManager.play(0x%x) self.retainCount=%d avplayer=0x%x [avplayer retainCount]=%d", self, [self retainCount], avplayer, [avplayer retainCount]);
	if (playerStatus == CGVideoAVPlayerInitialized || playerStatus == CGVideoAVPlayerPausing) {
		playerStatus = CGVideoAVPlayerPlaying;
		[avplayer play];
	}
}

- (void)
pause {
	NSLog(@"CGVideoAVPlayerManager.pause(0x%x) self.retainCount=%d avplayer=0x%x [avplayer retainCount]=%d", self, [self retainCount], avplayer, [avplayer retainCount]);
	if (playerStatus == CGVideoAVPlayerPlaying) {
		[avplayer pause];
		playerStatus == CGVideoAVPlayerPausing;
	}
}

- (void)
stop {
	NSLog(@"CGVideoAVPlayerManager.stop(0x%x) self.retainCount=%d avplayer=0x%x [avplayer retainCount]=%d", self, [self retainCount], avplayer, [avplayer retainCount]);
	if (playerStatus == CGVideoAVPlayerPlaying || playerStatus == CGVideoAVPlayerPausing) {
		if (playerStatus == CGVideoAVPlayerPlaying) {
			[avplayer pause];
		}
		playerStatus = CGVideoAVPlayerInitialized;
	}
}

- (void)
terminate {
	NSLog(@"CGVideoAVPlayerManager.terminate(0x%x) self.retainCount=%d avplayer=0x%x [avplayer retainCount]=%d", self, [self retainCount], avplayer, [avplayer retainCount]);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	//	[avplayer removeTimeObserver:timeObserver];
	//	[timeObserver release];
	if (playerStatus == CGVideoAVPlayerPlaying || playerStatus == CGVideoAVPlayerPausing) {
		[avplayer pause];
	}		
	[[NSNotificationCenter defaultCenter]
	 removeObserver:self
	 name:AVPlayerItemDidPlayToEndTimeNotification
	 object: [[self avplayer] currentItem]];
	[avplayer removeObserver:self forKeyPath:@"status"];
	[avplayer removeObserver:self forKeyPath:@"currentItem.asset.duration"];
	[avplayer removeObserver:self forKeyPath:@"currentItem.error"];	
	playerStatus = CGVideoAVPlayerNull;
	[pool release];
}

- (void)
removeFromSuper:(pair*) objs {
	if (objs == NULL)
		return;
	UIView* uiview = (UIView*) objs.first;
	CALayer* uilayer = (CALayer*) objs.second;
	NSLog(@"removeFromSuper(0x%x): uiview=0x%x uilayer=0x%x", self, uiview, uilayer);
	CALayer *superlayer = [uiview layer];
	NSMutableArray* sublayers = [NSMutableArray arrayWithArray: superlayer.sublayers];
	NSUInteger idx = [sublayers indexOfObject:uilayer];
	if (idx >= 0) {
		[sublayers removeObjectAtIndex:idx];
		superlayer.sublayers = [NSArray arrayWithArray: sublayers];		
	}
}
										 
@end

@implementation CGVideoAVPlayerManager

static NSMutableArray* players = NULL;
		
+ (CGVideoAVPlayer*)
getCGVideoAVPlayerWithURL: (NSURL*) url
{
	CGVideoAVPlayer* player = NULL;
	if (players == NULL) {
		players = [[NSMutableArray alloc] init];
	}
	for (int i = 0; i < [players count]; i++) {
		player = (CGVideoAVPlayer*) [players objectAtIndex:i];
		if (player.playerStatus == CGVideoAVPlayerNull) {
			AVPlayerItem* playerItem = [[AVPlayerItem alloc] initWithURL: url];
			[player.avplayer replaceCurrentItemWithPlayerItem:playerItem];
			[player addObservers];
			player.playerStatus = CGVideoAVPlayerInitialized;
			[players replaceObjectAtIndex: i withObject: player];
			return player;
		}
	}
	player = [[CGVideoAVPlayer alloc] initWithURL: url];
	player.playerStatus = CGVideoAVPlayerInitialized;
	[players addObject: player];
	return player;
}

+ (void)
releaseCGVideoAVPlayer: (CGVideoAVPlayer*) aplayer
{
	for (int i = 0; i < [players count]; i++) {
		CGVideoAVPlayer* player = (CGVideoAVPlayer*) [players objectAtIndex:i];
		if (player == aplayer) {
			aplayer.playerStatus = CGVideoAVPlayerNull;
			[players replaceObjectAtIndex: i withObject: aplayer];
			return;
		}
	}
}
@end

#endif//__OBJC__
										  
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

// static
//X CGVideoAVPlayerManager* cg_avfoundation_video_renderer::s_avplayer_manager = NULL;

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
	m_paused(false),
	m_previous_clip_position(-1),
	m_renderer_state(rs_created)
{
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(): 0x%x created", (void*)this);
}
	
cg_avfoundation_video_renderer::~cg_avfoundation_video_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_avfoundation_video_renderer(0x%x) [m_avplayer retainCount]=%d", (void *)this, m_avplayer == NULL ? -999 : [m_avplayer retainCount]);
	pair* objs = [[pair alloc] initWithFirst:m_avplayer_view Second:m_avplayer_layer];	
	
	if (m_avplayer_view != NULL) {
		m_lock.leave();
		[m_avplayer performSelectorOnMainThread:@selector(removeFromSuper:)withObject: objs waitUntilDone:YES];
		m_lock.enter();
		m_avplayer_layer = NULL;
		m_avplayer_view = NULL;
	}
	[objs release];

	if (m_avplayer_layer != NULL) {
		m_avplayer_layer == NULL;
	}
	if (m_avplayer != NULL) {
		[m_avplayer terminate];
		m_avplayer = NULL;
	}
	m_lock.leave();
}

void
cg_avfoundation_video_renderer::init_with_node(const lib::node *n)
{
	m_lock.enter();
	CMTime cm_clip_begin = CMTimeMakeWithSeconds(0.0, 1.0), cm_clip_end;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	renderer_playable::init_with_node(n);
	assert(m_renderer_state == rs_created || m_renderer_state == rs_prerolled || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped);
	m_renderer_state = rs_inited;	
	m_node = n;
	if (m_avplayer == NULL) {
		// Apparently the very first call.
		m_avplayer = [CGVideoAVPlayer alloc];
	}
	if (m_url.is_empty_path()) {
		m_url = m_node->get_url("src");
		NSURL *nsurl = [NSURL URLWithString: [NSString stringWithCString: m_url.get_url().c_str() encoding: NSUTF8StringEncoding]];
		if (nsurl == NULL) {
			lib::logger::get_logger()->error(gettext("%s: cannot convert to URL"), m_url.get_url().c_str());
			goto bad;
		}
//TBD		[m_avplayer initWithURL:nsurl parent:this endOfData: &ambulant::gui::cg::cg_avfoundation_video_renderer::eod_reached];
		m_avplayer = [CGVideoAVPlayerManager getCGVideoAVPlayerWithURL:nsurl];
		if (m_avplayer == NULL) {
			goto bad;
		}
	}
	_init_clip_begin_end();
	if (m_clip_begin != m_previous_clip_position) {
		m_previous_clip_position = m_clip_begin;
		cm_clip_begin = CMTimeMakeWithSeconds((Float64)m_clip_begin, 1);
		cm_clip_begin.timescale = USEC_PER_SEC;
		[m_avplayer.avplayer seekToTime: cm_clip_begin];
	}
	cm_clip_end = CMTimeMakeWithSeconds((Float64)m_clip_end, 1);
	cm_clip_end.timescale = USEC_PER_SEC;
	m_avplayer.avplayer.currentItem.forwardPlaybackEndTime = cm_clip_end;	

	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::init_with_node, m_avplayer=0x%x, url=%s, clipbegin=%d", this, m_avplayer, m_url.get_url().c_str(), m_clip_begin);
	
bad:
	[pool release];
//	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::init_with_node, [m_avplayer retainCount]=%d", (void *)this, m_avplayer == NULL ? -999 : [m_avplayer retainCount]);
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::start(double where) {

	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::start, m_avplayer=0x%x, [m_avplayer retainCount]=%d where=%lf", this, m_avplayer, [m_avplayer retainCount], where);

	if (m_avplayer) {
		m_dest->show(this);
		m_renderer_state = rs_started;
		m_context->started(m_cookie, where);
//		m_dest->need_redraw();
	}

	m_lock.leave();
}

bool
cg_avfoundation_video_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::stop, m_avplayer=0x%x, [m_avplayer retainCount]=%d", this, m_avplayer,  [m_avplayer retainCount]);
	m_lock.enter();
	bool rv = true;
	m_context->stopped(m_cookie);
	m_renderer_state = rs_stopped;
	m_lock.leave();
	return rv;
}
	
void
cg_avfoundation_video_renderer::post_stop() {
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::post_stop, m_avplayer=0x%x, [m_avplayer retainCount]=%d", this, m_avplayer, [m_avplayer retainCount]);
	m_lock.enter();
	m_renderer_state = rs_fullstopped;
	if (m_dest != NULL) {
		m_dest->need_redraw();
//		m_dest->renderer_done(this); //already done by smil_player::stop_playable()
	}
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::resume() {
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::resume, m_avplayer=0x%x, rate=%lf", this, m_avplayer, [m_avplayer.avplayer rate]);
	m_lock.enter();
	m_renderer_state = rs_started;
	if (m_dest != NULL) {
		m_dest->need_redraw();
	}
	m_lock.leave();
}
	
void
cg_avfoundation_video_renderer::pause(pause_display d) {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::pause, m_avplayer=0x%x, rate=%lf, pause_display=%d" , this, m_avplayer, [m_avplayer.avplayer rate], d);
	m_renderer_state = rs_stopped;
	if (m_dest != NULL) {
		m_dest->need_redraw();
	}
	m_lock.leave();
}

common::duration 
cg_avfoundation_video_renderer::get_dur() {
	common::duration rv = common::duration(false, 0);
	if ([m_avplayer durationIsKnown]) {
		CMTime cm_duration = [m_avplayer duration]; 
		if (cm_duration.flags = kCMTimeFlags_Valid)
			rv = common::duration(true, (double) cm_duration.value * (double) cm_duration.timescale);
	}
	 AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::get_dur returns: %d, %lf " , this, (bool) rv.first, (double) rv.second);
	 return rv;
}
	

void*
cg_avfoundation_video_renderer::eod_reached(void* arg) {
	cg_avfoundation_video_renderer* cavr = (cg_avfoundation_video_renderer*) arg;
	AM_DBG lib::logger::get_logger()->debug("cg_avfoundation_video_renderer(0x%x)::eod_eached, m_avplayer=0x%x" , cavr, cavr->m_avplayer.avplayer);
	if (cavr != NULL) {
		cavr->stop();
	}
	return NULL;
}
	
void
cg_avfoundation_video_renderer::redraw(const rect &dirty, gui_window *window)
{
	if (m_avplayer == NULL) {
		return;
	}
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	const rect &r = m_dest->get_rect();
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	
	AM_DBG logger::get_logger()->debug("cg_avfoundation_video_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)),  [m_avplayer retainCount]=%d", (void *)this, r.left(), r.top(), r.right(), r.bottom(), [m_avplayer retainCount]);
	
	assert(m_renderer_state == rs_started || m_renderer_state == rs_stopped || m_renderer_state == rs_fullstopped|| m_renderer_state == rs_error_state);
	if (m_avplayer == NULL || [m_avplayer.avplayer error] != NULL) {
		[pool release];
		m_lock.leave();
		if (m_renderer_state != rs_error_state && m_avplayer != NULL) {
			m_renderer_state = rs_error_state;
			NSString* ns_error = [[m_avplayer.avplayer error] localizedDescription];
			lib::logger::get_logger()->error("cg_video: %s", [ns_error cStringUsingEncoding:NSUTF8StringEncoding]);
		}
		return;
	}
//X	AVPlayerStatus status = [m_avplayer status];
	if (m_avplayer_layer == NULL) {		
		// Determine current position and size.
		CALayer *superlayer = [view layer];
		m_avplayer_layer = [AVPlayerLayer playerLayerWithPlayer:m_avplayer.avplayer];
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

	}

	[pool release];
	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant
#endif // WITH_AVFOUNDATION

