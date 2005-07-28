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
 * image = new QImage((uchar*) data, width, height, 32, NULL, 0, QImage::IgnoreEndian);
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

#include "ambulant/lib/logger.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/common/renderer.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"

#ifdef AMBULANT_PLATFORM_UNIX
#include "ambulant/net/posix_datasource.h"
#else
#include "ambulant/net/stdio_datasource.h"
inline double round(double v) {return floor(v+0.5);}
#endif
#ifndef round
#define round(v) ((int)(v+0.5))
#endif


//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;

typedef lib::no_arg_callback<renderer_playable_ds> readdone_callback;

renderer_playable::renderer_playable(
	playable_notification *context,
	cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp) 
:	playable_imp(context, cookie, node, evp),
	m_dest(0),
	m_alignment(0),
	m_activated(false),
	m_erase_never(false),
	m_clip_begin(0),
	m_clip_end(-1)

{
	const char *erase = m_node->get_attribute("erase");
	if (erase && strcmp(erase, "never") == 0)
		m_erase_never = true;
}

void
renderer_playable::start(double t)
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable.start(0x%x)", (void *)this);
	if (m_activated) {
		lib::logger::get_logger()->trace("renderer_playable.start(0x%x): already started", (void*)this);
		return;
	}
	m_activated = true;
	if (!m_dest) {
		lib::logger::get_logger()->trace("renderer_playable.start: no destination surface, skipping media item");
		m_context->stopped(m_cookie, 0);
		return;
	}
	m_dest->show(this);
}

void
renderer_playable::stop()
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable.stop(0x%x)", (void *)this);
	if (!m_activated) {
		lib::logger::get_logger()->trace("renderer_playable.stop(0x%x): not started", (void*)this);
	} else {
		if (m_dest)
			m_dest->renderer_done(this);
	}
	m_activated = false;
}

void
renderer_playable::user_event(const lib::point &where, int what) {
	if (what == user_event_click) m_context->clicked(m_cookie, 0);
	else if (what == user_event_mouse_over) m_context->pointed(m_cookie, 0);
	else assert(0);
}

void
renderer_playable::_init_clip_begin_end()
{
	// here we have to get clip_begin/clip_end from the node
	const char *clip_begin_attr = m_node->get_attribute("clipBegin");
	net::timestamp_t cb = 0;
#ifndef AMBULANT_PLATFORM_WIN32
	char *lastp;
#endif
	
	if (!clip_begin_attr) {
		clip_begin_attr = m_node->get_attribute("clip-begin");
	}
	
	if (clip_begin_attr) {
#ifdef AMBULANT_PLATFORM_WIN32
		cb = _atoi64(clip_begin_attr);
#else
		cb = strtoll(clip_begin_attr, &lastp,0);
#endif
	}
	
	const char *clip_end_attr = m_node->get_attribute("clipEnd");
	net::timestamp_t ce = -1;
	if (!clip_end_attr) {
		clip_end_attr = m_node->get_attribute("clip-end");
	}
	
	if (clip_end_attr) {
#ifdef AMBULANT_PLATFORM_WIN32
		ce = _atoi64(clip_end_attr);
#else
		ce = strtoll(clip_end_attr, &lastp,0);
#endif
	}
	AM_DBG lib::logger::get_logger()->debug("renderer_playable::init_clip_begin_end: cb=%lld, ce=%lld", cb,ce);
	m_clip_begin = cb;
	m_clip_end = ce;
}

renderer_playable_ds::renderer_playable_ds(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory)
:	renderer_playable(context, cookie, node, evp),
	m_src(NULL)
{
	// XXXX m_src = passive_datasource(node->get_url("src"))->activate()
	net::url url = node->get_url("src");
	m_src = factory->df->new_raw_datasource(url);	
}

renderer_playable_ds::~renderer_playable_ds()
{
	AM_DBG lib::logger::get_logger()->debug("~renderer_playable_ds(0x%x)", (void *)this);
}

