/* qt_gui.cpp - Qt GUI for Ambulant
 *              Contains some empty widgets (QDial, QSlider) that
 *              could be used in the future for Volume setting
 *              or Browsing through a MultiMedia presentation.
 *
 * Kees Blom, Sept.25 2003
 */

#include <pthread.h>
#include "qt_gui.h"
#include "qt_mainloop.h"
#include "qt_renderer.h"

const QString about_text = 
                    "This is the skeleton Qt GUI for Ambulant.\n"
                     "Work in  progress by Kees Blom (C) 2003";

qt_gui::qt_gui(const char* title,
	       const char* initfile) {

  m_ambulant_window = NULL;
  m_programfilename = title;
  m_smilfilename = QString("()");
  m_playing = false;
  m_pausing = false;
  setCaption(initfile);

  /* Menu bar */
  QMenuBar*  menubar = new QMenuBar(this,"MainMenu");
  {
    int id;
    /* File */
    QPopupMenu* filemenu = new QPopupMenu (this);
    assert(filemenu);
    filemenu->insertItem("&Open", this,SLOT(slot_open()));
    filemenu->insertItem("&Full Screen", this,SLOT(showFullScreen()));
    filemenu->insertItem("&Normal", this,SLOT(showNormal()));
    filemenu->insertItem("&Quit", qApp, SLOT(quit()));
    menubar->insertItem("&File", filemenu);
    
    /* Play */
    m_playmenu = new QPopupMenu (this, "PlayA");
    assert(m_playmenu);
    m_play_id =  m_playmenu->insertItem("Pla&y",  this, SLOT(slot_play()));
    m_playmenu->setItemEnabled(m_play_id, false);
    m_pause_id = m_playmenu->insertItem("&Pause", this, SLOT(slot_pause()));
    m_playmenu->setItemEnabled(m_pause_id, false);
    m_playmenu->insertItem("&Stop",  this, SLOT(slot_stop()));
    menubar->insertItem("Pla&y", m_playmenu);
    
    /* Help */
    QPopupMenu* helpmenu = new QPopupMenu (this, "HelpA");
    assert(helpmenu);
    helpmenu->insertItem("&About", this, SLOT(slot_about()));
    menubar->insertItem("&Help", helpmenu);
    menubar->setGeometry(0,0,320,20);
  }
  /* Workspace */
  m_workspace = new QWidget(this);
  assert(m_workspace);
  m_workspace->setGeometry(0,20,320,220);
  //  m_workspace->setBackgroundMode (Qt::PaletteBackground);
  m_workspace->setBackgroundMode (Qt::PaletteLight);
  //  m_workspace->setScrollBarsEnabled(true);
}
qt_gui::~qt_gui() {
  setCaption(QString::null);
}
void qt_gui::slot_about() {
  QMessageBox::information(this, m_programfilename, about_text,
			   QMessageBox::Ok | QMessageBox::Default
			   );
}
bool checkFilename(QString filename, int mode) {
  QFile* file =  new QFile(filename);
  return file->open(mode);
}
void qt_gui::slot_open() {
  QString smilfilename = 
    QFileDialog::getOpenFileName(
				 ".", // Initial dir
				 "SMIL files (*.smil)", // file types
				 this,
				 "open file dialog",
				 "Double Click a file to open"
				 );
  if (smilfilename.isNull()
      || ! checkFilename(smilfilename, IO_ReadOnly)) {
    char buf[1024];
    sprintf(buf, "Cannot open file \"%s\"\n%s\n",
	    (const char*) smilfilename, strerror(errno));
    QMessageBox::information(this, m_programfilename, buf);
    return;
  }
  m_smilfilename = smilfilename;
  setCaption(basename(m_smilfilename));
  m_playmenu->setItemEnabled(m_pause_id, false);
  m_playmenu->setItemEnabled(m_play_id, true);
}
void qt_gui::slot_player_done() {
  printf("%s-%s\n", m_programfilename, "slot_player_done");
  m_playmenu->setItemEnabled(m_pause_id, false);
  m_playmenu->setItemEnabled(m_play_id, true);
  m_playing = false;
  QObject::disconnect(this, SIGNAL(signal_player_done()),
		      this, SLOT(slot_player_done()));
 }
void qt_gui::need_redraw (const void* r, void* w, const void* pt) {
  printf
    ("qt_gui::need_redraw(0x%x)-r=(0x%x)\n",
     (void *)this,r);
}
void qt_gui::player_done() {
  printf("%s-%s\n", m_programfilename, "player_done");
  emit signal_player_done();
}
void qt_gui::slot_play() {
  printf("%s-%s\n", m_programfilename, "slot_play");
  if (m_smilfilename == NULL) {
    QMessageBox::information(this, m_programfilename,
			     "Please first select File->Open"
			     );
    return;
  }
  if (!m_playing) {
    m_playing = true;
    QObject::connect(this, SIGNAL(signal_player_done()),
		     this, SLOT(slot_player_done()));
    m_playmenu->setItemEnabled(m_play_id, false);
    m_playmenu->setItemEnabled(m_pause_id, true);
    pthread_t playthread;
    int rv = pthread_create(&playthread, NULL, &qt_mainloop::run, this);
  }
  if (!m_pausing) {
     m_pausing = true;
  } else {
    m_pausing = false;
  }
}
void qt_gui::slot_pause() {
  printf("%s-%s\n", m_programfilename, "slot_pause");
  m_playmenu->setItemEnabled(m_pause_id, false);
  m_playmenu->setItemEnabled(m_play_id, true);
}
void qt_gui::slot_stop() {
  printf("%s-%s\n", m_programfilename, "slot_stop");
  QMessageBox::information(this, m_programfilename,
			   "This will be \"Stop\"\n"
			   );
}
void qt_gui::paintEvent(QPaintEvent* e) {
  printf("%s-%s\n", m_programfilename, "PaintEvent");
  if (m_ambulant_window == NULL)
    printf("m_ambulant_window == NULL\n");
  else {
    using namespace ambulant::gui::qt_renderer;
    using namespace ambulant::lib;
    qt_passive_window* qpw = (qt_passive_window*) m_ambulant_window;
    QRect qr = e->rect();
    screen_rect<int> r =  screen_rect<int>(point(qr.left(),qr.top()),
					   point(qr.right(),qr.bottom()));
    qpw->redraw(r,qpw);
  }
}

int main (int argc, char*argv[]) {
  QApplication myapp(argc, argv);

  /* Setup widget */
  qt_gui* mywidget
                = new qt_gui(argv[0],
		  argc > 1 ? argv[1] : "");
  mywidget->setGeometry(750, 50, 320, 240);
  /* Fire */
  myapp.setMainWidget(mywidget);
  mywidget->show();
  return myapp.exec();
}
