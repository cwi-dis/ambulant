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

namespace ambulant {

using namespace ambulant;
using namespace net;

raw_video_datasource(string &directory)
:	m_filenr(0),
	m_size(0),
	m_eof(false),
	m_directory(directory)
	m_buffer(NULL);
{
}

~raw_video_datasource()
{
}

void
start_frame(lib::event_processor *evp, lib::event *cbevent, int timestamp)
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
get_frame(int &timestamp);
{
	timestamp = m_filenr;
	return m_buffer;
}

void
frame_done(int timestamp)
{
	if (timestamp > filenr) {
		filenr = timestamp;
	}
	free (mbuffer);
	m_buffer = NULL;
}


int
active_datasource::filesize(int stream)
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
 		lib::logger::get_logger()->fatal("active_datasource.filesize(): no file openXX");
		return 0;
	}
}

bool
read_next_frame()
{
	char ext[4];
	char filename[12];
	int file;
	int sz;
	int nb_read;
	char* buf_ptr;
	
	
	m_filenr++;
	strcpy(ext,".jpg")
	itoa(m_filenr,filename);sz = filesize
	strncat(filename,ext,4);
	
		file = open(filename, O_RDONLY);
		if (file < 0 ) {
				m_eof = true;
				return false;
		}
		sz = filesize(file);
		m_size = sz;
		m_buffer = (char*) malloc(sz);
		buf_ptr = m_buffer;
		do {
			nb_read = read(file, buf_ptr, BUFSIZ);
			if (nb_read > 0) 
					buf_ptr+=nb_read;
		} while (nb_read < 1)
		close(file);
		buf_ptr = NULL;		
		
		return true;
}

bool
end_of_file()
{
	return m_eof;
}
