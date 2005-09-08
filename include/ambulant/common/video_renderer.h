/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_COMMON_VIDEO_RENDERER_H
#define AMBULANT_COMMON_VIDEO_RENDERER_H

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/datasource.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/renderer_impl.h"

namespace ambulant {

namespace common {

/// Convenience class for video renderer_playables.
/// If your video renderer displays frame-by-frame this is the
/// baseclass to use. It handles reading the video file (through
/// a video_datasource object), calls show_frame for every frame,
/// and splitting out the optional audio track and handing it to
/// an audio playable. video_renderer will also control
/// the audio playable, forwarding play/stop/pause calls that it
/// receives to the audio_playable too.
///
/// So, the only thing you need to provide are a show_frame
/// and a redraw function.
class video_renderer : public common::renderer_playable {
  public:
	video_renderer(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	common::factories *factory);

  	virtual ~video_renderer();
	
	/// Return true if video is paused.
  	bool is_paused() { return m_is_paused; };
	
	/// Return true if video is not playing.
  	bool is_stopped() { return !m_activated;};
	
	/// Return true if video is playing.
  	bool is_playing() { return m_activated; };  
	
	/// Display video data.
	virtual void show_frame(const char* frame, int size) {};
    virtual void redraw(const lib::rect &dirty, common::gui_window *window);
	
	void start(double where);
    void stop();
	void seek(double where);
    void pause();
    void resume();
    void data_avail();
	duration get_dur();
//	void playdone() {};
	
		
  protected:
	lib::size m_size;		///< (width, height) of the video data.
  	net::video_datasource* m_src;	///< video datasource.
  	net::audio_datasource *m_audio_ds;	///< audio datasource.
  	common::playable *m_audio_renderer;	///< the audio playable.
  	empty_playable_notification m_playable_notification;
  private:
	typedef lib::no_arg_callback <video_renderer > dataavail_callback;
	double now();
	lib::timer *m_timer;
	long int m_epoch;
	bool m_activated;
	bool m_is_paused;
	long int m_paused_epoch;
	net::timestamp_t m_last_frame_timestamp;
	long int m_frame_displayed;
	long int m_frame_duplicate;
	long int m_frame_early;
	long int m_frame_late;
	long int m_frame_missing;
	lib::critical_section m_lock;
};

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_VIDEO_RENDERER_H
