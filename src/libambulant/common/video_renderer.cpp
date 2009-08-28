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
<<<<<<< video_renderer.cpp
 * @$Id$ 
=======
 * @$Id$ 
>>>>>>> 1.42.4.55
 */

#include "ambulant/lib/logger.h"
//#include "ambulant/lib/transition_info.h"
#include "ambulant/common/video_renderer.h"
//#include "ambulant/gui/none/none_gui.h"
//#include "ambulant/net/datasource.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;

video_renderer::video_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node * node,
	lib::event_processor * evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	renderer_playable (context, cookie, node, evp, factory, mdp),
	m_src(NULL),
	m_audio_ds(NULL),
	m_audio_renderer(NULL),
	m_timer(NULL),
	m_epoch(0),
	m_activated(false),
	m_post_stop_called(false),
	m_is_paused(false),
	m_paused_epoch(0),
	m_last_frame_timestamp(-1),
	m_frame_displayed(0),
	m_frame_duplicate(0),
	m_frame_early(0),
	m_frame_late(0),
#ifdef WITH_SEAMLESS_PLAYBACK
	m_previous_clip_position(-1),
#endif
	m_frame_missing(0)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::video_renderer() (this = 0x%x): Constructor ", (void *) this);
	net::url url = node->get_url("src");
	
	_init_clip_begin_end();
	
#ifdef WITH_SEAMLESS_PLAYBACK
	m_previous_clip_position = m_clip_begin;
#endif

	m_src = factory->get_datasource_factory()->new_video_datasource(url,m_clip_begin, m_clip_end);
	if (m_src == NULL) {
		lib::logger::get_logger()->warn(gettext("Cannot open video: %s"), url.get_url().c_str());
		m_lock.leave();
		return;
	}
	
	if (m_src->has_audio()) {
		m_audio_ds = m_src->get_audio_datasource();
	
		if (m_audio_ds) {
			AM_DBG lib::logger::get_logger()->debug("active_video_renderer::active_video_renderer: creating audio renderer !");
			m_audio_renderer = factory->get_playable_factory()->new_aux_audio_playable(context, cookie, node, evp, (net::audio_datasource*) m_audio_ds); //KB XXXX cast
			AM_DBG lib::logger::get_logger()->debug("active_video_renderer::active_video_renderer: audio renderer created(0x%x)!", (void*) m_audio_renderer);
		} else {
			m_audio_renderer = NULL;
		}
	}
	m_lock.leave();
}

video_renderer::~video_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~video_renderer(0x%x)", (void*)this);
	m_lock.enter();
#if 0
    // XXXJACK: need to check that the stop calls aren't needed...
	if (m_audio_renderer){
        m_audio_renderer->stop();
        m_audio_renderer->release();
        m_audio_renderer = NULL;
    }
	if (m_src) {
        m_src->stop();
        m_src->release();
        m_src = NULL;
    }
#else
    if (m_dest) m_dest->renderer_done(this);
    m_dest = NULL;
	if (m_audio_renderer) m_audio_renderer->release();
    m_audio_renderer = NULL;
	if (m_src) m_src->release();
	m_src = NULL;
#endif
	m_lock.leave();
}


void
video_renderer::init_with_node(const lib::node *n)
{
	m_lock.enter();
	renderer_playable::init_with_node(n);
	// Assumption in the following code (by Jack): if we have an audio renderer
    // then the streams are multiplexed, and we should seek only a single stream.
    // We let the audio handler do the seeking, as the video handler can
    // much more easily skip frames, etc.
#ifdef WITH_SEAMLESS_PLAYBACK
    AM_DBG lib::logger::get_logger()->debug("video_renderer::init_with_node: old pos %lld new pos %lld for %s", m_previous_clip_position, m_clip_begin, n->get_sig().c_str());
	if (m_clip_begin != m_previous_clip_position) {
        AM_DBG lib::logger::get_logger()->debug("video_renderer::init_with_node: seek from %lld to %lld for %s", m_previous_clip_position, m_clip_begin, n->get_sig().c_str());
		m_lock.leave();
		seek(m_clip_begin/1000);
		m_lock.enter();
        m_previous_clip_position = m_clip_begin;
	}
#else
		m_lock.leave();
		seek(m_clip_begin/1000);
		m_lock.enter();
#endif // WITH_SEAMLESS_PLAYBACK
	if (m_audio_renderer) {
		m_audio_renderer->init_with_node(n);
	} 
	m_lock.leave();
}


