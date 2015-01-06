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

#include "sdl_gui.h"
#include "sdl_logger.h"
#include "sdl_gui_player.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;

sdl_logger_ostream::sdl_logger_ostream(){
	m_string = "";
}

bool
sdl_logger_ostream::is_open() const {
	std::string id("sdl_logger_ostream:::is_open() const");
	return true;
}

int
sdl_logger_ostream::write(const unsigned char *buffer, int nbytes)
{
	std::string id("sdl_logger_ostream::write(const unsigned char *buffer, int nbytes)");
	write(id+" not implemented for SDL");
	return 0;
}

int
sdl_logger_ostream::write(const char *cstr)
{
	m_string += cstr;
	return 1;
}

int
sdl_logger_ostream::write(std::string s)
{
	return  write(s.data());
}

void
sdl_logger_ostream::close() {
	std::string id("sdl_logger_ostream::close()");
}

void
sdl_logger_ostream::flush() {
	std::string id("sdl_logger_ostream::flush()");
	sdl_logger::get_sdl_logger()->log(m_string.c_str());
	m_string = "";
}

sdl_logger* sdl_logger::s_sdl_logger = 0;

sdl_logger::sdl_logger()
:	m_log_FILE(NULL),
	m_gui(NULL)
{
	common::preferences* prefs = common::preferences::get_preferences();
	lib::logger* logger = lib::logger::get_logger();

	if (prefs != NULL && prefs->m_log_file != "") {
		if (prefs->m_log_file == "-")
			m_log_FILE = stderr;
		else
			m_log_FILE = fopen(prefs->m_log_file.c_str(), "w");
		if (m_log_FILE == NULL) {
		} else setbuf(m_log_FILE, NULL); // no buffering
	}
#ifdef  ANDROID
	if (m_log_FILE == NULL) {
		m_log_FILE = fopen("/sdcard/ambulant.log","w+");
		if (m_log_FILE == NULL) {
			perror("Cannot open logfile");
		}
	}
#endif//ANDROID

	// Tell the logger about the output level preference
	int level = prefs->m_log_level;
	logger->set_level(level);
	logger->set_ostream(new sdl_logger_ostream);
}

sdl_logger::~sdl_logger() {
	if(m_log_FILE) fclose (m_log_FILE);
	m_log_FILE = NULL;
}

sdl_logger*
sdl_logger::get_sdl_logger() {
	if (s_sdl_logger == NULL) {
		s_sdl_logger = new sdl_logger();
	}
	return s_sdl_logger;
}

void
sdl_logger::set_sdl_logger_gui(sdl_gui* gui) {
	(void) get_sdl_logger();
	if (s_sdl_logger->m_gui == NULL) {
		s_sdl_logger->m_gui = gui;
	}
}

void
sdl_logger::log(const char *logstring) {
	if (m_log_FILE != NULL) {
		fprintf(m_log_FILE, "%s", logstring);
	}
#ifdef ANDROID
	LOGI ("%s", logstring);
#endif // ANDROID
}

