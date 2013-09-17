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

#include <math.h>
#include <map>
#define round(x) ((int)((x)+0.5))

#include "ambulant/net/ffmpeg_common.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/logger.h"
#include "ambulant/net/url.h"
#include "ambulant/common/preferences.h"

// WARNING: turning on AM_DBG globally for the ffmpeg code seems to trigger
// a condition that makes the whole player hang or collapse. So you probably
// shouldn't do it:-)
//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

const AVRational AMBULANT_TIMEBASE = {1, 1000000};

// Update the minimal required MIN_AVFORMAT_BUILD whenever we use a newer libavformat.
#define MIN_LIBAVFORMAT_BUILD ((51<<16)+(12<<8)+2)
#if LIBAVFORMAT_BUILD < MIN_LIBAVFORMAT_BUILD
//#error Your ffmpeg is too old. Either download a newer one or remove this test in the sourcefile (at your own risk).
#endif

using namespace ambulant;
using namespace net;

// There is a bug in the ffpmeg http seek code, which causes http header data to be
// interspersed into the datastream. This bug has been registered in the ambulant bug database as
// #2916230. It has also been submitted to the ffmpeg developer team, as
// <https://roundup.ffmpeg.org/roundup/ffmpeg/issue1631>, it was fixed in January 2010.

void
ambulant::net::ffmpeg_init()
{
	static bool is_inited = false;
	if (is_inited) return;
#if 1
	// Enable this line to get lots of ffmpeg debug output:
	if (getenv("AMBULANT_FFMPEG_DEBUG")) av_log_set_level(AV_LOG_DEBUG);
	if (getenv("AMBULANT_FFMPEG_QUIET")) av_log_set_level(AV_LOG_PANIC);
#endif
// avcodec_init() was useless since years, replaced by av_register_all().
// After being marked deprecated for some time, it is now completely been from API.
	av_register_all();
// Version number from https://lists.ffmpeg.org/pipermail/ffmpeg-cvslog/2011-November/043068.html
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 13, 0)
	avformat_network_init();
#endif
	is_inited = true;
}

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
		AM_DBG lib::logger::get_logger()->debug("get_codec_id(%s) -> %d", str.c_str(), i->second);
		return i->second;
	}

	return CODEC_ID_NONE;
}

ffmpeg_codec_id::ffmpeg_codec_id()
{

	// XXX Lots of formats missing here obviously
	// Live.com formats
	add_codec("MPA", CODEC_ID_MP3);
	add_codec("MPA-ROBUST", CODEC_ID_MP3);
	add_codec("X_MP3-DRAFT-00", CODEC_ID_MP3);
	add_codec("X-MP3-DRAFT-00", CODEC_ID_MP3);
	add_codec("MPV", CODEC_ID_MPEG2VIDEO);
	add_codec("L16", CODEC_ID_PCM_S16LE);
	add_codec("MP4V-ES", CODEC_ID_MPEG4);
#ifdef CODEC_ID_MPEG4AAC
	add_codec("MPEG4-GENERIC", CODEC_ID_MPEG4AAC);
#else
	add_codec("MPEG4-GENERIC", CODEC_ID_AAC);
#endif
	add_codec("X-QT", CODEC_ID_MP3); //XXXX

	// added h264 map between live and ffmpeg
	add_codec("H264", CODEC_ID_H264);
}

// **************************** ffmpeg_demux *****************************


