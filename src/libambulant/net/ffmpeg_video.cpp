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

#include "ambulant/net/ffmpeg_video.h" 
#include "ambulant/net/ffmpeg_audio.h" 
#include "ambulant/net/ffmpeg_common.h" 
#include "ambulant/net/ffmpeg_factory.h" 
#include "ambulant/net/demux_datasource.h" 
#include "ambulant/lib/logger.h"
#include "ambulant/net/url.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif 

#if FFMPEG_VERSION_INT >= 0x000409
	#define WITH_FFMPEG_0_4_9					
#endif

// How many video frames we would like to buffer at most
#define MAX_VIDEO_FRAMES 300

using namespace ambulant;
using namespace net;

typedef lib::no_arg_callback<ffmpeg_video_decoder_datasource> framedone_callback;

video_datasource_factory *
ambulant::net::get_ffmpeg_video_datasource_factory()
{
	static video_datasource_factory *s_factory;
	
	if (!s_factory) s_factory = new ffmpeg_video_datasource_factory();
	return s_factory;
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
	ffmpeg_demux *thread = new ffmpeg_demux(context, clip_begin, clip_end);
	
	// Now, we can check that there is actually video in the file.
	if (thread->video_stream_nr() < 0) {
		thread->cancel();
		lib::logger::get_logger()->trace("ffmpeg: No video stream in %s", repr(url).c_str());
		return NULL;
	}
	
	// Next, if there is video we check that we can decode this type of video
	// stream.
	video_format fmt = thread->get_video_format();
	if (!ffmpeg_video_decoder_datasource::supported(fmt)) {
		thread->cancel();
		lib::logger::get_logger()->trace("ffmpeg: Unsupported video stream in %s", repr(url).c_str());
		return NULL;
	}
	
	// All seems well. Create the demux reader and the decoder.
	video_datasource *ds = demux_video_datasource::new_demux_video_datasource(url, thread);
	if (!ds) {
		lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: could not allocate ffmpeg_demux_video_datasource");
		thread->cancel();
		return NULL;
	}
	video_datasource *dds =  new ffmpeg_video_decoder_datasource(ds, fmt);
	if (dds == NULL) {
		lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}

	// Finally, tell the demux datasource to skip ahead to clipBegin, if
	// it can do so. No harm done if it can't: the decoder will then skip
	// any unneeded frames.
	ds->read_ahead(clip_begin);
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource (ds = 0x%x)", (void*) ds);
	return dds;		
}
		
// **************************** ffmpeg_video_decoder_datasource ********************
bool
ffmpeg_video_decoder_datasource::supported(const video_format& fmt)
{
	if (fmt.name != "ffmpeg") return false;
	AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
	if (enc->codec_type != CODEC_TYPE_VIDEO) return false;
	if (avcodec_find_decoder(enc->codec_id) == NULL) return false;
	return true;
}

ffmpeg_video_decoder_datasource::ffmpeg_video_decoder_datasource(video_datasource* src, video_format fmt)
:	m_src(src),
	m_con(NULL),
	m_event_processor(NULL),
	m_client_callback(NULL),
	m_pts_last_frame(0),
	m_last_p_pts(0),
	m_video_clock(src->get_clip_begin()),
	m_frame_count(0),
	m_elapsed(0)
{	
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::ffmpeg_video_decoder_datasource() (this = 0x%x)", (void*)this);
	
	ffmpeg_init();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource: Looking for %s(0x%x) decoder", fmt.name.c_str(), fmt.parameters);
	if (!_select_decoder(fmt))
		lib::logger::get_logger()->error(gettext("ffmpeg_video_decoder_datasource: could not select %s(0x%x) decoder"), fmt.name.c_str(), fmt.parameters);
	m_fmt = fmt;
	m_old_frame = ts_pointer_pair(0, NULL);
}

ffmpeg_video_decoder_datasource::~ffmpeg_video_decoder_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::~ffmpeg_video_decoder_datasource(0x%x)", (void*)this);
	stop();
	if (m_src) delete m_src;
	m_src = 0;
}

