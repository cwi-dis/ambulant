// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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
 * sdl_gui.cpp - SDL GUI for Ambulant
 *               intially keyboard based
 *               could look like iAmbulant
 */

#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>
#include <fcntl.h>
#include "sdl_gui.h"
#include "sdl_gui_player.h"
#include "unix_preferences.h"

//X #include "sdl_logger.h"
//X #include "sdl_renderer.h"

#if  TBD
#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#endif//TBD

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#undef WITH_SDL_LOGGER

#ifndef AMBULANT_DATADIR
#define AMBULANT_DATADIR "/usr/local/share/ambulant"
#endif


const char *about_text =
	"Ambulant SMIL 3.0 player.\n"
	"Version: %s\n"
	"Copyright Stichting CWI, 2003-2012.\n\n"
	"License: LGPL";


// Places where to look for the Welcome document
const char *welcome_locations[] = {
	"Welcome/Welcome.smil",
	"../Welcome/Welcome.smil",
	"Extras/Welcome/Welcome.smil",
	"../Extras/Welcome/Welcome.smil",
	AMBULANT_DATADIR "/Welcome/Welcome.smil",
	NULL
};

// Places where to look for the helpfile
const char *helpfile_locations[] = {
	"Documentation/user/index.html",
	"../Documentation/user/index.html",
	AMBULANT_DATADIR "/AmbulantPlayerHelp/index.html",
	NULL
};


static const char *
find_datafile(const char **locations)
{
	const char **p;
	for(p = locations; *p; p++) {
		if (access(*p, 0) >= 0) return *p;
	}
	return NULL;
}

sdl_gui::sdl_gui(const char* title, const char* initfile)
:
	m_programfilename(NULL),
	m_smilfilename(initfile),
	m_toplevelcontainer(NULL),
	m_guicontainer(NULL),
	m_documentcontainer(NULL),
	m_window(NULL),
	m_arrow_cursor(NULL),
	m_hand_cursor(NULL)
//JNK	m_renderer(NULL),
//JNK	m_texture(NULL)
//TBD	menubar(NULL),
//TBD	m_settings(NULL),
//TBD	m_actions(NULL)
{
	// create the player
	int width = 640, height = 480;
#ifdef  JNK
	m_window = SDL_CreateWindow("SDL2 Video_Test", 0,0,width,height,0); //XXXX consider SDL_CreateWindowFrom(XwinID) !
	assert (m_window);
	AM_DBG lib::logger::get_logger()->trace("sdl_gui::sdl_gui(0x%x): m_window=(SDL_Window*)0x%x", this, m_window);
#ifdef MORE_JNK
	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
	if (m_renderer == NULL) {
	       	AM_DBG lib::logger::get_logger()->trace("sdl_gui::sdl_gui(0x%x): trying software renderer", this);
       		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
		if (m_renderer == NULL) {
			lib::logger::get_logger()->warn("Cannot open: %s, error: %s", "SDL CreateRenderer", SDL_GetError());
       			return;
		}
	}
	assert(m_renderer);
	// From SDL documentation
	/* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order,
	   as expected by OpenGL for textures */
	//X SDL_Surface *surface;
	Uint32 rmask, gmask, bmask, amask;
	
	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	m_surface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
	if (m_surface == NULL) {
		/* or using the default masks for the depth: */
    		m_surface = SDL_CreateRGBSurface(0,width,height,32,0,0,0,0);
		lib::logger::get_logger()->warn("Cannot open %s, error: %s", "SDL_CreateRGBSurface",SDL_GetError());
		return;
	}
#endif//MORE_JNK
#endif//JNK
	m_toplevelcontainer = m_documentcontainer = m_window;
	m_gui_player = new sdl_gui_player(this);
	m_arrow_cursor = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_ARROW);
	m_hand_cursor = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_HAND);
	SDL_SetCursor (m_arrow_cursor);
}

