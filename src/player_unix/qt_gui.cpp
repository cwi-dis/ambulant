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

/* qt_gui.cpp - Qt GUI for Ambulant
 *
 */

#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "qt_gui.h"
#include "qt_mainloop.h"
#include "qt_logger.h"
#include "qt_renderer.h"
#include <qthread.h>
#include <X11/Xlib.h>

#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_factory.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define WITH_QT_LOGGER

#ifndef AMBULANT_DATADIR
#define AMBULANT_DATADIR "/usr/local/share/ambulant"
#endif

const QString about_text1 =
	"Ambulant SMIL 3.0 player.\n"
	"Version: ";
const QString about_text2 =
	"\nCopyright Stichting CWI, 2003-2012.\n\n"
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

qt_gui::qt_gui(const char* title, const char* initfile)
:
#ifdef	WITH_QT_HTML_WIDGET
	KMainWindow(0L, title),
#else /*WITH_QT_HTML_WIDGET*/
	QWidget(),
#endif/*WITH_QT_HTML_WIDGET*/
	m_cursor_shape(Qt::ArrowCursor),
	m_mainloop(NULL),
#ifdef	TRY_LOCKING
	m_gui_thread(0),
#endif/*TRY_LOCKING*/
	m_smilfilename(NULL)
{

	m_programfilename = title;
#ifdef	TRY_LOCKING
	pthread_cond_init(&m_cond_message, NULL);
	pthread_mutex_init(&m_lock_message, NULL);
	m_gui_thread = pthread_self();
#endif/*TRY_LOCKING*/
	// If the URL starts with "ambulant:" this is the trick-uri-scheme to
	// open URLs in Ambulant from the browser. Remove the trick.
	if (strncmp(initfile, "ambulant:", 9) == 0)
		initfile += 9;
	if (initfile != NULL && initfile != "")
		m_smilfilename = QString(initfile);
	setCaption(initfile);

	/* Menu bar */
	m_menubar = new QMenuBar(this,"MainMenu");
	{
		int id;
		/* File */
		m_filemenu = new QPopupMenu (this);
		assert(m_filemenu);
		int open_id = m_filemenu->insertItem(gettext("&Open..."), this, SLOT(slot_open()));
		m_filemenu->setAccel(CTRL+Key_O, open_id);
		int url_id = m_filemenu->insertItem(gettext("Open &URL..."), this, SLOT(slot_open_url()));
		m_filemenu->setAccel(CTRL+Key_L, url_id);
		m_reload_id = m_filemenu->insertItem(gettext("&Reload..."), this, SLOT(slot_reload()));
		m_filemenu->insertSeparator();

		m_filemenu->insertItem(gettext("&Preferences..."), this, SLOT(slot_settings_select()));
		m_filemenu->insertItem(gettext("&Document Settings..."), this, SLOT(slot_load_settings()));
		m_filemenu->insertSeparator();

		int quit_id = m_filemenu->insertItem(gettext("&Quit"), this, SLOT(slot_quit()));
		m_filemenu->setAccel(CTRL+Key_Q, quit_id);
		m_menubar->insertItem(gettext("&File"), m_filemenu);

		/* Play */
		m_playmenu = new QPopupMenu (this, "PlayA");
		assert(m_playmenu);
		m_play_id = m_playmenu->insertItem(gettext("Pla&y"), this, SLOT(slot_play()));
		m_playmenu->setAccel(CTRL+Key_P, m_play_id);
		m_playmenu->setItemEnabled(m_play_id, false);
		m_pause_id = m_playmenu->insertItem(gettext("&Pause"), this, SLOT(slot_pause()));
		m_playmenu->setAccel(CTRL+Key_Space, m_pause_id);
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_stop_id = m_playmenu->insertItem(gettext("&Stop"), this, SLOT(slot_stop()));
		m_playmenu->setAccel(CTRL+Key_S, m_stop_id);
		m_menubar->insertItem(gettext("Pla&y"), m_playmenu);

		/* View */
		m_viewmenu = new QPopupMenu(this, "View");
		int fullscreen_id = m_viewmenu->insertItem(gettext("&Full Screen"), this, SLOT(showFullScreen()));
		m_viewmenu->setAccel(CTRL+Key_F, fullscreen_id);
		int window_id = m_viewmenu->insertItem(gettext("&Window"), this, SLOT(showNormal()));
		m_viewmenu->setAccel(CTRL+SHIFT+Key_F, window_id);
		m_viewmenu->insertSeparator();
#ifdef	WITH_QT_LOGGER
		m_viewmenu->insertItem(gettext("&Log Window..."), this, SLOT(slot_logger_window()));
#endif/*WITH_QT_LOGGER*/
		m_menubar->insertItem(gettext("&View"), m_viewmenu);

		/* Help */
		m_helpmenu = new QPopupMenu (this, "HelpA");
		assert(m_helpmenu);
		m_helpmenu->insertItem(gettext("&About AmbulantPlayer"), this, SLOT(slot_about()));
		int help_id = m_helpmenu->insertItem(gettext("AmbulantPlayer &Help..."), this, SLOT(slot_help()));
		m_helpmenu->setAccel(CTRL+Key_Question, help_id);
		m_helpmenu->insertSeparator();
		m_helpmenu->insertItem(gettext("AmbulantPlayer &Website..."), this, SLOT(slot_homepage()));
		m_helpmenu->insertItem(gettext("&Play Welcome Document"), this, SLOT(slot_welcome()));
		m_menubar->insertItem(gettext("&Help"), m_helpmenu);
		m_menubar->setGeometry(0,0,320,20);
		m_menubar_height = 27;
	}
	QObject::connect(this, SIGNAL(signal_player_done()), this, SLOT(slot_player_done()));
	_update_menus();
}

