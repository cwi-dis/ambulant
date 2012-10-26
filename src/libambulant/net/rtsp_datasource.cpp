// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

#include "ambulant/config/config.h"
#include "ambulant/net/ffmpeg_audio.h"
#include "ambulant/net/ffmpeg_video.h"
#include "ambulant/net/rtsp_datasource.h"
#include "ambulant/net/demux_datasource.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"
#include "GroupsockHelper.hh"
#include <cfloat>

///// Added by Bo Gao begin 2007-11-07
//AVCodecParserContext * h264parserctx;

using namespace ambulant;
using namespace net;

//#define AM_DBG
// turn on the AM_DBG will hang on ambulant when use rtp over tcp
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define DESIRED_AUDIO_BUF_SIZE 100000
#define DESIRED_VIDEO_BUF_SIZE 2000000

#if AMBULANT_MSVC == 1500
// VC9 doesn't seem to define this
inline long long abs(long long i) { return i < 0 ? -i : i; }
#endif

// Helper routines: callback functions passed to live555 that will call back to our methods.
static void
after_reading_audio_stub(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	rtsp_demux* demux = (rtsp_demux*) data;
	assert(demux);
	demux->after_reading_audio((size_t)sz, truncated, pts, duration);
}

static void
after_reading_video_stub(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	rtsp_demux* demux = (rtsp_demux*) data;
	assert(demux);
	demux->after_reading_video((size_t)sz, truncated, pts, duration);
}

static void
on_source_close(void* data)
{
	rtsp_context_t* context = (rtsp_context_t*) data;
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::on_source_close() context=0x%x", data);
	if (context) {
		context->eof = true;
		context->blocking_flag = ~0;
	}
}


ambulant::net::rtsp_demux::rtsp_demux(rtsp_context_t* context, timestamp_t clip_begin, timestamp_t clip_end)
:	m_context(context),
	m_clip_begin(clip_begin),
	m_clip_end(clip_end),
	m_seektime(0),
#ifndef CLIP_BEGIN_CHANGED
	m_seektime_changed(false)
#else
	m_clip_begin_changed(false)
#endif
{
	assert(m_clip_begin >= 0);
#ifndef CLIP_BEGIN_CHANGED
	if ( m_clip_begin ) m_seektime_changed = true;
#else
	if ( m_clip_begin ) m_clip_begin_changed = true;
#endif

	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::rtsp_demux(0x%x)", (void*) this);

}

ambulant::net::rtsp_demux::~rtsp_demux() {
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::~rtsp_demux(0x%x)\n", (void*) this);
	delete m_context;
}

static void watchDog (rtsp_context_t *context) {
	// Call this again, after a brief delay:
	int uSecsToDelay = 100000; // 100 ms
	context->idle_time += uSecsToDelay;
	if (context->last_expected_pts > 0 &&
		context->highest_pts_seen + context->idle_time > context->last_expected_pts)
	{
		context->eof = true;
	}
	AM_DBG lib::logger::get_logger()->debug("watchDog: idle_time %lld, highest pts %lld, last pts %lld eof=%d", context->idle_time, context->highest_pts_seen, context->last_expected_pts,context->eof);
	context->blocking_flag = ~0;
	context->env->taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)watchDog, context);
}

void
ambulant::net::rtsp_demux::add_datasink(demux_datasink *parent, int stream_index)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::add_datasink(0x%x, parent=0x%x, stream_index=%d, m_context->nsinks=%d)", (void*) this,parent,stream_index,m_context->nsinks);
	m_critical_section.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_context && m_context->sinks && m_context->sinks[stream_index] == 0);
	m_context->sinks[stream_index] = parent;
	parent->add_ref();
	m_context->nsinks++;
	m_critical_section.leave();
}

void
ambulant::net::rtsp_demux::remove_datasink(int stream_index)
{
	m_critical_section.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_context && m_context->sinks);
	demux_datasink *ds = m_context->sinks[stream_index];
	m_context->sinks[stream_index] = 0;
	if (ds) m_context->nsinks--;
	m_context->blocking_flag = ~0;
	m_critical_section.leave();
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::remove_datasink(0x%x), ds=0x%x,stream_index=%d, m_context->nsinks=%d", (void*) this,ds,stream_index,m_context->nsinks);
	// XXXJACK This code suffers from the same problem as the ffmpeg_demux
	// code: if may get into a deadlock if we hold the lock
	// and it may deallocate ds while it's being used in run() otherwise.
	// But: it doesn't seem to occur just yet, so we live dangerously:-)
	if (ds) {
		// signal EOF
		ds->push_data(0, 0, 0);
		ds->release();
	}
	if (m_context->nsinks <= 0) _cancel();
}

