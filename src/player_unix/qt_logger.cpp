// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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
:	m_qstring(NULL)
{
}

bool
qt_logger_ostream::is_open() const {
	std::string id("qt_logger_ostream:::is_open() const");
	return true;
}

int
qt_logger_ostream::write(const unsigned char *buffer, int nbytes) {
	std::string id("qt_logger_ostream::write(const unsigned char *buffer, int nbytes)");
	write(id+" not implemented for Qt");
}

int
qt_logger_ostream::write(const char *cstr) {
	m_qstring += cstr;
	return 1;
}

int
qt_logger_ostream::write(std::string s) {
	return  write(s.data());
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
:	m_log_FILE(NULL),
	m_logger_window( NULL),
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
			logger->warn(gettext("Cannot open logfile: %s"), prefs->m_log_file.c_str());
		} else setbuf(m_log_FILE, NULL); // no buffering
	}
	// Connect logger to our message displayer and output processor
	logger->set_show_message(show_message);

	// Tell the logger about the output level preference
	int level = prefs->m_log_level;
	logger->set_level(level);
	logger->set_ostream(new qt_logger_ostream);
	m_logger_window = new QTextEdit();
	m_logger_window->setReadOnly(true);
	m_logger_window->setCaption("Ambulant-Logger");
	m_logger_window->setTextFormat(Qt::PlainText);
	m_logger_window->setGeometry(50, 50, 560, 240);
}

qt_logger::~qt_logger() {
	if(m_log_FILE) fclose (m_log_FILE);
	m_log_FILE = NULL;
	if(m_logger_window) delete m_logger_window;
	m_logger_window = NULL;
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
	s_qt_logger->m_gui->internal_message(-1, logstring);
}

void
qt_logger::show_message(int level, const char *msg) {
	s_qt_logger->m_gui->internal_message(level, msg);
}

QTextEdit*
qt_logger::get_logger_window() {
	return m_logger_window;
}

qt_message_event::qt_message_event(int level, const char *message)
:	_message(strdup(message)),
	QCustomEvent((QEvent::Type)level)
{
	setData(_message);
}

qt_message_event::~qt_message_event() {
	free(_message);

}
