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


#ifndef AM_DBG
#define AM_DBG if(0)
#endif


using namespace ambulant;

typedef lib::no_arg_callback<net::ffmpeg_audio_datasource> readdone_callback;
bool net::ffmpeg_audio_datasource::m_codec_selected = false;
bool net::ffmpeg_audio_datasource::m_avcodec_open = false;


net::ffmpeg_audio_datasource::ffmpeg_audio_datasource(abstract_active_datasource *const src, lib::event_processor *const evp) : m_event_processor(evp)
{
	m_src = src;
	m_readdone = new readdone_callback(this, &net::ffmpeg_audio_datasource::callback);
	init();
}

net::ffmpeg_audio_datasource::~ffmpeg_audio_datasource()
{
}	

int
net::ffmpeg_audio_datasource::decode(uint8_t* in, int size, uint8_t* out, int &outsize)
{
	return avcodec_decode_audio(m_con, (short*) out, &outsize, in, size);
}
		  
void 
net::ffmpeg_audio_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
		m_src->start(m_event_processor,  m_readdone);
	} else {
		m_blocked_full = true;
	}
	
	if( m_src->size() > 0 ) {
		if (evp && callbackk) {
			AM_DBG lib::logger::get_logger()->trace("active_datasource.start: trigger readdone callback");
		evp->add_event(callbackk, 0, ambulant::lib::event_processor::high);
	} else {
		m_client_waiting = true;
	}
}
}
 
void 
net::ffmpeg_audio_datasource::readdone(int len)
{
	m_buffer.readdone(len);
	if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
		m_src->start(m_event_processor, m_readdone);
	}
}

void 
net::ffmpeg_audio_datasource::callback()
{
	int result;
	int size;
	int outsize;
	int decoded;
	char* in_ptr;
	
	
	
	in_ptr = m_src->read_ptr();
	m_inbuf = (uint8_t*) in_ptr;
	size = m_src->size();
	m_outbuf = (uint8_t*) m_buffer.prepare();
	if(!m_codec_selected) {
		select_decoder("mp3");
		m_codec_selected = true;
	}
	if(!m_avcodec_open) {
		result = avcodec_open(m_con, m_codec);
    	if ( result < 0) {
			lib::logger::get_logger()->error("ffmpeg_audio_datasource.callback : Failed to open avcodec");
		}
		m_avcodec_open = true;
	}
	decoded = decode(m_inbuf, size, m_outbuf, outsize);
	m_buffer.pushdata(outsize);
	m_src->readdone(decoded);
	
	if ( m_client_waiting ) {
		m_client_waiting = false;
		callback();
	}
		
	if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
		m_src->start(m_event_processor, m_readdone);
	} else {
		m_blocked_full = true;
	}
}


bool 
net::ffmpeg_audio_datasource::end_of_file()
{
	return m_src->end_of_file();
}

bool 
net::ffmpeg_audio_datasource::buffer_full()
{
	return m_buffer.is_full();
}	

char* 
net::ffmpeg_audio_datasource::read_ptr()
{
	return m_buffer.get_read_ptr();
}

int 
net::ffmpeg_audio_datasource::size() const
{
		return m_buffer.used();
}	

int 
net::ffmpeg_audio_datasource::get_nchannels()
{
	return m_con->channels;
}

int 
net::ffmpeg_audio_datasource::get_nbits ()
{
	// XXX This should not be a fixed number ofcourse !
	return 16;	
}

int 
net::ffmpeg_audio_datasource::get_samplerate ()
{
	m_con->sample_rate;
}

int 
net::ffmpeg_audio_datasource::select_decoder(char* file_ext)
{
	m_codec = avcodec_find_decoder_by_name(file_ext);
	if (m_codec == NULL) {
			lib::logger::get_logger()->error("ffmpeg_audio_datasource.select_decoder : Failed to open codec");
	}
}

int 
net::ffmpeg_audio_datasource::init()
{
		avcodec_init();
		avcodec_register_all();
		m_con = avcodec_alloc_context();
}
