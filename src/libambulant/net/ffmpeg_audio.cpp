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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//#include <math.h>
//#include <map>

#include "ambulant/config/config.h"
#include "ambulant/net/ffmpeg_common.h"
#include "ambulant/net/ffmpeg_audio.h"
#include "ambulant/net/ffmpeg_factory.h"
#include "ambulant/net/demux_datasource.h"

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(54, 59, 0)
#error Ambulant needs at least version 54.59.0 of ffmpeg libavcodec
#endif
 
// WARNING: turning on AM_DBG globally for the ffmpeg code seems to trigger
// a condition that makes the whole player hang or collapse. So you probably
// shouldn't do it:-)

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace net;

typedef lib::no_arg_callback<ffmpeg_decoder_datasource> readdone_callback;
typedef lib::no_arg_callback<ffmpeg_resample_datasource> resample_callback;

#define INBUF_SIZE 4096

//
// Alignment of output buffer for ffmpeg_decode_audio2(). See the comment in avcodec.h
#define FFMPEG_OUTPUT_ALIGNMENT 16

// Factory functions
audio_datasource_factory *
ambulant::net::get_ffmpeg_audio_datasource_factory()
{
#if 0
	// It seems datasource factories are sometimes cleaned up, hence we cannot use
	// a singleton. Need to fix/document at some point.
	static audio_datasource_factory *s_factory;

	if (!s_factory) s_factory = new ffmpeg_audio_datasource_factory();
	return s_factory;
#else
	return new ffmpeg_audio_datasource_factory();
#endif
}

audio_decoder_finder *
ambulant::net::get_ffmpeg_audio_decoder_finder()
{
#if 0
	// It seems datasource factories are sometimes cleaned up, hence we cannot use
	// a singleton. Need to fix/document at some point.
	static audio_parser_finder *s_factory;

	if (!s_factory) s_factory = new ffmpeg_audio_decoder_finder();
	return s_factory;
#else
	return new ffmpeg_audio_decoder_finder();
#endif
}

audio_filter_finder *
ambulant::net::get_ffmpeg_audio_filter_finder()
{
#if 0
	// It seems datasource factories are sometimes cleaned up, hence we cannot use
	// a singleton. Need to fix/document at some point.
	static audio_filter_finder *s_factory;

	if (!s_factory) s_factory = new ffmpeg_audio_filter_finder();
	return s_factory;
#else
	return new ffmpeg_audio_filter_finder();
#endif
}

