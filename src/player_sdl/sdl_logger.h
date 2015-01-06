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

#ifndef __SDL_LOGGER_H__
#define __SDL_LOGGER_H__

#include <stdarg.h>
#include <string.h>

#include "ambulant/version.h"
#include "ambulant/config/config.h"
#include "ambulant/lib/amstream.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"

#include "SDL.h"

#ifdef ANDROID

#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "SDL/Ambulant", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "SDL/Ambulant", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "SDL/Ambulant", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "SDL/Ambulant", __VA_ARGS__))
#define LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "SDL/Ambulant", __VA_ARGS__))

#endif // ANDROID

class sdl_gui;	// forward declaration

class sdl_logger {
  public:
	static sdl_logger* get_sdl_logger();
	static void set_sdl_logger_gui(sdl_gui*);
	void log(const char *logstring);
	~sdl_logger();
  protected:
	sdl_logger();
  private:
	static sdl_logger* s_sdl_logger;  // singleton
	sdl_gui* m_gui;
	FILE* m_log_FILE;
};

class sdl_logger_ostream : public ambulant::lib::ostream {
  public:
	sdl_logger_ostream();
	bool is_open() const;
	void close();
	int  write(const unsigned char *buffer, int nbytes);
	int  write(const char *cstr);
	int  write(std::string s);
	void flush();
  private:
	std::string m_string;
};


#endif/*__SDL_LOGGER_H__*/
