
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
 
#include "ambulant/net/ffmpeg_datasource.h" 
#include "ambulant/net/datasource.h"
#include "ambulant/lib/logger.h"
#include "ambulant/net/url.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// Bug workaround: define RESAMPLE_READ_ALL to let the resampler
// collect all data before calling the client callback
//#define RESAMPLE_READ_ALL
using namespace ambulant;
using namespace net;

typedef lib::no_arg_callback<ffmpeg_decoder_datasource> readdone_callback;
typedef lib::no_arg_callback<ffmpeg_resample_datasource> resample_callback;

#define INBUF_SIZE 4096

// Static initializer function shared among ffmpeg classes
static void 
ffmpeg_init()
{
	static bool is_inited = false;
	if (is_inited) return;
	avcodec_init();
#ifdef WITH_FFMPEG_AVFORMAT
	av_register_all();
#else
	avcodec_register_all();
#endif
	is_inited = true;
}

// Hack, hack. Get extension of a URL.
static const char *
getext(const std::string &url)
{
	const char *curl = url.c_str();
	const char *dotpos = rindex(curl, '.');
	if (dotpos) return dotpos+1;
	return NULL;
}

audio_datasource* 
ffmpeg_audio_datasource_factory::new_audio_datasource(const std::string& url, audio_format_choices fmts)
{
#ifdef WITH_FFMPEG_AVFORMAT
	net::url   loc(url);
	
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource_factory::new_audio_datasource(%s)", url.c_str());
	AVFormatContext *context = ffmpeg_parser_datasource::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource_factory::new_audio_datasource: no support for %s", url.c_str());
		return NULL;
	}
	audio_datasource *ds = new ffmpeg_parser_datasource(url, context);
	if (ds == NULL) return NULL;
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource_factory::new_audio_datasource: parser ds = 0x%x", (void*)ds);
	// XXXX This code should become generalized in datasource_factory
	if (fmts.contains(ds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return ds;
	}
	audio_datasource *dds = new ffmpeg_decoder_datasource(url, ds);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource_factory::new_audio_datasource: decoder ds = 0x%x", (void*)dds);
	if (dds == NULL) {
		ds->release();
		return NULL;
	}
	if (fmts.contains(dds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return dds;
	}
	audio_datasource *rds = new ffmpeg_resample_datasource(dds, fmts);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource_factory::new_audio_datasource: resample ds = 0x%x", (void*)rds);
	if (rds == NULL)  {
		dds->release();
		return NULL;
	}
	if (fmts.contains(rds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return rds;
	}
	lib::logger::get_logger()->error("ffmpeg_audio_datasource_factory::new_audio_datasource: unable to create resampler");
	rds->release();
#endif // WITH_FFMPEG_AVFORMAT
	return NULL;	
}

audio_datasource* 
ffmpeg_audio_parser_finder::new_audio_parser(const std::string& url, audio_format_choices fmts, datasource *src)
{
	net::url   loc(url);
	
	audio_datasource *ds = NULL;
	if (src) {
			// XXXX Here we have to check for the mime type.
			if (!ffmpeg_decoder_datasource::supported(url)) {
				AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_parser_finder::new_audio_parser: no support for %s", url.c_str());
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
		lib::logger::get_logger()->warn("ffmpeg_audio_filter_finder: no support for %d-bit audio, only 16", fmt.bits);
		return NULL;
	}
	// XXXX Check that there is at least one destination format we understand too
	return new ffmpeg_resample_datasource(src, fmts);
}

// **************************** ffmpeg_parser_thread *****************************

#ifdef WITH_FFMPEG_AVFORMAT

unsigned long
detail::ffmpeg_parser_thread::run()
{
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser::run: started");
	while (!exit_requested()) {
		AVPacket pkt1, *pkt = &pkt1;
		// Read a packet
		int ret = av_read_packet(m_con, pkt);
		/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser::run: av_read_packet returned %d", ret);
		if (ret < 0) break;
		// Wait until there is room in the buffer
		while (m_parent->buffer_full())
			sleep(1);   // This is overdoing it
		/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser::run: calling data_avail(0x%x, %d)", pkt->data, pkt->size);
		m_parent->data_avail(pkt->pts, pkt->data, pkt->size);
		av_free_packet(pkt);
	}
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser::run: final data_avail(0, 0)");
	m_parent->data_avail(0, 0, 0);
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser::run: returning");
	return 0;
}
		
// **************************** ffmpeg_parser_datasource *****************************

ffmpeg_parser_datasource::ffmpeg_parser_datasource(const std::string& url, AVFormatContext *context)
:	m_url(url),
	m_con(context),
	m_fmt(audio_format(0, 0, 0)),
	m_src_end_of_file(false),
	m_event_processor(NULL),
	m_thread(NULL),
	m_client_callback(NULL)
{
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::ffmpeg_parser_datasource() -> 0x%x", (void*)this);
	ffmpeg_init();
	m_thread = new detail::ffmpeg_parser_thread(this, context);
	if (m_thread) {
		m_thread->start();
	} else {
		lib::logger::get_logger()->error("ffmpeg_parser_datasource::ffmpeg_parser_datasource: cannot start thread");
		m_src_end_of_file = true;
	}
}

ffmpeg_parser_datasource::~ffmpeg_parser_datasource()
{
	m_lock.enter();
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::~ffmpeg_parser_datasource(0x%x)", (void*)this);
	if (m_thread) m_thread->stop();
	delete m_thread;
	m_thread = NULL;
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::~ffmpeg_parser_datasource: thread stopped");
	if (m_con) av_close_input_file(m_con);
	m_con = NULL;
	m_lock.leave();
}	

void 
ffmpeg_parser_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	
	if (m_client_callback != NULL)
		/*AM_DBG*/ lib::logger::get_logger()->error("ffmpeg_parser_datasource::start(): m_client_callback already set!");
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (evp && callbackk) {
			/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::high);
		} else {
			/*AM_DBG*/ lib::logger::get_logger()->error("ffmpeg_parser_datasource::start(): no client callback!");
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
		
		lib::logger::get_logger()->trace("ffmpeg_parser_datasource::start(): Should start reading again");
//		lib::event *e = new readdone_callback(this, &ffmpeg_parser_datasource::data_avail);
//		/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
//		m_src->start(m_event_processor,  e);
	}
	m_lock.leave();
}
 
void 
ffmpeg_parser_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource.readdone : done with %d bytes", len);
//	restart_input();
	m_lock.leave();
}

