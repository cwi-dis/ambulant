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

#ifndef __QT_GUI_H__
#define __QT_GUI_H__

#include "unix_preferences.h"

#include <qfeatures.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qdial.h>
#include <qevent.h>
#include <qhbox.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qiodevice.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qwidget.h>

#ifdef	WITH_QT_HTML_WIDGET
#include <kapp.h>
#include <kmainwindow.h>
#endif/*WITH_QT_HTML_WIDGET*/

#include "qt_logger.h"
#include "qt_settings.h"

class qt_mainloop;

#ifdef	WITH_QT_HTML_WIDGET
#define qt_gui_BASE KMainWindow
#else /*WITH_QT_HTML_WIDGET*/
#define qt_gui_BASE QWidget
#endif/*WITH_QT_HTML_WIDGET*/

class qt_gui : public qt_gui_BASE
{

	Q_OBJECT

  public:
	qt_gui(const char* title, const char* initfile);
	~qt_gui();

	const char* filename() { return m_smilfilename.ascii(); }

	bool openSMILfile(const QString smilfilename, int mode);

	// send a QEvent to the gui thread
	void internal_message(int level, const char* msg);

	// signal interfaces
	void need_redraw(const void*, void*, const void*);
	void player_done();
	void player_start(QString,bool,bool);

/*TMP*/ qt_mainloop* m_mainloop;
  private:
	void _update_menus();

	QPushButton* m_cancel_pb; // for Settings window
	QPopupMenu*	m_filemenu;
	QHBox* m_finish_hb; // for Settings window
	QPopupMenu* m_helpmenu;
	QMenuBar* m_menubar;
	QPushButton* m_ok_pb;	  // for Settings window
	int m_pause_id;
	int m_play_id;
	int m_stop_id;
	int m_reload_id;
	QPopupMenu* m_playmenu;
	const char* m_programfilename;
	qt_settings* m_settings; // the Settings window
	QString m_smilfilename;
	QPopupMenu*	 m_viewmenu;
	int m_menubar_height; // Hack: top Y position of player widget

#define TRY_LOCKING
#ifdef TRY_LOCKING
	pthread_cond_t m_cond_message;
	pthread_mutex_t m_lock_message;
	pthread_t m_gui_thread;
#endif/*TRY_LOCKING*/
	Qt::CursorShape m_cursor_shape;
	void fileError(QString smilfilename);

  public slots:
	void setDocument(const QString&);
	// following slots are needed for Qt Embedded, and are implemented
	// as empty functions for normal Qt because Qt's moc doesn't recogzize
	// #ifdef and #define
#define DocLnk void*
	void slot_file_selected(const DocLnk&);
	void slot_close_fileselector();
	void slot_settings_selected(const DocLnk&);
	void slot_close_settings_selector();
	void slot_play();

  private slots:
	void slot_about();
	void slot_homepage();
	void slot_welcome();
	void slot_help();
	void slot_load_settings();
	void slot_logger_window();
	void slot_open();
	void slot_open_url();
	void slot_pause();
	void slot_player_done();
	void slot_quit();
	void slot_reload();
	void slot_settings_cancel();
	void slot_settings_ok();
	void slot_settings_select();
	void slot_stop();

  signals:
	void signal_player_done();
	void signal_need_redraw(const void*, void*, const void*);

  protected:
	void customEvent(QCustomEvent*);
};
#endif/*__QT_GUI_H__*/
