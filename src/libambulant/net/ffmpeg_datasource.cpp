/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */
 
#include <math.h>
#include <map>

#include "ambulant/net/ffmpeg_datasource.h" 
#include "ambulant/net/datasource.h"
#include "ambulant/lib/logger.h"
#include "ambulant/net/url.h"



//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif 

#if FFMPEG_VERSION_INT == 0x000409
	#define WITH_FFMPEG_0_4_9					
#endif

// How many video frames we would like to buffer at most
#define MAX_VIDEO_FRAMES 300

// Bug workaround: define RESAMPLE_READ_ALL to let the resampler
// collect all data before calling the client callback
//#define RESAMPLE_READ_ALL
using namespace ambulant;
using namespace net;

typedef lib::no_arg_callback<ffmpeg_decoder_datasource> readdone_callback;
typedef lib::no_arg_callback<ffmpeg_video_decoder_datasource> framedone_callback;
typedef lib::no_arg_callback<ffmpeg_resample_datasource> resample_callback;

#define INBUF_SIZE 4096



// Static initializer function shared among ffmpeg classes
ffmpeg_codec_id* ambulant::net::ffmpeg_codec_id::m_uniqueinstance = NULL;

ffmpeg_codec_id*
ffmpeg_codec_id::instance()
{
	if (m_uniqueinstance == NULL) {
        m_uniqueinstance = new ffmpeg_codec_id();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_codec_id::instance() returning 0x%x", (void*) m_uniqueinstance);
	}
    return m_uniqueinstance;
}

void
ffmpeg_codec_id::add_codec(const char* codec_name, CodecID id)
{
  std::string str(codec_name);
  m_codec_id.insert(std::pair<std::string, CodecID>(str, id));
}

CodecID
ffmpeg_codec_id::get_codec_id(const char* codec_name) 
{
	std::string str(codec_name);
	std::map<std::string, CodecID>::iterator i;
	i = m_codec_id.find(str);
	if (i != m_codec_id.end()) {
		return i->second;
	}
	
	return CODEC_ID_NONE;
}

ffmpeg_codec_id::ffmpeg_codec_id()
{
	add_codec("MPA", CODEC_ID_MP3);
	add_codec("MPA-ROBUST", CODEC_ID_MP3);
	add_codec("X_MP3-DRAFT-00", CODEC_ID_MP3);
	add_codec("MPV", CODEC_ID_MPEG2VIDEO);
}


// Hack, hack. Get extension of a URL.
static const char *
getext(const net::url &url)
{
	const char *curl = url.get_file().c_str();
	const char *dotpos = rindex(curl, '.');
	if (dotpos) return dotpos+1;
	return NULL;
}

video_datasource* 
ffmpeg_video_datasource_factory::new_video_datasource(const net::url& url)
{
#ifdef WITH_FFMPEG_AVFORMAT
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource(%s)", repr(url).c_str());
	AVFormatContext *context = detail::ffmpeg_demux::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: no support for %s", repr(url).c_str());
		return NULL;
	}
	detail::ffmpeg_demux *thread = new detail::ffmpeg_demux(context);
	if (thread->video_stream_nr() < 0) {
		thread->cancel();
		lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory: No video stream in %s", repr(url).c_str());
		return NULL;
	}
	video_datasource *ds = demux_video_datasource::new_demux_video_datasource(url, thread);
	video_datasource *dds = NULL;
	
	thread->start();
	if (ds) {
		 video_format fmt = thread->get_video_format();
		 //dds = ds;
		 dds = new ffmpeg_video_decoder_datasource(ds, fmt);
	} else {
		return NULL;
	}
	
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource (ds = 0x%x)", (void*) ds);

	if ((dds == NULL)  || (ds == NULL)) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}
	return dds;		
#else
	return NULL;	
#endif // WITH_FFMPEG_AVFORMAT
}