rtsp_context_t*
ambulant::net::rtsp_demux::supported(const net::url& url)
{
	if (url.get_protocol() != "rtsp") return NULL;
	rtsp_context_t *context = new rtsp_context_t();
	context->nstream = 0;

	// setup the basics.
	context->scheduler = BasicTaskScheduler::createNew();
	if (!context->scheduler) {
		lib::logger::get_logger()->trace("ambulant::net::rtsp_demux(net::url& url) failed to create scheduler");
		lib::logger::get_logger()->error(gettext("RTSP Connection Failed"));
		delete context;
		return NULL;
	}

	context->env = BasicUsageEnvironment::createNew(*context->scheduler);
	if (!context->env) {
		lib::logger::get_logger()->trace("ambulant::net::rtsp_demux(net::url& url) failed to create UsageEnvironment");
		lib::logger::get_logger()->error(gettext("RTSP Connection Failed"));
		delete context;
		return NULL;
	}
	// setup a rtp session
	int verbose = 0;
	context->rtsp_client = RTSPClient::createNew(*context->env, verbose, "AmbulantPlayer");
	if (!context->rtsp_client) {
		lib::logger::get_logger()->trace("ambulant::net::rtsp_demux(net::url& url) failed to create a RTSP Client");
		lib::logger::get_logger()->error(gettext("RTSP Connection Failed"));
		delete context;
		return NULL;
	}

	std::string str_url = url.get_url();
	const char* ch_url = str_url.c_str();
	assert(ch_url);
	context->sdp = context->rtsp_client->describeURL(ch_url);
	if (!context->sdp) {
		lib::logger::get_logger()->trace("%s: describeURL failed (no server available, not rtsp, or url not found?)", ch_url);
		lib::logger::get_logger()->error(gettext("RTSP Connection Failed"));
		delete context;
		return NULL;
	}

	AM_DBG lib::logger::get_logger()->debug("rtsp_demux: describe(\"%s\") -> \"%s\"", ch_url, context->sdp);
	context->media_session = MediaSession::createNew(*context->env, context->sdp);
	if (!context->media_session) {
		lib::logger::get_logger()->trace("%s: failed to create a MediaSession, sdp=%s", ch_url, context->sdp);
		lib::logger::get_logger()->error(gettext("RTSP Connection Failed"));
		delete context;
		return NULL;
	}
	context->duration = context->media_session->playEndTime();
	if (context->duration == 0) {
		context->duration = DBL_MAX;
	}
	context->last_expected_pts = (timestamp_t) (context->duration*1000000); // do not skip last frame
	AM_DBG lib::logger::get_logger()->debug("rtps_demux::supported: last_expected_pts = %ld", context->last_expected_pts);
	// next set up the rtp subsessions.
	context = _init_subsessions(context);
	if (context == NULL) return NULL;

	lib::logger::get_logger()->debug("rtps_demux::supported(%s): duration=%ld", ch_url, context->last_expected_pts);
	return context;

}

timestamp_t
ambulant::net::rtsp_demux::get_clip_begin()
{
	timestamp_t rv;
#if 0
	// XXXJACK: this lock can cause a race: if demux_datasource calls us
	// it's holding its lock, but if we are in after_reading_xxxx we are holding
	// our lock and trying to obtain the demux_datasource lock.
	m_critical_section.enter();
#endif
	rv = m_clip_begin;
#if 0
	m_critical_section.leave();
#endif
	return rv;
}

timestamp_t
ambulant::net::rtsp_demux::get_clip_end()
{
	timestamp_t rv;
	m_critical_section.enter();
	rv = m_clip_end;
	m_critical_section.leave();
	return rv;
}

void
ambulant::net::rtsp_demux::read_ahead(timestamp_t time)
{
	m_critical_section.enter();
	AM_DBG lib::logger::get_logger()->debug("rtsp_demux::read_ahead(0x%x),	time=%lld, m_clip_begin was %lld",	this, time, m_clip_begin);
#ifndef CLIP_BEGIN_CHANGED
	m_clip_begin = time;
	m_seektime_changed = true;
#else
	if (m_clip_begin != time) {
		m_clip_begin = time;
		m_clip_begin_changed = true;
	}
#endif

	m_critical_section.leave();
}

void
ambulant::net::rtsp_demux::seek(timestamp_t time)
{
	m_critical_section.enter();
	AM_DBG lib::logger::get_logger()->debug("rtsp_demux::seek(0x%x),  time=%lld, m_clip_begin was %lld",this, time, m_clip_begin);
	assert( time >= 0);
#ifndef CLIP_BEGIN_CHANGED
	m_seektime = time;
	m_seektime_changed = true;
	m_clip_begin = time;
#else
	m_clip_begin = time;
	m_clip_begin_changed = true;
#endif
	m_context->eof = false;
	m_context->highest_pts_seen = 0;
	m_critical_section.leave();
}

void
ambulant::net::rtsp_demux::set_clip_end(timestamp_t clip_end)
{
  AM_DBG lib::logger::get_logger()->debug("rtsp_demux::set_clip_end(0x%x),	clip_end=%lld, m_clip_end was %lld",this, clip_end, m_clip_end);
	m_critical_section.enter();
	m_clip_end = clip_end;
	m_critical_section.leave();
}