void
video_renderer::start (double where)
{
	m_lock.enter();
	AM_DBG { 
        std::string tag = m_node->get_local_name();
        assert(tag != "prefetch");
    }
	
	if (m_activated) {
		lib::logger::get_logger()->trace("video_renderer.start(0x%x): already started", (void*)this);
		m_post_stop_called = false;
		m_lock.leave();
        // XXXJACK: stopgap for continuous renderering: call show().
        // Interaction renderer/surface needs rethinking.
        if (m_dest) m_dest->show(this);
		return;
	}
	if (!m_src) {
		lib::logger::get_logger()->trace("video_renderer.start: no datasource, skipping media item");
		m_context->stopped(m_cookie, 0);
		m_lock.leave();
		return;
	}
	if (!m_dest) {
		lib::logger::get_logger()->trace("video_renderer.start: no destination surface, skipping media item");
		m_context->stopped(m_cookie, 0);
		m_lock.leave();
		return;
	}
	// Tell the datasource how we like our pixels.
	m_src->set_pixel_layout(pixel_layout());
	m_activated = true;

#if 1
	m_timer = m_event_processor->get_timer();
#else
	// XXX Note: comment below is possibly incorrect, but at  the very least the
	// code does not work, because video_datasource::start_frame() assumes a shared clock.
	//
	// This is a workaround for a bug: the "normal" timer
	// can be set back in time sometimes, and the video renderer
	// does not like that. For now use a private timer, will
	// have to be fixed eventually.
	m_timer = lib::realtime_timer_factory();
#endif

	// Now we need to define where we start playback. This depends on m_clip_begin (microseconds)
	// and where (seconds). We use these to set m_epoch (milliseconds) to the time (m_timer-based)
	// at which we would have played the frame with timestamp 0.
	assert(m_clip_begin >= 0);
	assert(where >= 0);
	m_epoch = m_timer->elapsed() - (long)(m_clip_begin/1000) - (int)(where*1000);
	m_is_paused = false;

	lib::event * e = new dataavail_callback (this, &video_renderer::data_avail);
	AM_DBG lib::logger::get_logger ()->debug ("video_renderer::start(%f) this = 0x%x, cookie=%d, dest=0x%x, timer=0x%x, epoch=%d", where, (void *) this, (int)m_cookie, (void*)m_dest, m_timer, m_epoch);
	m_src->start_frame (m_event_processor, e, 0);
	if (m_audio_renderer) 
		m_audio_renderer->start(where);

#ifdef WITH_SEAMLESS_PLAYBACK
    // We now no longer know where we are (until we get to end-of-clip).
    m_previous_clip_position = -1;
#endif
	m_lock.leave();
	
	// Note by Jack: I'm not 100% sure that calling show() after releasing the lock is safe, but (a)
	// calling it inside the lock leads to deadly embrace (this lock and the one in the destination region,
	// during a redraw) and (b) other renderers also call m_dest->show() without holding the lock.
	m_dest->show(this);
}

