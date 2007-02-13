// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* gtk_gui.cpp - GTK GUI for Ambulant
 *              
 */

#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>
#include <fcntl.h>
#include "gtk_gui.h"
#include "gtk_mainloop.h"
#include "gtk_logger.h"
#include "gtk_renderer.h"
#ifdef	WITH_GSTREAMER
#include "ambulant/gui/gstreamer/gstreamer_renderer_factory.h"
#endif/*WITH_GSTREAMER*/

#if 1
#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define	WITH_GTK_LOGGER

//KB XXXX FIXME 
#ifdef  WITH_NOKIA770
#ifdef  AMBULANT_DATADIR  //KB XXXX
#undef  AMBULANT_DATADIR
#define AMBULANT_DATADIR "/usr/share/ambulant"
#endif/*AMBULANT_DATADIR*/
#define UI_FILENAME AMBULANT_DATADIR "/ui_manager.xml"
#include <hildon-widgets/hildon-program.h>
#include <glib-object.h>
#else /*WITH_NOKIA770*/
#define UI_FILENAME "ui_manager.xml"
#endif/*WITH_NOKIA770*/

const char *ui_description =
	"<ui> \n"
		"<menubar name=\"MenuBar\"> \n"
			"<menu action=\"FileMenu\"> \n"
				"<menuitem action=\"open\"/> \n"
				"<menuitem action=\"openurl\"/> \n"
				"<menuitem action=\"reload\"/> \n"
				"<separator/> \n"
				"<menuitem action=\"preferences\"/> \n"
				"<separator/> \n"
				"<menuitem action=\"quit\"/> \n"
			"</menu> \n"
			"<menu action=\"PlayMenu\"> \n"
				"<menuitem action=\"play\"/> \n"
				"<menuitem action=\"pause\"/> \n"
				"<menuitem action=\"stop\"/> \n"
			"</menu> \n"
			"<menu action=\"ViewMenu\"> \n"
				"<menuitem action=\"fullscreen\"/> \n"
				"<menuitem action=\"window\"/> \n"
				"<separator/> \n"
				"<menuitem action=\"loadsettings\"/> \n"
				"<separator/> \n"
				"<menuitem action=\"logwindow\"/> \n" 
			"</menu> \n"
			"<menu action=\"HelpMenu\"> \n"
				"<menuitem action=\"about\"/> \n"
				"<menuitem action=\"help\"/> \n"
				"<separator/> \n"
				"<menuitem action=\"website\"/> \n"
				"<separator/> \n"
				"<menuitem action=\"welcome\"/> \n"
			"</menu> \n"
		"</menubar> \n"
	"</ui> \n"
	"\0";

const char *about_text = 
	"Ambulant SMIL 2.1 player.\n"
	"Copyright Stichting CWI, 2003-2007.\n\n"
	"License: LGPL.";


// Places where to look for the Welcome document
const char *welcome_locations[] = {
	"Welcome/Welcome.smil",
	"../Welcome/Welcome.smil",
	"Extras/Welcome/Welcome.smil",
	"../Extras/Welcome/Welcome.smil",
#ifdef AMBULANT_DATADIR
	AMBULANT_DATADIR "/Welcome/Welcome.smil",
#else
	"/usr/local/share/ambulant/Welcome/Welcome.smil",
#endif
#ifdef	GTK_NO_FILEDIALOG	/* Assume embedded Qt */
	"/home/zaurus/Documents/Ambulant/Extras/Welcome/Welcome.smil",
#endif/*QT_NO_FILEDIALOG*/
	NULL
};

// Places where to look for the helpfile
const char *helpfile_locations[] = {
	"Documentation/user/index.html",
	"../Documentation/user/index.html",
#ifdef AMBULANT_DATADIR
	AMBULANT_DATADIR "/AmbulantPlayerHelp/index.html",
#else
	"/usr/local/share/ambulant/AmbulantPlayerHelp/index.html",
#endif
	NULL
};

static GdkPixmap *pixmap = NULL;