qt_gui::~qt_gui() {
#define DELETE(X) if (X) { delete X; X = NULL; }
	setCaption(QString::null);
	DELETE(m_mainloop)
	DELETE(m_filemenu)
	DELETE(m_helpmenu)
	DELETE(m_playmenu)
	DELETE(m_viewmenu)
	DELETE(m_menubar)
	m_smilfilename = (char *)NULL;
}

void
qt_gui::slot_about() {
	int but = QMessageBox::information(this, gettext("About AmbulantPlayer"),
		about_text1+get_version()+about_text2,
		gettext("OK"));
}

void
qt_gui::slot_homepage() {
	open_web_browser("http://www.ambulantplayer.org");
}

void
qt_gui::slot_welcome() {
	const char *welcome_doc = find_datafile(welcome_locations);

	if (welcome_doc) {
		if( openSMILfile(welcome_doc, IO_ReadOnly)) {
			slot_play();
		}
	} else {
		QMessageBox::information(this, m_programfilename, gettext("Cannot find Welcome.smil document"));
	}
}

void
qt_gui::slot_help() {
	const char *help_doc = find_datafile(helpfile_locations);

	if (help_doc) {
		open_web_browser(help_doc);
	} else {
		QMessageBox::information(this, m_programfilename, gettext("Cannot find Ambulant Player Help"));
	}
}

void
qt_gui::slot_logger_window() {
	QTextEdit* logger_window = qt_logger::get_qt_logger()->get_logger_window();
	if (logger_window->isHidden())
		logger_window->show();
	else
		logger_window->hide();
}

bool
checkFilename(QString filename, int mode) {
	QFile* file = new QFile(filename);
	return file->open(mode);
}

void
qt_gui::fileError(QString smilfilename) {
	char buf[1024];
	sprintf(buf, gettext("%s: Cannot open file: %s"), (const char*) smilfilename, strerror(errno));
	QMessageBox::information(this, m_programfilename, buf);
}

bool
qt_gui::openSMILfile(QString smilfilename, int mode) {
	if (smilfilename.isNull())
		return false;
#if 0
	if (! checkFilename(smilfilename, mode)) {
		fileError(smilfilename);
		return false;
	}
#endif
	char* filename = strdup(smilfilename);
	setCaption(basename(filename));
	free(filename);
	m_playmenu->setItemEnabled(m_pause_id, false);
	m_playmenu->setItemEnabled(m_play_id, true);
	m_smilfilename = smilfilename;
	if (m_mainloop != NULL)
		delete m_mainloop;
	m_mainloop = new qt_mainloop(this, m_menubar_height);
	return m_mainloop->is_open();
}

void
qt_gui::slot_open() {
	QString smilfilename = QFileDialog::getOpenFileName(
		".", // Initial dir
		gettext("SMIL files (*.smil *.smi);; All files (*.smil *.smi *.grins);; Any file (*)"), // file types
		this,
		gettext("open file dialog"),
		gettext("Double Click a file to open")
		);
	if (openSMILfile(smilfilename, IO_ReadOnly))
		slot_play();
}


void
qt_gui::setDocument(const QString& smilfilename) {
}

void
qt_gui::slot_file_selected(const DocLnk& selected_file) {
}

void
qt_gui::slot_close_fileselector()
{
}

void
qt_gui::slot_settings_selected(const DocLnk& selected_file) {
}

