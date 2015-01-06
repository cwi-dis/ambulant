// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
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

/* 
 * @$Id: sdl_ttf_smiltext.cpp,v 1.28.4.3 2010/04/09 12:54:46 keesblom Exp $ 
 */
#ifdef  WITH_SDL_IMAGE
#ifdef  WITH_SDL_TTF
// TBD: media opacity, media background opacity, chroma keying, font family selection, offscreen rendering
#include "ambulant/gui/SDL/sdl_ttf_smiltext.h"

#include "ambulant/common/region_info.h"
#include "ambulant/lib/logger.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace sdl {

extern const char sdl_ttf_smiltext_playable_tag[] = "smilText";
extern const char sdl_ttf_smiltext_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererSdl");
extern const char sdl_ttf_smiltext_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererSmilText");

common::playable_factory *
create_sdl_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
    smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSdl"), true);
    smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSmilText"), true);
	return new common::single_playable_factory<
        sdl_ttf_smiltext_renderer, 
        sdl_ttf_smiltext_playable_tag, 
        sdl_ttf_smiltext_playable_renderer_uri,
        sdl_ttf_smiltext_playable_renderer_uri2,
        sdl_ttf_smiltext_playable_renderer_uri2>(factory, mdp);
} // create_sdl_ttf_smiltext_playable_factory

sdl_ttf_smiltext_renderer::sdl_ttf_smiltext_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp)
  :
#ifdef TBD
	m_sdl_ttf_transparent(redc(SDL_TTF_TRANSPARENT_COLOR),greenc(SDL_TTF_TRANSPARENT_COLOR),bluec( SDL_TTF_TRANSPARENT_COLOR)),
	m_sdl_ttf_alternative(redc(SDL_TTF_ALTERNATIVE_COLOR),greenc(SDL_TTF_ALTERNATIVE_COLOR),bluec( SDL_TTF_ALTERNATIVE_COLOR)),
#endif//TBD
	m_bgopacity(1.0),
	m_blending(false),
	m_ttf_font_filename(NULL),
	m_ttf_font(NULL),
	m_text_size (DEFAULT_FONT_HEIGHT),
	m_ttf_style (TTF_STYLE_NORMAL),
	sdl_renderer<renderer_playable>(context, cookie, node, evp, fp, mdp),
	m_layout_engine(smil2::smiltext_layout_engine(node, evp, this, this, true))
{
	AM_DBG lib::logger::get_logger()->debug("sdl_ttf_smiltext_renderer(0x%x)", this);
	TTF_Init();
#ifdef	TBD
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append);
#endif//TBD
	for (int i = 0; i < N_FONT_SIZES; i++) {
		m_ttf_fonts[i] = 0;
	}
	char* default_font_file = (char *) DEFAULT_FONT_FILE1;
	_open_font(default_font_file, DEFAULT_FONT_HEIGHT, &m_ttf_font);
#ifndef ANDROID
	default_font_file = (char *) DEFAULT_FONT_FILE2;
	if (m_ttf_font == NULL) {
		_open_font(default_font_file, DEFAULT_FONT_HEIGHT, &m_ttf_font);
	}
#endif// ! ANDROID
	if (m_ttf_font == NULL) {
  		AM_DBG lib::logger::get_logger()->error("TTF_OpenFont(%s, %d): %s", default_font_file, m_text_size, TTF_GetError());
		return;
	}
	m_text_size = 13;
	m_ttf_fonts[m_text_size - MIN_FONT_SIZE] = m_ttf_font;
} // sdl_ttf_smiltext_renderer

sdl_ttf_smiltext_renderer::~sdl_ttf_smiltext_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~sdl_ttf_smiltext_renderer(0x%x)", this);
	m_lock.enter();
	for (int i = 0; i < N_FONT_SIZES; i++) {
		_close_font(&m_ttf_fonts[i]);
	}
	TTF_Quit();
	m_lock.leave();
} // ~sdl_ttf_smiltext_renderer

void
sdl_ttf_smiltext_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("sdl_ttf_smiltext_renderer::start(0x%x)", this);
		
	sdl_renderer<common::renderer_playable>::start(t);
	m_lock.enter();
//JNK?	m_epoch = m_event_processor->get_timer()->elapsed();
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
			compute_chroma_range(chromakey, chromakeytolerance,
					     &chroma_low, &chroma_high);
		}
	}

	if ( ! (alpha_media == 1.0 && alpha_media_bg == 1.0 && alpha_chroma == 1.0) ) {
		m_blending = true;
	}
	m_context->started(m_cookie);
	m_lock.leave();
} // start

