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

#include <math.h>
#include <map>

#include "ambulant/net/ffmpeg_video.h"
#include "ambulant/net/ffmpeg_audio.h"
#include "ambulant/net/ffmpeg_common.h"
#include "ambulant/net/ffmpeg_factory.h"
#include "ambulant/net/demux_datasource.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/asb.h"
#include "ambulant/net/url.h"
#define round(x) ((int)((x)+0.5))
#ifndef INT64_C
#define INT64_C(x) x##LL
#endif

// WARNING: turning on AM_DBG globally for the ffmpeg code seems to trigger
// a condition that makes the whole player hang or collapse. So you probably
// shouldn't do it:-)
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// How many video frames we would like to buffer at least. This number should
// not be too low, otherwise a fast consumer will see only I and P frames
// because these are produced before the B frames.
// On second thoughts this seems a bad idea, so setting MIN_VIDEO_FRAMES to zero.
#define MIN_VIDEO_FRAMES 0
// How many video frames we would like to buffer at most.
// #define MAX_VIDEO_FRAMES 100 
// xxxbo: decrease the buffer size for the memory limited device, such as IPad, etc. 
#define MAX_VIDEO_FRAMES 10 

// How scaling of images is done.
// XXXJACK picked this scalerr because of "FAST" in the name. So there may be a better choice...
#define SWSCALE_FLAGS SWS_FAST_BILINEAR

using namespace ambulant;
using namespace net;

typedef lib::no_arg_callback<ffmpeg_video_decoder_datasource> framedone_callback;


video_datasource_factory *
ambulant::net::get_ffmpeg_video_datasource_factory()
{
#if 0
	// It seems datasource factories are sometimes cleaned up, hence we cannot use
	// a singleton. Need to fix/document at some point.
	static video_datasource_factory *s_factory;

	if (!s_factory) s_factory = new ffmpeg_video_datasource_factory();
	return s_factory;
#else
	return new ffmpeg_video_datasource_factory();
#endif
}


video_datasource*
ffmpeg_video_datasource_factory::new_video_datasource(const net::url& url, timestamp_t clip_begin, timestamp_t clip_end)
{

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource(%s)", repr(url).c_str());

	// First we check that the file format is supported by the file reader.
	// If it is we create a file reader (demux head end).
	AVFormatContext *context = ffmpeg_demux::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->trace("ffmpeg: no support for %s", repr(url).c_str());
		return NULL;
	}


	ffmpeg_demux *thread = new ffmpeg_demux(context, url, clip_begin, clip_end);

	// Now, we can check that there is actually video in the file.
	if (thread->video_stream_nr() < 0) {
		thread->cancel();
		lib::logger::get_logger()->trace("ffmpeg: No video stream in %s", repr(url).c_str());
		return NULL;
	}

	// Next, if there is video we check that we can decode this type of video
	// stream.
	video_format fmt = thread->get_video_format();
	//fmt.parameters = (void*) context;
	AVCodecContext *enc = (AVCodecContext *)fmt.parameters;


	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: Stream type %d, codec_id %d", enc->codec_type, enc->codec_id);


	if (!ffmpeg_video_decoder_datasource::supported(fmt)) {
		thread->cancel();
		lib::logger::get_logger()->trace("ffmpeg_video_datasource_factory::new_video_datasource: Unsupported video stream in %s", repr(url).c_str());
		return NULL;
	}

	// All seems well. Create the demux reader and the decoder.
	demux_video_datasource *ds = demux_video_datasource::new_demux_video_datasource(url, thread);
	if (!ds) {
		lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: could not allocate ffmpeg_demux_video_datasource");
		thread->cancel();
		return NULL;
	}
	video_datasource *dds =	 new ffmpeg_video_decoder_datasource(ds, fmt, url);
	if (dds == NULL) {
		lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}

	// Finally, tell the demux datasource to skip ahead to clipBegin, if
	// it can do so. No harm done if it can't: the decoder will then skip
	// any unneeded frames.
	ds->read_ahead(clip_begin);

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource (dds = %p)", (void*) dds);
	return dds;
}

