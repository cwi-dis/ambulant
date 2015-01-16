// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

//#include <math.h>
//#include <map>

#include "ambulant/config/config.h"
#include "ambulant/net/demux_datasource.h"
#include "ambulant/net/ffmpeg_audio.h"

// Minimum delay
#define MIN_EVENT_DELAY 1

// How many audio/video frames we would like to buffer
// at most between the reader and the decoder
#define MAX_PACKETS_BUFFERED 300

// WARNING: turning on AM_DBG globally in this file seems to trigger
// a condition that makes the whole player hang or collapse. So you probably
// shouldn't do it:-)
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace net;

// **************************** demux_datasource ******************************

demux_datasource *
demux_datasource::new_demux_datasource(
	const net::url& url,
	abstract_demux *thread)
{

	int stream_index;

	AM_DBG lib::logger::get_logger()->debug("demux_datasource::new_demux_datasource()");
	// Find the index of the audio stream
	stream_index = thread->audio_stream_nr();
	assert(stream_index >= 0);

	if (stream_index >= thread->nstreams()) {
		lib::logger::get_logger()->error(gettext("%s: no more audio streams"), url.get_url().c_str());
		return NULL;
	}

	AM_DBG lib::logger::get_logger()->debug("demux_datasource::new_demux_datasource() looking for the right codec");

	return new demux_datasource(url, thread, stream_index);
}

demux_datasource::demux_datasource(const net::url& url, abstract_demux *thread, int stream_index)
:	m_url(url),
	m_stream_index(stream_index),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(thread),
	m_current_time(-1),
	m_client_callback(NULL),
    m_is_live(false),
    m_is_video(false)
#ifdef XXXJACK_COMBINE_HACK
    , m_saved_packet(NULL)
#endif
{
	m_thread->add_datasink(this, stream_index);
	m_current_time = m_thread->get_clip_begin();
}

demux_datasource::~demux_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("demux_datasource::~demux_datasource(0x%x)", (void*)this);
	stop();
}

void
demux_datasource::stop()
{
	m_lock.enter();
	m_event_processor = NULL;
	AM_DBG lib::logger::get_logger()->debug("demux_datasource::stop(0x%x)", (void*)this);

	if (m_thread) {
		abstract_demux *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->remove_datasink(m_stream_index);
		m_lock.enter();
	}
	m_thread = NULL;
	AM_DBG lib::logger::get_logger()->debug("demux_datasource::stop: thread stopped");

	if (m_client_callback) {
		lib::logger::get_logger()->debug("demux_datasource::stop: m_client_callback non-NULL, after remove_datasink()");
		delete m_client_callback;
		m_client_callback = NULL;
	}
	while (m_frames.size() > 0) {
		datasource_packet tsp = m_frames.front();
		free(tsp.pkt);
		m_frames.pop();
	}
	m_current_time = -1;
	m_lock.leave();
}

void
demux_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	assert(m_thread);
	m_thread->start();

	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("demux_datasource::start(): m_client_callback already set!");
	}
	if (m_frames.size() > 0 || _end_of_file()) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("demux_datasource::start: trigger client callback");
			evp->add_event(callbackk, MIN_EVENT_DELAY, ambulant::lib::ep_med);
		} else {
			lib::logger::get_logger()->debug("Internal error: demux_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
        AM_DBG lib::logger::get_logger()->debug("demux_datasource::start: record client callback for later");
		m_client_callback = callbackk;
		m_event_processor = evp;
	}
	m_lock.leave();
}

void
demux_datasource::seek(timestamp_t time)
{
	m_lock.enter();
	assert( time >= 0);
	assert(m_thread);

	if (time == m_current_time) {
		// The head of our queue has the timestamp we want to seek to. Don't do anything.
		AM_DBG lib::logger::get_logger()->debug("demux_datasource::seek(%d): already there", time);
		m_lock.leave();
		return;
	}

	m_thread->seek(time);

	AM_DBG lib::logger::get_logger()->debug("demux_datasource::seek(%d): flushing %d packets", time, m_frames.size());
	size_t nbuf = m_frames.size();
	if ( nbuf > 0) {
		AM_DBG lib::logger::get_logger()->debug("demux_datasource: flush %d buffers due to seek", nbuf);
	}
	while (m_frames.size() > 0) {
		AVPacket* data = m_frames.front().pkt;
		if (data != NULL) {
			free(data);
		}
		m_frames.pop();
	}
	m_current_time = time;
	m_src_end_of_file = false;
	m_lock.leave();
}

void
demux_datasource::set_clip_end(timestamp_t clip_end)
{
	assert(m_thread);
	// NOTE: the seek is outside the lock, otherwise there's a deadlock with the
	// thread trying to deliver new data to this demux_datasource.
	m_thread->set_clip_end(clip_end);
}

