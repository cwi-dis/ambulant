// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

	printf("LIBAVCODEC_VERSION_INT=0x%x\n", LIBAVCODEC_VERSION_INT);
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
	video_datasource *dds =	 new ffmpeg_video_decoder_datasource(ds, fmt);
	if (dds == NULL) {
		lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}

	// Finally, tell the demux datasource to skip ahead to clipBegin, if
	// it can do so. No harm done if it can't: the decoder will then skip
	// any unneeded frames.
	ds->read_ahead(clip_begin);

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::new_video_datasource (dds = 0x%x)", (void*) dds);
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
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_datasource_factory::supported: cannot open video codec (codec_id: %d)", enc->codec_id);

		return false;
	}
	return true;
}

ffmpeg_video_decoder_datasource::ffmpeg_video_decoder_datasource(video_datasource* src, video_format fmt)
:	m_src(src),
	m_con(NULL),
	m_img_convert_ctx(NULL),
	m_con_owned(false),
	m_event_processor(NULL),
	m_client_callback(NULL),
	m_pts_last_frame(0),
	m_oldest_timestamp_wanted(0),
	m_video_clock(0), // XXX Mod by Jack (unsure). Was: src->get_clip_begin()
	m_frame_count(0),
	m_dropped_count(0),
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
	m_dropped_count_before_decoding(0),
	m_possibility_dropping_nonref(0),
	m_frame_count_temp(0),
	m_dropped_count_temp(0),
#endif
	m_elapsed(0),
	m_start_input(true),
	m_pixel_layout(pixel_unknown)
{

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::ffmpeg_video_decoder_datasource() (this = 0x%x)", (void*)this);

	ffmpeg_init();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource: Looking for %s(0x%x) decoder", fmt.name.c_str(), fmt.parameters);
	if (!_select_decoder(fmt))
		lib::logger::get_logger()->error(gettext("ffmpeg_video_decoder_datasource: could not select %s(0x%x) decoder"), fmt.name.c_str(), fmt.parameters);
	m_fmt = fmt;
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
	// initialize random seed
	srand ( time(NULL) );
	m_beforeDecodingDroppingFile = fopen ("beforeDecodingDropping.txt","w");
	m_afterDecodingDroppingFile = fopen ("afterDecodingDropping.txt","w");
	m_noDroppingFile = fopen ("noDropping.txt","w");
#endif
}

ffmpeg_video_decoder_datasource::~ffmpeg_video_decoder_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::~ffmpeg_video_decoder_datasource(0x%x)", (void*)this);
	stop();
	if (m_img_convert_ctx) sws_freeContext(m_img_convert_ctx);
	if (m_dropped_count) lib::logger::get_logger()->debug("ffmpeg_video_decoder: dropped %d of %d frames after decoding", m_dropped_count, m_frame_count);
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
	if (m_dropped_count_before_decoding) lib::logger::get_logger()->debug("ffmpeg_video_decoder: dropped %d frames before decoding", m_dropped_count_before_decoding);
	fclose(m_beforeDecodingDroppingFile);
	fclose(m_afterDecodingDroppingFile);
	fclose(m_noDroppingFile);
#endif
}