// **************************** ffmpeg_video_decoder_datasource ********************


bool
ffmpeg_video_decoder_datasource::supported(const video_format& fmt)
{
	assert(fmt.name == "ffmpeg");
	AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
	if (enc->codec_type != AVMEDIA_TYPE_VIDEO) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::supported: not a video stream !(%d, %d)", enc->codec_type, AVMEDIA_TYPE_VIDEO);
		return false;
	}
	if (avcodec_find_decoder(enc->codec_id) == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::supported: cannot open video codec (codec_id: %d)", enc->codec_id);

		return false;
	}
	return true;
}

ffmpeg_video_decoder_datasource::ffmpeg_video_decoder_datasource(demux_video_datasource* src, video_format fmt, net::url url)
:
	m_event_processor(NULL),
	m_client_callback(NULL),

	m_src(src),
    m_url(url),
	m_con(NULL),
	m_con_owned(false),
	m_img_convert_ctx(NULL),

	m_oldest_timestamp_wanted(0),

	m_start_input(true),
    m_complete_frame_seen(false),
    m_is_live(false),

	m_size(0),
	m_pixel_layout(pixel_unknown),

	m_frame_count(0),
	m_dropped_count(0)
{

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::ffmpeg_video_decoder_datasource() (this = %p)", (void*)this);
	ffmpeg_init();
    const std::string& frag = m_url.get_ref();
    if (frag.find("is_live=1") != std::string::npos) {
        AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource: is_live=1 specified on URL");
        m_is_live = true;
    }
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource: Looking for %s(%p) decoder", fmt.name.c_str(), fmt.parameters);
	if (!_select_decoder(fmt))
		lib::logger::get_logger()->error(gettext("ffmpeg_video_decoder_datasource: could not select %s(%p) decoder"), fmt.name.c_str(), fmt.parameters);
	m_fmt = fmt;
}

ffmpeg_video_decoder_datasource::~ffmpeg_video_decoder_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::~ffmpeg_video_decoder_datasource(%p)", (void*)this);
	stop();
	if (m_img_convert_ctx) sws_freeContext(m_img_convert_ctx);
	if (m_dropped_count) lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource: dropped %d of %d frames after decoding", m_dropped_count, m_frame_count);
}

void
ffmpeg_video_decoder_datasource::set_is_live (bool is_live)
{
    m_lock.enter();
    m_src->set_is_live(is_live);
    m_is_live = is_live;
    // XXXJACK: I think this is too late: the flag should be set before calling codec_open...
    if (is_live && m_con) {
        m_con->flags |= CODEC_FLAG_LOW_DELAY;
        m_con->has_b_frames = 0;
    }
    m_lock.leave();
}

