/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

// Ambulant standard base (ASB) compatibility implementations

#include <windows.h>

#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/lib/logger.h"

#include <string>

using namespace ambulant;

void lib::win32::sleep(unsigned long secs) {
	Sleep(1000*secs);
}

void lib::win32::sleep_msec(unsigned long msecs) {
	Sleep(msecs);
}

std::string lib::win32::getcwd() {
	char buf[MAX_PATH];
	char *pFilePart = 0;	
	GetFullPathName(".", MAX_PATH, buf, &pFilePart);
	return buf;
}

std::string lib::win32::resolve_path(const char *s) {
	char buf[MAX_PATH];
	char *pFilePart = 0;	
	GetFullPathName(s, MAX_PATH, buf, &pFilePart);
	return buf;
}

void lib::show_message(const char *format, ...) {
	va_list	args;
	va_start(args, format);
	int size = _vscprintf(format, args) + 1;
	char *buf = new char[size];
	vsprintf(buf, format, args);
	va_end(args);
	MessageBox(NULL, buf, "DemoPlayer", MB_OK);
	delete[] buf;
}


