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

#include "ambulant/net/raw_video_datasource.h"
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <unistd.h>


#ifndef AM_DBG
#define AM_DBG if(0)
#endif



using namespace ambulant;
using namespace net;

raw_video_datasource::raw_video_datasource(std::string &directory) 
:	m_filenr(0),
	m_size(0),
	m_eof(false),
	m_directory(directory)

{
	m_buffer = NULL;
}

raw_video_datasource::~raw_video_datasource()
{ 
}

void
raw_video_datasource::start_frame(lib::event_processor *evp, lib::event *cbevent, int timestamp)
{
	bool dummy;
	
	dummy = read_next_frame();
	if (dummy) {
    	if (evp && cbevent) {
			AM_DBG lib::logger::get_logger()->trace("raw_video_datasource.start: trigger readdone callback (x%x)", cbevent);
			evp->add_event(cbevent, 0, ambulant::lib::event_processor::high);
    	}
	}
}

char*
raw_video_datasource::get_frame(int &timestamp)
{
	timestamp = m_filenr;
	return m_buffer;
}

void
raw_video_datasource::frame_done(int timestamp)
{
	if (timestamp > m_filenr) {
		m_filenr = timestamp;
	}
	free (m_buffer);
	m_buffer = NULL;
}


int
raw_video_datasource::filesize(int stream)
{
 	using namespace std;
	int filesize;
	int dummy;
	if (stream >= 0) {
		// Seek to the end of the file, and get the filesize
		filesize=lseek(stream, 0, SEEK_END); 		
	 	dummy=lseek(stream, 0, SEEK_SET);		
		return filesize;		
	} else {
 		lib::logger::get_logger()->fatal("raw_video_datasource.filesize(): no file open");
		return 0;
	}
}

bool
raw_video_datasource::read_next_frame()
{
	char filename[255];
	int file;
	int sz;
	int nb_read;
	char* buf_ptr;

	m_filenr++;

	sprintf(filename, "%s/img%d.jpg", m_directory.c_str(),m_filenr);
	
	file = open(filename, O_RDONLY); 

	if (file < 0 ) {
			AM_DBG lib::logger::get_logger()->trace("active_datasource.read_next_frame(): Unable to open file");
			m_eof = true;
			return false;
	}

	sz = filesize(file);
	m_size = sz;
	m_buffer = (char*) malloc(sz);
	buf_ptr = m_buffer;
	do {
		nb_read = read(file, buf_ptr, sz);
		if (nb_read != sz) {
				lib::logger::get_logger()->fatal("raw_video_datasource.filesize(): Read error");
		}
	} while (nb_read < 1);
	close(file);
	buf_ptr = NULL;		
	
	return true;
}

bool
raw_video_datasource::end_of_file()
{
	return m_eof;
}
