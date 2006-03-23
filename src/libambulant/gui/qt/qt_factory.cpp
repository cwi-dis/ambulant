// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

 
//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
 
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#ifdef	WITH_QT_HTML_WIDGET
#include "ambulant/gui/qt/qt_html_renderer.h"
#endif/*WITH_QT_HTML_WIDGET*/
#include "ambulant/gui/qt/qt_text_renderer.h"
#include "ambulant/gui/qt/qt_video_renderer.h"


using namespace ambulant;
using namespace gui::qt;
using namespace net;

qt_video_factory::~qt_video_factory()
{
}

qt_renderer_factory::qt_renderer_factory(common::factories *factory)
:	m_factory(factory)
{
	AM_DBG lib::logger::get_logger()->debug("qt_renderer factory (0x%x)", (void*) this);
}
	
qt_window_factory::qt_window_factory( QWidget* parent_widget, int top_offset)
:	m_parent_widget(parent_widget),
    m_top_offset(top_offset)
{
	AM_DBG lib::logger::get_logger()->debug("qt_window_factory (0x%x)", (void*) this);
}	
  
ambulant_qt_window::ambulant_qt_window(const std::string &name,
	   lib::rect* bounds,
	   common::gui_events *region)
:	common::gui_window(region),
	m_ambulant_widget(NULL),
	m_pixmap(NULL),
	m_oldpixmap(NULL),
	m_tmppixmap(NULL),
#ifdef USE_SMIL21
	m_fullscreen_count(0),
	m_fullscreen_prev_pixmap(NULL),
	m_fullscreen_old_pixmap(NULL),
	m_fullscreen_engine(NULL),
	m_fullscreen_now(0),
#endif
	m_surface(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::ambulant_qt_window(0x%x)",(void *)this);
}

ambulant_qt_window::~ambulant_qt_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::~ambulant_qt_window(0x%x): m_ambulant_widget=0x%x, m_pixmap=0x%x",this,m_ambulant_widget, m_pixmap);
	// Note that we don't destroy the widget, only sver the connection.
	// the widget itself is destroyed independently.
	if (m_ambulant_widget ) {
		m_ambulant_widget->set_qt_window(NULL);
		delete m_ambulant_widget;
		m_ambulant_widget = NULL;
		delete m_pixmap;
		m_pixmap = NULL;
		if (m_tmppixmap != NULL) {
			delete m_tmppixmap;
			m_tmppixmap = NULL;
		}
	}
}
	
void
ambulant_qt_window::set_ambulant_widget(qt_ambulant_widget* qaw)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::set_ambulant_widget(0x%x)",(void *)qaw);
	// Don't destroy!
	//if (m_ambulant_widget != NULL)
	//	delete m_ambulant_widget;
	m_ambulant_widget = qaw;

	if (qaw != NULL) {
		QSize size = qaw->frameSize();
		m_pixmap = new QPixmap(size.width(), size.height());
		QPainter paint(m_pixmap);
		QColor bgc = QColor(255,255,255); // white color
		// in debugging mode, initialize with purple background
		AM_DBG bgc = QColor(255,  0,255); // purple color
		
		paint.setBrush(bgc);
		paint.drawRect(0,0,size.width(),size.height());
		paint.flush();
		paint.end();
	}
}

QPixmap*
ambulant_qt_window::get_ambulant_pixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::ambulant_pixmap(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return m_pixmap;
}

QPixmap*
ambulant_qt_window::get_pixmap_from_screen(const lib::rect &r)
{
	QPixmap *rv = new QPixmap(r.width(), r.height());
	bitBlt(rv, 0, 0, m_pixmap, r.left(), r.top(), r.width(), r.height());
	return rv;
}

qt_ambulant_widget*
ambulant_qt_window::get_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::get_ambulant_widget(0x%x)",(void *)m_ambulant_widget);
	return m_ambulant_widget;
}

QPixmap*
ambulant_qt_window::new_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::new_ambulant_surface(0x%x)",(void *)m_surface);
	if (m_surface != NULL) delete m_surface;
	QSize size = m_pixmap->size();
	m_surface = new QPixmap(size.width(), size.height());
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::new_ambulant_surface(0x%x)",(void *)m_surface);
        return m_surface;
}

QPixmap*
ambulant_qt_window::get_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::get_ambulant_surface(0x%x) = 0x%x",(void *)this,(void *)m_surface);
        return m_surface;
}

QPixmap*
ambulant_qt_window::get_ambulant_oldpixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::get_ambulant_oldpixmap(0x%x) = 0x%x",(void *)this,(void *)m_oldpixmap);
#ifdef USE_SMIL21
	if (m_fullscreen_count && m_fullscreen_old_pixmap)
		return m_fullscreen_old_pixmap;
