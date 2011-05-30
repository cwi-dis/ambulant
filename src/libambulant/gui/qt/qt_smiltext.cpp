// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_smiltext.h"
#include "ambulant/gui/qt/qt_util.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"
#include "ambulant/common/factory.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using ambulant::lib::logger;

extern const char qt_smiltext_playable_tag[] = "smilText";
extern const char qt_smiltext_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererQt");
extern const char qt_smiltext_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererSmilText");

common::playable_factory *
gui::qt::create_qt_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererQt"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSmilText"), true);
	return new common::single_playable_factory<
		gui::qt::qt_smiltext_renderer,
		qt_smiltext_playable_tag,
		qt_smiltext_playable_renderer_uri,
		qt_smiltext_playable_renderer_uri2,
		qt_smiltext_playable_renderer_uri2>(factory, mdp);
}

gui::qt::qt_smiltext_renderer::qt_smiltext_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	m_qt_transparent(
		redc(QT_TRANSPARENT_COLOR),
		greenc(QT_TRANSPARENT_COLOR),
		bluec(QT_TRANSPARENT_COLOR)),
	m_qt_alternative(
		redc(QT_ALTERNATIVE_COLOR),
		greenc(QT_ALTERNATIVE_COLOR),
		bluec(QT_ALTERNATIVE_COLOR)),
	m_bgopacity(1.0),
	m_blending(false),
	qt_renderer<renderer_playable>(context, cookie, node, evp, fp, mdp),
	m_layout_engine(smil2::smiltext_layout_engine(node, evp, this, this, true))
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

	gui::qt::qt_renderer<common::renderer_playable>::start(t);
	m_lock.enter();
	m_layout_engine.start(t);
	renderer_playable::start(t);
	m_layout_engine.set_dest_rect(m_rect = m_dest->get_rect());

	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		alpha_media_bg = ri->get_mediabgopacity();
		m_bgopacity = ri->get_bgopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			lib::color_t chroma_low, chroma_high;
			compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		}
	}

	if ( ! (alpha_media == 1.0 && alpha_media_bg == 1.0 && alpha_chroma == 1.0) ) {
		m_blending = true;
	}
	m_context->started(m_cookie);
	m_lock.leave();
}

bool
gui::qt::qt_smiltext_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_renderer::stop(0x%x)", this);
	m_lock.enter();
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	assert(m_context);
	m_context->stopped(m_cookie);
	m_lock.leave();
	return true;
}

void
gui::qt::qt_smiltext_renderer::marker_seen(const char *name)
{
	m_lock.enter();
	m_context->marker_seen(m_cookie, name);
	m_lock.leave();
}

void
gui::qt::qt_smiltext_renderer::smiltext_stopped() {
	m_context->stopped(m_cookie);
}

void
gui::qt::qt_smiltext_renderer::smiltext_changed() {
	m_layout_engine.smiltext_changed();
	m_dest->need_redraw();
}

smil2::smiltext_metrics
gui::qt::qt_smiltext_renderer::get_smiltext_metrics(const smil2::smiltext_run& strun) {
	unsigned int ascent = 0,
		descent = 0,
		height = 0,
		width = 0,
		line_spacing = 0,
		word_spacing = 0;

	if (strun.m_data.length() != 0) {

		_qt_smiltext_set_font (strun);

		QFontMetrics qfm(m_font);
		ascent = qfm.ascent();
		descent = qfm.descent();
		height = qfm.height();
		line_spacing = qfm.lineSpacing();
		
		// The simple qfm.boundingRect(QString) function sometimes
		// returns wrong (too small) rectangle
		QRect qr = qfm.boundingRect(
			m_rect.x, m_rect.y, m_rect.w, m_rect.h,
			Qt::AlignAuto,
			strun.m_data);
		width = qr.width();
	}
	return smil2::smiltext_metrics(ascent, descent, height, width, line_spacing);
}

const lib::rect&
gui::qt::qt_smiltext_renderer::get_rect() {
	return m_dest->get_rect();
}