void
qt_gui::slot_close_settings_selector()
{
}

void
qt_gui::slot_load_settings() {
	if (m_mainloop && m_mainloop->is_open())
		slot_stop();
	QString settings_filename = QFileDialog::getOpenFileName(
		".", // Initial dir
		gettext("Settings files (*.xml)"), // file types
		this,
		gettext("open settings file dialog"),
		gettext("Double Click a settings file to open")
		);
	if ( ! settings_filename.isNull()) {
		smil2::test_attrs::load_test_attrs(settings_filename.ascii());
		if (openSMILfile(m_smilfilename, IO_ReadOnly))
			slot_play();
	}
}

void
qt_gui::slot_open_url() {
	bool ok;
	QString smilfilename = QInputDialog::getText(
		"AmbulantPlayer",
		gettext("URL to open:"),
		QLineEdit::Normal,
		QString::null,
		&ok,
		this
		);
	if (ok && !smilfilename.isEmpty() && openSMILfile(smilfilename, IO_ReadOnly)) {
		slot_play();
	}
}

void
qt_gui::slot_player_done() {
}

void
qt_gui::need_redraw (const void* r, void* w, const void* pt) {
	emit signal_need_redraw(r,w,pt);
}

void
qt_gui::player_done() {
	emit signal_player_done();
}

void
no_fileopen_infodisplay(QWidget* w, const char* caption) {
	QMessageBox::information(w,caption,gettext("No file open: Please first select File->Open"));
}

void
qt_gui::slot_play() {
	assert(m_mainloop);
	m_mainloop->play();
	_update_menus();
}

void
qt_gui::slot_pause() {
	assert(m_mainloop);
	m_mainloop->pause();
	_update_menus();
}

void
qt_gui::slot_reload() {
	assert(m_mainloop);
	m_mainloop->restart(true);
	_update_menus();
}

void
qt_gui::slot_stop() {
	if(m_mainloop)
		m_mainloop->stop();
	_update_menus();
}

void
qt_gui::slot_settings_select() {
	m_settings = new qt_settings();
	QWidget* settings_widget = m_settings->settings_select();
	m_finish_hb = new QHBox(settings_widget);
	m_ok_pb = new QPushButton(gettext("OK"), m_finish_hb);
	m_finish_hb->setSpacing(50);
	QPushButton* m_cancel_pb= new QPushButton(gettext("Cancel"), m_finish_hb);
	QObject::connect(m_ok_pb, SIGNAL(released()), this, SLOT(slot_settings_ok()));
	QObject::connect(m_cancel_pb, SIGNAL(released()), this, SLOT(slot_settings_cancel()));
	settings_widget->show();
}

void
qt_gui::slot_settings_ok() {
	m_settings->settings_ok();
	slot_settings_cancel();
}

void
qt_gui::slot_settings_cancel() {
	m_settings->settings_finish();
	delete m_settings;
	m_settings = NULL;
}

void
qt_gui::slot_quit() {
	if (m_mainloop) {
		m_mainloop->stop();
		delete m_mainloop;
		m_mainloop = NULL;
	}
	qApp->quit();
}

void
qt_gui::customEvent(QCustomEvent* e) {
	char* msg = (char*)e->data();
	int level = e->type() - qt_logger::CUSTOM_OFFSET;
	switch (level) {
	case qt_logger::CUSTOM_NEW_DOCUMENT:
		if (m_mainloop) {
			bool start = msg[0] == 'S' ? true : false;
			bool old = msg[2] == 'O' ? true : false;
			m_mainloop->player_start(&msg[4], start, old);
		}
		break;
	case qt_logger::CUSTOM_LOGMESSAGE:
		qt_logger::get_qt_logger()->get_logger_window()->append(msg);
		break;
	case ambulant::lib::logger::LEVEL_FATAL:
		QMessageBox::critical(NULL, "AmbulantPlayer", msg);
		break;
	case ambulant::lib::logger::LEVEL_ERROR:
		QMessageBox::warning(NULL, "AmbulantPlayer", msg);
		break;
	case ambulant::lib::logger::LEVEL_WARN:
	default:
		QMessageBox::information(NULL, "AmbulantPlayer", msg);
		break;
	}
#ifdef	TRY_LOCKING
	if (level >= ambulant::lib::logger::LEVEL_WARN) {
		pthread_mutex_lock(&m_lock_message);
		pthread_cond_signal(&m_cond_message);
		pthread_mutex_unlock(&m_lock_message);
	}
#endif/*TRY_LOCKING*/
}

