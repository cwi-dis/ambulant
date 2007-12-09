// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/config/config.h"
#include "ambulant/net/rtsp_datasource.h"
#include "ambulant/net/demux_datasource.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"
#include "GroupsockHelper.hh"

///// Added by Bo Gao begin 2007-11-07
AVCodecParserContext * h264parserctx;

using namespace ambulant;
using namespace net;

//#define AM_DBG 
// turn on the AM_DBG will hang on ambulant when use rtp over tcp
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define DESIRED_AUDIO_BUF_SIZE 100000
#define DESIRED_VIDEO_BUF_SIZE 2000000

// Helper routines: callback functions passed to live555 that will call back to our methods.
static void 
after_reading_audio_stub(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	rtsp_demux* demux = (rtsp_demux*) data;
	assert(demux);
	demux->after_reading_audio(sz, truncated, pts, duration);
}

static void 
after_reading_video_stub(void* data, unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	rtsp_demux* demux = (rtsp_demux*) data;
	assert(demux);
	demux->after_reading_video(sz, truncated, pts, duration);
}

static void 
on_source_close(void* data) 
{
	rtsp_context_t* context = (rtsp_context_t*) data;
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
	m_seektime_changed(false)
//,	m_critical_section()
{
	assert(m_clip_begin >= 0);
	if ( m_clip_begin ) m_seektime_changed = true;

	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::rtsp_demux(0x%x)", (void*) this);

#ifdef JACK_IS_NOT_CONVINCED_YET
	// XXXJACK suspect: we shoulnd't mess with clip_end here...
	if ( m_clip_end < 0 || m_clip_end > m_context->time_left) 
		m_clip_end = m_context->time_left;	
#endif
}

ambulant::net::rtsp_demux::~rtsp_demux() {
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::~rtsp_demux(0x%x)", (void*) this);
	delete m_context;
}

//#define	DUMMYTASK
#ifdef	DUMMYTASK
/*
[Live-devel] Shutdown of testRTSPonDemandServer
Ross Finlayson finlayson at live555.com
Fri Mar 31 11:01:58 PST 2006

    * Previous message: [Live-devel] Non Blocking doGetNextFrame
    * Messages sorted by: [ date ] [ thread ] [ subject ] [ author ]

>  I need to shutdown my hardware cleanly when the program terminates so I
>have added a watch variable to Eventloop. The variable gets set when a
>signal handler is invoked.
>That should stop the eventloop and then I shutdown my hardware.
>
>This works well when the RTSP server is streaming to clients. However,
>when there are no clients the eventloop does not seem respond to the
>watch variable.
>
>Is this to be expected?

This was unexpected, but not really a bug.  It happens because - when 
the server is sitting around waiting for a request - no 'events' are 
happening (no incoming packets, no delayed tasks), so the server sits 
forever in "select()", and so never gets to check the watch variable.

>  Is there a way to get around this?

Yes.  You can schedule a dummy task (that does nothing) to run 
periodically (e.g., every 100 ms).  This will ensure that the server 
leaves "select()" (to check the watch variable) at least every 100ms

Add the following just before "main()":

static void dummyTask(void* / * clientData * /) {
   // Call this again, after a brief delay:
   int uSecsToDelay = 100000; // 100 ms
   env->taskScheduler().scheduleDelayedTask(uSecsToDelay,
                                            (TaskFunc*)dummyTask, NULL);
}

And then, just before the call to "doEventLoop()", do

   dummyTask(NULL);


	Ross Finlayson
	Live Networks, Inc. (LIVE555.COM)
	<http://www.live555.com/>

-----------------
Also see: http://lists.live555.com/pipermail/live-devel/2006-April/004215.html
*/

static void dummyTask (UsageEnvironment* env /*clientData*/) {
	// Call this again, after a brief delay:
	int uSecsToDelay = 100000; // 100 ms
	env->taskScheduler().scheduleDelayedTask(uSecsToDelay,
						 (TaskFunc*)dummyTask, env);
}
#endif/*DUMMYTASK*/

void 
ambulant::net::rtsp_demux::add_datasink(demux_datasink *parent, int stream_index)
{
	m_critical_section.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_context && m_context->sinks && m_context->sinks[stream_index] == 0);
	m_context->sinks[stream_index] = parent;
	m_context->nsinks++;
	m_critical_section.leave();
}

