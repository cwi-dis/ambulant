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

#ifndef AMBULANT_NET_FFMPEG_COMMON_H
#define AMBULANT_NET_FFMPEG_COMMON_H


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
#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/net/datasource.h"

#include "avcodec.h"
#include "avformat.h"
//#include "common.h"

// temporary debug messages
//#include <iostream>
#ifndef AMBULANT_NO_OSTREAM
//#include <ostream>
#else /*AMBULANT_NO_OSTREAM */
//#include <ostream.h>
#endif /*AMBULANT_NO_OSTREAM */
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

void ffmpeg_init();

class ffmpeg_codec_id {
  public:
	static ffmpeg_codec_id* instance();
  	~ffmpeg_codec_id() {};

	void add_codec(const char* codec_name, 	CodecID id);
	CodecID get_codec_id(const char* codec_name);
  private:
	ffmpeg_codec_id(); 
	static ffmpeg_codec_id* m_uniqueinstance;
	std::map<std::string, CodecID> m_codec_id;		  
};

class ffmpeg_demux : public abstract_demux {
  public:
	ffmpeg_demux(AVFormatContext *con, timestamp_t clip_begin, timestamp_t clip_end);
	~ffmpeg_demux();
	
	static AVFormatContext *supported(const net::url& url);
	  
	void add_datasink(demux_datasink *parent, int stream_index);
	void remove_datasink(int stream_index);
    int audio_stream_nr();
  	int video_stream_nr();
    // XXX this should also be timestamp_t instead of double
  	double duration();
  	int nstreams();
    void seek(timestamp_t time);
    void read_ahead(timestamp_t time);
    audio_format& get_audio_format();
  	video_format& get_video_format();
	void cancel();
	timestamp_t get_clip_end(); 
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_clip_begin; };
  protected:
	unsigned long run();
  private:
	audio_format m_audio_fmt;
  	video_format m_video_fmt;
	demux_datasink *m_sinks[MAX_STREAMS];
	AVFormatContext *m_con;
	int m_nstream;
	lib::critical_section m_lock;
  	timestamp_t m_clip_begin;
  	timestamp_t m_clip_end;
  	bool m_clip_begin_set;
};

}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_FFMPEG_COMMON_H
