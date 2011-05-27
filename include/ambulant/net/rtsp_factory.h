/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

#include "ambulant/net/datasource.h"


#ifndef AMBULANT_NET_RTSP_FACTORY_H
#define AMBULANT_NET_RTSP_FACTORY_H

namespace ambulant {

namespace net {


class live_audio_datasource_factory : public audio_datasource_factory {
  public:
	~live_audio_datasource_factory() {};
	audio_datasource* new_audio_datasource(const net::url& url, const audio_format_choices& fmts, timestamp_t clip_begin, timestamp_t clip_end);
};

class live_video_datasource_factory : public video_datasource_factory {
  public:
	~live_video_datasource_factory() {};
	video_datasource* new_video_datasource(const net::url& url,timestamp_t clip_begin, timestamp_t clip_end);
};

AMBULANTAPI video_datasource_factory *create_live_video_datasource_factory();
AMBULANTAPI audio_datasource_factory *create_live_audio_datasource_factory();

}
} //endofnamespaces


#endif
