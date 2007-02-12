/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DG_MP3_DECODER_H
#define AMBULANT_GUI_DG_MP3_DECODER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/byte_buffer.h"

#include <string>

namespace ambulant {

namespace gui {

namespace dg {

class mp3_decoder {
  public:
	mp3_decoder();
	~mp3_decoder();

	void initialize();
	void finalize();
	void reset();
	void get_wave_format(lib::byte_buffer& bbuf, WAVEFORMATEX& m_wfx);
		
	// Decodes as much as possible from data
	// Appends decoded data to 'm_decbuf' buf
	// Returns the mumber of bytes of data buffer not consumed yet
	void decode(lib::byte_buffer& bbuf, std::basic_string<char> *decbuf);
 
  private:
	enum {dec_buf_size = 8192};
	void *m_mp3lib_inst;
	char *m_dec_buf;
};


} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_MP3_DECODER_H
