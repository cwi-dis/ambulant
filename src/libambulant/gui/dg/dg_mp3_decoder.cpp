// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

/* 
 * @$Id$ 
 */

#include "ambulant/gui/dg/dg_mp3_decoder.h"
#include "ambulant/lib/logger.h"

#define MP3LIB_STATIC
#include "mp3lib.h"

#ifdef _DEBUG
#pragma comment (lib,"mp3lib_D.lib")
#else
#pragma comment (lib,"mp3lib.lib")
#endif

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::mp3_decoder::mp3_decoder()
:	m_mp3lib_inst(0), 
	m_dec_buf(0) {
	initialize();
	m_dec_buf = new char[dec_buf_size];
}

gui::dg::mp3_decoder::~mp3_decoder() {
	finalize();
	delete[] m_dec_buf;
}

void gui::dg::mp3_decoder::initialize() {
	if(!m_mp3lib_inst) {
		int equalizer = 0;
		char *eqfactors = NULL;
		mp3_lib_create_instance(&m_mp3lib_inst);
		mp3_lib_init(m_mp3lib_inst, equalizer, eqfactors);
	}
}

void gui::dg::mp3_decoder::finalize() {
	if(m_mp3lib_inst) {
		mp3_lib_finalize(m_mp3lib_inst);
		mp3_lib_release_instance(m_mp3lib_inst);
		m_mp3lib_inst = 0;
	}
}

void gui::dg::mp3_decoder::reset() {
	finalize();
	initialize();
}

void gui::dg::mp3_decoder::get_wave_format(lib::byte_buffer& bbuf, WAVEFORMATEX& m_wfx) {
	// Desired freq
	int freq = m_wfx.nSamplesPerSec;
		
	int nchannels, bitrate;
	mp3_lib_decode_header(m_mp3lib_inst, bbuf.data(), bbuf.remaining(), &freq, &nchannels, &bitrate);
			
	// channel sample size in bits
	const int depth = 16;
	 
	AM_DBG lib::logger::get_logger()->debug("freq:%d, channels:%d depth:%d bitrate:%d", 
		freq, nchannels, depth, bitrate);
	
	// sample size in bytes
	int samplesize = nchannels*depth/8; 
		
	// byte rate
	long byterate = samplesize*freq;
		
	WAVEFORMATEX wf = {
		WAVE_FORMAT_PCM, // the format type of this audio stream
		WORD(nchannels), // the number of channels of this audio stream
		DWORD(freq), // the samples frequency of this audio stream
		DWORD(byterate), // the byte rate of this audio stream
		WORD(samplesize), // the size in bytes of each sample
		WORD(depth), // the number of bits per sample per channel
		WORD(0) // the number of extra bytes after this field
		};
	m_wfx = wf;
}
		
// Decodes as much as possible from data
// Appends decoded data to 'm_dec_buf' buf
// Returns the mumber of bytes of data buffer not consumed yet
void gui::dg::mp3_decoder::decode(lib::byte_buffer& bbuf, std::basic_string<char> *decbuf) {
	assert(m_mp3lib_inst != 0);
	int inputpos; // current read position
	int produced; // bytes produced by m_decoder for each call
	int status = mp3_lib_decode_buffer(m_mp3lib_inst, bbuf.data(), bbuf.remaining(),
		m_dec_buf, dec_buf_size, &produced, &inputpos);
	//AM_DBG lib::logger::get_logger()->debug("produced: %d, position: %d, status: %d", produced, inputpos, status);
	if(produced > 0) decbuf->append(m_dec_buf, produced);
	while(status == 0) {
		status = mp3_lib_decode_buffer(m_mp3lib_inst, NULL, 0,
			m_dec_buf, dec_buf_size, &produced, &inputpos);
		//AM_DBG lib::logger::get_logger()->debug("produced: %d, position: %d, status: %d", produced, inputpos, status);
		if(produced > 0) {
			decbuf->append(m_dec_buf, produced);
		}
	}
	AM_DBG lib::logger::get_logger()->debug("Total bytes produced: %d", decbuf->length());
	// mp3lib conventions: 
	// -1	: error
	// 0	: continue
	// n>0	: n-1 is the remaining bytes
	if(status>0) status--;
	bbuf.set_position(bbuf.remaining() - status);
}

