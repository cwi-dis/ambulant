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

#include "qt_gui.h"
#include "qt_logger.h"
#include "qt_mainloop.h"
#include <qmessagebox.h>
#include "ambulant/lib/logger.h"

using namespace ambulant;

qt_logger_ostream::qt_logger_ostream()
  :		m_qstring(NULL)
{
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
	m_qstring += cstr;
	return 1;
}

int
qt_logger_ostream::write(std::string s)
{
	return  write(s.data());
}

void
qt_logger_ostream::write(lib::byte_buffer& bb)
{
	std::string id("qt_logger_ostream::write(lib::byte_buffer& bb)");
	write(id+" not implemented for Qt");
}

void
qt_logger_ostream::close() {
	std::string id("qt_logger_ostream::close()");
}

void
qt_logger_ostream::flush() {
	std::string id("qt_logger_ostream::flush()");
	qt_logger::get_qt_logger()->log(m_qstring);
//	logger->qt_logger::get_logger_window()->append(m_qstring);
	m_qstring.truncate(0);
}

qt_logger* qt_logger::s_qt_logger = 0;

qt_logger::qt_logger() 
  : 	m_log_FILE(NULL),
	m_gui(NULL) 
{
	common::preferences* prefs = 
	  common::preferences::get_preferences();
	lib::logger* logger = lib::logger::get_logger();
	if (prefs != NULL && prefs->m_log_file != "") {
		m_log_FILE = fopen(prefs->m_log_file.c_str(), "w");
		if (m_log_FILE == NULL) {
			logger->warn(gettext("Cannot open logfile: %s"), 
				     prefs->m_log_file.c_str());
		} else setbuf(m_log_FILE, NULL); // no buffering
	}
	// Connect logger to our message displayer and output processor
	logger->set_show_message(show_message);

	// Tell the logger about the output level preference
	int level = prefs->m_log_level;
	logger->set_level(level);
#ifndef QT_NO_FILEDIALOG	 /* Assume plain Qt */
	//logger->set_ostream(new qt_logger_ostream);
	logger_window = new QTextEdit();
	logger_window->setReadOnly(true);
	logger_window->setCaption("Ambulant-Logger");
	logger_window->setTextFormat(Qt::PlainText);
	logger_window->setGeometry(50, 50, 560, 240);
#endif/*QT_NO_FILEDIALOG*/
}

qt_logger*
qt_logger::get_qt_logger() {
	if (s_qt_logger == NULL) {
		s_qt_logger = new qt_logger();
	}
	return s_qt_logger;
}

void
qt_logger::set_qt_logger_gui(qt_gui* gui) {
	(void) get_qt_logger();
	if (s_qt_logger->m_gui == NULL) {
		s_qt_logger->m_gui = gui;
	}
}


void
qt_logger::log(QString logstring) {
 	if (m_log_FILE != NULL) {
		fprintf(m_log_FILE, "%s", (const char*)logstring);
	}
	char* message = strdup(logstring.ascii());
	s_qt_logger->m_gui->internal_message(-1, message);
}

void
qt_logger::show_message(int level, const char *msg) {
	char* message = strdup(msg);
	s_qt_logger->m_gui->internal_message(level, message);
}

#ifndef QT_NO_FILEDIALOG	 /* Assume plain Qt */
QTextEdit*
qt_logger::get_logger_window()
{
	return logger_window;
}
#endif/*QT_NO_FILEDIALOG*/

qt_message_event::qt_message_event(int level, char *msg)
 : QCustomEvent((QEvent::Type)level, (void*) msg)
{
}
