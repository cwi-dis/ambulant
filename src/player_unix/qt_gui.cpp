/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* qt_gui.cpp - Qt GUI for Ambulant
 *              
 *              Initial version renders images & text
 *
 * Kees Blom, Oct.29 2003
 */

#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>
#include <fcntl.h>
#include "qt_gui.h"
#include "qt_mainloop.h"
#include "qt_logger.h"
#include "qt_renderer.h"
#include "qthread.h"
#if 1
#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#endif

// #define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define	WITH_QT_LOGGER

const QString about_text = 
	"Ambulant SMIL 2.0 player.\n"
	"Copyright Stichting CWI, 2004.\n\n"
	"License: modified GPL.";


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
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	"/home/zaurus/Documents/Welcome/Welcome.smil",
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

static const char * 
find_datafile(const char **locations)
{
	const char **p;
	for(p = locations; *p; p++) {
		if (access(*p, 0) >= 0) return *p;
	}
	return NULL;
}

qt_gui::qt_gui(const char* title,
	       const char* initfile)
 :
	QWidget(),  
        m_busy(true),
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	m_cursor_shape(Qt::ArrowCursor),
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	//m_cursor_shape(arrowCursor);
	m_fileselector(NULL),
#endif/*QT_NO_FILEDIALOG*/
	m_mainloop(NULL),
	m_o_x(0),	 
	m_o_y(0),	 
	m_pause_id(),
	m_pausing(),
	m_play_id(),
	m_playing(),
	m_playmenu(),
	m_programfilename(),
	m_smilfilename(NULL)
{

	m_programfilename = title;
	if (initfile != NULL && initfile != "")
		m_smilfilename = QString(initfile);
#ifdef  QT_NO_FILEDIALOG
	else
		m_smilfilename = QString(
			"/home/zaurus/Documents/example.smil");
#endif/*QT_NO_FILEDIALOG*/
	m_playing = false;
	m_pausing = false;
	setCaption(initfile);

	/* Menu bar */
	m_menubar = new QMenuBar(this,"MainMenu");
	{
		int id;
		/* File */
		QPopupMenu* filemenu = new QPopupMenu (this);
		assert(filemenu);
		int open_id = filemenu->insertItem(gettext("&Open..."), this, 
						   SLOT(slot_open()));
		int url_id = filemenu->insertItem(gettext("Open &URL..."), this, 
						  SLOT(slot_open_url()));
#ifdef QT_NO_FILEDIALOG	/* Assume embedded Qt */
		// Disable unavailable menu entries
		filemenu->setItemEnabled(open_id, true);
		filemenu->setItemEnabled(url_id, false);
#endif/*QT_NO_FILEDIALOG*/
		filemenu->insertSeparator();
		
		filemenu->insertItem(gettext("&Settings"), this,
				     SLOT(slot_settings_select()));
		filemenu->insertSeparator();
		
		filemenu->insertItem(gettext("&Quit"), this, SLOT(slot_quit()));
		m_menubar->insertItem(gettext("&File"), filemenu);
		
		/* Play */
		m_playmenu = new QPopupMenu (this, "PlayA");
		assert(m_playmenu);
		m_play_id = m_playmenu->insertItem(gettext("Pla&y"), this,
						   SLOT(slot_play()));
		m_playmenu->setItemEnabled(m_play_id, false);
		m_pause_id = m_playmenu->insertItem(gettext("&Pause"), this,
						    SLOT(slot_pause()));
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_playmenu->insertItem(gettext("&Stop"),	this, SLOT(slot_stop()));
		m_menubar->insertItem(gettext("Pla&y"), m_playmenu);
		
		/* View */
		QPopupMenu* viewmenu = new QPopupMenu(this, "View");
		viewmenu->insertItem(gettext("&Full Screen"), this,
				     SLOT(showFullScreen()));
		viewmenu->insertItem(gettext("&Window"), this,SLOT(showNormal()));
		viewmenu->insertSeparator();
#ifdef	WITH_QT_LOGGER
		viewmenu->insertItem(gettext("&Log Window..."), this,
				     SLOT(slot_logger_window()));
#endif/*WITH_QT_LOGGER*/
		m_menubar->insertItem(gettext("&View"), viewmenu);
		
		/* Help */
		QPopupMenu* helpmenu = new QPopupMenu (this, "HelpA");
		assert(helpmenu);
		helpmenu->insertItem(gettext("&About AmbulantPlayer"), this,
				     SLOT(slot_about()));
		helpmenu->insertItem(gettext("AmbulantPlayer &Help"), this,
				     SLOT(slot_help()));
		helpmenu->insertSeparator();
		helpmenu->insertItem(gettext("AmbulantPlayer &Website..."), this,
				     SLOT(slot_homepage()));
		helpmenu->insertItem(gettext("&Play Welcome Document"), this,
				     SLOT(slot_welcome()));
		m_menubar->insertItem(gettext("&Help"), helpmenu);
		m_menubar->setGeometry(0,0,320,20);
		m_o_x = 0;
		m_o_y = 27;
	}
	QObject::connect(this, SIGNAL(signal_player_done()),
			    this, SLOT(slot_player_done()));
}