#endif
        return m_oldpixmap;
}

void
ambulant_qt_window::reset_ambulant_surface(void)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::reset_ambulant_surface(0x%x) m_oldpixmap = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_oldpixmap != NULL) m_pixmap = m_oldpixmap;
}

void
ambulant_qt_window::set_ambulant_surface(QPixmap* surf)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::set_ambulant_surface(0x%x) surf = 0x%x",(void *)this,(void *)surf);
	m_oldpixmap = m_pixmap;
	if (surf != NULL) m_pixmap = surf;
}

void
ambulant_qt_window::delete_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::delete_ambulant_surface(0x%x) m_surface = 0x%x",(void *)this, (void *)m_surface);
	delete m_surface;
	m_surface = NULL;
}

void
ambulant_qt_window::need_redraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::need_redraw(0x%x): ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (m_ambulant_widget == NULL) {
		lib::logger::get_logger()->error("ambulant_qt_window::need_redraw(0x%x): m_ambulant_widget == NULL !!!", (void*) this);
		return;
	}
#ifdef	QT_NO_FILEDIALOG	/* Assume embedded Qt */
	m_ambulant_widget->repaint(r.left(), r.top(), r.width(), r.height(), false);
	qApp->wakeUpGuiThread();
//	qApp->processEvents();
#else	/*QT_NO_FILEDIALOG*/	/* Assume plain Qt */
	m_ambulant_widget->update(r.left(), r.top(), r.width(), r.height());
	qApp->wakeUpGuiThread();
#endif	/*QT_NO_FILEDIALOG*/
}

void
ambulant_qt_window::redraw_now()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::redraw_now()");
	m_ambulant_widget->repaint(false);
}

void
ambulant_qt_window::mouse_region_changed()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::mouse_region_changed needs to be implemented");
}

/* test if there is something new to see */
static QImage* oldImageP;
static bool isEqualToPrevious(QPixmap* qpmP) {
	return false;
	QImage img = qpmP->convertToImage();
	if (oldImageP != NULL && img == *oldImageP) {
		AM_DBG lib::logger::get_logger()->debug("isEqualToPrevious: new image not different from old one");
		return true;
	} else {
		if (oldImageP != NULL) delete oldImageP;
		oldImageP = new QImage(img);
		return false;
	}
}

#ifdef DUMPPIXMAP
// doesn't compile on Zaurus
/**/
/* dumpPixmap on file */
void gui::qt::dumpPixmap(QPixmap* qpm, std::string filename) {
	if ( ! qpm) return;
	QImage img = qpm->convertToImage();
	if ( ! isEqualToPrevious(qpm)) {
		static int i;
		char buf[5];
		sprintf(buf,"%04d",i++);
		std::string newfile = buf + std::string(filename) +".png";
		qpm->save(newfile, "PNG");
		AM_DBG lib::logger::get_logger()->debug("dumpPixmap(%s)", newfile.c_str());
	}
}
/**/
#endif

void
ambulant_qt_window::redraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.right(), r.bottom());
#ifdef USE_SMIL21
	_screenTransitionPreRedraw();
#endif
	m_handler->redraw(r, this);
//XXXX	if ( ! isEqualToPrevious(m_pixmap))
#ifdef USE_SMIL21
	_screenTransitionPostRedraw(r);
#endif
	bitBlt(m_ambulant_widget,r.left(),r.top(), m_pixmap,r.left(),r.top(), r.right(),r.bottom());
//XXXX	dumpPixmap(m_pixmap, "top"); //AM_DBG 
}

void
ambulant_qt_window::user_event(const lib::point &where, int what) 
{
        AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::user_event(0x%x): point=(%d,%d)", this, where.x, where.y);
	m_handler->user_event(where, what);
}

void
ambulant_qt_window::need_events(bool want) 
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::need_events(0x%x): want=%d", this, want);
}

// XXXX
qt_ambulant_widget::qt_ambulant_widget(const std::string &name,
	lib::rect* bounds,
	QWidget* parent_widget)
:	QWidget(parent_widget,"qt_ambulant_widget",0),
	m_qt_window(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::qt_ambulant_widget(0x%x-0x%x(%d,%d,%d,%d))",
		(void *)this,
		(void*)  parent_widget,
		bounds->left(),
		bounds->top(),
		bounds->right(),
		bounds->bottom());
	setGeometry(bounds->left(), bounds->top(), bounds->right(), bounds->bottom());
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	setMouseTracking(true); // enable mouseMoveEvent() to be called
#endif/*QT_NO_FILEDIALOG*/
}

qt_ambulant_widget::~qt_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::~qt_ambulant_widget(0x%x): m_qt_window=0x%x", (void*)this, m_qt_window);
	if (m_qt_window) {
		m_qt_window->set_ambulant_widget(NULL);
		m_qt_window = NULL;
	}
}
	