audio_datasource* 
ffmpeg_audio_datasource_factory::new_audio_datasource(const net::url& url, audio_format_choices fmts)
{
#ifdef WITH_FFMPEG_AVFORMAT
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource(%s)", repr(url).c_str());
	AVFormatContext *context = detail::ffmpeg_demux::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: no support for %s", repr(url).c_str());
		return NULL;
	}
	detail::ffmpeg_demux *thread = new detail::ffmpeg_demux(context);
	audio_datasource *ds = demux_audio_datasource::new_demux_audio_datasource(url, thread);
	if (ds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("fdemux_audio_datasource_factory::new_audio_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}
	thread->start();
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: parser ds = 0x%x", (void*)ds);
	// XXXX This code should become generalized in datasource_factory
	if (fmts.contains(ds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return ds;
	}
	audio_datasource *dds = new ffmpeg_decoder_datasource(ds);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: decoder ds = 0x%x", (void*)dds);
	if (dds == NULL) {
		int rem = ds->release();
		assert(rem == 0);
		return NULL;
	}
	if (fmts.contains(dds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return dds;
	}
	audio_datasource *rds = new ffmpeg_resample_datasource(dds, fmts);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: resample ds = 0x%x", (void*)rds);
	if (rds == NULL)  {
		int rem = dds->release();
		assert(rem == 0);
		return NULL;
	}
	if (fmts.contains(rds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return rds;
	}
	lib::logger::get_logger()->error(gettext("%s: unable to create audio resampler"));
	int rem = rds->release();
	assert(rem == 0);
#endif // WITH_FFMPEG_AVFORMAT
	return NULL;	
}

audio_datasource* 
ffmpeg_audio_parser_finder::new_audio_parser(const net::url& url, audio_format_choices fmts, datasource *src)
{
	
	audio_datasource *ds = NULL;
	if (src) {
			// XXXX Here we have to check for the mime type.
			if (!ffmpeg_decoder_datasource::supported(url)) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_parser_finder::new_audio_parser: no support for %s", repr(url).c_str());
				return NULL;
			}
			ds = new ffmpeg_decoder_datasource(url, src);
			// XXX Here we have to check if ext & fmt are supported by ffmpeg
			if (ds) return ds;			
		}
	return NULL;	
}

audio_datasource*
ffmpeg_audio_filter_finder::new_audio_filter(audio_datasource *src, audio_format_choices fmts)
{
	audio_format& fmt = src->get_audio_format();
	// First check that we understand the source format
	if (fmt.bits != 16) {
		lib::logger::get_logger()->warn(gettext("No support for %d-bit audio, only 16"), fmt.bits);
		return NULL;
	}
	if (fmts.contains(fmt)) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_filter_finder::new_audio_datasource: matches!");
		return src;
	}
	// XXXX Check that there is at least one destination format we understand too
	return new ffmpeg_resample_datasource(src, fmts);
}

// **************************** ffmpeg_demux *****************************

#ifdef WITH_FFMPEG_AVFORMAT
#define CLIPBEGIN 3
detail::ffmpeg_demux::ffmpeg_demux(AVFormatContext *con)
:   m_con(con),
	m_nstream(0),
	m_clip_begin(0),
	m_clip_begin_set(false)
{
#if WITH_FFMPEG_0_4_9
	if (CLIPBEGIN > 0) {
		assert (m_con);
		assert (m_con->iformat);
		std::cout << "read_seek" << "\n";
		int seek = av_seek_frame(m_con, -1, CLIPBEGIN*1000000, 0);
	} 
#endif
	
	m_audio_fmt = audio_format("ffmpeg");
	m_audio_fmt.bits = 16;
	int audio_idx = audio_stream_nr();
	if ( audio_idx >= 0) {
		m_audio_fmt.parameters = (void *)&con->streams[audio_idx]->codec;
		m_audio_fmt.samplerate = con->streams[audio_idx]->codec.sample_rate;
		m_audio_fmt.channels = con->streams[audio_idx]->codec.channels;
		m_audio_fmt.bits = 16;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::ffmpeg_demux(): samplerate=%d, channels=%d, m_con->codec (0x%x)", m_audio_fmt.samplerate, m_audio_fmt.channels, m_audio_fmt.parameters  );
	} 
	m_video_fmt = video_format("ffmpeg");
	int video_idx = video_stream_nr();
	if (video_idx >= 0) {
		m_video_fmt.parameters = (void *)&con->streams[video_stream_nr()]->codec;
	} else {
		m_video_fmt.parameters = NULL;
	}
	m_video_fmt.framerate = 0;
	m_video_fmt.width = 0;
	m_video_fmt.height = 0;
	memset(m_sinks, 0, sizeof m_sinks);
}

detail::ffmpeg_demux::~ffmpeg_demux()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::~ffmpeg_demux()");
	if (m_con) av_close_input_file(m_con);
	m_con = NULL;
}

AVFormatContext *
detail::ffmpeg_demux::supported(const net::url& url)
{
	ffmpeg_init();
	// Setup struct to allow ffmpeg to determine whether it supports this
	AVInputFormat *fmt;
	AVProbeData probe_data;
	std::string url_str(url.get_url());
	
	probe_data.filename = url_str.c_str();
	probe_data.buf = NULL;
	probe_data.buf_size = 0;
	fmt = av_probe_input_format(&probe_data, 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::supported(%s): av_probe_input_format: 0x%x", url_str.c_str(), (void*)fmt);
	AVFormatContext *ic = NULL;
	int err = av_open_input_file(&ic, url_str.c_str(), fmt, 0, 0);
	if (err) {
		lib::logger::get_logger()->trace("ffmpeg_demux::supported(%s): av_open_input_file returned error %d, ic=0x%x", url_str.c_str(), err, (void*)ic);
		if (ic) av_close_input_file(ic);
		return NULL;
	}
	//err = av_read_play(ic);
 	err = av_find_stream_info(ic);
	if (err < 0) {
		lib::logger::get_logger()->trace("ffmpeg_demux::supported(%s): av_find_stream_info returned error %d, ic=0x%x", url_str.c_str(), err, (void*)ic);
		if (ic) av_close_input_file(ic);
		return NULL;
	}
	AM_DBG dump_format(ic, 0, url_str.c_str(), 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::supported: rate=%d, channels=%d", ic->streams[0]->codec.sample_rate, ic->streams[0]->codec.channels);
	return ic;
}

void
detail::ffmpeg_demux::cancel()
{
	if (is_running())
		stop();
	release();
}

int 
detail::ffmpeg_demux::audio_stream_nr() 
{
	int stream_index;
	for (stream_index=0; stream_index < m_con->nb_streams; stream_index++) {
		if (m_con->streams[stream_index]->codec.codec_type == CODEC_TYPE_AUDIO)
			return stream_index;
	}
	
	return -1;
}

int 
detail::ffmpeg_demux::video_stream_nr() 
{
	int stream_index;
	for (stream_index=0; stream_index < m_con->nb_streams; stream_index++) {
		if (m_con->streams[stream_index]->codec.codec_type == CODEC_TYPE_VIDEO)
			return stream_index;
	}
	
	return -1;
}

double
detail::ffmpeg_demux::duration()
{
	//XXX this is a double now, later this should retrun a long long int 
 	return (m_con->duration / (double)AV_TIME_BASE);
}
	
int 
detail::ffmpeg_demux::nstreams()
{
	return m_con->nb_streams;
}

video_format& 
detail::ffmpeg_demux::get_video_format() 
{ 
	
	if (m_video_fmt.width == 0) m_video_fmt.width = m_con->streams[video_stream_nr()]->codec.width;
	if (m_video_fmt.height == 0) m_video_fmt.height = m_con->streams[video_stream_nr()]->codec.height;
	return m_video_fmt; 
}

void 
detail::ffmpeg_demux::add_datasink(detail::datasink *parent, int stream_index)
{
	m_lock.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_sinks[stream_index] == 0);
	m_sinks[stream_index] = parent;
	m_nstream++;
	m_lock.leave();
}

void
detail::ffmpeg_demux::seek(timestamp_t time)
{
	m_clip_begin = time;
	m_clip_begin_set = false;
}

void
detail::ffmpeg_demux::remove_datasink(int stream_index)
{
	m_lock.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_sinks[stream_index] != 0);
	m_sinks[stream_index] = 0;
	m_nstream--;
	m_lock.leave();
	if (m_nstream <= 0) cancel();
}

unsigned long
detail::ffmpeg_demux::run()
{
	m_lock.enter();
	int pkt_nr;
	timestamp_t pts;
	pkt_nr = 0;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: started");
	while (!exit_requested()) {
		AVPacket pkt1, *pkt = &pkt1;
		
		pkt->pts = 0;
		// Read a packet
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run:  started");
		m_lock.leave();
		int ret = av_read_packet(m_con, pkt);
		m_lock.enter();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: av_read_packet returned ret= %d, (%lld, 0x%x, %d)", ret, pkt->pts ,pkt->data, pkt->size);
		if (ret < 0) break;
		pkt_nr++;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: av_read_packet number : %d",pkt_nr);
		// Find out where to send it to
		assert(pkt->stream_index >= 0 && pkt->stream_index < MAX_STREAMS);
		detail::datasink *sink = m_sinks[pkt->stream_index];
		if (sink == NULL) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: Drop data for stream %d (%lld, 0x%x, %d)", pkt->stream_index, pkt->pts ,pkt->data, pkt->size);
		} else {
			AM_DBG lib::logger::get_logger ()->debug ("ffmpeg_parser::run sending data to datasink (stream %d) (%lld, 0x%x, %d)",pkt->stream_index, pkt->pts ,pkt->data, pkt->size);
			// Wait until there is room in the buffer
			//while (sink->buffer_full() && !exit_requested()) {
			while (sink && sink->buffer_full() && !exit_requested()) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: waiting for buffer space");
				m_lock.leave();
				sleep(1);   // This is overdoing it
				m_lock.enter();
				sink = m_sinks[pkt->stream_index];
			}
			if (sink && !exit_requested()) {
				
#ifdef	WITH_FFMPEG_0_4_9					
				int num = 0;
				int den = 0;
#else /*WITH_FFMPEG_0_4_9*/
				int num = m_con->pts_num;
				int den = m_con->pts_den;
#endif/*WITH_FFMPEG_0_4_9*/
				pts = 0;
				if (pkt->pts != AV_NOPTS_VALUE) {
#ifdef	WITH_FFMPEG_0_4_9				
					pts = pkt->pts;							
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: ffmpeg 0.4.9 pts = 0x%llx",pts);
#else /*WITH_FFMPEG_0_4_9*/							
					pts = (timestamp_t) round(((double) pkt->pts * (((double) num)*1000000)/den));
#endif/*WITH_FFMPEG_0_4_9*/
				}
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: calling %d.data_avail(%lld, 0x%x, %d)", pkt->stream_index, pkt->pts, pkt->data, pkt->size);
#ifdef WITH_FFMPEG_0_4_9				
				sink->data_avail(pts, pkt->data, pkt->size);
#else
				if(pts > m_clip_begin) {				
					sink->data_avail(pts, pkt->data, pkt->size);
				}
#endif
			}
		}
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: freeing pkt (number %d)",pkt_nr);
//		av_free_packet(pkt);
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: final data_avail(0, 0)");
	int i;
	for (i=0; i<MAX_STREAMS; i++)
		if (m_sinks[i])
			m_sinks[i]->data_avail(0, 0, 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: returning");
	m_lock.leave();
	return 0;
}


// **************************** demux_audio_datasource ******************************

demux_audio_datasource *
demux_audio_datasource::new_demux_audio_datasource(
  		const net::url& url, 
		detail::abstract_demux *thread)
{
	
	int stream_index;
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource::new_ffmpeg_audio_datasource()");
	// Find the index of the audio stream
	stream_index = thread->audio_stream_nr();
	assert(stream_index >= 0);
	
	if (stream_index >= thread->nstreams()) {
		lib::logger::get_logger()->error(gettext("%s: no more audio streams"), url.get_url().c_str());
		return NULL;
	} 

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource::new_ffmpeg_audio_datasource() looking for the right codec");
	
	
	return new demux_audio_datasource(url, thread, stream_index);
}

demux_audio_datasource::demux_audio_datasource(const net::url& url, detail::abstract_demux *thread, int stream_index)
:	m_url(url),
	m_stream_index(stream_index),
//	m_fmt(audio_format("ffmpeg")),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(thread),
	m_client_callback(NULL),
	m_thread_started(false)
{	
	//AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource::ffmpeg_audio_datasource: rate=%d, channels=%d", context->streams[m_stream_index]->codec.sample_rate, context->streams[m_stream_index]->codec.channels);
	// XXX ignoring the codec for now but i'll have to look into this real soon
	//m_fmt.parameters = (void *)&context->streams[m_stream_index]->codec;
	m_thread->add_datasink(this, stream_index);
}

demux_audio_datasource::~demux_audio_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource::~ffmpeg_audio_datasource(0x%x)", (void*)this);
	stop();
}

