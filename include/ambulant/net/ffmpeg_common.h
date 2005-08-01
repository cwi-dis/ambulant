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
    audio_format& get_audio_format();
  	video_format& get_video_format();
	void cancel();
	timestamp_t get_clip_end(); 
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return 0; };
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
