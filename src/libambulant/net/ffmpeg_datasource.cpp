
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

#ifndef AM_DBG
#define AM_DBG if(0)
#endif


using namespace ambulant;
using namespace net;

typedef lib::no_arg_callback<ffmpeg_audio_datasource> readdone_callback;
typedef lib::no_arg_callback<ffmpeg_resample_datasource> resample_callback;

#define INBUF_SIZE 4096


audio_datasource* 
ffmpeg_audio_parser_finder::new_audio_parser(const std::string& url, audio_format_choices fmts, datasource *src)
{
	net::url   loc(url);
	
	audio_datasource *ds = NULL;
	if (src) {
			// XXXX Here we have to check for the mime type.
			ds = new ffmpeg_audio_datasource(src);
			// XXX Here we have to check if ext & fmt are supported by ffmpeg
			if (ds) return ds;			
		}
	return NULL;	
}
		
ffmpeg_audio_datasource::ffmpeg_audio_datasource(datasource *const src)
:	m_codec(NULL),
	m_con(NULL),
	m_fmt(audio_format(0, 0, 0)),
	m_event_processor(NULL),
	m_src(src),
	m_inbuf(NULL),
	m_outbuf(NULL),
//	m_buffer
	m_blocked_full(false),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource::ffmpeg_audio_datasource() -> 0x%x", (void*)this);
	init();
}

ffmpeg_audio_datasource::~ffmpeg_audio_datasource()
{
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource::~ffmpeg_audio_datasource(0x%x)", (void*)this);
	// XXXX free m_con?
	// XXXX free m_codec?
}	

int
ffmpeg_audio_datasource::decode(uint8_t* in, int size, uint8_t* out, int &outsize)
{
	return avcodec_decode_audio(m_con, (short*) out, &outsize, in, size);
}
		  
void 
ffmpeg_audio_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	
	if (m_client_callback != NULL)
		AM_DBG lib::logger::get_logger()->error("ffmpeg_audio_datasource::start(): m_client_callback already set!");
	if (m_buffer.buffer_not_empty() || m_src->end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (evp && callbackk) {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::event_processor::high);
		} else {
			AM_DBG lib::logger::get_logger()->error("ffmpeg_audio_datasource::start(): no client callback!");
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		m_client_callback = callbackk;
		m_event_processor = evp;
		lib::event *e = new readdone_callback(this, &ffmpeg_audio_datasource::callback);
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(m_event_processor,  e);
	}
	m_lock.leave();
}
 
void 
ffmpeg_audio_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource.readdone : done with %d bytes", len);
//	if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
//		m_src->start(m_event_processor, m_readdone);
//	}
	m_lock.leave();
}

void 
ffmpeg_audio_datasource::callback()
{
	int result;
	int size;
	int outsize;
	int decoded;
	int blocksize;
	const int max_block = INBUF_SIZE +  FF_INPUT_BUFFER_PADDING_SIZE;
	

	m_lock.enter();
	if(!m_codec) {
		select_decoder("mp3"); // XXX should get from m_src
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource.callback : Selected the MP3 decoder");
	}

	if (m_con) {
	    size = m_src->size();
		//while (size > 0 && !m_buffer.buffer_full()) {
		m_inbuf = (uint8_t*) m_src->get_read_ptr();
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource.callback: %d bytes available", size);
		if(!m_buffer.buffer_full()){
		m_outbuf = (uint8_t*) m_buffer.get_write_ptr(20*size);
		decoded = avcodec_decode_audio(m_con, (short*) m_outbuf, &outsize, m_inbuf, size);
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource.callback : %d bps",m_con->sample_rate);
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource.callback : %d bytes decoded  to %d bytes", decoded,outsize );
		m_buffer.pushdata(outsize);
		m_src->readdone(decoded);
		}
		//}
	
		if ( m_client_callback ) {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource::callback(): calling client callback");
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::high);
			m_client_callback = NULL;
			m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource::callback(): No client callback!");
		}
	} else {
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource::callback(): No decoder, flushing available data");
		m_src->readdone(size);
		m_lock.leave();
		return;
	}
		
//	if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
//		m_src->start(m_event_processor, m_readdone);
//	} else {
//		m_blocked_full = true;
//	}
	m_lock.leave();
}


bool 
ffmpeg_audio_datasource::end_of_file()
{
	if (m_buffer.buffer_not_empty()) return false;
	return m_src->end_of_file();
}

bool 
ffmpeg_audio_datasource::buffer_full()
{
	return m_buffer.buffer_full();
}	

char* 
ffmpeg_audio_datasource::get_read_ptr()
{
	return m_buffer.get_read_ptr();
}