long
ambulant::net::rtsp_demux::get_bandwidth_usage_data(int stream_index, const char **resource)
{
	m_critical_section.enter();
    assert(stream_index >= 0 && stream_index <= MAX_STREAMS);
    long rv = m_context->data_consumed[stream_index];
    m_context->data_consumed[stream_index] = 0;
    *resource = "rtsp";
	m_critical_section.leave();
    return rv;
}


static unsigned char* parseH264ConfigStr(char const* configStr, size_t& configSize);

rtsp_context_t *
ambulant::net::rtsp_demux::_init_subsessions(rtsp_context_t *context)
{
	assert(context->extraPacketHeaderData == NULL);
	assert(context->extraPacketHeaderSize == 0);
	assert(context->configData == NULL);
	assert(context->configDataLen == 0);
	assert(context->initialPacketData == NULL);
	assert(context->initialPacketDataLen == 0);
	MediaSubsession* subsession;
	MediaSubsessionIterator iter(*context->media_session);
	int stream_index = 0;
	for(stream_index=0, (subsession = iter.next()); subsession != NULL; stream_index++, (subsession = iter.next())) {
		if (!subsession->initiate()) {
			lib::logger::get_logger()->trace("rtsp_demux: failed to initiate subsession for medium \"%s\"", subsession->mediumName());
			lib::logger::get_logger()->error(gettext("RTSP Connection Failed"));
			delete context;
			return NULL;
		}
		const char *mediumName = subsession->mediumName();
		if (strcmp(mediumName, "audio") == 0) {
			if (context->audio_stream >= 0) {
				lib::logger::get_logger()->trace("rtsp_demux: ignoring additional audio stream");
				continue;
			}
			context->audio_stream = stream_index;
			context->audio_subsession = subsession;
			context->audio_codec_name = subsession->codecName();
			context->audio_fmt = audio_format("live", context->audio_codec_name);
			AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), audio codecname :%s ",context->audio_codec_name);
			int rtp_sock_num = subsession->rtpSource()->RTPgs()->socketNum();
			increaseReceiveBufferTo(*context->env, rtp_sock_num, DESIRED_AUDIO_BUF_SIZE);
#ifdef WITH_FFMPEG
			// As of August 2008, AAC audio also needs extra configuration data
			// Unfortunately, this code does not work: the Darwin server does not
			// provide this info the the sdl parameters. Maybe it is the first packet?
			if (!strcmp(context->audio_codec_name, "MPEG4-GENERIC")) {
				unsigned configLen;
				unsigned char* configData;
				configData = parseGeneralConfigStr(subsession->fmtp_config(), configLen);
				AVCodecContext *ffcon = ffmpeg_alloc_partial_codec_context(false, "MPEG4-GENERIC");
				ffcon->extradata = (uint8_t *)malloc(configLen + FF_INPUT_BUFFER_PADDING_SIZE);
				assert(ffcon->extradata);
				memcpy(ffcon->extradata, configData, configLen);
				ffcon->extradata_size = configLen;
				context->audio_fmt = audio_format("ffmpeg", ffcon);
			}
#endif // WITH_FFMPEG
		} else if (strcmp(mediumName, "video") == 0) {
			if (context->video_stream >= 0) {
				lib::logger::get_logger()->trace("rtsp_demux: ignoring additional video stream");
				continue;
			}
			context->video_stream = stream_index;
			context->video_subsession = subsession;
			context->video_codec_name = subsession->codecName();
			context->video_fmt = video_format("live", context->video_codec_name);
			AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), video codecname :%s ",context->video_codec_name);
			int rtp_sock_num = subsession->rtpSource()->RTPgs()->socketNum();
			increaseReceiveBufferTo(*context->env, rtp_sock_num, DESIRED_VIDEO_BUF_SIZE);
			// Try by Jack: disable waiting for packets, hope this fixes reorder problem.
			subsession->rtpSource()->setPacketReorderingThresholdTime(0);
			//For MP4V-ES video format we need to insert a packet into the RTSP stream
			//which should be present in the 'config' MIME parameter which should be present hopefully in the SDP description
			//this idea was copied from mplayer libmpdemux/demux_rtp.cpp
			if(strcmp(context->video_codec_name, "MP4V-ES")==0) {
				unsigned initialPacketDataLen;
				unsigned char* initialPacketData
					= parseGeneralConfigStr(subsession->fmtp_config(), initialPacketDataLen);
				context->initialPacketData = initialPacketData;
				context->initialPacketDataLen = (size_t)initialPacketDataLen;

			}
			if ( !strcmp( context->video_codec_name, "H264")){
				// H264 not only needs a magic first packet, but also four magic bytes at the
				// start of each subsequent packet.
				context->extraPacketHeaderSize = 4;
				context->extraPacketHeaderData = (unsigned char *)malloc(4);
				assert(context->extraPacketHeaderData);
				context->extraPacketHeaderData[0] = 0x00;
				context->extraPacketHeaderData[1] = 0x00;
				context->extraPacketHeaderData[2] = 0x00;
				context->extraPacketHeaderData[3] = 0x01;
				// Also, live555 doesn't deliver H264 streams with correct packetization (sigh)
				context->notPacketized = true;
				size_t configLen;
				unsigned char* configData;

				configData = parseH264ConfigStr(subsession->fmtp_spropparametersets(), configLen);
				context->configData = (unsigned char *)malloc(configLen + FF_INPUT_BUFFER_PADDING_SIZE);
				assert(context->configData);
				memcpy(context->configData, configData, configLen);
				context->configDataLen = configLen;
#ifdef WITH_FFMPEG
				// The configData contains stuff the H264 decoder needs to do its work. We need to
				// pass that to ffmpeg through the AVCodecContext extradata field.
				// Therefore, we change the video_format from {"live", "H264"} to
				// {"ffmpeg", AVCodecContext}.
				// If anything fails (it shouldn't) we just continue, we'll get the same error later.
				AVCodecContext *ffcon = ffmpeg_alloc_partial_codec_context(true, "H264");
				ffcon->extradata = context->configData;
				ffcon->extradata_size = (int)context->configDataLen;
				context->video_fmt = video_format("ffmpeg", ffcon);
#endif
			}
		} else {
			lib::logger::get_logger()->trace("rtsp_demux: ignoring subsession with unknown mediaName \"%s\"", mediumName);
			continue;
		}
		bool prefer_tcp = common::preferences::get_preferences()->m_prefer_rtsp_tcp;
		bool ok;
		if (prefer_tcp) {
			AM_DBG lib::logger::get_logger()->debug("rtsp: using TCP as transport stream");
			ok = (context->rtsp_client->setupMediaSubsession(*subsession, false, true) != 0);
		} else {
			ok = (context->rtsp_client->setupMediaSubsession(*subsession, false, false) != 0);
		}
		if (!ok) {
			lib::logger::get_logger()->trace("rtsp: setup command to subsession failed");
			delete context;
			return NULL;
		}
	}
	context->nstream = stream_index;
	return context;
}

