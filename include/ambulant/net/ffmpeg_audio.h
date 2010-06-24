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

#ifndef AMBULANT_NET_FFMPEG_AUDIO_H
#define AMBULANT_NET_FFMPEG_AUDIO_H

#include "ambulant/config/config.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/datasource.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}


#ifdef AMBULANT_PLATFORM_MACOS
	#define AM_MAC_CONST const
#else
	#define AM_MAC_CONST
#endif


namespace ambulant
{

namespace net
{

class ffmpeg_audio_datasource_factory : public audio_datasource_factory {
  public:
	~ffmpeg_audio_datasource_factory() {};
	audio_datasource* new_audio_datasource(const net::url& url, const audio_format_choices& fmts, timestamp_t clip_begin, timestamp_t clip_end);
};

class ffmpeg_audio_decoder_finder : public audio_decoder_finder {
  public:
	~ffmpeg_audio_decoder_finder() {};
	audio_datasource* new_audio_decoder(pkt_audio_datasource *src, const audio_format_choices& hint);
};

class ffmpeg_audio_filter_finder : public audio_filter_finder {
  public:
	~ffmpeg_audio_filter_finder() {};
	audio_datasource* new_audio_filter(audio_datasource *src, const audio_format_choices& fmts);
};

class ffmpeg_decoder_datasource: virtual public audio_datasource, virtual public lib::ref_counted_obj {
  public:
	static bool supported(const audio_format& fmt);
	static bool supported(const net::url& url);

	ffmpeg_decoder_datasource(const net::url& url, pkt_audio_datasource *src);
	ffmpeg_decoder_datasource(pkt_audio_datasource *src);
	~ffmpeg_decoder_datasource();


	void start(lib::event_processor *evp, lib::event *callback);
	void stop();

	void readdone(size_t len);
	void data_avail();
	bool end_of_file();
	bool buffer_full();
	void read_ahead(timestamp_t clip_begin);
	void seek(timestamp_t time);
#ifdef WITH_SEAMLESS_PLAYBACK
	void set_clip_end(timestamp_t clip_end);
	void start_prefetch(lib::event_processor *evp);
#endif

	char* get_read_ptr();
	size_t size() const;
	common::duration get_dur();
	audio_format& get_audio_format();
	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_src->get_start_time(); };
#ifdef WITH_SEAMLESS_PLAYBACK
	timestamp_t get_elapsed();
#endif

  protected:
	bool _select_decoder(const char* file_ext);
	bool _select_decoder(audio_format &fmt);

  private:
	bool _clip_end() const;
	bool _end_of_file();
	void _need_fmt_uptodate();
	AVCodecContext *m_con;
	bool m_con_owned;
	audio_format m_fmt;
	lib::event_processor *m_event_processor;
	pkt_audio_datasource* m_src;
	timestamp_t m_elapsed;      // Timestamp of the very last sample in the buffer
	bool m_is_audio_ds;

	databuffer m_buffer;
	bool m_blocked_full;

	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;

};

class ffmpeg_resample_datasource: virtual public audio_datasource, virtual public lib::ref_counted_obj {
  public:
	ffmpeg_resample_datasource(audio_datasource *src, audio_format_choices fmts);
	~ffmpeg_resample_datasource();

	void start(lib::event_processor *evp, lib::event *callback);
	void stop();
	void read_ahead(timestamp_t time);
	void seek(timestamp_t time);
#ifdef WITH_SEAMLESS_PLAYBACK
	void set_clip_end(timestamp_t clip_end);
	void start_prefetch(lib::event_processor *evp);
	timestamp_t get_elapsed();
#endif
	void readdone(size_t len);
	void data_avail();

	bool end_of_file();
	bool buffer_full();

	char* get_read_ptr();
	size_t size() const;

//	void get_input_format(audio_context &fmt);
//	void get_output_format(audio_context &fmt);
	audio_format& get_audio_format() { return m_out_fmt; };
	common::duration get_dur();
	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_src->get_start_time(); }
  protected:
	int init();


  private:
	bool _end_of_file();
	bool _src_end_of_file() const;

	audio_datasource* m_src;

	bool m_context_set;
	ReSampleContext *m_resample_context;

	databuffer m_buffer;

	lib::event_processor *m_event_processor;
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
	audio_format m_in_fmt;
	audio_format m_out_fmt;

};

}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_FFMPEG_AUDIO_H
