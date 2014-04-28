// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

#if defined(WITH_SDL2)

//X #include "ambulant/gui/SDL/sdl_includes.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_smiltext.h"
//X #include "ambulant/gui/SDL/sdl_util.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"
#if ! defined(WITH_SDL_PANGO)
#include "ambulant/gui/none/none_area.h"
#endif// ! defined(WITH_SDL_PANGO)
#if defined(WITH_SDL_TTF)
#include "SDL_ttf.h"
#endif

#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG  if (0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace sdl {

extern const char sdl_smiltext_playable_tag[] = "smilText";
extern const char sdl_smiltext_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererSdl");
extern const char sdl_smiltext_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererSmilText");

common::playable_factory *
create_sdl_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSdl"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSmilText"), true);
#define SDL_SMILTEXT_RENDERER sdl_smiltext_renderer 
#ifdef  WITH_SDL_TTF
	TTF_Init();
#endif // WITH_SDL_TTF
#ifndef WITH_SDL_PANGO
	lib::logger::get_logger()->trace("No full %s renderer available using SDL_Pango", "smilText");
#endif// ! defined(WITH_SDL_PANGO)
	return new common::single_playable_factory<
		SDL_SMILTEXT_RENDERER,
		sdl_smiltext_playable_tag,
		sdl_smiltext_playable_renderer_uri,
		sdl_smiltext_playable_renderer_uri2,
		sdl_smiltext_playable_renderer_uri2>(factory, mdp);
}

sdl_smiltext_renderer::sdl_smiltext_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	sdl_renderer<renderer_playable>(context, cookie, node, evp, fp, mdp),
	m_engine(smil2::smiltext_engine(node, evp, this, false)),
	m_params(m_engine.get_params()),
	m_epoch(0),
// for SDL_Pango
#ifdef WITH_SDL_PANGO
	m_sdl_pango_context(NULL),
	m_pango_attr_list(NULL),
	m_pango_context(NULL),
	m_pango_layout(NULL),
	m_bg_pango_attr_list(NULL),
	m_bg_layout(NULL),
#endif//WITH_SDL_PANGO
#ifdef  WITH_SDL_TTF
	m_ttf_font (NULL),
	m_text_size (DEFAULT_FONT_HEIGHT),
	m_ttf_style (TTF_STYLE_NORMAL),
	m_layout_engine(smil2::smiltext_layout_engine(node, evp, this, this, true)),
#endif//WITH_SDL_TTF
	m_transparent(SDL_TRANSPARENT_COLOR),
	m_alternative(SDL_ALTERNATIVE_COLOR),
	m_alpha_media(1.0),
	m_alpha_media_bg(1.0),
	m_alpha_chroma(1.0),
	m_chroma_low(0x000000),		//black
	m_chroma_high(0xFFFFFF),	//white
	m_align(smil2::sta_left),
	m_writing_mode(smil2::stw_lr_tb),// Left to Right, Top to Bottom
	m_needs_conditional_space(false),
	m_needs_conditional_newline(false),
	m_wrap(true),
	m_is_changed(false),
	m_motion_done(false),
	m_start(lib::point(0,0)),
	m_origin(lib::point(0,0)),
	m_log_rect(m_origin, lib::size(0,0))
{
#ifdef	TBD
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append);
#endif//TBD
    m_ttf_font = TTF_OpenFont(DEFAULT_FONT_FILE1, m_text_size*1.1);
	if (m_ttf_font == NULL) {
  		AM_DBG lib::logger::get_logger()->error("TTF_OpenFont(%s, %d): %s", DEFAULT_FONT_FILE1, m_text_size*1.1, TTF_GetError());
		return;
	}
	TTF_SetFontStyle(m_ttf_font, m_ttf_style);
	TTF_SetFontOutline(m_ttf_font, 0);
	TTF_SetFontKerning(m_ttf_font, 1);
	TTF_SetFontHinting(m_ttf_font, (int)TTF_HINTING_NORMAL);
}

sdl_smiltext_renderer::~sdl_smiltext_renderer()
{
	m_engine.lock();
#ifdef WITH_SDL_PANGO
	if ( m_pango_attr_list != NULL) {
		pango_attr_list_unref( m_pango_attr_list);
		m_pango_attr_list = NULL;
	}
	if (m_pango_context != NULL) {
		g_object_unref (G_OBJECT (m_pango_context));
		m_pango_context = NULL;
	}
	if (m_pango_layout != NULL) {
		g_object_unref(G_OBJECT (m_pango_layout));
		m_pango_layout = NULL;
	}
	if ( m_bg_pango_attr_list != NULL) {
		pango_attr_list_unref(m_bg_pango_attr_list);
		m_bg_pango_attr_list = NULL;
	}
	if (m_bg_layout) {
		g_object_unref(m_bg_layout);
		m_bg_layout = NULL;
	}
#endif
	m_engine.unlock();
}

void
sdl_smiltext_renderer::start(double t)
{
  	m_lock.enter();

	gui::sdl::sdl_renderer<common::renderer_playable>::start(t);
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	m_motion_done = false;
	m_layout_engine.start(t);
	renderer_playable::start(t);
	m_layout_engine.set_dest_rect(m_rect = m_dest->get_rect());
	m_context->started(m_cookie);

	m_lock.leave();
}