void 
ffmpeg_parser_datasource::data_avail(int64_t pts, uint8_t *inbuf, int sz)
{
	// XXX timestamp is ignored, for now
	m_lock.enter();
	m_src_end_of_file = (sz == 0);
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource.data_avail: %d bytes available", sz);
	if(sz && !m_buffer.buffer_full()){
		uint8_t *outbuf = (uint8_t*) m_buffer.get_write_ptr(sz);
		if (outbuf) {
			memcpy(outbuf, inbuf, sz);
			m_buffer.pushdata(sz);
			// XXX m_src->readdone(sz);
		}
	}

	if ( m_client_callback && (m_buffer.buffer_not_empty() || m_src_end_of_file ) ) {
		/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::data_avail(): calling client callback");
		m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::high);
		m_client_callback = NULL;
		//m_event_processor = NULL;
	} else {
		/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::data_avail(): No client callback!");
	}
	m_lock.leave();
}


bool 
ffmpeg_parser_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool 
ffmpeg_parser_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	return m_src_end_of_file;
}

bool 
ffmpeg_parser_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}	

char* 
ffmpeg_parser_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

int 
ffmpeg_parser_datasource::size() const
{
		return m_buffer.size();
}	


audio_format&
ffmpeg_parser_datasource::get_audio_format()
{
#if 0
	if (m_con) {
		// Refresh info on audio format
		m_fmt.samplerate = m_con->sample_rate;
		m_fmt.bits = 16; // XXXX
		m_fmt.channels = m_con->channels;
		/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::select_decoder: rate=%d, bits=%d,channels=%d",m_fmt.samplerate, m_fmt.bits, m_fmt.channels);
	}
#endif
	return m_fmt;
}

AVFormatContext *
ffmpeg_parser_datasource::supported(const std::string& url)
{
	ffmpeg_init();
	// Setup struct to allow ffmpeg to determine whether it supports this
	AVInputFormat *fmt;
	AVProbeData probe_data;
	
	probe_data.filename = url.c_str();
	probe_data.buf = NULL;
	probe_data.buf_size = 0;
	fmt = av_probe_input_format(&probe_data, 0);
	/*AM_DBG*/ lib::logger::get_logger()->trace("ffmpeg_parser_datasource::supported(%s): av_probe_input_format: 0x%x", url.c_str(), (void*)fmt);
	AVFormatContext *ic;
	int err = av_open_input_file(&ic, url.c_str(), fmt, 0, 0);
	if (err) {
		lib::logger::get_logger()->warn("ffmpeg_parser_datasource::supported(%s): av_open_input_file returned error %d, ic=0x%x", url.c_str(), err, (void*)ic);
		if (ic) av_close_input_file(ic);
	}
	return ic;
}