void
ambulant::net::rtsp_demux::remove_datasink(int stream_index)
{
	m_critical_section.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_context && m_context->sinks && m_context->sinks[stream_index] != 0);
	if (m_context->sinks[stream_index])
		// signal EOF
		m_context->sinks[stream_index]->data_avail(0, 0, 0);
	m_context->sinks[stream_index] = 0;
	m_context->nsinks--;
	if (m_context->nsinks <= 0) _cancel();
	m_critical_section.leave();
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
		lib::logger::get_logger()->error("ambulant::net::rtsp_demux(net::url& url) failed to create scheduler");
		delete context;
		return NULL;
	}
	
	context->env = BasicUsageEnvironment::createNew(*context->scheduler);
	if (!context->env) {
		lib::logger::get_logger()->error("ambulant::net::rtsp_demux(net::url& url) failed to create UsageEnvironment");
		delete context;
		return NULL;
	}
	// setup a rtp session
	int verbose = 0;
	context->rtsp_client = RTSPClient::createNew(*context->env, verbose, "AmbulantPlayer");
	if (!context->rtsp_client) {
		lib::logger::get_logger()->error("ambulant::net::rtsp_demux(net::url& url) failed to create  a RTSP Client");
		delete context;
		return NULL;
	}
	
	std::string str_url = url.get_url();
	const char* ch_url = str_url.c_str();
	assert(ch_url);
	context->sdp = context->rtsp_client->describeURL(ch_url);
	if (!context->sdp) {
		lib::logger::get_logger()->error("%s: describeURL failed (no server available, not rtsp, or url not found?)", ch_url);
		//lib::logger::get_logger()->error("RTSP Connection Failed");
		delete context;	
		return NULL;
	}
	
	AM_DBG lib::logger::get_logger()->debug("rtsp_demux: describe(\"%s\") -> \"%s\"", ch_url, context->sdp);
	context->media_session = MediaSession::createNew(*context->env, context->sdp);
	if (!context->media_session) {
		lib::logger::get_logger()->error("%s: failed to create a MediaSession, sdp=%s", ch_url, context->sdp);
		//lib::logger::get_logger()->error("RTSP Connection Failed");
		delete context;		
		return NULL;
	}	
	context->duration = context->media_session->playEndTime();
	context->time_left = (timestamp_t) (context->duration*1000000 - 40000); // skip last frame
//	context->time_left = (timestamp_t) (context->duration*1000000); // do not skip last frame
	AM_DBG lib::logger::get_logger()->debug("rtps_demux::supported: time_left = %ld", context->time_left);
	// next set up the rtp subsessions.
	context = _init_subsessions(context);
	if (context == NULL) return NULL;
	
	lib::logger::get_logger()->debug("rtps_demux::supported(%s): duration=%ld", ch_url, context->time_left);
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
	m_clip_begin = time;
	m_seektime_changed = true;
	m_critical_section.leave();
}

void
ambulant::net::rtsp_demux::seek(timestamp_t time)
{
	m_critical_section.enter();
	m_seektime = time;
	m_seektime_changed = true;
	m_critical_section.leave();
}

static unsigned char* parseH264ConfigStr( char const* configStr,
                                          unsigned int& configSize );

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
			lib::logger::get_logger()->error("rtsp_demux: failed to initiate subsession for medium \"%s\"", subsession->mediumName());
			//lib::logger::get_logger()->error("RTSP Connection Failed");
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
			context->audio_codec_name = subsession->codecName();
			context->audio_fmt = audio_format("live", context->audio_codec_name);
			AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), audio codecname :%s ",context->audio_codec_name);
#ifdef JACK_IS_NOT_CONVINCED_YET
			context->audio_fmt.channels = 0; //Let the decoder (ffmpeg) find out the channels subsession->numChannels() returns channels -1 ???
			context->audio_fmt.bits = 16;
			//context->audio_fmt.samplerate = subsession->rtpSource()->timestampFrequency();
			context->audio_fmt.samplerate = 0;