void
sdl_smiltext_renderer::seek(double t)
{
	m_lock.enter();
	m_engine.seek(t);
	//renderer_playable::seek(t);
	m_lock.leave();
}

bool
sdl_smiltext_renderer::stop()
{
	m_lock.enter();
	m_engine.stop();
	renderer_playable::stop();
	m_lock.leave();
	m_context->stopped(m_cookie);
	return true;
}

void
sdl_smiltext_renderer::marker_seen(const char *name)
{
	m_lock.enter();
	m_context->marker_seen(m_cookie, name);
	m_lock.leave();
}


void
sdl_smiltext_renderer::smiltext_changed()
{
	m_engine.lock();
#ifdef WITH_SDL_TTF
	m_layout_engine.smiltext_changed();
#endif//WITH_SDL_TTF
	if (m_engine.is_changed()) {
		m_is_changed = true;
	}
	m_engine.unlock();
	m_dest->need_redraw(); // cannot be called while locked
}

void
sdl_smiltext_renderer::_sdl_smiltext_changed()
{
AM_DBG lib::logger::get_logger()->debug("sdl_smiltext_changed(%p)",this);
#ifdef WITH_SDL_PANGO
#ifdef  GDK_PANGO
	if ( ! m_pango_context) {
		// initialize the pango context, layout...
		m_pango_context = gdk_pango_context_get();
		PangoLanguage* language = sdl_get_default_language();
		pango_context_set_language (m_pango_context, language);
		pango_context_set_base_dir (m_pango_context, PANGO_DIRECTION_LTR);
		const common::region_info *ri = m_dest->get_info();
		if (ri) {
			m_alpha_media = ri->get_mediaopacity();
			m_alpha_media_bg = ri->get_mediabgopacity();
			m_alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance, &m_chroma_low, &m_chroma_high);
		}
#ifndef WITH_SDL_ANTI_ALIASING
		if (m_alpha_media != 1.0 || m_alpha_media_bg != 1.0 || m_alpha_chroma != 1.0) {
			cairo_font_options_t* cairo_font_options = cairo_font_options_create();
			//
			// anti-aliasing by pango/cairo is disabled
			// when blending is necessary, because this
			// sometimes results in ugly glyphs.
			//
			cairo_font_options_set_antialias (cairo_font_options, CAIRO_ANTIALIAS_NONE);
			pango_cairo_context_set_font_options (m_pango_context, cairo_font_options);
			cairo_font_options_destroy (cairo_font_options);
		}
#endif // WITH_SDL_ANTI_ALIASING
		m_writing_mode = m_engine.begin()->m_writing_mode;
		switch (m_writing_mode) {
		default:
		case smil2::stw_lr_tb:
			pango_context_set_base_dir (m_pango_context, PANGO_DIRECTION_LTR);
			break;
		case smil2::stw_rl_tb:
			pango_context_set_base_dir (m_pango_context, PANGO_DIRECTION_RTL);
			break;
		}
	}
	if ( ! m_pango_layout) {
		m_pango_layout = pango_layout_new (m_pango_context);
		pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_LEFT);
	}
#else //SDL_PANGO
	if (m_sdl_pango_context == NULL) {
		// initialize the pango context, layout...
		m_sdl_pango_context = SDLPango_CreateContext();
		m_pango_context = *(PangoContext**) m_sdl_pango_context;
		PangoLanguage* language = pango_language_get_default();
		SDLPango_SetLanguage (m_sdl_pango_context, pango_language_to_string (language));
		SDLPango_SetBaseDirection (m_sdl_pango_context, SDLPANGO_DIRECTION_LTR);
//TBD		font_desc = SDLPango_GetPangoFontDescription(sdl_pango_context);
		m_writing_mode = m_engine.begin()->m_writing_mode;
		switch (m_writing_mode) {
		default:
		case smil2::stw_lr_tb:
		    SDLPango_SetBaseDirection (m_sdl_pango_context, SDLPANGO_DIRECTION_LTR);
			break;
		case smil2::stw_rl_tb:
		    SDLPango_SetBaseDirection (m_sdl_pango_context, SDLPANGO_DIRECTION_RTL);
			break;
		}
#ifdef  DUMP_SCREEN
// When screen dumps are used (for debugging), a white background is handy, otherwise you'll see nothing
		SDLPango_Matrix color_matrix = *MATRIX_WHITE_BACK;
		SDLPango_SetDefaultColor (m_sdl_pango_context, &color_matrix);
#endif//DUMP_SCREEN
	}
	if (m_pango_layout == NULL) {
		m_pango_layout = SDLPango_GetPangoLayout(m_sdl_pango_context);
		pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_LEFT);
	}
#endif//defined(WITH_SDL2) && defined(WITH_SDL_PANGO)
	if ( ! m_pango_attr_list)
		m_pango_attr_list = pango_attr_list_new();
#ifdef  TBD
	if ( ! m_bg_layout	&& (m_alpha_media != 1.0  || m_alpha_media_bg != 1.0 || m_alpha_chroma != 1.0)) {
		// prepare for blending: layout is setup twice:
		// m_bg_layout has textColor as m_transparent
		// m_pango_layout has textBackGroundColor as m_transparent
		// when blending, pixels in m_transparent color are ignored
		m_bg_layout = pango_layout_new(m_pango_context);
		pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_LEFT);
		m_bg_pango_attr_list = pango_attr_list_new();
	}
