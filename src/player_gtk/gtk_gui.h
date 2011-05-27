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

/*
 * @$Id$
 */

#ifndef __GTK_GUI_H__
#define __GTK_GUI_H__

#include "unix_preferences.h"
//#include <iostream>
//#include <fstream>

#include "gtk_settings.h"
#include "gtk_logger.h"

class gtk_mainloop;
class gtk_settings;

class gtk_gui : public GtkWidget{

  public:
	gtk_gui(const char* title, const char* initfile);
	~gtk_gui();

	const char* filename() {
		return m_smilfilename;
	}

	bool openSMILfile(const char *smilfilename, int mode, bool dupFlag);

	// send an event to the gui thread
	void internal_message(int level, char* msg);

	// signal interfaces
	void need_redraw(const void*, void*, const void*);
	void player_done();
	void player_start(GString,bool,bool);

	// major containers
	GtkWidget* get_gui_container();
	GtkWidget* get_document_container();
	GtkWindow* get_toplevel_container();

/*TMP*/	gtk_mainloop* m_mainloop;

  private:
	const char* m_programfilename;
	const char* m_smilfilename;
	gtk_settings* m_settings;
	GtkWindow* m_toplevelcontainer; // the actual top level window
	GtkWidget* menubar;		 // The UI (menubar)
	GtkWidget* m_guicontainer;	// The container (menubar + documentcontainer)
	GtkWidget* m_documentcontainer; // The drawable area
	GtkActionGroup *m_actions;

#define	LOCK_MESSAGE
#ifdef	LOCK_MESSAGE
	pthread_cond_t m_cond_message;
	pthread_mutex_t m_lock_message;
	pthread_t m_gui_thread;
#endif/*LOCK_MESSAGE*/
	bool m_pointinghand_cursor; //XXXX
	GtkFileChooser* m_file_chooser;
	GtkFileChooser* m_settings_chooser;
	GtkEntry* m_url_text_entry;
	void fileError(const gchar* smilfilename);

	void setDocument(const char* string);
  public:
	void do_file_selected();
	void do_url_selected();
	void do_settings_selected();
	void do_play();

	void do_about();
	void do_homepage();
	void do_welcome();
	void do_help();
	void do_load_settings();
	void do_logger_window();
	void do_open();
	void do_open_url();
	void do_pause();
	void do_player_done();
	void do_quit();
	void do_reload();
	void do_settings_select();
	void do_stop();

	void do_resize(GdkEventConfigure *event);

	guint signal_player_done_id;
	guint signal_need_redraw_id;
	guint signal_internal_message_id;

	void do_internal_message(gtk_message_event* e);

	void unsetCursor();

	void _update_menus();

	GMainLoop* main_loop;

};
#endif/*__GTK_GUI_H__*/