void
ffmpeg_video_decoder_datasource::stop()
{
	m_lock.enter();
	if (m_src) {
		m_src->stop();
		long rem = m_src->release();
		if (rem) lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop(0x%x): m_src refcount=%d", (void*)this, rem);
	}
	m_src = NULL;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop(0x%x)", (void*)this);
	if (m_con && m_con_owned) {
		lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
		ffmpeg_lock->enter();
		avcodec_close(m_con);
		ffmpeg_lock->leave();
		av_free(m_con);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::stop(): avcodec_close(m_con=0x%x) called", m_con);
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
ffmpeg_video_decoder_datasource::get_audio_datasource()
{
	m_lock.enter();
	audio_datasource *rv = NULL;
	if (m_src) rv = m_src->get_audio_datasource();
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
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame(): m_client_callback already set!");
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
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_frame: 0x%x: trigger client callback timestamp_milli=%d delta_milli=%d, now_milli=%d, %d frames in buffer", this, (int)timestamp_milli, (int)delta_milli, (int)now_milli, m_frames.size());
			// Sanity check: we don't want this to be more than a second into the future
			if (delta_milli > 1000) {
				lib::logger::get_logger()->debug("ffmpeg_video: frame is %f seconds in the future", delta_milli / 1000.0);
				lib::logger::get_logger()->debug("ffmpeg_video: elapsed()=%dms, timestamp=%dms", now_milli, timestamp_milli);
			}
			evp->add_event(callbackk, delta_milli+1, ambulant::lib::ep_high);
		} else {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
		}
	} else	{
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video: 0x%x: start_frame: record callback for later", this);
		m_client_callback = callbackk;
		m_event_processor = evp;
	}

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
ffmpeg_video_decoder_datasource::start_prefetch(ambulant::lib::event_processor *evp)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::start_prefetch: (this = 0x%x)", (void*) this);
	m_elapsed = 0;
	m_pts_last_frame = 0;
	// xxxbo: should not set m_oldest_timestamp_wanted to zero over here,
	// because prior to start_prefetch, seek already set m_oldest_timestamp_wanted
	// to the value according to m_clip_begin
	//m_oldest_timestamp_wanted = 0;	

	m_video_clock = 0;

	m_event_processor = evp;

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
ffmpeg_video_decoder_datasource::_pop_top_frame() {
	if (m_frames.empty()) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._pop_top_frame(): no frames? (programmer error)");
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
	m_oldest_timestamp_wanted = now+1;

	AM_DBG lib::logger::get_logger()->trace("ffmpeg_video_decoder_datasource::frame_processed_keepdata(%lld), m_oldest_timestamp_wanted = %lld", now, m_oldest_timestamp_wanted);
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

	m_oldest_timestamp_wanted = now+1;

	AM_DBG lib::logger::get_logger()->trace("ffmpeg_video_decoder_datasource::frame_processed(%lld), m_oldest_timestamp_wanted = %lld", now, m_oldest_timestamp_wanted);

	if (m_frames.size() == 0) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_processed: frame queue empty");
		m_lock.leave();
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.frame_processed(%lld)", now);

	while ( m_frames.size() > 0 && m_frames.front().first <= now) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::frame_processed: discarding first frame timestamp=%d, now=%d, data ptr = 0x%x",(int)m_frames.front().first,(int)now, m_frames.front().second);
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
	if(m_fmt.frameduration <=0)
		_need_fmt_uptodate();
		
	// frame rates > 100 fps are unlikely
	// For mp4 H263 video, ffmpeg fills its time_base.den with 1000,
	// resulting in frameduration == 1000 musec, which is wrong.
	// ffplay gets the correct frame rate from its stream, only takes
	// it from the codec if the stream doesn't have that information
	// See: ffmpeg/libavformat/utils.c, function dump_format().
	
	if(m_fmt.frameduration <= 9999)
		m_fmt.frameduration = 33000;
	return m_fmt.frameduration;
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
		// XXXJACK: Bad code. Use av_rescale_q with correct timebases.
		timestamp_t framedur = (timestamp_t) round(m_con->time_base.num *1000000.0 / (double) m_con->time_base.den) ;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::_need_fmt_uptodate(): frameduration = %lld, %d %d", framedur, m_con->time_base.num, m_con->time_base.den);
		m_fmt.frameduration = framedur;
	}
}

void
ffmpeg_video_decoder_datasource::read_ahead(timestamp_t clip_begin)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::read_ahead(): clip_begin=%d", clip_begin);
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
	AVPicture picture;
	int len;
	PixelFormat pic_fmt, dst_pic_fmt;
	int w,h;
	unsigned char* ptr;
	bool did_generate_frame = false;

	timestamp_t ipts = 0;
	uint8_t *inbuf;
	size_t sz;
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

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: %d bytes available, ipts = %lld", sz, ipts);

	if(sz == 0 && !m_src->end_of_file() ) {
		lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: no data, not eof?");
		// Attempt at bug fix for hanging video
		goto out_of_memory;
	}

	// No easy error conditions, so let's allocate our frame.
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail called (0x%x) ", (void*) this);
	if (inbuf && m_con) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:start decoding (0x%x) ", m_con);