#endif//TBD
#endif //WITH_SDL_PANGO
	if (m_is_changed) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;
		m_is_changed = false;
		if (m_engine.is_cleared()) {
			// Completely new text. Clear our copy and render everything.
			m_text_storage = "";
			i = m_engine.begin();
			smil2::smiltext_align align = i->m_align;
			if (align != m_align) {
#ifdef WITH_SDL_PANGO
				switch (align) {
				default:
				case smil2::sta_left:
					pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_LEFT);
#ifdef  TBD
					if (m_bg_layout)
						pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_LEFT);
#endif//TBD
					break;
				case smil2::sta_center:
					pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_CENTER);
#ifdef  TBD
					if (m_bg_layout)
						pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_CENTER);
#endif//TBD
					break;
				case smil2::sta_right:
					pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_RIGHT);
#ifdef  TBD
					if (m_bg_layout)
						pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_RIGHT);
#endif//TBD
					break;
				}
#endif // WITH_SDL_PANGO
				m_align = align;
			}
			m_needs_conditional_space = false;
			m_needs_conditional_newline = false;
			if ( ! i->m_wrap || m_params.m_mode == smil2::stm_crawl)
				m_wrap = false;
		} else {
			// Only additions. Don't clear and only copy the new stuff.
			i = m_engine.newbegin();
		}
		while (i != m_engine.end()) {
			// Add the new characters
			int start_index = m_text_storage.size();
			switch (i->m_command) {
			default:
				assert(0);
				break;
			case smil2::stc_break:
				if (m_params.m_mode == smil2::stm_crawl) {
					m_text_storage += " ";
				} else	{
					m_text_storage += "\n";
				}
				m_needs_conditional_space = false;
				m_needs_conditional_newline = false;
				break;
			case smil2::stc_condbreak:
				if (m_needs_conditional_newline) {
					if (m_params.m_mode == smil2::stm_crawl) {
						m_text_storage += " ";
					} else	{
						m_text_storage += "\n\n";
					}
					m_needs_conditional_space = false;
					m_needs_conditional_newline = false;
				}
				break;
			case smil2::stc_condspace:
				if (m_needs_conditional_space) {
					m_text_storage += " ";
					m_needs_conditional_newline = true;
					m_needs_conditional_space = false;
				}
				break;
			case smil2::stc_data:
				char lastch = *(i->m_data.rbegin());
				if (lastch == '\r' || lastch == '\n' || lastch == '\f' || lastch == '\v') {
					m_needs_conditional_newline = false;
					m_needs_conditional_space = false;
				} else if (lastch == ' ' || lastch == '\t') {
					m_needs_conditional_newline = true;
					m_needs_conditional_space = false;
				} else {
					m_needs_conditional_newline = true;
					m_needs_conditional_space = true;
				}
				if (m_params.m_mode != smil2::stm_crawl) {
					m_text_storage +=  i->m_data;
					break;
				}
				/* crawling: remove embedded newlines */
				std::string newdata = i->m_data;
				std::string::size_type nl = 0;
				while (nl != std::string::npos) {
					nl = newdata.find("\n", nl);
					newdata[nl] = ' ';
				}
				m_text_storage += newdata;
				break;
			}
			// Set font attributes
#ifdef WITH_SDL_TTF
      m_text_size = i->m_font_size;
      m_text_font = i->m_font_families[0].c_str();
      m_text_color = i->m_color == m_transparent ? m_alternative : i->m_color;
      switch (i->m_font_style) {
      case smil2::sts_reverse_oblique:
      case smil2::sts_oblique:
      case smil2::sts_italic:
        m_ttf_style |= TTF_STYLE_ITALIC;
        break;
      default: break;
      }
      if (i->m_font_weight == smil2::stw_bold) {
        m_ttf_style |= TTF_STYLE_BOLD;
      }
      //lib::logger::get_logger()->debug("Object %p : text size %.2f and font %s set for %s", this, m_text_size, m_text_font, m_text_storage.c_str());    
#endif

#ifdef WITH_SDL_PANGO
			_sdl_set_font_attr(m_pango_attr_list,
				i->m_font_families[0].c_str(),
				i->m_font_style,
				i->m_font_weight,
				i->m_font_size,
				start_index,
				m_text_storage.size());
#ifdef  TBD
			if (m_bg_pango_attr_list) {
				_sdl_set_font_attr(m_bg_pango_attr_list,
					i->m_font_families[0].c_str(),
					i->m_font_style,
					i->m_font_weight,
					i->m_font_size,
					start_index,
					m_text_storage.size());
			}
#endif//TBD
			// text foreground/background color settings.
			if ( ! i->m_transparent) {
				// Set foreground color attribute
				color_t fg_color = i->m_color == m_transparent ? m_alternative : i->m_color;
				_sdl_set_color_attr(
					m_pango_attr_list,
					fg_color,
					pango_attr_foreground_new,
					start_index,
					m_text_storage.size());
#ifdef  TBD
				if (m_bg_layout) {
					_sdl_set_color_attr(
						m_bg_pango_attr_list,
						m_transparent,
						pango_attr_foreground_new,
						start_index,
						m_text_storage.size());
				}
#endif//TBD
			}
#ifdef  TBD
			if ( ! i->m_bg_transparent) {
				// Set background color attribute
				// Select altenative color for m_transparent
				color_t bg_color = i->m_bg_color == m_transparent ? m_alternative : i->m_bg_color;
				_sdl_set_color_attr(m_pango_attr_list,
					m_bg_layout ?
					m_transparent : bg_color,
					pango_attr_background_new,
					start_index, m_text_storage.size());
				if (m_bg_layout) {
					_sdl_set_color_attr(m_bg_pango_attr_list,
						bg_color,
						pango_attr_background_new,
						start_index,
						m_text_storage.size());
				}
			}
#endif//TBD
			// Set the foreground attributes and text
			pango_layout_set_text(m_pango_layout, m_text_storage.c_str(), -1);
//			SDLPango_SetText (m_sdl_pango_context, m_text_storage.c_str(), -1);//m_text_size);
			pango_layout_set_attributes(m_pango_layout, m_pango_attr_list);
			pango_layout_context_changed(m_pango_layout);

#ifdef  TBD
			if (m_bg_layout) {
				// Set the background attributes and text
				pango_layout_set_attributes(m_bg_layout, m_bg_pango_attr_list);
				pango_layout_set_text(m_bg_layout, m_text_storage.c_str(), -1);
				pango_layout_context_changed(m_bg_layout);
			}
#endif//TBD
#endif //WITH_SDL_PANGO
			i++;
		}
		m_engine.done();
	}
	bool finished = m_engine.is_finished();