// callbacks for C++
/* File */
extern "C" {
gboolean gtk_C_callback_timer(void *userdata)
{
//	AM_DBG lib::logger::get_logger()->debug("gtk_C_callback_timer called");
	return TRUE;
}
}
extern "C" {
void gtk_C_callback_resize(void *userdata, GdkEventConfigure *event, GtkWidget *widget)
{
	((gtk_gui*) userdata)->do_resize(event);
}
}
extern "C" {
void gtk_C_callback_open(void *userdata)
{
	((gtk_gui*) userdata)->do_open();
}
}
extern "C" {
void gtk_C_callback_open_url(void *userdata)
{
	((gtk_gui*) userdata)->do_open_url();
}
void gtk_C_callback_reload(void *userdata)
{
	((gtk_gui*) userdata)->do_reload();
}
void gtk_C_callback_settings_select(void *userdata)
{
	((gtk_gui*) userdata)->do_settings_select();
}
void gtk_C_callback_quit(void *userdata)
{
	((gtk_gui*) userdata)->do_quit();
}
/* Play */
void gtk_C_callback_play(void *userdata)
{
	((gtk_gui*) userdata)->do_play();
}
void gtk_C_callback_pause(void *userdata)
{
	((gtk_gui*) userdata)->do_pause();
}
void gtk_C_callback_stop(void *userdata)
{
	((gtk_gui*) userdata)->do_stop();
}
/* View */
void gtk_C_callback_full_screen(void *userdata)
{
	gtk_window_fullscreen(((gtk_gui*) userdata)->get_toplevel_container());
}
void gtk_C_callback_normal_screen(void *userdata)
{
	gtk_window_unfullscreen(((gtk_gui*) userdata)->get_toplevel_container());
}
void gtk_C_callback_load_settings(void *userdata)
{
	((gtk_gui*) userdata)->do_load_settings();
}
void gtk_C_callback_logger_window(void *userdata)
{
	((gtk_gui*) userdata)->do_logger_window();
}
/* Help */
void gtk_C_callback_about(void *userdata)
{
	((gtk_gui*) userdata)->do_about();
}
void gtk_C_callback_help(void *userdata)
{
	((gtk_gui*) userdata)->do_help();
}
void gtk_C_callback_homepage(void *userdata)
{
	((gtk_gui*) userdata)->do_homepage();
}
void gtk_C_callback_welcome(void *userdata)
{
	((gtk_gui*) userdata)->do_welcome();
}
}
extern "C" {
void gtk_C_callback_do_player_done(void *userdata)
{
	((gtk_gui*) userdata)->do_player_done();
}
void gtk_C_callback_do_need_redraw(void *userdata, void* r_call, void* w_call, void* pt_call)
{
	const void* r = r_call;
	void* w = w_call;
	const void* pt = pt_call;
//	((gtk_gui*) userdata)->(r, w, pt);
}
void gtk_C_callback_do_internal_message(void *userdata, void* e)
{
	gtk_message_event* event = (gtk_message_event*) e;	
	((gtk_gui*) userdata)->do_internal_message(event);
}
}

static const char * 
find_datafile(const char **locations)
{
	const char **p;
	for(p = locations; *p; p++) {
		if (access(*p, 0) >= 0) return *p;
	}
	return NULL;
}

gtk_gui::gtk_gui(const char* title,
	       const char* initfile)
 :
// initialize all dynamic data pointers in the same order as they are declared
	m_programfilename(NULL),
	m_smilfilename(NULL),
	m_settings(NULL),
	m_toplevelcontainer(NULL),
	menubar(NULL),
	m_guicontainer(NULL),
	m_documentcontainer(NULL),
	m_actions(NULL),
#ifdef	LOCK_MESSAGE
	m_gui_thread(0),