void
ffmpeg_video_decoder_datasource::stop()
{
	m_lock.enter();
	if (m_src) {
		m_src->stop();
		long rem = m_src->release();
		if (rem) lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop(%p): m_src refcount=%d", (void*)this, rem);
	}
	m_src = NULL;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop(%p)", (void*)this);
	if (m_con && m_con_owned) {
		lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
		ffmpeg_lock->enter();
		avcodec_close(m_con);
		ffmpeg_lock->leave();
		av_free(m_con);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop: avcodec_close(m_con=%p) called", m_con);
	}
	m_con = NULL;
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	// And delete any frames left
	while ( ! m_frames.empty() ) {
		_pop_top_frame();
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
ffmpeg_video_decoder_datasource::get_audio_datasource(audio_format_choices fmts)
{
	m_lock.enter();
	audio_datasource *rv = NULL;
	if (m_src) rv = m_src->get_audio_datasource(fmts);
	m_lock.leave();
	return rv;
}

void
ffmpeg_video_decoder_datasource::start_frame(ambulant::lib::event_processor *evp,
	ambulant::lib::event *callbackk, timestamp_t timestamp)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: (this = %p)", (void*) this);

	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: m_client_callback already set!");
	}

	if (m_frames.size() > 0 || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			if (timestamp < 0) timestamp = 0;
			lib::timer::time_type timestamp_milli = (lib::timer::time_type)(timestamp/1000); // micro to milli
			lib::timer::time_type now_milli = evp->get_timer()->elapsed();
			lib::timer::time_type delta_milli = 0;
			if (now_milli < timestamp_milli)
				delta_milli = timestamp_milli - now_milli;
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: %p: trigger client callback timestamp_milli=%d delta_milli=%d, now_milli=%d, %d frames in buffer", this, (int)timestamp_milli, (int)delta_milli, (int)now_milli, m_frames.size());
			// Sanity check: we don't want this to be more than a second into the future
			if (delta_milli > 1000 && ! m_src->get_is_live()) {
				lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: frame is %f seconds in the future", delta_milli / 1000.0);
				lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: elapsed()=%dms, timestamp=%dms", now_milli, timestamp_milli);
			}
			evp->add_event(callbackk, delta_milli+1, ambulant::lib::ep_high);
		} else {
			lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: Internal error: no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		}
	} else	{
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: this=%p: record callback for later", this);
		m_client_callback = callbackk;
		m_event_processor = evp;
	}

	if (m_start_input) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: Calling m_src->start_frame(..)");
		lib::event *e = new framedone_callback(this, &ffmpeg_video_decoder_datasource::data_avail);
		assert(m_src);
		m_src->start(evp, e);
		m_start_input = false;
	}
	m_lock.leave();
}

void
ffmpeg_video_decoder_datasource::start_prefetch(ambulant::lib::event_processor *evp)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_prefetch: (this = %p)", (void*) this);

	m_event_processor = evp;

	if (m_start_input) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_prefetch: Calling m_src->start_frame(..)");
		lib::event *e = new framedone_callback(this, &ffmpeg_video_decoder_datasource::data_avail);
		assert(m_src);
		m_src->start(evp, e);
		m_start_input = false;
	}
	m_lock.leave();
}

void
ffmpeg_video_decoder_datasource::_pop_top_frame() {
	if (m_frames.empty()) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._pop_top_frame: no frames? (programmer error)");
		return;
	}

	char *data = m_frames.front().second;
	assert(data);
	if (data) free(data);
	m_frames.pop();
}

void
ffmpeg_video_decoder_datasource::frame_processed_keepdata(timestamp_t now, char *buf)
{
	m_lock.enter();

	AM_DBG lib::logger::get_logger()->trace("ffmpeg_video_decoder_datasource::frame_processed_keepdata: now=%lld, m_oldest_timestamp_wanted = %lld", now, m_oldest_timestamp_wanted);
	m_oldest_timestamp_wanted = now;

	assert(m_frames.size() > 0);
	assert(m_frames.front().first == now);
	assert(m_frames.front().second == buf);
	m_frames.pop();
	m_lock.leave();
}

void
ffmpeg_video_decoder_datasource::frame_processed(timestamp_t now)
{
	m_lock.enter();

	AM_DBG lib::logger::get_logger()->trace("ffmpeg_video_decoder_datasource::frame_processed: now=%lld, m_oldest_timestamp_wanted = %lld", now, m_oldest_timestamp_wanted);
	m_oldest_timestamp_wanted = now;


	if (m_frames.size() == 0) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_processed: frame queue empty");
		m_lock.leave();
		return;
	}

	while ( m_frames.size() > 0 && m_frames.front().first <= now) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_processed: discarding first frame timestamp=%d, now=%d, data ptr = %p",(int)m_frames.front().first,(int)now, m_frames.front().second);
		_pop_top_frame();
	}
	m_lock.leave();
}

