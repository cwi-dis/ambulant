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

#ifndef __SDL_GUI_H__
#define __SDL_GUI_H__

/* 
 * sdl_gui.h - SDL GUI for Ambulant: interface to SDL2 and Linux
 *             only keyboard based 
 *
 *             in future could look like iAmbulant
 */
// TBD: visual gui, file/url selection, preferences, error logger

#include "SDL.h"
#ifdef WITH_SDL_IMAGE
#include "SDL_image.h"
#endif

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
	SDL_Window* get_document_container();
	// SDL interface
	SDL_Window* get_window() { return m_window; }
	sdl_gui_player* m_gui_player;
  private:
	const char* m_programfilename;
	const char* m_smilfilename;
	SDL_Window* m_toplevelcontainer; // the actual top level window
	SDL_Window* m_guicontainer;	  // The container (menubar + documentcontainer)
	SDL_Window* m_documentcontainer; // The drawable area
	// SDL interface
	SDL_Window* m_window;	// the top-level window, containing all visuals
	SDL_Cursor* m_arrow_cursor;
	SDL_Cursor* m_hand_cursor;
#ifdef  TBD // no visual gui, only Keyboard shortcuts 
	sdl_settings* m_settings;
	SDL_Surface* menubar;		 // The UI (menubar)
	SdlActionGroup *m_actions;
#endif//TBD

//#define	LOCK_MESSAGE
#ifdef	LOCK_MESSAGE
	pthread_cond_t m_cond_message;
	pthread_mutex_t m_lock_message;
	pthread_t m_gui_thread;
#endif/*LOCK_MESSAGE*/
	void fileError(const char* smilfilename);

	void setDocument(const char* string);
  public:
	void do_play();

	void do_pause();
	void quit();
	void do_stop();
};
#endif/*__SDL_GUI_H__*/