void
renderer_playable_ds::start(double t)
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_ds.start(0x%x)", (void *)this);
	if (m_activated) {
		lib::logger::get_logger()->trace("renderer_playable_ds.start(0x%x): already started", (void*)this);
		return;
	}
	renderer_playable::start(t);
	if (m_src) {
		lib::event *e = new readdone_callback(this, &renderer_playable_ds::readdone);
		m_src->start(m_event_processor, e);
	} else {
		lib::logger::get_logger()->trace("renderer_playable_ds.start: no datasource, skipping media item");
		m_context->stopped(m_cookie, 0);
	}
}

void
renderer_playable_ds::seek(double t)
{
	lib::logger::get_logger()->trace("renderer_playable_ds: seek(%f) not implemented", t);
}

void
renderer_playable_ds::stop()
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_ds.stop(0x%x)", (void *)this);
	renderer_playable::stop();
	if (m_src)
		m_src->stop();
	m_src = NULL;
}

renderer_playable_dsall::~renderer_playable_dsall()
{
	if (m_data) free(m_data);
}

void
renderer_playable_dsall::readdone()
{
	if (!m_src) return;	
	unsigned cur_size = m_src->size();
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x, size=%d)", (void *)this, cur_size);
	
	if (!m_data)
		m_data = malloc(cur_size);
	else
		m_data = realloc(m_data, m_data_size + cur_size);
	
	if (m_data == NULL) {
		lib::logger::get_logger()->fatal("renderer_playable_dsall.readdone: cannot allocate %d bytes", cur_size);
		// What else can we do...
		m_context->stopped(m_cookie, 0);
	}
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x): calling m_src->get_read_ptr() m_src=0x%x", (void *)this,m_src);
	char *cur_data = m_src->get_read_ptr();

	memcpy((char *)m_data + m_data_size, cur_data, cur_size);
	m_data_size += cur_size;
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x): calling m_src->readdone(%d)", (void *)this,m_data_size);
	m_src->readdone(cur_size);
	
	if (m_src->end_of_file()) {
		// All done
		AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x):  all done, calling need_redraw() and stopped_callback", (void *)this);
		if (m_dest)
			m_dest->need_redraw();
		m_src->stop();
		m_context->stopped(m_cookie, 0);
	} else {
		// Continue reading
		AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x):  more to come, calling m_src->start()", (void *)this);
		lib::event *e = new readdone_callback(this, &renderer_playable_ds::readdone);
		m_src->start(m_event_processor, e);
	}
	
}

global_playable_factory_impl::global_playable_factory_impl()
:   m_default_factory(new gui::none::none_playable_factory())
{
}

global_playable_factory_impl::~global_playable_factory_impl()
{
    // XXXX Should I delete the factories in m_factories? I think
    // so, but I'm not sure...
    delete m_default_factory;
}
    
void
global_playable_factory_impl::add_factory(playable_factory *rf)
{
    m_factories.push_back(rf);
}
    
playable *
global_playable_factory_impl::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
    std::vector<playable_factory *>::iterator i;
    playable *rv;
    
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
        rv = (*i)->new_playable(context, cookie, node, evp);
        if (rv) return rv;
    }
    return m_default_factory->new_playable(context, cookie, node, evp);
}

playable *
global_playable_factory_impl::new_aux_audio_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	net::audio_datasource *src)
{
    std::vector<playable_factory *>::iterator i;
    playable *rv;
    
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
        rv = (*i)->new_aux_audio_playable(context, cookie, node, evp, src);
        if (rv) return rv;
    }
	
    return m_default_factory->new_playable(context, cookie, node, evp);
}

global_playable_factory *
common::get_global_playable_factory()
{
    return new global_playable_factory_impl();
}