#endif/*LOCK_MESSAGE*/
	m_file_chooser(NULL),
	m_settings_chooser(NULL),
	m_url_text_entry(NULL),
	m_mainloop(NULL)
{

	GError *error = NULL;

	// Initialization of the Menu Bar Items
	// There is a problem in here because the callbacks in Actions go like g_signal_connect (but, we need g_sginal_connect_swapped)
	static GtkActionEntry entries[] = {
	{ "FileMenu", NULL, gettext_noop("File")},
	{ "open", GTK_STOCK_OPEN, gettext_noop("Open..."), "<alt>F", gettext_noop("Open a file"), NULL},
	{ "openurl", NULL, gettext_noop("Open URL..."), "<alt>U", gettext_noop("Open a url"), NULL},
	{ "reload", GTK_STOCK_REFRESH, gettext_noop("Reload"), "<alt>R", gettext_noop("Reload a document"), NULL},
	{ "preferences", GTK_STOCK_PREFERENCES , gettext_noop("Preferences"), "<alt>S", gettext_noop("Launch preferences window"), NULL},
	{ "quit", GTK_STOCK_QUIT, gettext_noop("Quit"), "<alt>Q", gettext_noop("Quit ambulant"), NULL},	
	// Play Menu
	{ "PlayMenu", "<alt>Y", gettext_noop("Play")},
	{ "play", GTK_STOCK_MEDIA_PLAY, gettext_noop("Play"), "<alt>Y", gettext_noop("Play document"), NULL},
	{ "pause", GTK_STOCK_MEDIA_PAUSE, gettext_noop("Pause"), "<alt>P", gettext_noop("Pause document"), NULL},
	{ "stop", GTK_STOCK_MEDIA_STOP, gettext_noop("Stop"), "<alt>S", gettext_noop("Stop document"), NULL},
	// View Menu
	{ "ViewMenu", "<alt>V", gettext_noop("View")},
	{ "fullscreen", NULL, gettext_noop("Full Screen"), "<alt>F", gettext_noop("Full Screen"), NULL},
	{ "window", NULL, gettext_noop("Window"), "<alt>W", gettext_noop("Normal Screen"), NULL},
	{ "loadsettings", GTK_STOCK_PROPERTIES, gettext_noop("Load Settings..."), "<alt>R", gettext_noop("Launch the settings window"), NULL},
	{ "logwindow", NULL, gettext_noop("Log Window..."), "<alt>L", gettext_noop("Launch log window"), NULL},
	// Help Menu
	{ "HelpMenu", "<alt>H", gettext_noop("Help")},
	{ "about", GTK_STOCK_ABOUT, gettext_noop("About AmbulantPlayer"), "<alt>A", gettext_noop("Information about Ambulant"), NULL},
	{ "help", GTK_STOCK_HELP, gettext_noop("AmbulantPlayer Help"), "<alt>H", gettext_noop("Help in AmbulantPlayer Webpage"), NULL},
	{ "website", NULL, gettext_noop("AmbulantPlayer Website..."), "<alt>W", gettext_noop("Open the Ambulant Webpage"), NULL},
	{ "welcome", GTK_STOCK_HOME, gettext_noop("Play Welcome Document"), "<alt>P", gettext_noop("Plays a simple SMIL file"), NULL}
	};

	int n_entries = G_N_ELEMENTS(entries);

	m_programfilename = title;
#ifdef	LOCK_MESSAGE
	pthread_cond_init(&m_cond_message, NULL);
	pthread_mutex_init(&m_lock_message, NULL);
	m_gui_thread = pthread_self();
#endif/*LOCK_MESSAGE*/
	if (initfile != NULL && initfile != "")
		m_smilfilename = strdup(initfile);

	/*Initialization of the Main Window */
	m_toplevelcontainer = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title(m_toplevelcontainer, initfile);
	gtk_window_set_resizable(m_toplevelcontainer, true); 	
	gtk_widget_set_size_request(GTK_WIDGET (m_toplevelcontainer), 150, 150);
	gtk_widget_set_uposition(GTK_WIDGET (m_toplevelcontainer), 240, 320);	
	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "delete-event", G_CALLBACK (gtk_C_callback_quit), (void *) this);
	// Callback for the resize events
	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "expose-event", G_CALLBACK (gtk_C_callback_resize), (void *) this);

	/* Initialization of the signals */
	signal_player_done_id = g_signal_new ("signal-player-done", gtk_window_get_type(), G_SIGNAL_RUN_LAST, 0, 0, 0, g_cclosure_marshal_VOID__VOID,GTK_TYPE_NONE, 0, NULL);

	signal_need_redraw_id = g_signal_new ("signal-need-redraw", gtk_window_get_type(), G_SIGNAL_RUN_LAST, 0, 0, 0, gtk_marshal_NONE__POINTER_POINTER_POINTER,GTK_TYPE_NONE, 3, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER);
	
	signal_internal_message_id = g_signal_new ("signal-internal-message", gtk_window_get_type(), G_SIGNAL_RUN_LAST, 0, 0, 0, gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1, G_TYPE_POINTER);

	// Signal connections
	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "signal-player-done",  G_CALLBACK (gtk_C_callback_do_player_done), (void*)this);
	
	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "signal-need-redraw",  G_CALLBACK (gtk_C_callback_do_player_done), (void*)this);

	g_signal_connect_swapped (GTK_OBJECT (m_toplevelcontainer), "signal-internal-message",  G_CALLBACK (gtk_C_callback_do_internal_message), (void*)this);

	/* VBox (m_guicontainer) to place the Menu bar in the correct place */ 
	m_guicontainer = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(m_toplevelcontainer), GTK_WIDGET (m_guicontainer));
	
	/* The Action Group that includes the menu bar */
	m_actions = gtk_action_group_new("Actions");
	AM_DBG printf("%s0x%X\n", "gtk_gui::gtk_gui(), PACKAGE=", PACKAGE);
	gtk_action_group_set_translation_domain(m_actions, PACKAGE);
	gtk_action_group_add_actions(m_actions, entries, n_entries, (void*)this);

	/* The Gtk UI Manager */
	GtkUIManager *ui = gtk_ui_manager_new();

//	if (!gtk_ui_manager_add_ui_from_file(ui, UI_FILENAME, &error))
	if (!gtk_ui_manager_add_ui_from_string(ui, ui_description, -1, &error))
		g_error("Could not merge UI, error was: %s\n", error->message);
	gtk_ui_manager_insert_action_group(ui, m_actions, 0);
	
	/* Disable and make invisible menus and menu items */
#ifdef GTK_NO_FILEDIALOG
	gtk_action_set_sensitive(gtk_action_group_get_action(m_actions, "open"), true);
	gtk_action_set_sensitive(gtk_action_group_get_action(m_actions, "openurl"), false);