void
demux_audio_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource::stop(0x%x)", (void*)this);
	if (m_thread) {
		detail::abstract_demux *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->remove_datasink(m_stream_index);
		m_lock.enter();
	}
	m_thread = NULL;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource::stop: thread stopped");
	//if (m_con) delete m_con;
	//m_con = NULL; // owned by the thread
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
}	

void 
demux_audio_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	if (!m_thread_started) {
		m_thread->start();
	}
	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource::start(): m_client_callback already set!");
	}
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_audio_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
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
demux_audio_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource.readdone : done with %d bytes", len);
//	restart_input();
	m_lock.leave();
}

void 
demux_audio_datasource::seek(timestamp_t time)
{
	assert(m_thread_started); // the thread should be running before we can seek it
	assert(m_thread);
	m_thread->seek(time);
}

void 
demux_audio_datasource::read_ahead(timestamp_t time)
{
	assert(!m_thread_started);
	assert(m_thread);
	
	m_thread->seek(time);
	m_thread->start();
	m_thread_started = true;
}


void 
demux_audio_datasource::data_avail(timestamp_t pts, uint8_t *inbuf, int sz)
{
	// XXX timestamp is ignored, for now
	m_lock.enter();
	m_src_end_of_file = (sz == 0);
	AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource.data_avail: %d bytes available (ts = %lld)", sz, pts);
	if(sz && !m_buffer.buffer_full()){
		uint8_t *outbuf = (uint8_t*) m_buffer.get_write_ptr(sz);
		if (outbuf) {
			memcpy(outbuf, inbuf, sz);
			m_buffer.pushdata(sz);
			// XXX m_src->readdone(sz);
		} else {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_audio_datasource::data_avail: no room in output buffer");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
			m_buffer.pushdata(0);
		}
	}

	if ( m_client_callback && (m_buffer.buffer_not_empty() || m_src_end_of_file ) ) {
		AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), m_src_end_of_file);
		assert(m_event_processor);
		m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::med);
		m_client_callback = NULL;
		//m_event_processor = NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::data_avail(): No client callback!");
	}
	m_lock.leave();
}


bool 
demux_audio_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
demux_audio_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	return m_src_end_of_file;
}

bool 
demux_audio_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}	

char* 
demux_audio_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

int 
demux_audio_datasource::size() const
{
	const_cast <demux_audio_datasource*>(this)->m_lock.enter();
	int rv = m_buffer.size();
	const_cast <demux_audio_datasource*>(this)->m_lock.leave();
	return rv;
}	


audio_format&
demux_audio_datasource::get_audio_format()
{

	return m_thread->get_audio_format();
}

std::pair<bool, double>
demux_audio_datasource::get_dur()
{
	std::pair<bool, double> rv(false, 0.0);
	m_lock.enter();
	if (m_thread->duration() >= 0) {
		rv = std::pair<bool, double>(true, m_thread->duration());
		AM_DBG lib::logger::get_logger()->debug("demux_audio_datasource::get_dur: duration=%f", rv.second);
	}
	m_lock.leave();
	return rv;
}


		
// **************************** demux_video_datasource *****************************

demux_video_datasource *
demux_video_datasource::new_demux_video_datasource(
  		const net::url& url, 
		detail::abstract_demux *thread)
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

demux_video_datasource::demux_video_datasource(const net::url& url, detail::abstract_demux *thread, int stream_index)
:	m_url(url),
	m_stream_index(stream_index),
//	m_fmt(audio_format(0,0,0)),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(thread),
	m_client_callback(NULL),
	m_audio_src(NULL),
	m_thread_started(false)
{	
	m_thread->add_datasink(this, stream_index);
	int audio_stream_idx = m_thread->audio_stream_nr();
	if (audio_stream_idx >= 0) 
		m_audio_src = new demux_audio_datasource(m_url, m_thread, audio_stream_idx);
	
}

demux_video_datasource::~demux_video_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::~demux_video_datasource(0x%x)", (void*)this);
	stop();
}

void
demux_video_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::stop(0x%x)", (void*)this);
	if (m_thread) {
		detail::abstract_demux *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->remove_datasink(m_stream_index);
		m_lock.enter();
	}
	m_thread = NULL;
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::stop: thread stopped");
	//if (m_con) delete m_con;
	//m_con = NULL; // owned by the thread
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
	
}	



void 
demux_video_datasource::read_ahead(timestamp_t time)
{
	assert(!m_thread_started);
	assert(m_thread);
	
	m_thread->seek(time);
	m_thread->start();
	m_thread_started = true;
}

void 
demux_video_datasource::start_frame(ambulant::lib::event_processor *evp, 
	ambulant::lib::event *callbackk, timestamp_t timestamp)
{
	if(!m_thread_started) {
		m_thread->start();
		m_thread_started = true;
	}
	
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::start_frame: (this = 0x%x), callback = 0x%x", (void*) this, (void*) callbackk);

	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::start(): m_client_callback already set!");
	}
	if (m_frames.size() > 0 /* XXXX Check timestamp! */ || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->debug("Internal error: demux_video_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
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
demux_video_datasource::frame_done(timestamp_t pts, bool keepdata)
{
	// Note: we ignore pts and always discard a single frame.
	m_lock.enter();
	if (m_frames.size() == 0) {
		lib::logger::get_logger()->debug("Internal error: demux_video_datasource.readdone: frame_done() called with no current frames");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		m_lock.leave();
		return;
	}
	
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource.frame_done(%d)", pts);
	std::pair<timestamp_t, video_frame> element = m_frames.front();

	if (m_old_frame.second.data) {
		free(m_old_frame.second.data);
		AM_DBG  lib::logger::get_logger()->debug("demux_video_datasource::frame_done: free(0x%x)", m_old_frame.second.data);
		m_old_frame.second.data = NULL;
	}
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::frame_done(%d): removing frame with ts=%d", pts, element.first);
	AM_DBG  lib::logger::get_logger()->debug("demux_video_datasource::frame_done: %lld 0x%x %d", element.first, element.second.data, element.second.size);
	m_old_frame = element;
	m_frames.pop();
	if (!keepdata) {
		free(m_old_frame.second.data);
		m_old_frame.second.data = NULL;
	}
	m_lock.leave();
}


void 
demux_video_datasource::data_avail(timestamp_t pts, uint8_t *inbuf, int sz)
{
	m_lock.enter();

	m_src_end_of_file = (sz == 0);
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::data_avail(): recieving data sz=%d ,eof=%d", sz, m_src_end_of_file);
	if(sz > 0) {
		char* frame_data = (char*) malloc(sz+1);
		assert(frame_data);
		memcpy(frame_data, inbuf, sz);
		video_frame vframe;
		vframe.data = frame_data;
		vframe.size = sz;
		m_frames.push(std::pair<timestamp_t, video_frame>(pts,vframe));
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::data_avail(): %lld 0x%x %d", pts, vframe.data, vframe.size);
	}		
	if ( m_frames.size() || m_src_end_of_file  ) {
		if ( m_client_callback ) {
			AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::data_avail(): calling client callback (eof=%d)", m_src_end_of_file);
			assert(m_event_processor);
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::med);
			m_client_callback = NULL;
			//m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::data_avail(): No client callback! (callback = 0x%x)",(void*) m_client_callback);
		}
	}		
	m_lock.leave();
}


bool 
demux_video_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
demux_video_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_frames.size()) {
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::_end_of_file() returning false (still %d frames in local buffer)",m_frames.size());
		return false;
	}
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::_end_of_file() no frames in buffer returning %d", m_src_end_of_file);
	return m_src_end_of_file;
}

