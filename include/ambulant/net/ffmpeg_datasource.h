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


#ifndef AMBULANT_NET_FFMPEG_DATASOURCE_H
#define AMBULANT_NET_FFMPEG_DATASOURCE_H

#include "ambulant/config/config.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/net/datasource.h"

#include "avcodec.h"
#ifdef WITH_FFMPEG_AVFORMAT
#include "avformat.h"
#endif // WITH_FFMPEG_AVFORMAT
#include "common.h"

// temporary debug messages
#include <iostream>
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM */
#include <ostream.h>
#endif /*AMBULANT_NO_OSTREAM */
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace ambulant
{

namespace net
{
	

	
class ffmpeg_audio_datasource_factory : public audio_datasource_factory {
  public:
	~ffmpeg_audio_datasource_factory() {};
	audio_datasource* new_audio_datasource(const net::url& url, audio_format_choices fmts);
};

class ffmpeg_video_datasource_factory : public video_datasource_factory {
  public:
	~ffmpeg_video_datasource_factory() {};
	video_datasource* new_video_datasource(const net::url& url);
};

class ffmpeg_audio_parser_finder : public audio_parser_finder {
  public:
	~ffmpeg_audio_parser_finder() {};
	audio_datasource* new_audio_parser(const net::url& url, audio_format_choices hint, datasource *src);
};

class ffmpeg_audio_filter_finder : public audio_filter_finder {
  public:
	~ffmpeg_audio_filter_finder() {};
	audio_datasource* new_audio_filter(audio_datasource *src, audio_format_choices fmts);
};

#ifdef WITH_FFMPEG_AVFORMAT

class ffmpeg_audio_datasource;

namespace detail {

// Actually, these classes could easily be used for a general demultiplexing
// datasource, there is very little code that is ffmpeg-dependent.

class datasink {
  public:
    virtual void data_avail(int64_t pts, uint8_t *data, int size) = 0;
	virtual bool buffer_full() = 0;
};
	
class ffmpeg_demux : public lib::unix::thread, public lib::ref_counted_obj {
  public:
	ffmpeg_demux(AVFormatContext *con);
	~ffmpeg_demux();
	
	static AVFormatContext *supported(const net::url& url);
	  
	void add_datasink(datasink *parent, int stream_index);
	void remove_datasink(int stream_index);
	void cancel();
  protected:
	unsigned long run();
  private:
    datasink *m_sinks[MAX_STREAMS];
	AVFormatContext *m_con;
	int m_nstream;
};

}

class ffmpeg_audio_datasource: 
	virtual public audio_datasource,
	public detail::datasink,
	virtual public lib::ref_counted_obj
{
  public:
	 static ffmpeg_audio_datasource *new_ffmpeg_audio_datasource(
  		const net::url& url, 
  		AVFormatContext *context,
		detail::ffmpeg_demux *thread);
  	
  	ffmpeg_audio_datasource(
  		const net::url& url, 
  		AVFormatContext *context,
		detail::ffmpeg_demux *thread, 
  		int stream_index);
  
    ~ffmpeg_audio_datasource();

    void start(lib::event_processor *evp, lib::event *callback);
	void stop();  

    void readdone(int len);
    void data_avail(int64_t pts, uint8_t *data, int size);
    bool end_of_file();
	bool buffer_full();
		
	char* get_read_ptr();
	int size() const;   
	audio_format& get_audio_format();

	std::pair<bool, double> get_dur();

  private:
    bool _end_of_file();
	const net::url m_url;
	AVFormatContext *m_con;
	int m_stream_index;
	audio_format m_fmt;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;

	databuffer m_buffer;
	detail::ffmpeg_demux *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
  
};

class ffmpeg_video_datasource:
	virtual public video_datasource,
	public detail::datasink,
	virtual public lib::ref_counted_obj {
  public:
	 static ffmpeg_video_datasource *new_ffmpeg_video_datasource(
		const net::url& url, AVFormatContext *context,
		detail::ffmpeg_demux *thread);

	 ffmpeg_video_datasource(const net::url& url, AVFormatContext *context,
		detail::ffmpeg_demux *thread, int stream_index);
    ~ffmpeg_video_datasource();

	bool has_audio();
    int width();
  	int height();
	audio_datasource *get_audio_datasource();

    void start_frame(lib::event_processor *evp, lib::event *callback, double timestamp);  
	void stop();  

    bool end_of_file();
	char* get_frame(double *timestamp, int *size);
	void frame_done(double timestamp, bool keepdata);
	
    void data_avail(int64_t ipts, uint8_t *data, int size);
	bool buffer_full();
	std::pair<bool, double> get_dur();

  private:
	int get_audio_stream_nr();
    bool _end_of_file();
	const net::url m_url;
	AVFormatContext *m_con;
	int m_stream_index;
	bool m_src_end_of_file;
    lib::event_processor *m_event_processor;
	std::queue<std::pair<double, char*> > m_frames;
	std::pair<double, char*> m_old_frame;
	int m_size;		// NOTE: this assumes all decoded frames are the same size!
//	databuffer m_buffer;
	detail::ffmpeg_demux *m_thread;
	lib::event *m_client_callback;  // This is our calllback to the client
  	bool m_thread_started;
  	double m_pts_last_frame;
  	double m_last_p_pts;
    lib::critical_section m_lock;
};

#endif // WITH_FFMPEG_AVFORMAT

class ffmpeg_decoder_datasource: virtual public audio_datasource, virtual public lib::ref_counted_obj {
  public:
	 ffmpeg_decoder_datasource(const net::url& url, datasource *src);
	 ffmpeg_decoder_datasource(audio_datasource *src);
    ~ffmpeg_decoder_datasource();
     
		  
    void start(lib::event_processor *evp, lib::event *callback);  
	void stop();  

    void readdone(int len);
    void data_avail();
    bool end_of_file();
	bool buffer_full();
		
	char* get_read_ptr();
	int size() const;   
	
	std::pair<bool, double> get_dur();
	audio_format& get_audio_format();
	bool select_decoder(const char* file_ext);
	bool select_decoder(audio_format &fmt);
	
	static bool supported(const net::url& url);
  protected:
  	int decode(uint8_t* in, int size, uint8_t* out, int &outsize);
	  
  private:
    bool _end_of_file();
    AVCodecContext *m_con;
	audio_format m_fmt;
    lib::event_processor *m_event_processor;
  	datasource* m_src;

	std::pair<bool, double> m_duration;
	
	databuffer m_buffer;
	bool m_blocked_full;
		
	lib::event *m_client_callback;  // This is our calllback to the client
	lib::critical_section m_lock;
 
};

class ffmpeg_resample_datasource: virtual public audio_datasource, virtual public lib::ref_counted_obj {
  public:
     ffmpeg_resample_datasource(audio_datasource *src, audio_format_choices fmts);
    ~ffmpeg_resample_datasource();
    
    void start(lib::event_processor *evp, lib::event *callback);  
	void stop();  

    void readdone(int len);
    void data_avail();
  
    bool end_of_file();
    bool buffer_full();
		
    char* get_read_ptr();
    int size() const;   
   
//    void get_input_format(audio_context &fmt);  
//    void get_output_format(audio_context &fmt);
	audio_format& get_audio_format() { return m_out_fmt; };
	std::pair<bool, double> get_dur();
		
  protected:
    int init(); 
  	
	  
  private:
    bool _end_of_file();

    audio_datasource* m_src;

    bool m_context_set;
    ReSampleContext *m_resample_context;
  
    databuffer m_buffer;
  	
    lib::event_processor *m_event_processor;
    lib::event *m_client_callback;  // This is our calllback to the client
    lib::critical_section m_lock;
  	audio_format m_in_fmt;
  	audio_format m_out_fmt;
 
};

}	// end namespace net
}	// end namespace ambulant

#endif