AM_DBG	lib::logger::get_logger()->debug("sdl_smiltext_changed(%p), m_text_storage=%s",this,m_text_storage.c_str());
	if (finished)
		m_context->stopped(m_cookie);
}
#ifdef  TBD
#endif//TBD

#if defined(WITH_SDL_PANGO)
void
sdl_smiltext_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_engine.lock();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("sdl_smiltext_renderer.redraw(%p, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	bool changed = m_is_changed;
	if (m_is_changed) {
		// compute the changes
		_sdl_smiltext_changed();
		m_is_changed = false;
	}
#ifdef  TBD
#endif//TBD
	PangoRectangle ink_rect;
	PangoRectangle log_rect;

	if (changed) {
		// get the extents of the lines
		pango_layout_set_width(m_pango_layout, m_wrap ? r.w*PANGO_SCALE : -1);
#ifdef  TBD
		if (m_bg_layout)
			pango_layout_set_width(m_bg_layout, m_wrap ? r.w*PANGO_SCALE : -1);
#endif//TBD
		// get extents of first line (contains space for all lines in layout)
		PangoLayoutIter* iter_p = pango_layout_get_iter(m_pango_layout);
		PangoLayoutLine* line_p = pango_layout_iter_get_line(iter_p);
		pango_layout_iter_get_layout_extents (iter_p, &ink_rect, &log_rect);
		std::string line(m_text_storage, line_p->start_index, line_p->length);
		AM_DBG logger::get_logger()->debug("pango line extents %s: x=%d y=%d width=%d height=%d",line.c_str(), log_rect.x, log_rect.y, log_rect.width, log_rect.height);
		pango_layout_iter_free(iter_p);
		m_log_rect.x = log_rect.x/PANGO_SCALE;
		m_log_rect.y = log_rect.y/PANGO_SCALE;
		m_log_rect.w = log_rect.width/PANGO_SCALE;
		m_log_rect.h = log_rect.height/PANGO_SCALE;
#ifdef  TBD
#endif//TBD
	}
// Compute the shifted position of what we want to draw w.r.t. the visible origin
	switch (m_params.m_mode) {
	default:
	case smil2::stm_append:
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			break;
		case smil2::stc_initial:
			break;
		case smil2::stc_final:
			break;
		case smil2::stc_both:
			break;
		}
		break;
	case smil2::stm_crawl:
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			m_motion_done = m_origin.x > (int)m_log_rect.width() + (int)r.width();
			break;
		case smil2::stc_initial:
			m_start.x = -(int)r.width();
			m_motion_done = m_origin.x > (int)m_log_rect.width() + (int) r.width();
			break;
		case smil2::stc_final:
			break;
		case smil2::stc_both:
			m_start.x = -(int)r.width();
			break;
		}
		break;
	case smil2::stm_jump:
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			break;
		case smil2::stc_initial:
			break;
		case smil2::stc_final:
			break;
		case smil2::stc_both:
			break;
		}
		break;
	case smil2::stm_scroll:
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			if (m_origin.y > (int)m_log_rect.height()  + (int)r.height())
				m_motion_done = true;
			break;
		case smil2::stc_initial:
			m_start.y = -(int)r.height();
			if (m_origin.y > (int)m_log_rect.height()  + (int)r.height())
				m_motion_done = true;
			break;
		case smil2::stc_final:
			break;
		case smil2::stc_both:
			m_start.y = -(int)r.height();
			break;
		}
		break;
	}
	if ( ! m_motion_done ) {
		if (m_params.m_rate == 0 && m_engine.is_auto_rate()) {
			// all information to compute the rate is now available
			unsigned int dur = m_engine.get_dur();
			if (dur) {
				lib::size size(m_log_rect.w, m_log_rect.h);
				unsigned int rate = smil2::smiltext_layout_engine::compute_rate(m_params, m_align, size, r, m_engine.get_dur());
				m_engine.set_rate(rate);
			}
		}
		m_origin.x = m_start.x;
		m_origin.y = m_start.y;
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		switch (m_params.m_mode) {
		default: // no smilText motion, don't come here again
			m_motion_done = true;
			break;
		case smil2::stm_crawl:
			m_origin.x += (int) now * m_params.m_rate / 1000;
			break;
		case smil2::stm_scroll:
			m_origin.y += (int) now * m_params.m_rate / 1000;
			break;
		}

	} else {
		m_context->stopped(m_cookie);
	}
	AM_DBG logger::get_logger()->debug("sdl_smiltext_renderer.redraw: logical_origin(%d,%d) log_rect(%d,%d) r(%d,%d)", m_origin.x, m_origin.y, m_log_rect.w, m_log_rect.h, r.w, r.h);
	_sdl_smiltext_render(r, m_origin,(ambulant_sdl_window*)window);