int
ffmpeg_video_decoder_datasource::width()
{
	m_lock.enter();
	_need_fmt_uptodate();
	m_lock.leave();
	return m_fmt.width;
}

int
ffmpeg_video_decoder_datasource::height()
{
	m_lock.enter();
	_need_fmt_uptodate();
	m_lock.leave();
	return m_fmt.height;
}

timestamp_t
ffmpeg_video_decoder_datasource::frameduration()
{
	timestamp_t rv;
	m_lock.enter();
	if(m_fmt.frameduration <=0)
		_need_fmt_uptodate();
	rv = m_fmt.frameduration;
	m_lock.leave();
	// Clamp the frame duration to reasonable values.
	if (rv < 1000) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frameduration: frameduration %d increased to 1000 uS", rv);
		rv = 1000;
	}
	return rv;
}
void
ffmpeg_video_decoder_datasource::_need_fmt_uptodate()
{
	// Private method: no locking
	if (m_fmt.height == 0) {
		m_fmt.height = m_con->height;
	}
	if (m_fmt.width == 0) {
		m_fmt.width = m_con->width;
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_need_fmt_uptodate: frameduration = %lld", m_fmt. frameduration);
}

void
ffmpeg_video_decoder_datasource::read_ahead(timestamp_t clip_begin)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::read_ahead: clip_begin=%d", clip_begin);
	assert(m_src);
	m_src->read_ahead(clip_begin);
	m_oldest_timestamp_wanted = clip_begin;
	seek (clip_begin);
}

void
ffmpeg_video_decoder_datasource::seek(timestamp_t time)
{
	m_lock.enter();
	int nframes_dropped = 0;
	assert( time >= 0);

	//xxxbo: For the node which is already prefetched in advance,
	//       there is no need to call m_src->seek.
	if (m_oldest_timestamp_wanted == time) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::seek: m_oldest_timestamp_wanted = %lld, time = %lld", m_oldest_timestamp_wanted, time);
		m_lock.leave();
		return;
	}

	m_oldest_timestamp_wanted = time;

	// Do the seek before flush
	if (m_src) m_src->seek(time);

	while ( m_frames.size() > 0) {
		_pop_top_frame();
		nframes_dropped++;
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::seek(%d): flushed %d frames", time, nframes_dropped);
	m_lock.leave();
}

void
ffmpeg_video_decoder_datasource::set_clip_end(timestamp_t clip_end)
{
	m_lock.enter();
	if (m_src) m_src->set_clip_end(clip_end);
	m_lock.leave();
}

