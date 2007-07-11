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
  :     qt_renderer<renderer_playable>(context, cookie, node, evp),
	m_layout_engine(smil2::smiltext_layout_engine(node, evp, this, this))
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
//JUNK?	m_epoch = m_event_processor->get_timer()->elapsed();
	m_layout_engine.start(t);
	renderer_playable::start(t);
	m_layout_engine.set_dest_rect(m_dest->get_rect());
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

smil2::smiltext_metrics
gui::qt::qt_smiltext_renderer::get_smiltext_metrics(const smil2::smiltext_run& strun) {
	unsigned int ascent = 0, descent = 0, height = 0, width = 0, line_spacing = 0, word_spacing = 0;

	if (strun.m_command == smil2::stc_data 	&& strun.m_data.length() != 0) {

		_qt_smiltext_set_font (strun);

		QFontMetrics qfm(m_font);
		ascent	= qfm.ascent();
		descent	= qfm.descent();
		height	= qfm.height();
		line_spacing = qfm.lineSpacing();
		word_spacing = qfm.width(' ');

		QRect qr = qfm.boundingRect(strun.m_data);
		width	 = qr.width();
	}
	return smil2::smiltext_metrics(ascent, descent, height, width, line_spacing, word_spacing);
}

void
gui::qt::qt_smiltext_renderer::render_smiltext(const smil2::smiltext_run& strun, const lib::rect& r, unsigned int word_spacing) {
	if (strun.m_command != smil2::stc_data)
		return;
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_render(): command=%d data=%s color=0x%x",strun.m_command,strun.m_data.c_str()==NULL?"(null)":strun.m_data.c_str(),strun.m_color);
	int L = r.left(), 
	    T = r.top(),
	    W = r.width(),
	    H = r.height();
	m_layout_engine.set_dest_rect(m_dest->get_rect());
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

	paint.drawText(L,T,W,H,Qt::AlignLeft|Qt::AlignTop, strun.m_data);
	paint.flush();
	paint.end();
}

void
gui::qt::qt_smiltext_renderer::_qt_smiltext_set_font(const smil2::smiltext_run& strun) {
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
		
	m_layout_engine.redraw(r);
}
#endif //WITH_SMIL30