#ifdef  TBD
#endif//TBD
	m_engine.unlock();
}
#endif//WITH_SDL_PANGO

#ifdef WITH_SDL_TTF
void
sdl_smiltext_renderer::redraw_body(const lib::rect& dirty, common::gui_window *window) {
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
}
#endif//WITH_SDL_TTF

#ifdef WITH_SDL_PANGO
// private methods
void
sdl_smiltext_renderer::_sdl_set_font_attr (PangoAttrList* pal,
	const char* smiltext_font_family,
	smil2::smiltext_font_style smiltext_font_style,
	smil2::smiltext_font_weight smiltext_font_weight,
	int smiltext_font_size,
	unsigned int start_index, unsigned int end_index)
{
	PangoFontDescription* pango_font_description = pango_font_description_new();

	// smiltext font style to pango font style conversion
	PangoStyle pango_font_style;
	switch (smiltext_font_style) {
	default:
	case smil2::sts_normal:
	case smil2::sts_reverse_oblique:	// Not supported by pango
		pango_font_style = PANGO_STYLE_NORMAL;
		break;
	case smil2::sts_italic:
		pango_font_style = PANGO_STYLE_ITALIC;
		break;
	case smil2::sts_oblique:
		pango_font_style = PANGO_STYLE_OBLIQUE;
		break;
	}
	pango_font_description_set_style(pango_font_description, pango_font_style);
	// smiltext font weight to pango font weight conversion
	PangoWeight pango_font_weight;
	switch (smiltext_font_weight) {
	default:
	case smil2::stw_normal:
		pango_font_weight = PANGO_WEIGHT_NORMAL;
		break;
	case smil2::stw_bold:
		pango_font_weight = PANGO_WEIGHT_BOLD;
		break;
	}
	pango_font_description_set_weight(pango_font_description, pango_font_weight);
	// smiltext font size to pango font size conversion
	double pango_font_size = smiltext_font_size*PANGO_SCALE;
	pango_font_description_set_absolute_size(pango_font_description, pango_font_size);
	// smiltext font family to pango font family conversion
	const char* pango_font_family = smiltext_font_family;
	pango_font_description_set_family(pango_font_description, pango_font_family);
	PangoAttribute* pango_attribute = pango_attr_font_desc_new(pango_font_description);
	// add the new PangoAttribute to the PangoAttrList for the range given
	pango_attribute->start_index = start_index;
	pango_attribute->end_index	 = end_index;
	pango_attr_list_insert(pal, pango_attribute);
}

void
sdl_smiltext_renderer::_sdl_set_color_attr(PangoAttrList* pal,
	lib::color_t smiltext_color,
	PangoAttribute* (*pango_attr_color_new)(guint16 r, guint16 g, guint16 b),
	unsigned int start_index, unsigned int end_index)
{
	// smiltext color to pango color conversion
	guint16 pango_red = redc(smiltext_color)*0x101;
	guint16 pango_green = greenc(smiltext_color)*0x101;
	guint16 pango_blue = bluec(smiltext_color)*0x101;
	// Create new PangoAttribute containing the specified color
	PangoAttribute* pango_attribute = pango_attr_color_new(pango_red, pango_green, pango_blue);
	// add the new PangoAttribute to the PangoAttrList for the range given
	pango_attribute->start_index = start_index;
	pango_attribute->end_index = end_index;
	pango_attr_list_insert(pal, pango_attribute);
}
#ifdef  TBD
#endif//TBD

#endif // WITH_SDL_PANGO

#ifdef WITH_SDL_TTF

smil2::smiltext_metrics
sdl_smiltext_renderer::get_smiltext_metrics(const smil2::smiltext_run& strun)
{
	int ascent = 0, descent = 0, height = 0, width = 0, line_spacing = 0;

	if (strun.m_data.length() != 0) {
//TBD		_sdl_smiltext_set_font (strun);
		ascent	= TTF_FontAscent(m_ttf_font);
		descent	= TTF_FontDescent(m_ttf_font);
		line_spacing = TTF_FontLineSkip(m_ttf_font);
		TTF_SizeText(m_ttf_font, strun.m_data.c_str(), &width, &height);
	}
	return smil2::smiltext_metrics(ascent, descent, height, width, line_spacing);
}

