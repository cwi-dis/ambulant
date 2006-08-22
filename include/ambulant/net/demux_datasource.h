/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
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

#ifndef AMBULANT_NET_DEMUX_DATASOURCE_H
#define AMBULANT_NET_DEMUX_DATASOURCE_H


//#include <vector>
//#include <queue>

//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>


#include "ambulant/config/config.h"
//#include "ambulant/lib/callback.h"
//#include "ambulant/lib/refcount.h"
//#include "ambulant/lib/event_processor.h"
//#include "ambulant/lib/mtsync.h"
//#include "ambulant/lib/event_processor.h"
//#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/net/databuffer.h"
//#include "ambulant/net/posix_datasource.h"
#include "ambulant/net/datasource.h"

//#include "avcodec.h"
//#include "avformat.h"
//#include "common.h"

// temporary debug messages
//#include <iostream>
//#ifndef AMBULANT_NO_OSTREAM
//#include <ostream>
//#else /*AMBULANT_NO_OSTREAM */
//#include <ostream.h>
//#endif /*AMBULANT_NO_OSTREAM */
//#include <cstring>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <map>



struct video_frame {
	char* data;
	int	  size;
};

namespace ambulant
{

namespace net
{  

class demux_audio_datasource: 
	virtual public audio_datasource,
	public demux_datasink,
	virtual public lib::ref_counted_obj
{
  public:
	 static demux_audio_datasource *new_demux_audio_datasource(
  		const net::url& url, abstract_demux *thread);
  	
  		demux_audio_datasource(
  		const net::url& url, 
  		abstract_demux *thread, 
  		int stream_index);
  
    ~demux_audio_datasource();

    void start(lib::event_processor *evp, lib::event *callback);
	void stop();  
	void read_ahead(timestamp_t clip_begin);
  	void seek(timestamp_t time);
    void readdone(int len);
    void data_avail(timestamp_t pts, const uint8_t *data, int size);
    bool end_of_file();
	bool buffer_full();
  	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
  	timestamp_t get_start_time() { return m_thread->get_start_time(); };
	char* get_read_ptr();
	int size() const;   
	audio_format& get_audio_format();

	common::duration get_dur();

  private:
    bool _end_of_file();
	const net::url m_url;
	//AVFormatContext *m_con;
	int m_stream_index;
//	audio_format m_fmt;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;

	databuffer m_buffer;
	abstract_demux *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
};

typedef std::pair<timestamp_t, video_frame> ts_frame_pair;

class demux_video_datasource: 
	virtual public video_datasource,
	public demux_datasink,
	virtual public lib::ref_counted_obj
{
  public:
	 static demux_video_datasource *new_demux_video_datasource(
  		const net::url& url, abstract_demux *thread);
  	
  		demux_video_datasource(
  		const net::url& url, 
  		abstract_demux *thread, 
  		int stream_index);
  
    ~demux_video_datasource();
	void read_ahead(timestamp_t clip_end);
    void start_frame(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk, timestamp_t timestamp);
    void stop();  
	char* get_frame(timestamp_t now, timestamp_t *timestamp, int *sizep);
    void frame_done(timestamp_t timestamp, bool keepdata);
    void data_avail(timestamp_t pts, const uint8_t *data, int size);
    bool end_of_file();
	bool buffer_full();
  	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
  	timestamp_t get_start_time() { return m_thread->get_start_time(); };
  	int width();
  	int height();
	int frameduration();
  
    bool has_audio();
  	audio_datasource* get_audio_datasource();
		
	char* get_read_ptr();
	int size() const;   

	video_format& get_video_format();
	
	common::duration get_dur();

  private:
    bool _end_of_file();
	const net::url m_url;
	//AVFormatContext *m_con;
	int m_stream_index;
//	audio_format m_fmt;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;
	std::queue<ts_frame_pair > m_frames;
	ts_frame_pair m_old_frame;
	abstract_demux *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
  	audio_datasource* m_audio_src;
	lib::critical_section m_lock;
  	//FILE *m_file;
  	long long int m_frame_nr;
  
};



}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_DEMUX_DATASOURCE_H