sdl_gui::~sdl_gui() {

	// remove all dynamic data in reverse order as they are constructed
	// m_programfilename - not dynamic
	if (m_arrow_cursor != NULL) {
//		SDL_FreeCursor (m_arrow_cursor);
	}
	if (m_hand_cursor != NULL) {
//		SDL_FreeCursor (m_hand_cursor);
	}
	if (m_smilfilename != NULL) {
		free((void*)m_smilfilename);
		m_smilfilename = NULL;
	}
#ifdef JNK
	if (m_surface != NULL) {
		SDL_FreeSurface(m_surface);
	}
	if (m_renderer != NULL) {
		SDL_DestroyRenderer(m_renderer);
	}
	if (m_renderer != NULL) {
		SDL_DestroyRenderer(m_renderer);
	}
#endif//JNK
	if (m_window != NULL) {
		SDL_DestroyWindow(m_window);
	}
#ifdef JNK
	if (m_settings != NULL) {
		delete m_settings;
		m_settings = NULL;
	}
	if (m_toplevelcontainer != NULL) {
		m_toplevelcontainer = NULL;
	}
	if (menubar) {
		sdl_surface_destroy(SDL_SURFACE (menubar));
		menubar = NULL;
	}
/*KB these seem to be destroyed already by destroying m_toplevelcontainer
	if (m_guicontainer) {
		sdl_surface_destroy(SDL_SURFACE (m_guicontainer));
		m_guicontainer = NULL;
	}
	if (m_documentcontainer) {
		sdl_surface_destroy(SDL_SURFACE (m_documentcontainer));
		m_documentcontainer = NULL;
	}
*/
	if (m_actions) {
		g_object_unref(G_OBJECT (m_actions));
		m_actions = NULL;
	}
	if (m_file_chooser) {
		sdl_surface_destroy(SDL_SURFACE (m_file_chooser));
		m_file_chooser = NULL;
	}
	if (m_settings_chooser) {
		sdl_surface_destroy(SDL_SURFACE (m_settings_chooser));
		m_settings_chooser = NULL;
	}
	if (m_url_text_entry) {
		sdl_surface_destroy(SDL_SURFACE (m_url_text_entry));
		m_url_text_entry = NULL;
	}
#endif//JNK
	if (m_gui_player) {
		delete m_gui_player;
		m_gui_player = NULL;
	}
}

#ifdef JNK
SDL_Surface*
sdl_gui::get_toplevel_container()
{
	return this->m_toplevelcontainer;
}

SDL_Surface*
sdl_gui::get_gui_container()
{
	return this->m_guicontainer;
}
#endif//JNK

SDL_Window*
sdl_gui::get_document_container()
{
	return this->m_documentcontainer;
}

#ifdef WITH_SDL2XX // code here copied from gtk_gui
#ifdef JNK

void
sdl_gui::do_about() {
	SdlMessageDialog* dialog = (SdlMessageDialog*) sdl_message_dialog_new (NULL,
		SDL_DIALOG_DESTROY_WITH_PARENT,
		SDL_MESSAGE_INFO,
		SDL_BUTTONS_OK,
		gettext("About AmbulantPlayer"));
	sdl_message_dialog_format_secondary_text(dialog,about_text,get_version());
	sdl_dialog_run (SDL_DIALOG (dialog));
	sdl_surface_destroy (SDL_SURFACE (dialog));
}

void
sdl_gui::do_homepage() {
	open_web_browser("http://www.ambulantplayer.org");
}

void
sdl_gui::do_welcome() {
	const char *welcome_doc = find_datafile(welcome_locations);

	if (welcome_doc) {
		if( openSMILfile(welcome_doc, 0, true)) {
			do_play();
		}
	} else {
		SdlMessageDialog* dialog = (SdlMessageDialog*) sdl_message_dialog_new (NULL,
			SDL_DIALOG_DESTROY_WITH_PARENT,
			SDL_MESSAGE_ERROR,
			SDL_BUTTONS_OK,
			gettext("Cannot find Welcome.smil document"));
		sdl_dialog_run (SDL_DIALOG (dialog));
		sdl_surface_destroy (SDL_SURFACE (dialog));
	}
}