#endif // WITH_FFMPEG_AVFORMAT

// **************************** ffpmeg_decoder_datasource *****************************

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(const std::string& url, datasource *const src)
:	m_url(url),
	m_con(NULL),
	m_fmt(audio_format(0, 0, 0)),
	m_event_processor(NULL),
	m_src(src),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource() -> 0x%x", (void*)this);
	ffmpeg_init();
	const char *ext = getext(m_url);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource: Selecting \"%s\" decoder", ext);
	if (!select_decoder(ext))
		lib::logger::get_logger()->error("ffmpeg_decoder_datasource: could not select \"%s\" decoder", ext);
}

ffmpeg_decoder_datasource::~ffmpeg_decoder_datasource()
{
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::~ffmpeg_decoder_datasource(0x%x)", (void*)this);
	if (m_con) avcodec_close(m_con);
	m_con = NULL;
	if (m_src) m_src->release();
	m_src = NULL;
}	

int
ffmpeg_decoder_datasource::decode(uint8_t* in, int sz, uint8_t* out, int &outsize)
{
	return avcodec_decode_audio(m_con, (short*) out, &outsize, in, sz);
}
		  
void 
ffmpeg_decoder_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	
	if (m_client_callback != NULL)
		AM_DBG lib::logger::get_logger()->error("ffmpeg_decoder_datasource::start(): m_client_callback already set!");
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (evp && callbackk) {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::high);
		} else {
			AM_DBG lib::logger::get_logger()->error("ffmpeg_decoder_datasource::start(): no client callback!");
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
		lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(m_event_processor,  e);
	}
	m_lock.leave();
}
 
void 
ffmpeg_decoder_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource.readdone : done with %d bytes", len);
//	if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
//		m_src->start(m_event_processor, m_readdone);
//	}
	m_lock.leave();
}