qt_gui::~qt_gui() {
	AM_DBG printf("%s0x%X\n", "qt_gui::~qt_gui(), m_mainloop=",m_mainloop);
	setCaption(QString::null);
	if (m_menubar != NULL) {
		delete m_menubar;
		m_menubar = NULL;
	}
	if (m_mainloop != NULL) {
		delete m_mainloop;
		m_mainloop = NULL;
	}
}

void 
qt_gui::slot_about() {
	int but = QMessageBox::information(this, gettext("About AmbulantPlayer"),
					   about_text,
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
		QMessageBox::information(this, m_programfilename, 
			gettext("Cannot find Welcome.smil document"));
	}
}

void 
qt_gui::slot_help() {
	const char *help_doc = find_datafile(helpfile_locations);
	
	if (help_doc) {
		open_web_browser(help_doc);
	} else {
		QMessageBox::information(this, m_programfilename, 
			gettext("Cannot find Ambulant Player Help"));
	}
}

void
qt_gui::slot_logger_window() {
	AM_DBG printf("slot_logger_window()\n");
#ifndef QT_NO_FILEDIALOG	 /* Assume plain Qt */
	QTextEdit* logger_window =
		qt_logger::get_qt_logger()->get_logger_window();
	if (logger_window->isHidden())
		logger_window->show();
	else
		logger_window->hide();
#endif/*QT_NO_FILEDIALOG*/
}

bool 
checkFilename(QString filename, int mode) {
	QFile* file = new QFile(filename);
	return file->open(mode);
}

void
qt_gui::fileError(QString smilfilename) {
 	char buf[1024];
	sprintf(buf, gettext("%s: Cannot open file: %s"),
		(const char*) smilfilename, strerror(errno));
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
	smilfilename = strdup(smilfilename);
	m_smilfilename = smilfilename;
	if (m_mainloop != NULL)
		delete m_mainloop;
	m_mainloop = new qt_mainloop(this);
	m_playing = false;
	m_pausing = false;
	return m_mainloop->is_open();
}

void 
qt_gui::slot_open() {
#ifndef QT_NO_FILEDIALOG
	QString smilfilename =
		QFileDialog::getOpenFileName(
				 ".", // Initial dir
				 gettext("SMIL files (*.smil *.smi);; All files (*.smil *.smi *.mms *.grins);; Any file (*)"), // file types
				 this,
				 gettext("open file dialog"),
				 gettext("Double Click a file to open")
				 );
	openSMILfile(smilfilename, IO_ReadOnly);
	slot_play();
#else	/*QT_NO_FILEDIALOG*/	
	if (m_fileselector == NULL) {
	  QString mimeTypes("application/smil;");
	  m_fileselector = new FileSelector(mimeTypes, NULL,
					    "slot_open", false);
	  m_fileselector->resize(240, 280);
	  QObject::connect(m_fileselector, 
			   SIGNAL(fileSelected(const DocLnk&)),
			   this, 
			   SLOT(slot_file_selected(const DocLnk&)));
	  QObject::connect(m_fileselector, SIGNAL(closeMe()), 
			   this, SLOT(slot_close_fileselector()));
	} else {
	  m_fileselector->reread();
	}
	m_fileselector->show();
#endif	/*QT_NO_FILEDIALOG*/
}


