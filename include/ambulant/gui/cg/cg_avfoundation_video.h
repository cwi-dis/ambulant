/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
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

#ifndef AMBULANT_GUI_CG_CG_AVFOUNDATION_VIDEO_H
#define AMBULANT_GUI_CG_CG_AVFOUNDATION_VIDEO_H

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

@property (nonatomic, retain) AVPlayer *mAVPlayer;
@property (nonatomic, assign) CMTime duration;
@property (nonatomic, assign, readonly) BOOL durationIsKnown;
@property (nonatomic, retain) id timeObserver;
@property (nonatomic, retain) NSURL* mNSURL;

- (CGVideoAVPlayerManager *)initWithURL:(NSURL*)url parent: (void*)arg endOfData: (void*(*)(void*))fun;
- (void) set_clip_begin:(Float64) begin_time end: (Float64) end_time;
- (AVPlayer*) mAVPlayer;
- (void) play;
- (void) pause;
- (void) dealloc;

//- (void)setPositionWanted: (ambulant::net::timestamp_t)begintime;
//- (QTMovie *)movie;
//- (void)movieWithURL: (NSURL*)url;
//- (void)moviePrepare: (id)sender;
//- (void)movieStart: (id)sender;
//- (void)movieRelease: (id)sender;
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
	// control values: the action triggered for redraw() executing from main thread is controlled by these values 
	enum { rs_created, rs_inited, rs_prerolled, rs_started, rs_paused, rs_stopping, rs_stopped, rs_fullstopped, rs_error_state } m_renderer_state;
	net::url m_url;						// The URL of the movie we play
	AVPlayerLayer* m_avplayer_layer;	// The AVPlayerLayer where video is displayed
	CALayer* m_superlayer;				// The CALayer to which m_avplayer_layer is added
	UIView* m_avplayer_view;			// The view for the avplayer
	size m_srcsize;						// size of this view
	CGVideoAVPlayerManager *m_avplayer_manager;	// The helper ObjC class to control the players using observers
	bool m_stopped;						// Flag indicating whether its layer is added on the layer stack
	net::timestamp_t m_previous_clip_position; // Where we are officially positioned
#ifdef WITH_CLOCK_SYNC
	lib::timer::signed_time_type m_video_epoch;    // Ambulant clock value corresponding to video clock 0.
	void _fix_video_epoch();    // Set m_video_epoch according to current movie time
	void _fix_clock_drift();    // Synchronise movie clock and ambulant clock
#endif
	critical_section m_lock;
};

} // namespace cg

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_CG_CG_AVFOUNDATION_VIDEO_H