ffmpeg_demux::ffmpeg_demux(AVFormatContext *con, const net::url& url, timestamp_t clip_begin, timestamp_t clip_end)
:	m_con(con),
    m_url(url),
	m_nstream(0),
	m_clip_begin(clip_begin),
	m_clip_end(clip_end),
	m_clip_begin_changed(false),
	m_is_live(false)
{
	assert(m_clip_begin >= 0);
	if ( m_clip_begin ) m_clip_begin_changed = true;

	m_audio_fmt = audio_format("ffmpeg");
	m_audio_fmt.bits = 16;
	int audio_idx = audio_stream_nr();
	if ( audio_idx >= 0) {
		m_audio_fmt.parameters = (void *) con->streams[audio_idx]->codec;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::ffmpeg_demux(): audio_codec_name=%s", m_con->streams[audio_idx]->codec->codec_name);
		m_audio_fmt.samplerate = con->streams[audio_idx]->codec->sample_rate;
		m_audio_fmt.channels = con->streams[audio_idx]->codec->channels;
		m_audio_fmt.bits = 16;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::ffmpeg_demux(): samplerate=%d, channels=%d, m_con->codec (0x%x)", m_audio_fmt.samplerate, m_audio_fmt.channels, m_audio_fmt.parameters  );
	}
	m_video_fmt = video_format("ffmpeg");
	int video_idx = video_stream_nr();
	if (video_idx >= 0) {
		m_video_fmt.parameters = (void *) m_con->streams[video_stream_nr()]->codec;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::ffmpeg_demux(): video_codec_name=%s", m_con->streams[video_stream_nr()]->codec->codec_name);
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::ffmpeg_demux(): No Video stream ?");
		m_video_fmt.parameters = NULL;
	}
	memset(m_sinks, 0, sizeof m_sinks);
    memset(m_data_consumed, 0, sizeof m_data_consumed);
	m_current_sink = NULL;
    m_bandwidth_resource = m_url.get_protocol();
}

ffmpeg_demux::~ffmpeg_demux()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::~ffmpeg_demux()");
	if (m_con) {
		if (m_con->nb_streams < MAX_STREAMS) {
			unsigned int stream_index;
			for (stream_index=0; stream_index < m_con->nb_streams; stream_index++) {
				if (m_con->streams[stream_index]->codec->codec_type == CODEC_TYPE_AUDIO
					|| m_con->streams[stream_index]->codec->codec_type == CODEC_TYPE_VIDEO) {
					avcodec_close(m_con->streams[stream_index]->codec);
				}
			}
		}
#if LIBAVFORMAT_VERSION_INT < 0x350400
		av_close_input_file(m_con);
		// Implies av_free(m_con);
#else
		avformat_close_input(&m_con);
#endif
	}
	m_con = NULL;
	m_lock.leave();
}

timestamp_t
ffmpeg_demux::get_clip_end()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::get_clip_end(): %lld", m_clip_end);
	return m_clip_end;
}

timestamp_t
ffmpeg_demux::get_clip_begin()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::get_clip_begin(): %lld", m_clip_begin);
	return m_clip_begin;
}

AVFormatContext *
ffmpeg_demux::supported(const net::url& url)
{
	ffmpeg_init();
	// Setup struct to allow ffmpeg to determine whether it supports this
	AVInputFormat *fmt;
	AVProbeData probe_data;
	std::string url_str(url.get_url());
	std::string ffmpeg_name = url_str;
	if (url.is_local_file()) {
		ffmpeg_name = url.get_file();
	}

	probe_data.filename = ffmpeg_name.c_str();
	probe_data.buf = NULL;
	probe_data.buf_size = 0;
	fmt = av_probe_input_format(&probe_data, 0);
	if (!fmt && url.is_local_file()) {
		// get the file extension
		std::string short_name = ffmpeg_name.substr(ffmpeg_name.length()-3,3);
		if (short_name == "mov" || short_name == "mp4") {
			// ffmpeg's id for QuickTime/MPEG4/Motion JPEG 2000 format
			short_name = "mov,mp4,m4a,3gp,3g2,mj2";
		}
		fmt = av_find_input_format(short_name.c_str());

	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::supported(%s): (%s) av_probe_input_format: 0x%x", url_str.c_str(), ffmpeg_name.c_str(), (void*)fmt);
	AVFormatContext *ic = NULL;
	int err;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
    err = av_open_input_file(&ic, ffmpeg_name.c_str(), fmt, 0, 0);
#else
	// Force rtsp-over-tcp, if that preference has been set.
	common::preferences* prefs = common::preferences::get_preferences();
	AVDictionary *options = 0;
	if (prefs->m_prefer_rtsp_tcp) {
		av_dict_set(&options, "rtsp_transport", "tcp", 0);
	}
    err = avformat_open_input(&ic, ffmpeg_name.c_str(), fmt, &options);
#endif

	if (err) {
		lib::logger::get_logger()->trace("ffmpeg_demux::supported(%s): av_open_input_file returned error %d, ic=0x%x", url_str.c_str(), err, (void*)ic);
		if (ic) {
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 17, 0)
			av_close_input_file(ic);
#else
			avformat_close_input(&ic);
#endif
		}
		return NULL;
	}
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 17, 0)
	err = av_find_stream_info(ic);
