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

/* 
 * @$Id$ 
 */
#ifdef  WITH_SMIL30
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_smiltext.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"
#include "ambulant/common/factory.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using ambulant::lib::logger;

gui::qt::qt_smiltext_renderer::qt_smiltext_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp)
:  qt_renderer<renderer_playable>(context, cookie, node, evp),
	m_epoch(0),
	m_x(0),
	m_y(0),
	m_max_ascent(0),
	m_max_descent(0),
	m_engine(smil2::smiltext_engine(node, evp, this, true)),
	m_params(m_engine.get_params())
{
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_renderer(0x%x)", this);
}


gui::qt::qt_smiltext_renderer::~qt_smiltext_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~qt_smiltext_renderer(0x%x)", this);
	m_lock.enter();

	m_lock.leave();
}

void
gui::qt::qt_smiltext_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_renderer::start(0x%x)", this);
		
	m_lock.enter();
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	renderer_playable::start(t);
	m_lock.leave();
}

void 
gui::qt::qt_smiltext_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_renderer::stop(0x%x)", this);
	m_lock.enter();
	m_dest->renderer_done(this);
	m_activated = false;
	assert(m_context);
	m_context->stopped(m_cookie);
	m_lock.leave();
}

void
gui::qt::qt_smiltext_renderer::smiltext_changed() {
	m_dest->need_redraw();
}

void
gui::qt::qt_smiltext_renderer::_qt_smiltext_changed(const lib::rect r) {
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_renderer::_qt_smiltext_changed(0x%x) r=(L=%d,T=%d,W=%d,H=%d", this,r.left(),r.top(),r.width(),r.height());
	int nbr = 0; // number of breaks (newlines) before current line

	// Compute the shifted position of what we want to draw w.r.t. the visible origin
	lib::point logical_origin(0, 0);
	if (m_params.m_mode == smil2::stm_crawl) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		logical_origin.x += (int) now * m_params.m_rate / 1000;
		if (logical_origin.x < 0)
			AM_DBG lib::logger::get_logger()->debug("qt_smiltext_renderer::_qt_smiltext_changed(0x%x): strange: logical_x=%d, m_epoch=%ld, elpased=%ld !", this, logical_origin.x, m_epoch, elapsed);
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		logical_origin.y += (int) now * m_params.m_rate / 1000;
	}
	AM_DBG logger::get_logger()->debug("_qt_smiltext_changed: logical_origin(%d,%d)", logical_origin.x, logical_origin.y);

	if (logical_origin.x || logical_origin.y)
		_qt_smiltext_shift( m_dest->get_rect(), logical_origin);

	// Always re-compute and re-render everything when new text is added.
	// E.g. word with bigger font may need adjustment of prior text 
	// Therefore, m_engine.newbegin() is can NOT be used
	smil2::smiltext_runs::const_iterator cur = m_engine.begin();
	lib::rect ldr = m_dest->get_rect(); // logical destination rectangle

	AM_DBG lib::logger::get_logger()->debug("_qt_smiltext_changed(0x%x): ldr=(L=%d,T=%d,W=%d,H=%d", this,ldr.left(),ldr.top(),ldr.width(),ldr.height());

	m_y = ldr.y;
	m_max_ascent = m_max_descent = 0;
	// count number of initial breaks before first line
	while (cur->m_command == smil2::stc_break) {
		nbr++;
		cur++;
	}
	while (cur != m_engine.end()) {
		// compute layout of next line
		smil2::smiltext_runs::const_iterator bol = cur; // begin of line pointer
		bool fits = false;
		m_x = ldr.x;
		if (nbr > 0 && (m_max_ascent != 0 || m_max_descent != 0)) {
			// correct m_y for proper size of previous line
			m_y += (m_max_ascent + m_max_descent);
			nbr--;
		}
		m_max_ascent = m_max_descent = 0;
		AM_DBG lib::logger::get_logger()->debug("_qt_smiltext_changed(): command=%d data=%s",cur->m_command,cur->m_data.c_str()==NULL?"(null)":cur->m_data.c_str());
		while (cur != m_engine.end()) {
			fits = _qt_smiltext_fits(*cur, m_dest->get_rect());
			if ( ! fits || cur->m_command == smil2::stc_break)
				break;
			cur++;
		}
		m_x = ldr.x; // m_x was modified by qt_smiltext_fits()
		// move down number of breaks times height of current line
		m_y += (m_max_ascent + m_max_descent) * nbr;
		// count number of breaks in front of next line
		nbr = 0;
		while (cur->m_command == smil2::stc_break) {
			nbr++;
			cur++;
		}
		if ( ! fits && nbr == 0)
			nbr = 1;
		while (bol != cur) {
			// compute rectangle where to render this text
			lib::rect cr = _qt_smiltext_compute(*bol, r);
			_qt_smiltext_render(*bol, cr, logical_origin);
			bol++;
		}
	}
	m_engine.done();
}