void
demux_datasource::read_ahead(timestamp_t time)
{
	m_lock.enter();
	assert(m_thread);
	assert(!m_thread->is_running());
	assert(m_frames.size() == 0);	// Jack assumes this should be true
	m_current_time = time;
	m_thread->read_ahead(time);
	m_thread->start();
	m_lock.leave();
}


bool
demux_datasource::push_data(timestamp_t pts, struct AVPacket *pkt, datasource_packet_flag flag)
{
	// XXX timestamp is ignored, for now
	m_lock.enter();
#ifdef XXXJACK_COMBINE_HACK
    // XXXJACK Note: this code is incorrect: non-data packets will overtake data packets.
    if (1 /*m_is_live*/ && flag == datasource_packet_flag_avpacket) {
        if (m_saved_packet && pkt->pts == m_saved_packet->pts && pkt->dts == m_saved_packet->dts) {
            // combine
            AM_DBG lib::logger::get_logger()->debug("demux_datasource::push_data: %p: combine(pts=%lld, oldsize=%d, addedsize=%d)", (void*)this, pkt->pts, m_saved_packet->size, pkt->size);
            int prev_size = m_saved_packet->size;
            av_grow_packet(m_saved_packet, pkt->size);
            memcpy(m_saved_packet->data + prev_size, pkt->data, pkt->size);
            av_free_packet(pkt);
            m_lock.leave();
            return true;
        } else {
            // forward old packet, keep new packet
            AVPacket *tmp = m_saved_packet;
            m_saved_packet = pkt;
            AM_DBG lib::logger::get_logger()->debug("demux_datasource::push_data: %p: save(pts=%lld, size=%d)", (void*)this, pkt->pts, pkt->size);
            int ok = av_dup_packet(m_saved_packet);
            if (tmp == NULL || ok < 0) {
                m_lock.leave();
                return true;
            }
            pkt = tmp;
            pts = pkt->pts;
            AM_DBG lib::logger::get_logger()->debug("demux_datasource::push_data: %p: forward(pts=%lld, size=%d)", (void*)this, pkt->pts, pkt->size);
        }
    }
#endif
	m_src_end_of_file = (flag == datasource_packet_flag_eof);
	AM_DBG lib::logger::get_logger()->debug("demux_datasource.push_data: pts=%lld, pkt=%p, pkt->pts=%lld, flag=%d", pts, pkt, pkt?pkt->pts : -1, flag);
	if ( !m_thread) {
		// video stopped
		AM_DBG lib::logger::get_logger()->debug("demux_datasource::push_data(): no demux thread, returning");
		m_lock.leave();
		return true;
	}
	if ( _buffer_full()) {
		// video stopped
		AM_DBG lib::logger::get_logger()->debug("demux_datasource::push_data(): buffer full, returning");
		m_lock.leave();
		return false;
	}
#ifdef LOGGER_VIDEOLATENCY
    lib::logger::get_logger(LOGGER_VIDEOLATENCY)->trace("videolatency 2-push %lld %lld %s", 0LL, pts, m_url.get_url().c_str());
#endif

	datasource_packet qel(pts, pkt, flag);
	m_frames.push(qel);
    
	if ( m_frames.size() > 0 || _end_of_file()  ) {
		if ( m_client_callback && m_event_processor) {
			AM_DBG lib::logger::get_logger()->debug("demux_datasource::push_data(pts=%lld): calling client callback (eof=%d)", pts, m_src_end_of_file);
			m_event_processor->add_event(m_client_callback, MIN_EVENT_DELAY, ambulant::lib::ep_med);
			m_client_callback = NULL;
			m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->debug("demux_datasource::push_data(pts=%lld): No client callback", pts);
		}
	}
	m_lock.leave();
	return true;
}


bool
demux_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
demux_datasource::_end_of_file()
{
	// private method - no need to lock
	return m_src_end_of_file && m_frames.empty();
}

bool
demux_datasource::_buffer_full()
{
	return m_frames.size() >= MAX_PACKETS_BUFFERED;
}

datasource_packet
demux_datasource::get_packet()
{
	datasource_packet tsp;
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("demux_datasource::get_packet: m_frames.size=%d, m_src_eof=%d", m_frames.size(), m_src_end_of_file);
	if (m_frames.size() > 0) {
		tsp = m_frames.front();
		m_frames.pop();
		m_current_time = -1;
	}
	m_lock.leave();
	return tsp;
}

timestamp_t
demux_datasource::get_clip_end()
{
	m_lock.enter();
	assert(m_thread);
	timestamp_t clip_end = m_thread->get_clip_end();
	AM_DBG lib::logger::get_logger()->debug("demux_datasource::get_clip_end: clip_end=%d", clip_end);
	m_lock.leave();
	return	clip_end;
}

