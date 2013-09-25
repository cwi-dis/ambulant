/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#include "ambulant/config/config.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/datasource.h"

#include <queue>


namespace ambulant
{

namespace net
{

class demux_audio_datasource:
	virtual public pkt_audio_datasource,
	public demux_datasink,
	virtual public lib::ref_counted_obj
{
  public:
	static demux_audio_datasource *new_demux_audio_datasource(
		const net::url& url,
		abstract_demux *thread);

	demux_audio_datasource(
		const net::url& url,
		abstract_demux *thread,
		int stream_index);

	~demux_audio_datasource();

	void start(lib::event_processor *evp, lib::event *callback);
	void stop();
	void read_ahead(timestamp_t clip_begin);
	void seek(timestamp_t time);
	void set_clip_end(timestamp_t clip_end);
	timestamp_t get_elapsed() { assert(0); /* XXXJACK Should be based on pts in head of queue */ return 0; }
	bool push_data(timestamp_t pts, struct AVPacket *pkt, datasource_packet_flag flag);
	bool end_of_file();
	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_thread->get_start_time(); };
	datasource_packet get_packet();
	audio_format& get_audio_format();

	common::duration get_dur();

    long get_bandwidth_usage_data(const char **resource) { return m_thread->get_bandwidth_usage_data(m_stream_index, resource); }
    void set_is_live (bool is_live) { m_thread->set_is_live(is_live); }
    bool get_is_live () { return m_thread->get_is_live(); }

  private:
	bool _end_of_file();
	bool _buffer_full();
	const net::url m_url;
	//AVFormatContext *m_con;
	int m_stream_index;
//	audio_format m_fmt;
	bool m_src_end_of_file;
	lib::event_processor *m_event_processor;
	std::queue<datasource_packet> m_queue;
	abstract_demux *m_thread;
	timestamp_t m_current_time;
	lib::event *m_client_callback;  // This is our callback to the client
	lib::critical_section m_lock;
    bool m_is_live;
};

class demux_video_datasource:
	virtual public pkt_video_datasource,
	public demux_datasink,
	virtual public lib::ref_counted_obj
{
  public:
	static demux_video_datasource *new_demux_video_datasource(
		const net::url& url,
		abstract_demux *thread);

	demux_video_datasource(
		const net::url& url,
		abstract_demux *thread,
		int stream_index);

	~demux_video_datasource();
	void set_pixel_layout(pixel_order l) { assert(l == pixel_unknown); }
	void read_ahead(timestamp_t clip_begin);
	void seek(timestamp_t time);
	void set_clip_end(timestamp_t clip_end);
	void start_prefetch(lib::event_processor *evp){};
	void start_frame(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk, timestamp_t timestamp);
	void stop();
	/// Return the next timestamped packet and discard it.
	datasource_packet get_packet();
	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback);
//	char* get_frame(timestamp_t now, timestamp_t *timestamp, size_t *sizep);
//	void frame_processed_keepdata(timestamp_t timestamp, char *data);
//	void frame_processed(timestamp_t timestamp);
	bool push_data(timestamp_t pts, struct AVPacket *pkt, datasource_packet_flag flag);
	bool end_of_file();
	timestamp_t get_clip_end();
	timestamp_t get_clip_begin();
	timestamp_t get_start_time() { return m_thread->get_start_time(); };
	timestamp_t get_buffer_time() { return m_frames.size() * frameduration(); };
	int width();
	int height();
	timestamp_t frameduration();

	bool has_audio();
	audio_datasource* get_audio_datasource();
	// Silly: need this method but don't implement it
	net::audio_format& get_audio_format() { static net::audio_format fmt("this-is-video-not-audio"); return fmt; };
	// Silly: need this method but don't implement it
	timestamp_t get_elapsed() { return -1; }
	char* get_read_ptr();
	int size() const;

	video_format& get_video_format();

	common::duration get_dur();

    long get_bandwidth_usage_data(const char **resource) { return m_thread->get_bandwidth_usage_data(m_stream_index, resource); }
    void set_is_live (bool is_live) { m_thread->set_is_live(is_live); }
    bool get_is_live () { return m_thread->get_is_live(); }

  private:
	bool _end_of_file();
	bool _buffer_full();
	const net::url m_url;
	//AVFormatContext *m_con;
	int m_stream_index;
//	audio_format m_fmt;
	bool m_src_end_of_file;
	lib::event_processor *m_event_processor;
	std::queue<datasource_packet > m_frames;
	datasource_packet m_old_frame;
	abstract_demux *m_thread;
	timestamp_t m_current_time;
	lib::event *m_client_callback;  // This is our calllback to the client
	pkt_audio_datasource* m_audio_src;
	lib::critical_section m_lock;
	//FILE *m_file;
	long long int m_frame_nr;
    bool m_is_live;
};



}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_DEMUX_DATASOURCE_H