void
ffmpeg_video_decoder_datasource::data_avail()
{
	m_lock.enter();
	int got_pic;
	int len;

	timestamp_t ipts = 0;	// Timestamp of the incoming packet
	timestamp_t pts = 0;	// Timestamp of outgoing frame
	got_pic = 0;

	if (!m_src) {
		// Cleaning up, apparently
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: no m_src, bailing out");
		m_lock.leave();
		return;
	}
	if (m_con == NULL) {
        lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: m_con==NULL, dropping packet");
		_restart_queues();
		m_lock.leave();
		return;
    }

	// Get the input data
	datasource_packet dspacket = m_src->get_packet(); // xxxjack get_frame(0, &ipts, &sz);
	ipts = dspacket.pts;
	AVPacket *avpkt = dspacket.pkt;
#ifdef LOGGER_VIDEOLATENCY
    lib::logger::get_logger(LOGGER_VIDEOLATENCY)->trace("videolatency 3-predecode %lld %lld %s", 0LL, ipts, m_url.get_url().c_str());
#endif

    // Check for end-of-file inconsistency
	if(dspacket.flag == datasource_packet_flag_eof) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: datasource_packet_flag_eof received, bailing out");
		if (!m_src->end_of_file() ) {
			lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: datasource_packet_flag_eof, but not eof?");
			// Unsure whether we have to restart input in this case...
			m_start_input = true;
		}
		// XXXJACK Or should we pass dummy packets to flush the buffer?
		_restart_queues();
#if 1
        // Added by Jack, because hitting end-of-file and seeking backwards doesn't
        // restart input...
        m_start_input = true;
#endif
		m_lock.leave();
		return;
	}

	// Now that we have gotten this callback we need to restart input at some point.
	m_start_input = true;
    
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: start decoding (%p) ", m_con);

	if (dspacket.flag == datasource_packet_flag_avpacket) {
		if (avpkt == NULL) {
			lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: pkt=NULL for flag=datasource_packet_flag_avpacket, pts=%lld", ipts);
			m_lock.leave();
			return;
		}
	} else if (dspacket.flag == datasource_packet_flag_flush) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: flush buffers");
		avcodec_flush_buffers(m_con);
		_restart_queues();
		m_lock.leave();
		return;
	} else if (dspacket.flag == datasource_packet_flag_nodata) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: flag=datasource_packet_flag_nodata");
		m_lock.leave();
		return;
	} else {
		assert(0);
	}

	// Allocate and initialize the frame, which will receive the decoded output.
	AVFrame *frame = avcodec_alloc_frame();
	if (frame == NULL) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: avcodec_alloc_frame() failed");
		lib::logger::get_logger()->error(gettext("Out of memory playing video"));
		m_src->stop();
		_restart_queues();
		m_lock.leave();
		return;
	}
	avcodec_get_frame_defaults(frame);

	// We use skip_frame to make the decoder run faster in case we
	// are not interested in the data (still seeking forward, or live stream with no i-frame seen yet).
	if (_should_fast_forward(ipts)) {
		m_con->skip_frame = AVDISCARD_NONREF;
	} else {
		m_con->skip_frame = AVDISCARD_DEFAULT;
	}
	
#if 0
	// For live streams we also skip frames if we have too much data in our input or output buffer.
	// NOTE: this may not be a good idea if there is an audio stream attached to this video stream...
    if (m_is_live) {
        // Give a warning if there is too much data in the input or output queue
        int inSize = m_src->size();
        int outSize = size();
        if (inSize > 1 || outSize > 1) {
            lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: backlog: %d packets before decoder, %d frames after decoder", inSize, outSize);
            m_con->skip_frame = AVDISCARD_NONREF;
        }
    }
#endif

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: pts=%lld, len=%d, flags=0x%x data[4,5]=0x%x 0x%x, side_data=%p:%d", avpkt->pts, avpkt->size, avpkt->flags, avpkt->data[4], avpkt->data[5], avpkt->side_data, avpkt->side_data_elems);
	// Let's decode the packet, and see whether we get a frame back.
	assert(avpkt);
	got_pic = 0;
	len = avcodec_decode_video2(m_con, frame, &got_pic, avpkt);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: avcodec_decode_video: gotpic = %d, ipts = %lld, packet_pts=%lld, packet_dts=%lld", got_pic, ipts, frame->pkt_pts, frame->pkt_dts);

	if (len < 0) {
		lib::logger::get_logger()->trace(gettext("ffmpeg_video_decoder_datasource.data_avail: error decoding video packet (timestamp=%lld)"), ipts);
		_restart_queues();
		m_lock.leave();
		return;
	}
	if (!got_pic) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: no frame produced for this packet");
		_restart_queues();
		m_lock.leave();
		return;
	}

	// We have gotten a frame. Check whether it's the first complete frame.
	if (!m_complete_frame_seen) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: found full frame");
	}
	m_complete_frame_seen = true;

	// At this point we need m_fmt to be correct, we are going to use
	// sizes, durations, etc.
	_need_fmt_uptodate();
	
    // avpkt->duration has been scaled to the Ambulant timebase in ffmpeg_demux::run()
	timestamp_t frame_delay = avpkt->duration;
	if (m_fmt.frameduration <= 0) {
        AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: frame_duration set to to %lld", frame_delay);
        
		m_fmt.frameduration = frame_delay;
	} else if (m_fmt.frameduration != frame_delay) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: frame_duration changed from %lld to %lld", m_fmt.frameduration, frame_delay);
		m_fmt.frameduration = frame_delay;
	}

    // Compute timestamp of this frame, in case it is missing, and updated the
    // video clock so we can compute future missing timestamps.

	pts = av_frame_get_best_effort_timestamp(frame);