void 
ffmpeg_decoder_datasource::data_avail()
{
	m_lock.enter();
	int sz = m_src->size();
	if (m_con) {
		uint8_t *inbuf = (uint8_t*) m_src->get_read_ptr();
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource.data_avail: %d bytes available", sz);
		if(!m_buffer.buffer_full()){
			int outsize = 20*sz;
			uint8_t *outbuf = (uint8_t*) m_buffer.get_write_ptr(outsize);
			if (outbuf) {
				int decoded = avcodec_decode_audio(m_con, (short*) outbuf, &outsize, inbuf, sz);
				AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource.data_avail : %d bps",m_con->sample_rate);
				AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource.data_avail : %d bytes decoded  to %d bytes", decoded,outsize );
				m_buffer.pushdata(outsize);
				m_src->readdone(decoded);
			}
		}
		// Restart reading if we still have room to accomodate more data
		if (!m_src->end_of_file() && m_event_processor && !m_buffer.buffer_full()) {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::data_avail(): calling m_src->start() again");
			lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
			m_src->start(m_event_processor, e);
		} else {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::data_avail: not calling start: eof=%d m_ep=0x%x buffull=%d", 
				(int)m_src->end_of_file(), (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
	
		if ( m_client_callback && (m_buffer.buffer_not_empty() || m_src->end_of_file()) ) {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::data_avail(): calling client callback");
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::high);
			m_client_callback = NULL;
			//m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::data_avail(): No client callback!");
		}
	} else {
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::data_avail(): No decoder, flushing available data");
		m_src->readdone(sz);
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
		return m_buffer.size();
}	

bool 
ffmpeg_decoder_datasource::select_decoder(const char* file_ext)
{
	AVCodec *codec = avcodec_find_decoder_by_name(file_ext);
	if (codec == NULL) {
			lib::logger::get_logger()->error("ffmpeg_decoder_datasource.select_decoder: Failed to find codec for \"%s\"", file_ext);
			return false;
	}
	m_con = avcodec_alloc_context();
	
	if(avcodec_open(m_con,codec) < 0) {
			lib::logger::get_logger()->error("ffmpeg_decoder_datasource.select_decoder: Failed to open avcodec for \"%s\"", file_ext);
			return false;
	}
	return true;
}

audio_format&
ffmpeg_decoder_datasource::get_audio_format()
{
	if (m_con) {
		// Refresh info on audio format
		m_fmt.samplerate = m_con->sample_rate;
		m_fmt.bits = 16; // XXXX
		m_fmt.channels = m_con->channels;
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource::select_decoder: rate=%d, bits=%d,channels=%d",m_fmt.samplerate, m_fmt.bits, m_fmt.channels);
	}
	return m_fmt;
}

bool
ffmpeg_decoder_datasource::supported(const std::string& url)
{
	ffmpeg_init();
	const char *file_ext = getext(url);
	AVCodec  *codec = avcodec_find_decoder_by_name(file_ext);
	if (codec == NULL) lib::logger::get_logger()->warn("ffmpeg_decoder_datasource: no such decoder: \"%s\"", file_ext);
	return codec != NULL;
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
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::ffmpeg_resample_datasource() : constructor");
#ifdef RESAMPLE_READ_ALL
	m_buffer.set_max_size(0);
#endif
}

ffmpeg_resample_datasource::~ffmpeg_resample_datasource() 
{
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::~ffmpeg_resample_datasource() : destructor");
	if (m_src) m_src->release();
	m_src = NULL;
	if (m_resample_context) audio_resample_close(m_resample_context);
	m_resample_context = NULL;
}

void
ffmpeg_resample_datasource::data_avail()
{
	m_lock.enter();
	// We now have enough information to determine the resample parameters
	if (!m_context_set) {
		m_in_fmt = m_src->get_audio_format();
		assert(m_in_fmt.bits == 16);
		assert(m_out_fmt.bits == 16);
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource: initializing context: inrate, ch=%d, %d, outrate, ch=%d, %d", m_in_fmt.samplerate,  m_in_fmt.channels, m_out_fmt.samplerate,  m_out_fmt.channels);
		m_resample_context = audio_resample_init(m_out_fmt.channels, m_in_fmt.channels, m_out_fmt.samplerate,m_in_fmt.samplerate);
		m_context_set = true;
	}
	int sz = m_src->size();
	if (m_resample_context) {
		// Convert all the input data we have available. We make an educated guess at the number of bytes
		// this will produce on output.
		int outsz = sz + 4;
		if (m_in_fmt.channels && m_in_fmt.samplerate) 
			outsz = (sz + 4) * (m_out_fmt.channels*m_out_fmt.samplerate) / (m_in_fmt.channels*m_in_fmt.samplerate);
		// DBG
		outsz = 20 * sz;
		assert( sz || m_src->end_of_file());
		if (sz & 1) lib::logger::get_logger()->warn("ffmpeg_resample_datasource::data_avail: warning: oddsized datasize %d", sz);
		short int *inbuf = (short int*) m_src->get_read_ptr();
		short int *outbuf = (short int*) m_buffer.get_write_ptr(outsz);
		if (inbuf && outbuf) {
			int resampled = audio_resample(m_resample_context, outbuf, inbuf, sz / 2);
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail(): resampled %d samples from %d", resampled, sz/2);
			m_buffer.pushdata(resampled*2);
			//XXXX : daniel wonders if audio_resample resamples everything that's in m_inbuf ?
			m_src->readdone(sz);
		}
		// Restart reading if we still have room to accomodate more data
		if (!m_src->end_of_file() && m_event_processor && !m_buffer.buffer_full()) {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail(): calling m_src->start() again");
			lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
			m_src->start(m_event_processor, e);
#ifdef RESAMPLE_READ_ALL
			// workaround for sdl bug: if RESAMPLE_READ_ALL is defined we continue
			// reading until we have all data
			m_lock.leave();
			return;
#endif /* RESAMPLE_READ_ALL */
		} else {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail: not calling start: eof=%d m_ep=0x%x buffull=%d", 
				(int)m_src->end_of_file(), (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
		// If the client is currently interested tell them about data being available
		if (m_client_callback && (m_buffer.buffer_not_empty() || m_src->end_of_file())) {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail(): calling client callback");
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::high);
			m_client_callback = NULL;
			//m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail(): No client callback!");
		}
	} else {
		// Something went wrong during initialization, we just drop the data
		// on the floor.
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail(): No resample context, flushing data");
		m_src->readdone(sz);
	}
	m_lock.leave();
}


void 
ffmpeg_resample_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_decoder_datasource.readdone : done with %d bytes", len);
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
	return m_src->end_of_file();
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
	// const method - cannot lock
	return m_buffer.size();
}	

void 
ffmpeg_resample_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::start(): start(0x%x) called", this);
	if (m_client_callback != NULL)
		AM_DBG lib::logger::get_logger()->error("ffmpeg_resample_datasource::start(): m_client_callback already set!");
	
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (evp && callbackk) {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::high);
		} else {
			AM_DBG lib::logger::get_logger()->error("ffmpeg_resample_datasource::start(): no client callback!");
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
		lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(m_event_processor,  e);
	}
	
	m_lock.leave();
}