//		assert(sz < 1000000); // XXXX This is soft, and probably incorrect. Remove when it fails.
		AVFrame *frame = avcodec_alloc_frame();
		if (frame == NULL) {
			lib::logger::get_logger()->debug("ffmpeg_video_decoder: avcodec_alloc_frame() failed");
			lib::logger::get_logger()->error(gettext("Out of memory playing video"));
			m_src->stop();
			goto out_of_memory;
		}
		ptr = inbuf;

		while (sz > 0) {
			bool drop_this_frame = false;
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: decoding picture(s),  %d bytes of data ", sz);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: m_con: 0x%x, gotpic = %d, sz = %d ", m_con, got_pic, sz);

#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
			// we begin to compute the dropping rate 
			if (m_dropped_count_temp > 5 ) {
				// if the dropping rate is bigger than 1/2 
				// then increase the threshold value of dropping possibility 
				if (m_frame_count_temp/m_dropped_count_temp <= 2) {
					m_possibility_dropping_nonref+=3;
					m_frame_count_temp = 0;
					m_dropped_count_temp = 0;
				} else {
					// otherwise decrease the threshold of dropping possibility
					m_possibility_dropping_nonref-=3;
					if (m_possibility_dropping_nonref < 0 )
						m_possibility_dropping_nonref = 0;
					m_frame_count_temp = 0;
					m_dropped_count_temp = 0;				
				}
			}
			AM_DBG lib::logger::get_logger()->debug("m_possibility_dropping_nonref = %d", m_possibility_dropping_nonref);
			
			//xxxbo tell ffmpeg to drop no reference frames with some possibility 
			// generate a random number between 1 to 10
			int random_dropping_nonref;
			random_dropping_nonref = rand() % 10 + 1;
			
			// the possibility of dropping no refence frame
			assert(random_dropping_nonref > 0);
			
			if (random_dropping_nonref < m_possibility_dropping_nonref)
				m_con->skip_frame = AVDISCARD_NONREF; 
#endif
			// We use skip_frame to make the decoder run faster in case we
			// are not interested in the data (still seeking forward).
			if (ipts != (int64_t)AV_NOPTS_VALUE && ipts < m_oldest_timestamp_wanted) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: setting hurry_up: ipts=%lld, m_oldest_timestamp_wanted=%lld (%lld us late)",ipts, m_oldest_timestamp_wanted, m_oldest_timestamp_wanted-ipts);
				m_con->skip_frame = AVDISCARD_NONREF;
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
				//m_dropped_count++; // This is not necessarily correct
				m_dropped_count_before_decoding++;
				//AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: frame analysis before decoding dropping frame ipts=%lld, m_oldest_timestamp_wanted=%lld (%lld us late)", ipts, m_oldest_timestamp_wanted, m_oldest_timestamp_wanted-ipts);
				AM_DBG lib::logger::get_logger()->debug("BeforeDecodingDropping pts %lld m_dropped_count_before_decoding %d", ipts, m_dropped_count_before_decoding);
				fprintf(m_beforeDecodingDroppingFile,"BeforeDecodingDropping\t pts\t %lld\t 1\n",ipts);
