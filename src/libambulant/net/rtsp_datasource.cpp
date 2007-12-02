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
#include "GroupsockHelper.hh"

///// Added by Bo Gao begin 2007-11-07
AVCodecParserContext * h264parserctx;

///// Added by Bo Gao end 2007-11-07

using namespace ambulant;
using namespace net;

//#define AM_DBG 
// turn on the AM_DBG will hang on ambulant when use rtp over tcp
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define MIN_VIDEO_PACKET_SIZE 1024

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

	m_context->audio_fmt.parameters = (void*) m_context->audio_codec_name;
	m_context->video_fmt.parameters = (void*) m_context->video_codec_name;
#if 0
	m_context->vbuffer = (unsigned char*)malloc(20000);
#else //xxxbo increase the vbuffer to 40000 for renderering mpg
	m_context->vbuffer = (unsigned char*)malloc(40000);
#endif
	m_context->vbufferlen = 0;
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::rtsp_demux(0x%x)", (void*) this);

#if 0
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

rtsp_context_t::~rtsp_context_t()
{
	AM_DBG  lib::logger::get_logger()->debug("ambulant::net::rtsp_context_t::~rtsp_context_t()");
	//Have to tear down session here, so that the server is not left hanging till timeout.
	rtsp_client->teardownMediaSession(*media_session);
	//deleting stuff, in reverse order as created in rtsp_demux::supported()
	if (vbuffer)
		free(vbuffer);
	for (int i=0; i < MAX_STREAMS; i++) {
		if (sinks[i] != NULL)
			delete sinks[i];
	}
	if (media_session) {
		Medium::close(media_session);
	}
	if (sdp)
		free(sdp);
	if (rtsp_client) {
		Medium::close(rtsp_client);
	}
	if (env)
		env->reclaim();
	if (scheduler)
		delete scheduler;
}

rtsp_context_t*
ambulant::net::rtsp_demux::supported(const net::url& url) 
{
	if (url.get_protocol() != "rtsp") return NULL;
	
	rtsp_context_t* context = new rtsp_context_t;
	context->first_sync_time.tv_sec = 0;
	context->first_sync_time.tv_usec = 0;
	context->scheduler = NULL;
	context->env = NULL;
	context->rtsp_client = NULL;
	context->media_session = NULL;
	context->sdp = NULL;
	context->audio_stream = -1;
	context->video_stream = -1;
	context->nstream = 0;
	context->blocking_flag = 0;
	context->audio_packet = NULL;
	context->video_packet = NULL;
	context->last_pts = 0;
	context->audio_codec_name = NULL;
	context->video_codec_name = NULL;
	context->audio_fmt = audio_format("live");
	context->video_fmt = video_format("live");
	context->eof = false;
	context->need_video = true;
	context->need_audio = true;
	context->nsinks = 0;
	context->vbuffer = NULL;
	context->vbufferlen = 0;

	context->gb_first_sync = 0;


	memset(context->sinks, 0, sizeof context->sinks);
	
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
	
	unsigned int desired_buf_size;
	MediaSubsession* subsession;
	MediaSubsessionIterator iter(*context->media_session);
	// Only audio/video session need to apply for a job !
	while ((subsession = iter.next()) != NULL) {
		if (!subsession->initiate()) {
			lib::logger::get_logger()->error("ambulant::net::rtsp_demux(net::url& url) failed to initiate subsession for medium \"%s\"", subsession->mediumName());
			//lib::logger::get_logger()->error("RTSP Connection Failed");
			delete context;
			return NULL;
		}
		if (strcmp(subsession->mediumName(), "audio") == 0) {
			desired_buf_size = 100000;
			if (context->audio_stream < 0) {
				context->audio_stream = context->nstream;
				context->audio_codec_name = subsession->codecName();
				AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), audio codecname :%s ",context->audio_codec_name);
				context->audio_fmt.channels = 0; //Let the decoder (ffmpeg) find out the channels subsession->numChannels() returns channels -1 ???
				context->audio_fmt.bits = 16;
				//context->audio_fmt.samplerate = subsession->rtpSource()->timestampFrequency();
				context->audio_fmt.samplerate = 0;
			}
		} else if (strcmp(subsession->mediumName(), "video") == 0) {
			desired_buf_size = 200000;
			if (context->video_stream < 0) {
				context->video_stream = context->nstream;
				context->video_codec_name = subsession->codecName();
				AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), video codecname :%s ",context->video_codec_name);
				//KB getting video_fmt from RTSP doesn't work 
				context->video_fmt.frameduration = (timestamp_t) (1000000.0/subsession->videoFPS());
				context->video_fmt.width = subsession->videoWidth();
				context->video_fmt.height = subsession->videoHeight();
				lib::logger::get_logger()->debug("ambulant::net::rtsp_demux(net::url& url), width: %d, height: %d, FPS: %f",context->video_fmt.width, context->video_fmt.height, 1000000.0/context->video_fmt.frameduration);

			}
		} else {
			AM_DBG lib::logger::get_logger()->debug("rtsp_demux: ignoring \"%s\" subsession", subsession->mediumName());
		}
		context->nstream++;
		