audio_datasource*
ffmpeg_audio_datasource_factory::new_audio_datasource(const net::url& url, const audio_format_choices& fmts, timestamp_t clip_begin, timestamp_t clip_end)
{

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource(%s)", repr(url).c_str());

	// First we check that the file format is supported by the file reader.
	// If it is we create a file reader (demux head end).
	AVFormatContext *context = ffmpeg_demux::supported(url);
	if (!context) {
		lib::logger::get_logger()->trace("ffmpeg: no support for %s", repr(url).c_str());
		return NULL;
	}
	ffmpeg_demux *thread = new ffmpeg_demux(context, url, clip_begin, clip_end);

	// Now, we can check that there is actually audio in the file.
	if (thread->audio_stream_nr() < 0) {
		thread->cancel();
		lib::logger::get_logger()->trace("ffmpeg: No audio stream in %s", repr(url).c_str());
		return NULL;
	}

	// Next, if there is audio we check that it is either in a format our caller wants,
	// or we can decode this type of audio stream.
	//
	// XXX This code is incomplete. There could be more audio streams in the file,
	// and we could have trouble decoding the first one but not others.... Oh well...
	audio_format fmt = thread->get_audio_format();
	if (!fmts.contains(fmt) && !ffmpeg_decoder_datasource::supported(fmt)) {
		thread->cancel();
		lib::logger::get_logger()->trace("ffmpeg: Unsupported audio stream in %s", repr(url).c_str());
		return NULL;
	}

	// All seems well. Create the demux reader, the decoder and optionally the resampler.
	pkt_audio_datasource *pds = demux_audio_datasource::new_demux_audio_datasource(url, thread);
	if (pds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("fdemux_audio_datasource_factory::new_audio_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}
	pds->read_ahead(clip_begin);

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: parser ds = 0x%x", (void*)pds);
	// XXXX This code should become generalized in datasource_factory
	// XXXX It is also unclear whether this code will work for, say, wav or aiff streams.
	audio_datasource *dds = new ffmpeg_decoder_datasource(pds);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: decoder ds = 0x%x", (void*)dds);
	if (dds == NULL) {
		pds->stop();
		long rem = pds->release();
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
		dds->stop();
		long rem = dds->release();
		assert(rem == 0);
		return NULL;
	}
	if (fmts.contains(rds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return rds;
	}
	lib::logger::get_logger()->error(gettext("%s: unable to create audio resampler"));
	rds->stop();
	long rem = rds->release();
	assert(rem == 0);
	return NULL;
}

audio_datasource*
ffmpeg_audio_decoder_finder::new_audio_decoder(pkt_audio_datasource *src, const audio_format_choices& fmts)
{
	if (src == NULL) return NULL;
	audio_datasource *ds = NULL;
	if (!ffmpeg_decoder_datasource::supported(src->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_parser_finder::new_audio_parser: no support for format");
		return NULL;
	}
	ds = new ffmpeg_decoder_datasource(src);
	if (ds == NULL) {
		return NULL;
	}
	return ds;
}

audio_datasource*
ffmpeg_audio_filter_finder::new_audio_filter(audio_datasource *src, const audio_format_choices& fmts)
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

// **************************** ffpmeg_decoder_datasource *****************************
bool
ffmpeg_decoder_datasource::supported(const audio_format& fmt)
{
	if (fmt.name == "ffmpeg") {
		AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
		if (enc->codec_type != AVMEDIA_TYPE_AUDIO) return false;
		if (avcodec_find_decoder(enc->codec_id) == NULL) return false;
		return true;
	}
	if (fmt.name == "live") {
		//AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
		const char* codec_name = (char*) fmt.parameters;

		ffmpeg_codec_id* codecid = ffmpeg_codec_id::instance();
		AVCodec *codec = avcodec_find_decoder(codecid->get_codec_id(codec_name));

		if(!codec) {
			return false;
		}

		return true;
	}
	return false;
}

// Hack, hack. Get extension of a URL.
static const char *
getext(const net::url &url)
{
	const char *curl = url.get_path().c_str();
	const char *dotpos = strrchr(curl, '.');
	if (dotpos) return dotpos+1;
	return NULL;
}

bool
ffmpeg_decoder_datasource::supported(const net::url& url)
{
	const char *ext = getext(url);
	if (ext == NULL || avcodec_find_decoder_by_name(ext) == NULL) return false;
	return true;
}

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(const net::url& url, pkt_audio_datasource *const src)
:	m_con(NULL),
	m_con_owned(false),
	m_fmt(audio_format(0,0,0)),
	m_event_processor(NULL),
	m_src(src),
	m_elapsed(m_src->get_start_time()),
	m_is_audio_ds(false),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource() -> 0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
	ffmpeg_init();
	const char *ext = getext(url);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: Selecting \"%s\" decoder", ext);
	if (!_select_decoder(ext))
		lib::logger::get_logger()->error(gettext("%s: audio decoder \"%s\" not supported"), url.get_url().c_str(), ext);
}

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(pkt_audio_datasource *const src)
:	m_con(NULL),
	m_fmt(src->get_audio_format()),
	m_event_processor(NULL),
	m_src(src),
	m_elapsed(m_src->get_start_time()),
	m_is_audio_ds(true),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource() -> 0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
	ffmpeg_init();
	audio_format fmt = src->get_audio_format();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: Looking for %s(0x%x) decoder", fmt.name.c_str(), fmt.parameters);
	if (!_select_decoder(fmt)) {
		lib::logger::get_logger()->error(gettext("ffmpeg_decoder_datasource: could not select %s(0x%x) decoder"), fmt.name.c_str(), fmt.parameters);
	}
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
	if (m_con && m_con_owned) {
		lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
		ffmpeg_lock->enter();
		avcodec_close(m_con);
		ffmpeg_lock->leave();
		av_free(m_con);
	}
	m_con = NULL;
	if (m_src) {
		m_src->stop();
		long rem = m_src->release();
		if (rem) lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::stop(0x%x): m_src refcount=%d", (void*)this, rem);
	}
	m_src = NULL;
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	bool restart_input = false;

	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): m_client_callback already set, cleared.");
	}
	if (evp == NULL) {
		lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): event_processor is null, clearing callback.");
		callbackk = NULL;
	}
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::ep_med);
		} else {
			lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		restart_input = true;
		m_client_callback = callbackk;
		m_event_processor = evp;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: storing callback");
	}

	// Also restart our source if we still have room and there is
	// data to read.
	if ( !_end_of_file() && !m_buffer.buffer_full() ) restart_input = true;

	if (restart_input) {
		lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(evp,  e);
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): not restarting, eof=%d, buffer_full=%d", (int)_end_of_file(), (int)m_buffer.buffer_full());
	}
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::start_prefetch(ambulant::lib::event_processor *evp)
{
	m_lock.enter();
	bool restart_input = false;

	m_event_processor = evp;

	if ( !_end_of_file() && !m_buffer.buffer_full() ) restart_input = true;

	if (restart_input) {
		lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start_prefetch(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(evp,  e);
	}
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::readdone(size_t len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.readdone : done with %d bytes", len);
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::data_avail()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: called : m_src->get_read_ptr() m_src=0x%x, this=0x%x", (void*) m_src, (void*) this);
	if (m_con == NULL) {
		m_lock.leave();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No decoder, flushing available data");
		return;
	}
	if (m_src == NULL) {
		m_lock.leave();
		lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No datasource !");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		return;
	}
	if (!m_buffer.buffer_full()) {
		AVPacket *tmp_pkt = NULL;
		uint8_t *inbuf = NULL;
		size_t sz = 0;
		timestamp_t old_elapsed;

		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: m_src->get_read_ptr() m_src=0x%x, this=0x%x", (void*) m_src, (void*) this);

		if (m_src->end_of_file()) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: end of file");
		} else {
			datasource_packet audio_packet = m_src->get_packet();
			old_elapsed = audio_packet.pts;
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: m_elapsed %lld, pts=%lld pkt=%p flag=%d", m_elapsed, audio_packet.pts, audio_packet.pkt, (int)audio_packet.flag);
			if (audio_packet.flag == datasource_packet_flag_avpacket) {
				tmp_pkt = audio_packet.pkt;
				if (tmp_pkt == NULL) {
					lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: pkt=NULL for pts=%lld", audio_packet.pts);
					m_lock.leave();
					return;
				}
				inbuf = tmp_pkt->data;
				sz = tmp_pkt->size;
			} else if (sz == datasource_packet_flag_flush) {
				/* flush codec */;
				sz = 0;
			}
	}
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: %d bytes available", (int)sz);

		// Note: outsize is only written by avcodec_decode_audio, not read!
		// You must always supply a buffer that is AVCODEC_MAX_AUDIO_FRAME_SIZE
		// bytes big!
		int outsize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		uint8_t *outbuf = (uint8_t*) m_buffer.get_write_ptr(outsize+FFMPEG_OUTPUT_ALIGNMENT-1);
		if (outbuf) {
			if(inbuf) {
				// Don't feed too much data to the decoder, it doesn't like to do lists ;-)
				size_t cursz = sz;
				if (cursz > AVCODEC_MAX_AUDIO_FRAME_SIZE/2) cursz = AVCODEC_MAX_AUDIO_FRAME_SIZE/2;
				// avcodec_decode_audio2 may require the output buffer to be aligned on a 16-byte boundary.
				// So we request 15 bytes more, pass an aligned pointer, and copy down if needed.
				short *ffmpeg_outbuf = (short *)(((size_t)outbuf+FFMPEG_OUTPUT_ALIGNMENT-1) & ~(FFMPEG_OUTPUT_ALIGNMENT-1));
				AM_DBG lib::logger::get_logger()->debug("avcodec_decode_audio(0x%x, 0x%x, 0x%x(%d), 0x%x, %d)", (void*)m_con, (void*)outbuf, (void*)&outsize, outsize, (void*)inbuf, (int)cursz);

				// Adapted to the new api avcodec_decode_audio3
				AVPacket avpkt;
				av_init_packet(&avpkt);
				avpkt.data = inbuf;
				avpkt.size = (int)cursz;
				AM_DBG lib::logger::get_logger()->debug("avocodec_decode_audio: calling avcodec_decode_audio3(..., 0x%x, %d, ...)", (void*)ffmpeg_outbuf, (int)outsize);
				assert(ffmpeg_outbuf);
				int decoded = avcodec_decode_audio3(m_con, ffmpeg_outbuf, &outsize, &avpkt);
				if (decoded < 0) outsize = 0;
#if FFMPEG_OUTPUT_ALIGNMENT-1
				if (outsize > 0 && (uint8_t *)ffmpeg_outbuf != outbuf)
					memmove(outbuf, ffmpeg_outbuf, outsize);
#endif
				///// Feeding the successive block of one rtsp mp3 packet to ffmpeg to decode,
				///// since ffmpeg can only decode the limited length of around 522(522 or 523
				///// in the case of using testOnDemandRTSPServer as the RTSP server) bytes data
				///// at one time. This idea is borrowed from VLC, according to:
				///// vlc-0.8.6c/module/codec/ffmpeg/audio.c:L253-L254.
				AM_DBG lib::logger::get_logger()->debug("avocodec_decode_audio: converted %d of %d bytes to %d", decoded, (int)cursz, outsize);
				while (decoded > 0 && decoded < (int)cursz) {
					inbuf += decoded;
					cursz -= decoded;
					m_buffer.pushdata(outsize);
					outsize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
					outbuf = (uint8_t*) m_buffer.get_write_ptr(outsize);
					if (outbuf == NULL) {
						// At this point we are committed to push the data downstream. So if the output buffer is full our
						// only option is to enlarge the buffer.
						size_t newbufsize = m_buffer.size()*2;
						lib::logger::get_logger()->trace("avcodec_decode_audio: enlarging audio output buffer to %d", newbufsize);
						m_buffer.set_max_size(newbufsize);
						outbuf = (uint8_t*) m_buffer.get_write_ptr(outsize);
					}
					assert(outbuf);
					ffmpeg_outbuf = (short *)(((size_t)outbuf+FFMPEG_OUTPUT_ALIGNMENT-1) & ~(FFMPEG_OUTPUT_ALIGNMENT-1));
					//xxxbo Over rtsp, one packet may contain multiple frames, so updating the beginning address of avpkt.data is needed  
					avpkt.data = inbuf;
					AM_DBG lib::logger::get_logger()->debug("avcodec_decode_audio: again calling avcodec_decode_audio3(..., 0x%x, %d, ...)", (void*)ffmpeg_outbuf, (int)outsize);
					assert(ffmpeg_outbuf);
					decoded = avcodec_decode_audio3(m_con, (short*) ffmpeg_outbuf, &outsize, &avpkt);
					if (decoded < 0) outsize = 0;
					AM_DBG lib::logger::get_logger()->debug("avocodec_decode_audio: converted additional %d of %d bytes to %d", decoded, cursz, outsize);
#if FFMPEG_OUTPUT_ALIGNMENT-1
					if (outsize > 0 && (uint8_t *)ffmpeg_outbuf != outbuf)
						memmove(outbuf, ffmpeg_outbuf, outsize);
#endif
				}

				// If this loop ends with decoded == 0 and cursz > 0, it means that not all bytes
				// have been fed to the decoder.
				if (decoded == 0 && cursz > 0)
					lib::logger::get_logger()->trace("ffmpeg_audio_decoder: last %d bytes of packet dropped");

				_need_fmt_uptodate();
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail : %d bps, %d channels",m_fmt.samplerate, m_fmt.channels);
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail : %d bytes decoded  to %d bytes", decoded,outsize );

				assert(m_fmt.samplerate);
				timestamp_t duration = ((timestamp_t) outsize) * sizeof(uint8_t)*8 / (m_fmt.samplerate* m_fmt.channels * m_fmt.bits);
#if 1
				// We only warn, we don't reset. Resetting has adverse consequences...
				if (old_elapsed < m_elapsed) {
					lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: got old data for timestamp %lld. Reset from %lld", old_elapsed, m_elapsed);
				}
#else
				if (old_elapsed < m_elapsed) {
					size_t to_discard = m_buffer.size();
					(void)m_buffer.get_read_ptr();
					m_buffer.readdone(to_discard);
					lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: got old data for timestamp %lld. Flushing buffer (%d bytes) from %lld", old_elapsed, to_discard, m_elapsed);
				}
#endif
				m_elapsed = old_elapsed + duration;
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail elapsed = %d ", m_elapsed);

				// We need to do some tricks to handle clip_begin falling within this buffer.
				// First we push all the data we have into the buffer, then we check whether the beginning
				// should have been skipped and, if so, read out the bytes.
				if (m_elapsed > m_src->get_clip_begin()) {
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail We passed clip_begin : (outsize = %d) ", outsize);
					if (outsize > 0) {
						m_buffer.pushdata(outsize);
					} else {
						m_buffer.pushdata(0);
					}
					if (old_elapsed < m_src->get_clip_begin()) {
						assert(m_buffer.size() == (size_t)outsize);
						timestamp_t delta_t_unwanted = m_src->get_clip_begin() - old_elapsed;
						assert(delta_t_unwanted > 0);
						size_t bytes_unwanted = (size_t)(delta_t_unwanted * ((m_fmt.samplerate* m_fmt.channels * m_fmt.bits)/(sizeof(uint8_t)*8))/1000000);
						bytes_unwanted &= ~3;
						AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: clip_begin within buffer, dropping %lld us, %d bytes", delta_t_unwanted, bytes_unwanted);
						(void)m_buffer.get_read_ptr();
						assert(m_buffer.size() > bytes_unwanted);
						m_buffer.readdone(bytes_unwanted);
					}
				} else {
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: m_elapsed = %lld < clip_begin = %lld, skipped %d bytes", m_elapsed, m_src->get_clip_begin(), outsize);
					m_buffer.pushdata(0);
				}

				AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail : m_src->readdone(%d) called m_src=0x%x, this=0x%x", decoded,(void*) m_src, (void*) this );
			} else {
				m_buffer.pushdata(0);
			}
		} else {
			lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: no room in output buffer");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
			m_buffer.pushdata(0);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail m_src->readdone(0) called this=0x%x");
		}
		if (tmp_pkt) {
            AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: free pkt=%p (data %p, size %d)\n", tmp_pkt, tmp_pkt->data, tmp_pkt->size);
			av_free_packet(tmp_pkt);
            free(tmp_pkt);
		}
		// Restart reading if we still have room to accomodate more data
		// XXX The note regarding m_elapsed holds here as well.
		if (!m_src->end_of_file() && m_event_processor ) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): calling m_src->start() again");
			lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
			m_src->start(m_event_processor, e);
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: not calling start: eof=%d m_ep=0x%x buffull=%d", (int)m_src->end_of_file(), (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
	}

	if ( m_client_callback && (m_buffer.buffer_not_empty() ||  _end_of_file()  ) ) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), _end_of_file());
		assert(m_event_processor);
		if (m_elapsed >= m_src->get_clip_begin()) {
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::ep_med);
			m_client_callback = NULL;
			m_event_processor = NULL;
		}
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No client callback!");
	}
	m_lock.leave();
}