timestamp_t
demux_datasource::get_clip_begin()
{
	m_lock.enter();
	assert(m_thread);
	timestamp_t clip_begin = m_thread->get_clip_begin();
	AM_DBG lib::logger::get_logger()->debug("demux_datasource::get_clip_begin: clip_begin=%d", clip_begin);
	m_lock.leave();
	return	clip_begin;
}

audio_format&
demux_datasource::get_audio_format()
{
	assert(m_thread);
	return m_thread->get_audio_format();
}

common::duration
demux_datasource::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	assert(m_thread);
	if (m_thread->duration() >= 0) {
		rv = common::duration(true, m_thread->duration());
		AM_DBG lib::logger::get_logger()->debug("demux_datasource::get_dur: duration=%f", rv.second);
	}
	m_lock.leave();
	return rv;
}

// **************************** demux_video_datasource *****************************

demux_video_datasource *
demux_video_datasource::new_demux_video_datasource(
		const net::url& url,
		abstract_demux *thread)
{

	int stream_index;

	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::new_demux_video_datasource()");
	// Find the index of the audio stream
	stream_index = thread->video_stream_nr();
	assert(stream_index >= 0);

	if (stream_index >= thread->nstreams()) {
		lib::logger::get_logger()->error(gettext("%s: no more audio streams"), url.get_url().c_str());
		return NULL;
	}

	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::new_demux_video_datasource() looking for the right codec");


	return new demux_video_datasource(url, thread, stream_index);
}

demux_video_datasource::demux_video_datasource(const net::url& url, abstract_demux *thread, int stream_index)
:   demux_datasource(url, thread, stream_index), 
	m_audio_src(NULL),
	m_frame_nr(0)
{
    m_is_video = true;
	int audio_stream_idx = m_thread->audio_stream_nr();
	if (audio_stream_idx >= 0)
		m_audio_src = new demux_datasource(m_url, m_thread, audio_stream_idx);
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::demux_video_datasource(0x%x) m_audio_src=0x%x url=%s", (void*)this, m_audio_src, url.get_url().c_str());

}

demux_video_datasource::~demux_video_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::~demux_video_datasource(0x%x)", (void*)this);
}

#if 0
int
demux_video_datasource::size() const
{
	const_cast <demux_video_datasource*>(this)->m_lock.enter();
	int rv = (int)m_frames.size();
	const_cast <demux_video_datasource*>(this)->m_lock.leave();
	return rv;
}

video_format&
demux_video_datasource::get_video_format()
{
	m_lock.enter();
	assert(m_thread);
	m_lock.leave();
	return m_thread->get_video_format();
}

int
demux_video_datasource::width()
{
	m_lock.enter();
	assert(m_thread);
	video_format fmt = m_thread->get_video_format();
	m_lock.leave();
	return fmt.width;
}

int
demux_video_datasource::height()
{
	m_lock.enter();
	assert(m_thread);
	video_format fmt = m_thread->get_video_format();
	m_lock.leave();
	return fmt.height;
}

timestamp_t
demux_video_datasource::frameduration()
{//STUB CODE, not to be used currently
	assert(0);
	return 0;
}
#endif

bool
demux_video_datasource::has_audio()
{
    bool rv;
	m_lock.enter();
	assert(m_thread);
	rv = (m_thread->audio_stream_nr() >= 0);
    m_lock.leave();
    return rv;
}

audio_datasource*
demux_video_datasource::get_audio_datasource(audio_format_choices fmts)
{
	m_lock.enter();
	if (m_audio_src) {
		//XXX a factory should take care of getting a decoder ds.
		audio_format fmt = m_audio_src->get_audio_format();
		audio_datasource *dds = NULL;
		if (ffmpeg_decoder_datasource::supported(fmt))
			dds = new ffmpeg_decoder_datasource(m_audio_src, fmts);
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_audio_datasource: decoder ds = 0x%x", (void*)dds);
		if (dds == NULL) {
			lib::logger::get_logger()->warn(gettext("%s: Ignoring audio, unsupported encoding"), m_url.get_url().c_str());
			pkt_datasource *tmp = m_audio_src;
			m_audio_src = NULL;
			m_lock.leave();
			tmp->stop();
			long rem = tmp->release();
			assert(rem == 0);
			return NULL;
		}
		m_lock.leave();
		return dds;
	}
	m_lock.leave();
	return NULL;
}

#if 0
common::duration
demux_video_datasource::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	assert(m_thread);
	if (m_thread->duration() >= 0) {
		rv = common::duration(true, m_thread->duration());
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_dur: duration=%f", rv.second);
	}
	m_lock.leave();
	return rv;
}
#endif