bool 
demux_video_datasource::buffer_full()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::buffer_full() (this=0x%x, count=%d)", (void*) this, m_frames.size(), MAX_VIDEO_FRAMES);
	bool rv = (m_frames.size() > MAX_VIDEO_FRAMES);
	m_lock.leave();
	return rv;
}	

char*
demux_video_datasource::get_frame(timestamp_t now,timestamp_t *timestamp, int *size)
{
	
	// We ignore now here and always return a the oldest frame in the queue.
	m_lock.enter();
	if (m_frames.size() == 0) {
		m_lock.leave();
		*timestamp = 0;
		*size = 0;
		return NULL;
	}
	AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_frame() (this=0x%x, size=%d", (void*) this, m_frames.size());
	std::pair<timestamp_t, video_frame> frame;
	frame = m_frames.front();
	AM_DBG  lib::logger::get_logger()->debug("demux_video_datasource::get_frame(): %lld 0x%x %d", frame.first, frame.second.data, frame.second.size);
	char *rv = (char*) frame.second.data;
	*size = frame.second.size;
	*timestamp = frame.first;
	m_lock.leave();
	return rv;
}

int 
demux_video_datasource::size() const
{
	const_cast <demux_video_datasource*>(this)->m_lock.enter();
	int rv = m_frames.size();
	const_cast <demux_video_datasource*>(this)->m_lock.leave();
	return rv;
}

video_format&
demux_video_datasource::get_video_format()
{

	return m_thread->get_video_format();
}

int
demux_video_datasource::width()
{
	video_format fmt = m_thread->get_video_format();
	return fmt.width;
}

int
demux_video_datasource::height()
{
	video_format fmt = m_thread->get_video_format();
	return fmt.height;
}

bool 
demux_video_datasource::has_audio()
{
	
	if (m_thread->audio_stream_nr() >= 0)
		return true;
	else
		return false;
}


audio_datasource*
demux_video_datasource::get_audio_datasource()
{

	if (m_audio_src) {
		audio_datasource *dds = new ffmpeg_decoder_datasource(m_audio_src);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: decoder ds = 0x%x", (void*)dds);
		if (dds == NULL) {
			int rem = m_audio_src->release();
			assert(rem == 0);
			return NULL;
		}
		return dds;
	}
	return NULL;
}

std::pair<bool, double>
demux_video_datasource::get_dur()
{
	std::pair<bool, double> rv(false, 0.0);
	m_lock.enter();
	if (m_thread->duration() >= 0) {
		rv = std::pair<bool, double>(true, m_thread->duration());
		AM_DBG lib::logger::get_logger()->debug("demux_video_datasource::get_dur: duration=%f", rv.second);
	}
	m_lock.leave();
	return rv;
}


// **************************** ffmpeg_video_decoder_datasource ********************

ffmpeg_video_decoder_datasource::ffmpeg_video_decoder_datasource(video_datasource* src, video_format fmt)
:	m_src(src),
	m_src_end_of_file(false),
	m_thread(NULL),
	m_event_processor(NULL),
	m_client_callback(NULL),
	m_pts_last_frame(0),
	m_last_p_pts(0),
	m_frame_count(0)
{	
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::ffmpeg_video_decoder_datasource() (this = 0x%x)", (void*)this);
	
	ffmpeg_init();
	//m_duration = m_src->get_dur();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource: Looking for %s(0x%x) decoder", fmt.name.c_str(), fmt.parameters);
	if (!_select_decoder(fmt))
		lib::logger::get_logger()->error(gettext("ffmpeg_video_decoder_datasource: could not select %s(0x%x) decoder"), fmt.name.c_str(), fmt.parameters);
	m_fmt = fmt;
	m_old_frame.first = 0;
	m_old_frame.second = NULL;
}

ffmpeg_video_decoder_datasource::~ffmpeg_video_decoder_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::~ffmpeg_video_decoder_datasource(0x%x)", (void*)this);
	stop();
}

void
ffmpeg_video_decoder_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop(0x%x)", (void*)this);
	if (m_thread) {
		detail::ffmpeg_demux *tmpthread = m_thread;
		m_thread = NULL;
		m_lock.leave();
		tmpthread->remove_datasink(m_stream_index);
		m_lock.enter();
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop: thread stopped");
	//if (m_con) delete m_con;
	m_con = NULL; // owned by the thread
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	// And delete any frames left
	while ( ! m_frames.empty() ) {
		_pop_top_frame();
	}
	if (m_old_frame.second) {
		free(m_old_frame.second);
		m_old_frame.second = NULL;
	}
	m_lock.leave();
}	

bool
ffmpeg_video_decoder_datasource::has_audio()
{
	m_lock.enter();
	bool rv = m_src && m_src->has_audio();
	m_lock.leave();
	return rv;
}

audio_datasource *
ffmpeg_video_decoder_datasource::get_audio_datasource()
{
	m_lock.enter();
	audio_datasource *rv = NULL;
	if (m_src) 
		rv = m_src->get_audio_datasource();
	m_lock.leave();
	return rv;
}

void 
ffmpeg_video_decoder_datasource::start_frame(ambulant::lib::event_processor *evp, 
	ambulant::lib::event *callbackk, timestamp_t timestamp)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: (this = 0x%x)", (void*) this);

	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start(): m_client_callback already set!");
	}
	if (m_frames.size() > 0 /* XXXX Check timestamp! */ || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			lib::timer::time_type stp_t = timestamp/1000; // micro to milli
			lib::timer::time_type rel_t = 33; // 30 frames/sec
			lib::timer::time_type elp_t = evp->get_timer()->elapsed();
			if (stp_t > elp_t)
				rel_t = stp_t - elp_t;
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start: trigger client callback timestamp=%d rel_t=%d, elp_t=%d", (int)timestamp,(int)rel_t,(int)elp_t);
			evp->add_event(callbackk, rel_t, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
		if (!m_src->end_of_file()) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame() Calling m_src->start_frame(..)");
			lib::event *e = new framedone_callback(this, &ffmpeg_video_decoder_datasource::data_avail);
			m_src->start_frame(evp, e, timestamp);
		}
	}
	//~ if (!m_thread_started) {
		//~ m_thread->start();
		//~ m_thread_started = true;
	//~ }
	m_lock.leave();
}

void
print_frames(sorted_frames frames) {
  	sorted_frames f(frames);
	while (f.size()) {
		qelt e = f.top();
		printf("e.first=%d ", (int) e.first);
		f.pop();
	}
	printf("\n");
	return;
}

