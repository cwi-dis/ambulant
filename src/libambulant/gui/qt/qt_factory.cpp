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
 
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
 
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#include "ambulant/gui/qt/qt_text_renderer.h"
#include "ambulant/gui/qt/qt_video_renderer.h"


using namespace ambulant;
using namespace gui::qt;
using namespace net;

qt_video_factory::~qt_video_factory()
{
}

qt_renderer_factory::qt_renderer_factory(datasource_factory *df)
:	m_datasource_factory(df)
	{
	AM_DBG lib::logger::get_logger()->trace("qt_renderer factory (0x%x)", (void*) this);
	}
	
qt_window_factory::qt_window_factory( QWidget* parent_widget, int x, int y)
:	m_parent_widget(parent_widget), m_p(lib::point(x,y)) 
	{
	AM_DBG lib::logger::get_logger()->trace("qt_window_factory (0x%x)", (void*) this);
	}	
  
ambulant_qt_window::ambulant_qt_window(const std::string &name,
	   lib::screen_rect<int>* bounds,
	   common::gui_events *region)
:	common::gui_window(region),
	m_ambulant_widget(NULL),
	m_pixmap(NULL),
	m_oldpixmap(NULL),
	m_surface(NULL)
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::ambulant_qt_window(0x%x)",(void *)this);
}

ambulant_qt_window::~ambulant_qt_window()
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::~ambulant_qt_window(0x%x): m_ambulant_widget=0x%x, m_pixmap=0x%x",this,m_ambulant_widget, m_pixmap);
	// Note that we don't destroy the widget, only sver the connection.
	// the widget itself is destroyed independently.
	if (m_ambulant_widget ) {
		m_ambulant_widget->set_qt_window(NULL);
		delete m_ambulant_widget;
		m_ambulant_widget = NULL;
		delete m_pixmap;
		m_pixmap = NULL;
	}
}
	
void
ambulant_qt_window::set_ambulant_widget(qt_ambulant_widget* qaw)
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::set_ambulant_widget(0x%x)",(void *)qaw);
	// Don't destroy!
	//if (m_ambulant_widget != NULL)
	//	delete m_ambulant_widget;
	m_ambulant_widget = qaw;
	if (qaw != NULL) {
		QSize size = qaw->frameSize();
		m_pixmap = new QPixmap(size.width(), size.height());
	}
}

QPixmap*
ambulant_qt_window::ambulant_pixmap()
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::ambulant_pixmap(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
//	return m_ambulant_widget;
        return m_pixmap;
}

qt_ambulant_widget*
ambulant_qt_window::get_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::get_ambulant_widget(0x%x)",(void *)m_ambulant_widget);
	return m_ambulant_widget;
//       return m_pixmap;
}

QPixmap*
ambulant_qt_window::new_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::new_ambulant_surface(0x%x)",(void *)m_surface);
//	return m_ambulant_widget;
	QSize size = m_pixmap->size();
	m_surface = new QPixmap(size.width(), size.height());
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::new_ambulant_surface(0x%x)",(void *)m_surface);
        return m_surface;
}

QPixmap*
ambulant_qt_window::get_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::get_ambulant_surface(0x%x) = 0x%x",(void *)this,(void *)m_surface);
        return m_surface;
}

QPixmap*
ambulant_qt_window::get_ambulant_oldpixmap()
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::get_ambulant_oldpixmap(0x%x) = 0x%x",(void *)this,(void *)m_oldpixmap);
        return m_oldpixmap;
}

void
ambulant_qt_window::reset_ambulant_surface(void)
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::reset_ambulant_surface(0x%x) m_oldpixmap = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_oldpixmap != NULL) m_pixmap = m_oldpixmap;
}

void
ambulant_qt_window::set_ambulant_surface(QPixmap* surf)
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::set_ambulant_surface(0x%x) surf = 0x%x",(void *)this,(void *)surf);
	m_oldpixmap = m_pixmap;
	if (surf != NULL) m_pixmap = surf;
}

void
ambulant_qt_window::delete_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::delete_ambulant_surface(0x%x) m_surface = 0x%x",(void *)this, (void *)m_surface);
	delete m_surface;
	m_surface = NULL;
}

void
ambulant_qt_window::need_redraw(const lib::screen_rect<int> &r)
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::need_redraw(0x%x): ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
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
ambulant_qt_window::mouse_region_changed()
{
	lib::logger::get_logger()->trace("ambulant_qt_window::mouse_region_changed needs to be implemented");
}

void
ambulant_qt_window::redraw(const lib::screen_rect<int> &r)
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",
		(void *)this, r.left(), r.top(), r.right(), r.bottom());
	m_handler->redraw(r, this);
	bitBlt(m_ambulant_widget, 0, 0, m_pixmap);
}

void
ambulant_qt_window::user_event(const lib::point &where, int what) 
{
        AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::user_event(0x%x): point=(%d,%d)", this, where.x, where.y);
	m_handler->user_event(where, what);
}

void
ambulant_qt_window::need_events(bool want) 
{
	AM_DBG lib::logger::get_logger()->trace("ambulant_qt_window::need_events(0x%x): want=%d", this, want);
}

// XXXX
qt_ambulant_widget::qt_ambulant_widget(const std::string &name,
	lib::screen_rect<int>* bounds,
	QWidget* parent_widget)
