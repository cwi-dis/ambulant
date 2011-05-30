/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

#ifndef __QT_LOGGER_H__
#define __QT_LOGGER_H__
#include <qfeatures.h>

#include <stdarg.h>
#include <string.h>
#include <qtextedit.h>

#include "ambulant/version.h"
#include "ambulant/config/config.h"
#include "ambulant/lib/amstream.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"

class qt_gui;	// forward declaration

class qt_logger {
  public:
	enum custom_events {
		CUSTOM_OFFSET=10000,
		CUSTOM_LOGMESSAGE=-1,
		CUSTOM_NEW_DOCUMENT=-2
	};
	static qt_logger* get_qt_logger();
	static void show_message(int level, const char *message);
	QTextEdit* get_logger_window();
	static void set_qt_logger_gui(qt_gui*);
	void log(QString logstring);
	~qt_logger();
  protected:
	qt_logger();
  private:
	static qt_logger* s_qt_logger;  // singleton
	QTextEdit* m_logger_window;
	qt_gui* m_gui;
	FILE* m_log_FILE;
};

class qt_logger_ostream : public ambulant::lib::ostream {
  public:
	qt_logger_ostream();
	bool is_open() const;
	void close();
	int  write(const unsigned char *buffer, int nbytes);
	int  write(const char *cstr);
	int  write(std::string s);
	void flush();
  private:
	QString m_qstring;
};

class qt_message_event : public QCustomEvent {
  public:
	qt_message_event(int type, const char* message);
	~qt_message_event();
  private:
	char* _message;
};
#endif/*__QT_LOGGER_H__*/