#ifdef LOGGER_VIDEOLATENCY
	lib::logger::get_logger(LOGGER_VIDEOLATENCY)->trace("videolatency 4-decoded %lld %lld %s", 0LL, pts, m_url.get_url().c_str());
#endif
	if (frame->repeat_pict)
		frame_delay += (timestamp_t)(frame->repeat_pict*m_fmt.frameduration*0.5);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: ipts=%lld pts=%lld frame_delay=%lld", ipts, pts, frame_delay);
	m_frame_count++;

	// In some special cases we want to drop the frame here, after decoding.
	// That only happens if displaying the frame would be bad for the user experience,
	// i.e. the frame comes before a frame already displayed or before
	// begin-of-clip.
	if (pts >= m_oldest_timestamp_wanted) {

		// Convert the frame and send it down the pipe.
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: storing frame with pts = %lld",pts );
		_forward_frame(pts, frame);
		
	} else {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: drop frame ipts=%lld, pts=%lld before oldest_timestamp_wanted %lld", ipts, pts, m_oldest_timestamp_wanted);
		m_dropped_count++;
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: done decoding (%p) ", m_con);
	// Free packet and frame
	av_free(frame);
	av_free_packet(avpkt);

	_restart_queues();
	m_lock.leave();
}

bool
ffmpeg_video_decoder_datasource::_should_fast_forward(timestamp_t ipts)
{
	bool ff = false;
	if (ipts != (int64_t)AV_NOPTS_VALUE && ipts < m_oldest_timestamp_wanted) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: setting hurry_up: ipts=%lld, m_oldest_timestamp_wanted=%lld (%lld us late)",ipts, m_oldest_timestamp_wanted, m_oldest_timestamp_wanted-ipts);
		ff = true;
	}
	if (m_is_live) {
		if (!m_complete_frame_seen) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: waiting for full frame");
			ff = true;
		}
	}
	return ff;
}

void
ffmpeg_video_decoder_datasource::_restart_queues()
{
	// Tell our client if we have data available or are at end of file.
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail: m_frames.size() returns %d, (eof=%d)", m_frames.size(), m_src->end_of_file());
	if (m_frames.size() > MIN_VIDEO_FRAMES || m_src->end_of_file()) {

		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail: there is some data for the renderer ! (eof=%d)", m_src->end_of_file());
		if ( m_client_callback ) {

			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail: calling client callback (eof=%d)", m_src->end_of_file());
			assert(m_event_processor);
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::ep_high);
			m_client_callback = NULL;
			//m_event_processor = NULL;
		} else {

			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail: No client callback!");
		}
	}

	// Restart input if there is buffer space and anything remains to be read. Otherwise we
	// leave m_start_input true, and restarting is taken care of in start_frame().
	if (!m_src->end_of_file() && !_buffer_full()) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail: Calling m_src->start(..)");
		lib::event *e = new framedone_callback(this, &ffmpeg_video_decoder_datasource::data_avail);
		m_src->start(m_event_processor, e);
		m_start_input = false;
	}
}

