// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#include "ambulant/lib/win32/win32_fstream.h"
#include "ambulant/lib/textptr.h"

#include <cassert>
using namespace ambulant;

lib::win32::fstream::fstream()
:	m_hf(INVALID_HANDLE_VALUE),
	m_gptr(0),
	m_pptr(0) {
}

lib::win32::fstream::~fstream() {
	close();
}

bool lib::win32::fstream::open(const std::basic_string<char>& url) {
	return open(lib::textptr(url.c_str()));
}

bool lib::win32::fstream::open(const text_char *filename) {
	m_hf = CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		0,  // lpSecurityAttributes
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);
	return  m_hf != INVALID_HANDLE_VALUE;
}

bool lib::win32::fstream::open_for_writing(const text_char *filename) {
	m_hf = CreateFile(filename,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		0,  // lpSecurityAttributes
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	return  m_hf != INVALID_HANDLE_VALUE;
}

void lib::win32::fstream::close() {
	if(m_hf != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hf);
		m_hf = INVALID_HANDLE_VALUE;
	}
}

int lib::win32::fstream::read(unsigned char *buffer, int nbytes) {
	if(m_hf == INVALID_HANDLE_VALUE) return 0;
	unsigned long nread = 0;
	if(ReadFile(m_hf, buffer, nbytes, &nread, NULL) != 0) {
		m_gptr += nread;
		return int(nread);
	}
	return 0;
}

int lib::win32::fstream::read() {
	assert(m_hf != INVALID_HANDLE_VALUE);
	unsigned char buffer[4];
	unsigned long nread = 0;
	if(ReadFile(m_hf, buffer, 1, &nread, NULL) == 0 || nread == 0)
		return EOF;
	m_gptr++;
	return buffer[0];
}

unsigned long lib::win32::fstream::get_size() {
	assert(m_hf != INVALID_HANDLE_VALUE);
	return GetFileSize(m_hf, NULL);
}

bool lib::win32::fstream::seek(unsigned long pos) {
	assert(m_hf != INVALID_HANDLE_VALUE);
	if(SetFilePointer(m_hf, pos, 0, FILE_BEGIN) == 0xFFFFFFFF)
		return false;
	m_gptr = pos;
	return true;
}

int lib::win32::fstream::write(const unsigned char *buffer, int nbytes) {
	assert(m_hf != INVALID_HANDLE_VALUE);
	unsigned long nwritten = 0;
	if(WriteFile(m_hf, buffer, nbytes, &nwritten, NULL) != 0) {
		m_pptr += nwritten;
		return int(nwritten);
	}
	return 0;
}

int lib::win32::fstream::write(const char *cstr) {
	assert(m_hf != INVALID_HANDLE_VALUE);
	return write((const unsigned char *)cstr, (int)strlen(cstr));
}
