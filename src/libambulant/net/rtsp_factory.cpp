// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#include "ambulant/net/rtsp_datasource.h"
#include "ambulant/net/rtsp_factory.h"
#include "ambulant/net/demux_datasource.h"
#include "ambulant/net/ffmpeg_audio.h"
#include "ambulant/net/ffmpeg_video.h"
#include "ambulant/lib/logger.h"


using namespace ambulant;
using namespace net;



#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif




audio_datasource* 
live_audio_datasource_factory::new_audio_datasource(const net::url& url, const audio_format_choices& fmts, timestamp_t clip_begin, timestamp_t clip_end)
{
	AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource(%s)", repr(url).c_str());
	rtsp_context_t *context = rtsp_demux::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: no support for %s", repr(url).c_str());
		return NULL;
	}
	rtsp_demux *thread = new rtsp_demux(context, clip_begin, clip_end);
	
	if (context->video_stream > -1) {
		thread->cancel();
		AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: rtsp stream contains video");
		return NULL;
	}
	//int stream_index;
	
	audio_datasource *ds = demux_audio_datasource::new_demux_audio_datasource(url, thread);
	if (ds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: could not allocate live_audio_datasource");
		thread->cancel();
		return NULL;
	}
	thread->start();
	
	AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: parser ds = 0x%x", (void*)ds);
	// XXXX This code should become generalized in datasource_factory
	if (fmts.contains(ds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: matches!");
		return ds;
	}
	audio_datasource *dds = new ffmpeg_decoder_datasource(ds);
	AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: decoder ds = 0x%x", (void*)dds);
	if (dds == NULL) {
		int rem = ds->release();
		assert(rem == 0);
		return NULL;
	}
	if (fmts.contains(dds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: matches!");
		return dds;
	}
	audio_datasource *rds = new ffmpeg_resample_datasource(dds, fmts);
	AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: resample ds = 0x%x", (void*)rds);
	if (rds == NULL)  {
		int rem = dds->release();
		assert(rem == 0);
		return NULL;
	}
	if (fmts.contains(rds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_audio_datasource: matches!");
		return rds;
	}
	lib::logger::get_logger()->error(gettext("%s: unable to create audio resampler"));
	int rem = rds->release();
	assert(rem == 0);
	return NULL;	
}


video_datasource* 
live_video_datasource_factory::new_video_datasource(const net::url& url, timestamp_t clip_begin, timestamp_t clip_end)
{
	AM_DBG lib::logger::get_logger()->debug("live_video_datasource_factory::new_video_datasource(%s)", repr(url).c_str());
	rtsp_context_t *context = rtsp_demux::supported(url);
	if (!context) {
		AM_DBG lib::logger::get_logger()->debug("live_video_datasource_factory::new_video_datasource: no support for %s", repr(url).c_str());
		return NULL;
	}
	rtsp_demux *thread = new rtsp_demux(context, clip_begin, clip_end);

	//int stream_index;
	
	video_datasource *ds = demux_video_datasource::new_demux_video_datasource(url, thread);
	if (ds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("live_audio_datasource_factory::new_video_datasource: could not allocate live_audio_datasource");
		thread->cancel();
		return NULL;
	}
	video_datasource *dds = NULL;
	thread->start();
	
	if (thread) {
		 video_format fmt = thread->get_video_format();
		 //dds = ds;
		 dds = new ffmpeg_video_decoder_datasource(ds, fmt);
	} else {
		return NULL;
	}
	
	
	AM_DBG lib::logger::get_logger()->debug("live_video_datasource_factory::new_video_datasource (ds = 0x%x)", (void*) ds);

	if (dds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("live_video_datasource_factory::new_video_datasource: could not allocate rtsp_video_datasource");
		thread->cancel();
		return NULL;
	}
	return dds;		
}