void
ffmpeg_video_decoder_datasource::_forward_frame(timestamp_t pts, AVFrame *frame)
{
	AVPicture picture;
	PixelFormat pic_fmt, dst_pic_fmt;
	int w,h;

    // Convert the frame to the image format we want.
    int bpp = 0;
    switch(m_pixel_layout) {
        case pixel_rgba:
            dst_pic_fmt = PIX_FMT_RGB32_1;
            bpp = 4;
            break;
        case pixel_bgra:
            dst_pic_fmt = PIX_FMT_BGR32_1;
            bpp = 4;
            break;
        case pixel_argb:
            dst_pic_fmt = PIX_FMT_RGB32;
            bpp = 4;
            break;
        case pixel_abgr:
            dst_pic_fmt = PIX_FMT_BGR32;
            bpp = 4;
            break;
#ifdef AV_PIX_FMT_NE
        case pixel_rgbx:
            dst_pic_fmt = AV_PIX_FMT_NE(RGB0, 0BGR);
            bpp = 4;
            break;
        case pixel_bgrx:
            dst_pic_fmt = AV_PIX_FMT_NE(BGR0, 0RGB);
            bpp = 4;
            break;
#endif//AV_PIX_FMT_NE
        case pixel_xrgb:
            dst_pic_fmt = PIX_FMT_0RGB32;
            bpp = 4;
            break;
        case pixel_xbgr:
            dst_pic_fmt = PIX_FMT_0BGR32;
            bpp = 4;
            break;
        case pixel_rgb:
            dst_pic_fmt = PIX_FMT_RGB24;
            bpp = 3;
            break;
        case pixel_bgr:
            dst_pic_fmt = PIX_FMT_BGR24;
            bpp = 3;
            break;
        default:
            assert(0);
    }
    w = m_fmt.width;
    h = m_fmt.height;
    m_size = w * h * bpp;
    assert(m_size);
    char *framedata = (char*) malloc(m_size);
    AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: framedata=%p", framedata);
    if (framedata == NULL) {
        lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: malloc(%d) failed", m_size);
        lib::logger::get_logger()->error(gettext("Out of memory playing video"));
        m_src->stop();
        return;
    }
    int datasize = avpicture_fill(&picture, (uint8_t*) framedata, dst_pic_fmt, w, h);
    assert(datasize == m_size);
    // The format we have is already in frame. Convert.
    pic_fmt = m_con->pix_fmt;
    m_img_convert_ctx = (struct SwsContext *) sws_getCachedContext(
       m_img_convert_ctx,
       w, h, pic_fmt,
       w, h, dst_pic_fmt,
       SWSCALE_FLAGS, NULL, NULL, NULL);
    assert(m_img_convert_ctx);
    
    sws_scale(m_img_convert_ctx, frame->data, frame->linesize, 0, h, picture.data, picture.linesize);
    
#ifdef LOGGER_VIDEOLATENCY
    lib::logger::get_logger(LOGGER_VIDEOLATENCY)->trace("videolatency 5-scaled %lld %lld %s", 0LL, pts, m_url.get_url().c_str());
#endif
    
    // Finally send the frame upstream.
    std::pair<timestamp_t, char*> element(pts, framedata);
    m_frames.push(element);
}

bool
ffmpeg_video_decoder_datasource::end_of_file()
{
	m_lock.enter();
	if (_clip_end()) {
		m_lock.leave();
		return true;
	}
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
ffmpeg_video_decoder_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_frames.size() > 0)  {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_end_of_file: returning false (still %d frames in local buffer)", m_frames.size());
		return false;
	}
	return m_src == NULL || m_src->end_of_file();
}

bool
ffmpeg_video_decoder_datasource::_clip_end()
{
	return false;
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
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_buffer_full: (this=%p, count=%d)", (void*) this, m_frames.size());
	bool rv = (m_frames.size() > MAX_VIDEO_FRAMES);
	return rv;
}


