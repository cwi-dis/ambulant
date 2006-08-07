// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/gstreamer/gstreamer_audio_renderer.h"
#include "ambulant/gui/gstreamer/gstreamer_player.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/common/region_info.h"

#include <stdlib.h>

using namespace ambulant;
using namespace gui::gstreamer;


// ***************** gstreamer_audio_renderer **************************

typedef lib::no_arg_callback<gui::gstreamer::gstreamer_audio_renderer> readdone_callback;

gstreamer_audio_renderer::gstreamer_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory)
:	common::renderer_playable(context, cookie, node, evp),
	m_player(NULL),
	m_pipeline(NULL),
	m_is_playing(false),
	m_is_paused(false),
	m_read_ptr_called(false),
	m_volcount(0)
#ifdef USE_SMIL21
	,
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
#endif                                   
{
        init_player(node);
}

gstreamer_audio_renderer::gstreamer_audio_renderer(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	common::factories* factory,
	net::audio_datasource *ds)
:	common::renderer_playable(context, cookie, node, evp),
	m_player(NULL),
	m_pipeline(NULL),
	m_is_playing(false),
	m_is_paused(false),
	m_read_ptr_called(false)
#ifdef USE_SMIL21
	,
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
#endif                                   
{
        init_player(node);
}

gstreamer_audio_renderer::~gstreamer_audio_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::~gstreamer_audio_renderer(0x%x) m_url=%s",  this, m_url.get_url().c_str());		
	if (m_player && m_is_playing) {
	//	m_lock.leave();
		//TBD stop it
		m_player->stop();
	//	m_lock.enter();
	}	
#ifdef USE_SMIL21
	if (m_transition_engine) {
		delete m_transition_engine;
		m_transition_engine = NULL;
	}
#endif                                   
	m_is_playing = false;
	if (m_player) delete m_player;
	m_player = NULL;
	m_lock.leave();
}

extern "C" {
void
gstreamer_audio_renderer_pipeline_store(void* obj, GstElement*pipeline) {
	gstreamer_audio_renderer* rend = (gstreamer_audio_renderer*) obj;
	if (rend) rend->m_pipeline = pipeline;
}
}

void
gstreamer_audio_renderer::init_player(const lib::node *node) {
	assert (node);
	m_url = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::init_player(0x%x) url=",  this, m_url.get_url().c_str());
	_init_clip_begin_end();
	m_player = new gstreamer_player(m_url.get_url().c_str(), this);
	m_player->init();
	m_context->started(m_cookie, 0);
	//KB	if (gst_player failed)m_context->stopped(m_cookie, 0);
}
#ifdef USE_SMIL21

void
gstreamer_audio_renderer::set_intransition(const lib::transition_info* info) {
 	if (m_transition_engine)
		delete m_transition_engine;
	m_intransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, false, info);
}

void
gstreamer_audio_renderer::start_outtransition(const lib::transition_info* info) {
 	if (m_transition_engine)
		delete m_transition_engine;
	m_outtransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, true, info);
}
#endif

bool
gstreamer_audio_renderer::is_supported(const lib::node *node)
{
	if ( ! node)
    		return false;
	std::string mimetype(node->get_url("src").guesstype());
	
#ifdef  WITH_NOKIA770
	if (mimetype == "audio/mpeg") // .mp3
		return true;
#else //WITH_NOKIA770
	if (mimetype == "audio/wav") // .wav
		return true;
#endif//WITH_NOKIA770
	return false;
}

bool
gstreamer_audio_renderer::is_paused()
{
	m_lock.enter();
	bool rv;
	rv = m_is_paused;
	m_lock.leave();
	return rv;
}

bool
gstreamer_audio_renderer::is_stopped()
{
	m_lock.enter();
	bool rv;
	rv = !m_is_playing;
	m_lock.leave();
	return rv;
}

bool
gstreamer_audio_renderer::is_playing()
{
	m_lock.enter();
	bool rv;
	rv = m_is_playing;
	m_lock.leave();
	return rv;
}


void
gstreamer_audio_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer::stop(0x%x)",(void*)this);
	if (m_player && m_is_playing) {
		m_lock.leave();
		m_player->stop_player();
		m_lock.enter();
		// XXX Should we call stopped_callback?
		m_context->stopped(m_cookie, 0);
	}
	m_is_playing = false;
	m_lock.leave();
}

void
gstreamer_audio_renderer::pause()
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.pause(0x%x)", (void *)this);
	m_lock.enter();
	if (m_player)
		m_player->pause();
	m_is_paused = true;
	m_lock.leave();
}

void
gstreamer_audio_renderer::resume()
{
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.resume(0x%x)", (void *)this);
	m_lock.enter();
	if (m_player)
		m_player->play();
	m_is_playing = true;
	m_is_paused = false;
	m_lock.leave();
}

void
gstreamer_audio_renderer::start(double where)
{
	double microsec = 1e6;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.start(0x%x): url=%s, where=%f", (void *)this, m_url.get_url().c_str(),where);
	pause();
	seek(where);
	resume(); // turn on playing
	m_lock.enter();
	m_context->started(m_cookie, 0);
	m_lock.leave();
}


void
gstreamer_audio_renderer::seek(double where)
{
       lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek(%f)", where);
       m_lock.enter();
       if (m_player) {
              double microsec = 1e6;
	      guint64 where_guint64;
	      where += (m_clip_begin / microsec);
	      where_guint64 = llrint(where)* GST_SECOND;	      
	      lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek() where=%f, where_guint64=%lu", where, where_guint64);
	      m_player->mutex_acquire("gstreamer_audio_renderer::seek");
	      if ( ! gst_element_seek(m_player->gst_player(), 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, where_guint64, GST_SEEK_TYPE_NONE, 0)) {
	             lib::logger::get_logger()->trace("gstreamer_audio_renderer: seek() failed.");
	      }
	      m_player->mutex_release("gstreamer_audio_renderer::seek");
       }
       m_lock.leave();
}

common::duration 
gstreamer_audio_renderer::get_dur()
{
	gint64 length = -1;
	GstFormat fmtTime = GST_FORMAT_TIME;
	common::duration rv(false, 0.0);

	AM_DBG lib::logger::get_logger()->debug("gstreamer_audio_renderer.get_dur(0x%x)", (void *)this);
	m_lock.enter();
	if (m_player) {
	        m_player->mutex_acquire("gstreamer_audio_renderer::get_dur");
		gst_element_query_duration(m_player->gst_player(), &fmtTime, &length);
		m_player->mutex_release("gstreamer_audio_renderer::get_dur");
	}
	m_lock.leave();

	if (length != -1) {
	        double nanosec = 1e9, microsec = 1e6;
		double dur = double(length) / nanosec;
		double clip_begin = m_clip_begin / microsec;
		double clip_end = m_clip_end / microsec;

		if (clip_end > 0 && dur > clip_end)
	               dur = clip_end;
		if (clip_begin > 0)
	               dur -= clip_begin;
		lib::logger::get_logger()->trace("gstreamer_audio_renderer: get_dur() clip_begin=%f clip_end=%f dur=%f", clip_begin, clip_end, dur);
		rv = common::duration(true, dur);
  	}
	return rv;
}