void
sdl_gui::do_help() {
	const char *help_doc = find_datafile(helpfile_locations);

	if (help_doc) {
		open_web_browser(help_doc);
	} else {
		SdlMessageDialog* dialog = (SdlMessageDialog*) sdl_message_dialog_new (NULL,
			SDL_DIALOG_DESTROY_WITH_PARENT,
			SDL_MESSAGE_ERROR,
			SDL_BUTTONS_OK,
			gettext("Cannot find Ambulant Player Help"));
		sdl_dialog_run (SDL_DIALOG (dialog));
		sdl_surface_destroy (SDL_SURFACE (dialog));
	}
}

void
sdl_gui::do_logger_window() {
	SdlWindow* logger_window =	sdl_logger::get_sdl_logger()->get_logger_window();
	if (SDL_SURFACE_VISIBLE (SDL_SURFACE (logger_window))) {
		sdl_surface_hide(SDL_SURFACE (logger_window));
	} else {
		sdl_surface_show(SDL_SURFACE (logger_window));
	}
}

void
sdl_gui::fileError(const gchar* smilfilename) {
	char buf[1024];
	sprintf(buf, gettext(gettext("%s: Cannot open file: %s")), (const char*) smilfilename, strerror(errno));
	SdlMessageDialog* dialog = (SdlMessageDialog*) sdl_message_dialog_new (NULL,
		SDL_DIALOG_DESTROY_WITH_PARENT,
		SDL_MESSAGE_ERROR,
		SDL_BUTTONS_OK,
		"%s",
		m_programfilename);
	sdl_message_dialog_format_secondary_text(dialog, "%s", buf);
	sdl_dialog_run (SDL_DIALOG (dialog));
	sdl_surface_destroy (SDL_SURFACE (dialog));
}

void
sdl_gui::do_open(){
  m_file_chooser = SDL_FILE_CHOOSER (sdl_file_chooser_dialog_new(gettext("Please, select a file"), NULL, SDL_FILE_CHOOSER_ACTION_OPEN, SDL_STOCK_CANCEL, SDL_RESPONSE_CANCEL, SDL_STOCK_OPEN, SDL_RESPONSE_ACCEPT, NULL));

	SdlFileFilter *filter_smil = sdl_file_filter_new();
	sdl_file_filter_set_name(filter_smil, gettext("SMIL files"));
	sdl_file_filter_add_pattern(filter_smil, "*.smil");
	sdl_file_filter_add_pattern(filter_smil, "*.smi");
	sdl_file_chooser_add_filter(m_file_chooser, filter_smil);
	SdlFileFilter *filter_all = sdl_file_filter_new();
	sdl_file_filter_set_name(filter_all, gettext("All files"));
	sdl_file_filter_add_pattern(filter_all, "*.smil");
	sdl_file_filter_add_pattern(filter_all, "*.smi");
	sdl_file_filter_add_pattern(filter_all, "*.grins");
	sdl_file_chooser_add_filter(m_file_chooser, filter_all);
	SdlFileFilter *filter_any = sdl_file_filter_new();
	sdl_file_filter_set_name(filter_any, gettext("Any file"));
	sdl_file_filter_add_pattern(filter_any, "*");
	sdl_file_chooser_add_filter(m_file_chooser, filter_any);

	gint result = sdl_dialog_run (SDL_DIALOG (m_file_chooser));
	if (result == SDL_RESPONSE_ACCEPT) {
		do_file_selected();
	}
	sdl_surface_hide(SDL_SURFACE (m_file_chooser));
}

void sdl_gui::do_file_selected() {
	const gchar *smilfilename  = sdl_file_chooser_get_filename (m_file_chooser);
	if (openSMILfile(smilfilename, 0, true)) {
		g_free((void*)smilfilename);
		do_play();
	}
}

void
sdl_gui::setDocument(const char* smilfilename) {
}

void
sdl_gui::do_settings_selected() {
	const gchar *settings_filename = sdl_file_chooser_get_filename (m_settings_chooser);
	if ( settings_filename != NULL) {
		smil2::test_attrs::load_test_attrs(settings_filename);
		if (openSMILfile(m_smilfilename, 0, false))
			do_play();
	}
}

