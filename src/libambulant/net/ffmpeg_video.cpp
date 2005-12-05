// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

 
#include <math.h>
#include <map>

#include "ambulant/net/ffmpeg_video.h" 
#include "ambulant/net/ffmpeg_audio.h" 
#include "ambulant/net/ffmpeg_common.h" 
#include "ambulant/net/ffmpeg_factory.h" 
#include "ambulant/net/demux_datasource.h" 
#include "ambulant/lib/logger.h"
#include "ambulant/net/url.h"

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
#define MAX_VIDEO_FRAMES 30

// This construction is needed to get the CVS version of ffmpeg to work:
// AVStream.codec got changed from AVCodecContext to AVCodecContext*
#if LIBAVFORMAT_BUILD > 4628
	#define am_get_codec_var(codec,var) codec->var
	#define am_get_codec(codec) codec
#else
	#define am_get_codec_var(codec,var) codec.var
	#define am_get_codec(codec) &codec
#endif

using namespace ambulant;
using namespace net;

typedef lib::no_arg_callback<ffmpeg_video_decoder_datasource> framedone_callback;

video_datasource_factory *
ambulant::net::get_ffmpeg_video_datasource_factory()
{
#if 0
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
	//fmt.parameters = (void*) context;
	AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
	

	AM_DBG lib::logger::get_logger()->debug("ffmpeg: Stream type %d, codec_id %d", enc->codec_type, enc->codec_id);

   
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
	if (enc->codec_type != CODEC_TYPE_VIDEO) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::supported: not a video stream !(%d, %d)", enc->codec_type, CODEC_TYPE_VIDEO);
		return false;
	}
	if (avcodec_find_decoder(enc->codec_id) == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::supported cannot open video codec (codec_id: %d)", enc->codec_id);

		return false;
	}
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
	m_elapsed(0),
	m_start_input(true)
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
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start: trigger client callback timestamp_milli=%d delta_milli=%d, now_milli=%d", (int)timestamp_milli, (int)delta_milli, (int)now_milli);
			evp->add_event(callbackk, delta_milli+1, ambulant::lib::event_processor::high);
		} else {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
	}
	// Don't restart our source if we are at end of file.
	if ( _end_of_file() ) m_start_input = false;
	
	if (m_start_input) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame() Calling m_src->start_frame(..)");
		lib::event *e = new framedone_callback(this, &ffmpeg_video_decoder_datasource::data_avail);
		assert(m_src);
		m_src->start_frame(evp, e, 0);
		m_start_input = false;
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
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._pop_top_frame(): free'n m_old_frame.second");

	}
	
	if (m_frames.empty()) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._pop_top_frame():no more frames left returning m_old_frame");
		return m_old_frame;
	}

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
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_need_fmt_uptodate(): frameduration = %lld", m_fmt. frameduration);

	if (m_fmt.frameduration <= 0) {
		// And convert the timestamp
#if LIBAVFORMAT_BUILD <= 4623
		timestamp_t framerate = m_con->frame_rate;
		timestamp_t framebase = m_con->frame_rate_base;
		timestamp_t frameduration = (framebase*1000000)/framerate;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_need_fmt_uptodate(): frameduration = %lld", frameduration);
#else
		timestamp_t frameduration = (timestamp_t) round(m_con->time_base.num *1000000.0 / (double) m_con->time_base.den) ;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_need_fmt_uptodate(): frameduration = %lld, %d %d", frameduration, m_con->time_base.num, m_con->time_base.den);
#endif
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
	AVPicture picture;
	int len, dummy2;
	int pic_fmt, dst_pic_fmt;
	int w,h;
	unsigned char* ptr;
	
	timestamp_t ipts = 0;
	uint8_t *inbuf;
	int sz;
	got_pic = 0;
	
	if (!m_src) {
		// Cleaning up, apparently
		m_lock.leave();
		return;
	}
	
	// Now that we have gotten this callback we need to restart input at some point.
	m_start_input = true;
	
	// Get the input data
	inbuf = (uint8_t*) m_src->get_frame(0, &ipts, &sz);
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: %d bytes available", sz);
	if(sz == 0 && !m_src->end_of_file() ) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: no data, not eof?");
		m_lock.leave();
		return;
	}
	
	// No easy error conditions, so let's allocate our frame.
	AVFrame *frame = avcodec_alloc_frame();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail called (0x%x) ", (void*) this);
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
#if LIBAVFORMAT_BUILD > 4609
            // XXX Dirac hack, to be removed.
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
					timestamp_t frame_delay = 0;
				