void 
video_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	AM_DBG lib::logger::get_logger ()->debug("video_renderer::redraw (this = 0x%x)", (void *) this);
}

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
	m_paused_epoch(0)	
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("video_renderer::video_renderer() (this = 0x%x): Constructor ", (void *) this);
	net::url url = node->get_url("src");
	
	_init_clip_begin_end();

	m_src = factory->df->new_video_datasource(url,m_clip_begin, m_clip_end);
	if (m_src == NULL) {
		lib::logger::get_logger()->warn(gettext("Cannot open video: %s"), url.get_url().c_str());
		m_lock.leave();
		return;
	}
	if (m_src->has_audio()) {
		m_audio_ds = m_src->get_audio_datasource();
	
		if (m_audio_ds) {
			AM_DBG lib::logger::get_logger()->debug("active_video_renderer::active_video_renderer: creating audio renderer !");
			m_audio_renderer = factory->rf->new_aux_audio_playable(context, cookie, node, evp, m_audio_ds);
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
	if (m_audio_renderer) m_audio_renderer->release();
	// m_audio_ds released by audio renderer
	if (m_src) m_src->release();
	m_src = NULL;
	m_lock.leave();
}

void
video_renderer::stop()
{ 
	m_lock.enter();
	AM_DBG lib::logger::get_logger ()->debug ("video_renderer::stop() this=0x%x, dest=0x%x", (void *) this, (void*)m_dest);
	if (!m_activated) {
		lib::logger::get_logger()->trace("video_renderer.stop(0x%x): not started", (void*)this);
		m_lock.leave();
		return;
	}
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
		m_src->release();
		m_src = NULL;
	}
#if 0
	// Assumes we own the timer
	if (m_timer) {
		delete m_timer;
		m_timer = NULL;
	}
#endif
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
	m_activated = true;

#if 1
	m_timer = m_event_processor->get_timer();
#else
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
	m_epoch = m_timer->elapsed() - m_clip_begin/1000 - (int)(where*1000);

	lib::event * e = new dataavail_callback (this, &video_renderer::data_avail);
	AM_DBG lib::logger::get_logger ()->debug ("video_renderer::start(%f) this = 0x%x, dest=0x%x", where, (void *) this, (void*)m_dest);
	m_src->start_frame (m_event_processor, e, 0);
	if (m_audio_renderer) 
		m_audio_renderer->start(where);

	m_dest->show(this);

	m_lock.leave();
}

void
video_renderer::seek(double t)
{
	lib::logger::get_logger()->trace("video_renderer: seek(%f) not implemented", t);
}

common::duration 
video_renderer::get_dur()
{
	//DBG return common::duration(true, 7);
	common::duration rv(false, 0.0);
	common::duration rv2(false, 0.0);

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
	// private method - no locking
	double rv;
	if (m_is_paused)
		rv = (double)(m_paused_epoch - m_epoch) / 1000;
	else
		rv = ((double)m_timer->elapsed() - m_epoch)/1000;
	return rv;
}

void
video_renderer::pause()
{
	m_lock.enter();
	if (m_activated && !m_is_paused) {
		if (m_audio_renderer) 
			m_audio_renderer->pause();
		m_is_paused = true;
		m_paused_epoch = m_timer->elapsed();
	}
	m_lock.leave();
}

void
video_renderer::resume()
{
	m_lock.enter();
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
	net::timestamp_t frame_duration = 33000; // XXX For now: assume 30fps
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
	net::timestamp_t frame_ts_micros;
	buf = m_src->get_frame(now_micros, &frame_ts_micros, &size);
	
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
		return;
	}

	AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: buf=0x%x, size=%d, ts=%d, now=%d", (void *) buf, size, (int)frame_ts_micros, (int)now_micros);	
	// If we have a frame and it should be on-screen already we show it.
	// If the frame's timestamp is still in the future we fall through, and schedule another
	// callback at the time this frame is due.
	if (buf && frame_ts_micros <= now_micros+frame_duration && frame_ts_micros >= m_clip_begin-frame_duration) {
		AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: display frame");
		show_frame(buf, size);
		m_dest->need_redraw();
		m_src->frame_done(frame_ts_micros, true);
		// Now we need to decide when we want the next callback.
		frame_ts_micros += frame_duration;						
	}
	AM_DBG lib::logger::get_logger()->debug("video_renderer::data_avail: start_frame(..., %d)", (int)frame_ts_micros);
	lib::event * e = new dataavail_callback (this, &video_renderer::data_avail);
	m_src->start_frame (m_event_processor, e, frame_ts_micros-m_clip_begin);
	m_lock.leave();
}