#endif
			int rtp_sock_num = subsession->rtpSource()->RTPgs()->socketNum();
			increaseReceiveBufferTo(*context->env, rtp_sock_num, DESIRED_AUDIO_BUF_SIZE);
		} else if (strcmp(mediumName, "video") == 0) {
			if (context->video_stream >= 0) {
				lib::logger::get_logger()->trace("rtsp_demux: ignoring additional video stream");
				continue;
			}
			context->video_stream = stream_index;
			context->video_codec_name = subsession->codecName();
			context->video_fmt = video_format("live", context->video_codec_name);
			AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), video codecname :%s ",context->video_codec_name);
#ifdef JACK_IS_NOT_CONVINCED_YET
			//KB getting video_fmt from RTSP doesn't work 
			context->video_fmt.frameduration = (timestamp_t) (1000000.0/subsession->videoFPS());
			context->video_fmt.width = subsession->videoWidth();
			context->video_fmt.height = subsession->videoHeight();
			lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), width: %d, height: %d, FPS: %f",context->video_fmt.width, context->video_fmt.height, 1000000.0/context->video_fmt.frameduration);
#endif
			int rtp_sock_num = subsession->rtpSource()->RTPgs()->socketNum();
			increaseReceiveBufferTo(*context->env, rtp_sock_num, DESIRED_VIDEO_BUF_SIZE);

			//For MP4V-ES video format we need to insert a packet into the RTSP stream
			//which should be present in the 'config' MIME parameter which should be present hopefully in the SDP description
			//this idea was copied from mplayer libmpdemux/demux_rtp.cpp
			if(strcmp(context->video_codec_name, "MP4V-ES")==0) {
				unsigned configLen;
				unsigned char* configData 
					= parseGeneralConfigStr(subsession->fmtp_config(), configLen);
				context->initialPacketData = configData;
				context->initialPacketDataLen = configLen;
				
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
				unsigned configLen;
				unsigned char* configData;
				
				configData = parseH264ConfigStr(subsession->fmtp_spropparametersets(), configLen);
				context->configData = configData;
				context->configDataLen = configLen;
#ifdef WITH_FFMPEG
				// The configData contains stuff the H264 decoder needs to do its work. We need to
				// pass that to ffmpeg through the AVCodecContext extradata field.
				// Therefore, we change the video_format from {"live", "H264"} to
				// {"ffmpeg", AVCodecContext}.
				// If anything fails (it shouldn't) we just continue, we'll get the same error later.
				ffmpeg_live_h264_format(context->video_fmt, context->configData, context->configDataLen);
#endif
			}
		} else {
			lib::logger::get_logger()->trace("rtsp_demux: ignoring subsession with unknown mediaName \"%s\"", mediumName);
			continue;
		}
		bool prefer_tcp = common::preferences::get_preferences()->m_prefer_rtsp_tcp;
		bool ok;
		if (prefer_tcp) {
			/*AM_DBG*/ lib::logger::get_logger()->debug("rtsp: using TCP as transport stream");
			ok = context->rtsp_client->setupMediaSubsession(*subsession, false, true);
		} else {
			ok = context->rtsp_client->setupMediaSubsession(*subsession, false, false);
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
		lib::logger::get_logger()->error("playing RTSP connection failed");
		return 1;
	}
	if(!m_context->rtsp_client->playMediaSession(*m_context->media_session, (m_clip_begin+m_seektime)/1000000.0, -1.0, 1.0)) {
		lib::logger::get_logger()->error("playing RTSP connection failed");
		return 1;
	}
	m_seektime_changed = false;
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() starting the loop ");
	m_critical_section.enter();
	add_ref();
		
	while(!m_context->eof && !exit_requested()) {
		AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run: start another loop iteration");
		m_context->blocking_flag = 0;
		
		// First thing to do for each loop iteration: check whether we need to seek.
		if (m_seektime_changed) {
			// Note: we have not tested (yet) how seeking influences the timestamps returned by live555. Needs to be
			// tested later.
			float seektime_secs = (m_clip_begin+m_seektime)/1000000.0;
			AM_DBG lib::logger::get_logger()->debug("rtsp_demux::run: seeking to %f", seektime_secs);
			if(!m_context->rtsp_client->pauseMediaSession(*m_context->media_session)) {
				lib::logger::get_logger()->error("pausing RTSP media session failed");
			}
			if(!m_context->rtsp_client->playMediaSession(*m_context->media_session, seektime_secs, -1.0, 1.0)) {
				lib::logger::get_logger()->error("resuming RTSP media session failed");
			}
			m_seektime_changed = false;
		}
		
		// Next, we loop over the subsessions, and check each one for data availability.
		// We ignore all streams except audio or video streams.
		MediaSubsession* subsession;
		MediaSubsessionIterator iter(*m_context->media_session);
		while ((subsession = iter.next()) != NULL) {
			if (strcmp(subsession->mediumName(), "audio") == 0) {
				if(m_context->need_audio) {
					// XXXJACK We don't actually need to malloc every time, we could probably reuse the old one if it was copied.
					assert(!m_context->audio_packet);
					m_context->audio_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
					AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() Calling getNextFrame for an audio frame");
					m_context->need_audio = false;
					m_critical_section.leave();
					subsession->readSource()->getNextFrame(m_context->audio_packet, MAX_RTP_FRAME_SIZE, after_reading_audio_stub, this,  on_source_close ,m_context);
					m_critical_section.enter();
				}
			} else if (strcmp(subsession->mediumName(), "video") == 0) {
				if (m_context->need_video) {
					// XXXJACK We don't actually need to malloc every time, we could probably reuse the old one if it was copied.
					assert(!m_context->video_packet);
					m_context->video_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
					//std::cout << " MAX_RTP_FRAME_SIZE = " << MAX_RTP_FRAME_SIZE;
					AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() Calling getNextFrame for an video frame");
					m_context->need_video = false;
					AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() video_packet 0x%x", m_context->video_packet);
					m_critical_section.leave();
					subsession->readSource()->getNextFrame(&m_context->video_packet[m_context->extraPacketHeaderSize], MAX_RTP_FRAME_SIZE-m_context->extraPacketHeaderSize, after_reading_video_stub, this, on_source_close, m_context);
					m_critical_section.enter();
					
				}
			} else {
				AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() not interested in this data");
			}
		}
		
		AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() blocking_flag: 0x%x, %d, need_audio %d", &m_context->blocking_flag, m_context->blocking_flag, m_context->need_audio);		
		TaskScheduler& scheduler = m_context->env->taskScheduler();
		m_critical_section.leave();
#ifdef	DUMMYTASK
		dummyTask (m_context->env);
#endif/*DUMMYTASK*/
		scheduler.doEventLoop(&m_context->blocking_flag);
		m_critical_section.enter();
	}
	for (int i=0; i<MAX_STREAMS; i++) {
		demux_datasink *sink = m_context->sinks[i];
		if (sink)
			sink->data_avail(0, 0, 0);
	}
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run(0x%x): returning", (void*)this);
	if (m_context->extraPacketHeaderData) free(m_context->extraPacketHeaderData);
	release();
	m_critical_section.leave();
	return 0;
}