void
ffmpeg_video_decoder_datasource::stop()
{
	m_lock.enter();
	if (m_src) {
		m_src->stop();
		int rem = m_src->release();
		if (rem) lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop(0x%x): m_src refcount=%d", (void*)this, rem); 
	}
	m_src = NULL;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop(0x%x)", (void*)this);
	m_con = NULL;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop: thread stopped");
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
		lib::logger::get_logger()->error("ffmpeg_video_decoder_datasource::start(): m_client_callback already set!");
	}
	if (m_frames.size() > 0 /* XXXX Check timestamp! */ || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			assert(evp);
			lib::timer::time_type timestamp_milli = timestamp/1000; // micro to milli
			lib::timer::time_type now_milli = evp->get_timer()->elapsed();
			lib::timer::time_type delta_milli = 0;
			if (now_milli < timestamp_milli)
				delta_milli = timestamp_milli - now_milli;
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start: trigger client callback timestamp=%d delta_milli=%d, now_milli=%d", (int)timestamp, (int)delta_milli, (int)now_milli);
			evp->add_event(callbackk, delta_milli, ambulant::lib::event_processor::high);
		} else {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame() Calling m_src->start_frame(..)");
		lib::event *e = new framedone_callback(this, &ffmpeg_video_decoder_datasource::data_avail);
		assert(m_src);
		m_src->start_frame(evp, e, 0);
	}
	m_lock.leave();
}

void
print_frames(sorted_frames frames) {
  	sorted_frames f(frames);
	while (f.size()) {
		ts_pointer_pair e = f.top();
		printf("e.first=%d ", (int) e.first);
		f.pop();
	}
	printf("\n");
	return;
}

ts_pointer_pair 
ffmpeg_video_decoder_datasource::_pop_top_frame() {
	// pop a frame, return the new top frame
	// the old top frame is remembered in m_old_frame
	// old data in m_old_frame is freed.
  
	if (m_old_frame.second) {
		free (m_old_frame.second);
		m_old_frame.second = NULL;
	}
	if (m_frames.empty())
		return m_old_frame;

	m_old_frame = m_frames.top();
	m_frames.pop();
	return m_frames.top();
}

void 
ffmpeg_video_decoder_datasource::frame_done(timestamp_t now, bool keepdata)
{
	m_lock.enter();
	if (m_frames.size() == 0) {
		m_lock.leave();
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.frame_done(%d)", (int)now);

	while ( m_frames.size() && m_old_frame.first < now) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_done: discarding m_old_frame timestamp=%d, now=%d, data ptr = 0x%x",(int)m_old_frame.first,(int)now, m_old_frame.second);
		_pop_top_frame();
	}
	if (!keepdata) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_done(%d): free(0x%x)", (int)now, m_old_frame.second);
		free(m_old_frame.second);
		m_old_frame.second = NULL;
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
	if (m_fmt.frameduration == 0) {
		// And convert the timestamp
		timestamp_t framerate = m_con->frame_rate;
		timestamp_t framebase = m_con->frame_rate_base;
		timestamp_t frameduration = (framebase*1000000)/framerate;
		m_fmt.frameduration = frameduration;
	}
}

void
ffmpeg_video_decoder_datasource::read_ahead(timestamp_t clip_begin)
{
	assert(m_src);
	m_src->read_ahead(clip_begin);
}

void 
ffmpeg_video_decoder_datasource::data_avail()
{
	m_lock.enter();
	int got_pic;
	AVFrame *frame = avcodec_alloc_frame();
	AVPicture picture;
	int len, dummy2;
	int pic_fmt, dst_pic_fmt;
	int w,h;
	unsigned char* ptr;
	
	timestamp_t ipts;
	uint8_t *inbuf;
	int sz;
	got_pic = 0;
	
	if (!m_src) {
		// Cleaning up, apparently
		m_lock.leave();
		return;
	}
	
	inbuf = (uint8_t*) m_src->get_frame(0, &ipts, &sz);
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: %d bytes available", sz);
	if(sz == 0 && !m_src->end_of_file() ) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: no data, not eof?");
		m_lock.leave();
		return;
	}
	while(inbuf && sz && m_con) {	
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:start decoding (0x%x) ", m_con);
		assert(&m_con != NULL);
		assert(inbuf);
		assert(sz < 1000000); // XXXX This is soft, and probably incorrect. Remove when it fails.
		ptr = inbuf;
		
		while (sz > 0) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: decoding picture(s),  %d byteas of data ", sz);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: m_con: 0x%x, gotpic = %d, sz = %d ", m_con, got_pic, sz);
			len = avcodec_decode_video(m_con, frame, &got_pic, ptr, sz);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: avcodec_decode_video: used %d of %d bytes, gotpic = %d, ipts = %lld", len, sz, got_pic, ipts);
#ifdef WITH_FFMPEG_0_4_9
            // I'm not sure wheter this is a hack or not.
            // Some codecs (notably Dirac) always gobble up all bytes,
            // and only return len==sz if got_pic is true.
            len = sz;