#if LIBAVFORMAT_BUILD > 4906
				    pts = ipts;
					if (pts != 0) {
						m_video_clock = pts;
					} else {
						pts = m_video_clock;
					}
						frame_delay = m_fmt.frameduration;
						if (frame->repeat_pict)
							frame_delay += (timestamp_t)(frame->repeat_pict*m_fmt.frameduration*0.5);
						m_video_clock += frame_delay;
#else // ffmpeg 0.4.8 
					if (ipts != AV_NOPTS_VALUE) pts = ipts;
					if (m_con->has_b_frames && frame->pict_type != FF_B_TYPE) {
						pts = m_last_p_pts;
						m_last_p_pts = ipts;
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:frame has B frames but this frame is no B frame  (this=0x%x) ", this);
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:pts set to %f, remember %f", pts, m_last_p_pts);
					}
					if (pts != 0) {
						m_video_clock = pts;
					} else {
						pts = m_video_clock;
						frame_delay = m_fmt.frameduration;
						if (frame->repeat_pict)
							frame_delay += (timestamp_t)(frame->repeat_pict*m_fmt.frameduration*0.5);
						m_video_clock += frame_delay;
					}
#endif				
					AM_DBG lib::logger::get_logger()->debug("videoclock: ipts=%lld pts=%lld video_clock=%lld, frame_delay=%lld", ipts, pts, m_video_clock, frame_delay);
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: storing frame with pts = %lld",pts );
					m_frame_count++;
					if (pts >= m_old_frame.first) {
						std::pair<timestamp_t, char*> element(pts, framedata);
						m_frames.push(element);
					} else {
						// A frame that came after this frame has already been consumed.
						// We should drop this frame.
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder: dropping frame %d: too late", m_frame_count);
					}
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
	av_free(frame);
	// Now tell our client, if we have data available or are at end of file.
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): m_frames.size() returns %d, (eof=%d)", m_frames.size(), m_src->end_of_file());
	if ( m_frames.size() > MIN_VIDEO_FRAMES || m_src->end_of_file()) {
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
	// Restart input if there is buffer space and anything remains to be read. Otherwise we
	// leave m_start_input true, and restarting is taken care of in start_frame().
	if (!m_src->end_of_file() && !_buffer_full()) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame() Calling m_src->start_frame(..)");
		lib::event *e = new framedone_callback(this, &ffmpeg_video_decoder_datasource::data_avail);
		m_src->start_frame(m_event_processor, e, ipts);
		m_start_input = false;
	}

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
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame(now=%lld)\n", now);
	// XXX now can be negative, due to time manipulation by the scheduler. assert(now >= 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame() %d frames available\n", m_frames.size());
	assert(m_frames.size() > 0 || _end_of_file());
	if(m_fmt.frameduration  <= 0) {
		_need_fmt_uptodate();
	}
	
	timestamp_t frame_duration = m_fmt.frameduration; 
	assert (frame_duration > 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame:  timestamp=%lld, now=%lld, frameduration = %lld",m_old_frame.first,now, frame_duration);


	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame(now=%lld): %lld (m_old_frame.first) <  %lld (now - frame_duration)",  now, m_old_frame.first, now - frame_duration );

	while ( m_frames.size() && m_old_frame.first < now - frame_duration) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: discarding m_old_frame timestamp=%lld, now=%lld, data ptr = 0x%x",m_old_frame.first,now, m_old_frame.second);
		_pop_top_frame();
	}
	AM_DBG if (m_frames.size()) lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: next timestamp=%lld, now=%lld", m_frames.top().first, now);
	// The next assert assures that we have indeed removed all old frames (and, therefore, either there
	// are no frames left, or the first frame has a time that is in the future). It also assures that
	// the frames in m_frames are indeed in the right order.
#if 0
	if (!(m_frames.size() == 0 || m_frames.top().first >= now-frame_duration)) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: top frame before now!");
		lib::logger::get_logger()->debug("now-frameduration = %lld-%lld = %lld", now, frame_duration, now-frame_duration);
		lib::logger::get_logger()->debug("m_old_frame.first = %lld", m_old_frame.first);
		lib::logger::get_logger()->debug("m_frames.top().first = %lld", m_frames.top().first);
		lib::logger::get_logger()->debug("go figure...");
	}
#endif
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

		if (codec == NULL) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Failed to find codec for %s(0x%x)", fmt.name.c_str(), (void*) fmt.parameters);
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				return false;
		}
		//m_con = avcodec_alloc_context();
		
		if(avcodec_open(m_con,codec) < 0) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Failed to open avcodec for %s(0x%x)", fmt.name.c_str(), (void*) fmt.parameters);
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
		// We doe a fmt update here to sure that we have the correct values.
		_need_fmt_uptodate();
		return true;
	}
	// Could add support here for raw mp3, etc.
	return false;
}
