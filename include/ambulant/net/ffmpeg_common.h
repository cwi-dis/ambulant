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

#ifndef AMBULANT_NET_FFMPEG_COMMON_H
#define AMBULANT_NET_FFMPEG_COMMON_H


#include "ambulant/config/config.h"
#ifdef AMBULANT_PLATFORM_UNIX
#include "ambulant/lib/unix/unix_thread.h"
#endif
#ifdef AMBULANT_PLATFORM_WIN32
// This assumes that we are building on Windows with a dll-based ffmpeg
// that has been built with mingw32. These defines are then needed to get
// the right macros defined. See third_party_packages/readme.txt, section
// ffmpeg for windows, for a reference to the webpage that is a source
// for this.
#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include "ambulant/net/datasource.h"

// Needed for avcodec.h:
// Needed for avcodec.h:
#ifndef INT64_C
#define INT64_C(x) x ## LL
#endif
#ifndef UINT64_C
#define UINT64_C(x) x ## ULL
#endif
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"

#if LIBAVCODEC_VERSION_MAJOR < 55
typedef CodecID AVCodecID;
#define av_frame_alloc() avcodec_alloc_frame()
#define av_frame_get_channels(frame) (m_con->channels)
#define av_frame_free(framep) av_freep(framep)
#endif

#define AMBULANT_MAX_FFMPEG_STREAMS 20

}

namespace ambulant
{

namespace net
{

void ffmpeg_init();

class ffmpeg_codec_id {
  public:
	static ffmpeg_codec_id* instance();
	~ffmpeg_codec_id() {};

	void add_codec(const char* codec_name, 	AVCodecID id);
	AVCodecID get_codec_id(const char* codec_name);
  private:
	ffmpeg_codec_id();
	static ffmpeg_codec_id* m_uniqueinstance;
	std::map<std::string, AVCodecID> m_codec_id;
};

class ffmpeg_demux : public abstract_demux {
  public:
	ffmpeg_demux(AVFormatContext *con, const net::url& url, timestamp_t clip_begin, timestamp_t clip_end);
	~ffmpeg_demux();

	static AVFormatContext *supported(const net::url& url);

	void add_datasink(demux_datasink *parent, int stream_index);
	void remove_datasink(int stream_index);
	int audio_stream_nr();
	int video_stream_nr();
	// XXX this should also be timestamp_t instead of double
	double duration();
	int nstreams();
	void seek(timestamp_t time);
	void set_clip_end(timestamp_t clip_end);
	void read_ahead(timestamp_t time);
	audio_format& get_audio_format();
	video_format& get_video_format();
	void cancel();
	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_clip_begin; };
    long get_bandwidth_usage_data(int stream_index, const char **resource);
    void set_is_live (bool is_live) { m_is_live = is_live; }
    bool get_is_live () { return m_is_live; }
  protected:
	unsigned long run();
  private:
	audio_format m_audio_fmt;
	video_format m_video_fmt;
	demux_datasink *m_sinks[AMBULANT_MAX_FFMPEG_STREAMS];
    long m_data_consumed[AMBULANT_MAX_FFMPEG_STREAMS];
    std::string m_bandwidth_resource;
	demux_datasink *m_current_sink;
	AVFormatContext *m_con;
    net::url m_url;
	int m_nstream;
	lib::critical_section m_lock;
	timestamp_t m_clip_begin;
	timestamp_t m_clip_end;
	bool m_clip_begin_changed;	// True if m_clip_begin has changed.
	bool m_is_live;		// True if this is a live stream
};

/// Helper routine: return a reference to a global lock (needed for ffmpeg serialization)
lib::critical_section* ffmpeg_global_critical_section();
static lib::critical_section s_lock;

}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_FFMPEG_COMMON_H
