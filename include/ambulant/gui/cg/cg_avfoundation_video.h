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

/* Interfacing with AVFoundation video by a single static CGVideoAVPlayerManager
 * controlling a pool of (possible parallel) AVPlayer objects
 */
typedef enum CGVideoAVPlayerStatus_decl {
	CGVideoAVPlayerNull, CGVideoAVPlayerInitialized, CGVideoAVPlayerPlaying, CGVideoAVPlayerPausing, CGVideoAVPlayerError
} CGVideoAVPlayerStatus;

@interface CGVideoAVPlayer : NSObject
{
	AVPlayer* avplayer;
	CMTime duration;
//X	id timeObserver;
	BOOL durationIsKnown;
	NSURL* url;	
	void*(*eod_fun)(void*);	// extern function to call on end of data
	void* fun_arg;			// arguments for this function
	
	ambulant::net::timestamp_t position_wanted;
	
	CGVideoAVPlayerStatus playerStatus;
}

@property (nonatomic, retain) AVPlayer *avplayer;
@property (nonatomic, assign) CMTime duration;
@property (nonatomic, assign, readonly) BOOL durationIsKnown;
//X @property (nonatomic, retain) id timeObserver;
@property (nonatomic, retain) NSURL* url;
@property (nonatomic, assign) CGVideoAVPlayerStatus_decl playerStatus;

- (CGVideoAVPlayer*) initWithURL:(NSURL*)url; // parent:(void*)an_object endOfData:(void*(*)(void*)) a_method_pointer;
- (AVPlayer*) avplayer;
- (void) play;
- (void) pause;
- (void) stop;
- (void) terminate;
- (void) dealloc;
@end

@interface CGVideoAVPlayerManager : NSObject
{
	NSMutableArray* avplayers;
}

+ (CGVideoAVPlayer*) getCGVideoAVPlayerWithURL:(NSURL*) url;
+ (void) releaseCGVideoAVPlayer: (CGVideoAVPlayer*) player;
@end
										  
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
	bool stop();
	void post_stop();
	void init_with_node(const lib::node *n);
	void preroll(double when, double where, double how_much) {}
	void pause(pause_display d=display_show);
	void resume();
	void seek(double t) {}

	common::duration get_dur();
	
	void redraw(const rect &dirty, gui_window *window);
	void set_intransition(const lib::transition_info *info) {} 
	void start_outtransition(const lib::transition_info *info) {}
private:
	static void* eod_reached(void* arg);
	enum { rs_created, rs_inited, rs_prerolled, rs_started, rs_stopped, rs_fullstopped, rs_error_state } m_renderer_state; // Debugging, mainly
	net::url m_url;						// The URL of the movie we play
	AVPlayerLayer* m_avplayer_layer;	// The AVPlayerLayer where video is displayed
	CALayer* m_superlayer;				// The CALayer to which m_avplayer_layer is added
	UIView* m_avplayer_view;			// The view for the avplayer
	size m_srcsize;						// size of this view
	CGVideoAVPlayer* m_avplayer;			// Helper ObjC class to control the actual player using observers
	bool m_paused;
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