void
qt_gui::setDocument(const QString& smilfilename) {
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	openSMILfile(smilfilename, IO_ReadOnly);
	slot_play();
#endif/*QT_NO_FILEDIALOG*/
}

void
qt_gui::slot_file_selected(const DocLnk& selected_file) {
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	QString* smilfilepointer = new QString(selected_file.file());
	QString smilfilename = *smilfilepointer;
	delete smilfilepointer;
	m_fileselector->hide();
	openSMILfile(smilfilename, IO_ReadOnly);
	slot_play();
#endif/*QT_NO_FILEDIALOG*/
}
void
qt_gui::slot_close_fileselector()
{
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	m_fileselector->hide();
#endif/*QT_NO_FILEDIALOG*/
}

void 
qt_gui::slot_open_url() {
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
  	bool ok;
	QString smilfilename =
		QInputDialog::getText(
				      "AmbulantPlayer",
				      gettext("URL to open:"),
				      QLineEdit::Normal,
				      QString::null,
				      &ok,
				      this
				 );
	if (ok && !smilfilename.isEmpty()
	    && openSMILfile(smilfilename, IO_ReadOnly)) {
		slot_play();
	}
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	QMessageBox::information (this, m_programfilename,
		gettext("Open URL not implemented for Embedded Qt"));
#endif/*QT_NO_FILEDIALOG*/
}

void 
qt_gui::slot_player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_player_done");
	/*
	if (m_mainloop->player_done()) {
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_playmenu->setItemEnabled(m_play_id, true);
		m_playing = false;
	}
	*/
}

void 
qt_gui::need_redraw (const void* r, void* w, const void* pt) {
	AM_DBG printf("qt_gui::need_redraw(0x%x)-r=(0x%x)\n",
	(void *)this,r?r:0);
	emit signal_need_redraw(r,w,pt);
}

void 
qt_gui::player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "player_done");
	emit signal_player_done();
}

void 
qt_gui::slot_play() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_play");
	if (m_smilfilename == NULL || m_mainloop == NULL
	    || ! m_mainloop->is_open()) {
		QMessageBox::information(
			this, m_programfilename,
			gettext("No file open: Please first select File->Open"));
		return;
	}
	if (!m_playing) {
		m_playing = true;
		m_playmenu->setItemEnabled(m_play_id, false);
		m_playmenu->setItemEnabled(m_pause_id, true);
#if 1
		m_mainloop->play();
#else
		pthread_t playthread;
		int rv = pthread_create(&playthread, NULL,
					&qt_mainloop::run,
					m_mainloop);
#endif
	}
	if (m_pausing) {
		m_pausing = false;
		m_playmenu->setItemEnabled(m_pause_id, true);
		m_playmenu->setItemEnabled(m_play_id, false);
		m_mainloop->set_speed(1);
	}
}

void 
qt_gui::slot_pause() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_pause");
	if (! m_pausing) {
		m_pausing = true;
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_playmenu->setItemEnabled(m_play_id, true);
		m_mainloop->set_speed(0);
	}
}

void 
qt_gui::slot_stop() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_stop");
	if(m_mainloop)
		m_mainloop->stop();
	m_playmenu->setItemEnabled(m_pause_id, false);
	m_playmenu->setItemEnabled(m_play_id, true);
	m_playing = false;
}

void 
qt_gui::slot_settings_select() {
	m_settings = new qt_settings();
	QWidget* settings_widget = m_settings->settings_select();
	m_finish_hb = new QHBox(settings_widget);
	m_ok_pb	= new QPushButton(gettext("OK"), m_finish_hb);
	m_finish_hb->setSpacing(50);
	QPushButton* m_cancel_pb= new QPushButton(gettext("Cancel"), m_finish_hb);
	QObject::connect(m_ok_pb, SIGNAL(released()),
			 this, SLOT(slot_settings_ok()));
	QObject::connect(m_cancel_pb, SIGNAL(released()),
			 this, SLOT(slot_settings_cancel()));
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
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_quit");
	if (m_mainloop)	{
	  m_mainloop->stop();
	  m_mainloop->release();
	  m_mainloop = NULL;
	}
	m_busy = false;
	qApp->quit();
}