#else
	err = avformat_find_stream_info(ic, NULL);
#endif
	if (err < 0) {
		lib::logger::get_logger()->trace("ffmpeg_demux::supported(%s): av_find_stream_info returned error %d, ic=0x%x", url_str.c_str(), err, (void*)ic);
		if (ic) {
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 17, 0)
			av_close_input_file(ic);
#else
			avformat_close_input(&ic);
#endif
		}
		return NULL;
	}

	AM_DBG av_dump_format(ic, 0, ffmpeg_name.c_str(), 0);
	/*AM_DBG*/ lib::logger::get_logger()->debug("ffmpeg_demux::supported: rate=%d, channels=%d", ic->streams[0]->codec->sample_rate, ic->streams[0]->codec->channels);
	assert(ic);
	return ic;
}

void
ffmpeg_demux::cancel()
{
	if (is_running()) stop();
	release();
}

int
ffmpeg_demux::audio_stream_nr()
{
	// Failure of this assert may be an c-compiler alignment problem
	assert(m_con->nb_streams >= 0 && m_con->nb_streams < MAX_STREAMS);
	unsigned int stream_index;
	for (stream_index=0; stream_index < m_con->nb_streams; stream_index++) {
		if (m_con->streams[stream_index]->codec->codec_type == CODEC_TYPE_AUDIO)
			return stream_index;
	}

	return -1;
}

int
ffmpeg_demux::video_stream_nr()
{
	// Failure of this assert may be an c-compiler alignment problem
	assert(m_con->nb_streams >= 0 && m_con->nb_streams < MAX_STREAMS);
	unsigned int stream_index;
	for (stream_index=0; stream_index < m_con->nb_streams; stream_index++) {
		if (m_con->streams[stream_index]->codec->codec_type == CODEC_TYPE_VIDEO) {
			return stream_index;
		}
	}

	return -1;
}

double
ffmpeg_demux::duration()
{
	// XXX this is a double now, later this should retrun a long long int
	// XXX Note that this code knows that m_clip_{begin,end} and m_con->duration
	// are both microseconds
	timestamp_t dur = m_con->duration;
	if (m_clip_end > 0 && m_clip_end < dur)
		dur = m_clip_end;
	dur -= m_clip_begin;
	return dur / (double)AV_TIME_BASE;

}

int
ffmpeg_demux::nstreams()
{
	return m_con->nb_streams;
}

audio_format&
ffmpeg_demux::get_audio_format()
{
	return m_audio_fmt;
}

video_format&
ffmpeg_demux::get_video_format()
{
	m_lock.enter();
	if (m_video_fmt.width == 0) {
		m_video_fmt.width = m_con->streams[video_stream_nr()]->codec->width;
	}
	if (m_video_fmt.height == 0) {
		m_video_fmt.height = m_con->streams[video_stream_nr()]->codec->height;
	}
	m_lock.leave();
	return m_video_fmt;

}

void
ffmpeg_demux::add_datasink(demux_datasink *parent, int stream_index)
{
	m_lock.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	assert(m_sinks[stream_index] == 0);
	m_sinks[stream_index] = parent;
	parent->add_ref();
	m_nstream++;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::add_datasink(0x%x): stream_index=%d parent=0x%x m_current_sink=0x%x m_nstream=%d", this, stream_index, parent, m_current_sink, m_nstream);
	m_lock.leave();
}