#endif
			} else {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: decoder is %lld us ahead of playout", ipts-m_oldest_timestamp_wanted);
			}
			AVPacket avpkt;
			av_init_packet(&avpkt);
			avpkt.data = ptr;
			avpkt.size = sz;
			len = avcodec_decode_video2(m_con, frame, &got_pic, &avpkt);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: avcodec_decode_video: used %d of %d bytes, gotpic = %d, ipts = %lld", len, sz, got_pic, ipts);
			// It seems avcodec_decode_video sometimes returns 0 if skip_frame is used. Sigh.
			if (len == 0 && !got_pic) {
				len = (int)sz;
				assert((size_t)len == sz);
			}
			m_con->skip_frame = AVDISCARD_DEFAULT;
			if (len < 0) {
				lib::logger::get_logger()->trace(gettext("error decoding video frame (timestamp=%lld)"), ipts);
				sz = 0; // Throw away the rest of the packet, there's little else we can do.
				break;
			}
			assert((size_t)len <= sz);
			ptr += len;
			sz	-= len;
			if (!got_pic) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: incomplete picture, used %d bytes, %d left", len, sz);
				continue;
			}
			AM_DBG lib::logger::get_logger()->debug("pts seems to be : %lld",ipts);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: decoded picture, used %d bytes, %d left", len, sz);
		
			// At this point we need m_fmt to be correct, we are going to use
			// sizes, durations, etc.
			_need_fmt_uptodate();
			
			// Let's first compute the timestamp for this frame. If it is an old frame
			// we can drop it straight away and don't have to go through the motion
			// of doing image conversion.
			timestamp_t pts = 0;
			timestamp_t frame_delay = 0;

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
			AM_DBG lib::logger::get_logger()->debug("videoclock: ipts=%lld pts=%lld video_clock=%lld, frame_delay=%lld", ipts, pts, m_video_clock, frame_delay);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail: storing frame with pts = %lld",pts );
			m_frame_count++;
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
			m_frame_count_temp++;
#endif
			if (pts < m_oldest_timestamp_wanted) {
				// A non-essential frame while skipping forward.
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
				AM_DBG lib::logger::get_logger()->debug("AfterDecodingDropping pts %lld", pts);
				m_dropped_count_before_decoding--;
				fprintf(m_afterDecodingDroppingFile, "AfterDecodingDropping\t pts\t %lld\t 2\n",pts);
#endif
				drop_this_frame = true;
			}
			if (m_frames.size() > 0 && pts < m_frames.front().first) {
				// A frame that came after this frame has already been consumed.
				// We should drop this frame.
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
				AM_DBG lib::logger::get_logger()->debug("AfterDecodingDropping pts %lld", pts);
				m_dropped_count_before_decoding--;
				fprintf(m_afterDecodingDroppingFile, "AfterDecodingDropping\t pts\t %lld\t 2\n",pts);
#endif
				drop_this_frame = true;
			}
			m_elapsed = pts;
			if (drop_this_frame) {
				m_dropped_count++;
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
				m_dropped_count_temp++;
#endif
				continue;
			}

			// Next step: deocde the frame to the image format we want.
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
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:framedata=0x%x", framedata);
			if (framedata == NULL) {
				lib::logger::get_logger()->debug("ffmpeg_video_decoder: malloc(%d) failed", m_size);
				lib::logger::get_logger()->error(gettext("Out of memory playing video"));
				m_src->stop();
				sz = 0;
				goto out_of_memory;
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

			// Finally send the frame upstream.
			std::pair<timestamp_t, char*> element(pts, framedata);
			m_frames.push(element);

			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): push pts = %lld into buffer", pts);
#ifdef WITH_EXPERIMENTAL_FRAME_DROP_STATISTICS
			fprintf(m_noDroppingFile, "NoDropping\t pts\t %lld\t 3\n",pts);