void
video_renderer::preroll(double when, double where, double how_much)
{
#ifdef WITH_SEAMLESS_PLAYBACK
	m_lock.enter();
	if (m_activated) {
		lib::logger::get_logger()->trace("video_renderer.preroll(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	if (!m_src) {
		lib::logger::get_logger()->trace("video_renderer.preroll: no datasource, skipping media item");
		m_lock.leave();
		return;
	}
	// Tell the datasource how we like our pixels.
	m_src->set_pixel_layout(pixel_layout());

	// Now we need to define where we start prefetching. This depends on m_clip_begin (microseconds)
	// and where (seconds).
	assert(m_clip_begin >= 0);
	assert(where >= 0);
	m_epoch = (long)(m_clip_begin/1000) - (int)(where*1000);
	m_is_paused = false;
	
	// We need to initial these variables over here
	m_paused_epoch = 0;
	m_last_frame_timestamp = -1;
	m_frame_displayed = 0;
	m_frame_duplicate = 0;
	m_frame_early = 0;
	m_frame_late = 0;
	m_previous_clip_position = m_clip_begin;
	m_frame_missing = 0;
	
	AM_DBG lib::logger::get_logger ()->debug ("video_renderer::start(%f) this = 0x%x, cookie=%d, dest=0x%x, timer=0x%x, epoch=%d", where, (void *) this, (int)m_cookie, (void*)m_dest, m_timer, m_epoch);
	m_src->start_prefetch (m_event_processor);
	if (m_audio_renderer) 
		m_audio_renderer->preroll(where);
	
	m_lock.leave();	
#endif // WITH_SEAMLESS_PLAYBACK
}


bool
video_renderer::stop()
{ 
	m_lock.enter();
	/*AM_DBG*/ lib::logger::get_logger()->debug("video_renderer::stop() this=0x%x, dest=0x%x", (void *) this, (void*)m_dest);

	if (m_audio_renderer) {
		m_audio_renderer->stop();
	} else {
        m_context->stopped(m_cookie, 0);
    }
	m_lock.leave();
	return false; // xxxbo: note, "false" means this renderer is reusable (and still running, needing post_stop() to actually stop)
}

void 
video_renderer::post_stop()
{
	m_lock.enter();
	m_post_stop_called = true;
	if (m_dest) m_dest->renderer_done(this);
    m_dest = NULL;
	if (m_audio_renderer)
		m_audio_renderer->post_stop();
	lib::logger::get_logger()->debug("video_renderer: displayed %d frames; skipped %d dups, %d late, %d early, %d NULL",
									 m_frame_displayed, m_frame_duplicate, m_frame_late, m_frame_early, m_frame_missing);
	m_lock.leave();	
}


void
video_renderer::seek(double t)
{
	//assert(m_audio_renderer == NULL);
	AM_DBG lib::logger::get_logger()->trace("video_renderer: seek(%f) curtime=%f", t, (double)m_timer->elapsed()/1000.0);
    assert( t >= 0);
	long int t_ms = (long int)(t*1000.0);
	if (m_src) m_src->seek(t_ms);
}

common::duration 
video_renderer::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	// video is the important one so we ask the video source
	if (m_src) {
		rv = m_src->get_dur();
		AM_DBG lib::logger::get_logger()->trace("video_renderer: get_dur() duration = %f", rv.second);

	}

	m_lock.leave();
	return rv;
}

// now() returns the time in seconds !
double
video_renderer::now() 
{
	assert( m_timer );
	// private method - no locking
	double rv;
	long elapsed;

	if (m_is_paused)
		elapsed = m_paused_epoch;
	else
		elapsed = m_timer->elapsed();
		
	if (elapsed < m_epoch)
		rv = 0;
	else
		rv = (double)(elapsed - m_epoch) / 1000;
	AM_DBG lib::logger::get_logger()->trace("video_renderer: now(0x%x): m_paused_epoch=%d, m_epoch=%d rv=%lf", this, m_paused_epoch,  m_epoch, rv);
	return rv;
}

void
video_renderer::pause(pause_display d)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::pause() this=0x%x, dest=0x%x", (void *) this, (void*)m_dest);
	// XXX if d==display_hide we should hide the content
	if (m_activated && !m_is_paused) {
		if (m_audio_renderer) 
			m_audio_renderer->pause(d);
		m_is_paused = true;
		assert (m_timer);
		m_paused_epoch = m_timer->elapsed();
	}
	m_lock.leave();
}

void
video_renderer::resume()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::resume() this=0x%x, dest=0x%x", (void *) this, (void*)m_dest);
	if (m_activated && m_is_paused) {
		if (m_audio_renderer) 
			m_audio_renderer->resume();
		m_is_paused = false;
		assert (m_timer);
		unsigned long int pause_length = m_timer->elapsed() - m_paused_epoch;
		m_epoch += pause_length;
	}
	m_lock.leave();
}