#if 0 // XXXJack: Corresponding code in live playCommon.cpp is prety different...
		int rtp_sock_num = subsession->rtpSource()->RTPgs()->socketNum();
		int buf_size = increaseReceiveBufferTo(*context->env, rtp_sock_num, desired_buf_size);
		(void)buf_size; // Forestall compiler warning
#endif
#ifndef WITH_TCP  //xxxBo setup RTP over udp
		if(!context->rtsp_client->setupMediaSubsession(*subsession, false, false)) {
#else //xxxBo setup RTP over tcp
		if(!context->rtsp_client->setupMediaSubsession(*subsession, false, 1)) {
#endif /*WITH_TCP*/
			lib::logger::get_logger()->error("ambulant::net::rtsp_demux(net::url& url) failed to send setup command to subsesion");
			//lib::logger::get_logger()->error("RTSP Connection Failed");
			delete context;
			return NULL;
		}
	}
	
	lib::logger::get_logger()->debug("rtps_demux::supported(%s): duration=%ld", ch_url, context->time_left);
	return context;
		
}

timestamp_t
ambulant::net::rtsp_demux::get_clip_begin()
{
	timestamp_t rv;
	m_critical_section.enter();
	rv = m_clip_begin;
	m_critical_section.leave();
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

unsigned long 
ambulant::net::rtsp_demux::run() 
{
	AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() called (%d)", m_context->need_audio);
	m_context->blocking_flag = 0;
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
	int firstTime=0;
		
	while(!m_context->eof && !exit_requested()) {
		AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run: start another loop iteration");
		m_context->blocking_flag = 0;
		if (m_seektime_changed) {
			// Note: we have not tested (yet) how seeking influences the timestamps returned by live555. Needs to be
			// tested later.
			float seektime_secs = (m_clip_begin+m_seektime)/1000000.0;
			/*AM_DBG*/ lib::logger::get_logger()->debug("rtsp_demux::run: seeking to %f", seektime_secs);
			if(!m_context->rtsp_client->pauseMediaSession(*m_context->media_session)) {
				lib::logger::get_logger()->error("pausing RTSP media session failed");
			}
			if(!m_context->rtsp_client->playMediaSession(*m_context->media_session, seektime_secs, -1.0, 1.0)) {
				lib::logger::get_logger()->error("resuming RTSP media session failed");
			}
			m_seektime_changed = false;
		}
		MediaSubsession* subsession;
		MediaSubsessionIterator iter(*m_context->media_session);
		// Only audio/video session need to apply for a job !
		while ((subsession = iter.next()) != NULL) {
			if (strcmp(subsession->mediumName(), "audio") == 0) {
				if(m_context->need_audio) {
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
					assert(!m_context->video_packet);
					m_context->configDataLen=0;//Required by after_reading_video
					if(firstTime==0){
						//For MP4V-ES video format we need to insert a header into the RTSP stream
					        //which should be present in the 'config' MIME parameter which should be present hopefully in the SDP description
						//this idea was copied from mplayer libmpdemux/demux_rtp.cpp
						firstTime=1;		
						if(strcmp(gettext(m_context->video_codec_name), "MP4V-ES")==0)
						//Optional check(therefore removed), since it should not matter for other formats.
						{
							AM_DBG lib::logger::get_logger()->debug("Came here good %s", m_context->video_codec_name);
							unsigned configLen;
		    				unsigned char* configData 
       							= parseGeneralConfigStr(subsession->fmtp_config(), configLen);
							m_context->configData = configData;
							m_context->configDataLen = configLen;
							
						}

						///// Added by Bo Gao begin 2007-10-29 
#ifdef WITH_H264
						if ( !strcmp( gettext(m_context->video_codec_name), "H264")){
						    unsigned configLen;
						    unsigned char* configData;
						    
						    configData = parseH264ConfigStr(subsession->fmtp_spropparametersets(), configLen);
 						    m_context->configData = configData;
						    m_context->configDataLen = configLen;
						    
						}
#endif
							///// Added by Bo Gao end 2007-10-29

					}		
					m_context->video_packet = (unsigned char*) malloc(MAX_RTP_FRAME_SIZE);
					//std::cout << " MAX_RTP_FRAME_SIZE = " << MAX_RTP_FRAME_SIZE;
					AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() Calling getNextFrame for an video frame");
					m_context->need_video = false;
					AM_DBG lib::logger::get_logger()->debug("ambulant::net::rtsp_demux::run() video_packet 0x%x", m_context->video_packet);
					m_critical_section.leave();
					subsession->readSource()->getNextFrame(m_context->video_packet, MAX_RTP_FRAME_SIZE, after_reading_video_stub, this, on_source_close, m_context);
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
	AM_DBG lib::logger::get_logger()->debug("after_reading_video: called sz = %d, truncated = %d pts=(%d s, %d us), dur=%d", sz, truncated, pts.tv_sec, pts.tv_usec, duration);
	assert(m_context->video_packet);
	assert(m_context->video_stream >= 0);
	
	// For the first packet, we remember the timestamp so we can convert Live's wallclock timestamps to
	// our zero-based timestamps.
	if (m_context->first_sync_time.tv_sec == 0 && m_context->first_sync_time.tv_usec == 0 ) {
		m_context->first_sync_time.tv_sec = pts.tv_sec;
		m_context->first_sync_time.tv_usec = pts.tv_usec;
		m_context->last_pts=0;
		if(m_context->configDataLen > 0)//Required by MP4V-ES. Only required for the first packet.
			m_context->sinks[m_context->video_stream]->data_avail(0, (uint8_t*) m_context->configData , m_context->configDataLen);
		
		//memcpy(m_context->vbuffer, m_context->video_packet, sz);
		//m_context->vbufferlen=sz;
	}

		///// Added by Bo Gao begin 2007-11-19
	//if (strcmp(m_context->video_codec_name, "H264") == 0){
		MediaSubsession* subsession;
		MediaSubsessionIterator iter(*m_context->media_session);
		while ((subsession = iter.next()) != NULL) {
			if (strcmp(subsession->mediumName(), "video") == 0) {
				// Set the packet's presentation time stamp, depending on whether or
				// not our RTP source's timestamps have been synchronized yet: 
				// This idea is borrowed from mplayer at demux_rtp.cpp::after_reading
				Boolean hasBeenSynchronized = subsession->rtpSource()->hasBeenSynchronizedUsingRTCP();
				if (hasBeenSynchronized && m_context->gb_first_sync == 0) {
					/*AM_DBG*/ lib::logger::get_logger()->debug("after_reading_video: resync video, from %ds %dus to %ds %dus", m_context->first_sync_time.tv_sec, m_context->first_sync_time.tv_usec, pts.tv_sec, pts.tv_usec);
					m_context->first_sync_time.tv_sec = pts.tv_sec;
					m_context->first_sync_time.tv_usec = pts.tv_usec;
					m_context->gb_first_sync = 1;
				}
			}
		}
	//}
		///// Added by Bo Gao begin 2007-11-19

	timestamp_t rpts =  (timestamp_t)(pts.tv_sec - m_context->first_sync_time.tv_sec) * 1000000LL  +  (timestamp_t) (pts.tv_usec - m_context->first_sync_time.tv_usec);
	// XXXJACK: Some code downstream expects timestamps to be file-based, i.e. when playing with clipBegin=10s it expects the first frame to have
	// timestamp=10s. Not sure this is correct, but for now we fix up the timestamp. I'm also not sure how this interacts with seeking.
	// Please remove this comment once it has been checked that the current behaviour is correct.
	rpts += m_clip_begin;
	/*AM_DBG*/ lib::logger::get_logger()->debug("after_reading_video: called timestamp 0x%08.8x%08.8x, sec = %d, usec =  %d", (long)(rpts>>32), (long)(rpts&0xffffffff), pts.tv_sec, pts.tv_usec);
	
	
	//Frame alignment for Mpeg1 or 2 frames, Live doesn't do it.
	//If the frame is bigger than 20kb display the rest next time
	//TODO display what I have currently as well : the impartial frame.
	 if (rpts == m_context->last_pts) {
#if 0
		 if((sz + m_context->vbufferlen)>20000)
#else //xxxbo increase the vbuffer to 40000 for renderering mpg
		 if((sz + m_context->vbufferlen)>40000)
#endif
		 {
			 lib::logger::get_logger()->trace("Frame too large to display");
			 m_context->vbufferlen=0;
		 }else{
#ifdef WITH_H264
			 if (strcmp(m_context->video_codec_name,"H264") == 0){
				 unsigned char* newgbcontext = (unsigned char*)malloc((sz+4)*sizeof(unsigned char));
				 if (newgbcontext == NULL) {
					 lib::logger::get_logger()->debug("after_reading_video: malloc(%d) failed", sz+4);
					 lib::logger::get_logger()->error("Out of memory after_reading_video");
					 m_context->eof = true;
				 }
				 newgbcontext[0] = 0x00;
				 newgbcontext[1] = 0x00;
				 newgbcontext[2] = 0x00;
				 newgbcontext[3] = 0x01;
				 memcpy(&newgbcontext[4],m_context->video_packet, sz);
				 sz += 4;
				 //m_context->vbuffer = (unsigned char*)realloc(m_context->vbuffer, m_context->vbufferlen+sz);
				 memcpy((m_context->vbuffer + m_context->vbufferlen), newgbcontext, sz);
				 free(newgbcontext);

			 }
			 else
#endif
				 memcpy((m_context->vbuffer + m_context->vbufferlen), m_context->video_packet, sz);
			 m_context->vbufferlen += sz;
		 }
	 }else{		 

		demux_datasink *sink = m_context->sinks[m_context->video_stream];
		// This is not good: because after_reading_video is not a member of rtsp_demux we cannot access exit_requested() to
		// check whether we are terminating (see code in ffmpeg_demux for an example). In stead we have to rely on our
		// sink being freed and zeroed.
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
			AM_DBG lib::logger::get_logger()->debug("Video packet length %d", m_context->vbufferlen);
			assert(m_context->vbufferlen);
			sink->data_avail(m_context->last_pts, (uint8_t*) m_context->vbuffer , m_context->vbufferlen);
		}
		
		m_context->last_pts=rpts;
		//copy the first packet of the next frame

		///// Added by Bo Gao begin 2007-11-01
#ifdef WITH_H264	  
		if (strcmp(m_context->video_codec_name,"H264") == 0){
			// printf("the m_context->video_codec_name is %s\n", m_context->video_codec_name);
			unsigned char* newgbcontext = (unsigned char*)malloc((sz+4)*sizeof(unsigned char));
			if (newgbcontext == NULL) {
				lib::logger::get_logger()->debug("after_reading_video: malloc(%d) failed", sz+4);
				lib::logger::get_logger()->error("Out of memory after_reading_video");
				m_context->eof = true;
			}
			newgbcontext[0] = 0x00;
			newgbcontext[1] = 0x00;
			newgbcontext[2] = 0x00;
			newgbcontext[3] = 0x01;
			memcpy(&newgbcontext[4],m_context->video_packet, sz);
			m_context->video_packet = (unsigned char*)realloc(m_context->video_packet, sz+4);
			memcpy(m_context->video_packet, newgbcontext, sz+4);
			free(newgbcontext);

			sz += 4;

		}
#endif
	  ///// Added by Bo Gao end 2007-11-01

		memcpy(m_context->vbuffer, m_context->video_packet, sz);
		m_context->vbufferlen=sz;
	} 
	 

		 // Tell the main demux loop that we're ready for another packet.
	m_context->need_video = true;
	free(m_context->video_packet);
	if(m_context->configDataLen > 0){
		free(m_context->configData);
		m_context->configData = NULL;
	}
	m_context->video_packet  = NULL;

	// xxxbo In the case that m_context->time_left is a negative from the beginning for some reason,
	// Ambulant should render the video other than stop at the beginning.
	if (m_context->time_left >= 0 && m_context->last_pts >= m_context->time_left) {
		lib::logger::get_logger()->debug("after_reading_video: last_pts = %lld", m_context->last_pts);
	 	m_context->eof = true;
	}
	m_context->blocking_flag = ~0;
	//XXX Do we need to free data here ?
	m_critical_section.leave();
}

#ifdef WITH_H264
///// Added by Bo Gao begin 2007-10-29 
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
	psz = dup = (char *)StrDup(LPCTSTR (configStr));
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

///// Added by Bo Gao end 2007-10-29
#endif