#endif

	// The actual activation calls
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "open"), "activate",  G_CALLBACK (gtk_C_callback_open), (void *) this );	
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "openurl"), "activate",  G_CALLBACK (gtk_C_callback_open_url), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "reload"), "activate",  G_CALLBACK (gtk_C_callback_reload), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "preferences"), "activate",  G_CALLBACK (gtk_C_callback_settings_select), (void *) this );	
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "quit"), "activate",  G_CALLBACK (gtk_C_callback_quit), (void*)this);		

	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "play"), "activate",  G_CALLBACK (gtk_C_callback_play), (void*)this);
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "pause"), "activate",  G_CALLBACK (gtk_C_callback_pause), (void*)this );	
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "stop"), "activate",  G_CALLBACK (gtk_C_callback_stop), (void*)this);

	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "fullscreen"), "activate",  G_CALLBACK (gtk_C_callback_full_screen), (void*)this);
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "window"), "activate",  G_CALLBACK (gtk_C_callback_normal_screen), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "loadsettings"), "activate",  G_CALLBACK (gtk_C_callback_load_settings), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "logwindow"), "activate",  G_CALLBACK (gtk_C_callback_logger_window), (void*)this);		

	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "about"), "activate",  G_CALLBACK (gtk_C_callback_about), (void*)this);
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "help"), "activate",  G_CALLBACK (gtk_C_callback_help), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "website"), "activate",  G_CALLBACK (gtk_C_callback_homepage), (void*)this);		
	g_signal_connect_swapped (gtk_action_group_get_action (m_actions, "welcome"), "activate",  G_CALLBACK (gtk_C_callback_welcome), (void*)this);		

	
	/* Creation of the Menubar and Menu Items */
	menubar = gtk_ui_manager_get_widget (ui, "/MenuBar");
	gtk_box_pack_start (GTK_BOX (m_guicontainer), menubar, FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET (m_toplevelcontainer));


	/* Creation of the document area */
	m_documentcontainer = gtk_drawing_area_new();
	gtk_widget_hide(m_documentcontainer);
	gtk_box_pack_start (GTK_BOX (m_guicontainer), m_documentcontainer, TRUE, TRUE, 0);

	// creates the main loop
	main_loop = g_main_loop_new(NULL, FALSE);
	_update_menus();
}


gtk_gui::~gtk_gui() {

	AM_DBG printf("%s0x%X\n", "gtk_gui::~gtk_gui(), m_mainloop=",m_mainloop);

	// remove all dynamic data in the same order as they are declared
	// m_programfilename - not dynamic

	if  (m_smilfilename) {
		free((void*)m_smilfilename);
		m_smilfilename = NULL;
	}
	if  (m_settings) {
		delete m_settings;
		m_settings = NULL;
	}
	if (m_toplevelcontainer) {
		/*
		  Although it seems a good idea to destroy the top-level
		  widget at this point, by some unrevealed magic
		  it seems already been done by gtk, resulting in cryptic
		  error messages like: 
		  GLib-GObject-WARNING **: invalid uninstantiatable type `(null)' in cast to `GtkWidget'
		  Gtk-CRITICAL **: gtk_widget_destroy: assertion `GTK_IS_WIDGET (widget)' failed
		  Commenting out next line doesn't seem to contribute to
		  any extra memory leaks, gtk_init() alone is responsible
		  for almost all of them

		gtk_widget_destroy(GTK_WIDGET (m_toplevelcontainer));
		 */
		m_toplevelcontainer = NULL;
	}
	if (menubar) {
		gtk_widget_destroy(GTK_WIDGET (menubar));
		menubar = NULL;
	}
/*KB these seem to be destroyed already by destroying m_toplevelcontainer
	if (m_guicontainer) {
		gtk_widget_destroy(GTK_WIDGET (m_guicontainer));
		m_guicontainer = NULL;
	}
	if (m_documentcontainer) {
		gtk_widget_destroy(GTK_WIDGET (m_documentcontainer));
		m_documentcontainer = NULL;
	}
*/
	if (m_actions) {
		g_object_unref(G_OBJECT (m_actions));
		m_actions = NULL;
	}
	if (m_file_chooser) {
		gtk_widget_destroy(GTK_WIDGET (m_file_chooser));
		m_file_chooser = NULL;
	}
	if (m_settings_chooser) {
		gtk_widget_destroy(GTK_WIDGET (m_settings_chooser));
		m_settings_chooser = NULL;
	}
	if (m_url_text_entry) {
		gtk_widget_destroy(GTK_WIDGET (m_url_text_entry));
		m_url_text_entry = NULL;
	}
	if (m_mainloop) {
		delete m_mainloop;
		m_mainloop = NULL;
	}
}

GtkWindow*
gtk_gui::get_toplevel_container()
{
	return this->m_toplevelcontainer;
}



GtkWidget*
gtk_gui::get_gui_container()
{
	return this->m_guicontainer;
}

GtkWidget*
gtk_gui::get_document_container()
{
	return this->m_documentcontainer;
}

void 
gtk_gui::do_about() {
	GtkMessageDialog* dialog = 
	(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_INFO,
         GTK_BUTTONS_OK,
         gettext("About AmbulantPlayer"));
	gtk_message_dialog_format_secondary_text(dialog,about_text);
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (GTK_WIDGET (dialog));
}

void 
gtk_gui::do_homepage() {
	open_web_browser("http://www.ambulantplayer.org");
}