void
sdl_smiltext_renderer::render_smiltext(const smil2::smiltext_run& strun, const lib::rect& r)
{

	AM_DBG lib::logger::get_logger()->debug("sdl_render_smiltext(): command=%d data=%s color=0x%x bg_color=0x%x",strun.m_command,strun.m_data.c_str()==NULL?"(null)":strun.m_data.c_str(),strun.m_color,strun.m_bg_color);
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
	AM_DBG lib::logger::get_logger()->debug("sdl_render_smiltext(): data=%s r=L=%d,T=%d,W=%d,H=%d", strun.m_data.c_str(),r.x,r.y,r.w,r.h);
	const lib::point p = m_dest->get_global_topleft();
	int L = rct.left()+p.x,
	    T = rct.top()+p.y,
	    W = rct.width(),
	    H = rct.height();

	if (W == 0 || H == 0)
		return; // cannot render anything
	AM_DBG lib::logger::get_logger()->debug("sdl_render_smiltext():p=(%d,%d)  L=%d,T=%d,W=%d,H=%d",p.x,p.y,L,T,W,H);
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
		tx_pixmap->fill(m_sdl_transparent);
		if ( ! m_blending)
			alpha_media = alpha_chroma = 1.0;
	}
#endif//TBD
//TBD	_sdl_smiltext_set_font(strun);
#ifdef TBD
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
	SDL_Color sdl_color = {redc(text_color), greenc(text_color), bluec(text_color), 1};
	SDL_Color sdl_bg_color= {redc(bg_color), greenc(bg_color), bluec(bg_color), 1 };
	
#ifdef TBD
	if (m_blending) {
		tx_paint.begin( tx_pixmap );
		if ( ! strun.m_bg_transparent) {
			tx_paint.setBackgroundMode(Qt::OpaqueMode);
			tx_paint.setBackgroundColor(m_sdl_transparent);
		}
		if ( ! strun.m_transparent) {
			tx_paint.setPen(sdl_color);
		}
		
		if (bg_pixmap) {
			bg_paint.begin( bg_pixmap );
			bg_paint.setFont(m_font);
			bg_paint.setBrush(sdl_bg_color);
			bg_paint.setPen(Qt::NoPen);
			bg_paint.drawRect(0,0,W,H);
			bg_paint.setPen(sdl_color);
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
		tx_paint.setBrush(sdl_bg_color);
		tx_paint.setPen(Qt::NoPen);
		tx_paint.drawRect(L,T,W,H);
	}
	int flags = Qt::AlignAuto;
	tx_paint.setPen(sdl_color);
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
			sdl_image_blend (screen_img, rct, bg_image, rct0, 
					alpha_media_bg, 0.0,
//XX					chroma_low, chroma_high);
					bg_color, bg_color);
			delete bg_pixmap;
		}
		sdl_image_blend (screen_img, rct, tx_image, rct0, 
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
	m_text_storage = strun.m_data.c_str();
	m_text_color = text_color;
	m_text_bg_color = bg_color;
	SDL_Rect sdl_rect = { L, T, W, H};
	SDL_Color sdl_text_color = {redc(m_text_color),greenc(m_text_color),bluec(m_text_color)};
	SDL_Surface* text_surface = TTF_RenderText_Blended(m_ttf_font, strun.m_data.c_str(), sdl_text_color);
	if (text_surface == NULL) {
        AM_DBG lib::logger::get_logger()->error("%s(%p): Failed rendering %s: %s ",
   					__PRETTY_FUNCTION__,this, strun.m_data.c_str(), TTF_GetError());
	} else {
		sdl_ambulant_window* saw = m_window->get_sdl_ambulant_window();
		saw->copy_to_sdl_surface (text_surface, NULL, &sdl_rect, 255);
		SDL_FreeSurface (text_surface);
	}
//X	_sdl_smiltext_render_wrapped_ttf( L, T, W, H, m_window->get_sdl_ambulant_window(), /*const lib::point offset*/ lib::point());
}

const lib::rect&
sdl_smiltext_renderer::get_rect()
{
	return m_dest->get_rect();
}

void
sdl_smiltext_renderer::smiltext_stopped() {
	m_context->stopped(m_cookie);
}

