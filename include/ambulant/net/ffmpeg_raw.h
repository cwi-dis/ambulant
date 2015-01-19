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

#ifndef AMBULANT_NET_FFMPEG_RAW_H
#define AMBULANT_NET_FFMPEG_RAW_H

#include "ambulant/config/config.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#ifdef AMBULANT_PLATFORM_UNIX
#include "ambulant/lib/unix/unix_thread.h"
#define BASE_THREAD lib::unix::thread
#endif
#ifdef AMBULANT_PLATFORM_WIN32
#include "ambulant/lib/win32/win32_thread.h"
#define BASE_THREAD lib::win32::thread
#endif
#include "ambulant/net/databuffer.h"
#include "ambulant/net/datasource.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/common.h"
}

// temporary debug messages
#include <iostream>
#include <ostream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace ambulant
{

namespace net
{

class ffmpeg_raw_datasource_factory : public raw_datasource_factory {
  public:
	~ffmpeg_raw_datasource_factory() {};
	datasource* new_raw_datasource(const net::url& url);
};

namespace detail {

// Actually, these classes could easily be used for a general demultiplexing
// datasource, there is very little code that is ffmpeg-dependent.

class rawdatasink {
  public:
	virtual ~rawdatasink(){}
	virtual size_t get_sinkbuffer(uint8_t **datap) = 0;
	virtual void pushdata(size_t size) = 0;
};

class ffmpeg_rawreader : public BASE_THREAD, public lib::ref_counted_obj {
  public:
	ffmpeg_rawreader(AVIOContext *con, const net::url& url);
	~ffmpeg_rawreader();

	static AVIOContext *supported(const net::url& url);

	void set_datasink(rawdatasink *parent);
	void cancel();
    long get_bandwidth_usage_data(const char **resource);
  protected:
	unsigned long run();
  private:
	AVIOContext *m_con;
	rawdatasink *m_sink;
    long m_bytes_read;
    std::string m_resource_type;
	lib::critical_section m_lock;
};

}

class ffmpeg_raw_datasource:
	virtual public datasource,
	public detail::rawdatasink,
	virtual public lib::ref_counted_obj
{
  public:
	ffmpeg_raw_datasource(const net::url& url, AVIOContext *context, detail::ffmpeg_rawreader *thread);
	~ffmpeg_raw_datasource();

	void start(lib::event_processor *evp, lib::event *callback);
	void start_prefetch(lib::event_processor *evp){};  // XXXJACK need to implement this at some point
	void stop();

	char* get_read_ptr();
	void readdone(size_t len);
	bool end_of_file();
	size_t size() const;

	size_t get_sinkbuffer(uint8_t **datap);
	void pushdata(size_t size);

    long get_bandwidth_usage_data(const char **resource) { return m_thread ? m_thread->get_bandwidth_usage_data(resource) : 0; }
  private:
	bool _end_of_file();
	const net::url m_url;
	AVIOContext *m_con;
	bool m_src_end_of_file;
	lib::event_processor *m_event_processor;

	databuffer m_buffer;
	detail::ffmpeg_rawreader *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
};

}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_FFMPEG_RAW_H
