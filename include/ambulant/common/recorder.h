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

#ifndef RECORDER_H
#define RECORDER_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace common {

class recorder {
public:
	/// API for recording audio/video in ambulant
	/// Typical use is in a renderer/player to produce a file/stream of a SMIL presentation
	/// Maybe usable for audio/video capture as well (through an extension 
	/// a.o. implementing capture device details such as channel selection, etc.)
	enum frame_type { default_frame_type };
	enum packet_format { default_format };
	enum stream_type { default_stream_type };

	/// Initialize for receiving video frames of a particular type and frame size
	/// Multiple initializations may be allowed, to receive different types
	virtual void initalize_frames(frame_type, lib::size) = 0;

	/// Initialize for receiving audio packets of a particular type
	/// Multiple initializations may be allowed, to receive different formats
	virtual void initalize_packets(packet_format) = 0;

	/// Initialize for producing an AV stream of a particular type on a file or (named) stream
	/// Multiple initializations may be allowed, to produce different streamtypes
	virtual void initalize_output_stream(stream_type, const char* stream_name="") = 0;

	/// Record a single (video) frame.
	virtual void new_frame(void* data, size_t datasize, lib::timer::time_type documenttimestamp, frame_type=default_frame_type) = 0;

	/// Record a single (audio) packet.
	virtual void new_packet(void* data, size_t datasize, lib::timer::timer::time_type _documentimestamp,  packet_format=default_format) = 0;

	/// May be some (optional) methods could be provided for handling multiple
	/// frames/packets e.g. benefiting from a datasource interface
};

class recorder_factory {  /*TBD: details not yet known*/
public:
	recorder_factory() {} // factory constructor
	~recorder_factory() {} // factory destructor
	recorder* new_recorder() {} // return new recorder with default settings
private:
	std::list <recorder*> m_recorders; // register recorders produced
};

}; // namespace common

}; // namespace ambulant

#endif /* _RECORDER_H */