void 
sdl_smiltext_renderer::_sdl_smiltext_render_text (
    const char* text,
    sdl_ambulant_window* saw, 
    SDL_Rect *sdl_dst_rect
  )
{
  if (strcmp(text,"") == 0) // skip empty string
	return;
  SDL_Surface *text_surface;
  SDL_Color color = {redc(m_text_color),greenc(m_text_color),bluec(m_text_color)};
  if (!(text_surface = TTF_RenderText_Blended(m_ttf_font, text,  color))) {
        AM_DBG lib::logger::get_logger()->debug("_sdl_smiltext_render_wrapped_ttf(%p): Failed rendering %s ", this, text);
  } else {
    saw->copy_to_sdl_surface (text_surface, NULL, sdl_dst_rect, 255 );
  }
  if (text_surface) {
    SDL_FreeSurface(text_surface);
  }
}
void 
sdl_smiltext_renderer::_sdl_smiltext_render_wrapped_ttf(
    int L, 
    int T, 
    int W, 
    int H, 
    sdl_ambulant_window* saw,
    const lib::point offset
  )
{
  uint text_width = W - m_text_size;
  int current_width = 0;
  std::string temp( m_text_storage );
  if (m_wrap) {
    temp += " ";
    int n = 0, p = 0, lines = 0;
    
    while ( n!= -1)
    {
      std::string strSub;
      n = temp.find( " ", p + 1 );		// -- Find the next " "
      TTF_SizeText(m_ttf_font, temp.substr(0,n).c_str(), &current_width,NULL);
      if( n==-1 || current_width >= text_width ) {
        strSub = temp.substr( 0, p );	// -- sets strSub to the of the current line
        SDL_Rect sdl_dst_rect = {L-offset.x, T -offset.y + TTF_FontLineSkip(m_ttf_font) * lines++ , W, H};
        _sdl_smiltext_render_text(strSub.c_str(), saw, &sdl_dst_rect);
        if( n != -1 ){
          temp = temp.substr( p+1, std::string::npos );
        }
        p = 0;
      } else {
        p = n;
      }
      
    }
  } else {
     SDL_Rect sdl_dst_rect = {L-offset.x, T-offset.y, W, H};
//     _sdl_smiltext_render_text(m_text_storage.c_str(), saw, &sdl_dst_rect);
  }
  
}
#endif

void
sdl_smiltext_renderer::_sdl_smiltext_render(
	const lib::rect r, 
	const lib::point offset,
	ambulant_sdl_window* window)
{
	// Determine current position and size.
	const lib::point p = m_dest->get_global_topleft();
	const char* data = m_text_storage.c_str();
	ambulant_sdl_window* asdlw = (ambulant_sdl_window*) window;
	sdl_ambulant_window* saw = asdlw->get_sdl_ambulant_window();
  
	AM_DBG lib::logger::get_logger()->debug("sdl_smiltext_render(%p): ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):offsetp=(%d,%d):",(void *)this,r.left(),r.top(),r.width(),r.height(),data==NULL?"(null)":data,p.x,p.y,offset.x,offset.y);
	if ( ! (
#ifdef WITH_SDL_PANGO
    m_pango_layout && 
#endif // WITH_SDL_PANGO
    window))
		return; // nothing to do

	int L = r.left()+p.x,
		T = r.top()+p.y,
		W = r.width(),
		H = r.height();

#ifdef  GDK_PANGO
	GdkRectangle gdk_rectangle;
	gdk_rectangle.x = L;
	gdk_rectangle.y = T;
	gdk_rectangle.width = W;
	gdk_rectangle.height = H;
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (window->get_ambulant_pixmap()));
	gdk_gc_set_clip_rectangle(gc, &gdk_rectangle);

	// include the text
	pango_layout_set_width(m_pango_layout, m_wrap ? W*PANGO_SCALE : -1);
	if (m_bg_layout) {
		// blending
		pango_layout_set_width(m_bg_layout, m_wrap ? W*PANGO_SCALE : -1);

		GdkPixmap* text_pixmap = gdk_pixmap_new((window->get_ambulant_pixmap()),W,H,-1);
		GdkPixmap* bg_pixmap = gdk_pixmap_new((window->get_ambulant_pixmap()),W,H,-1);
		GdkGC* text_gc = gdk_gc_new (GDK_DRAWABLE (text_pixmap));
		GdkGC* bg_gc =	gdk_gc_new (GDK_DRAWABLE (bg_pixmap));
		gdk_rectangle.x = gdk_rectangle.y = 0;
		gdk_gc_set_clip_rectangle(text_gc, &gdk_rectangle);
		gdk_gc_set_clip_rectangle(bg_gc, &gdk_rectangle);

		GdkPixmap* pixmap = window->get_ambulant_pixmap();
		int PW = -1, PH = -1;
		if (pixmap != NULL)
			gdk_drawable_get_size (pixmap, &PW, &PH);
		if (pixmap == NULL || PW < L+W || PH  < T+H ) {
			g_object_unref (G_OBJECT (text_pixmap));
			g_object_unref (G_OBJECT (bg_pixmap));
			g_object_unref (G_OBJECT (text_gc));
			g_object_unref (G_OBJECT (bg_gc));
			g_object_unref (G_OBJECT (gc));
			lib::logger::get_logger()->trace("smilText: gdk_pixbuf_get_from_drawable failed, pixmap.size()=(%d,%d), (L,T,W,H)=(%d,%d,%d,%d)", PW,PH,L,T,W,H);
			lib::logger::get_logger()->error(gettext("Geometry error in smil document at %s"), m_node->get_sig().c_str());
			return;
		}
		GdkPixbuf* screen_pixbuf = gdk_pixbuf_get_from_drawable (NULL, pixmap, NULL, L, T, 0, 0, W, H);
		GdkColor gdk_transparent;
		gdk_transparent.red = redc(m_transparent)*0x101;
		gdk_transparent.blue = bluec(m_transparent)*0x101;
		gdk_transparent.green = greenc(m_transparent)*0x101;

		gdk_gc_set_rgb_bg_color (bg_gc, &gdk_transparent);
		gdk_gc_set_rgb_fg_color (bg_gc, &gdk_transparent);
		gdk_gc_set_rgb_bg_color (text_gc, &gdk_transparent);
		gdk_gc_set_rgb_fg_color (text_gc, &gdk_transparent);

		// clear text_pixmap and bg_pixmap to transparent color
		gdk_draw_rectangle (bg_pixmap, bg_gc, TRUE, 0, 0, W, H);
		gdk_draw_rectangle (text_pixmap, text_gc, TRUE, 0, 0, W, H);

		// draw m_bg_layout containing smilText runs with text in
		// m_transparent color and background in required color
		gdk_draw_layout(GDK_DRAWABLE (bg_pixmap), bg_gc , 0-offset.x, 0-offset.y, m_bg_layout);
		g_object_unref (G_OBJECT (bg_gc));
//DBG	gdk_pixmap_dump(bg_pixmap, "bg");
		GdkPixbuf* bg_pixbuf = gdk_pixbuf_get_from_drawable(
			NULL,
			bg_pixmap,
			NULL,
			0,0,0,0,W,H);
		lib::rect rc(lib::point(L,T),lib::size(W,H));
		// blend the screen pixbuf with th background pixbuf
		gdk_pixbuf_blend (
			screen_pixbuf,
			rc, bg_pixbuf,
			rc,
			m_alpha_chroma,
			m_alpha_media_bg,
			m_chroma_low,
			m_chroma_high,
			m_transparent);
//DBG	gdk_pixmap_dump( window->get_ambulant_pixmap(), "screen0");

		// draw m_pango_layout containing smilText runs with text in
		// required colors and background in m_transparant color
		gdk_draw_layout(GDK_DRAWABLE (text_pixmap), text_gc , 0-offset.x, 0-offset.y, m_pango_layout);
		g_object_unref (G_OBJECT (text_gc));
//DBG	gdk_pixmap_dump(text_pixmap, "text");
		GdkPixbuf* text_pixbuf = gdk_pixbuf_get_from_drawable(
			NULL,
			text_pixmap,
			NULL,
			0,0,0,0,W,H);
		gdk_pixbuf_blend (
			screen_pixbuf,
			rc,
			text_pixbuf,
			rc,
			m_alpha_chroma,
			m_alpha_media,
			m_chroma_low,
			m_chroma_high,
			m_transparent);
		// draw the blended pixbuf on the screen
		gdk_draw_pixbuf(
			window->get_ambulant_pixmap(),
			gc,
			screen_pixbuf,
			0, 0,
			L, T, W, H,
			GDK_RGB_DITHER_NONE,
			0, 0);
//DBG	gdk_pixmap_dump( window->get_ambulant_pixmap(), "screen1");
		g_object_unref (G_OBJECT (text_pixbuf));
		g_object_unref (G_OBJECT (bg_pixbuf));
		g_object_unref (G_OBJECT (screen_pixbuf));
		g_object_unref (G_OBJECT (bg_pixmap));
		g_object_unref (G_OBJECT (text_pixmap));
	} else {
		gdk_draw_layout(
			GDK_DRAWABLE (window->get_ambulant_pixmap()),
			gc,
			L-offset.x,
			T-offset.y,
			m_pango_layout);
	}
	g_object_unref (G_OBJECT (gc));
