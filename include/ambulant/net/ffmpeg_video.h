/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

#ifndef AMBULANT_NET_FFMPEG_VIDEO_H
#define AMBULANT_NET_FFMPEG_VIDEO_H


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
//#include "ambulant/net/databuffer.h"
//#include "ambulant/net/posix_datasource.h"
#include "ambulant/net/datasource.h"

extern "C" {
#include "avcodec.h"
#include "avformat.h"
//#include "common.h"
}

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

namespace ambulant
{

namespace net
{  

class ffmpeg_video_datasource_factory : public video_datasource_factory {
  public:
	~ffmpeg_video_datasource_factory() {};
	video_datasource* new_video_datasource(const net::url& url, timestamp_t clip_begin, timestamp_t clip_end);
};

typedef std::pair<timestamp_t, char*> ts_pointer_pair;

class sorted_frame_compare {
 public:
	bool operator () (const ts_pointer_pair left, const ts_pointer_pair right) {
    		return left.first > right.first;
  	}
};
typedef std::priority_queue<ts_pointer_pair, std::vector<ts_pointer_pair>, sorted_frame_compare > sorted_frames;

class ffmpeg_video_decoder_datasource:
	virtual public video_datasource,
	virtual public lib::ref_counted_obj {
  public:
	static bool supported(const video_format& fmt);
	
	 //ffmpeg_video_decoder_datasource(const net::url& url, datasource *src);
	 ffmpeg_video_decoder_datasource(video_datasource *src, video_format fmt);
	 
    ~ffmpeg_video_decoder_datasource();

	bool has_audio();
    int width();
  	int height();
	timestamp_t frameduration();
	audio_datasource *get_audio_datasource();

    void start_frame(lib::event_processor *evp, lib::event *callback, timestamp_t timestamp);  
	void stop();  

    bool end_of_file();
	char* get_frame(timestamp_t now, timestamp_t *timestamp, int *size);
	void frame_done(timestamp_t timestamp, bool keepdata);
	void read_ahead(timestamp_t clip_begin);
	void seek(timestamp_t time);
    void data_avail();
	bool buffer_full();
  	timestamp_t get_clip_end() { return m_src->get_clip_end(); };
  	timestamp_t get_clip_begin() { return m_src->get_clip_begin(); };
	timestamp_t get_start_time() { return m_src->get_start_time(); };
	common::duration get_dur();
	
  private:
	bool _select_decoder(const char* file_ext);
	bool _select_decoder(video_format &fmt);
	bool _end_of_file();
  	bool _clip_end();
	bool _buffer_full();
	void _pop_top_frame();
	void _need_fmt_uptodate();
	
	video_datasource* m_src;
	AVCodecContext *m_con;
	bool m_con_owned;	// True if we have to close/free m_con
//	int m_stream_index;
  	video_format m_fmt;
// 	bool m_src_end_of_file;
 	lib::event_processor *m_event_processor;
	sorted_frames  m_frames;
	ts_pointer_pair m_old_frame;
	int m_size;		// NOTE: this assumes all decoded frames are the same size!
//	databuffer m_buffer;
//	detail::ffmpeg_demux *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
  	timestamp_t m_pts_last_frame;
  	timestamp_t m_last_p_pts;
	timestamp_t m_video_clock;
  	int m_frame_count;
	int m_dropped_count;
    lib::critical_section m_lock;
	timestamp_t m_elapsed;
	bool m_start_input;		// True when m_src->start_frame() is needed
	//FILE* m_file;
  	
};

}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_FFMPEG_VIDEO_H
