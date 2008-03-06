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
 * @$Id$ 
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
	common::factories *factory)
:	renderer_playable (context, cookie, node, evp),
	m_src(NULL),
	m_audio_ds(NULL),
	m_audio_renderer(NULL),
	m_timer(NULL),
	m_epoch(0),
	m_activated(false),
	m_is_paused(false),
	m_paused_epoch(0),
	m_last_frame_timestamp(-1),
	m_frame_displayed(0),
	m_frame_duplicate(0),
	m_frame_early(0),
	m_frame_late(0),
	m_frame_missing(0)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::video_renderer() (this = 0x%x): Constructor ", (void *) this);
	net::url url = node->get_url("src");
	
	_init_clip_begin_end();

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
	// m_audio_ds released by audio renderer
	stop(); // releases m_src, m_audio_renderer (in most cases)
	AM_DBG lib::logger::get_logger()->debug("~video_renderer(0x%x)", (void*)this);
	m_lock.enter();
	if (m_audio_renderer) m_audio_renderer->release();
	if (m_src) m_src->release();
	m_src = NULL;
	m_lock.leave();
}

void
video_renderer::start (double where)
{
	m_lock.enter();
	if (m_activated) {
		lib::logger::get_logger()->trace("video_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
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
	if (where) m_src->seek((net::timestamp_t)(where*1000000));
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

	lib::event * e = new dataavail_callback (this, &video_renderer::data_avail);
	AM_DBG lib::logger::get_logger ()->debug ("video_renderer::start(%f) this = 0x%x, cookie=%d, dest=0x%x, timer=0x%x, epoch=%d", where, (void *) this, (int)m_cookie, (void*)m_dest, m_timer, m_epoch);
	m_src->start_frame (m_event_processor, e, 0);
	if (m_audio_renderer) 
		m_audio_renderer->start(where);

	m_dest->show(this);

	m_lock.leave();
}

void
video_renderer::stop()
{ 
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::stop() this=0x%x, dest=0x%x", (void *) this, (void*)m_dest);
	if (!m_activated) {
		lib::logger::get_logger()->trace("video_renderer.stop(0x%x): not started", (void*)this);
		m_lock.leave();
		return;
	}
	m_context->stopped(m_cookie, 0);
	m_activated = false;
	if (m_dest) {
		m_dest->renderer_done(this);
		m_dest = NULL;
	}
	if (m_audio_renderer) {
		m_audio_renderer->stop();
		m_audio_renderer->release();
		m_audio_renderer = NULL;
	}
	if (m_src) {
		m_src->stop();
		m_src->release();
		m_src = NULL;
	}
	lib::logger::get_logger()->debug("video_renderer: displayed %d frames; skipped %d dups, %d late, %d early, %d NULL",
		m_frame_displayed, m_frame_duplicate, m_frame_late, m_frame_early, m_frame_missing);
	m_lock.leave();
}

void
video_renderer::seek(double t)
{
	AM_DBG lib::logger::get_logger()->trace("video_renderer: seek(%f) curtime=%f", t, (double)m_timer->elapsed()/1000.0);
	long int t_ms = (long int)(t*1000.0);
#if 0
	// m_timer is already changed by the scheduler.
	long int delta = t_ms - m_timer->elapsed();  // Positive delta: move forward in time
	m_epoch -= delta;	// Which means the epoch moves back in time
#endif
	if (m_src) m_src->seek(t_ms);
	if (m_audio_renderer) m_audio_renderer->seek(t);
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
	if (!m_activated || !m_src) {
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: returning (already shutting down)");
		m_lock.leave();
		return;
	}
	
	m_size.w = m_src->width();
	m_size.h = m_src->height();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: size=(%d, %d)", m_size.w, m_size.h);
	
	// Get the next frame, dropping any frames whose timestamp has passed. 
	char *buf = NULL;
	int size = 0;
	net::timestamp_t now_micros = (net::timestamp_t)(now()*1000000);
	net::timestamp_t frame_ts_micros;	// Timestamp of frame in "buf" (in microseconds)
#if 0
	// We really want to 
	stop_show_frame();	// Tell renderer previous frame data may become invalid
#endif
	buf = m_src->get_frame(now_micros, &frame_ts_micros, &size);
	net::timestamp_t frame_duration = m_src->frameduration(); // XXX For now: assume 30fps
	
	// If we are at the end of the clip we stop and signal the scheduler.
	if (m_src->end_of_file() || (m_clip_end > 0 && frame_ts_micros > m_clip_end)) {
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: stopping playback. eof=%d, ts=%lld, now=%lld, clip_end=%lld ", (int)m_src->end_of_file(), frame_ts_micros, now_micros, m_clip_end );
		if (m_src) {
			m_src->stop();
			m_src->release();
			m_src = NULL;
		}
		m_lock.leave();
		m_context->stopped(m_cookie, 0);
		//stop(); // XXX Attempt by Jack. I think this is really a bug in the scheduler, so it may need to go some time.
		lib::logger::get_logger()->debug("video_renderer: displayed %d frames; skipped %d dups, %d late, %d early, %d NULL",
			m_frame_displayed, m_frame_duplicate, m_frame_late, m_frame_early, m_frame_missing);
		return;
	}

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
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: skip frame (ts=%lld), aleready displayed earlier (ts=%lld)", frame_ts_micros, m_last_frame_timestamp);
		m_frame_duplicate++;
		m_src->frame_processed(frame_ts_micros);
		frame_ts_micros = m_last_frame_timestamp+frame_duration;
	} else {
		// Everything is fine. Display the frame.
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: display frame (timestamp = %lld)",frame_ts_micros);
		push_frame(buf, size);
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
