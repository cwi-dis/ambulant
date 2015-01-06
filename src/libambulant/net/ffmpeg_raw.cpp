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

#include "ambulant/net/ffmpeg_common.h"
#include "ambulant/net/ffmpeg_raw.h"
#include "ambulant/net/ffmpeg_factory.h"
#include "ambulant/lib/logger.h"
#include "ambulant/net/url.h"

// WARNING: turning on AM_DBG globally for the ffmpeg code seems to trigger
// a condition that makes the whole player hang or collapse. So you probably
// shouldn't do it:-)
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace net;

raw_datasource_factory *
ambulant::net::get_ffmpeg_raw_datasource_factory()
{
#if 0
	// It seems datasource factories are sometimes cleaned up, hence we cannot use
	// a singleton. Need to fix/document at some point.
	static raw_datasource_factory *s_factory;

	if (!s_factory) s_factory = new ffmpeg_raw_datasource_factory();
	return s_factory;
#else
	return new ffmpeg_raw_datasource_factory();
#endif
}

#define DEFAULT_BUFFER_SIZE 128*1024

datasource*
ffmpeg_raw_datasource_factory::new_raw_datasource(const net::url& url)
{

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource_factory::new_raw_datasource(%s)", repr(url).c_str());
	AVIOContext *context = detail::ffmpeg_rawreader::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource_factory::new_raw_datasource: no support for %s", repr(url).c_str());
		return NULL;
	}
	detail::ffmpeg_rawreader *thread = new detail::ffmpeg_rawreader(context, url);
	datasource *ds = new ffmpeg_raw_datasource(url, context, thread);
	if (ds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource_factory::new_raw_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}
	thread->start();
	return ds;
}

// **************************** ffmpeg_rawreader *****************************


detail::ffmpeg_rawreader::ffmpeg_rawreader(AVIOContext *con, const net::url &url)
:   m_con(con),
	m_sink(NULL),
    m_resource_type("unknown"),
	m_bytes_read(0)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_rawreader::ffmpeg_rawreader() m_con=0x%x", con);
    m_resource_type = url.get_protocol();
}

detail::ffmpeg_rawreader::~ffmpeg_rawreader()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_rawreader::~ffmpeg_rawreader() m_con=0x%x", m_con);
	if (m_con) avio_close(m_con);
	m_con = NULL;
	m_lock.leave();
}

AVIOContext *
detail::ffmpeg_rawreader::supported(const net::url& url)
{
	ffmpeg_init();
	// Setup struct to allow ffmpeg to determine whether it supports this
	AVIOContext *ic = NULL;
	int err = avio_open(&ic, url.get_url().c_str(), AVIO_FLAG_READ);
	if (err) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_rawreader::supported(%s): url_open returned error %d, ic=0x%x", repr(url).c_str(), err, (void*)ic);
		if (ic) {
			avio_close(ic);
			ic = NULL;
		}
	}
	return ic;
}

void
detail::ffmpeg_rawreader::cancel()
{
	if (is_running())
		stop();
	release();
}

void
detail::ffmpeg_rawreader::set_datasink(detail::rawdatasink *parent)
{
	m_lock.enter();
	assert(m_sink == 0);
	m_sink = parent;
	m_lock.leave();
}

long
detail::ffmpeg_rawreader::get_bandwidth_usage_data(const char **resource)
{
    m_lock.enter();
    long rv = m_bytes_read;
    m_bytes_read = 0;
    *resource = m_resource_type.c_str();
    m_lock.leave();
    return rv;
}


unsigned long
detail::ffmpeg_rawreader::run()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_rawreader::run: started");
	while (!exit_requested()) {
		uint8_t *sinkbuffer;
		size_t sinkbuffersize;

		assert(m_sink);
		sinkbuffersize = m_sink->get_sinkbuffer(&sinkbuffer);
		if (sinkbuffersize == 0) {
			m_lock.leave();
			AM_DBG lib::logger::get_logger("ffmpeg_rawreader::run:sleep(1)");
			lib::sleep(1);
			m_lock.enter();
		} else {
			int bytecount;
			AM_DBG lib::logger::get_logger("ffmpeg_rawreader::run: calling url_read(size=%d)", (int)sinkbuffersize);
			bytecount = avio_read(m_con, sinkbuffer, (int)sinkbuffersize);
			AM_DBG lib::logger::get_logger("ffmpeg_rawreader::run:url_read() returned %d", bytecount);
			if (bytecount >= 0) {
                m_bytes_read += bytecount;
				m_sink->pushdata((size_t)bytecount);
			} else {
				m_sink->pushdata(0);
			}

			if (bytecount <= 0) {
				break;
			}
		}
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_rawreader::run: final sinkdata(0)");
	m_lock.leave();
	return 0;
}

// **************************** ffmpeg_raw_datasource *****************************

ffmpeg_raw_datasource::ffmpeg_raw_datasource(const net::url& url, AVIOContext *context,
	detail::ffmpeg_rawreader *thread)
:	m_url(url),
	m_con(context),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(thread),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource::ffmpeg_raw_datasource() -> 0x%x", (void*)this);
	if (!m_thread) {
		lib::logger::get_logger()->error(gettext("%s: Cannot start reader thread"), url.get_url().c_str());
		m_src_end_of_file = true;
		return;
	}
	m_thread->set_datasink(this);
}

ffmpeg_raw_datasource::~ffmpeg_raw_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource::~ffmpeg_raw_datasource(0x%x)", (void*)this);
	stop();
}

void
ffmpeg_raw_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource::stop(0x%x)", (void*)this);
	if (m_thread) {
		detail::ffmpeg_rawreader *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->cancel();
		m_lock.enter();
	}
	m_thread = NULL;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource::stop: thread stopped");
	m_con = NULL; // owned by the thread
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
}

void
ffmpeg_raw_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();

	if (m_client_callback != NULL) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource::start(): m_client_callback already set!");
		delete m_client_callback;
		m_client_callback = NULL;
	}
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (evp && callbackk) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::ep_med);
		} else {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_raw_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
	}
	m_lock.leave();
}

void
ffmpeg_raw_datasource::readdone(size_t len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource.readdone : done with %d bytes", len);
//	restart_input();
	m_lock.leave();
}

size_t
ffmpeg_raw_datasource::get_sinkbuffer(uint8_t **datap)
{
	m_lock.enter();
	if (m_buffer.buffer_full()) {
		*datap = NULL;
		m_lock.leave();
		return 0;
	}
	// Otherwise we just take a guess.
	*datap = (uint8_t *)m_buffer.get_write_ptr(DEFAULT_BUFFER_SIZE);
	m_lock.leave();
	if (*datap)
		return DEFAULT_BUFFER_SIZE;
	else
		return 0;
}

void
ffmpeg_raw_datasource::pushdata(size_t sz)
{
	m_lock.enter();
	m_buffer.pushdata(sz);
	if (sz <= 0)
		m_src_end_of_file = true;
	if ( m_client_callback && (m_buffer.buffer_not_empty() || m_src_end_of_file ) ) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource::pushdata(): calling client callback (%d, %d)", m_buffer.size(), m_src_end_of_file);
		assert(m_event_processor);
		m_event_processor->add_event(m_client_callback, 0, ambulant::lib::ep_med);
		m_client_callback = NULL;
		m_event_processor = NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_raw_datasource::pushdata(): No client callback!");
	}
	m_lock.leave();
}

bool
ffmpeg_raw_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
ffmpeg_raw_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	return m_src_end_of_file;
}

char*
ffmpeg_raw_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

size_t
ffmpeg_raw_datasource::size() const
{
		return m_buffer.size();
}

