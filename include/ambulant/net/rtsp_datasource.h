/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

#ifndef AMBULANT_NET_RTSP_DATASOURCE_H
#define AMBULANT_NET_RTSP_DATASOURCE_H

#include "ambulant/config/config.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"


#define MAX_RTP_FRAME_SIZE 100000

//#define CLIP_BEGIN_CHANGED 1 //xxxbo: a switch for m_clip_begin_changed and m_seektime_changed;

// Somehow, the time stamps produced by live555 are incorrect.
// Enable the next define to try and re-create correct
// timestamps.

#ifdef AMBULANT_PLATFORM_MACOS
// Both MacHeaders.h and Live typedef Boolean, but to imcompatible
// types.
#define Boolean LiveBoolean
// Similarly for EventTime
#define EventTime LiveEventTime
#endif

// LiveMedia includes
#define RTSPCLIENT_SYNCHRONOUS_INTERFACE 1
// #include "BasicUsageEnvironment.hh"
// #include "liveMedia.hh"

// Needed for avcodec.h:
// Needed for avcodec.h:
#ifndef INT64_C
#define INT64_C(x) x ## LL
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/common.h"

#include "ambulant/net/databuffer.h"
//#include "ambulant/net/posix_datasource.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/ffmpeg_common.h"

namespace ambulant
{

namespace net
{

struct rtsp_context_t {
	rtsp_context_t()
	:	scheduler(NULL),
		env(NULL),
		rtsp_client(NULL),
		media_session(NULL),
		sdp(NULL),
		nstream(0),
		audio_stream(-1),
		audio_subsession(NULL),
		video_stream(-1),
		video_subsession(NULL),
		configData(NULL),
		configDataLen(0),
		initialPacketData(NULL),
		initialPacketDataLen(0),
		extraPacketHeaderData(NULL),
		extraPacketHeaderSize(0),
		notPacketized(false),
		vbuffer(NULL),
		vbufferlen(0),
		last_pts(0),
		frame_duration(0),
		need_audio(true),
		need_video(true),
		audio_packet(NULL),
		video_packet(NULL),
		blocking_flag(0),
		eof(false),
		clip_end(-1),
		duration(-1),
		last_expected_pts(-1),
		highest_pts_seen(-1),
		idle_time(0),
		audio_codec_name("none"),
		video_codec_name("none"),
		first_sync_time_set(false),
		audio_fmt("none"),
		video_fmt("none"),
		nsinks(0)
	{
		first_sync_time.tv_sec = 0;
		first_sync_time.tv_usec = 0;
		memset(sinks, 0, sizeof sinks);
        memset(data_consumed, 0, sizeof data_consumed);
	}
	~rtsp_context_t() {
		//Have to tear down session here, so that the server is not left hanging till timeout.
		rtsp_client->teardownMediaSession(*media_session);
		for (int i=0; i < MAX_STREAMS; i++) {
			if (sinks[i] != NULL)
				delete sinks[i];
		}
		if (configData) free(configData);
		if (initialPacketData) free(initialPacketData);
		if (extraPacketHeaderData) free(extraPacketHeaderData);
		if (vbuffer) free(vbuffer);
		if (audio_packet) free(audio_packet);
		if (video_packet) free(video_packet);
		if (media_session) Medium::close(media_session);
		if (sdp) free(sdp);
		if (rtsp_client) Medium::close(rtsp_client);
		if (env) env->reclaim();
		if (scheduler) delete scheduler; // XXXJACK no close method?
	}

	TaskScheduler* scheduler;
	UsageEnvironment* env;
	RTSPClient* rtsp_client;
	MediaSession* media_session;
	char* sdp;
	int nstream;		// Total number of streams
	int audio_stream;	// Index of the audio subsession
	MediaSubsession *audio_subsession;
	int video_stream;	// Index of the video subsession
	MediaSubsession *video_subsession;