void
ambulant::net::rtsp_demux::_cancel()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::_cancel(0x%x): m_context=0x%x rtspClient=0x%x mediaSession=0x%x", (void*)this, m_context, m_context?m_context->rtsp_client:0,m_context?m_context->media_session:0);
	if (m_context) {
	 	m_context->eof = true;
		m_context->blocking_flag = 0;
	}
//	if (is_running())
//		stop();
	release();
}

void
ambulant::net::rtsp_demux::cancel()
{
	m_critical_section.enter();
	_cancel();
	m_critical_section.leave();
}

void 
rtsp_demux::after_reading_audio(unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	m_critical_section.enter();
	AM_DBG lib::logger::get_logger()->debug("after_reading_audio: called sz = %d, truncated = %d", sz, truncated);
	assert(m_context);
	assert(m_context->audio_packet);
	assert(m_context->audio_stream >= 0);
	timestamp_t rpts = (pts.tv_sec* 1000000 )+  pts.tv_usec;
	if(m_context->sinks[m_context->audio_stream]) {
		AM_DBG lib::logger::get_logger()->debug("after_reading_audio: calling data_avail");
		m_context->sinks[m_context->audio_stream]->data_avail(rpts, (uint8_t*) m_context->audio_packet, sz);
		AM_DBG lib::logger::get_logger()->debug("after_reading_audio: calling data_avail done");
	}
	assert (m_context->audio_packet);
	free(m_context->audio_packet);
	m_context->audio_packet = NULL;
	//XXX Do we need to free data here ?
	m_context->blocking_flag = ~0;
	m_context->need_audio = true;
	m_critical_section.leave();
}	