unsigned long
ambulant::net::rtsp_demux::run()
{

	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() called (%d)", m_context->need_audio);
	if (!m_context->media_session) {
		lib::logger::get_logger()->error(gettext("playing RTSP connection failed"));
		return 1;
	}

	//xxxbo 13 nov. 2009
	AM_DBG lib::logger::get_logger()->debug("rtsp_demux::run() m_clip_begin=%lld, m_seektime = %lld playMediaSession(%f)", (long long int)m_clip_begin, (long long int)m_seektime,float((m_clip_begin+m_seektime)/1000000.0));

	if(!m_context->rtsp_client->playMediaSession(*m_context->media_session, float((m_clip_begin)/1000000.0), -1.0F, 1.0F)) {
		lib::logger::get_logger()->error(gettext("playing RTSP connection failed"));
		return 1;
	}
#ifndef CLIP_BEGIN_CHANGED
	m_seektime_changed = false;
#else
	m_clip_begin_changed = false;
#endif
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() starting the loop ");
	m_critical_section.enter();
	add_ref();

	// Schedul our watchdog timer.
	watchDog(m_context);

	while(!m_context->eof && !exit_requested()) {
		AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run: start another loop iteration");
		m_context->blocking_flag = 0;

		// First thing to do for each loop iteration: check whether we need to seek.
#ifndef CLIP_BEGIN_CHANGED
		if (m_seektime_changed) {
#else
		if (m_clip_begin_changed) {
#endif
			// Note: we have not tested (yet) how seeking influences the timestamps returned by live555. Needs to be
			// tested later.
#ifndef CLIP_BEGIN_CHANGED
			//float seektime_secs = (m_clip_begin+m_seektime)/1000000.0;
			double seektime_secs = 0.0;
			if (m_seektime == 0) {
				seektime_secs = m_clip_begin/1000000.0;
			} else {
				seektime_secs = m_seektime/1000000.0;
			}
#else
			double seektime_secs = m_clip_begin/1000000.0;
#endif
			AM_DBG lib::logger::get_logger()->debug("rtsp_demux::run: pauseMediaSession(), seeking to %f", seektime_secs);
			if(!m_context->rtsp_client->pauseMediaSession(*m_context->media_session)) {
				lib::logger::get_logger()->error(gettext("pausing RTSP media session failed"));
			}
			//xxxbo 13 nov. 2009
			AM_DBG lib::logger::get_logger()->debug("rtsp_demux::run(0x%x) m_clip_begin=%d, playMediaSession(%f)",	 this, m_clip_begin, seektime_secs);
			if(!m_context->rtsp_client->playMediaSession(*m_context->media_session, (float)seektime_secs, -1.0F, 1.0F)) {
				lib::logger::get_logger()->error(gettext("resuming RTSP media session failed"));
			}
#ifndef CLIP_BEGIN_CHANGED
			m_seektime_changed = false;
#else
			m_clip_begin_changed = false;
#endif
		}
		if (m_context->audio_subsession && m_context->need_audio) {
			assert(!m_context->audio_packet);
			m_context->audio_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
			AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() Calling getNextFrame for an audio frame");
			m_context->need_audio = false;
			m_critical_section.leave();
			m_context->audio_subsession->readSource()->getNextFrame(m_context->audio_packet, MAX_RTP_FRAME_SIZE, after_reading_audio_stub, this,  on_source_close, m_context);
			m_critical_section.enter();
		}
		if (m_context->video_subsession && m_context->need_video) {
			// XXXJACK We don't actually need to malloc every time, we could probably reuse the old one if it was copied.
			assert(!m_context->video_packet);
			m_context->video_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
			//std::cout << " MAX_RTP_FRAME_SIZE = " << MAX_RTP_FRAME_SIZE;
			AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() Calling getNextFrame for an video frame");
			m_context->need_video = false;
			AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() video_packet 0x%x", m_context->video_packet);
			
			m_critical_section.leave();
			m_context->video_subsession->readSource()->getNextFrame(&m_context->video_packet[m_context->extraPacketHeaderSize], (unsigned)(MAX_RTP_FRAME_SIZE-m_context->extraPacketHeaderSize), after_reading_video_stub, this, on_source_close, m_context);
			m_critical_section.enter();
		}

		AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() blocking_flag: 0x%x, %d, need_audio %d", &m_context->blocking_flag, m_context->blocking_flag, m_context->need_audio);
		TaskScheduler& scheduler = m_context->env->taskScheduler();
		
		m_critical_section.leave();
		scheduler.doEventLoop(&m_context->blocking_flag);
		m_critical_section.enter();
	}
	for (int i=0; i<MAX_STREAMS; i++) {
		demux_datasink *sink = m_context->sinks[i];
		if (sink) {
			_push_data_to_sink(i, 0, 0, 0);
			sink->release();
			m_context->sinks[i] = NULL;
		}
	}
	m_context->nsinks = 0;
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run(0x%x): returning", (void*)this);
	m_critical_section.leave();
	release();
	return 0;
}

void
ambulant::net::rtsp_demux::_cancel()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::_cancel(0x%x): m_context=0x%x rtspClient=0x%x mediaSession=0x%x", (void*)this, m_context, m_context?m_context->rtsp_client:0,m_context?m_context->media_session:0);
	if (m_context) {
		m_context->eof = true;
		m_context->blocking_flag = ~0;
	}
	if (is_running())
		stop();
}

