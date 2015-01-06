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

#ifndef AMBULANT_COMMON_VIDEO_RENDERER_H
#define AMBULANT_COMMON_VIDEO_RENDERER_H

#define WITH_LIVE_VIDEO_FEEDBACK

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/datasource.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/smil2/params.h"

namespace ambulant {

namespace common {

/// Convenience class for video renderer_playables.
/// If your video renderer displays frame-by-frame this is the
/// baseclass to use. It handles reading the video file (through
/// a video_datasource object), calls push_frame for every frame,
/// and splitting out the optional audio track and handing it to
/// an audio playable. video_renderer will also control
/// the audio playable, forwarding play/stop/pause calls that it
/// receives to the audio_playable too.
///
/// So, the only thing you need to provide are a push_frame
/// and a redraw function.
class video_renderer : public common::renderer_playable {
  public:
	/// Constructor.
	video_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp);

	virtual ~video_renderer();

	/// The pixel format this renderer wants. Override in subclass to fit
	/// what the hardware wants (so we don't need to do an extra pass of
	/// byte reordering).
	virtual net::pixel_order pixel_layout() { return net::pixel_argb; }

	/// Callback routine passed to the datasource, to be called when new data is available.
	void data_avail();

	virtual void redraw(const lib::rect &dirty, common::gui_window *window);

	virtual void set_surface(common::surface *dest) {
		common::renderer_playable::set_surface(dest);
		if (m_audio_renderer) {
			renderer *r = m_audio_renderer->get_renderer();
			if (r) r->set_surface(dest);
		}
	}
	void start(double where);
	//void stop();
	bool stop();
	void post_stop();
	void preroll(double when, double where, double how_much);
	void init_with_node(const lib::node *n);
	void seek(double where);
	void pause(pause_display d=display_show);
	void resume();
	duration get_dur();

  protected:
	/// Display video data. Subclass providing this method is responsible for
	/// eventually free()ing frame. This method is protected because it shares
	/// the m_lock mutex.
	virtual void _push_frame(char* frame, size_t size) = 0;
	// Signal back from the renderer that it has actually displayed a frame
	void _frame_was_displayed();

  protected:
	lib::size m_size;		///< (width, height) of the video data.
	net::video_datasource* m_src;	///< video datasource.
	net::audio_datasource *m_audio_ds;	///< audio datasource.
	common::playable *m_audio_renderer;	///< the audio playable.
	net::timestamp_t m_last_frame_timestamp;
  private:
	typedef lib::no_arg_callback <video_renderer > dataavail_callback;
	double _now();
	void _resync(lib::timer::signed_time_type drift);
	lib::timer *m_timer;
	long int m_epoch;
	bool m_activated;			// True if a datasource callback is outstanding
	bool m_post_stop_called;	// True if we are expecting only one more callback
	bool m_is_stalled;			// True if we emitted a stalled() feedback call
	bool m_is_paused;
	long int m_paused_epoch;
	long int m_frame_received;	// Number of frames received from the decoder
	long int m_frame_duplicate;	// Number of frames with a duplicate timestamp
	long int m_frame_early;		// Number of frames that were too early
	long int m_frame_late;		// Number of frames that were too late
	long int m_frame_missing;	// Number of frames that were missing (?)
	long int m_frame_displayed;	// Number of frames actually displayed (in the redraw callback)
	net::timestamp_t m_last_frame_displayed_timestamp;	// Timestamp of last frame actually displayed
	net::timestamp_t m_previous_clip_position;
#ifdef WITH_LIVE_VIDEO_FEEDBACK
    const char *m_video_feedback_var;
#endif
  protected:
	lib::critical_section m_lock;	///< Critical section.
};

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_VIDEO_RENDERER_H
