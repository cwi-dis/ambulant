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


/* CGVideoAVPlayerManager - a class to manage the AVPlayer in iOs 4.0.
 * currently it appears we can use one AVPlayer object at the same time, and the
 * only way to play a next video is to call its replaceCurrentItemWithItem.
 */
@interface CGVideoAVPlayerManager : NSObject
{
//	id timeObserver;			// A periodic observer to maintain video progress
	CMTime m_duration;			// The total duration of the video
	BOOL m_is_duration_known;	// Initially duration is not known
	NSURL* m_nsurl;				// The url of the video
	void*(*m_eod_fun)(void*);	// A C++ function to be called at end of the video
	void* m_eod_arg;			// Arguments for this function 
	void*(*m_err_fun)(void*);	// A C++ function to be called when an error occurs
	void* m_err_arg;			// Arguments for this function 
	BOOL m_observers_added;		// A flag whether any observers are watching the video
	AVPlayerLayer* m_avplayer_layer; // The AVPlayerLayer where video is displayed
	AVPlayerItem* m_avplayer_item; // The video item being played 
	
	ambulant::net::timestamp_t m_position_wanted;
}
static AVPlayer* s_avplayer;	// The global AVPlayer

//@property (nonatomic, retain) AVPlayer *s_avplayer;
@property (nonatomic, retain) AVPlayerItem *m_avplayer_item;
@property (nonatomic, assign) CMTime m_duration;
@property (nonatomic, assign, readonly) BOOL m_is_duration_known;
//@property (nonatomic, retain) id timeObserver;
@property (nonatomic, retain) NSURL* m_nsurl;
@property (readonly) AVPlayerLayer* m_avplayer_layer;

// ** video initialization methods **
// initWithURL:url - returns NULL on error
- (CGVideoAVPlayerManager*) initWithURL:(NSURL*) nsurl;

// onEndOfDataCall:fun witharg:arg - call 'fun' the the opaque 'arg' when end of data is reached while playing video
- (void) onEndOfDataCall:(void*(*)(void*))fun withArg: (void*) arg;

// onErrorCall:fun: witharg:arg - call 'fun' the the opaque 'arg' when an error occures during video play
- (void) onErrorCall:(void*(*)(void*))fun withArg: (void*) arg;

// set_clip_begin:tb end:te - select video begin/end time
- (void) set_clip_begin:(Float64) begin_time end: (Float64) end_time;

// add_layer:layer withSize:size - add a video layer to the specified CALayer
- (void) add_layer: (CALayer*) layer withSize: (CGSize) size;

// set_frame:rect - (re)set the frame of the current video to 'rect'
- (void) set_frame: (CGRect) rect;

// ** player control methods **
- (void) play;
- (void) pause;
- (void) stop;
// return the last error observed, NULL if none
- (NSError*) get_error;
// return the current player rate (0: paused, 1.0:playing at normal speed
- (Float64) rate;

// ** av_player__manager** private methods **
- (AVPlayer*) avplayer;
- (void) add_observers;
- (void) remove_observers;
- (BOOL) s_busy;
- (void) dealloc;

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
//	void freeze() {}
//	void stop();
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
	static void* error_occurred(void* arg);
	enum { rs_created, rs_inited, rs_prerolled, rs_started, rs_stopped, rs_fullstopped, rs_error_state } m_renderer_state; // Debugging, mainly
	net::url m_url;						// The URL of the movie we play
//X	QTMovie *m_movie;           
//X	QTMovieView *m_movie_view;	// The view displaying the movie
//X	AVPlayer* m_avplayer;				// The avplayer itself
	CALayer* m_superlayer;				// The CALayer to which m_avplayer_layer is added
	size m_srcsize;						// size of this view
	CGVideoAVPlayerManager *m_avplayer_manager;			// Our helper ObjC class to control the players using observers
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