bool 
sdl_ttf_smiltext_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("sdl_ttf_smiltext_renderer::stop(0x%x)", this);
	m_lock.enter();
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	assert(m_context);
	m_context->stopped(m_cookie);
	m_lock.leave();
	return true;
} // stop

void
sdl_ttf_smiltext_renderer::marker_seen(const char *name)
{
	m_lock.enter();
	m_context->marker_seen(m_cookie, name);
	m_lock.leave();
} // marker_seen

void
sdl_ttf_smiltext_renderer::smiltext_stopped() {
	m_context->stopped(m_cookie);
} // smiltext_stopped

void
sdl_ttf_smiltext_renderer::smiltext_changed() {
	m_layout_engine.smiltext_changed();
	m_dest->need_redraw();
} // smiltext_changed

smil2::smiltext_metrics
sdl_ttf_smiltext_renderer::get_smiltext_metrics(const smil2::smiltext_run& strun) {
	int ascent = 0, descent = 0, height = 0, width = 0, line_spacing = 0, word_spacing = 0;

	if (strun.m_data.length() != 0 && m_ttf_font != NULL) {
		_set_font_style (strun);
		ascent	= TTF_FontAscent(m_ttf_font);
		descent	= TTF_FontDescent(m_ttf_font);
		line_spacing = TTF_FontLineSkip(m_ttf_font);
		TTF_SizeText(m_ttf_font, strun.m_data.c_str(), &width, &height);
	}
// SDL_ttf does not support multiline rendering (doc. TTF_FontHeight()), add 1px.
	return smil2::smiltext_metrics(ascent, abs(descent), height, width, line_spacing);
}

const lib::rect&
sdl_ttf_smiltext_renderer::get_rect() {
	return m_dest->get_rect();
} // get_rect

void
sdl_ttf_smiltext_renderer::render_smiltext(const smil2::smiltext_run& strun, const lib::rect& r) {

	AM_DBG lib::logger::get_logger()->debug("sdl_ttf_smiltext_render(): command=%d data=%s color=0x%x bg_color=0x%x",strun.m_command,strun.m_data.c_str()==NULL?"(null)":strun.m_data.c_str(),strun.m_color,strun.m_bg_color);
	if (m_ttf_font == NULL) {
		return;
	}
	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		alpha_media_bg = ri->get_mediabgopacity();
#ifdef TBD
		m_bgopacity = ri->get_bgopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance,
					     &chroma_low, &chroma_high);   
		}
#endif//TBD
	}
#ifdef TBD
	// prepare for blending
	QPixmap* bg_pixmap = NULL;
	QPixmap* tx_pixmap = NULL;
#endif//TBD
	lib::rect rct(r); // rct encloses leading blank and word
	AM_DBG lib::logger::get_logger()->debug("sdl_ttf_smiltext_render(): data=%s r=L=%d,T=%d,W=%d,H=%d", strun.m_data.c_str(),r.x,r.y,r.w,r.h);
	const lib::point p = m_dest->get_global_topleft();
	int L = rct.left()+p.x,
	    T = rct.top()+p.y,
	    W = rct.width(),
	    H = rct.height();

	if (W == 0 || H == 0)
		return; // cannot render anything
	AM_DBG lib::logger::get_logger()->debug("sdl_ttf_smiltext_render():p=(%d,%d)  L=%d,T=%d,W=%d,H=%d",p.x,p.y,L,T,W,H);
#ifdef TBD
	if (m_blending) {
		// create pixmaps for blending
		if ( ! strun.m_bg_transparent) {
			/*** optimization suggestion:
			     maybe it is possible for blending to render
			     everything in one extra pixmap, then first
			     blend with text background color, next blend
			     with text color, without creating bg_ pixmap
			     and new_pixmap (i.e. directly on the screen)
			     This should result in far less round trips to
			     the X-server.
			***/
		        bg_pixmap = new QPixmap(W,H);
			assert( bg_pixmap );
			bg_pixmap->fill(strun.m_bg_color);
		}
		tx_pixmap = new QPixmap(W,H);
		assert( tx_pixmap);
		tx_pixmap->fill(m_sdl_ttf_transparent);
		if ( ! m_blending)
			alpha_media = alpha_chroma = 1.0;
	}

	QPainter tx_paint, bg_paint;