void 
gtk_gui::do_welcome() {
	const char *welcome_doc = find_datafile(welcome_locations);
	
	if (welcome_doc) {
	  if( openSMILfile(welcome_doc, 0, true)) {
			do_play();
		}
	} else {
		GtkMessageDialog* dialog = 
		(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_ERROR,
         	GTK_BUTTONS_OK,
	 	gettext("Cannot find Welcome.smil document"));
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}

void 
gtk_gui::do_help() {
	const char *help_doc = find_datafile(helpfile_locations);
	
	if (help_doc) {
		open_web_browser(help_doc);
	} else {
		GtkMessageDialog* dialog = 
		(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_ERROR,
         	GTK_BUTTONS_OK,
	        gettext("Cannot find Ambulant Player Help"));
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
}

void
gtk_gui::do_logger_window() {
	AM_DBG printf("do_logger_window()\n");
	GtkWindow* logger_window =  gtk_logger::get_gtk_logger()->get_logger_window();
	if (GTK_WIDGET_VISIBLE (GTK_WIDGET (logger_window)))
		gtk_widget_hide(GTK_WIDGET (logger_window));
	else
		gtk_widget_show(GTK_WIDGET (logger_window));
}

void
gtk_gui::fileError(const gchar* smilfilename) {
 	char buf[1024];
	sprintf(buf, gettext(gettext("%s: Cannot open file: %s")),
		(const char*) smilfilename, strerror(errno));
	GtkMessageDialog* dialog = 
	(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_ERROR,
         GTK_BUTTONS_OK,
	 m_programfilename);
	gtk_message_dialog_format_secondary_text(dialog, buf);
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (GTK_WIDGET (dialog));
}

bool 
gtk_gui::openSMILfile(const char *smilfilename, int mode, bool dupFlag) {

	if (smilfilename==NULL)
		return false;
	
	gtk_window_set_title(GTK_WINDOW (m_toplevelcontainer), smilfilename);
	if (dupFlag) {
	  if (m_smilfilename) free ((void*)m_smilfilename);
		m_smilfilename = strdup(smilfilename);
	}
	if (m_mainloop)
		delete m_mainloop;
    
	m_mainloop = new gtk_mainloop(this);
	_update_menus();
	return m_mainloop->is_open();
}

void 
gtk_gui::do_open(){
  m_file_chooser = GTK_FILE_CHOOSER (gtk_file_chooser_dialog_new(gettext("Please, select a file"), NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL));

	GtkFileFilter *filter_smil = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_smil, gettext("SMIL files"));
	gtk_file_filter_add_pattern(filter_smil, "*.smil");
	gtk_file_filter_add_pattern(filter_smil, "*.smi");
	gtk_file_chooser_add_filter(m_file_chooser, filter_smil);
	GtkFileFilter *filter_all = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_all, gettext("All files"));
	gtk_file_filter_add_pattern(filter_all, "*.smil");
	gtk_file_filter_add_pattern(filter_all, "*.smi");	
	gtk_file_filter_add_pattern(filter_all, "*.mms");	
	gtk_file_filter_add_pattern(filter_all, "*.grins");	
	gtk_file_chooser_add_filter(m_file_chooser, filter_all);
	GtkFileFilter *filter_any = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_any, gettext("Any file"));
	gtk_file_filter_add_pattern(filter_any, "*");
	gtk_file_chooser_add_filter(m_file_chooser, filter_any);

	gint result = gtk_dialog_run (GTK_DIALOG (m_file_chooser));
  	switch (result){
      	case GTK_RESPONSE_ACCEPT:
		do_file_selected();
         	break;
      	default:
         	break;
    	}
	gtk_widget_hide(GTK_WIDGET (m_file_chooser));	
}

void gtk_gui::do_file_selected() {
	const gchar *smilfilename  = gtk_file_chooser_get_filename (m_file_chooser);
	if (openSMILfile(smilfilename, 0, true)) {
		g_free((void*)smilfilename);
		do_play();
	}
}



void
gtk_gui::setDocument(const char* smilfilename) {
#ifdef	GTK_NO_FILEDIALOG	/* Assume embedded GTK */
	if (openSMILfile(smilfilename, 0, true))
		do_play();
#endif/*GTK_NO_FILEDIALOG*/
}


void
gtk_gui::do_settings_selected() {
	const gchar *settings_filename  = gtk_file_chooser_get_filename (m_settings_chooser);
	if ( settings_filename != NULL) {
		smil2::test_attrs::load_test_attrs(settings_filename);
		if (openSMILfile(m_smilfilename, 0, false))
			do_play();
	}
}

void 
gtk_gui::do_load_settings() {
	if (m_mainloop && m_mainloop->is_open())
		do_stop();
	
	m_settings_chooser = GTK_FILE_CHOOSER (gtk_file_chooser_dialog_new(gettext("Please, select a settings file"), NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL));

	GtkFileFilter *filter_xml = gtk_file_filter_new();
	gtk_file_filter_set_name(filter_xml, gettext("XML files"));
	gtk_file_filter_add_pattern(filter_xml, "*.xml");
	gtk_file_chooser_add_filter(m_settings_chooser, filter_xml);

	gint result = gtk_dialog_run (GTK_DIALOG (m_settings_chooser));
  	switch (result){
      	case GTK_RESPONSE_ACCEPT:
		do_settings_selected();
         	break;
      	default:
         	break;
    	}
	gtk_widget_hide(GTK_WIDGET (m_settings_chooser));	

	/* Ensure that the dialog box is hidden when the user clicks a button.
	We don't need anymore the callbacks, because they can be embedded in this part of the code */
/*
	g_signal_connect_swapped (GTK_FILE_SELECTION (m_settings_selector)->ok_button,
                             "clicked",
                             G_CALLBACK (gtk_widget_hide), 
                             m_settings_selector);

   	g_signal_connect_swapped (GTK_FILE_SELECTION (m_settings_selector)->cancel_button,
                             "clicked",
                             G_CALLBACK (gtk_widget_hide),
                             m_settings_selector);
	g_signal_connect_swapped (GTK_OBJECT ((m_settings_selector)->ok_button),"clicked", G_CALLBACK (gtk_C_callback_settings_selected),(void*) this);
*/
}


