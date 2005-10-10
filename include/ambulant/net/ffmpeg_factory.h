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

#ifndef AMBULANT_NET_FFMPEG_FACTORY_H
#define AMBULANT_NET_FFMPEG_FACTORY_H


#include "ambulant/config/config.h"
#include "ambulant/net/datasource.h"

namespace ambulant
{

namespace net
{  

raw_datasource_factory *get_ffmpeg_raw_datasource_factory();
video_datasource_factory *get_ffmpeg_video_datasource_factory();
audio_datasource_factory *get_ffmpeg_audio_datasource_factory();
audio_parser_finder *get_ffmpeg_audio_parser_finder();
audio_filter_finder *get_ffmpeg_audio_filter_finder();

}	// end namespace net
}	// end namespace ambulant

#endif // AMBULANT_NET_FFMPEG_FACTORY_H
