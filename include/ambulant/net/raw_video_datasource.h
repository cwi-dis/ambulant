/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

	
#ifndef _RAW_VIDEO_DATASOURCE_H
#define _RAW_VIDEO_DATASOURCE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/databuffer.h"



namespace ambulant {

namespace net {
	
class raw_video_datasource : virtual public video_datasource {
  public:
	raw_video_datasource(const std::string &directory);
  	~raw_video_datasource();
  
	bool has_audio() { return false; };
	audio_datasource *get_audio_datasource() { return NULL; };

	void start_frame(lib::event_processor *evp, lib::event *callback, double timestamp);
  	
  	bool end_of_file();
  	
  	char* get_frame(double *timestamp, int *size); 
  	void frame_done(double timestamp, bool keepdata);
  
  private:
	bool read_next_frame();	 
  	int filesize(int stream);
  
	int m_filenr;
  	long int m_size;
  	bool m_eof;
	std::string m_directory;  
  	char* m_buffer;
	lib::critical_section m_lock;
};

}
}

#endif /* _RAW_VIDEO_DATASOURCE_H */
