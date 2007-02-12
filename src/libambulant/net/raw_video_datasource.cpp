// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/net/raw_video_datasource.h"
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <unistd.h>



//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
#ifndef round
#define round(v) ((int)(v+0.5))
#endif

#define FRAME_RATE 15

using namespace ambulant;
using namespace net;

raw_video_datasource::raw_video_datasource(const std::string &directory) 
:	m_filenr(0),
	m_size(0),
	m_eof(false),
	m_directory(directory)

{
	m_buffer = NULL;
}

raw_video_datasource::~raw_video_datasource()
{
	m_lock.enter();
	if (m_buffer) free(m_buffer);
	m_buffer = NULL;
	m_lock.leave();
}

void
raw_video_datasource::start_frame(lib::event_processor *evp, lib::event *cbevent, double timestamp)
{
	m_lock.enter();
	bool dummy;
	AM_DBG lib::logger::get_logger()->debug("raw_video_datasource.start (0x%x)", (void*) this);
	if (m_buffer) {
		lib::logger::get_logger()->error("Internal error: raw_video_datasource::start_frame: Previous frame not frame_done()d yet!");
		m_buffer = NULL;
	}
	dummy = read_next_frame();
	AM_DBG lib::logger::get_logger()->debug("raw_video_datasource.start (0x%x): read_next_frame returned %d", (void*) this, dummy);
	if (1) {
    	if (evp && cbevent) {
			AM_DBG lib::logger::get_logger()->debug("raw_video_datasource.start: trigger readdone callback (x%x)", cbevent);
			evp->add_event(cbevent, 0, ambulant::lib::ep_med);
    	}
	}
	m_lock.leave();
}

char*
raw_video_datasource::get_frame(double *timestamp, int *size)
{
	m_lock.enter();
	*timestamp =  (double) m_filenr/FRAME_RATE;
	*size = m_size;
	char *rv = m_buffer;
	m_lock.leave();
	return rv;
}

void
raw_video_datasource::frame_done(double timestamp, bool keepdata)
{
	assert(!keepdata);
	m_lock.enter();
	if (timestamp > (double) m_filenr/FRAME_RATE) {
		m_filenr = (int)round(timestamp*FRAME_RATE);
	}
	if (m_buffer) {
		free (m_buffer);
		m_buffer = NULL;
	} else {
		lib::logger::get_logger()->error("Internal error: raw_video_datasource::frame_done: no current frame!");
	}
	m_lock.leave();
}


int
raw_video_datasource::filesize(int stream)
{
	// private method - no locking
 	using namespace std;
	int sz;
	int dummy;
	if (stream >= 0) {
		// Seek to the end of the file, and get the filesize
		sz=lseek(stream, 0, SEEK_END); 		
	 	dummy=lseek(stream, 0, SEEK_SET);		
		return sz;		
	} else {
 		lib::logger::get_logger()->fatal("raw_video_datasource.filesize(): no file open");
		return 0;
	}
}

bool
raw_video_datasource::read_next_frame()
{
	// private method - no locking
	char filename[255];
	int file;
	int sz;
	int nb_read;
	
	m_filenr++;
	AM_DBG lib::logger::get_logger()->debug("raw_video_datasource.read_next_frame(): framenr : %d", m_filenr);
	
	sprintf(filename, "%s/0%07i.jpg", m_directory.c_str(), m_filenr);
	AM_DBG lib::logger::get_logger()->debug("raw_video_datasource.read_next_frame(): Opening %s", filename);
	file = open(filename, O_RDONLY); 

	if (file < 0 ) {
			AM_DBG lib::logger::get_logger()->debug("raw_video_datasource.read_next_frame(): Unable to open file");
			m_eof = true;
			m_buffer = NULL;
			return false;
	}

	sz = filesize(file);
	m_size = sz;
	m_buffer = (char*) malloc(sz);
	AM_DBG lib::logger::get_logger()->debug("raw_video_datasource.read_next_frame(): m_buffer = 0x%x", (void*) m_buffer);

	do {
		nb_read = read(file, m_buffer, sz);
		if (nb_read != sz) {
			lib::logger::get_logger()->fatal("raw_video_datasource.filesize(): Read error");
		}
	} while (nb_read < 1);
	close(file);
		
	
	return true;
}

bool
raw_video_datasource::end_of_file()
{
	return m_eof;
}
