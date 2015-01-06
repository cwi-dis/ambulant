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

// Ambulant standard base (ASB) compatibility interface

#ifndef AMBULANT_LIB_WIN32_ASB_H
#define AMBULANT_LIB_WIN32_ASB_H

#include "ambulant/config/config.h"

#include <string>

namespace ambulant {

namespace lib {

namespace win32 {

AMBULANTAPI void sleep(unsigned long secs);
AMBULANTAPI void sleep_msec(unsigned long msecs);

AMBULANTAPI std::basic_string<text_char> getcwd();
AMBULANTAPI std::basic_string<text_char> resolve_path(const text_char *s);
AMBULANTAPI bool file_exists(const std::string& fn);
AMBULANTAPI std::basic_string<text_char> get_module_filename();
AMBULANTAPI std::string get_module_dir();
AMBULANTAPI void show_message(int level, const char *message);
} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_ASB_H
