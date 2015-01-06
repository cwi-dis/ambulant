/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
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

#undef XXXJACK_COMBINE_HACK

namespace ambulant
{

namespace net
{

class demux_datasource:
	virtual public pkt_datasource,
	public demux_datasink,
	virtual public lib::ref_counted_obj
{
  public:
	static demux_datasource *new_demux_datasource(
		const net::url& url,
		abstract_demux *thread);

	demux_datasource(
		const net::url& url,
		abstract_demux *thread,
		int stream_index);

	~demux_datasource();

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
    int size() const { return m_frames.size(); }

  protected:
	bool _end_of_file();
	bool _buffer_full();
	const net::url m_url;
	//AVFormatContext *m_con;
	int m_stream_index;
//	audio_format m_fmt;
	bool m_src_end_of_file;
	lib::event_processor *m_event_processor;
	std::queue<datasource_packet> m_frames;
	abstract_demux *m_thread;
	timestamp_t m_current_time;
	lib::event *m_client_callback;  // This is our callback to the client
    bool m_is_live;
    bool m_is_video;
    int m_max_packets_buffered;
#ifdef XXXJACK_COMBINE_HACK
    AVPacket *m_saved_packet;
#endif
	lib::critical_section m_lock;
};

class demux_video_datasource:
    public demux_datasource,
    public video_datasource_mixin
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
//	void set_pixel_layout(pixel_order l) { assert(l == pixel_unknown); }
//	timestamp_t get_buffer_time() { return m_frames.size() * frameduration(); };

	audio_format& get_audio_format() { assert(0); static audio_format a; return a;}
	bool has_audio();
	audio_datasource* get_audio_datasource(audio_format_choices fmts);

//	char* get_read_ptr();
//	int size() const;

//	video_format& get_video_format();

//	common::duration get_dur();

  private:
	pkt_datasource* m_audio_src;
	long long int m_frame_nr;
};



}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_DEMUX_DATASOURCE_H