void 
gtk_gui::do_open_url() {
	GtkDialog* url_dialog =  GTK_DIALOG (gtk_dialog_new_with_buttons
	("AmbulantPlayer", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	
	GtkLabel* label = GTK_LABEL (gtk_label_new(gettext("URL to open:")));
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
	gtk_widget_show(GTK_WIDGET (label));
	
	m_url_text_entry = GTK_ENTRY (gtk_entry_new());
	gtk_entry_set_editable(m_url_text_entry, true);
	gtk_entry_set_text(m_url_text_entry,"http://www");
	gtk_widget_show(GTK_WIDGET (m_url_text_entry));
	
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(url_dialog)->vbox), GTK_WIDGET (label));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(url_dialog)->vbox), GTK_WIDGET (m_url_text_entry));

	gint result = gtk_dialog_run (GTK_DIALOG (url_dialog));
  	switch (result){
      	case GTK_RESPONSE_ACCEPT:
		do_url_selected();
         	break;
      	default:
         	break;
    	}
	gtk_widget_destroy (GTK_WIDGET (url_dialog));
/*	
	g_signal_connect_swapped (url_dialog,"response", G_CALLBACK (gtk_C_callback_url_selected), (void*) this);
	
	g_signal_connect_swapped (url_dialog, "response",
                             G_CALLBACK (gtk_widget_hide), 
                            GTK_WIDGET (url_dialog));
	gtk_widget_show_all(GTK_WIDGET (url_dialog));
*/
}

void gtk_gui::do_url_selected() {
	/* Next string points to internally allocated storage in the widget
	   and must not be freed, modified or stored.*/
	const gchar *smilfilename  = gtk_entry_get_text(m_url_text_entry);
	gtk_window_set_title(GTK_WINDOW (m_toplevelcontainer), smilfilename);
	if (openSMILfile(smilfilename, 0, true)) {		
		do_play();
	}
}

void 
gtk_gui::do_player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_player_done");
    _update_menus();
}

void 
gtk_gui::need_redraw (const void* r, void* w, const void* pt) {

	AM_DBG printf("gtk_gui::need_redraw(0x%x)-r=(0x%x)\n",
	(void *)this,r?r:0);
	g_signal_emit(GTK_OBJECT (m_toplevelcontainer), signal_need_redraw_id, 0, r, w, pt);
}

void 
gtk_gui::player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "player_done");
	g_signal_emit(GTK_OBJECT (m_toplevelcontainer), signal_player_done_id, 0);
}

void
no_fileopen_infodisplay(gtk_gui* w, const char* caption) {
	GtkMessageDialog* dialog = 
	(GtkMessageDialog*) gtk_message_dialog_new (NULL,
         GTK_DIALOG_DESTROY_WITH_PARENT,
         GTK_MESSAGE_INFO,
         GTK_BUTTONS_OK,
	 caption);
	gtk_message_dialog_format_secondary_markup (dialog, "No file open: Please first select File->Open");
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (GTK_WIDGET (dialog));
}

void 
gtk_gui::do_play() {
	AM_DBG printf("%s-%s m_mainloop=0x%x\n", m_programfilename, "do_play", m_mainloop);
 	assert(m_mainloop);
 	m_mainloop->play();
 	_update_menus();
}

void 
gtk_gui::do_pause() {
	AM_DBG printf("%s-%s m_mainloop=0x%x\n", m_programfilename, "do_pause", m_mainloop);
 	assert(m_mainloop);
 	m_mainloop->pause();
 	_update_menus();
}

void 
gtk_gui::do_reload() {
	AM_DBG printf("%s-%s m_mainloop=0x%x\n", m_programfilename, "do_reload", m_mainloop);
     	assert(m_mainloop);
     	m_mainloop->restart(true);
     	_update_menus();
}

void 
gtk_gui::do_stop() {
	AM_DBG printf("%s-%s m_mainloop=0x%x\n", m_programfilename, "do_stop", m_mainloop);
 	assert(m_mainloop);
	if(m_mainloop)
		m_mainloop->stop();
 	_update_menus();
}

void 
gtk_gui::do_settings_select() {

	m_settings = new gtk_settings();
	gint result = gtk_dialog_run (GTK_DIALOG (m_settings->getWidget()));
  	switch (result){
      	case GTK_RESPONSE_ACCEPT:
		m_settings->settings_ok();
	       	break;
      	default:
         	break;
    	}
	gtk_widget_destroy (GTK_WIDGET (m_settings->getWidget()));
}

void
gtk_gui::do_quit() {
	AM_DBG printf("%s-%s\n", m_programfilename, "do_quit");
	if (m_mainloop)	{
		m_mainloop->stop();
		delete m_mainloop;
		m_mainloop = NULL;
	}
	g_main_loop_quit (main_loop);	
}

void
gtk_gui::do_resize(GdkEventConfigure *event) {
//	gtk_window_set_default_size(m_toplevelcontainer, event->width, menubar->allocation.height + event->height);
}


