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
#include "qt_renderer.h"
#include <qinputdialog.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

const QString about_text = 
	"Ambulant SMIL 2.0 player.\n"
	"Copyright Stichting CWI, 2004.\n\n"
	"License: modified GPL.";


// Find welcome document.
// XXX This code is incorrect, really.
char *welcome_locations[] = {
	"Welcome/Welcome.smil",
	"../Welcome/Welcome.smil",
	"Extras/Welcome/Welcome.smil",
	"../Extras/Welcome/Welcome.smil",
	"/usr/local/lib/ambulant/Welcome/Welcome.smil",
	NULL
};

static char * 
find_welcome_doc()
{
	char **p;
	for(p = welcome_locations; *p; p++) {
		if (access(*p, 0) >= 0) return *p;
	}
	return NULL;
}

qt_gui::qt_gui(const char* title,
	       const char* initfile) :
        m_busy(true),
	m_o_x(0),	 
	m_o_y(0),	 
	m_mainloop(NULL),
	m_pause_id(),
	m_pausing(),
	m_play_id(),
	m_playing(),
	m_playmenu(),
	m_programfilename(),
	m_smilfilename()
{

	m_programfilename = title;
	if (initfile != NULL && initfile != "")
		m_smilfilename = initfile;
#ifdef  QT_NO_FILEDIALOG
	else
		m_smilfilename = QString(
			"/home/zaurus/Documents/example.smil");
#endif/*QT_NO_FILEDIALOG*/
	m_playing = false;
	m_pausing = false;
	setCaption(initfile);

	/* Menu bar */
	QMenuBar* menubar = new QMenuBar(this,"MainMenu");
	{
		int id;
		/* File */
		QPopupMenu* filemenu = new QPopupMenu (this);
		assert(filemenu);
		filemenu->insertItem("&Open", this, SLOT(slot_open()));
		filemenu->insertItem("Open &URL", this, SLOT(slot_open_url()));
		filemenu->insertItem("&Full Screen", this,
				     SLOT(showFullScreen()));
		filemenu->insertItem("&Normal", this,SLOT(showNormal()));
//		filemenu->insertItem("&Quit", qApp, SLOT(quit()));
		filemenu->insertItem("&Quit", this, SLOT(slot_quit()));
		menubar->insertItem("&File", filemenu);
		
		/* Play */
		m_playmenu = new QPopupMenu (this, "PlayA");
		assert(m_playmenu);
		m_play_id = m_playmenu->insertItem("Pla&y", this,
						   SLOT(slot_play()));
		m_playmenu->setItemEnabled(m_play_id, false);
		m_pause_id = m_playmenu->insertItem("&Pause", this,
						    SLOT(slot_pause()));
		m_playmenu->setItemEnabled(m_pause_id, false);
		m_playmenu->insertItem("&Stop",	this, SLOT(slot_stop()));
		menubar->insertItem("Pla&y", m_playmenu);
		
		/* Help */
		QPopupMenu* helpmenu = new QPopupMenu (this, "HelpA");
		assert(helpmenu);
		helpmenu->insertItem("&About AmbulantPlayer", this, SLOT(slot_about()));
		menubar->insertItem("&Help", helpmenu);
		menubar->setGeometry(0,0,320,20);
		m_o_x = 0;
		m_o_y = 27;
	}
}

qt_gui::~qt_gui() {
	setCaption(QString::null);
}

void 
qt_gui::slot_about() {
	int but = QMessageBox::information(this, "About AmbulantPlayer", about_text,
		"Homepage...",
		"Welcome doc",
		"OK",
		2);
	if (but == 0) {
		// Show homepage
		open_web_browser("http://www.ambulantplayer.org");
	} else if (but == 1) {
		// Play welcome document
		char *welcome_doc = find_welcome_doc();
		if (welcome_doc) {
			openSMILfile(welcome_doc, IO_ReadOnly);
			slot_play();
		}
	} else if (but == 2) {
		// Do nothing
	}
}

bool 
checkFilename(QString filename, int mode) {
	QFile* file = new QFile(filename);
	return file->open(mode);
}