void
qt_ambulant_widget::paintEvent(QPaintEvent* e)
{
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::paintEvent(0x%x): e=0x%x)", (void*) this, (void*) e);
	QRect qr = e->rect();
	lib::rect r =  lib::rect(
		lib::point(qr.left(),qr.top()),
		lib::size(qr.width(),qr.height()));
	if (m_qt_window == NULL) {
		lib::logger::get_logger()->debug("qt_ambulant_widget::paintEvent(0x%x): e=0x%x m_qt_window==NULL",
			(void*) this, (void*) e);
		return;
	}
	m_qt_window->redraw(r);
}

void
qt_ambulant_widget::mouseReleaseEvent(QMouseEvent* e) {
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::mouseReleaseEvxent(0x%x): e=0x%x, position=(%d, %d))",
		(void*) this, (void*) e, e->x(), e->y());
	if (m_qt_window == NULL) {
		lib::logger::get_logger()->debug("qt_ambulant_widget::mouseReleaseEvent(0x%x): e=0x%x  position=(%d, %d) m_qt_window==NULL",
			(void*) this, (void*) e, e->x(), e->y());
		return;
	}
	lib::point amwhere = lib::point(e->x(), e->y());
	m_qt_window->user_event(amwhere);
}

#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
void 
qt_ambulant_widget::mouseMoveEvent(QMouseEvent* e) {
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::mouseMoveEvent:(%d,%d)\n", e->x(),e->y());
	ambulant::lib::point ap = ambulant::lib::point(e->x(), e->y());
#if 0
    // XXX This code temporarily disabled, because with the current
    // structure there is no easy way to get at the gui_player, which
    // is needed to tell the scheduler we're about to start telling it
    // about pointed() nodes.
    xxx_gui_player->before_mousemove(0);
	m_qt_window->user_event(ap, 1);
	int cursid = xxx_gui_player->after_mousemove();
	Qt::CursorShape cursor_shape = Qt::ArrowCursor;
	if (cursid == 0) {
        ; // pass
    } else if (cursid == 1) {
        cursor_shape = Qt::PointingHandCursor;
    } else {
        lib::logger::get_logger()->debug("mouseMoveEvent: unknown cursor id %d", cursid);
    }
    setCursor(cursor_shape);
#endif
   
}
#endif/*QT_NO_FILEDIALOG*/

void 
qt_ambulant_widget::set_qt_window( ambulant_qt_window* aqw)
{
	// Note: the window and widget are destucted independently.
	//	if (m_qt_window != NULL)
	//	  delete m_qt_window;
	m_qt_window = aqw;
	AM_DBG lib::logger::get_logger()->debug("qt_ambulant_widget::set_qt_window(0x%x): m_qt_window==0x%x)",
		(void*) this, (void*) m_qt_window);
}

ambulant_qt_window* 
qt_ambulant_widget::qt_window() {
	return m_qt_window;
}

