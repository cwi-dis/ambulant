/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
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

#ifndef AMBULANT_GUI_CG_CG_AVFOUNDATION_VIDEO_H
#define AMBULANT_GUI_CG_CG_AVFOUNDATION_VIDEO_H
//XXXX #define WITH_AVFOUNDATION 1
#include "ambulant/gui/cg/cg_renderer.h"
#include "ambulant/common/video_renderer.h"
#include "ambulant/lib/mtsync.h"
#import <AVFoundation/AVFoundation.h>

//#ifdef	__OBJC__
@interface CGVideoAVPlayerManager : NSObject
{
	AVPlayer* mAVPlayer;
	CMTime duration;
	BOOL mDurationIsKnown;
	CMTime mBeginTime;
	CMTime mEndTime;
	id timeObserver;
	NSURL* mNSURL;
	void*(*eod_fun)(void*);
	void* fun_arg;
	AVPlayerStatus mStatus;
	BOOL mCurrentItemObserver;
	BOOL mAssetObserver;
	BOOL mDurationObserver;
	BOOL mStatusObserver;
	BOOL mErrorObserver;
	BOOL mWantToPlay;
	
	ambulant::net::timestamp_t position_wanted;
}

// XXXJACK Do these really need to be properties?
@property (nonatomic, retain) AVPlayer *mAVPlayer;
@property (nonatomic, assign) CMTime duration;
@property (nonatomic, assign, readonly) BOOL durationIsKnown;
@property (nonatomic, retain) id timeObserver;
@property (nonatomic, retain) NSURL* mNSURL;

// initialize AVPlayer with 'url' and call 'fun' on endOfData with 'parent' as its argument 
- (CGVideoAVPlayerManager *)initWithURL:(NSURL*)url parent: (void*)arg endOfData: (void*(*)(void*))fun;
// set 'beginTime' and 'endTime' for the media to be played
- (void) setClipBegin:(Float64) beginTime end: (Float64) endTime;
// return the current AVPlayer
- (AVPlayer*) mAVPlayer;
// (re)start playing
- (void) play;
// pause playing
- (void) pause;
// remove the 'layer' from the 'uiview' that was associated with the player and release
// callable from main thread only
- (void) removeLayer: (CALayer*) layer fromView: (UIView*) uiview;
// private
// - (void) dealloc;
@end
//#endif//__OBJC__
										  
namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cg {

class cg_avfoundation_video_renderer :
	public renderer_playable {
  public:
	cg_avfoundation_video_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp);
	~cg_avfoundation_video_renderer();

	void start(double where);
//	void freeze() {}
//	void stop();
	bool stop();
	void post_stop(); 
	void init_with_node(const lib::node *n);
	void preroll(double when, double where, double how_much) {}
	void pause(pause_display d=display_show);
	void play();
	void resume();
	void seek(double t) {}

	common::duration get_dur();
	
	void redraw(const rect &dirty, gui_window *window);
	void set_intransition(const lib::transition_info *info) {} 
	void start_outtransition(const lib::transition_info *info) {}
private:
	static void* eod_reached(void* arg);
	// state values control stages of operation
	enum {
		rs_unknown		= 0x0000,	// sanity check
		rs_created		= 0x0001,	// set in constructor, valid in 'initialize_with_node': basic initialization
		rs_initialized	= 0x0002,	// set in 'initialize_with_node', valid in 'start': full initialization
		rs_prerolled	= 0x0004,	// unused: prefetch not supported
		rs_playing		= 0x0008,	// set in 'start', play', valid in 'redraw', 'play', 'pause', 'stop', 'full_stop', destructor: plays
		rs_paused		= 0x0010,	// set in 'pause', valid in 'redraw', 'play', 'pause', 'start', 'stop', 'full_stop', destructor 
		rs_stopping		= 0x0020,	// set in 'stop', valid in 'redraw', 'full_stop', destructor: is paused, wants to stop
		rs_stopped		= 0x0040,	// set in 'redraw', valid in 'full_stop', destructor: is stopped, wants full stop
		rs_fullstopped	= 0x0080,	// set in 'full_stop', valid in destructor: avplayer is removed from screen and released
		rs_error_state	= 0x0100,	// may be set in all members, valid in all members: disables all operations, no error recovery.

	} m_renderer_state;

	net::url m_url;					// The URL of the media we play
	AVPlayerLayer* m_avplayer_layer;// The AVPlayerLayer where media is displayed
	CALayer* m_superlayer;			// The CALayer to which m_avplayer_layer is added
	UIView* m_avplayer_view;		// The view for the avplayer
	size m_srcsize;					// size of this view
	CGVideoAVPlayerManager *m_avplayer_manager;	// The helper ObjC class to control AVPlayer using observers
	bool m_visible;					// Flag indicating whether the player layer is added on the layer stack
	net::timestamp_t m_previous_clip_position; // Where we are officially positioned
	lib::timer::signed_time_type m_video_epoch;    // Ambulant clock value corresponding to video clock 0.
	void _fix_video_epoch();    // Set m_video_epoch according to current movie time
	void _fix_clock_drift();    // Synchronise movie clock and ambulant clock
	critical_section m_lock;
};

} // namespace cg

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_CG_CG_AVFOUNDATION_VIDEO_H