bool
ffmpeg_decoder_datasource::end_of_file()
{
	m_lock.enter();
	if (_clip_end()) {
		m_lock.leave();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::end_of_file(): clip_end reached");
		return true;
	}

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
ffmpeg_decoder_datasource::_clip_end() const
{
	// private method - no need to lock
	timestamp_t clip_end = m_src->get_clip_end();
	if (clip_end == -1) return false;

	timestamp_t buffer_begin_elapsed = m_elapsed - 1000000LL * (m_buffer.size() * 8) / (m_fmt.samplerate* m_fmt.channels * m_fmt.bits);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::_clip_end(): m_elapsed=%lld, buffer_begin_elapsed=%lld , clip_end=%lld", m_elapsed, buffer_begin_elapsed, clip_end);
	if (buffer_begin_elapsed > clip_end) {
		return true;
	}

	return false;
}

void
ffmpeg_decoder_datasource::read_ahead(timestamp_t clip_begin)
{
	m_lock.enter();
	m_src->read_ahead(clip_begin);
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::seek(timestamp_t time)
{
	m_lock.enter();
	bool skip_seek = false;
	assert( time >= 0);

	// Do the seek before the flush
#if 1
	if (!skip_seek) {
		m_src->seek(time);
		m_elapsed = time; // XXXJACK not needed??
	}
#endif
	size_t nbytes = m_buffer.size();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource(0x%x)::seek(%ld), discard %d bytes, old time was %ld", (void*)this, (long)time, (int)nbytes, m_elapsed);
	if (nbytes) {
#if 0
		/* Temporarily disabled, to see whether it fixes #2954199 */
		timestamp_t buffer_begin_elapsed = m_elapsed - 1000000LL * (m_buffer.size() * 8) / (m_fmt.samplerate* m_fmt.channels * m_fmt.bits);
		// If the requested seek time falls within the buffer we are in luck, and do the seek by dropping some data.
		if (time >= buffer_begin_elapsed && time < m_elapsed) {
			nbytes = ((time-buffer_begin_elapsed) * (m_fmt.samplerate* m_fmt.channels * m_fmt.bits)) / (8LL * 1000000LL);
			nbytes &= ~0x1; //	nbytes may be odd s.t. resulting pointer becomes unuseable for ffmpeg; fixes #2954199
			skip_seek = true;
		}
#endif
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: flush buffer (%d bytes) due to seek", nbytes);
		(void)m_buffer.get_read_ptr();
		m_buffer.readdone(nbytes);
	}
#if 0
	if (!skip_seek) {
		m_src->seek(time);
		m_elapsed = time; // XXXJACK not needed??
	}
#endif
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::set_clip_end(timestamp_t clip_end)
{
	m_src->set_clip_end(clip_end);
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

size_t
ffmpeg_decoder_datasource::size() const
{
	const_cast <ffmpeg_decoder_datasource*>(this)->m_lock.enter();
	size_t rv = m_buffer.size();
	if (_clip_end()) {
		// Check whether clip end falls within the current buffer (or maybe even before it)
		timestamp_t clip_end = m_src->get_clip_end();
		assert(m_elapsed > clip_end);
		timestamp_t delta_t_unwanted = m_elapsed - clip_end;
		assert(delta_t_unwanted >= 0);
		size_t bytes_unwanted = (size_t)((delta_t_unwanted * ((m_fmt.samplerate* m_fmt.channels * m_fmt.bits)/(sizeof(uint8_t)*8)))/1000000);
		assert(bytes_unwanted >= 0);
        assert(bytes_unwanted >= rv);
		rv -= bytes_unwanted;
		rv &= ~3;
	}
	const_cast <ffmpeg_decoder_datasource*>(this)->m_lock.leave();
	return rv;
}

timestamp_t
ffmpeg_decoder_datasource::get_clip_end()
{
	m_lock.enter();
	timestamp_t clip_end;
	clip_end =	m_src->get_clip_end();
	m_lock.leave();
	return clip_end;
}

timestamp_t
ffmpeg_decoder_datasource::get_clip_begin()
{
	m_lock.enter();
	timestamp_t clip_begin;
	clip_begin =  m_src->get_clip_begin();
	m_lock.leave();
	return clip_begin;
}

timestamp_t
ffmpeg_decoder_datasource::get_elapsed()
{
	m_lock.enter();
	_need_fmt_uptodate();
	int bps = m_fmt.samplerate* m_fmt.channels * m_fmt.bits;
	timestamp_t buffer_duration = 0;
	if (bps != 0)
		buffer_duration = 1000000LL * (m_buffer.size() * 8) / bps;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::get_elapsed: m_elapsed %ld, buffer %ld", m_elapsed, buffer_duration);
	timestamp_t	 elapsed =	m_elapsed - buffer_duration;
	m_lock.leave();
	return elapsed;
}

bool
ffmpeg_decoder_datasource::_select_decoder(const char* file_ext)
{
	// private method - no need to lock
	AVCodec *codec = avcodec_find_decoder_by_name(file_ext);
	if (codec == NULL) {
		lib::logger::get_logger()->trace("ffmpeg_decoder_datasource._select_decoder: Failed to find codec for \"%s\"", file_ext);
		lib::logger::get_logger()->error(gettext("No support for \"%s\" audio"), file_ext);
		return false;
	}
	m_con = avcodec_alloc_context3(codec);
	m_con_owned = true;

	lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
	ffmpeg_lock->enter();
	if(avcodec_open2(m_con,codec,NULL) < 0) {
		ffmpeg_lock->leave();
		lib::logger::get_logger()->trace("ffmpeg_decoder_datasource._select_decoder: Failed to open avcodec for \"%s\"", file_ext);
		lib::logger::get_logger()->error(gettext("No support for \"%s\" audio"), file_ext);
		return false;
	}
	ffmpeg_lock->leave();
	return true;
}

bool
ffmpeg_decoder_datasource::_select_decoder(audio_format &fmt)
{
	// private method - no need to lock
	if (fmt.name == "ffmpeg") {
		m_con = (AVCodecContext *)fmt.parameters;
		m_con_owned = false;

		if (m_con == NULL) {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Parameters missing for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
			return false;
		}
		if (m_con->codec_type != AVMEDIA_TYPE_AUDIO) {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Non-audio stream for %s(0x%x)", fmt.name.c_str(), m_con->codec_type);
			return false;
		}

		AVCodec *codec = avcodec_find_decoder(m_con->codec_id);
		if (codec == NULL) {
			lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Failed to find codec for %s(0x%x)", fmt.name.c_str(), m_con->codec_id);
			return false;
		}
//		m_con = avcodec_alloc_context();

		lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
		ffmpeg_lock->enter();
		if(avcodec_open2(m_con,codec,NULL) < 0) {
			ffmpeg_lock->leave();
			lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Failed to open avcodec for %s(0x%x)", fmt.name.c_str(), m_con->codec_id);
			av_free(m_con);
			m_con = NULL;
			return false;
		}
		ffmpeg_lock->leave();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::_select_decoder: codec_name=%s, codec_id=%d", m_con->codec_name, m_con->codec_id);
		m_fmt = audio_format(m_con->sample_rate, m_con->channels, 16);
		return true;
	} else if (fmt.name == "live") {
		const char* codec_name = (char*) fmt.parameters;

		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::selectdecoder(): audio codec : %s", codec_name);
		ffmpeg_codec_id* codecid = ffmpeg_codec_id::instance();
		AVCodec *codec = avcodec_find_decoder(codecid->get_codec_id(codec_name));

		if( !codec) {
			//lib::logger::get_logger()->error(gettext("%s: Audio codec %d(%s) not supported"), repr(url).c_str(), m_con->codec_id, m_con->codec_name);
			return false;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::selectdecoder(): codec found!");
		}

		m_con = avcodec_alloc_context3(codec);
		m_con_owned = true;
		m_con->channels = 0;
		lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
		ffmpeg_lock->enter();
	    if((avcodec_open2(m_con,codec, NULL) < 0) ) {
			ffmpeg_lock->leave();
			//lib::logger::get_logger()->error(gettext("%s: Cannot open audio codec %d(%s)"), repr(url).c_str(), m_con->codec_id, m_con->codec_name);
			av_free(m_con);
			m_con = NULL;
			return false;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(): succesfully opened codec");
		}
		ffmpeg_lock->leave();

		m_con->codec_type = AVMEDIA_TYPE_AUDIO;
		m_fmt = audio_format(m_con->sample_rate, m_con->channels, 16);
		return true;
	}
	// Could add support here for raw mp3, etc.
	return false;
}

audio_format&
ffmpeg_decoder_datasource::get_audio_format()
{
	m_lock.enter();
	_need_fmt_uptodate();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::get_audio_format: rate=%d, bits=%d,channels=%d",m_fmt.samplerate, m_fmt.bits, m_fmt.channels);
	m_lock.leave();
	return m_fmt;
}

void
ffmpeg_decoder_datasource::_need_fmt_uptodate()
{
	// Private method - no locking
	if (m_fmt.samplerate == 0) {
		m_fmt.samplerate = m_con->sample_rate;
	}
	if (m_fmt.channels == 0) {
		m_fmt.channels = m_con->channels;
	}
	if (m_fmt.channels == 0 || m_fmt.samplerate == 0) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: No samplerate/channel data available yet, guessing...");
	}
}

common::duration
ffmpeg_decoder_datasource::get_dur()
{
	return m_src->get_dur();
}

// **************************** ffmpeg_resample_datasource *****************************

ffmpeg_resample_datasource::ffmpeg_resample_datasource(audio_datasource *src, audio_format_choices fmts)
:	m_src(src),
	m_context_set(false),
	m_resample_context(NULL),
	m_event_processor(NULL),
	m_client_callback(NULL),
	m_in_fmt(src->get_audio_format()),
	m_out_fmt(fmts.best()),
    m_is_live(false)
{
	ffmpeg_init();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::ffmpeg_resample_datasource()->0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
}

ffmpeg_resample_datasource::~ffmpeg_resample_datasource()
{
	stop();
}

void
ffmpeg_resample_datasource::stop()
{
	m_lock.enter();
	long oldrefcount = get_ref_count();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x)", (void*)this);
	if (m_src) {
		m_src->stop();
		long rem = m_src->release();
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
	size_t sz;

	size_t cursize = 0;
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
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource: initializing context: inrate, ch=%d, %d, outrate, ch=%d, %d", m_in_fmt.samplerate,	 m_in_fmt.channels, m_out_fmt.samplerate,  m_out_fmt.channels);
		AVSampleFormat samplefmt_out = AV_SAMPLE_FMT_S16;
		AVSampleFormat samplefmt_in = AV_SAMPLE_FMT_S16;
		// Note: the four filter parameters are taken from ffmpeg.c
		// It is unclear how one could make a better determination of these parameters.
		m_resample_context = av_audio_resample_init(
			m_out_fmt.channels,
			m_in_fmt.channels,
			m_out_fmt.samplerate,
			m_in_fmt.samplerate,
			samplefmt_out,
			samplefmt_in,
			16, 10, 0, 0.8
			);
		if (!m_resample_context) {
			lib::logger::get_logger()->error(gettext("Audio cannot be converted to 44Khz stereo"));
			//m_src->stop();
			///m_src->release();
			//m_src = NULL;
			//m_lock.leave();
			//return;
		}
		m_context_set = true;
	}
	if(m_src) {
		sz = m_src->size();
	} else {
		lib::logger::get_logger()->debug("Internal error: ffmpeg_audio_datasource::data_avail: No datasource");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		m_lock.leave();
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

		size_t insamples = cursize / (m_in_fmt.channels * sizeof(short));	// integer division !!!!
		if (insamples * m_in_fmt.channels * sizeof(short) != cursize) {
			lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: warning: incomplete samples: %d", cursize);
		}
		assert (m_in_fmt.samplerate);
		timestamp_t tmp = (timestamp_t)((insamples+1) * m_out_fmt.samplerate * m_out_fmt.channels * sizeof(short) / m_in_fmt.samplerate);
		size_t outsz = (size_t)tmp;
		assert(tmp == (timestamp_t)outsz); // Check for silly type mismatches

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
				int outsamples = audio_resample(m_resample_context, outbuf, inbuf, (int)insamples);
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
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: not calling start: eof=%d m_ep=0x%x buffull=%d",
				(int)m_src->end_of_file(), (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
		// If the client is currently interested tell them about data being available
		if (m_client_callback && (m_buffer.buffer_not_empty() || _end_of_file() )) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), _end_of_file());
			assert(m_event_processor);
			lib::event *clientcallback = m_client_callback;
			m_client_callback = NULL;
			m_event_processor->add_event(clientcallback, 0, ambulant::lib::ep_med);
			m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): No client callback!");
		}
	} else {
		// Something went wrong during initialization, we just drop the data
		// on the floor.
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): No resample context, flushing data");
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): m_src->readdone(%d) called m_src=0x%x, this=0x%x", sz, (void*) m_src, (void*) this);
		//m_src->readdone(sz);
	}
	m_lock.leave();
}