void
video_renderer::data_avail()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail(this = 0x%x):", (void *) this);
	if (m_post_stop_called || !m_activated || !m_src) {
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: returning (already shutting down)");
		m_activated = false;
		m_lock.leave();
		return;
	}
	assert(m_dest);
    
	m_size.w = m_src->width();
	m_size.h = m_src->height();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: size=(%d, %d)", m_size.w, m_size.h);
	
	// Get the next frame, dropping any frames whose timestamp has passed. 
	char *buf = NULL;
	int size = 0;
	net::timestamp_t now_micros = (net::timestamp_t)(now()*1000000);
	net::timestamp_t frame_ts_micros;	// Timestamp of frame in "buf" (in microseconds)
	buf = m_src->get_frame(now_micros, &frame_ts_micros, &size);
    AM_DBG lib::logger::get_logger()->debug("data_avail(%s): %lld", m_node->get_sig().c_str(), frame_ts_micros);

	if (buf == NULL) {
		// This can only happen immedeately after a seek, or if we have read past end-of-file.
 		lib::logger::get_logger()->debug("video_renderer::data_avail: get_frame returned NULL");
        if (m_src->end_of_file()) {
            // If we have an audio renderer we let it send the stopped() callback.
            if (m_audio_renderer == NULL)
                m_context->stopped(m_cookie, 0);
        } else {
            assert(m_last_frame_timestamp < 0);
        }
		m_lock.leave();
		return;
	}
	net::timestamp_t frame_duration = m_src->frameduration(); // XXX For now: assume 30fps
	
	// If we are at the end of the clip we stop and signal the scheduler..
#ifndef WITH_SEAMLESS_PLAYBACK
	if (m_src->end_of_file() || (m_clip_end > 0 && frame_ts_micros > m_clip_end)) {
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: stopping playback. eof=%d, ts=%lld, now=%lld, clip_end=%lld ", (int)m_src->end_of_file(), frame_ts_micros, now_micros, m_clip_end );
		if (m_src) {
			m_src->stop();
			m_src->release();
			m_src = NULL;
		}
		m_lock.leave();
        // If we have an audio renderer we should let it do the stopped() callback.
		if (m_audio_renderer == NULL) m_context->stopped(m_cookie, 0);
		//stop(); // XXX Attempt by Jack. I think this is really a bug in the scheduler, so it may need to go some time.
		lib::logger::get_logger()->debug("video_renderer: displayed %d frames; skipped %d dups, %d late, %d early, %d NULL",
			m_frame_displayed, m_frame_duplicate, m_frame_late, m_frame_early, m_frame_missing);
		return;
	}
#else
    AM_DBG { 
        std::string tag = m_node->get_local_name();
        assert(tag != "prefetch");
    }
    if (m_src->end_of_file() || (m_clip_end > 0 && frame_ts_micros > m_clip_end)) {
        AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: stopping playback. eof=%d, ts=%lld, now=%lld, clip_end=%lld ", (int)m_src->end_of_file(), frame_ts_micros, now_micros, m_clip_end );
        // If we have an audio renderer we should let it do the stopped() callback.
        if (m_audio_renderer == NULL) m_context->stopped(m_cookie, 0);
        
        // Remember how far we officially got (discounting any fill=continue behaviour)
        m_previous_clip_position = m_clip_end;
        
        lib::logger::get_logger()->debug("video_renderer: displayed %d frames; skipped %d dups, %d late, %d early, %d NULL",
                                         m_frame_displayed, m_frame_duplicate, m_frame_late, m_frame_early, m_frame_missing);
        
        // If we are past real end-of-file we always stop playback.
        // If we are past clip_end we continue playback if we're playing a fill=ambulant:continue node.
        // XXXJACK: this may lead to multiple stopped() callbacks (above). Need to fix.
        if (m_src->end_of_file() || !is_fill_continue_node()) {
            m_lock.leave();
            return;
        }
    }		