void
sdl_gui::do_load_settings() {
	if (m_gui_player && m_gui_player->is_open())
		do_stop();

	m_settings_chooser = SDL_FILE_CHOOSER (sdl_file_chooser_dialog_new(gettext("Please, select a settings file"), NULL, SDL_FILE_CHOOSER_ACTION_OPEN, SDL_STOCK_CANCEL, SDL_RESPONSE_CANCEL, SDL_STOCK_OPEN, SDL_RESPONSE_ACCEPT, NULL));

	SdlFileFilter *filter_xml = sdl_file_filter_new();
	sdl_file_filter_set_name(filter_xml, gettext("XML files"));
	sdl_file_filter_add_pattern(filter_xml, "*.xml");
	sdl_file_chooser_add_filter(m_settings_chooser, filter_xml);

	gint result = sdl_dialog_run (SDL_DIALOG (m_settings_chooser));
	if (result == SDL_RESPONSE_ACCEPT) {
		do_settings_selected();
	}
	sdl_surface_hide(SDL_SURFACE (m_settings_chooser));

	/* Ensure that the dialog box is hidden when the user clicks a button.
	We don't need anymore the callbacks, because they can be embedded in this part of the code */
/*
	g_signal_connect_swapped (SDL_FILE_SELECTION (m_settings_selector)->ok_button,
		"clicked",
		G_CALLBACK (sdl_surface_hide),
		m_settings_selector);

	g_signal_connect_swapped (SDL_FILE_SELECTION (m_settings_selector)->cancel_button,
		"clicked",
		G_CALLBACK (sdl_surface_hide),
		m_settings_selector);
	g_signal_connect_swapped (SDL_OBJECT ((m_settings_selector)->ok_button),"clicked", G_CALLBACK (sdl_C_callback_settings_selected),(void*) this);
*/
}

void
sdl_gui::do_open_url() {
	SdlDialog* url_dialog =	 SDL_DIALOG (sdl_dialog_new_with_buttons(
		"AmbulantPlayer",
		NULL,
		SDL_DIALOG_DESTROY_WITH_PARENT,
		SDL_STOCK_OK,
		SDL_RESPONSE_ACCEPT,
		SDL_STOCK_CANCEL,
		SDL_RESPONSE_REJECT,
		NULL));

	SdlLabel* label = SDL_LABEL (sdl_label_new(gettext("URL to open:")));
	sdl_misc_set_alignment (SDL_MISC (label), 0, 0);
	sdl_surface_show(SDL_SURFACE (label));

	m_url_text_entry = SDL_ENTRY (sdl_entry_new());
	sdl_entry_set_editable(m_url_text_entry, true);
	sdl_entry_set_text(m_url_text_entry,"http://www");
	sdl_surface_show(SDL_SURFACE (m_url_text_entry));

	sdl_container_add(SDL_CONTAINER(SDL_DIALOG(url_dialog)->vbox), SDL_SURFACE (label));
	sdl_container_add(SDL_CONTAINER(SDL_DIALOG(url_dialog)->vbox), SDL_SURFACE (m_url_text_entry));

	gint result = sdl_dialog_run (SDL_DIALOG (url_dialog));
	if (result == SDL_RESPONSE_ACCEPT) {
		do_url_selected();
	}
	sdl_surface_destroy (SDL_SURFACE (url_dialog));
/*
	g_signal_connect_swapped (url_dialog,"response", G_CALLBACK (sdl_C_callback_url_selected), (void*) this);

	g_signal_connect_swapped (url_dialog, "response",
		G_CALLBACK (sdl_surface_hide),
		SDL_SURFACE (url_dialog));
	sdl_surface_show_all(SDL_SURFACE (url_dialog));
*/
}

void sdl_gui::do_url_selected() {
	// Next string points to internally allocated storage in the surface
	// and must not be freed, modified or stored.
	const gchar *smilfilename  = sdl_entry_get_text(m_url_text_entry);
	sdl_window_set_title(SDL_WINDOW (m_toplevelcontainer), smilfilename);
	if (openSMILfile(smilfilename, 0, true)) {
		do_play();
	}
}

void
sdl_gui::do_player_done() {
	_update_menus();
}