#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
void
gtk_gui::unsetCursor() { //XXXX Hack
//	AM_DBG printf("%s-%s\n", m_programfilename, ":unsetCursor");
/**
	Qt::CursorShape cursor_shape = m_mainloop->get_cursor() ?
		Qt::PointingHandCursor : Qt::ArrowCursor;
	if (cursor_shape != m_cursor_shape) {
		m_cursor_shape = cursor_shape;
		setCursor(cursor_shape);
	}
#ifdef	QCURSOR_ON_ZAURUS
	bool pointinghand_cursor = m_mainloop->get_cursor();
	QCursor cursor_shape = pointinghand_cursor ?
		pointingHandCursor : arrowCursor;
	if (m_pointinghand_cursor != pointinghand_cursor) {
		m_pointinghand_cursor = pointinghand_cursor;
		setCursor(cursor_shape);
	}
#endif**//*QCURSOR_ON_ZAURUS*/
//	m_mainloop->set_cursor(0);
}
#endif/*QT_NO_FILEDIALOG*/

void
gtk_gui::do_internal_message(gtk_message_event* e) {
	char* msg = (char*)e->get_message();
	//std::string id("gtk_gui::do_internal_message");
	//std::cerr<<id<<std::endl;
	//std::cerr<<id+" type: "<<e->get_type()<<" msg:"<<msg<<std::endl;
	int level = e->get_type() - gtk_logger::CUSTOM_OFFSET;
	GtkMessageDialog* dialog; // just in case is needed
	switch (level) {
	case gtk_logger::CUSTOM_NEW_DOCUMENT:
		if (m_mainloop) {
			bool start = msg[0] == 'S' ? true : false;
			bool old = msg[2] == 'O' ? true : false;
			m_mainloop->player_start(&msg[4], start, old);
		}
		break;
	case gtk_logger::CUSTOM_LOGMESSAGE:
#ifdef	LOCK_MESSAGE
		if (pthread_mutex_lock(&m_lock_message) != 0){
			printf("pthread_mutex_lock(&m_lock_message) sets errno to %d.\n", errno);
			abort();
		}
#endif /*LOCK_MESSAGE*/
		/* mutex lcoking is required for calling gtk_text_buffer_insert_at_cursor()
		 * Otherwiser GTK+ may crash with a cd $Ecryptic message suck as:
		 * Gtk-WARNING **: Invalid text buffer iterator: either the iterator is
		 * uninitialized, or the characters/pixbufs/widgets in the buffer have been
		 * modified since the iterator was created.
		 */
		gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER (gtk_logger::get_gtk_logger()->get_logger_buffer()), msg, strlen(msg));
#ifdef	LOCK_MESSAGE
		pthread_mutex_unlock(&m_lock_message);
#endif /*LOCK_MESSAGE*/
		break;
	case ambulant::lib::logger::LEVEL_FATAL:
		dialog = (GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_ERROR,
         	GTK_BUTTONS_OK,
	 	msg);
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	case ambulant::lib::logger::LEVEL_ERROR:
		dialog = (GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_WARNING,
         	GTK_BUTTONS_OK,
	 	msg);
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	case ambulant::lib::logger::LEVEL_WARN:
	default:
		dialog = (GtkMessageDialog*) gtk_message_dialog_new (NULL,
         	GTK_DIALOG_DESTROY_WITH_PARENT,
         	GTK_MESSAGE_INFO,
         	GTK_BUTTONS_OK,
	 	msg);
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (GTK_WIDGET (dialog));
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
}

void
gtk_gui::internal_message(int level, char* msg) {
	
	int msg_id = level+gtk_logger::CUSTOM_OFFSET;
	gtk_message_event* event = new gtk_message_event(msg_id, msg);
	g_signal_emit(GTK_OBJECT (m_toplevelcontainer), signal_internal_message_id, 0, event);

#ifdef	LOCK_MESSAGE
	if (level >= ambulant::lib::logger::LEVEL_WARN
	    && pthread_self() != m_gui_thread) {
	  // wait until the message as been OK'd by the user
		pthread_mutex_lock(&m_lock_message);
		pthread_cond_wait(&m_cond_message, &m_lock_message);
		pthread_mutex_unlock(&m_lock_message);
	}
#endif /*LOCK_MESSAGE*/
}

void
gtk_gui::_update_menus()
{
  AM_DBG if (m_mainloop) printf("gtk_gui::_update_menus(0x%x) play_enabled=%d play_active=%d pause_enabled=%d pause_active=%d stop_enabled=%d stop_active=%d \n", this, m_mainloop->is_play_enabled(), m_mainloop->is_play_active(), m_mainloop->is_pause_enabled(), m_mainloop->is_pause_active(), m_mainloop->is_stop_enabled(), m_mainloop->is_stop_active());
    gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "play"),
        m_mainloop && m_mainloop->is_play_enabled() && ! m_mainloop->is_play_active());
    gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "pause"),
        m_mainloop && m_mainloop->is_pause_enabled() && ! m_mainloop->is_pause_active());
    gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "stop"),
        m_mainloop && m_mainloop->is_stop_enabled() && ! m_mainloop->is_stop_active());
    gtk_action_set_sensitive(gtk_action_group_get_action (m_actions, "reload"),
        (m_mainloop != NULL));
}


#ifdef	WITH_NOKIA770
#include <libosso.h>