void 
rtsp_demux::after_reading_video(unsigned sz, unsigned truncated, struct timeval pts, unsigned duration)
{
	m_critical_section.enter();
	assert(m_context);
	/*AM_DBG*/ lib::logger::get_logger()->debug("after_reading_video: called sz = %d, truncated = %d pts=(%d s, %d us), dur=%d", sz, truncated, pts.tv_sec, pts.tv_usec, duration);
	assert(m_context->video_packet);
	assert(m_context->video_stream >= 0);
	
	// For the first packet, we remember the timestamp so we can convert Live's wallclock timestamps to
	// our zero-based timestamps.
	if (m_context->first_sync_time.tv_sec == 0 && m_context->first_sync_time.tv_usec == 0 ) {
		m_context->first_sync_time.tv_sec = pts.tv_sec;
		m_context->first_sync_time.tv_usec = pts.tv_usec;
		m_context->last_pts=0;
		// Some formats (notably mp4v and h264) get an initial synthesized packet of data. This is
		// where we deliver that.
#if 1 // XXXJACK
		if (m_context->configData) {
			m_context->vbuffer = m_context->configData;
			m_context->vbufferlen = m_context->configDataLen;
			m_context->configData = NULL;
		}
#endif
		if(m_context->initialPacketDataLen > 0) {
			/*AM_DBG*/ lib::logger::get_logger()->debug("after_reading_video: inserting initialPacketData packet, size=%d", m_context->initialPacketDataLen);
			if (m_context->notPacketized) {
				assert(m_context->vbuffer == NULL);
				assert(m_context->vbufferlen == 0);
				m_context->vbuffer = m_context->initialPacketData;
				m_context->vbufferlen = m_context->initialPacketDataLen;
				m_context->initialPacketData = NULL;
				m_context->initialPacketDataLen = 0;
			} else {
				m_context->sinks[m_context->video_stream]->data_avail(0, (uint8_t*) m_context->initialPacketData , m_context->initialPacketDataLen);
			}
		}
	}

	if (!m_context->first_sync_time_set) {
		// We have not been synced yet. If the video stream has been synced for this packet
		// we can set the epoch of the timing info. 
		MediaSubsession* subsession;
		MediaSubsessionIterator iter(*m_context->media_session);
		while ((subsession = iter.next()) != NULL) {
			if (strcmp(subsession->mediumName(), "video") == 0)
				break;
		}
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
	timestamp_t rpts =  (timestamp_t)(pts.tv_sec - m_context->first_sync_time.tv_sec) * 1000000LL  +  (timestamp_t) (pts.tv_usec - m_context->first_sync_time.tv_usec);
	// XXXJACK: Some code downstream expects timestamps to be file-based, i.e. when playing with clipBegin=10s it expects the first frame to have
	// timestamp=10s. Not sure this is correct, but for now we fix up the timestamp. I'm also not sure how this interacts with seeking.
	// Please remove this comment once it has been checked that the current behaviour is correct.
	AM_DBG lib::logger::get_logger()->debug("after_reading_video: called timestamp 0x%08.8x%08.8x, sec = %d, usec =  %d", (long)(rpts>>32), (long)(rpts&0xffffffff), pts.tv_sec, pts.tv_usec);
	
	
	demux_datasink *sink = m_context->sinks[m_context->video_stream];
again:
	if (m_context->notPacketized) {
		// We have to re-packetize ourselves. We do this by combining all data with the same timestamp.
		if (rpts == 0 || rpts == m_context->last_pts) {
			// Still the same packet. Combine the data and return.
			if (m_context->vbuffer == NULL) {
				// There is no old data. We may need to insert the extra header at the beginning.
				assert(m_context->vbufferlen == 0);
			}
			// See if we can steal the packet in stead of copying it
			if (m_context->vbuffer == NULL) {
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
			goto done;
		}
	}
	// Now we may have to wait until there is room in our output buffer.
	while (sink && sink->buffer_full() && !exit_requested()) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: waiting for buffer space for stream %d", m_context->video_stream);
			m_critical_section.leave();
			 // sleep 10 millisec, hardly noticeable
#ifdef	AMBULANT_PLATFORM_WIN32
			ambulant::lib::sleep_msec(10); // XXXX should be woken by readdone()
#else
			usleep(10000);
#endif //AMBULANT_PLATFORM_WIN32
			m_critical_section.enter();
			sink = m_context->sinks[m_context->video_stream];
	}

	// Send the data to our sink, which is responsible for copying/saving it before returning.
	if(sink && !exit_requested()) {
		if (m_context->notPacketized) {
			/*AM_DBG*/ lib::logger::get_logger()->debug("Video packet length (buffered)=%d, timestamp=%lld", m_context->vbufferlen, m_context->last_pts+m_clip_begin);
			sink->data_avail(m_context->last_pts+m_clip_begin, (uint8_t*) m_context->vbuffer, m_context->vbufferlen);
			free(m_context->vbuffer);
			m_context->vbuffer = NULL;
			m_context->vbufferlen = 0;
			m_context->last_pts=rpts;
			goto again;
		} else {
			/*AM_DBG*/ lib::logger::get_logger()->debug("Video packet length %d+%d=%d, timestamp=%lld", sz, m_context->extraPacketHeaderSize, sz+m_context->extraPacketHeaderSize, rpts+m_clip_begin);
			if (m_context->extraPacketHeaderSize) {
				// This magic was gleamed from mplayer, file demux_rtp.cpp and vlc, file live555.cpp.
				// The space in video_packet was already left free in run().
				memcpy(m_context->video_packet, m_context->extraPacketHeaderData, m_context->extraPacketHeaderSize);
			}
			sink->data_avail(rpts+m_clip_begin, (uint8_t*) m_context->video_packet, sz+m_context->extraPacketHeaderSize);
		}
	}
	
	m_context->last_pts=rpts;
	
done:
// Tell the main demux loop that we're ready for another packet.
	m_context->need_video = true;
	if (m_context->video_packet) free(m_context->video_packet);
	m_context->video_packet  = NULL;
	if(m_context->initialPacketDataLen > 0){
		free(m_context->initialPacketData);
		m_context->initialPacketData = NULL;
		m_context->initialPacketDataLen = 0;
	}

#ifdef JACK_IS_NOT_CONVINCED_YET
	// xxxbo In the case that m_context->time_left is a negative from the beginning for some reason,
	// Ambulant should render the video other than stop at the beginning.
	if (m_context->time_left >= 0 && m_context->last_pts >= m_context->time_left) {
		lib::logger::get_logger()->debug("after_reading_video: last_pts = %lld", m_context->last_pts);
	 	m_context->eof = true;
	}
#endif
	m_context->blocking_flag = ~0;
	//XXX Do we need to free data here ?
	m_critical_section.leave();
}

static int b64_decode( char *dest, char *src );

static unsigned char* parseH264ConfigStr( char const* configStr,
                                          unsigned int& configSize )
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
static int b64_decode( char *dest, char *src )
{
    const char *dest_start = dest;
    int  i_level;
    int  last = 0;
    int  b64[256] = {
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
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
        };

    for( i_level = 0; *src != '\0'; src++ )
    {
        int  c;

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