void
sdl_gui::need_redraw (const void* r, void* w, const void* pt) {

	g_signal_emit(SDL_OBJECT (m_toplevelcontainer), signal_need_redraw_id, 0, r, w, pt);
}

void
sdl_gui::player_done() {
	g_signal_emit(SDL_OBJECT (m_toplevelcontainer), signal_player_done_id, 0);
}

void
no_fileopen_infodisplay(sdl_gui* w, const char* caption) {
	SdlMessageDialog* dialog = (SdlMessageDialog*) sdl_message_dialog_new (NULL,
		SDL_DIALOG_DESTROY_WITH_PARENT,
		SDL_MESSAGE_INFO,
		SDL_BUTTONS_OK,
		"%s",
		caption);
	sdl_message_dialog_format_secondary_markup (dialog, "No file open: Please first select File->Open");
	sdl_dialog_run (SDL_DIALOG (dialog));
	sdl_surface_destroy (SDL_SURFACE (dialog));
}

void
sdl_gui::do_play() {
	assert(m_gui_player);
	m_gui_player->play();
	_update_menus();
}

void
sdl_gui::do_pause() {
	assert(m_gui_player);
	m_gui_player->pause();
	_update_menus();
}

void
sdl_gui::do_reload() {
	assert(m_gui_player);
	m_gui_player->restart(true);
	_update_menus();
}

void
sdl_gui::do_stop() {
	assert(m_gui_player);
	if(m_gui_player)
		m_gui_player->stop();
	_update_menus();
}

void
sdl_gui::do_settings_select() {

	m_settings = new sdl_settings();
	gint result = sdl_dialog_run (SDL_DIALOG (m_settings->getSurface()));
	if (result == SDL_RESPONSE_ACCEPT) {
		m_settings->settings_ok();
	}
	sdl_surface_destroy (SDL_SURFACE (m_settings->getSurface()));
}

void
sdl_gui::do_quit() {
	if (m_gui_player) {
		m_gui_player->stop();
		delete m_gui_player;
		m_gui_player = NULL;
	}
	g_main_loop_quit (main_loop);
}

void
sdl_gui::do_resize(GdkEventConfigure *event) {
//	sdl_window_set_default_size(m_toplevelcontainer, event->width, menubar->allocation.height + event->height);
}


void
sdl_gui::unsetCursor() {
//	m_gui_player->set_cursor(0);
}