char*
ffmpeg_video_decoder_datasource::get_frame(timestamp_t now, timestamp_t *timestamp_p, size_t *size_p)
{
	// pop frames until (just before) "now". Then return the last frame popped.
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame(now=%lld)\n", now);
	// XXX now can be negative, due to time manipulation by the scheduler. assert(now >= 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: %d frames available\n", m_frames.size());
	//bo 18-03-2008, for some reasons, somtimes, m_frames.size() is 0;
	if (m_frames.size() == 0) {
		m_lock.leave();
		return NULL;
	}
	assert(m_frames.size() > 0 || _end_of_file());

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame:  timestamp=%lld, now=%lld, %d in queue",m_frames.front().first, now, m_frames.size());

	AM_DBG if (m_frames.size()) lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: next timestamp=%lld, now=%lld", m_frames.front().first, now);
	// The next assert assures that we have indeed removed all old frames (and, therefore, either there
	// are no frames left, or the first frame has a time that is in the future). It also assures that
	// the frames in m_frames are indeed in the right order.

	if (timestamp_p) *timestamp_p = m_frames.front().first;
	if (size_p) *size_p = m_size;
	char *rv = m_frames.front().second;
	if (rv == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: now=%lld, about to return NULL frame (ts=%lld), should not happen", now, *timestamp_p);
	}
	m_lock.leave();
	return rv;
}

common::duration
ffmpeg_video_decoder_datasource::get_dur()
{
	if( m_src == 0) return common::duration(0, true);
	return m_src->get_dur();
}

bool
ffmpeg_video_decoder_datasource::_select_decoder(const char* file_ext)
{
	// private method - no need to lock
	AVCodec *codec = avcodec_find_decoder_by_name(file_ext);
	if (codec == NULL) {
		lib::logger::get_logger()->trace("ffmpeg_video_decoder_datasource._select_decoder: Failed to find codec for \"%s\"", file_ext);
		lib::logger::get_logger()->error(gettext("No support for \"%s\" video"), file_ext);
		return false;
	}
	m_con = avcodec_alloc_context3(codec);
	m_con_owned = true;

	lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
	ffmpeg_lock->enter();
	  if(avcodec_open2(m_con,codec,NULL) < 0) {
		ffmpeg_lock->leave();
		lib::logger::get_logger()->trace("ffmpeg_video_decoder_datasource._select_decoder: Failed to open avcodec for \"%s\"", file_ext);
		lib::logger::get_logger()->error(gettext("No support for \"%s\" video"), file_ext);
		return false;
	}
	ffmpeg_lock->leave();
	return true;
}

bool
ffmpeg_video_decoder_datasource::_select_decoder(video_format &fmt)
{
	// private method - no need to lock
	assert(fmt.name == "ffmpeg");
	AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
	m_con = enc;
	m_con_owned = false;

	if (enc == NULL) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._select_decoder: Internal error: Parameters missing for %s(%p)", fmt.name.c_str(), fmt.parameters);
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		return false;
	}
	if (enc->codec_type != AVMEDIA_TYPE_VIDEO) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._select_decoder: Internal error: Non-video stream for %s(%d)", fmt.name.c_str(), enc->codec_type);
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		return false;
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._select_decoder: enc->codec_id = %d", enc->codec_id);
	AVCodec *codec = avcodec_find_decoder(enc->codec_id);

	if (codec == NULL) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._select_decoder: Internal error: Failed to find codec for %s(%d)", fmt.name.c_str(),  enc->codec_id);
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		return false;
	}
	//m_con = avcodec_alloc_context();
	if (m_is_live && m_con) {
		m_con->flags |= CODEC_FLAG_LOW_DELAY;
		m_con->has_b_frames = 0;
	}

	lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
	ffmpeg_lock->enter();
	  if(avcodec_open2(m_con,codec,NULL) < 0) {
		ffmpeg_lock->leave();
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._select_decoder: Internal error: Failed to open avcodec for %s(%d)", fmt.name.c_str(), enc->codec_id);
		return false;
	}
	ffmpeg_lock->leave();
	if (fmt.width == 0) fmt.width = m_con->width;
	if (fmt.height == 0) fmt.width = m_con->height;
	return true;
}
