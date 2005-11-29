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

 
//#include <math.h>
//#include <map>

#include "ambulant/config/config.h"
#include "ambulant/net/ffmpeg_common.h"
#include "ambulant/net/ffmpeg_audio.h" 
#include "ambulant/net/ffmpeg_factory.h" 
#include "ambulant/net/demux_datasource.h" 

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

// Factory functions
audio_datasource_factory *
ambulant::net::get_ffmpeg_audio_datasource_factory()
{
#if 0
	static audio_datasource_factory *s_factory;
	
	if (!s_factory) s_factory = new ffmpeg_audio_datasource_factory();
	return s_factory;
#else
	return new ffmpeg_audio_datasource_factory();
#endif
}

audio_parser_finder *
ambulant::net::get_ffmpeg_audio_parser_finder()
{
#if 0
	static audio_parser_finder *s_factory;
	
	if (!s_factory) s_factory = new ffmpeg_audio_parser_finder();
	return s_factory;
#else
	return new ffmpeg_audio_parser_finder();
#endif
}

audio_filter_finder *
ambulant::net::get_ffmpeg_audio_filter_finder()
{
#if 0
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
	ffmpeg_demux *thread = new ffmpeg_demux(context, clip_begin, clip_end);

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
	audio_datasource *ds = demux_audio_datasource::new_demux_audio_datasource(url, thread);
	if (ds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("fdemux_audio_datasource_factory::new_audio_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}
	ds->read_ahead(clip_begin);
	
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
	return NULL;	
}

audio_datasource* 
ffmpeg_audio_parser_finder::new_audio_parser(const net::url& url, const audio_format_choices& fmts, audio_datasource *src)
{
	if (src == NULL) return NULL;
	audio_datasource *ds = NULL;
#if 0
	// XXXX Here we have to check for the mime type, but raw_audio_datasource doesn't
	// give it...
	if (!ffmpeg_decoder_datasource::supported(src->get_audio_format())) {
#else
	if (!ffmpeg_decoder_datasource::supported(url)) {
#endif
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_parser_finder::new_audio_parser: no support for %s", repr(url).c_str());
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
	if (fmt.name != "ffmpeg") return false;
	AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
	if (enc->codec_type != CODEC_TYPE_AUDIO) return false;
	if (avcodec_find_decoder(enc->codec_id) == NULL) return false;
	return true;
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

bool
ffmpeg_decoder_datasource::supported(const net::url& url)
{
	const char *ext = getext(url);
	if (avcodec_find_decoder_by_name(ext) == NULL) return false;
	return true;
}

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(const net::url& url, audio_datasource *const src)
:	m_con(NULL),
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

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(audio_datasource *const src)
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
ffmpeg_decoder_datasource::_decode(const uint8_t* in, int sz, uint8_t* out, int &outsize)
{
	return avcodec_decode_audio(m_con, (short*) out, &outsize, const_cast<uint8_t*>(in), sz);
}
		  
void 
ffmpeg_decoder_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	bool restart_input = false;
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
		restart_input = true;
		m_client_callback = callbackk;
		m_event_processor = evp;
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
					
					
					AM_DBG lib::logger::get_logger()->debug("avcodec_decode_audio(0x%x, 0x%x, 0x%x(%d), 0x%x, %d)", (void*)m_con, (void*)outbuf, (void*)&outsize, outsize, (void*)inbuf, sz);
					int decoded = avcodec_decode_audio(m_con, (short*) outbuf, &outsize, inbuf, cursz);
					_need_fmt_uptodate();
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail : %d bps, %d channels",m_fmt.samplerate, m_fmt.channels);
					AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail : %d bytes decoded  to %d bytes", decoded,outsize );
					assert(m_fmt.samplerate);
					double duration = ((double) outsize)* sizeof(uint8_t)*8 / (m_fmt.samplerate* m_fmt.channels * m_fmt.bits);
					timestamp_t old_elapsed = m_elapsed;
					m_elapsed += (timestamp_t) (duration*1000000);
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
							timestamp_t delta_t_unwanted = m_src->get_clip_begin() - old_elapsed;
							assert(delta_t_unwanted > 0);
							int bytes_unwanted = (delta_t_unwanted * ((m_fmt.samplerate* m_fmt.channels * m_fmt.bits)/(sizeof(uint8_t)*8)))/1000000;
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
		// XXX The note regarding m_elapsed holds here as well.
		if (!m_src->end_of_file() && m_event_processor && !m_buffer.buffer_full() && !_clip_end() ) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): calling m_src->start() again");
			lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
			m_src->start(m_event_processor, e);
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: not calling start: eof=%d m_ep=0x%x buffull=%d", 
				(int)m_src->end_of_file(), (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
		
		if ( m_client_callback && (m_buffer.buffer_not_empty() ||  _end_of_file() || _clip_end()  ) ) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), _end_of_file());
			assert(m_event_processor);
			if (m_elapsed >= m_src->get_clip_begin()) {
				m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::med);
				m_client_callback = NULL;
			}
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
	
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::_clip_end(): m_elapsed=%lld , clip_end=%lld", m_elapsed, clip_end);
	if (m_elapsed > clip_end) {
		return true;
	}
	
	return false;
}

void 
ffmpeg_decoder_datasource::read_ahead(timestamp_t clip_begin)
{
		m_src->read_ahead(clip_begin);
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
	if (_clip_end()) {
		// clip end falls within the current buffer (or maybe even before it)
		timestamp_t clip_end = m_src->get_clip_end();
		timestamp_t delta_t_unwanted = m_elapsed - clip_end;
		assert(delta_t_unwanted >= 0);
		// ((double) outsize)* sizeof(uint8_t)*8 / (m_fmt.samplerate* m_fmt.channels * m_fmt.bits);
		int bytes_unwanted = (delta_t_unwanted * ((m_fmt.samplerate* m_fmt.channels * m_fmt.bits)/(sizeof(uint8_t)*8)))/1000000;
		assert(bytes_unwanted >= 0);
		rv -= bytes_unwanted;
		rv &= ~3;
		if (rv < 0) rv = 0;
	}
	const_cast <ffmpeg_decoder_datasource*>(this)->m_lock.leave();
	return rv;
}	

timestamp_t
ffmpeg_decoder_datasource::get_clip_end()
{
	m_lock.enter();
	timestamp_t clip_end;
	clip_end =  m_src->get_clip_end();
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

bool 
ffmpeg_decoder_datasource::_select_decoder(const char* file_ext)
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
ffmpeg_decoder_datasource::_select_decoder(audio_format &fmt)
{
	// private method - no need to lock
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
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::_select_decoder: codec_name=%s, codec_id=%d", m_con->codec_name, m_con->codec_id);

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

		int insamples = cursize / (m_in_fmt.channels * sizeof(short));	// integer division !!!!
		if (insamples * m_in_fmt.channels * sizeof(short) != (size_t)cursize) {
			lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: warning: incomplete samples: %d", cursize);
		}
		
		timestamp_t tmp = (timestamp_t)((insamples+1) * m_out_fmt.samplerate * m_out_fmt.channels * sizeof(short) / m_in_fmt.samplerate);
		timestamp_t outsz = tmp;
		

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
		if (m_client_callback && (m_buffer.buffer_not_empty() || _end_of_file() )) {
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
ffmpeg_resample_datasource::_src_end_of_file() const
{
	// private mathod - no need to lock
	
	
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
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::med);
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
