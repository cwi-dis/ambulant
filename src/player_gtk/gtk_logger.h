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

#ifndef __GTK_LOGGER_H__
#define __GTK_LOGGER_H__
//#include <qfeatures.h>

#include <stdarg.h>
#include <string.h>

#include "ambulant/version.h"
#include "ambulant/config/config.h"
#include "ambulant/lib/amstream.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"

#include <gtk/gtk.h>

class gtk_gui;	// forward declaration

class gtk_logger {
  public:
	enum custom_events {
		CUSTOM_OFFSET=10000,
		CUSTOM_LOGMESSAGE=-1,
		CUSTOM_NEW_DOCUMENT=-2
	};
	static gtk_logger* get_gtk_logger();
	static void show_message(int level, const char *message);
	GtkTextBuffer* get_logger_buffer();
	GtkWindow* get_logger_window();
	static void set_gtk_logger_gui(gtk_gui*);
	void log(gchar *logstring);
	~gtk_logger();
  protected:
	gtk_logger();
  private:
	static gtk_logger* s_gtk_logger;  // singleton
	GtkWindow* m_logger_window;
	GtkTextView* m_text_view;
	GtkTextBuffer *m_text_buffer;
	gtk_gui* m_gui;
	FILE* m_log_FILE;
};

class gtk_logger_ostream : public ambulant::lib::ostream {
  public:
	gtk_logger_ostream();
	bool is_open() const;
	void close();
	int  write(const unsigned char *buffer, int nbytes);
	int  write(const char *cstr);
	int  write(std::string s);
	void flush();
  private:
	GString *m_string;
};

//: public GtkEvent
class gtk_message_event{
  public:
	gtk_message_event(int type, gchar* message);
	int get_type();
	gchar* get_message();
  private:
	int type;
	gchar *message;
};

#endif/*__GTK_LOGGER_H__*/
