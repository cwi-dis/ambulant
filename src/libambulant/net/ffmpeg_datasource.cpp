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

#include "net/ffmpeg_datasource.h"
#include "avcodec.h"



using namespace ambulant;
bool net::ffmpeg_audio_datasource::m_ffmpeg_init = false;

net::ffmpeg_audio_datasource::ffmpeg_audio_datasource(active_datasource *const src)
{
	m_src = src;
}

net::ffmpeg_audio_datasource::~ffmpeg_audio_datasource()
{
}	
		  
void 
net::ffmpeg_audio_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback)
{
if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
		m_src->start(evp, callback);
	} else {
		m_blocked_full = true;
	}
	
	if( m_src->size() > 0 ) {
		callbackk();
	} else {
		m_client_waiting = true;
	}
}
 
void 
net::ffmpeg_audio_datasource::readdone(int len)
{
	m_buffer->readdone(size);
	if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
		m_src->start(evp, callback);
	}
}

void callback()
{
	int size;
	int outsize;
	int decoded;
	
	
	
	m_inbuf = m_src->get_ptr();
	size = m_src->size();
	m_outbuf = m_buffer->prepare();
	decoded = decode(m_inbuf, size, m_outbuf, outsize);
	m_buffer->pushdata(outsize)
	m_src->readdone(decoded);
	
	if ( m_client_waiting ) {
		m_client_waiting = false;
		callback();
	}
		
	if(( !m_src->buffer_full() && !m_src->end_of_file() )) {
		m_src->start(evp, callback);
	} else {
		m_blocked_full = true;
	}
}


bool end_of_file()
{
	return m_src->end_of_file();
}

bool buffer_full()
{
	return m_buffer->is_full();
}	

char* read_ptr()
{
	return m_src->read_ptr();
}

int size() const;   
{
	return m_src->size();
}	

int get_nchannels();
{
	
}

int get_nbits ();

int get_bitrate ();

int select_decoder(char* file_ext)
{
	m_codec = avcodec_find_decoder_by_name(m_ext.cstr());
}

int init()
{
	if (!m_ffmpeg_init) {
		avcodec_init();
		avcodec_register_all();
	}
}
