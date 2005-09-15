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


#ifndef AMBULANT_NET_RTSP_DATASOURCE_H
#define AMBULANT_NET_RTSP_DATASOURCE_H

#include "ambulant/config/config.h"

#define MAX_RTP_FRAME_SIZE 50000

#ifdef AMBULANT_PLATFORM_MACOS
// Both MacHeaders.h and Live typedef Boolean, but to imcompatible
// types.
#define Boolean LiveBoolean
// Similarly for EventTime
#define EventTime LiveEventTime
#endif

// LiveMedia includes
#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"


#include "avcodec.h"
#include "avformat.h"
#include "common.h"

#include "ambulant/config/config.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/net/databuffer.h"
//#include "ambulant/net/posix_datasource.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/ffmpeg_common.h"

namespace ambulant
{

namespace net
{

struct rtsp_context_t {
	RTSPClient* rtsp_client;
  	MediaSession* media_session;
	char* sdp;
	int audio_stream;
	int video_stream;
	unsigned char* audio_packet;
	unsigned char* video_packet;
	unsigned char* video_buffer;
	int video_buffer_size;
	timestamp_t last_pts;
	bool need_audio;
	bool need_video;
	int nstream;
	char blocking_flag;
	bool eof;
	timestamp_t clip_end;
	bool is_clip_end;
	const char* audio_codec_name;
	const char* video_codec_name;
	timeval first_sync_time;
	audio_format audio_fmt;	
	video_format video_fmt;
	demux_datasink *sinks[MAX_STREAMS];
};
	
class rtsp_demux : public abstract_demux {
  public:
	rtsp_demux(rtsp_context_t* context, timestamp_t clip_begin, timestamp_t clip_end);
	
	static rtsp_context_t* supported(const net::url& url);
	
	void add_datasink(demux_datasink *parent, int stream_index);
	void remove_datasink(int stream_index);
	void cancel() {};
  	int audio_stream_nr() { return m_context->audio_stream; };
	int video_stream_nr() { return m_context->video_stream; };  
	int nstreams() { return m_context->nstream; };
	double duration(){ return 0.0; };
	audio_format& get_audio_format() { return m_context->audio_fmt; };
	video_format& get_video_format() { return m_context->video_fmt; };
	void seek(timestamp_t time);
	void set_position(timestamp_t time);
	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_clip_begin; };
  protected:
	unsigned long run();
  private:	
	rtsp_context_t* m_context;
  	timestamp_t m_clip_begin;
	timestamp_t m_clip_end;
  	bool m_clip_begin_set;
};


}
} //end namespaces

#endif