void
ambulant::net::rtsp_demux::cancel()
{
	m_critical_section.enter();
	_cancel();
	m_critical_section.leave();
}

void
rtsp_demux::after_reading_audio(size_t sz, unsigned truncated, struct timeval pts, unsigned dur)
{
	m_critical_section.enter();
	AM_DBG lib::logger::get_logger()->debug("after_reading_audio: called sz = %d, truncated = %d, pts=%lld.%ld, %d",sz , truncated, pts.tv_sec,	 pts.tv_usec, dur);
	if (truncated)
		lib::logger::get_logger()->trace("rtsp_demux: truncated audio packet");

	//xxxbo: 13-nov-2009
	AM_DBG lib::logger::get_logger()->debug("after_reading_audio: pts is %d.%ld s", pts.tv_sec, pts.tv_usec);

	assert(m_context);
	assert(m_context->audio_packet);
	assert(m_context->audio_stream >= 0);

	// For the first packet, we remember the timestamp so we can convert Live's wallclock timestamps to
	// our zero-based timestamps.
	if (m_context->first_sync_time.tv_sec == 0 && m_context->first_sync_time.tv_usec == 0 ) {
		m_context->first_sync_time.tv_sec = pts.tv_sec;
		m_context->first_sync_time.tv_usec = pts.tv_usec;
		m_context->last_pts=0;
	}
#ifdef AM_DBG
	if (m_context->media_session) {
		MediaSession* ms = m_context->media_session;
		//		Boolean hasBeenSynchronized = ms->hasBeenSynchronizedUsingRTCP();
		double start_time = ms->playStartTime();
		double stop_time = ms->playEndTime();
		// lib::logger::get_logger()->debug("after_reading_audio: hasBeenSynchronized=%d, start_time=%f, stop_time=%f ", hasBeenSynchronized, start_time, stop_time);
		AM_DBG lib::logger::get_logger()->debug("after_reading_audio: start_time=%f, stop_time=%f ", start_time, stop_time);
	}
#endif
	if (!m_context->first_sync_time_set) {
		// We have not been synced yet. If the video stream has been synced for this packet
		// we can set the epoch of the timing info.
		MediaSubsession* subsession = m_context->audio_subsession;

		if (subsession) {
			// Set the packet's presentation time stamp, depending on whether or
			// not our RTP source's timestamps have been synchronized yet:
			// This idea is borrowed from mplayer at demux_rtp.cpp::after_reading
			Boolean hasBeenSynchronized = subsession->rtpSource()->hasBeenSynchronizedUsingRTCP();
			if (hasBeenSynchronized) {
				AM_DBG lib::logger::get_logger()->debug("after_reading_audio: resync video, from %ds %dus to %ds %dus", m_context->first_sync_time.tv_sec, m_context->first_sync_time.tv_usec, pts.tv_sec, pts.tv_usec);
				m_context->first_sync_time.tv_sec = pts.tv_sec;
				m_context->first_sync_time.tv_usec = pts.tv_usec;
				m_context->last_pts = 0;
				m_context->first_sync_time_set = true;
			}
		}
	}

	AM_DBG lib::logger::get_logger()->debug("after_reading_audio: first_sync_time is %d.%ld s", m_context->first_sync_time.tv_sec, m_context->first_sync_time.tv_usec);

	timestamp_t rpts =	(timestamp_t)(pts.tv_sec - m_context->first_sync_time.tv_sec) * 1000000LL  +  (timestamp_t) (pts.tv_usec - m_context->first_sync_time.tv_usec);

	AM_DBG lib::logger::get_logger()->debug("after_reading_audio: rtps is %lld us", rpts);

	if(m_context->sinks[m_context->audio_stream]) {
		AM_DBG lib::logger::get_logger()->debug("after_reading_audio: calling _push_data_to_sink");
		_push_data_to_sink(m_context->audio_stream, rpts, (uint8_t*) m_context->audio_packet, sz);
		AM_DBG lib::logger::get_logger()->debug("after_reading_audio: calling push_data_to_sink done");
	}
	assert (m_context->audio_packet);
	free(m_context->audio_packet);
	m_context->audio_packet = NULL;
	AM_DBG lib::logger::get_logger()->debug("after reading audio: rpts=%lld, end=%lld\n\n", rpts, m_context->last_expected_pts);

	// Note by Jack: we use strict greater than zero for last_expected_pts. It seems live streams will have
	// a duration of zero, and therefore a last_expected_pts of zero.
	if (m_context->last_expected_pts > 0 && rpts >= m_context->last_expected_pts) {
		AM_DBG lib::logger::get_logger()->debug("after_reading_audio: last_pts = %lld\n\n", rpts);
		m_context->eof = true;
	}
	if (rpts > m_context->highest_pts_seen)
		m_context->highest_pts_seen = rpts;
	m_context->idle_time = 0;
	m_context->blocking_flag = ~0;
	m_context->need_audio = true;
	m_critical_section.leave();
}

