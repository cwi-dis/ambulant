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

#ifndef AMBULANT_LIB_WIN32_ERROR_H
#define AMBULANT_LIB_WIN32_ERROR_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace lib {

namespace win32 {

void AMBULANTAPI win_report_error(const char *func, unsigned long  err);

void AMBULANTAPI win_report_last_error(const char *func);

void AMBULANTAPI win_trace_error(const char *func, unsigned long  err);

void AMBULANTAPI win_trace_last_error(const char *func);

void AMBULANTAPI win_show_error(const char *func, unsigned long  err);

void AMBULANTAPI win_show_last_error(const char *func);


} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_ERROR_H