bool
gui::qt::qt_smiltext_renderer::_qt_smiltext_fits(const smil2::smiltext_run strun, const lib::rect r) {

	text_metrics tm = _qt_smiltext_get_text_metrics (strun);

	if (tm.get_ascent() > m_max_ascent)
		m_max_ascent = tm.get_ascent();
	if (tm.get_descent() > m_max_descent)
		m_max_descent = tm.get_descent();
	m_x += tm.get_width();
	if (m_x > r.right())
		return false;
	else 	return true;
}

lib::rect
gui::qt::qt_smiltext_renderer::_qt_smiltext_compute(const smil2::smiltext_run strun, const lib::rect r) {

	lib::rect rv = r;
	text_metrics tm = _qt_smiltext_get_text_metrics (strun);

	rv.x += m_x;
	rv.w = tm.get_width();
	m_x  += rv.w;

	rv.y += (m_y + m_max_ascent - tm.get_ascent());
	rv.h = tm.get_line_spacing();

	return rv;
}

gui::qt::text_metrics
gui::qt::qt_smiltext_renderer::_qt_smiltext_get_text_metrics(const smil2::smiltext_run strun) {
	unsigned int ascent = 0, descent = 0, height = 0, width = 0, line_spacing = 0;

	if (strun.m_command == smil2::stc_data 	&& strun.m_data.length() != 0) {

		_qt_smiltext_set_font (strun);

		QFontMetrics qfm(m_font);
		ascent	= qfm.ascent();
		descent	= qfm.descent();
		height	= qfm.height();
		line_spacing = qfm.lineSpacing();

		QRect qr = qfm.boundingRect(strun.m_data);
		width	= qr.width()+qfm.width(' ');
	}
	return text_metrics(ascent, descent, height, width, line_spacing);
}

void
gui::qt::qt_smiltext_renderer::_qt_smiltext_render(const smil2::smiltext_run strun, const lib::rect r, lib::point p) {
	if (strun.m_command != smil2::stc_data)
		return;
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_render(): command=%d data=%s color=0x%x",strun.m_command,strun.m_data.c_str()==NULL?"(null)":strun.m_data.c_str(),strun.m_color);
	int L = r.left()-p.x, 
	    T = r.top()-p.y,
	    W = r.width(),
	    H = r.height();

	_qt_smiltext_set_font(strun);

	QPainter paint;
	paint.begin(m_window->get_ambulant_pixmap());

	if ( ! strun.m_bg_transparent) {
		lib::color_t bg_color = strun.m_bg_color;
		QColor qt_bg_color(redc(bg_color), greenc(bg_color), bluec(bg_color));
		paint.setBackgroundMode(Qt::OpaqueMode);
		paint.setBackgroundColor(qt_bg_color);
	}
	if ( ! strun.m_transparent) {
		lib::color_t text_color = strun.m_color;
		QColor qt_color(redc(text_color), greenc(text_color), bluec(text_color));
		paint.setPen(qt_color);
	}
		
	paint.setFont(m_font);

	paint.drawText(L,T,W,H,Qt::AlignLeft|Qt::AlignTop|Qt::WordBreak, strun.m_data);
	paint.flush();
	paint.end();
}

void
gui::qt::qt_smiltext_renderer::_qt_smiltext_shift(const lib::rect r, const lib::point p) {
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_shift(): r=(%d,%d,%d,%d) p=%d,%d,",r.left(),r.top(),r.right(),r.bottom(),p.x,p.y);
}

void
gui::qt::qt_smiltext_renderer::_qt_smiltext_set_font(const smil2::smiltext_run strun) {
	const char *fontname = strun.m_font_family;
	m_font = QFont(QApplication::font());
	if (fontname) {
                m_font.setFamily(fontname);
	} else {
	        m_font.setFamily(m_font.defaultFamily());
	}
	switch(strun.m_font_style) {
		default:
		case smil2::sts_normal:
		// use default style
			break;
		case smil2::sts_italic:
			m_font.setItalic(true);
			break;
		case smil2::sts_oblique:
		case smil2::sts_reverse_oblique:
		// no (reverse) oblique fonts available in Qt 3.3
			m_font.setItalic(true);
			break;
	}
	int weight = QFont::Normal;
	switch(strun.m_font_weight) {
		default:
		case smil2::stw_normal:
			break;
		case smil2::stw_bold:
		  weight = QFont::Bold;
			break;
	}
	m_font.setWeight(weight);
	m_font.setPixelSize(strun.m_font_size);

}

void
gui::qt::qt_smiltext_renderer::user_event(const lib::point& pt, int what) {
	m_lock.enter();
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	m_lock.leave();
}

void
gui::qt::qt_smiltext_renderer::redraw_body(const lib::rect& dirty, common::gui_window *window) {
	m_window = (ambulant_qt_window*) window;

	lib::rect r = dirty;
	
	// Translate smiltext region dirty rect. to final viewport coordinates 
	lib::point pt = m_dest->get_global_topleft();
	r.translate(pt);
		
	_qt_smiltext_changed(r);
}
#endif //WITH_SMIL30