qelt 
ffmpeg_video_decoder_datasource::_pop_top_frame() {
	// pop a frame, return the new top frame
	// the old top frame is remembered in m_old_frame
	// old data in m_old_frame is freed
  
	if (m_old_frame.second)
		free (m_old_frame.second);
	if (m_frames.empty()) {
		m_old_frame.second = NULL;
		return m_old_frame;
	}
	m_old_frame = m_frames.top();
	m_frames.pop();
	return m_frames.top();
}

void 
ffmpeg_video_decoder_datasource::frame_done(timestamp_t timestamp, bool keepdata)
{
	m_lock.enter();
	if (m_frames.size() == 0) {
		lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource.readdone: frame_done() called with no current frames");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		m_lock.leave();
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.frame_done(%f)", timestamp);
//	printf("timestamp=%d\n", timestamp);
//	print_frames(m_frames);
	while( m_frames.size() > 0 ) {
		std::pair<timestamp_t, char*> element = m_frames.top();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_done(%d): element=<%d,0x%x>",(int)timestamp,(int)element.first, element.second);
		if (element.first > timestamp)
			break;
		if (m_old_frame.second) {
			free(m_old_frame.second);
			m_old_frame.second = NULL;
		}
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_done(%d): removing frame with ts=%d",(int)timestamp,(int)element.first);
		m_old_frame = element;
		m_frames.pop();
		if (!keepdata) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_done(%f): free(0x%x)", timestamp, m_old_frame.second);
			free(m_old_frame.second);
			m_old_frame.second = NULL;
		}
	}
	m_lock.leave();
}

int 
ffmpeg_video_decoder_datasource::width()
{
	return m_fmt.width;
}

int 
ffmpeg_video_decoder_datasource::height()
{
	return m_fmt.height;
}

//~ #undef AM_DBG
//~ #define AM_DBG

void 
ffmpeg_video_decoder_datasource::data_avail()
{
	m_lock.enter();
	int got_pic;
	AVFrame *frame = avcodec_alloc_frame();
	AVPicture picture;
	int len, dummy2;
	int pic_fmt, dst_pic_fmt;
	int width,height;
	int num, den;
	int framerate;
	int framebase;
	timestamp_t pts, pts1;
    unsigned char* ptr;
	double frame_delay;
	
	timestamp_t ipts;
	uint8_t *inbuf;
	int sz;
	char fn[50];
	got_pic = 0;
	
	inbuf = (uint8_t*) m_src->get_frame(0, &ipts,&sz);
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: %d bytes available", sz);
	while(inbuf && sz ) {	
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:start decoding (0x%x) ", m_con);
		assert(&m_con != NULL);
		assert(inbuf);
		assert(sz < 1000000); // XXXX This is soft, and probably incorrect. Remove when it fails.
		ptr = inbuf;
		
		while (sz > 0) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: decoding picture(s),  %d byteas of data ", sz);
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: m_con: 0x%x, gotpic = %d, sz = %d ", m_con, got_pic, sz);
				len = avcodec_decode_video(m_con, frame, &got_pic, ptr, sz);
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: avcodec_decode_video:, returned %d decoded bytes , %d left, gotpic = %d, ipts = %lld", len, sz - len, got_pic, ipts);
				if (len >= 0) {
					assert(len <= sz);
					ptr +=len;	
					sz  -= len;
					if (got_pic) {
						AM_DBG lib::logger::get_logger()->debug("pts seems to be : %lld",ipts);
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: decoded picture, used %d bytes, %d left", len, sz);
						// Setup the AVPicture for the format we want, plus the data pointer
						width = m_fmt.width;
						height = m_fmt.height;
						m_size = width * height * 4;
						char *framedata = (char*) malloc(m_size);
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:framedata=0x%x", framedata);
						assert(framedata != NULL);
						dst_pic_fmt = PIX_FMT_RGBA32;
						dummy2 = avpicture_fill(&picture, (uint8_t*) framedata, dst_pic_fmt, width, height);
						// The format we have is already in frame. Convert.
						pic_fmt = m_con->pix_fmt;
						img_convert(&picture, dst_pic_fmt, (AVPicture*) frame, pic_fmt, width, height);
						
						// And convert the timestamp

						framerate = m_con->frame_rate;
						framebase = m_con->frame_rate_base;
						
						
						//~ pts = ipts;
						

						//~ AM_DBG lib::logger::get_logger()->debug("pts seems to be : %f",pts / 1000000.0);
						//~ pts1 = pts;
						
						if (m_con->has_b_frames && frame->pict_type != FF_B_TYPE) {
							pts = m_last_p_pts;
							m_last_p_pts = pts1;
							AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:frame has B frames but this frame is no B frame  (this=0x%x) ", this);
							AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:pts set to %f, remember %f", pts, m_last_p_pts);
						}
						
						
						//~ if (pts != 0 ) {
							//~ m_pts_last_frame = pts;
						//~ } else {
							//~ if (framerate != 0) {
								//~ frame_delay = (double) (framebase*1000000)/framerate;
							//~ } else {
								//~ frame_delay = 0;
							//~ }
							//~ pts = m_pts_last_frame + (timestamp_t) round(frame_delay);
							//~ m_pts_last_frame = pts;			
							//~ //if( frame.repeat_pict) {
							//~ //	pts += frame.repeat_pict * (frame_delay * 0.5);
							//~ //}
							//~ /*AM_DBG*/ lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:pts was 0, set to %f (frame_delay = %f)", pts / 1000000.0, round(frame_delay*1000000));
						//~ }
						
						//~ AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: timestamp=%lld num=%d, den=%d",pts, num,den);
						
						//~ AM_DBG {
							//~ switch(frame->pict_type) {
								//~ case FF_B_TYPE:
									//~ lib::logger::get_logger()->debug("BBBBB ffmpeg_video_decoder_datasource.data_avail: B-frame(%d), pts = %lld, ipts = %lld, m_pts_last_frame = %lld",m_frame_count, pts, ipts, m_pts_last_frame); 
									//~ break;
								//~ case FF_P_TYPE:
									//~ lib::logger::get_logger()->debug("PPPPP ffmpeg_video_decoder_datasource.data_avail: P-frame(%d), pts = %lld, ipts = %lld, m_pts_last_frame = %lld",m_frame_count, pts, ipts, m_pts_last_frame); 
									//~ break;
								//~ case FF_I_TYPE:
									//~ lib::logger::get_logger()->debug("IIIII ffmpeg_video_decoder_datasource.data_avail: I-frame(%d), pts = %lld, ipts = %lld, m_pts_last_frame = %lld",m_frame_count, pts, ipts, m_pts_last_frame); 
									//~ break;
								//~ default:
									//~ lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: unkown frame(%d), pts = %lld, ipts = %lld, m_pts_last_frame = %lld",m_frame_count, pts, ipts, m_pts_last_frame); 
							//~ }
						//~ }
						
						// Stupid HAck to get the pts right, we will have to look again to this later
						frame_delay = (double) (framebase*1000000)/framerate;
						pts = (timestamp_t) round(frame_delay*m_frame_count);
						// And store the data.
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: storing frame with pts = %lld",pts );
						//~ sprintf(fn,"%lld-%d.png",pts,m_frame_count);		
						//~ image = new QImage((uchar*) framedata, width, height, 32, NULL, 0, QImage::IgnoreEndian);
						//~ image->save(fn,"PNG");
						m_frame_count++;
						std::pair<timestamp_t, char*> element(pts, framedata);
						m_frames.push(element);
					} else {
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: incomplete picture, used %d bytes, %d left", len, sz);
					}
				} else {
					lib::logger::get_logger()->error(gettext("error decoding video frame"));
				}

		} // End of while loop
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:done decoding (0x%x) ", m_con);
		m_src->frame_done(0, false);
  	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): m_frames.size() returns %d, (eof=%d)", m_frames.size(), m_src->end_of_file());
	if ( m_frames.size() || m_src->end_of_file()  ) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): there is some data for the renderer ! (eof=%d)", m_src->end_of_file());
	  if ( m_client_callback ) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): calling client callback (eof=%d)", m_src->end_of_file());
		assert(m_event_processor);
		m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::med);
		m_client_callback = NULL;
		//m_event_processor = NULL;
	  } else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource::data_avail(): No client callback!");
	  }
  	}
	if (!m_src->end_of_file()) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame() Calling m_src->start_frame(..)");
			lib::event *e = new framedone_callback(this, &ffmpeg_video_decoder_datasource::data_avail);
			m_src->start_frame(m_event_processor, e, ipts);
	}
	av_free(frame);
	m_lock.leave();
}