void
ffmpeg_demux::read_ahead(timestamp_t time)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::read_ahead(%d), m_clip_begin was %d", time, m_clip_begin);
	if (m_clip_begin != time) {
		m_clip_begin = time;
		m_clip_begin_changed = true;
	}
	m_lock.leave();
}

void
ffmpeg_demux::seek(timestamp_t time)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::seek(0x%x): time=%d", this, time);
	assert( time >= 0);
	m_clip_begin = time;
	m_clip_begin_changed = true;
	m_lock.leave();
}

void
ffmpeg_demux::set_clip_end(timestamp_t clip_end)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_set_clip_end(0x%x): clip_end=%d", this, clip_end);
	m_clip_end = clip_end;
	m_lock.leave();
}

long
ffmpeg_demux::get_bandwidth_usage_data(int stream_index, const char **resource)
{
	m_lock.enter();
    assert(stream_index >= 0 && stream_index <= MAX_STREAMS);
    long rv = m_data_consumed[stream_index];
    m_data_consumed[stream_index] = 0;
    *resource = m_bandwidth_resource.c_str();
	m_lock.leave();
    return rv;
}

void
ffmpeg_demux::remove_datasink(int stream_index)
{
	demux_datasink* ds;
	m_lock.enter();
	assert(stream_index >= 0 && stream_index < MAX_STREAMS);
	ds = m_sinks[stream_index];
	m_sinks[stream_index] = 0;
	if (ds) m_nstream--;
	m_lock.leave();
	if (ds && ds != m_current_sink){
		// If the sink is currently busy (in run()) then
		// run() will take care of disposal.
		// signal EOF
		ds->push_data(0, 0, 0);
		ds->release();
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_demux::remove_datasink(0x%x): stream_index=%d ds=0x%x m_current_sink=0x%x m_nstream=%d", this, stream_index, ds, m_current_sink, m_nstream);
	if (m_nstream <= 0) cancel();
}


unsigned long
ffmpeg_demux::run()
{
	m_lock.enter();
	int pkt_nr;
	int video_streamnr = video_stream_nr();
	int audio_streamnr = audio_stream_nr();
	int64_t pts = 0;
//#define RESYNC_TO_INITIAL_AUDIO_PTS 1
#if RESYNC_TO_INITIAL_AUDIO_PTS
	timestamp_t initial_audio_pts = 0;
	bool initial_audio_pts_set = false;
#endif

	timestamp_t last_valid_audio_pts = 0;

	pkt_nr = 0;
	assert(m_con);

	bool eof_sent_to_clients = false;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: started");
	while (!exit_requested()) {
		AVPacket pkt1, *pkt = &pkt1;

		pkt->pts = 0;
		// Read a packet
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run:  started");
		if (m_clip_begin_changed) {
			eof_sent_to_clients = false;
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: seek to %lld", m_clip_begin );
			int64_t seektime = m_clip_begin;

			// Seeking in ffmpeg seems to cause no end to problems. The previous code, which seeked only one
			// of the streams, seems to (sometimes? always?) leave the other stream positioned where it was.
			// We work around this by seeking all streams.
			int64_t seektime_a, seektime_v;
			if (audio_streamnr >= 0)
				seektime_a = av_rescale_q(seektime, AMBULANT_TIMEBASE, m_con->streams[audio_streamnr]->time_base);
			if (video_streamnr >= 0)
				seektime_v = av_rescale_q(seektime, AMBULANT_TIMEBASE, m_con->streams[video_streamnr]->time_base);
			int seekresult = 0;
			m_lock.leave();
			if (audio_streamnr >= 0)
				seekresult = av_seek_frame(m_con, audio_streamnr, seektime_a, AVSEEK_FLAG_BACKWARD);
			//if (seekresult > 0 && video_streamnr >= 0)
			// xxxbo: the original seekresult > 0 will make the video-only not work
			if (seekresult >= 0 && video_streamnr >= 0)
				seekresult = av_seek_frame(m_con, video_streamnr, seektime_v, AVSEEK_FLAG_BACKWARD);
			m_lock.enter();

			if (seekresult < 0) {
				lib::logger::get_logger()->debug("ffmpeg_demux: av_seek_frame() returned %d", seekresult);
			}
			m_clip_begin_changed = false;
		}
		m_lock.leave();
		int ret = av_read_frame(m_con, pkt);
		m_lock.enter();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: av_read_packet returned ret= %d, (%d, 0x%x, %d, %d)", ret, (int)pkt->pts ,pkt->data, pkt->size, pkt->stream_index);

		if (ret < 0) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: eof encountered (%d), wait some time before continuing the while loop", ret);
			if (!m_is_live && !eof_sent_to_clients) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: sending eof to clients");
				for (int i=0; i<MAX_STREAMS; i++) {
					if (m_sinks[i]) {
						m_sinks[i]->push_data(0, 0, 0);
					}
				}
				eof_sent_to_clients = true;
			}
			// wait some time before continuing the while loop to avoid consuming too much cpu resource
			// sleep 10 millisec, hardly noticeable
			m_lock.leave();
			ambulant::lib::sleep_msec(10);
			m_lock.enter();
			continue;
		}

		pkt_nr++;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: av_read_packet number : %d",pkt_nr);
		// Find out where to send it to
		assert(pkt->stream_index >= 0 && pkt->stream_index < MAX_STREAMS);
		// Keep statistics
		m_data_consumed[pkt->stream_index] += pkt->size;
		
		demux_datasink *sink = m_sinks[pkt->stream_index];
		if (sink == NULL) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: Drop data for stream %d (%lld, %lld, 0x%x, %d)", pkt->stream_index, pts, pkt->pts ,pkt->data, pkt->size);
		} else {
			AM_DBG lib::logger::get_logger ()->debug ("ffmpeg_parser::run sending data to datasink (stream %d) (%lld, %lld, 0x%x, %d)", pkt->stream_index, pts, pkt->pts ,pkt->data, pkt->size);
			if (sink && !exit_requested()) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: raw pts=%lld, dts=%lld", pkt->pts, pkt->dts);

				pts = pkt->dts;
				if (pts == (int64_t)AV_NOPTS_VALUE) {
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: dts invalid using pts=%lld", pkt->dts);
					pts = pkt->pts;
				}
				if (!m_is_live) {
					// 17-feb-2010 To fix the chopping audio playback in vobis/ogg
					// For some reason which I don't understand, In the current version of ffmpeg, for reading vorbis in ogg,
					// sometime, the pts and dts got by ffmpeg is not valid any more (which equal to AV_NOPTS_VALUE)
					// and this kind of invalid value of pts and dts will last in the following packets for some
					// small while. This invalid pts and dts will cause ffmpeg_decoder_datasource.data_avail dropping
					// packets which should not be dropped. This kind of dropping will cause the chopping audio
					// effect when playback vorbis in ogg. In this case, we use the latest valid pts as the current
					// pts instead of the invalid AV_NOPTS_VALUE

					if (pts != (int64_t)AV_NOPTS_VALUE) {
						pts = av_rescale_q(pts, m_con->streams[pkt->stream_index]->time_base, AMBULANT_TIMEBASE);

						if (pkt->stream_index == audio_streamnr) {
							last_valid_audio_pts = pts;
						}
					} else {
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: pts and dts invalid using pts=%lld", last_valid_audio_pts);

						last_valid_audio_pts++;
						pts = last_valid_audio_pts;

#if RESYNC_TO_INITIAL_AUDIO_PTS
						// We seem to be getting values with a non-zero epoch sometimes (?)
						// Remember initial audio pts, and resync everything to that.
						// Fixes bug #2046564.
						if (!initial_audio_pts_set && pkt->stream_index == audio_streamnr) {
							initial_audio_pts = pts;
							// Bugfix to bugfix: need to take initial seek/clipbegin into account too
							initial_audio_pts -= m_clip_begin;
							initial_audio_pts_set = true;
						}
						pts -= initial_audio_pts;
#else // RESYNC_TO_INITIAL_AUDIO_PTS
						// If we don't resync to initial audio PTS we resync to start_time.
						int64_t stream_start = m_con->start_time; // m_con->streams[audio_streamnr]->start_time;
						if (stream_start != (int64_t)AV_NOPTS_VALUE) pts -= stream_start;
						// assert(pts >= 0);
#endif // RESYNC_TO_INITIAL_AUDIO_PTS
					}
				}
			}
			// We are now going to push data to one of our clients. This means that we should re-send an EOF at the end, even if
			// we have already sent one earlier.
			eof_sent_to_clients = false;
			bool accepted = false;

			// NOTE: Without checking the value of m_cli_begin_changed will possibly
			// push the wrong packets to demux_datasource, which was read by the
			// av_read_frame after demux_datasource flushes its buffer.
			// The flushing action sets the value of m_clip_begin_changed to indicate
			// the seek opperation and the temporary obsolete of the packets read from
			// ffmpeg_demux until it checks m_clip_begin_checked again before its
			// push_data.
			// If ffmpeg_demux doesn't check the latest value of
			// m_clip_begin_changed, which is changed by demux_datasource when
			// ffmpeg_demux is sleeping, and pushs data to demux_datasource will cause
			// the problem. This problem is found by Bo on linux and fixed by
			// Jack and Bo.
			while ( ! accepted && sink && !exit_requested() && !m_clip_begin_changed) {
				m_current_sink = sink;

				AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: calling %d.push_data(%lld, 0x%x, %d, %d) pts=%lld", pkt->stream_index, pkt->pts, pkt->data, pkt->size, pkt->duration, pts);
				m_lock.leave();
				accepted = sink->push_data((timestamp_t)pts, pkt->data, (size_t)pkt->size);
				if ( ! accepted) {
					// wait until space available in sink
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: waiting for buffer space for stream %d", pkt->stream_index);
					// sleep 10 millisec, hardly noticeable
					ambulant::lib::sleep_msec(10); // XXXX should be woken by readdone()
				}
				m_lock.enter();
				// Check whether our sink should have been deleted while we were outside of the lock.
				if (m_sinks[pkt->stream_index] == NULL)
				{
					sink->push_data(0,0,0);
					sink->release();
				}
				m_current_sink = NULL;
				sink = m_sinks[pkt->stream_index];
			}
		}
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: freeing pkt (number %d)",pkt_nr);
		av_free_packet(pkt);
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: final push_data(0, 0)");
	int i;
	m_lock.leave();
	for (i=0; i<MAX_STREAMS; i++) {
		if (m_sinks[i]) {
			m_sinks[i]->push_data(0, 0, 0);
			m_sinks[i]->release();
			m_sinks[i] = NULL;
		}
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_parser::run: returning");
	return 0;
}

AVCodecContext *
ambulant::net::ffmpeg_alloc_partial_codec_context(bool video, const char *name)
{
	ffmpeg_codec_id* codecid = ffmpeg_codec_id::instance();
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 8, 0)
	AVCodecContext *ffcontext = avcodec_alloc_context();
#else
	AVCodecContext *ffcontext = avcodec_alloc_context3(NULL);
#endif
	if (video) {
		ffcontext->codec_type = CODEC_TYPE_VIDEO;
	} else {
		ffcontext->codec_type = CODEC_TYPE_AUDIO;
	}
	ffcontext->codec_id = codecid->get_codec_id(name);
	return ffcontext;
}

lib::critical_section s_lock;
lib::critical_section*
ambulant::net::ffmpeg_global_critical_section()
{
	return &s_lock;
}