#elif defined (WITH_SDL_PANGO)//GDK_PANGO
	pango_layout_set_width(m_pango_layout, m_wrap ? W*PANGO_SCALE : -1);
	SDL_Surface* sdl_surface = SDLPango_CreateSurfaceDraw (m_sdl_pango_context);
	SDLPango_Draw(m_sdl_pango_context, sdl_surface, 0, 0);
	double alpha_media = 1.0;
	L -= offset.x;
	T -= offset.y;
	SDL_Rect sdl_dst_rect = {L,T,W,H}; //X {dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height() };	
	saw->copy_to_sdl_surface (sdl_surface, NULL, &sdl_dst_rect, 255 * alpha_media);
	SDL_FreeSurface(sdl_surface);
#endif// ! GDK_PANGO

#ifdef WITH_SDL_TTF
	if (m_ttf_font == NULL) {
        m_ttf_font = TTF_OpenFont(DEFAULT_FONT_FILE1, m_text_size*1.1);
		if (m_ttf_font == NULL) {
			AM_DBG lib::logger::get_logger()->error("TTF_OpenFont(%s, %d): %s", DEFAULT_FONT_FILE1, m_text_size*1.1, TTF_GetError());
			return;
		}
        TTF_SetFontStyle(m_ttf_font, m_ttf_style);
		TTF_SetFontOutline(m_ttf_font, 0);
		TTF_SetFontKerning(m_ttf_font, 1);
		TTF_SetFontHinting(m_ttf_font, (int)TTF_HINTING_NORMAL);
	}
	_sdl_smiltext_render_wrapped_ttf (L, T, W, H, saw, offset);
#endif // WITH_SDL_TTF
}

} // namespace sdl

} // namespace gui

} //namespace ambulant

#endif//WITH_SDL2