#endif
			if (len >= 0) {
				assert(len <= sz);
				ptr +=len;	
				sz  -= len;
				if (got_pic) {
					AM_DBG lib::logger::get_logger()->debug("pts seems to be : %lld",ipts);
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: decoded picture, used %d bytes, %d left", len, sz);
					// Setup the AVPicture for the format we want, plus the data pointer
					_need_fmt_uptodate();
					w = m_fmt.width;
					h = m_fmt.height;
					m_size = w * h * 4;
					assert(m_size);
					char *framedata = (char*) malloc(m_size);
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:framedata=0x%x", framedata);
					assert(framedata != NULL);
					dst_pic_fmt = PIX_FMT_RGBA32;
					dummy2 = avpicture_fill(&picture, (uint8_t*) framedata, dst_pic_fmt, w, h);
					// The format we have is already in frame. Convert.
					pic_fmt = m_con->pix_fmt;
					img_convert(&picture, dst_pic_fmt, (AVPicture*) frame, pic_fmt, w, h);
					
					// Try and compute the timestamp and update the video clock.
					timestamp_t pts = 0;
					if (ipts != AV_NOPTS_VALUE) pts = ipts;
					
					if (m_con->has_b_frames && frame->pict_type != FF_B_TYPE) {
						pts = m_last_p_pts;
						m_last_p_pts = ipts;
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:frame has B frames but this frame is no B frame  (this=0x%x) ", this);
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:pts set to %f, remember %f", pts, m_last_p_pts);
					}
					
					if (pts != 0)
						m_video_clock = pts;
					else
						pts = m_video_clock;
					timestamp_t frame_delay = m_fmt.frameduration;
					if (frame->repeat_pict)
						frame_delay += (timestamp_t)(frame->repeat_pict*m_fmt.frameduration*0.5);
					m_video_clock += frame_delay;
					AM_DBG lib::logger::get_logger()->debug("videoclock: ipts=%lld pts=%lld video_clock=%lld", ipts, pts, m_video_clock);
					// Stupid HAck to get the pts right, we will have to look again to this later
					//pts = m_fmt.frameduration*m_frame_count;
					// And store the data.
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: storing frame with pts = %lld",pts );
					m_frame_count++;
					std::pair<timestamp_t, char*> element(pts, framedata);
					m_frames.push(element);
					m_elapsed = pts;
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
	// Now tell our client, if we have data available or are at end of file.
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): m_frames.size() returns %d, (eof=%d)", m_frames.size(), m_src->end_of_file());
	if ( m_frames.size() || m_src->end_of_file()) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): there is some data for the renderer ! (eof=%d)", m_src->end_of_file());
		if ( m_client_callback ) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): calling client callback (eof=%d)", m_src->end_of_file());
			assert(m_event_processor);
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::high);
			m_client_callback = NULL;
			//m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): No client callback!");
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
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_end_of_file() returning false (still %d frames in local buffer)", m_frames.size());
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
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_buffer_full() (this=0x%x, count=%d)", (void*) this, m_frames.size());
	bool rv = (m_frames.size() > MAX_VIDEO_FRAMES);
	return rv;
}	


char* 
ffmpeg_video_decoder_datasource::get_frame(timestamp_t now, timestamp_t *timestamp_p, int *size_p)
{
	// pop frames until (just before) "now". Then return the last frame popped.
	m_lock.enter();
	assert(now >= 0);
	timestamp_t frame_duration = 33000; // XXX For now assume 30fps
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame(now=%d)\n", (int) now);

	while ( m_frames.size() && m_old_frame.first < now - frame_duration) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: discarding m_old_frame timestamp=%d, now=%d, data ptr = 0x%x",(int)m_old_frame.first,(int)now, m_old_frame.second);
		_pop_top_frame();
	}
	AM_DBG if (m_frames.size()) lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: next timestamp=%lld, now=%lld", m_frames.top().first, now);
	assert(m_frames.size() == 0 || m_frames.top().first >= now-frame_duration);
	 
	if (timestamp_p) *timestamp_p = m_old_frame.first;
	if (size_p) *size_p = m_size;
	char *rv = m_old_frame.second;
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
	// private method - no need to lock
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
		//m_con->width=176;
		//m_con->height=128;
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
		
		m_con->codec_type = CODEC_TYPE_VIDEO;
		return true;
	}
	// Could add support here for raw mp3, etc.
	return false;
}