//KB XXXX FIXME #include "libAmbulantPlayer.h"

gint
dbus_callback (const gchar *interface, const gchar *method,
               GArray *arguments, gpointer data,
               osso_rpc_t *retval)
{
  printf ("AmbulantPlayer dbus: %s, %s\n", interface, method);

  if (!strcmp (method, "top_application"))
      gtk_window_present (GTK_WINDOW (data));

  retval->type = DBUS_TYPE_INVALID;
  return OSSO_OK;
}
#endif/*WITH_NOKIA770*/


int
main (int argc, char*argv[]) {
#ifdef	WITH_NOKIA770
	osso_context_t *ctxt;
	osso_return_t ret;
	GtkWindow *window;
	HildonProgram* hildon_program;
	HildonWindow* hildon_window;
#endif/*WITH_NOKIA770*/
	
	printf ("AmbulantPlayer: starting up\n");
	
#ifdef	WITH_NOKIA770
	ctxt = osso_initialize ("ambulantplayer_app", PACKAGE_VERSION, TRUE, NULL);
	if (ctxt == NULL){
		fprintf (stderr, "osso_initialize failed.\n");
		exit (1);
	}
	if (chdir(AMBULANT_DATADIR) < 0) {
		fprintf(stderr, "AmbulantPlayer: cannot chdir(%s) errno=%d\n", AMBULANT_DATADIR, errno);
		return -1;
	}
#endif/*WITH_NOKIA770*/

#ifdef	WITH_GSTREAMER
	/* initialize GStreamer */
	AM_DBG fprintf(stderr, "initialize GStreamer\n");
	gstreamer_player_initialize (&argc, &argv);
#endif/*WITH_GSTREAMER*/

//	g_thread_init(NULL);
//	gdk_threads_init ();
	
	gtk_init(&argc,&argv);

//#undef	ENABLE_NLS
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
	FILE* DBG = stdout;
	
	/* Setup widget */
	gtk_gui *mywidget = new gtk_gui(argv[0], argc > 1 ? argv[1] : "AmbulantPlayer");

#ifdef	WITH_HILDON
	/* Hildonize AmbulantPlayer */
	hildon_program = HILDON_PROGRAM(hildon_program_get_instance());
	g_set_application_name("AmbulantPlayer");
	hildon_window = HILDON_WINDOW(hildon_window_new());
	hildon_program_add_window(hildon_program, hildon_window);
	/*XXX
	gtk_container_add (GTK_CONTAINER(hildon_window),
			   GTK_WIDGET(gtk_label_new("HildonAmbulantPlayer")));
	gtk_widget_show_all(GTK_WIDGET(hildon_window));
	g_signal_connect(G_OBJECT(hildon_window), "delete_event", 
			 G_CALLBACK(g_main_loop_quit), &mywidget->main_loop);
	*/
#endif/*WITH_HILDON*/
	
	// take log level from preferences	
	gtk_logger::set_gtk_logger_gui(mywidget);
	gtk_logger* gtk_logger = gtk_logger::get_gtk_logger();
	lib::logger::get_logger()->debug("Ambulant Player: now logging to a window");
	// Print welcome banner
	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Unix/GTK"), __DATE__);
#if ENABLE_NLS
	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif
	AM_DBG fprintf(DBG, "argc=%d argv[0]=%s\n", argc, argv[0]);
	AM_DBG for (int i=1;i<argc;i++){fprintf(DBG,"%s\n", argv[i]);}
	
	bool exec_flag = false;

	if (argc > 1) {
		char last[6];
		char* str = argv[argc-1];
		int len = strlen(str);
		strcpy(last, &str[len-5]);
		AM_DBG fprintf(DBG, "%s %s %x\n", str, last);
		if (strcmp(last, ".smil") == 0
		|| strcmp(&last[1], ".smi") == 0
	  	|| strcmp(&last[1], ".sml") == 0) {
 			if (mywidget->openSMILfile(argv[argc-1],
						   0, true)
			    && (exec_flag = true)){
				mywidget->do_play();
			}
		}
	} else {
		preferences* prefs = preferences::get_preferences();
		if ( ! prefs->m_welcome_seen) {
			const char *welcome_doc = find_datafile(welcome_locations);
			if (welcome_doc
			&& mywidget->openSMILfile(welcome_doc,
						  0, true)) {
				mywidget->do_play();
				prefs->m_welcome_seen = true;
			}
		}
		exec_flag = true;
	}	
	g_timeout_add(100, (GSourceFunc) gtk_C_callback_timer, NULL);
	g_main_loop_run(mywidget->main_loop);

	unix_prefs.save_preferences();
	delete gtk_logger::get_gtk_logger();
	mywidget->do_quit();
	delete mywidget;
	gdk_threads_leave ();
#ifdef	WITH_GSTREAMER
	/* finalize GStreamer */
	AM_DBG fprintf(stderr, "finalize GStreamer\n");
	gstreamer_player_finalize();
#endif/*WITH_GSTREAMER*/
	
#ifdef	WITH_NOKIA770
	if (ctxt) osso_deinitialize (ctxt);
#endif/*WITH_NOKIA770*/
	std::cout << "Exiting program" << std::endl;
	return exec_flag ? 0 : -1;
}
