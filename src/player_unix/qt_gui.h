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

/* 
 * @$Id$ 
 */

#ifndef __QT_GUI_H__
#define __QT_GUI_H__

#include "unix_preferences.h"

#include <qfeatures.h>
#ifndef QT_NO_FILEDIALOG	 /* Assume plain Qt */
# include <qapplication.h>
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
#include <qpe/qpeapplication.h>
#include <qpe/applnk.h>
#include <fileselector.h>
#endif/*QT_NO_FILEDIALOG*/
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

 class qt_gui : public qt_gui_BASE {

   Q_OBJECT

   public:
  	qt_gui(const char* title, const char* initfile);
	~qt_gui();
	bool is_busy() { return m_busy; }

	int  get_o_x() {
		return m_o_x;
	}

 	int  get_o_y() {
		return m_o_y;
	}

	const char* filename() { 
		return m_smilfilename.ascii();
	}

	bool openSMILfile(const QString smilfilename, int mode);

	// send a QEvent to the gui thread
	void internal_message(int level, char* msg);

	// signal interfaces
	void need_redraw(const void*, void*, const void*);
	void player_done();
	void player_start(QString,bool,bool);

/*TMP*/	qt_mainloop* m_mainloop;
   private:
	bool	     m_busy;
	QPushButton* m_cancel_pb; // for Settings window
	QPopupMenu*  m_filemenu;
	QHBox* 	     m_finish_hb; // for Settings window
	QPopupMenu*  m_helpmenu;
	QMenuBar*    m_menubar;
	QPushButton* m_ok_pb;	  // for Settings window
	int	     m_o_x;	  // x coord of origin play window
	int	     m_o_y;	  // y coord of origin play window
	int          m_pause_id;
	bool         m_pausing;
	int          m_play_id;
	bool         m_playing;
	QPopupMenu*  m_playmenu;
	const char*  m_programfilename;
	qt_settings* m_settings; // the Settings window
	QString      m_smilfilename;
	QPopupMenu*  m_viewmenu;

#define	TRY_LOCKING
#ifdef	TRY_LOCKING
	pthread_cond_t	  m_cond_message;
	pthread_mutex_t   m_lock_message;
	unsigned long int m_gui_thread;
#endif/*TRY_LOCKING*/
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	Qt::CursorShape m_cursor_shape;
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	bool         m_pointinghand_cursor; //XXXX
	FileSelector*m_fileselector;
	FileSelector*m_settings_selector;
	const DocLnk m_selectedDocLnk;
#endif/*QT_NO_FILEDIALOG*/
	void	     fileError(QString smilfilename);

  public slots:
	void setDocument(const QString&);
  /* following slots are needed for Qt Embedded, and are implemented
     as empty functions for normal Qt because Qt's moc doesn't recogzize
     #ifdef and #define
  */
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
#define DocLnk void*
#endif/*QT_NO_FILEDIALOG*/
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
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	void unsetCursor(); //XXXX
#endif/*QT_NO_FILEDIALOG*/
};
#endif/*__QT_GUI_H__*/