	unsigned char* configData; // For H264 (and maybe other formats): Extra configuration data, to be passed to ffmpeg
	size_t configDataLen;
	unsigned char *initialPacketData;	// For MP4V (and maybe other formats): a synthetic initial packet
	size_t initialPacketDataLen;
	unsigned char *extraPacketHeaderData;	// H264 (and some other formats) need a couple extra bytes at the beginning of each packet.
	size_t extraPacketHeaderSize;
	bool notPacketized;			// H264 streams from live555 are not packetized, we need to do that ourselves.
	unsigned char* vbuffer;		// Buffer for re-packetizing
	size_t vbufferlen;
	timestamp_t last_pts;		// Timestamp of packet data being accumulated in vbuffer
	timestamp_t frame_duration; // Current guess at frame duration
	bool need_audio;			// True if we're interested in an audio packet
	bool need_video;			// True if we're interested in a video packet
	unsigned char* audio_packet;	// The current audio packet data (size is a parameter to after_reading_audio)
	unsigned char* video_packet;	// The current video packet data (size is a parameter to after_reading_video)
	char blocking_flag;		// Flag determining whether live555 should block in read
	bool eof;				// True when we reach end-of-file on any stream (XXXJACK: correct behaviour?)
	timestamp_t clip_end;	// Where we want to stop (microseconds)
	double duration;			// How long the stream will take in total (XXXJACK: needed?)
	timestamp_t last_expected_pts;	// The same, but different
	timestamp_t highest_pts_seen;   // Highest pts seen in any stream
	timestamp_t idle_time;      // Amount of time we have not seen any packets
	const char* audio_codec_name;
	const char* video_codec_name;
	timeval first_sync_time;	// timestamp of first synchronized packet
	bool first_sync_time_set;	// True when first_sync_time has been set
	audio_format audio_fmt;
	video_format video_fmt;
	demux_datasink *sinks[MAX_STREAMS];
    long data_consumed[MAX_STREAMS];
	int nsinks;

};

class rtsp_demux : public abstract_demux {
  public:
	rtsp_demux(rtsp_context_t* context, timestamp_t clip_begin, timestamp_t clip_end);
	~rtsp_demux();
	static rtsp_context_t* supported(const net::url& url);

	void add_datasink(demux_datasink *parent, int stream_index);
	void remove_datasink(int stream_index);
	int audio_stream_nr() { return m_context->audio_stream; };
	int video_stream_nr() { return m_context->video_stream; };
	int nstreams() { return m_context->nstream; };
	double duration(){ return 0.0; };
	audio_format& get_audio_format() { return m_context->audio_fmt; };
	video_format& get_video_format() { return m_context->video_fmt; };
	void seek(timestamp_t time);
	void set_clip_end(timestamp_t clip_end);
	void read_ahead(timestamp_t time);
	void cancel();
	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_clip_begin; };
	// These next two should be protected, but I don't know how to make a static function a friend.
	void after_reading_audio(size_t sz, unsigned truncated, struct timeval pts, unsigned duration);
	void after_reading_video(size_t sz, unsigned truncated, struct timeval pts, unsigned duration);
    long get_bandwidth_usage_data(int stream_index, const char **resource);
  protected:
	unsigned long run();
  private:
	void _push_data_to_sink (int sink_index, timestamp_t pts, const uint8_t*inbuf, size_t sz);
	static rtsp_context_t *_init_subsessions(rtsp_context_t *context);
	void _set_position(timestamp_t time);
	void _cancel();
	lib::critical_section m_critical_section;
	rtsp_context_t* m_context;
	timestamp_t m_clip_begin;
	timestamp_t m_clip_end;
	timestamp_t m_seektime;
#ifndef CLIP_BEGIN_CHANGED
	bool m_seektime_changed; // true if either m_clip_begin or m_seektime has changed
#else
	bool m_clip_begin_changed;	// True if m_clip_begin has changed.
#endif

};


}
} //end namespaces

#endif