int 
ffmpeg_audio_datasource::size() const
{
		return m_buffer.size();
}	

int 
ffmpeg_audio_datasource::select_decoder(char* file_ext)
{
	m_codec = avcodec_find_decoder_by_name(file_ext);
	if (m_codec == NULL) {
			lib::logger::get_logger()->error("ffmpeg_audio_datasource.select_decoder : Failed to open codec");
	}
	m_con = avcodec_alloc_context();
	
	if(avcodec_open(m_con,m_codec) < 0) {
			lib::logger::get_logger()->error("ffmpeg_audio_datasource.select_decoder : Failed to open avcodec");
	}
	m_fmt.samplerate = m_con->sample_rate;
	m_fmt.bits = 16; // XXXX
	m_fmt.channels = m_con->channels;
}

int 
ffmpeg_audio_datasource::init()
{
		avcodec_init();
		avcodec_register_all();
}

ffmpeg_resample_datasource::ffmpeg_resample_datasource(audio_datasource *src, audio_format_choices fmts) 
:	m_src(src),
	m_context_set(false),
	m_resample_context(NULL),
	m_inbuf(NULL),
	m_outbuf(NULL),
	m_blocked_full(false),
	m_event_processor(NULL),
	m_client_callback(NULL),
	m_in_fmt(src->get_audio_format()),
	m_out_fmt(fmts.best())
{
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::ffmpeg_resample_datasource() : constructor");
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::ffmpeg_resample_datasource() : IN : %d, %d", m_in_fmt.samplerate,  m_in_fmt.channels);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::ffmpeg_resample_datasource() : OUT : %d, %d", m_out_fmt.samplerate,  m_out_fmt.channels);
	assert(m_in_fmt.bits == 16);
	assert(m_out_fmt.bits == 16);
	m_resample_context = audio_resample_init(m_out_fmt.channels, m_in_fmt.channels, m_out_fmt.samplerate,m_in_fmt.samplerate);
	m_context_set = true;
	
}

ffmpeg_resample_datasource::~ffmpeg_resample_datasource() 
{
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::~ffmpeg_resample_datasource() : destructor");
}

void
ffmpeg_resample_datasource::data_avail()
{
  int resampled;
  int size;

  if (m_context_set) {
	size = m_src->size();
	m_inbuf = (short int*) m_src->get_read_ptr();
	m_outbuf = (short int*) m_buffer.get_write_ptr(20*size);	
	resampled = audio_resample(m_resample_context, m_outbuf, m_inbuf, size / 2);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail(): resampled %d samples", resampled);
    m_buffer.pushdata(resampled*2);
	//XXXX : daniel wonders if audio_resample resamples everything that's in m_inbuf ?
	m_src->readdone(size);
	if (m_client_callback) {
	   AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail(): calling client callback");
	   m_event_processor->add_event(m_client_callback, 0, ambulant::lib::event_processor::high);
	   m_client_callback = NULL;
	   m_event_processor = NULL;
   	} else {
		AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::callback(): No client callback!");
	}

  } else {
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::data_avail(): No resample context flusshing data");
	m_src->readdone(size);
  }
}


void 
ffmpeg_resample_datasource::readdone(int len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_audio_datasource.readdone : done with %d bytes", len);
	m_lock.leave();
}

bool 
ffmpeg_resample_datasource::end_of_file()
{
	if (m_buffer.buffer_not_empty()) return false;
	return m_src->end_of_file();
}

bool 
ffmpeg_resample_datasource::buffer_full()
{
	return m_buffer.buffer_full();
}	

char* 
ffmpeg_resample_datasource::get_read_ptr()
{
	return m_buffer.get_read_ptr();
}

int 
ffmpeg_resample_datasource::size() const
{
	return m_buffer.size();
}	

#if 0
void	
ffmpeg_resample_datasource::get_input_format(audio_context &fmt)
{
	fmt.samplerate = m_in_fmt.samplerate;
	fmt.channels = m_in_fmt.channels;
	fmt.bits = m_in_fmt.bits;
}

void
ffmpeg_resample_datasource::get_output_format(audio_context &fmt)
{ 
	fmt.samplerate = m_out_fmt.samplerate;
	fmt.channels = m_out_fmt.channels;
	fmt.bits = m_out_fmt.bits;
}
#endif

void 
ffmpeg_resample_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("ffmpeg_resample_datasource::start(): start(0x%x) called", this);
	if (m_client_callback != NULL)
		AM_DBG lib::logger::get_logger()->error("ffmpeg_resample_datasource::start(): m_client_callback already set!");
	
	if (m_buffer.buffer_not_empty() || m_src->end_of_file() ) {
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