void
rtsp_demux::after_reading_video(size_t sz, unsigned truncated, struct timeval pts, unsigned dur)
{
	m_critical_section.enter();
	assert(m_context);
	AM_DBG lib::logger::get_logger()->debug("after_reading_video: called sz = %d, truncated = %d pts=(%d s, %d us), dur=%d", sz, truncated, pts.tv_sec, pts.tv_usec, dur);
	if (truncated)
		lib::logger::get_logger()->trace("rtsp_demux: truncated video packet");
	assert(m_context->video_packet);
	assert(m_context->video_stream >= 0);
	
	// For the first packet, we remember the timestamp so we can convert Live's wallclock timestamps to
	// our zero-based timestamps.
	if (m_context->first_sync_time.tv_sec == 0 && m_context->first_sync_time.tv_usec == 0 ) {
		m_context->first_sync_time.tv_sec = pts.tv_sec;
		m_context->first_sync_time.tv_usec = pts.tv_usec;
		m_context->last_pts=0;
	}
	// Some formats (notably mp4v and h264) get an initial synthesized packet of data. This is
	// where we deliver that.
	if(m_context->initialPacketDataLen > 0) {
		AM_DBG lib::logger::get_logger()->debug("after_reading_video: inserting initialPacketData packet, size=%d", (int)m_context->initialPacketDataLen);
		if (m_context->notPacketized) {
			assert(m_context->vbuffer == NULL);
			assert(m_context->vbufferlen == 0);
			m_context->vbuffer = m_context->initialPacketData;
			m_context->vbufferlen = m_context->initialPacketDataLen;
			m_context->initialPacketData = NULL;
			m_context->initialPacketDataLen = 0;
		} else {
			_push_data_to_sink(m_context->video_stream, 0, (uint8_t*) m_context->initialPacketData , m_context->initialPacketDataLen);
		}
	}
	
	if (!m_context->first_sync_time_set) {
		// We have not been synced yet. If the video stream has been synced for this packet
		// we can set the epoch of the timing info.
		MediaSubsession* subsession = m_context->video_subsession;
		
		if (subsession) {
			// Set the packet's presentation time stamp, depending on whether or
			// not our RTP source's timestamps have been synchronized yet:
			// This idea is borrowed from mplayer at demux_rtp.cpp::after_reading
			Boolean hasBeenSynchronized = subsession->rtpSource()->hasBeenSynchronizedUsingRTCP();
			if (hasBeenSynchronized) {
				AM_DBG lib::logger::get_logger()->debug("after_reading_video: resync video, from %ds %dus to %ds %dus", m_context->first_sync_time.tv_sec, m_context->first_sync_time.tv_usec, pts.tv_sec, pts.tv_usec);
				m_context->first_sync_time.tv_sec = pts.tv_sec;
				m_context->first_sync_time.tv_usec = pts.tv_usec;
				m_context->last_pts = 0;
				m_context->first_sync_time_set = true;
			}
		}
	}
	timestamp_t rpts =	(timestamp_t)(pts.tv_sec - m_context->first_sync_time.tv_sec) * 1000000LL  +  (timestamp_t) (pts.tv_usec - m_context->first_sync_time.tv_usec);

	AM_DBG lib::logger::get_logger()->debug("after_reading_video: rpts = %lld", rpts);
	timestamp_t delta_pts = abs(rpts-m_context->last_pts);
	if (m_context->frame_duration == 0 || (delta_pts != 0 && delta_pts < m_context->frame_duration)) 
		m_context->frame_duration = delta_pts;
	
	// We may beed to insert a few bytes at the front of the packet. run() has made sure
	// there's room for that.
	if (m_context->extraPacketHeaderSize) {
		// This magic was gleamed from mplayer, file demux_rtp.cpp and vlc, file live555.cpp.
		// The space in video_packet was already left free in run().
		memcpy(m_context->video_packet, m_context->extraPacketHeaderData, m_context->extraPacketHeaderSize);
	}
	
	if (m_context->notPacketized) {
		// We have to re-packetize ourselves. We do this by combining all data with the same timestamp.
		if (rpts != 0 && rpts != m_context->last_pts && m_context->vbuffer) {
			// The new fragment has a different timestamp from what we have buffered. We first emit the buffered data.
			// Determine the pts we want to send to upper layers. The pts live555 gives us seems to be incorrect:
			// the data we get in the packets is in strictly increasing order, but the timestamps are not.
			// We re-invent the timestamp.
			timestamp_t out_pts;
			if (rpts > m_context->last_pts)
				out_pts = rpts;
			else 
				out_pts = m_context->last_pts + m_context->frame_duration;

			AM_DBG lib::logger::get_logger()->debug("Video packet length (buffered)=%d, timestamp=%lld, rpts=%lld synced=%d", (int)m_context->vbufferlen, out_pts+m_clip_begin, rpts+m_clip_begin, m_context->video_subsession->rtpSource()->hasBeenSynchronizedUsingRTCP());
			_push_data_to_sink(m_context->video_stream, out_pts, (uint8_t*) m_context->vbuffer, m_context->vbufferlen);
			free(m_context->vbuffer);
			m_context->vbuffer = NULL;
			m_context->vbufferlen = 0;
			//m_context->last_pts=rpts;
			m_context->last_pts = out_pts;
		}
		// We store the new data but don't process it yet (the next fragment may belong to the same packet).
		if (m_context->vbuffer == NULL) {
			// If it's the first fragment, steal it.
			assert(m_context->vbufferlen == 0);
			m_context->vbuffer = m_context->video_packet;
			m_context->vbufferlen = sz+m_context->extraPacketHeaderSize;
			m_context->video_packet = NULL;
		} else {
			// We cannot reuse the video_packet because there is already data in vbuffer.
			// Realloc and copy.
			m_context->vbuffer = (unsigned char *)realloc(m_context->vbuffer, m_context->vbufferlen+sz+m_context->extraPacketHeaderSize);
			if (m_context->vbuffer == NULL) {
				lib::logger::get_logger()->trace("rtsp: out of memory rebuffering. Dropping packet.");
				m_context->vbufferlen = 0;
				goto done;
			}
			memcpy(m_context->vbuffer+m_context->vbufferlen, m_context->video_packet, sz+m_context->extraPacketHeaderSize);
			m_context->vbufferlen += sz+m_context->extraPacketHeaderSize;
		}
	} else {
		// The data is correctly packetized. Simply send it upstream.
		
		// Determine the pts we want to send to upper layers. The pts live555 gives us seems to be incorrect:
		// the data we get in the packets is in strictly increasing order, but the timestamps are not.
		// We re-invent the timestamp.
		timestamp_t out_pts;
		if (rpts > m_context->last_pts)
			out_pts = rpts;
		else 
			out_pts = m_context->last_pts + m_context->frame_duration;
		
		AM_DBG lib::logger::get_logger()->debug("Video packet length %d+%d=%d, timestamp=%lld, rpts=%lld", sz, (int)m_context->extraPacketHeaderSize, sz+m_context->extraPacketHeaderSize, out_pts+m_clip_begin, rpts+m_clip_begin);
		_push_data_to_sink(m_context->video_stream, out_pts, (uint8_t*) m_context->video_packet, sz+m_context->extraPacketHeaderSize);
	}
	
done:
	// Record the pts of the last packet processed (not necessarily sent upstream, yet).
	m_context->last_pts=rpts;

	if (rpts > m_context->highest_pts_seen)
		m_context->highest_pts_seen = rpts;
	m_context->idle_time = 0;
	// Tell the main demux loop that we're ready for another packet.
	m_context->need_video = true;
	if (m_context->video_packet) free(m_context->video_packet);
	m_context->video_packet	 = NULL;
	if(m_context->initialPacketDataLen > 0){
		delete [] m_context->initialPacketData;
		m_context->initialPacketData = NULL;
		m_context->initialPacketDataLen = 0;
	}
	
	AM_DBG lib::logger::get_logger()->debug("after reading video: pts=%lld, end=%lld", m_context->last_pts, m_context->last_expected_pts);
	// Note by Jack: we use strict greater than zero for last_expected_pts. It seems live streams will have
	// a duration of zero, and therefore a last_expected_pts of zero.
	if (m_context->last_expected_pts > 0 && m_context->last_pts >= m_context->last_expected_pts) {
		lib::logger::get_logger()->debug("after_reading_video: last_pts = %lld", m_context->last_pts);
		m_context->eof = true;
	}
	m_context->blocking_flag = ~0;
	//XXX Do we need to free data here ?
	m_critical_section.leave();
}	
	
