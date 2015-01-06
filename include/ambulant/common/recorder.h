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

#ifndef RECORDER_H
#define RECORDER_H

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/net/datasource.h"

namespace ambulant {

namespace common {

/// API for recording audio/video in ambulant to produce 
/// a file/stream of a SMIL presentation
class recorder {
public:
	
	/// Destructor
	virtual ~recorder() {}
	
	/// Record new video data with timestamp (ms) in document time
	virtual void new_video_data (const char* data, size_t datasize, lib::timer::time_type documenttimestamp) = 0;

	/// Record new audio data  with timestamp (ms) in document time
	virtual void new_audio_data (const char* data, size_t datasize, lib::timer::time_type documenttimestamp) = 0;
};

class recorder_factory {

 public:
	/// recorder factory destructor
	virtual ~recorder_factory() {}
	
	/// return new recorder for the given 'pixel_order' and window size, or NULL if not supported
 	virtual recorder* new_recorder(net::pixel_order pixel_order, lib::size window_size) = 0;
};

}; // namespace common

}; // namespace ambulant

#endif /* _RECORDER_H */