void
sdl_gui::do_internal_message(sdl_message_event* e) {
	char* msg = (char*)e->get_message();
	int level = e->get_type() - sdl_logger::CUSTOM_OFFSET;
	static char* last_msg = NULL;
	if (level >= ambulant::lib::logger::LEVEL_SHOW) {
		/* do not redisplay same windowed (modal) message */
		if (last_msg != NULL && msg != NULL && strcmp(last_msg, msg) == 0) {
			free (msg);
			return;
		} else if (msg == NULL) {
			return;
		} else {
			if (last_msg != NULL)
				free (last_msg);
			last_msg = strdup (msg);
		}
	}
	SdlMessageDialog* dialog; // just in case is needed

	switch (level) {
	case sdl_logger::CUSTOM_NEW_DOCUMENT:
		if (m_gui_player) {
			bool start = msg[0] == 'S' ? true : false;
			bool old = msg[2] == 'O' ? true : false;
			m_gui_player->player_start(&msg[4], start, old);
		}
		break;
	case sdl_logger::CUSTOM_LOGMESSAGE:
#ifdef	LOCK_MESSAGE
		if (pthread_mutex_lock(&m_lock_message) != 0){
			printf("pthread_mutex_lock(&m_lock_message) sets errno to %d.\n", errno);
			abort();
		}
#endif /*LOCK_MESSAGE*/
		// mutex lcoking is required for calling sdl_text_buffer_insert_at_cursor()
		// Otherwiser SDL+ may crash with a cd $Ecryptic message suck as:
		// Sdl-WARNING **: Invalid text buffer iterator: either the iterator is
		// uninitialized, or the characters/pixbufs/surfaces in the buffer have been
		// modified since the iterator was created.
		sdl_text_buffer_insert_at_cursor(SDL_TEXT_BUFFER (sdl_logger::get_sdl_logger()->get_logger_buffer()), msg, strlen(msg));
#ifdef	LOCK_MESSAGE
		pthread_mutex_unlock(&m_lock_message);
#endif /*LOCK_MESSAGE*/
		break;
	case ambulant::lib::logger::LEVEL_FATAL:
		dialog = (SdlMessageDialog*) sdl_message_dialog_new (NULL,
			SDL_DIALOG_DESTROY_WITH_PARENT,
			SDL_MESSAGE_ERROR,
			SDL_BUTTONS_OK,
			"%s",
			msg);
		g_signal_connect_swapped (dialog, "response",
			G_CALLBACK (sdl_surface_destroy),
			dialog);
		sdl_surface_show_all (SDL_SURFACE(dialog));
		break;
	case ambulant::lib::logger::LEVEL_ERROR:
		dialog = (SdlMessageDialog*) sdl_message_dialog_new (NULL,
			SDL_DIALOG_DESTROY_WITH_PARENT,
			SDL_MESSAGE_WARNING,
			SDL_BUTTONS_OK,
			"%s",
			msg);
		g_signal_connect_swapped (dialog, "response",
			G_CALLBACK (sdl_surface_destroy),
			dialog);
		sdl_surface_show_all (SDL_SURFACE(dialog));
		break;
	case ambulant::lib::logger::LEVEL_WARN:
	default:
		dialog = (SdlMessageDialog*) sdl_message_dialog_new (NULL,
			SDL_DIALOG_DESTROY_WITH_PARENT,
			SDL_MESSAGE_INFO,
			SDL_BUTTONS_OK,
			"%s",
			msg);
		g_signal_connect_swapped (dialog, "response",
			G_CALLBACK (sdl_surface_destroy),
			dialog);
		sdl_surface_show_all (SDL_SURFACE(dialog));
		break;
	}
#ifdef	LOCK_MESSAGE
	if (level >= ambulant::lib::logger::LEVEL_WARN) {
		pthread_mutex_lock(&m_lock_message);
		pthread_cond_signal(&m_cond_message);
		pthread_mutex_unlock(&m_lock_message);
	}
#endif/*LOCK_MESSAGE*/
	free(msg);
	delete e;
}

void
sdl_gui::internal_message(int level, char* msg) {

	int msg_id = level+sdl_logger::CUSTOM_OFFSET;
	sdl_message_event* event = new sdl_message_event(msg_id, msg);
	g_signal_emit(SDL_OBJECT (m_toplevelcontainer), signal_internal_message_id, 0, event);

#ifdef	LOCK_MESSAGE
	if (level >= ambulant::lib::logger::LEVEL_WARN
		&& pthread_self() != m_gui_thread)
	{
		// wait until the message as been OK'd by the user
		pthread_mutex_lock(&m_lock_message);
		pthread_cond_wait(&m_cond_message, &m_lock_message);
		pthread_mutex_unlock(&m_lock_message);
	}
#endif /*LOCK_MESSAGE*/
}

void
sdl_gui::_update_menus()
{
	sdl_action_set_sensitive(sdl_action_group_get_action (m_actions, "play"),
		m_gui_player && m_gui_player->is_play_enabled() && ! m_gui_player->is_play_active());
	sdl_action_set_sensitive(sdl_action_group_get_action (m_actions, "pause"),
		m_gui_player && m_gui_player->is_pause_enabled() && ! m_gui_player->is_pause_active());
	sdl_action_set_sensitive(sdl_action_group_get_action (m_actions, "stop"),
		m_gui_player && m_gui_player->is_stop_enabled() && ! m_gui_player->is_stop_active());
	sdl_action_set_sensitive(sdl_action_group_get_action (m_actions, "reload"),
		(m_gui_player != NULL));
}
#endif//JNK
#endif//WITH_SDL2XX
#ifdef WITH_SDL2