#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
void
qt_gui::unsetCursor() { //XXXX Hack
//	AM_DBG printf("%s-%s\n", m_programfilename, ":unsetCursor");
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
#endif/*QCURSOR_ON_ZAURUS*/
	m_mainloop->set_cursor(0);
}
#endif/*QT_NO_FILEDIALOG*/

void
qt_gui::customEvent(QCustomEvent* e) {
	char* msg = (char*)e->data();
//	std::string id("qt_gui::customEvent");
//	std::cerr<<id<<std::endl;
//	std::cerr<<id+" type: "<<e->type()<<" msg:"<<msg<<std::endl;
	switch (e->type()-qt_logger::CUSTOM_OFFSET) {
	case qt_logger::CUSTOM_NEW_DOCUMENT:
		if (m_mainloop) {
			bool start = msg[0] == 'S' ? true : false;
			bool old = msg[2] == 'O' ? true : false;
			m_mainloop->player_start(&msg[4], start, old);
		}
		break;
#ifndef QT_NO_FILEDIALOG	 /* Assume plain Qt */
	case qt_logger::CUSTOM_LOGMESSAGE:
		qt_logger::get_qt_logger()->
			get_logger_window()->append(msg);
		break;
#else /*QT_NO_FILEDIALOG*/
/* No logger window on an embedded system, logging there on "stdout" */
#endif/*QT_NO_FILEDIALOG*/
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
	free(msg);
}

void
qt_gui::internal_message(int level, char* msg) {
	int msg_id = level+qt_logger::CUSTOM_OFFSET;
  	qt_message_event* qme = new qt_message_event(msg_id, msg);
#ifdef	QT_THREAD_SUPPORT
	QThread::postEvent(this, qme);
#else /*QT_THREAD_SUPPORT*/
	QApplication::postEvent(this, qme);
#endif/*QT_THREAD_SUPPORT*/
}

int
main (int argc, char*argv[]) {

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
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	QApplication myapp(argc, argv);
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	QPEApplication myapp(argc, argv);
#endif/*QT_NO_FILEDIALOG*/

	/* Setup widget */
	qt_gui* mywidget = new qt_gui(argv[0], argc > 1 ? argv[1] 
				      : "AmbulantPlayer");
#ifndef QT_NO_FILEDIALOG     /* Assume plain Qt */
	mywidget->setGeometry(240, 320, 320, 240);
	QCursor qcursor(Qt::ArrowCursor);
	mywidget->setCursor(qcursor);
	myapp.setMainWidget(mywidget);
#else /*QT_NO_FILEDIALOG*/   /* Assume embedded Qt */
	if (argc > 1 && strcmp(argv[1], "-qcop") != 0)
	  myapp.showMainWidget(mywidget);
	else
	  myapp.showMainDocumentWidget(mywidget);
#endif/*QT_NO_FILEDIALOG*/
	mywidget->show();
/*TMP initialize logger after gui*/	
	// take log level from preferences
	qt_logger::set_qt_logger_gui(mywidget);
	qt_logger* qt_logger = qt_logger::get_qt_logger();
	lib::logger::get_logger()->debug("Ambulant Player: %s",
					 "now logging to a window");
	// Print welcome banner
	lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Unix/Qt"), __DATE__);
#if ENABLE_NLS
	lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english)"));
#endif

	AM_DBG fprintf(DBG, "argc=%d argv[0]=%s\n", argc, argv[0]);
	AM_DBG for (int i=1;i<argc;i++){fprintf(DBG,"%s\n", argv[i]);
	}

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
						   IO_ReadOnly)
			    && (exec_flag = true))
				mywidget->slot_play();
		}
	} else {
		preferences* prefs = preferences::get_preferences();
		if ( ! prefs->m_welcome_seen) {
			const char *welcome_doc = find_datafile(welcome_locations);
			if (welcome_doc
			&& mywidget->openSMILfile(welcome_doc,
						  IO_ReadOnly)) {
				mywidget->slot_play();
				prefs->m_welcome_seen = true;
			}
		}
		exec_flag = true;
	}
	if (exec_flag)
		myapp.exec();
	else if (argc > 1)
		std::cerr << "Cannot open \"" << (const char*)argv[1]
			  << "\""<<std::endl;
	delete mywidget;
	unix_prefs.save_preferences();
	std::cout << "Exiting program" << std::endl;
	return exec_flag ? 0 : -1;
}