void
qt_gui::fileError(QString smilfilename) {
 	char buf[1024];
	sprintf(buf, "Cannot open file \"%s\":\n%s\n",
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
#if 0
	if ( ! (*smilfilename == '/')) {
 	         char* workdir = getcwd(NULL, 0);
		 int workdirlen = strlen(workdir);
		 int pathnamelen = workdirlen + strlen(smilfilename) + 2;
		 char* pathname = (char*) malloc(pathnamelen);
		 strcpy(pathname, workdir);
		 strcpy(&pathname[workdirlen], "/");
		 strcpy(&pathname[workdirlen+1], smilfilename);
		 smilfilename = pathname;
		 free(workdir);
	} else
 #endif
		smilfilename = strdup(smilfilename);
	        
	m_smilfilename = smilfilename;
	if (m_mainloop != NULL)
		delete m_mainloop;
	m_mainloop = new qt_mainloop(this);
	m_playing = false;
	m_pausing = false;
	return true;
}

void 
qt_gui::slot_open() {
	QString smilfilename =
#ifndef QT_NO_FILEDIALOG
		QFileDialog::getOpenFileName(
				 ".", // Initial dir
				 "SMIL files (*.smil *.smi);; All files (*.smil *.smi *.mms *.grins);; Any file (*)", // file types
				 this,
				 "open file dialog",
				 "Double Click a file to open"
				 );
#else	/*QT_NO_FILEDIALOG*/	
		m_smilfilename;
#endif	/*QT_NO_FILEDIALOG*/
	openSMILfile(smilfilename, IO_ReadOnly);
	slot_play();
}

void 
qt_gui::slot_open_url() {
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
  	bool ok;
	QString smilfilename =
		QInputDialog::getText(
				      "AmbulantPlayer",
				      "URL to open:",
				      QLineEdit::Normal,
				      QString::null,
				      &ok,
				      this
				 );
	if (ok && !smilfilename.isEmpty()) {
	    openSMILfile(smilfilename, IO_ReadOnly);
	    slot_play();
	}
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	QMessageBox::information (this, m_programfilename,
		"Open URL not implemented for Embedded Qt");
#endif/*QT_NO_FILEDIALOG*/
}

void 
qt_gui::slot_player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_player_done");
	m_playmenu->setItemEnabled(m_pause_id, false);
	m_playmenu->setItemEnabled(m_play_id, true);
	m_playing = false;
	QObject::disconnect(this, SIGNAL(signal_player_done()),
			    this, SLOT(slot_player_done()));
}

void 
qt_gui::need_redraw (const void* r, void* w, const void* pt) {
	AM_DBG printf("qt_gui::need_redraw(0x%x)-r=(0x%x)\n",
	(void *)this,r);
}

void 
qt_gui::player_done() {
	AM_DBG printf("%s-%s\n", m_programfilename, "player_done");
	emit signal_player_done();
}

void 
qt_gui::slot_play() {
	AM_DBG printf("%s-%s\n", m_programfilename, "slot_play");
	if (m_smilfilename == NULL || m_mainloop == NULL) {
		QMessageBox::information(
			this, m_programfilename,
			"No file open: Please first select File->Open");
		return;
	}
	if (!m_playing) {
		m_playing = true;
		QObject::connect(this, SIGNAL(signal_player_done()),
				 this, SLOT(slot_player_done()));
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
	m_mainloop->stop();
	m_playmenu->setItemEnabled(m_pause_id, false);
	m_playmenu->setItemEnabled(m_play_id, true);
	m_playing = false;
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
	AM_DBG printf("%s-%s\n", m_programfilename, ":unsetCursor");
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

int
main (int argc, char*argv[]) {
	FILE* DBG = stdout;
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	QApplication myapp(argc, argv);
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	QPEApplication myapp(argc, argv);
#endif/*QT_NO_FILEDIALOG*/

	/* Setup widget */
	qt_gui* mywidget = new qt_gui(argv[0], argc > 1 ? argv[1] : "");
#ifndef QT_NO_FILEDIALOG     /* Assume plain Qt */
	mywidget->setGeometry(750, 50, 320, 240);
	QCursor qcursor(Qt::ArrowCursor);
	mywidget->setCursor(qcursor);
	myapp.setMainWidget(mywidget);
#else /*QT_NO_FILEDIALOG*/   /* Assume embedded Qt */
	myapp.showMainWidget(mywidget);
#endif/*QT_NO_FILEDIALOG*/
	mywidget->show();
	
	fprintf(DBG, "argc=%d argv[0]=%s\n", argc, argv[0]);
	if (argc > 1) {
	  char last[6];
	  char*str = argv[argc-1];
	  int len = strlen(str);
	  strcpy(last, &str[len-5]);
	  fprintf(DBG, "%s %s %d\n", str, last, strcpy(last, ".smil"));
	  if (strcmp(last, ".smil") == 0
	      || strcmp(&last[1], ".smi") == 0
	      || strcmp(&last[1], ".sml") == 0) {
	    mywidget->openSMILfile(argv[argc-1], IO_ReadOnly);
	    mywidget->slot_play();
	  }
	} else {
	  char fnbuf[1024];
	  snprintf(fnbuf, 1024, "%s/.ambulant_welcomeSeen", getenv("HOME"));
	  if (access(fnbuf, 0) < 0) {
	  	close(creat(fnbuf, 0666));
		char *welcome_doc = find_welcome_doc();
		if (welcome_doc) {
			mywidget->openSMILfile(welcome_doc, IO_ReadOnly);
			mywidget->slot_play();
		}
	  }
	}
	  	
	myapp.exec();
	std::cout << "Exiting program" << std::endl;
	return 0;
}