bool
sdl_gui::openSMILfile(const char *smilfilename, int mode, bool dupFlag) {

	if (smilfilename==NULL)
		return false;

//TBD	sdl_window_set_title(SDL_WINDOW (m_toplevelcontainer), smilfilename);
	if (dupFlag) {
		if (m_smilfilename) free ((void*)m_smilfilename);
		m_smilfilename = strdup(smilfilename);
	}
	if (m_gui_player)
		delete m_gui_player;

	m_gui_player = new sdl_gui_player(this);
	return m_gui_player->is_open();
}

void
sdl_gui::quit() {
	if (m_gui_player) {
		m_gui_player->stop();
		delete m_gui_player;
		m_gui_player = NULL;
	}
	SDL_Quit ();
}

// sdl_gather_events collects  all window and redraw events into a single redraw() call
// using SDL_PeepEvents()
void
sdl_gather_events () {

}

void
sdl_gui::sdl_loop() {
	bool busy = true;
	SDL_Event event;

	while (busy) {
		if (SDL_WaitEvent(&event) == 0) {
			lib::logger::get_logger()->fatal("ambulant::sdl_gui::sdl_loop(0x%x): SDL error %s", this, SDL_GetError());
			busy = false;
		}
		switch (event.type) {
		case SDL_QUIT:
			busy = false;
			break;
		case SDL_USEREVENT:
			AM_DBG lib::logger::get_logger()->debug("%s SDL_USEREVENT: code=%d data1=0x%x data2=0x%x",__PRETTY_FUNCTION__, event.user.code,event.user.data1,event.user.data2);
			if (event.user.code == 317107) {
				if (m_gui_player != NULL) {
					m_gui_player->redraw(event.user.data1, event.user.data2);
				}
 			}
			break;
		case SDL_WINDOWEVENT:
			ambulant::gui::sdl::ambulant_sdl_window* asw; //XX no refs to 'ambulant' 
			ambulant::gui::sdl::sdl_ambulant_window* saw;
			saw = ambulant::gui::sdl::sdl_ambulant_window::get_sdl_ambulant_window (event.window.windowID);
			AM_DBG lib::logger::get_logger()->debug("%s SDL_WINDOWEVENT: type=%d windowID=%d code=%d data1=0x%x data2=0x%x saw=0x%x",__PRETTY_FUNCTION__, event.window.type, event.window.windowID, event.window.event,event.window.data1,event.window.data2, saw);
			if (saw != NULL && (asw = saw->get_ambulant_sdl_window()) != NULL) {
				common::player* player = m_gui_player->get_player();
				if (player != NULL && saw->get_evp() == NULL) {
					// First window event after creation, complete initialization
					saw->set_evp(player->get_evp()); // for timestamps
					saw->get_ambulant_sdl_window()->set_gui_player (m_gui_player);
				}
				ambulant::lib::rect r; //XX no refs to 'ambulant' 
				switch ( event.window.event ) {
				case  SDL_WINDOWEVENT_SHOWN:
				case  SDL_WINDOWEVENT_EXPOSED:
				case  SDL_WINDOWEVENT_FOCUS_GAINED:
					r = asw->get_bounds();
					asw->redraw(r);
					break;
				default:  
					break;
				}
			}
			break;
		case SDL_MOUSEMOTION: // mouse moved
			AM_DBG lib::logger::get_logger()->debug("%s SDL_MOUSEMOTION: type=%d windowID=%d which=%d state=%d x=%d y=%d relx=%d rely=%d",__PRETTY_FUNCTION__, event.motion.type,  event.motion.windowID, event.motion.which, event.motion.state,event.motion.x,event.motion.y,event.motion.xrel,event.motion.yrel);
			if (m_gui_player != NULL) {
				SDL_Point p;
				p.x = event.motion.x;
				p.y = event.motion.y;
				m_gui_player->before_mousemove(0);
				m_gui_player->user_event(p, 1);
				if (m_gui_player->after_mousemove()) {
					SDL_SetCursor (m_hand_cursor);
				} else {
					SDL_SetCursor (m_arrow_cursor);
				}	
			}
			break;
		case SDL_MOUSEBUTTONDOWN: // mouse button pressed
		case SDL_MOUSEBUTTONUP: // mouse button released
			AM_DBG lib::logger::get_logger()->debug("%s %s: type=%d windowID=%d which=%d button=%d, state=%d, x=%d y=%d",__PRETTY_FUNCTION__, event.button.state ? "SDL_MOUSEBUTTONDOWN":"SDL_MOUSEBUTTONUP", event.button.type,  event.button.windowID, event.button.which, event.button.button, event.button.state ,event.button.x,event.button.y);
			if (m_gui_player != NULL && event.button.state == 0) { // button released
				SDL_Point p;
				p.x = event.motion.x;
				p.y = event.motion.y;
				m_gui_player->user_event(p, 0);
			}
			break;
		case SDL_MOUSEWHEEL: // mouse wheel motion
			break;
		default:
			break;
		}
	}
}