#endif

	AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: buf=0x%x, size=%d, ts=%d, now=%d", (void *) buf, size, (int)frame_ts_micros, (int)now_micros);	
	AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: frame_ts_micros=%lld (<=) now_micros(%lld) + frame_duration(%lld)= %lld", frame_ts_micros, now_micros, frame_duration, now_micros + frame_duration);

	// If we have a frame and it should be on-screen already we show it.
	// If the frame's timestamp is still in the future we fall through, and schedule another
	// callback at the time this frame is due.
	if (!buf) {
		// No data: skip. Probably out-of-memory or something similar.
		m_frame_missing++;
		m_src->frame_processed(frame_ts_micros);
	} else
	if (frame_ts_micros + frame_duration < m_clip_begin) {
		// Frame from before begin-of-movie (funny comparison because of unsignedness). Skip silently, and schedule another callback asap.
		AM_DBG lib::logger::get_logger()->debug("video_renderer: frame skipped, ts (%lld) < clip_begin(%lld)", frame_ts_micros, m_clip_begin);
        m_src->frame_processed(frame_ts_micros);
	} else
#ifdef DROP_LATE_FRAMES
	if (frame_ts_micros <= now_micros - frame_duration && !m_prev_frame_dropped) {
		// Frame is too late. Skip forward to now. Schedule another callback asap.
		AM_DBG lib::logger::get_logger()->debug("video_renderer: skip late frame, ts=%lld, now-dur=%lld", frame_ts_micros, now_micros-frame_duration);
		m_frame_late++;
		frame_ts_micros = now_micros;
		m_src->frame_processed(frame_ts_micros);
		m_prev_frame_dropped = true;
	} else
#endif
	if (frame_ts_micros > now_micros + frame_duration) {
		// Frame is too early. Do nothing, just schedule a new event at the correct time and we will get this same frame again.
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: frame early, ts=%lld, now=%lld, dur=%lld)",frame_ts_micros, now_micros, frame_duration);
		m_frame_early++;
	} else
	if (frame_ts_micros <= m_last_frame_timestamp) {
		// This frame, or a later one, has been displayed already. Skip.
		// We always print this message, as it could potentially denote a bug in the decoder (which should
		// have dropped frames that are earlier than a frame already displayed).
		lib::logger::get_logger()->debug("video_renderer::data_avail: skip frame (ts=%lld), already displayed later frame (ts=%lld)", frame_ts_micros, m_last_frame_timestamp);
		m_frame_duplicate++;
		m_src->frame_processed(frame_ts_micros);
		frame_ts_micros = m_last_frame_timestamp+frame_duration;
	} else {
		// Everything is fine. Display the frame.
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: display frame (timestamp = %lld)",frame_ts_micros);
		_push_frame(buf, size);
		m_src->frame_processed_keepdata(frame_ts_micros, buf);
#ifdef DROP_LATE_FRAMES
		m_prev_frame_dropped = false;
#endif
		// Do the redraw, and do it now: we want video to go smoothly, not wait for
		// random scheduler activity.
		m_dest->need_redraw();
		gui_window *w = m_dest->get_gui_window();
		assert(w);
		w->redraw_now();
		m_last_frame_timestamp = frame_ts_micros;
		m_frame_displayed++;
		
		// Now we need to decide when we want the next callback, by computing what the timestamp
		// of the next frame is expected to be.
		frame_ts_micros += frame_duration;
		if (frame_ts_micros < now_micros - frame_duration) {
			// And if we're lagging more than one frame duration we also skip some
			// frames.
			frame_ts_micros = now_micros - frame_duration;
			m_src->frame_processed(frame_ts_micros);
		}
	}
	AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: start_frame(..., %d)", (int)frame_ts_micros);
	lib::event * e = new dataavail_callback (this, &video_renderer::data_avail);
	// Grmpf. frame_ts_micros is on the movie timescale, but start_frame() expects a time relative to
	// the m_event_processor clock (even though it is in microseconds, not milliseconds). Very bad design,
	// for now we hack around it.
	m_src->start_frame (m_event_processor, e, frame_ts_micros+(m_epoch*1000));
	m_lock.leave();
}

void 
video_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	AM_DBG lib::logger::get_logger ()->debug("video_renderer::redraw (this = 0x%x)", (void *) this);
}