bool 
ffmpeg_video_decoder_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
ffmpeg_video_decoder_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_frames.size() > 0)  {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_end_of_file() returning false (still %d frames in local buffer)", m_frames.size());
			return false;
	}
	return m_src->end_of_file();
}

bool 
ffmpeg_video_decoder_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = _buffer_full();
	m_lock.leave();
	return rv;
}	


bool 
ffmpeg_video_decoder_datasource::_buffer_full()
{
	// private method - no need to lock
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_buffer_full() (this=0x%x, count=%d)", (void*) this, m_frames.size());
	bool rv = (m_frames.size() > MAX_VIDEO_FRAMES);
	return rv;
}	


char* 
ffmpeg_video_decoder_datasource::get_frame(timestamp_t now, timestamp_t *timestamp_p, int *size_p)
{
	// drop frames until (just before) "now"
	qelt frame;
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame(now=%d)\n", (int) now);

	while ( m_frames.size() 
		&& (frame = _pop_top_frame(), frame.first < now)) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: discarding m_old_frame timestamp=%d, now=%d, data ptr = 0x%x",(int)m_old_frame.first,(int)now, m_old_frame.second);
	}

	frame = m_old_frame;
	m_lock.leave();
	if (timestamp_p) *timestamp_p = frame.first;
	if (size_p) *size_p = m_size;
	return frame.second;
}

std::pair<bool, double>
ffmpeg_video_decoder_datasource::get_dur()
{
	
	return m_src->get_dur();
}

#endif // WITH_FFMPEG_AVFORMAT

// **************************** ffpmeg_decoder_datasource *****************************

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(const net::url& url, datasource *const src)
:	m_con(NULL),
	m_fmt(audio_format(0,0,0)),
	m_event_processor(NULL),
	m_src(src),
	m_duration(false, 0),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource() -> 0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
	ffmpeg_init();
	const char *ext = getext(url);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: Selecting \"%s\" decoder", ext);
	if (!_select_decoder(ext))
		lib::logger::get_logger()->error(gettext("%s: audio decoder \"%s\" not supported"), url.get_url().c_str(), ext);
}

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(audio_datasource *const src)
:	m_con(NULL),
	m_fmt(src->get_audio_format()),
	m_event_processor(NULL),
	m_src(src),
	m_duration(false, 0),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource() -> 0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
	ffmpeg_init();
	audio_format fmt = src->get_audio_format();
	m_duration = src->get_dur();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: Looking for %s(0x%x) decoder", fmt.name.c_str(), fmt.parameters);
	if (!_select_decoder(fmt))
		lib::logger::get_logger()->error(gettext("ffmpeg_decoder_datasource: could not select %s(0x%x) decoder"), fmt.name.c_str(), fmt.parameters);
}

ffmpeg_decoder_datasource::~ffmpeg_decoder_datasource()
{
	stop();
}

void
ffmpeg_decoder_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::stop(0x%x)", (void*)this);
	if (m_con) avcodec_close(m_con);
	m_con = NULL;
	if (m_src) {
		m_src->stop();
		int rem = m_src->release();
		if (rem) lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::stop(0x%x): m_src refcount=%d", (void*)this, rem); 
	}
	m_src = NULL;
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
}	

int
ffmpeg_decoder_datasource::_decode(uint8_t* in, int sz, uint8_t* out, int &outsize)
{
	return avcodec_decode_audio(m_con, (short*) out, &outsize, in, sz);
}
		  
void 
ffmpeg_decoder_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	
	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): m_client_callback already set!");
	}
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
		lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(m_event_processor,  e);
	}
	m_lock.leave();
}
 
void 
ffmpeg_decoder_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.readdone : done with %d bytes", len);
/* 	if(( !(buffer_full()) && !m_src->end_of_file() )) {
 * 		lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
 * 		m_src->start(m_event_processor, e);
 * 	}
 */
	m_lock.leave();
}

void 
ffmpeg_decoder_datasource::data_avail()
{
	m_lock.enter();		    
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: called : m_src->get_read_ptr() m_src=0x%x, this=0x%x", (void*) m_src, (void*) this);		
	int sz;
	if (m_con) {
		if (m_src) {
			sz = m_src->size();
		} else {
			lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No datasource !");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
			return;
		}	
		if (sz && !m_buffer.buffer_full()) {
		    AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: m_src->get_read_ptr() m_src=0x%x, this=0x%x", (void*) m_src, (void*) this);		
			uint8_t *inbuf = (uint8_t*) m_src->get_read_ptr();
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: %d bytes available", sz);
			// Note: outsize is only written by avcodec_decode_audio, not read!
			// You must always supply a buffer that is AVCODEC_MAX_AUDIO_FRAME_SIZE
			// bytes big!
			int outsize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
			uint8_t *outbuf = (uint8_t*) m_buffer.get_write_ptr(outsize);
			if (outbuf) {
				if(inbuf) {
					// Don't feed to much data to the decoder, it doesn't like to do lists ;-)
					int cursz = sz;
					if (cursz > AVCODEC_MAX_AUDIO_FRAME_SIZE/2) cursz = AVCODEC_MAX_AUDIO_FRAME_SIZE/2;
					
					//XXX Ugly hack, but it doesn't work  :-(
					// Someone kicks away the buffer while we still need it.
					//uint8_t *tmpptr = (uint8_t*) malloc(cursz);
					//memcpy(tmpptr, inbuf, cursz);
					//XXX end hack
					
					AM_DBG lib::logger::get_logger()->debug("avcodec_decode_audio(0x%x, 0x%x, 0x%x(%d), 0x%x, %d)", (void*)m_con, (void*)outbuf, (void*)&outsize, outsize, (void*)inbuf, sz);
					int decoded = avcodec_decode_audio(m_con, (short*) outbuf, &outsize, inbuf, cursz);
					//free(tmpptr);
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail : %d bps",m_con->sample_rate);
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail : %d bytes decoded  to %d bytes", decoded,outsize );
					if (outsize > 0) {
						m_buffer.pushdata(outsize);
					} else {
						m_buffer.pushdata(0);
					}
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail : m_src->readdone(%d) called m_src=0x%x, this=0x%x", decoded,(void*) m_src, (void*) this );
					m_src->readdone(decoded);
				} else {
					m_buffer.pushdata(0);
					m_src->readdone(0);
				}
			} else {
				lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: no room in output buffer");
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				m_buffer.pushdata(0);
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail m_src->readdone(0) called this=0x%x");
				m_src->readdone(0);
			}
		//	sz = m_src->size();
		}
		// Restart reading if we still have room to accomodate more data
		if (!m_src->end_of_file() && m_event_processor && !m_buffer.buffer_full()) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): calling m_src->start() again");
			lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
			m_src->start(m_event_processor, e);
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: not calling start: eof=%d m_ep=0x%x buffull=%d", 
				(int)m_src->end_of_file(), (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
	
		if ( m_client_callback && (m_buffer.buffer_not_empty() || _end_of_file()) ) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), _end_of_file());
			assert(m_event_processor);
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::med);
			m_client_callback = NULL;
			//m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No client callback!");
		}
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No decoder, flushing available data");
		if (m_src) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): m_src->readdone(%d) called m_src=0x%x, this=0x%x",m_src->size(), (void*) m_src, (void*) this );
			m_src->readdone(m_src->size());
		}
	}
	m_lock.leave();
}