void
gui::qt::qt_smiltext_renderer::render_smiltext(const smil2::smiltext_run& strun, const lib::rect& r) {

	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_render(): command=%d data=%s color=0x%x bg_color=0x%x",strun.m_command,strun.m_data.c_str()==NULL?"(null)":strun.m_data.c_str(),strun.m_color,strun.m_bg_color);
	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		alpha_media_bg = ri->get_mediabgopacity();
		m_bgopacity = ri->get_bgopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		}
	}
	// prepare for blending
	QPixmap* bg_pixmap = NULL;
	QPixmap* tx_pixmap = NULL;
	lib::rect rct(r); // rct encloses leading blank and word
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_render(): data=%s r=L=%d,T=%d,W=%d,H=%d", strun.m_data.c_str(),r.x,r.y,r.w,r.h);
	const lib::point p = m_dest->get_global_topleft();
	int L = rct.left()+p.x,
		T = rct.top()+p.y,
		W = rct.width(),
		H = rct.height();

	if (W == 0 || H == 0)
		return; // cannot render anything
	AM_DBG lib::logger::get_logger()->debug("qt_smiltext_render():p=(%d,%d)	 L=%d,T=%d,W=%d,H=%d",p.x,p.y,L,T,W,H);
	if (m_blending) {
		// create pixmaps for blending
		if ( ! strun.m_bg_transparent) {
			// optimization suggestion:
			// maybe it is possible for blending to render
			// everything in one extra pixmap, then first
			// blend with text background color, next blend
			// with text color, without creating bg_ pixmap
			// and new_pixmap (i.e. directly on the screen)
			// This should result in far less round trips to
			// the X-server.
		
			bg_pixmap = new QPixmap(W,H);
			assert( bg_pixmap );
			bg_pixmap->fill(strun.m_bg_color);
		}
		tx_pixmap = new QPixmap(W,H);
		assert( tx_pixmap);
		tx_pixmap->fill(m_qt_transparent);
		if ( ! m_blending)
			alpha_media = alpha_chroma = 1.0;
	}
	_qt_smiltext_set_font(strun);

	QPainter tx_paint, bg_paint;
	lib::color_t text_color = strun.m_color;
	lib::color_t bg_color = strun.m_bg_color;
	if (ri && ri->is_chromakey_specified()) {
		if (color_t_in_range (text_color, chroma_low, chroma_high))
			alpha_media = alpha_chroma;
		if (color_t_in_range (bg_color, chroma_low, chroma_high))
			alpha_media_bg = alpha_chroma;
	}
	QColor qt_color(redc(text_color), greenc(text_color), bluec(text_color));
	QColor qt_bg_color(redc(bg_color), greenc(bg_color), bluec(bg_color));

	if (m_blending) {
		tx_paint.begin( tx_pixmap );
		if ( ! strun.m_bg_transparent) {
			tx_paint.setBackgroundMode(Qt::OpaqueMode);
			tx_paint.setBackgroundColor(m_qt_transparent);
		}
		if ( ! strun.m_transparent) {
			tx_paint.setPen(qt_color);
		}

		if (bg_pixmap) {
			bg_paint.begin( bg_pixmap );
			bg_paint.setFont(m_font);
			bg_paint.setBrush(qt_bg_color);
			bg_paint.setPen(Qt::NoPen);
			bg_paint.drawRect(0,0,W,H);
			bg_paint.setPen(qt_color);
			// Qt::AlignLeft|Qt::AlignTop
			// Qt::AlignAuto
			bg_paint.drawText(0,0,W,H, Qt::AlignAuto, strun.m_data);
			bg_paint.flush();
			bg_paint.end();
		}
	} else {
		// if possible, paint directly into the final destination
		tx_paint.begin( m_window->get_ambulant_pixmap() );
	}
	tx_paint.setFont(m_font);
	if ( ! strun.m_bg_transparent) {
		tx_paint.setBrush(qt_bg_color);
		tx_paint.setPen(Qt::NoPen);
		tx_paint.drawRect(L,T,W,H);
	}
	int flags = Qt::AlignAuto;
	tx_paint.setPen(qt_color);
	if (m_blending)
		tx_paint.drawText(0,0,W,H,flags, strun.m_data);
	else {
		tx_paint.drawText(L,T,W,H, flags, strun.m_data);
	}
	tx_paint.flush();
	tx_paint.end();

	if (m_blending) {
		QImage tx_image = tx_pixmap->convertToImage();
		QImage screen_img = m_window->get_ambulant_pixmap()->convertToImage();
		AM_DBG DUMPIMAGE(&screen_img, "sc");

		AM_DBG DUMPIMAGE(&tx_image, "tx");

		lib::rect rct0 (lib::point(0, 0), lib::size(W, H));

		if (bg_pixmap) {
			QImage bg_image = bg_pixmap->convertToImage();
			AM_DBG DUMPIMAGE(&bg_image, "bg");
			qt_image_blend (screen_img, rct, bg_image, rct0,
					alpha_media_bg, 0.0,
//XX					chroma_low, chroma_high);
					bg_color, bg_color);
			delete bg_pixmap;
		}
		qt_image_blend (screen_img, rct, tx_image, rct0,
				alpha_media, 0.0,
//XX				chroma_low, chroma_high);
				text_color, text_color);
		// see optimization suggestion above.
		// also, it should not be necessary to copy
		// the whole image, only the rect (L,T,W,H)
		// is sufficient (copyBlt), blend it, then
		// bitBlt() it back after blending.
		QPixmap new_pixmap(W,H);
		new_pixmap.convertFromImage(screen_img);
		AM_DBG DUMPPIXMAP(&new_pixmap, "nw");
		bitBlt(m_window->get_ambulant_pixmap(), L, T,
			&new_pixmap, L, T, W, H);
		AM_DBG DUMPPIXMAP(m_window->get_ambulant_pixmap(), "rs");
		delete tx_pixmap;
	}
}

void
gui::qt::qt_smiltext_renderer::_qt_smiltext_set_font(const smil2::smiltext_run& strun) {
	const char *fontname = strun.m_font_families[0].c_str();
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
	if (m_blending)
		m_font.setStyleStrategy(QFont::NoAntialias);
}

void
gui::qt::qt_smiltext_renderer::redraw_body(const lib::rect& dirty, common::gui_window *window) {
	m_lock.enter();
	m_window = (ambulant_qt_window*) window;

	lib::rect r = dirty;

	// Translate smiltext region dirty rect. to final viewport coordinates
	//lib::point pt = m_dest->get_global_topleft();
	//r.translate(pt);

	m_layout_engine.redraw(r);
	bool finished = m_layout_engine.is_finished();
	m_lock.leave();
	if (finished)
		m_context->stopped(m_cookie);

}