#endif//TBD
	lib::color_t text_color = strun.m_color;
	lib::color_t bg_color = strun.m_bg_color;
	if (ri && ri->is_chromakey_specified()) {
		if (color_t_in_range (text_color, chroma_low, chroma_high))
			alpha_media = alpha_chroma;
		if (color_t_in_range (bg_color, chroma_low, chroma_high))
			alpha_media_bg = alpha_chroma;
	}
	SDL_Color sdl_ttf_color = {redc(text_color), greenc(text_color), bluec(text_color)};
	SDL_Color sdl_ttf_bg_color = {redc(bg_color), greenc(bg_color), bluec(bg_color)};
	_set_font_style(strun);

#ifdef TBD	
	if (m_blending) {
		tx_paint.begin( tx_pixmap );
		if ( ! strun.m_bg_transparent) {
			tx_paint.setBackgroundMode(Sdl_Ttf::OpaqueMode);
			tx_paint.setBackgroundColor(m_sdl_ttf_transparent);
		}
		if ( ! strun.m_transparent) {
			tx_paint.setPen(sdl_ttf_color);
		}
		
		if (bg_pixmap) {
			bg_paint.begin( bg_pixmap );
			bg_paint.setFont(m_font);
			bg_paint.setBrush(sdl_ttf_bg_color);
			bg_paint.setPen(Sdl_Ttf::NoPen);
			bg_paint.drawRect(0,0,W,H);
			bg_paint.setPen(sdl_ttf_color);
			// Sdl_Ttf::AlignLeft|Sdl_Ttf::AlignTop
			// Sdl_Ttf::AlignAuto
			bg_paint.drawText(0,0,W,H, Sdl_Ttf::AlignAuto, strun.m_data);
			bg_paint.flush();
			bg_paint.end();
		}
	} else {
		// if possible, paint directly into the final destination
		tx_paint.begin( m_window->get_ambulant_pixmap() );
	}	
	tx_paint.setFont(m_font);
	if ( ! strun.m_bg_transparent) {
		tx_paint.setBrush(sdl_ttf_bg_color);
		tx_paint.setPen(Sdl_Ttf::NoPen);
		tx_paint.drawRect(L,T,W,H);
	}
	int flags = Sdl_Ttf::AlignAuto;
	tx_paint.setPen(sdl_ttf_color);
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
			sdl_ttf_image_blend (screen_img, rct, bg_image, rct0, 
					alpha_media_bg, 0.0,
//XX					chroma_low, chroma_high);
					bg_color, bg_color);
			delete bg_pixmap;
		}
		sdl_ttf_image_blend (screen_img, rct, tx_image, rct0, 
				alpha_media, 0.0,
//XX				chroma_low, chroma_high);
				text_color, text_color);
		/*** see optimization suggestion above.
		     also, it should not be necessary to copy
		     the whole image, only the rect (L,T,W,H)
		     is sufficient (copyBlt), blend it, then
		     bitBlt() it back after blending.
		***/
		QPixmap new_pixmap(W,H);
		new_pixmap.convertFromImage(screen_img);
		AM_DBG DUMPPIXMAP(&new_pixmap, "nw");
		bitBlt(m_window->get_ambulant_pixmap(), L, T,
		       &new_pixmap, L, T, W, H);	
		AM_DBG DUMPPIXMAP(m_window->get_ambulant_pixmap(), "rs");
		delete tx_pixmap;
	}
#endif//TBD
	m_text_color = text_color;
	m_text_bg_color = bg_color;
	SDL_Rect sdl_dst_rect = { L, T, W, H};
	SDL_Color sdl_text_color = {redc(m_text_color),greenc(m_text_color),bluec(m_text_color), alpha_media*255};
	SDL_Surface* text_surface = TTF_RenderText_Blended(m_ttf_font, strun.m_data.c_str(), sdl_text_color);
	if (text_surface == NULL) {
        AM_DBG lib::logger::get_logger()->error("%s(%p): Failed rendering %s: %s ",
   					__PRETTY_FUNCTION__,this, strun.m_data.c_str(), TTF_GetError());
	} else {
		Uint32 bg_alpha = alpha_media_bg*255; //  from region_info;
		sdl_ambulant_window* saw = m_window->get_sdl_ambulant_window();
		SDL_Renderer* renderer = saw->get_sdl_renderer(); 
		if (bg_color != 0) {
			SDL_SetRenderDrawColor (renderer, redc(bg_color), greenc(bg_color), bluec(bg_color), bg_alpha);
			SDL_RenderFillRect(renderer, &sdl_dst_rect);
		}
		SDL_Rect sdl_src_rect = { 0, 0, text_surface->w, text_surface->h };
//		lib::rect clip_rect = rct & m_window->get_bounds();
//		SDL_Rect sdl_clip_rect =  SDL_Rect_from_ambulant_rect(clip_rect);
//		saw->dump_sdl_surface (text_surface, "txt");
		saw->copy_to_sdl_surface (text_surface, &sdl_src_rect, &sdl_dst_rect, alpha_media*255); //, &sdl_clip_rect);
		SDL_FreeSurface (text_surface);
	}
} // render_smiltext