int
main (int argc, char*argv[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
		fprintf (stderr, "Ambulant: SDL_Init failed: %s\n", SDL_GetError());
		exit (-1);
	}
	int img_flags = 0; // XXXJACK: don't think we need these: IMG_INIT_JPG | IMG_INIT_PNG ; 
	if (IMG_Init(img_flags) != img_flags) {
		fprintf (stderr, "Ambulant: IMG_Init failed: %s\n", SDL_GetError());
		exit (-2);
	}
#ifdef	ENABLE_NLS
	// Load localisation database
	bool private_locale = false;
	char *home = getenv("HOME");
	if (home) {
		std::string localedir = std::string(home) + "/.ambulant/locale";
		if (access(localedir.c_str(), 0) >= 0) {
			private_locale = true;
			bindtextdomain(PACKAGE, localedir.c_str());
		}
	}
	if (!private_locale)
		bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
#endif /*ENABLE_NLS*/

	// Load preferences, initialize app and logger
	unix_preferences unix_prefs;
	unix_prefs.load_preferences();
// TBD begin code from gtk_logger
	common::preferences* prefs = common::preferences::get_preferences();
	lib::logger* logger = lib::logger::get_logger();
	// Connect logger to our message displayer and output processor
	// logger->set_show_message(show_message);
	// Tell the logger about the output level preference
	int level = prefs->m_log_level;
	logger->set_level(level);
// TBD end code from gtk_logger

	/* Setup surface */
	sdl_gui *gui = new sdl_gui(argv[0], NULL);
	// take log level from preferences
//TBD	sdl_logger::set_sdl_logger_gui(gui);
//TBD	sdl_logger* sdl_logger = sdl_logger::get_sdl_logger();
//TBD	lib::logger::get_logger()->debug("Ambulant Player: now logging to a window");
//TBD	// Print welcome banner
//TBD	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
//TBD	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Unix/SDL"), __DATE__);
#if ENABLE_NLS
//TBD	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif

	bool exec_flag = true; // for make check

	if (argc > 1) {
		char last[6];
		char* str = argv[argc-1];
		// If the URL starts with "ambulant:" this is the trick-uri-scheme to
		// open URLs in Ambulant from the browser. Remove the trick.
		if (strncmp(str, "ambulant:", 9) == 0)
			str += 9;
		int len = strlen(str);
		strcpy(last, &str[len-5]);
		if (strcmp(last, ".smil") == 0
			|| strcmp(&last[1], ".smi") == 0
			|| strcmp(&last[1], ".sml") == 0)
		{
			if (gui->openSMILfile(str, 0, true) && exec_flag) {
				gui->m_gui_player->play();
			}
		}
	} else {
		preferences* prefs = preferences::get_preferences();
		if ( ! prefs->m_welcome_seen) {
			const char *welcome_doc = find_datafile(welcome_locations);
			if (welcome_doc && gui->openSMILfile(welcome_doc, 0, true)) {
				gui->m_gui_player->play();
				prefs->m_welcome_seen = true;
			}
		}
		exec_flag = true;
	}
	gui->sdl_loop();
//TBD	unix_prefs.save_preferences();
//TBD	delete sdl_logger::get_sdl_logger();
	gui->quit();
	delete gui;

	SDL_Quit();

	return exec_flag ? 0 : -1;
}

#endif//WITH_SDL2