bool 
ffmpeg_decoder_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
ffmpeg_decoder_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	return m_src->end_of_file();
}

bool 
ffmpeg_decoder_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}	

char* 
ffmpeg_decoder_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

int 
ffmpeg_decoder_datasource::size() const
{
	const_cast <ffmpeg_decoder_datasource*>(this)->m_lock.enter();
	int rv = m_buffer.size();
	const_cast <ffmpeg_decoder_datasource*>(this)->m_lock.leave();
	return rv;
}	
 
bool 
ffmpeg_decoder_datasource::_select_decoder(const char* file_ext)
{
	AVCodec *codec = avcodec_find_decoder_by_name(file_ext);
	if (codec == NULL) {
			lib::logger::get_logger()->trace("ffmpeg_decoder_datasource._select_decoder: Failed to find codec for \"%s\"", file_ext);
			lib::logger::get_logger()->error(gettext("No support for \"%s\" audio"));
			return false;
	}
	m_con = avcodec_alloc_context();
	
	if(avcodec_open(m_con,codec) < 0) {
			lib::logger::get_logger()->trace("ffmpeg_decoder_datasource._select_decoder: Failed to open avcodec for \"%s\"", file_ext);
			lib::logger::get_logger()->error(gettext("No support for \"%s\" audio"));
			return false;
	}
	return true;
}

bool 
ffmpeg_decoder_datasource::_select_decoder(audio_format &fmt)
{
	if (fmt.name == "ffmpeg") {
		AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
		if (enc == NULL) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Parameters missing for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		if (enc->codec_type != CODEC_TYPE_AUDIO) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Non-audio stream for %s(0x%x)", fmt.name.c_str(), enc->codec_type);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		AVCodec *codec = avcodec_find_decoder(enc->codec_id);
		if (codec == NULL) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Failed to find codec for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		m_con = avcodec_alloc_context();
		
		if(avcodec_open(m_con,codec) < 0) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Failed to open avcodec for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		
		m_fmt = audio_format(enc->sample_rate, enc->channels, 16);
		return true;
	} else if (fmt.name == "live") {
		const char* codec_name = (char*) fmt.parameters;
	
		AM_DBG lib::logger::get_logger()->debug("ffmpe_decoder_datasource::selectdecoder(): audio codec : %s", codec_name);
		ffmpeg_codec_id* codecid = ffmpeg_codec_id::instance();
		AVCodec *codec = avcodec_find_decoder(codecid->get_codec_id(codec_name));
		m_con = avcodec_alloc_context();	
		
		if( !codec) {
			//lib::logger::get_logger()->error(gettext("%s: Audio codec %d(%s) not supported"), repr(url).c_str(), m_con->codec_id, m_con->codec_name);
			return false;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::selectdecoder(): codec found!");
		}
	
		if((avcodec_open(m_con,codec) < 0) ) {
			//lib::logger::get_logger()->error(gettext("%s: Cannot open audio codec %d(%s)"), repr(url).c_str(), m_con->codec_id, m_con->codec_name);
			return false;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(): succesfully opened codec");
		}
		
		m_con->codec_type = CODEC_TYPE_AUDIO;
		return true;
	}
	// Could add support here for raw mp3, etc.
	return false;
}

audio_format&
ffmpeg_decoder_datasource::get_audio_format()
{
	m_lock.enter();
#if 0
	if (m_con) {
		// Refresh info on audio format
		m_fmt.samplerate = m_con->sample_rate;
		m_fmt.bits = 16; // XXXX
		m_fmt.channels = m_con->channels;
	}
#endif
	//m_fmt.bits = 16; // XXXX
	if (m_fmt.samplerate == 0) {
		lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::get_audio_format: samplerate not set, asking ffmpeg");
		m_fmt.samplerate = m_con->sample_rate;
	}
	if (m_fmt.channels == 0) {
		lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::get_audio_format: channels not set, guessing 2");
		m_fmt.channels = m_con->channels;
	}
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::get_audio_format: rate=%d, bits=%d,channels=%d",m_fmt.samplerate, m_fmt.bits, m_fmt.channels);
	m_lock.leave();
	return m_fmt;
}

std::pair<bool, double>
ffmpeg_decoder_datasource::get_dur()
{
	return m_duration;
}

bool
ffmpeg_decoder_datasource::supported(const net::url& url)
{
	ffmpeg_init();
	const char *file_ext = getext(url);
	AVCodec  *codec = avcodec_find_decoder_by_name(file_ext);
	if (codec == NULL) lib::logger::get_logger()->trace("ffmpeg_decoder_datasource: no such decoder: \"%s\"", file_ext);
	return codec != NULL;
}


bool 
ffmpeg_video_decoder_datasource::_select_decoder(const char* file_ext)
{
	AVCodec *codec = avcodec_find_decoder_by_name(file_ext);
	if (codec == NULL) {
			lib::logger::get_logger()->trace("ffmpeg_decoder_datasource._select_decoder: Failed to find codec for \"%s\"", file_ext);
			lib::logger::get_logger()->error(gettext("No support for \"%s\" audio"));
			return false;
	}
	m_con = avcodec_alloc_context();
	
	if(avcodec_open(m_con,codec) < 0) {
			lib::logger::get_logger()->trace("ffmpeg_decoder_datasource._select_decoder: Failed to open avcodec for \"%s\"", file_ext);
			lib::logger::get_logger()->error(gettext("No support for \"%s\" audio"));
			return false;
	}
	return true;
}

bool 
ffmpeg_video_decoder_datasource::_select_decoder(video_format &fmt)
{
	if (fmt.name == "ffmpeg") {
		AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
		m_con = enc;
		if (enc == NULL) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Parameters missing for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		if (enc->codec_type != CODEC_TYPE_VIDEO) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Non-audio stream for %s(0x%x)", fmt.name.c_str(), enc->codec_type);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._select_decoder: enc->codec_id = 0x%x", enc->codec_id);
		AVCodec *codec = avcodec_find_decoder(enc->codec_id);
		//AVCodec *codec = avcodec_find_decoder(CODEC_ID_MPEG2VIDEO);
		if (codec == NULL) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Failed to find codec for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		//m_con = avcodec_alloc_context();
		
		if(avcodec_open(m_con,codec) < 0) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Failed to open avcodec for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		if (fmt.width == 0) fmt.width = m_con->width;
		if (fmt.height == 0) fmt.width = m_con->height;
		return true;
	} else if (fmt.name == "live") {
		const char* codec_name = (char*) fmt.parameters;
	
		AM_DBG lib::logger::get_logger()->debug("ffmpe_video_decoder_datasource::selectdecoder(): audio codec : %s", codec_name);
		ffmpeg_codec_id* codecid = ffmpeg_codec_id::instance();
		AVCodec *codec = avcodec_find_decoder(codecid->get_codec_id(codec_name));
		m_con = avcodec_alloc_context();	
		
		if( !codec) {
			//lib::logger::get_logger()->error(gettext("%s: Audio codec %d(%s) not supported"), repr(url).c_str(), m_con->codec_id, m_con->codec_name);
			return false;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::selectdecoder(): codec found!");
		}
	
		if((avcodec_open(m_con,codec) < 0) ) {
			//lib::logger::get_logger()->error(gettext("%s: Cannot open audio codec %d(%s)"), repr(url).c_str(), m_con->codec_id, m_con->codec_name);
			return false;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::ffmpeg_decoder_datasource(): succesfully opened codec");
		}
		
		m_con->codec_type = CODEC_TYPE_AUDIO;
		return true;
	}
	// Could add support here for raw mp3, etc.
	return false;
}

// **************************** ffmpeg_resample_datasource *****************************