void
sdl_ttf_smiltext_renderer::_set_font_size(int font_size)
{
	if (m_text_size == font_size
		|| font_size < MIN_FONT_SIZE || font_size > MAX_FONT_SIZE) {
		return;
	}
	TTF_Font** ttfsp = &m_ttf_fonts [font_size - MIN_FONT_SIZE];
	if (*ttfsp == NULL) {
		_open_font (font_size, ttfsp);
		if (*ttfsp == NULL) {
			return;
		}
	}
	m_ttf_font = *ttfsp;
	m_text_size = font_size;
}

void
sdl_ttf_smiltext_renderer::_set_font_style(const smil2::smiltext_run& strun) {
#ifdef TBD
	const char *fontname = strun.m_font_families[0].c_str();
	m_font = QFont(QApplication::font());
	if (fontname) {
                m_font.setFamily(fontname);
	} else {
	        m_font.setFamily(m_font.defaultFamily());
	}
#endif//TBD
	_set_font_size (strun.m_font_size);
	int ttf_font_style = TTF_STYLE_NORMAL;
	switch(strun.m_font_style) {
		default:
		case smil2::sts_normal:
		// use default style
			break;
		case smil2::sts_italic:
			ttf_font_style |= TTF_STYLE_ITALIC;
			break;
		case smil2::sts_oblique:
		case smil2::sts_reverse_oblique:
		// no (reverse) oblique fonts available in Sdl_Ttf 3.3
			ttf_font_style |= TTF_STYLE_ITALIC;
			break;
	}
	switch(strun.m_font_weight) {
		default:
		case smil2::stw_normal:
			break;
		case smil2::stw_bold:
			ttf_font_style |= TTF_STYLE_BOLD;
			break;
	}
	TTF_SetFontStyle(m_ttf_font, ttf_font_style);
//TBD	TTF_SetTextSize(m_ttf_font, strun.m_font_size);
//TBD	if (m_blending)
//TBD		m_font.setStyleStrategy(QFont::NoAntialias);
} // _sdl_ttf_smiltext_set_font

void
sdl_ttf_smiltext_renderer::_open_font(const char* font_filename, int pt_size, TTF_Font** fp)
{
	assert(fp != NULL && font_filename != NULL && pt_size > 0); 
	if (m_ttf_font_filename != NULL
		&& strcmp(font_filename, m_ttf_font_filename) != 0) {
		_close_font(fp);
	}
	TTF_Font* ttf = TTF_OpenFont(font_filename, pt_size*1.3);
	if (ttf != NULL) {
		m_ttf_font_filename = (char*) font_filename;
		TTF_SetFontStyle(ttf, m_ttf_style);
		TTF_SetFontOutline(ttf, 0);
		TTF_SetFontKerning(ttf, 1);
		TTF_SetFontHinting(ttf, (int)TTF_HINTING_NORMAL);
	}
	*fp = ttf;
}

void
sdl_ttf_smiltext_renderer::_open_font(int pt_size, TTF_Font** fp)
{
	if (fp != NULL) {
		_open_font (m_ttf_font_filename, pt_size, fp);
	}
}

void
sdl_ttf_smiltext_renderer::_close_font(TTF_Font** fp)
{
	if (fp == NULL || *fp == NULL) {
		return;
	}
	TTF_CloseFont(*fp);
	if (*fp == m_ttf_font) {
		m_ttf_font = NULL;
	}
	*fp = NULL;
}

void
sdl_ttf_smiltext_renderer::redraw_body(const lib::rect& dirty, common::gui_window *window) {
	m_lock.enter();
	m_window = (ambulant_sdl_window*) window;

	lib::rect r = dirty;
	
	// Translate smiltext region dirty rect. to final viewport coordinates 
	//lib::point pt = m_dest->get_global_topleft();
	//r.translate(pt);
		
	m_layout_engine.redraw(r);
	bool finished = m_layout_engine.is_finished();
	m_lock.leave();
	if (finished)
		m_context->stopped(m_cookie);

} // redraw_body

} // namespace sdl

} // namespace gui

} //namespace ambulant

#endif //WITH_SDL_TTF
#endif // WITH_SDL_IMAGE
