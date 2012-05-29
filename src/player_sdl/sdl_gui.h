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

#ifndef __SDL_GUI_H__
#define __SDL_GUI_H__

/* 
 * sdl_gui.h - SDL GUI for Ambulant
 *             intially only keyboard based
 *             in future could look like iAmbulant
 */

//X #include "unix_preferences.h"
//X #include <iostream>
//X #include <fstream>

//X #include "sdl_settings.h"
//X #include "sdl_logger.h"

#include "SDL.h"

class sdl_gui_player;
//X class sdl_settings;

class sdl_gui {

  public:
	sdl_gui(const char* title, const char* initfile);
	~sdl_gui();

	const char* filename() {
		return m_smilfilename;
	}

	bool openSMILfile(const char *smilfilename, int mode, bool dupFlag);

	// send an event to the gui thread
	void internal_message(int level, char* msg);

	// event interfaces
	void need_redraw(const void*, void*, const void*);
	void player_done();
	void player_start(char*,bool,bool);
	void sdl_loop(); // the SDL Event loop in main thread

	// major containers
	SDL_Surface* get_gui_container();
	SDL_Surface* get_document_container();
	SDL_Surface* get_toplevel_container();

	sdl_gui_player* m_gui_player;

  private:
	const char* m_programfilename;
	const char* m_smilfilename;
	SDL_Surface* m_toplevelcontainer; // the actual top level window
	SDL_Surface* m_guicontainer;	// The container (menubar + documentcontainer)
	SDL_Surface* m_documentcontainer; // The drawable area

#ifdef JNK // no visual gui, onlu Keyboars shortcuts 
	sdl_settings* m_settings;
	SDL_Surface* menubar;		 // The UI (menubar)
	SdlActionGroup *m_actions;
#endif//JNK

//#define	LOCK_MESSAGE
#ifdef	LOCK_MESSAGE
	pthread_cond_t m_cond_message;
	pthread_mutex_t m_lock_message;
	pthread_t m_gui_thread;
#endif/*LOCK_MESSAGE*/
//X	bool m_pointinghand_cursor;
//X	SdlFileChooser* m_file_chooser;
//X	SdlFileChooser* m_settings_chooser;
//X	SdlEntry* m_url_text_entry;
	void fileError(const char* smilfilename);

	void setDocument(const char* string);
  public:
//X	void do_file_selected();
//X	void do_url_selected();
//X	void do_settings_selected();
	void do_play();

//X	void do_about();
//X	void do_homepage();
//X	void do_welcome();
//X	void do_help();
//X	void do_load_settings();
//X	void do_logger_window();
//X	void do_open();
//X	void do_open_url();
	void do_pause();
//X	void do_player_done();
	void quit();
//X	void do_reload();
//X	void do_settings_select();
	void do_stop();

//X	void do_resize(GdkEventConfigure *event);

//X	void do_internal_message(sdl_message_event* e);

//X	void unsetCursor();

//X	void _update_menus();

//X	GPlayer* main_loop;

};
#endif/*__SDL_GUI_H__*/
