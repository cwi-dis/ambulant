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

#include "gtk_gui.h"
#include "gtk_logger.h"
#include "gtk_mainloop.h"
#include <gtk/gtk.h>
#include "ambulant/lib/logger.h"

using namespace ambulant;

gtk_logger_ostream::gtk_logger_ostream(){
	m_string = g_string_new("");
}

bool
gtk_logger_ostream::is_open() const {
	std::string id("gtk_logger_ostream:::is_open() const");
	return true;
}

int
gtk_logger_ostream::write(const unsigned char *buffer, int nbytes)
{
	std::string id("gtk_logger_ostream::write(const unsigned char *buffer, int nbytes)");
	write(id+" not implemented for GTK");
}

int
gtk_logger_ostream::write(const gchar *cstr)
{
	m_string = g_string_append(m_string, cstr);
	return 1;
}

int
gtk_logger_ostream::write(std::string s)
{
	return  write(s.data());
}

void
gtk_logger_ostream::close() {
	g_string_free (m_string, TRUE);
	std::string id("gtk_logger_ostream::close()");
}

void
gtk_logger_ostream::flush() {
	std::string id("gtk_logger_ostream::flush()");
	gtk_logger::get_gtk_logger()->log(m_string->str);
	m_string = g_string_truncate(m_string, 0);
}

gtk_logger* gtk_logger::s_gtk_logger = 0;

gtk_logger::gtk_logger()
:	m_log_FILE(NULL),
	m_gui(NULL)
{
	m_logger_window = NULL;


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
	logger->set_ostream(new gtk_logger_ostream);

	// create the GUI object
	m_logger_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title (m_logger_window, "Ambulant-logger");
	gtk_window_set_resizable (m_logger_window, false);
#if GTK_MAJOR_VERSION >= 3
	g_signal_connect (G_OBJECT (m_logger_window), "delete-event",G_CALLBACK (gtk_widget_hide), GTK_WIDGET (m_logger_window));
#else
	gtk_signal_connect (GTK_OBJECT (m_logger_window), "delete-event",G_CALLBACK (gtk_widget_hide), GTK_WIDGET (m_logger_window));
#endif // GTK_MAJOR_VERSION
	GtkWidget* sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (
		GTK_SCROLLED_WINDOW (sw),
		GTK_POLICY_AUTOMATIC,
		GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(GTK_WIDGET (m_logger_window), 560, 240);
#if GTK_MAJOR_VERSION >= 3
	gtk_window_set_position(m_logger_window, GTK_WIN_POS_CENTER);
#else
	gtk_widget_set_uposition(GTK_WIDGET (m_logger_window), 50, 50);
#endif // GTK_MAJOR_VERSION
	gtk_container_add (GTK_CONTAINER (m_logger_window), sw);
	gtk_widget_show(GTK_WIDGET (sw));
	m_text_view =  (GtkTextView*) gtk_text_view_new();
	gtk_text_view_set_editable(m_text_view, false);
	gtk_text_view_set_cursor_visible(m_text_view, false);
	gtk_widget_show(GTK_WIDGET (m_text_view));
	gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (m_text_view));

	// create the text buffer
	m_text_buffer= gtk_text_view_get_buffer (m_text_view);
}

gtk_logger::~gtk_logger() {
	if(m_log_FILE) fclose (m_log_FILE);
	m_log_FILE = NULL;
	gtk_widget_destroy (GTK_WIDGET(m_logger_window));
	m_logger_window = NULL;
}

gtk_logger*
gtk_logger::get_gtk_logger() {
	if (s_gtk_logger == NULL) {
		s_gtk_logger = new gtk_logger();
	}
	return s_gtk_logger;
}

void
gtk_logger::set_gtk_logger_gui(gtk_gui* gui) {
	(void) get_gtk_logger();
	if (s_gtk_logger->m_gui == NULL) {
		s_gtk_logger->m_gui = gui;
	}
}

void
gtk_logger::log(gchar *logstring) {
	if (m_log_FILE != NULL) {
		fprintf(m_log_FILE, "%s", (const char*)(logstring));
	}
	char* message = strdup(logstring);
	s_gtk_logger->m_gui->internal_message(-1, message);
}

void
gtk_logger::show_message(int level, const char *msg) {
	char* message = strdup(msg);
	s_gtk_logger->m_gui->internal_message(level, message);
}

GtkTextBuffer*
gtk_logger::get_logger_buffer()
{
	return m_text_buffer;
}

GtkWindow*
gtk_logger::get_logger_window()
{
	return m_logger_window;
}



gtk_message_event::gtk_message_event(int level, char *msg){
	type = level;
	message = msg;
}

int
gtk_message_event::get_type(){
	return type;
}

gchar*
gtk_message_event::get_message(){
	return message;
}
