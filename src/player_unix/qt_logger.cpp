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

#include "qt_logger.h"

qt_logger_ostream::qt_logger_ostream()
 #if 0
 :		buf_len(1024),
		buf_idx(0)
#endif
{
#if 0
	buf = (char*) malloc(buf_len);
	if (!buf) {
	abort();	 
	}
#endif
}

bool 
qt_logger_ostream::is_open() const {
	std::string id("qt_logger_ostream:::is_open() const");
	return true;
}

int
qt_logger_ostream::write(const unsigned char *buffer, int nbytes)
{
	std::string id("qt_logger_ostream::write(const unsigned char *buffer, int nbytes)");
	write(id+" not implemented for Qt");
}

int
qt_logger_ostream::write(const char *cstr)
{
//	std::string id("qt_logger_ostream::write(const char *cstr)");
#if 0
        int cstr_len = strlen(cstr), i, line_len;
	bool found = false;
	// glue strings to lines
	for (i=0; i < cstr_len; i++) {
		if (cstr[i] =='\n') {
			found = true;
			break;
		}
	}
	line_len = i;
	if (line_len > 0) {
		strncpy(&buf[buf_idx], cstr, line_len);
		buf_idx += line_len;
	}
	if (found) {
		buf[buf_idx] = '\0';
		qt_logger::get_qt_logger()->
			get_logger_window()->append(buf);
		buf_idx = 0;
		if (cstr_len > line_len+1)
			write(&cstr[line_len+1]);
	}
#else
	qt_logger::get_qt_logger()->get_logger_window()->append(cstr);
#endif
	return 1;
}

int
qt_logger_ostream::write(std::string s)
{
//	std::string id("qt_logger_ostream::write(string s)");
	return  write(s.data());
}

void
qt_logger_ostream::write(ambulant::lib::byte_buffer& bb)
{
	std::string id("qt_logger_ostream::write(ambulant::lib::byte_buffer& bb)");
	write(id+" not implemented for Qt");
}

void
qt_logger_ostream::close() {
	std::string id("qt_logger_ostream::close()");
}

void
qt_logger_ostream::flush() {
	std::string id("qt_logger_ostream::flush()");
}

qt_logger* qt_logger::s_qt_logger = 0;

qt_logger::
qt_logger::qt_logger()
{
	ambulant::lib::logger* logger =
		ambulant::lib::logger::get_logger();
	// Connect logger to our message displayer and output processor
	logger->set_show_message(show_message);
	//logger->set_ostream(new qt_logger_ostream);
	// Tell the logger about the output level preference
	int level = ambulant::common::preferences::get_preferences()->m_log_level;
	logger->set_level(level);
	logger_window = new QTextEdit();
	logger_window->setReadOnly(true);
	logger_window->setCaption("Ambulant-Logger");
	logger_window->setTextFormat(Qt::PlainText);
	logger_window->setGeometry(50, 50, 560, 240);
}

qt_logger*
qt_logger::get_qt_logger() {
	if (s_qt_logger == 0) {
		s_qt_logger = new qt_logger();
	}
	return s_qt_logger;
}

void
qt_logger::show_message(const char *format,...)
{
}

QTextEdit*
qt_logger::get_logger_window()
{
	return logger_window;
}