void
qt_gui::internal_message(int level, const char* msg) {
	int msg_id = level+qt_logger::CUSTOM_OFFSET;
	qt_message_event* qme = new qt_message_event(msg_id, msg);
#ifdef	QT_THREAD_SUPPORT
	QThread::postEvent(this, qme);
#else /*QT_THREAD_SUPPORT*/
	QApplication::postEvent(this, qme);
#endif/*QT_THREAD_SUPPORT*/
#ifdef	TRY_LOCKING
	if (level >= ambulant::lib::logger::LEVEL_WARN && pthread_self() != m_gui_thread) {
		// wait until the message as been OK'd by the user
		pthread_mutex_lock(&m_lock_message);
		pthread_cond_wait(&m_cond_message, &m_lock_message);
		pthread_mutex_unlock(&m_lock_message);

	}
#endif/*TRY_LOCKING*/
}

void
qt_gui::_update_menus()
{
	m_playmenu->setItemEnabled(m_play_id, m_mainloop && m_mainloop->is_play_enabled());
	m_playmenu->setItemChecked(m_play_id, m_mainloop && m_mainloop->is_play_active());
	m_playmenu->setItemEnabled(m_pause_id, m_mainloop && m_mainloop->is_pause_enabled());
	m_playmenu->setItemChecked(m_pause_id, m_mainloop && m_mainloop->is_pause_active());
	m_playmenu->setItemEnabled(m_stop_id, m_mainloop && m_mainloop->is_stop_enabled());
	m_playmenu->setItemChecked(m_stop_id, m_mainloop && m_mainloop->is_stop_active());
	m_filemenu->setItemEnabled(m_reload_id, (m_mainloop != NULL));
}

int
main (int argc, char*argv[]) {

	FILE* DBG = stdout;

	AM_DBG fprintf(DBG, "argc=%d argv[0]=%s\n", argc, argv[0]);
	AM_DBG for (int i=1;i<argc;i++) {
		fprintf(DBG,"%s\n", argv[i]);
	}
	// Initializing Xlib for threading early helps to get rid of assertions such as:
	// xcb_lock.c:77: _XGetXCBBuffer: Assertion `((int) ((xcb_req) - (dpy->request)) >= 0)' failed
	// ref: http://www.mail-archive.com/debian-bugs-dist@lists.debian.org/msg557273.html
	if (XInitThreads() == 0) {
		fprintf(DBG, "XInitThreads() returned zero");
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
#ifdef	WITH_QT_HTML_WIDGET
	KApplication myapp( argc, argv, "AmbulantPlayer" );
#else /*WITH_QT_HTML_WIDGET*/
	/* From the documentation of QApplication:	Note that argc and argv might be changed. */
	QApplication myapp(argc, argv);
#endif/*WITH_QT_HTML_WIDGET*/

	/* Setup widget */
	qt_gui* mywidget = new qt_gui(argv[0], argc > 1 ? argv[1] : "AmbulantPlayer");
	mywidget->setGeometry(240, 240, 200, 32);
	QCursor qcursor(Qt::ArrowCursor);
	mywidget->setCursor(qcursor);
	myapp.setMainWidget(mywidget);
	mywidget->show();
/*TMP initialize logger after gui*/
	// take log level from preferences
	qt_logger::set_qt_logger_gui(mywidget);
	qt_logger* qt_logger = qt_logger::get_qt_logger();
	lib::logger::get_logger()->debug("Ambulant Player: now logging to a window");
	// Print welcome banner
	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Unix/Qt"), __DATE__);
#if ENABLE_NLS
	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif

	bool exec_flag = true;

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
			if (mywidget->openSMILfile(str, IO_ReadOnly) && exec_flag)
				mywidget->slot_play();
		}
	} else {
		preferences* prefs = preferences::get_preferences();
		if ( ! prefs->m_welcome_seen) {
			const char *welcome_doc = find_datafile(welcome_locations);
			if (welcome_doc && mywidget->openSMILfile(welcome_doc, IO_ReadOnly)) {
				mywidget->slot_play();
				prefs->m_welcome_seen = true;
			}
		}
		exec_flag = true;
	}
	if (exec_flag) {
		myapp.exec();
	} else if (argc > 1) {
		std::string error_message = gettext("Cannot open: ");
		error_message = error_message + "\"" + argv[1] + "\"";
		std::cerr << error_message << std::endl;
		myapp.exec();
	}
#ifndef WITH_QT_HTML_WIDGET
	delete mywidget;
#endif/*WITH_QT_HTML_WIDGET*/
	unix_prefs.save_preferences();
	delete qt_logger::get_qt_logger();
	return exec_flag ? 0 : -1;
}