// XXXX
common::playable *
qt_renderer_factory::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp) {

	lib::xml_string tag = node->get_qname().second;
	common::playable* rv;
	if (tag == "img") {
 		rv = new qt_image_renderer(context, cookie, node,
						  evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("qt_renderer_factory: node 0x%x: returning qt_image_renderer 0x%x", 
			(void*) node, (void*) rv);
	} else if (tag == "brush") {
 		rv = new qt_fill_renderer(context, cookie, node,
					  evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("qt_renderer_factory: node 0x%x: returning qt_fill_renderer 0x%x", 
			(void*) node, (void*) rv);
	} else if ( tag == "text") {
#ifdef	WITH_QT_HTML_WIDGET
		net::url url = net::url(node->get_url("src"));
		if (url.guesstype() == "text/html") {
			rv = new qt_html_renderer(context, cookie, node, evp, m_factory);
			AM_DBG lib::logger::get_logger()->debug("qt_renderer_factory: node 0x%x: returning qt_html_renderer 0x%x", (void*) node, (void*) rv);
		} else {
#endif/*WITH_QT_HTML_WIDGET*/
		rv = new qt_text_renderer(context, cookie, node,
						 evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("qt_renderer_factory: node 0x%x: returning qt_text_renderer 0x%x",
			(void*) node, (void*) rv);
#ifdef	WITH_QT_HTML_WIDGET
		}
#endif/*WITH_QT_HTML_WIDGET*/
	} else {
		return NULL;
	}
    return rv;
}

common::playable *
qt_renderer_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}
  
common::gui_window *
qt_window_factory::new_window (const std::string &name,
			       lib::size bounds,
			       common::gui_events *region)
{
	lib::rect* r = new lib::rect(lib::point(0, m_top_offset), bounds);
	AM_DBG lib::logger::get_logger()->debug("qt_window_factory::new_window (0x%x): name=%s %d,%d,%d,%d",
		(void*) this, name.c_str(), r->left(),r->top(),r->right(),r->bottom());
 	ambulant_qt_window * aqw = new ambulant_qt_window(name, r, region);
	qt_ambulant_widget * qaw = new qt_ambulant_widget(name, r, m_parent_widget);
#ifndef	QT_NO_FILEDIALOG     /* Assume plain Qt */
	qaw->setBackgroundMode(Qt::NoBackground);
	assert(qApp);
	assert(qApp->mainWidget());
	qApp->mainWidget()->resize(bounds.w, bounds.h+m_top_offset);
#else	/*QT_NO_FILEDIALOG*/  /* Assume embedded Qt */
	qaw->setBackgroundMode(QWidget::NoBackground);
	/* No resize implemented for embedded Qt */
#endif	/*QT_NO_FILEDIALOG*/
	aqw->set_ambulant_widget(qaw);
	qaw->set_qt_window(aqw);
 	AM_DBG lib::logger::get_logger()->debug("qt_window_factory::new_window(0x%x): ambulant_widget=0x%x qt_window=0x%x",
		(void*) this, (void*) qaw, (void*) aqw);
	qaw->show();
	return aqw;
}

common::bgrenderer *
qt_window_factory::new_background_renderer(const common::region_info 
					   *src)
{
	AM_DBG lib::logger::get_logger()->debug("qt_window_factory::new_background_renderer(0x%x): src=0x%x",
		(void*) this, src);
	return new qt_background_renderer(src);
}

common::playable *
qt_video_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;
	
	lib::xml_string tag = node->get_qname().second;
    AM_DBG lib::logger::get_logger()->debug("qt_video_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "video") {
	  rv = new qt_video_renderer(context, cookie, node, evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("qt_video_factory: node 0x%x: returning qt_video_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("qt_video_factory: no renderer for tag \"%s\"", tag.c_str());
		return NULL;
	}
	return rv;
}

common::playable *
qt_video_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}
  
#ifdef USE_SMIL21

void 
ambulant_qt_window::startScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::startScreenTransition()");
	if (m_fullscreen_count)
		logger::get_logger()->warn("ambulant_qt_window::startScreenTransition():multiple Screen transitions in progress (m_fullscreen_count=%d)",m_fullscreen_count);
	m_fullscreen_count++;
	if (m_fullscreen_old_pixmap) delete m_fullscreen_old_pixmap;
	m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
	m_fullscreen_prev_pixmap = NULL;
}

void 
ambulant_qt_window::endScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::endScreenTransition()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_count--;
}

void 
ambulant_qt_window::screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::screenTransitionStep()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_engine = engine;
	m_fullscreen_now = now;
}
		
void 
ambulant_qt_window::_screenTransitionPreRedraw()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPreRedraw()");
	if (m_fullscreen_count == 0) return;
	// XXX setup drawing to transition surface
//	[[self getTransitionSurface] lockFocus];
}

void 
ambulant_qt_window::_screenTransitionPostRedraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPostRedraw()");
	if (m_fullscreen_count == 0 && m_fullscreen_old_pixmap == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPostRedraw: screen snapshot");
		if (m_fullscreen_prev_pixmap) delete m_fullscreen_prev_pixmap;
		m_fullscreen_prev_pixmap = get_pixmap_from_screen(r); // XXX wrong
//		dumpPixmap(m_fullscreen_prev_pixmap, "snap");
		return;
	}
	if (m_fullscreen_old_pixmap == NULL) {
		// Just starting a new fullscreen transition. Get the
		// background bits from the snapshot saved during the previous
		// redraw.
		m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
		m_fullscreen_prev_pixmap = NULL;
	}
	
	AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPostRedraw: bitblit");
	if (m_fullscreen_engine) {
		// Do the transition step
		QPixmap* new_src = get_ambulant_surface();
		if ( ! new_src) new_src = new_ambulant_surface();
		bitBlt(m_surface, 0, 0, m_pixmap);
		bitBlt(m_pixmap, 0, 0, m_fullscreen_old_pixmap);
//		dumpPixmap(new_src, "fnew");
//		dumpPixmap(m_pixmap, "fold");
		m_fullscreen_engine->step(m_fullscreen_now);
//		dumpPixmap(m_pixmap, "fres");
	}

	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG lib::logger::get_logger()->debug("ambulant_qt_window::_screenTransitionPostRedraw: cleanup after transition done");
		if (m_fullscreen_old_pixmap) delete m_fullscreen_old_pixmap;
		m_fullscreen_old_pixmap = NULL;
		m_fullscreen_engine = NULL;
	}
}
#endif