void
ffmpeg_resample_datasource::readdone(size_t len)
{
	m_lock.enter();
	if (m_src == NULL) {
		m_lock.leave();
		return;
	}
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
ffmpeg_resample_datasource::_src_end_of_file() const
{
	// private mathod - no need to lock

	if (m_src)
		return m_src->end_of_file();

	return true;
}

void
ffmpeg_resample_datasource::read_ahead(timestamp_t clip_begin)
{
	m_lock.enter();
	m_src->read_ahead(clip_begin);
	m_lock.leave();
}

void
ffmpeg_resample_datasource::seek(timestamp_t time)
{
	m_lock.enter();
	assert( time >= 0);
	size_t nbytes = m_buffer.size();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource(0x%x)::seek(%ld), discard %d bytes", (void*)this, (long)time, nbytes);
	if (nbytes) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_resample_datasource: flush buffer (%d bytes) due to seek", nbytes);
		(void)m_buffer.get_read_ptr();
		m_buffer.readdone(nbytes);
	}
	m_src->seek(time);
	m_lock.leave();
}

void
ffmpeg_resample_datasource::set_clip_end(timestamp_t clip_end)
{
	m_lock.enter();
	m_src->set_clip_end(clip_end);
	m_lock.leave();
}

timestamp_t
ffmpeg_resample_datasource::get_elapsed()
{
	m_lock.enter();
	timestamp_t src_elapsed = m_src->get_elapsed();
	size_t nbytes = m_buffer.size();
	timestamp_t buffer_elapsed = (1000000LL * nbytes * 8) / (m_out_fmt.channels * m_out_fmt.samplerate * m_out_fmt.bits);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::get_elapsed: src_elapsed %ld, buffer %ld", src_elapsed, buffer_elapsed);
	timestamp_t rv = src_elapsed - buffer_elapsed;
	m_lock.leave();
	return rv;
}