:	QWidget(parent_widget,"qt_ambulant_widget",0),
	m_qt_window(NULL)
{
	AM_DBG lib::logger::get_logger()->trace("qt_ambulant_widget::qt_ambulant_widget(0x%x-0x%x(%d,%d,%d,%d))",
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
	AM_DBG lib::logger::get_logger()->trace("qt_ambulant_widget::~qt_ambulant_widget(0x%x): m_qt_window=0x%x", (void*)this, m_qt_window);
	if (m_qt_window) {
		m_qt_window->set_ambulant_widget(NULL);
		m_qt_window = NULL;
	}
}
	
void
qt_ambulant_widget::paintEvent(QPaintEvent* e)
{
	AM_DBG lib::logger::get_logger()->trace("qt_ambulant_widget::paintEvent(0x%x): e=0x%x)", (void*) this, (void*) e);
	QRect qr = e->rect();
	lib::screen_rect<int> r =  lib::screen_rect<int>(
		lib::point(qr.left(),qr.top()),
		lib::point(qr.right(),qr.bottom()));
	if (m_qt_window == NULL) {
		lib::logger::get_logger()->trace("qt_ambulant_widget::paintEvent(0x%x): e=0x%x m_qt_window==NULL",
			(void*) this, (void*) e);
		return;
	}
	m_qt_window->redraw(r);
}

void
qt_ambulant_widget::mouseReleaseEvent(QMouseEvent* e) {
	AM_DBG lib::logger::get_logger()->trace("qt_ambulant_widget::mouseReleaseEvxent(0x%x): e=0x%x, position=(%d, %d))",
		(void*) this, (void*) e, e->x(), e->y());
	if (m_qt_window == NULL) {
		lib::logger::get_logger()->trace("qt_ambulant_widget::mouseReleaseEvent(0x%x): e=0x%x  position=(%d, %d) m_qt_window==NULL",
			(void*) this, (void*) e, e->x(), e->y());
		return;
	}
	lib::point amwhere = lib::point(e->x(), e->y());
	m_qt_window->user_event(amwhere);
}

#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
void 
qt_ambulant_widget::mouseMoveEvent(QMouseEvent* e) {
	int m_o_x = 0, m_o_y = 0; //27; // XXXX Origin of MainWidget
	AM_DBG lib::logger::get_logger()->trace("%s:(%d,%d)\n",
	       "qt_ambulant_widget::mouseMoveEvent", e->x(),e->y());
	ambulant::lib::point ap = ambulant::lib::point(e->x()-m_o_x,
						       e->y()-m_o_y);
	m_qt_window->user_event(ap, 1);
	qApp->mainWidget()->unsetCursor(); //XXXX
}
#endif/*QT_NO_FILEDIALOG*/

void 
qt_ambulant_widget::set_qt_window( ambulant_qt_window* aqw)
{
	// Note: the window and widget are destucted independently.
	//	if (m_qt_window != NULL)
	//	  delete m_qt_window;
	m_qt_window = aqw;
	AM_DBG lib::logger::get_logger()->trace("qt_ambulant_widget::set_qt_window(0x%x): m_qt_window==0x%x)",
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
 		rv = new qt_active_image_renderer(context, cookie, node,
			 evp, m_datasource_factory);
		AM_DBG lib::logger::get_logger()->trace("qt_renderer_factory: node 0x%x: returning qt_active_image_renderer 0x%x", 
			(void*) node, (void*) rv);
	} else if ( tag == "text") {
		rv = new qt_active_text_renderer(context, cookie, node,
			evp, m_datasource_factory);
		AM_DBG lib::logger::get_logger()->trace("qt_renderer_factory: node 0x%x: returning qt_active_text_renderer 0x%x",
			(void*) node, (void*) rv);
	} else {
		return NULL;
	}
    return rv;
}
  
common::gui_window *
qt_window_factory::new_window (const std::string &name,
			       lib::size bounds,
			       common::gui_events *region)
{
	lib::screen_rect<int>* r = new lib::screen_rect<int>(m_p, bounds);
	AM_DBG lib::logger::get_logger()->trace("qt_window_factory::new_window (0x%x): name=%s %d,%d,%d,%d",
		(void*) this, name.c_str(), r->left(),r->top(),r->right(),r->bottom());
 	ambulant_qt_window * aqw = new ambulant_qt_window(name, r, region);
	qt_ambulant_widget * qaw = new qt_ambulant_widget(name, r, m_parent_widget);
#ifndef	QT_NO_FILEDIALOG     /* Assume plain Qt */
	qaw->setBackgroundMode(Qt::NoBackground);
	if (qApp == NULL || qApp->mainWidget() == NULL) {
		lib::logger::get_logger()->error("qt_window_factory::new_window (0x%x) %s",
			(void*) this,
	   		"qApp == NULL || qApp->mainWidget() == NULL");
	}
	qApp->mainWidget()->resize(bounds.w + m_p.x, bounds.h + m_p.y);
#else	/*QT_NO_FILEDIALOG*/  /* Assume embedded Qt */
	qaw->setBackgroundMode(QWidget::NoBackground);
	/* No resize implemented for embedded Qt */
#endif	/*QT_NO_FILEDIALOG*/
	aqw->set_ambulant_widget(qaw);
	qaw->set_qt_window(aqw);
 	AM_DBG lib::logger::get_logger()->trace("qt_window_factory::new_window(0x%x): ambulant_widget=0x%x qt_window=0x%x",
		(void*) this, (void*) qaw, (void*) aqw);
	qaw->show();
	return aqw;
}

common::bgrenderer *
qt_window_factory::new_background_renderer(const common::region_info 
					   *src)
{
	AM_DBG lib::logger::get_logger()->trace("qt_window_factory::new_background_renderer(0x%x): src=0x%x",
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
    AM_DBG lib::logger::get_logger()->trace("qt_video_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "video") {
	  rv = new qt_active_video_renderer(context, cookie, node, evp, m_datasource_factory);
		AM_DBG lib::logger::get_logger()->trace("qt_video_factory: node 0x%x: returning qt_video_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->trace("qt_video_factory: no renderer for tag \"%s\"", tag.c_str());
		return NULL;
	}
	return rv;
}