void
rtsp_demux::_push_data_to_sink (int sink_index, timestamp_t pts, const uint8_t* inbuf, size_t sz) {
	demux_datasink* sink = m_context->sinks[sink_index];
	bool accepted = false;

	// Keep statistics
	m_context->data_consumed[sink_index] += sz;

	while (sink && ! exit_requested() && ! accepted) {
		m_critical_section.leave();
		accepted = sink->push_data (pts, inbuf, sz);
		if (accepted) {
			m_critical_section.enter();
			return;
		}
		// Now we have to wait until there is room in the sink buffer.
		// sleep 10 millisec, hardly noticeable
		ambulant::lib::sleep_msec(10); // XXXX should be woken by readdone()
		m_critical_section.enter();
		sink = m_context->sinks[sink_index];
	}
}
// Following code from : vlc-1.0.5/modules/demux/live555.cpp
static size_t b64_decode( char *dest, char *src );

static unsigned char*
parseH264ConfigStr(char const* configStr, size_t& configSize)
{
	char *dup, *psz;
	int i, i_records = 1;

	if( configSize )
	configSize = 0;

	if( configStr == NULL || *configStr == '\0' )
		return NULL;

	//printf("configStr is %s\n", configStr);
#ifdef AMBULANT_PLATFORM_WIN32
	psz = dup = (char *)malloc(strlen(configStr)+1);
	strcpy(dup, configStr);
#else
	psz = dup = strdup( configStr );
#endif
	//printf("dup oirginal is %s\n", dup);

	/* Count the number of comma's */
	for( psz = dup; *psz != '\0'; ++psz )
	{
		if( *psz == ',')
		{
			++i_records;
			*psz = '\0';
		}
	}

	//printf("i_records is %d, and dup is %s\n", i_records, dup);
	unsigned char *cfg = new unsigned char[5 * strlen(dup)];
	psz = dup;
	for( i = 0; i < i_records; i++ )
	{
		cfg[configSize++] = 0x00;
		cfg[configSize++] = 0x00;
		cfg[configSize++] = 0x00;
		cfg[configSize++] = 0x01;

		//printf("configSize is %d\n", configSize);
		configSize += b64_decode( (char*)&cfg[configSize], psz );
		//printf("configSize is %d\n", configSize);
		psz += strlen(psz)+1;
		//printf("cfg %d is %x\n",i,cfg);
	}

	if( dup ) free( dup );
	// printf("cfg is %x\n",cfg);
	return cfg;
}


/*char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";*/
static size_t
b64_decode( char *dest, char *src )
{
	const char *dest_start = dest;
	int	 i_level;
	int	 last = 0;
	int	 b64[256] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
		52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
		41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1	  /* F0-FF */
		};

	for( i_level = 0; *src != '\0'; src++ )
	{
		int	 c;

		c = b64[(unsigned int)*src];
		if( c == -1 )
		{
			continue;
		}

		switch( i_level )
		{
		case 0:
			i_level++;
			break;
		case 1:
			*dest++ = ( last << 2 ) | ( ( c >> 4)&0x03 );
			i_level++;
			break;
		case 2:
			*dest++ = ( ( last << 4 )&0xf0 ) | ( ( c >> 2 )&0x0f );
			i_level++;
			break;
		case 3:
			*dest++ = ( ( last &0x03 ) << 6 ) | c;
			i_level = 0;
		}
		last = c;
	}

	*dest = '\0';

	return dest - dest_start;
}