bool
ffmpeg_resample_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}

timestamp_t
ffmpeg_resample_datasource::get_clip_end()
{
	return m_src->get_clip_end();
}

timestamp_t
ffmpeg_resample_datasource::get_clip_begin()
{
	return m_src->get_clip_begin();
}

char*
ffmpeg_resample_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

size_t
ffmpeg_resample_datasource::size() const
{
	const_cast <ffmpeg_resample_datasource*>(this)->m_lock.enter();
	size_t rv = m_buffer.size();
	const_cast <ffmpeg_resample_datasource*>(this)->m_lock.leave();
	return rv;
}

void
ffmpeg_resample_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	bool restart_input = false;

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): start(0x%x) called", this);
	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): m_client_callback already set!");
	}

	if ( m_buffer.buffer_not_empty() && _end_of_file() ) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): (%d  || %d) = %d ", _end_of_file(), m_buffer.buffer_not_empty(), _end_of_file() || m_buffer.buffer_not_empty());

		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		restart_input = false;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): no restart EOF (or clipend reached) but no data available");
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::ep_med);
		} else {
			lib::logger::get_logger()->error("Internal error: ffmpeg_resample_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): (%d && %d) = %d ", !_end_of_file(), !m_buffer.buffer_full(), !_end_of_file() && !m_buffer.buffer_full());
		restart_input = true;
		m_client_callback = callbackk;
		m_event_processor = evp;
	}
	// Also restart our source if we still have room and there is
	// data to read.
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): (%d && %d) = %d ", !_end_of_file(), !m_buffer.buffer_full(), !_end_of_file() && !m_buffer.buffer_full());
	if ( !_src_end_of_file() && !m_buffer.buffer_full() ) {
		restart_input = true;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): no EOF and buffer is not full so we need to do a restart, (%d && %d) = %d ", !_end_of_file(), !m_buffer.buffer_full(), !_end_of_file() && !m_buffer.buffer_full());
	}


	if (restart_input) {
		// Restart the input stream
		lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(evp,  e);
	}

	m_lock.leave();
}

void
ffmpeg_resample_datasource::start_prefetch(ambulant::lib::event_processor *evp)
{
	m_lock.enter();
	bool restart_input = false;

	m_event_processor = evp;

	if ( !_end_of_file() && !m_buffer.buffer_full() ) restart_input = true;

	if (restart_input) {
		lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start_prefetch(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(evp,  e);
	}
	m_lock.leave();
}

common::duration
ffmpeg_resample_datasource::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	if (m_src)
		rv = m_src->get_dur();
	m_lock.leave();
	return rv;
}