ffmpeg_resample_datasource::ffmpeg_resample_datasource(audio_datasource *src, audio_format_choices fmts) 
:	m_src(src),
	m_context_set(false),
	m_resample_context(NULL),
	m_event_processor(NULL),
	m_client_callback(NULL),
	m_in_fmt(src->get_audio_format()),
	m_out_fmt(fmts.best())
{
	ffmpeg_init();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::ffmpeg_resample_datasource()->0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
#ifdef RESAMPLE_READ_ALL
	m_buffer.set_max_size(0);
#endif
}

ffmpeg_resample_datasource::~ffmpeg_resample_datasource() 
{
	stop();
}

void
ffmpeg_resample_datasource::stop() 
{
	m_lock.enter();
	int oldrefcount = get_ref_count();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x)", (void*)this);
	if (m_src) {
		m_src->stop();
		int rem = m_src->release();
		if (rem) lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x): m_src refcount=%d", (void*)this, rem); 
		m_src = NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x): m_src already NULL", (void*)this);
	}
	m_src = NULL;
	if (m_resample_context) audio_resample_close(m_resample_context);
	m_resample_context = NULL;
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x) refcount was %d is %d", (void*)this, oldrefcount, get_ref_count());
	m_lock.leave();
}

void
ffmpeg_resample_datasource::data_avail()
{
	m_lock.enter();
	int sz;
	
	int cursize = 0;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(0x%x) refcount is %d", (void*)this, get_ref_count());
	if (!m_src) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(0x%x): already stopping", (void*)this);
		m_lock.leave();			
		return;
	}
	// We now have enough information to determine the resample parameters
	if (!m_context_set) {
		m_in_fmt = m_src->get_audio_format();
		assert(m_in_fmt.bits == 16);
		assert(m_out_fmt.bits == 16);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource: initializing context: inrate, ch=%d, %d, outrate, ch=%d, %d", m_in_fmt.samplerate,  m_in_fmt.channels, m_out_fmt.samplerate,  m_out_fmt.channels);
		m_resample_context = audio_resample_init(m_out_fmt.channels, m_in_fmt.channels, m_out_fmt.samplerate,m_in_fmt.samplerate);
		m_context_set = true;
	}
		if(m_src) {
			sz = m_src->size();
		} else {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_audio_datasource::data_avail: No datasource");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
			return;
		}
		
		if (m_resample_context) {
		// Convert all the input data we have available. We make an educated guess at the number of bytes
		// this will produce on output.
	
			cursize = sz;
			// Don't feed to much data to the resampler, it doesn't like to do lists ;-)
			if (cursize > INBUF_SIZE) 
				cursize = INBUF_SIZE;
			
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: cursize=%d, sz=%d, in channels=%d", cursize,sz,m_in_fmt.channels);

			int insamples = cursize / (m_in_fmt.channels * sizeof(short));	// integer division !!!!
			if (insamples * m_in_fmt.channels * sizeof(short) != cursize) {
				lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: warning: incomplete samples: %d", cursize);
			}
			
			timestamp_t tmp = (timestamp_t)((insamples+1) * m_out_fmt.samplerate * m_out_fmt.channels * sizeof(short) / m_in_fmt.samplerate);
			long outsz = tmp;
			
	
			if (!cursize && !m_src->end_of_file()) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(0x%x): no data available, not end-of-file!", (void*)this);
				m_lock.leave();			
				return;
			}
			assert( cursize || m_src->end_of_file());
			//if (sz & 1) lib::logger::get_logger()->warn("ffmpeg_resample_datasource::data_avail: warning: oddsized datasize %d", sz);
			if (!m_buffer.buffer_full()) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): m_src->get_read_ptr() m_src=0x%x, this=0x%x",(void*) m_src, (void*) this);
				short int *inbuf = (short int*) m_src->get_read_ptr();
				short int *outbuf = (short int*) m_buffer.get_write_ptr(outsz);
				if (!outbuf) {
					lib::logger::get_logger()->debug("Internal error: ffmpeg_audio_datasource::data_avail: no room in output buffer");
					lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
					m_src->readdone(0);
					m_buffer.pushdata(0);
				}
				if (inbuf && outbuf && insamples > 0) {
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: sz=%d, insamples=%d, outsz=%d, inbuf=0x%x, outbuf=0x%x", cursize, insamples, outsz, inbuf, outbuf);
					int outsamples = audio_resample(m_resample_context, outbuf, inbuf, insamples);
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): putting %d bytes in %d bytes buffer space", outsamples*m_out_fmt.channels*sizeof(short), outsz);
					assert(outsamples*m_out_fmt.channels*sizeof(short) <= outsz);
					m_buffer.pushdata(outsamples*m_out_fmt.channels*sizeof(short));
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling m_src->readdone(%d) this=0x%x", insamples*m_in_fmt.channels*sizeof(short), (void*) this);
					m_src->readdone(insamples*m_in_fmt.channels*sizeof(short));

				} else {					
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling m_src->readdone(0) m_src=0x%x, this=0x%x",  (void*) m_src, (void*) this);
					m_src->readdone(0);
					m_buffer.pushdata(0);
				}
			}
		// Restart reading if we still have room to accomodate more data
		if (!m_src->end_of_file() && m_event_processor && !m_buffer.buffer_full()) {
			lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling m_src->start(), refcount=%d", get_ref_count());
			m_src->start(m_event_processor, e);
#ifdef RESAMPLE_READ_ALL
			// workaround for sdl bug: if RESAMPLE_READ_ALL is defined we continue
			// reading until we have all data
			m_lock.leave();
			return;
#endif /* RESAMPLE_READ_ALL */
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: not calling start: eof=%d m_ep=0x%x buffull=%d", 
				(int)m_src->end_of_file(), (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
		// If the client is currently interested tell them about data being available
		if (m_client_callback && (m_buffer.buffer_not_empty() || _end_of_file())) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), _end_of_file());
			assert(m_event_processor);
			lib::event *clientcallback = m_client_callback;
			m_client_callback = NULL;
			m_event_processor->add_event(clientcallback, 0, ambulant::lib::event_processor::med);
			//m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): No client callback!");
		}
	} else {
		// Something went wrong during initialization, we just drop the data
		// on the floor.
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): No resample context, flushing data");
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): m_src->readdone(%d) called m_src=0x%x, this=0x%x", sz, (void*) m_src, (void*) this);
		m_src->readdone(sz);
	}
	m_lock.leave();
}


void 
ffmpeg_resample_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::readdone: done with %d bytes", len);
	if (!m_src->end_of_file() && m_event_processor && !m_buffer.buffer_full()) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::readdone: calling m_src->start() again");
		lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
		m_src->start(m_event_processor, e);
	}
	m_lock.leave();
}

bool 
ffmpeg_resample_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
ffmpeg_resample_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	if (m_src)
		return m_src->end_of_file();
	
	return true;
}

bool 
ffmpeg_resample_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}	

char* 
ffmpeg_resample_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

int 
ffmpeg_resample_datasource::size() const
{
	const_cast <ffmpeg_resample_datasource*>(this)->m_lock.enter();
	int rv = m_buffer.size();
	const_cast <ffmpeg_resample_datasource*>(this)->m_lock.leave();
	return rv;
}	

void 
ffmpeg_resample_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): start(0x%x) called", this);
	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): m_client_callback already set!");
	}
	
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::med);
		} else {
			lib::logger::get_logger()->error("Internal error: ffmpeg_resample_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
		lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(m_event_processor,  e);
	}
	
	m_lock.leave();
}

std::pair<bool, double>
ffmpeg_resample_datasource::get_dur()
{
	std::pair<bool, double> rv(false, 0.0);
	m_lock.enter();
	if (m_src)
		rv = m_src->get_dur();
	m_lock.leave();
	return rv;
}