#endif
			did_generate_frame = true;
		} // End of while loop
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource.data_avail:done decoding (0x%x) ", m_con);
		av_free(frame);
		m_src->frame_processed(0); // XXXJACK: Should pass ipts, for sanity check
	}
  out_of_memory:
	// Now tell our client, if we have data available or are at end of file.
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): m_frames.size() returns %d, (eof=%d)", m_frames.size(), m_src->end_of_file());
	if (m_frames.size() > MIN_VIDEO_FRAMES || m_src->end_of_file()) {

		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): there is some data for the renderer ! (eof=%d)", m_src->end_of_file());
		if ( m_client_callback ) {

			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::data_avail(): calling client callback (eof=%d)", m_src->end_of_file());
			assert(m_event_processor);
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::ep_high);
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
ffmpeg_video_decoder_datasource::get_frame(timestamp_t now, timestamp_t *timestamp_p, size_t *size_p)
{
	// pop frames until (just before) "now". Then return the last frame popped.
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame(now=%lld)\n", now);
	// XXX now can be negative, due to time manipulation by the scheduler. assert(now >= 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame() %d frames available\n", m_frames.size());
	//bo 18-03-2008, for some reasons, somtimes, m_frames.size() is 0;
	if (m_frames.size() == 0) {
		m_lock.leave();
		return NULL;
	}
	assert(m_frames.size() > 0 || _end_of_file());

	timestamp_t frame_duration = frameduration();
	assert (frame_duration > 0);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame:  timestamp=%lld, now=%lld, frameduration = %lld",m_frames.front().first, now, frame_duration);

	AM_DBG if (m_frames.size()) lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame: next timestamp=%lld, now=%lld", m_frames.front().first, now);
	// The next assert assures that we have indeed removed all old frames (and, therefore, either there
	// are no frames left, or the first frame has a time that is in the future). It also assures that
	// the frames in m_frames are indeed in the right order.

	if (timestamp_p) *timestamp_p = m_frames.front().first;
	if (size_p) *size_p = m_size;
	char *rv = m_frames.front().second;
	if (rv == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::get_frame(now=%lld): about to return NULL frame (ts=%lld), should not happen", now, *timestamp_p);
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
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 8, 0)
	m_con = avcodec_alloc_context();
#else
	m_con = avcodec_alloc_context3(codec);
#endif
	m_con_owned = true;

	lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
	ffmpeg_lock->enter();
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 8, 0)
	if(avcodec_open(m_con,codec) < 0) {
#else
	  if(avcodec_open2(m_con,codec,NULL) < 0) {
#endif
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
	if (fmt.name == "ffmpeg") {
		AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
		m_con = enc;
		m_con_owned = false;

		if (enc == NULL) {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Parameters missing for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
			return false;
		}
		if (enc->codec_type != CODEC_TYPE_VIDEO) {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Non-video stream for %s(0x%x)", fmt.name.c_str(), enc->codec_type);
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
			return false;
		}
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource._select_decoder: enc->codec_id = 0x%x", enc->codec_id);
		AVCodec *codec = avcodec_find_decoder(enc->codec_id);

		if (codec == NULL) {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Failed to find codec for %s(0x%x)", fmt.name.c_str(),  enc->codec_id);
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during video playback"));
			return false;
		}
		//m_con = avcodec_alloc_context();

		lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
		ffmpeg_lock->enter();
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 8, 0)
		if(avcodec_open(m_con,codec) < 0) {
#else
		  if(avcodec_open2(m_con,codec,NULL) < 0) {
#endif
			ffmpeg_lock->leave();
			lib::logger::get_logger()->debug("Internal error: ffmpeg_video_decoder_datasource._select_decoder: Failed to open avcodec for %s(0x%x)", fmt.name.c_str(), enc->codec_id);
			return false;
		}
		ffmpeg_lock->leave();
		if (fmt.width == 0) fmt.width = m_con->width;
		if (fmt.height == 0) fmt.width = m_con->height;
		return true;
	} else if (fmt.name == "live") {
		const char* codec_name = (char*) fmt.parameters;

		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::selectdecoder(): video codec : %s", codec_name);
		ffmpeg_codec_id* codecid = ffmpeg_codec_id::instance();
		AVCodec *codec = avcodec_find_decoder(codecid->get_codec_id(codec_name));
		if( !codec) {
			return false;
		}
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::selectdecoder(): codec found!");

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 8, 0)
		m_con = avcodec_alloc_context();
#else
		m_con = avcodec_alloc_context3(codec);
#endif
		m_con_owned = true;
		lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
		ffmpeg_lock->enter();
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 8, 0)
		if((avcodec_open(m_con,codec) < 0) ) {
#else
		  if(avcodec_open2(m_con,codec,NULL) < 0) {
#endif
			ffmpeg_lock->leave();
			//lib::logger::get_logger()->error(gettext("%s: Cannot open video codec %d(%s)"), repr(url).c_str(), m_con->codec_id, m_con->codec_name);
			return false;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_video_decoder_datasource::ffmpeg_decoder_datasource(): succesfully opened codec");
		}
		ffmpeg_lock->leave();

		m_con->codec_type = CODEC_TYPE_VIDEO;
		// We doe a fmt update here to sure that we have the correct values.
		_need_fmt_uptodate();
		return true;
	}
	// Could add support here for raw mp3, etc.
	return false;
}
