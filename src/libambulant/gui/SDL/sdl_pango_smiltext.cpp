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

#if defined(WITH_SDL2) && !defined(WITH_SDL_TTF)
// TBD: media opacity, media background opacity, chroma keying, offscreen rendering
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_pango_smiltext.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"
#if ! defined(WITH_SDL_PANGO)
#include "ambulant/gui/none/none_area.h"
#endif// ! defined(WITH_SDL_PANGO)

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
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
#if defined(WITH_SDL_PANGO)
#define SDL_SMILTEXT_RENDERER sdl_smiltext_renderer 
#else// ! defined(WITH_SDL_PANGO)
#define SDL_SMILTEXT_RENDERER none::none_area_renderer
	lib::logger::get_logger()->trace("No %s renderer available", "smilText");
#endif// ! defined(WITH_SDL_PANGO)
	return new common::single_playable_factory<
		SDL_SMILTEXT_RENDERER,
		sdl_smiltext_playable_tag,
		sdl_smiltext_playable_renderer_uri,
		sdl_smiltext_playable_renderer_uri2,
		sdl_smiltext_playable_renderer_uri2>(factory, mdp);
}
#if defined(WITH_SDL_PANGO)

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
	m_sdl_pango_context(NULL),
	m_pango_attr_list(NULL),
	m_pango_context(NULL),
	m_pango_layout(NULL),
	m_bg_pango_attr_list(NULL),
	m_bg_layout(NULL),

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
}

sdl_smiltext_renderer::~sdl_smiltext_renderer()
{
	m_engine.lock();

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
	m_engine.unlock();
}

void
sdl_smiltext_renderer::start(double t)
{
	gui::sdl::sdl_renderer<common::renderer_playable>::start(t);
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	m_motion_done = false;
	renderer_playable::start(t);
	m_context->started(m_cookie);
}

void
sdl_smiltext_renderer::seek(double t)
{
	m_engine.seek(t);
}

bool
sdl_smiltext_renderer::stop()
{
	m_engine.stop();
	renderer_playable::stop();
	m_context->stopped(m_cookie);
	return true;
}

void
sdl_smiltext_renderer::marker_seen(const char *name)
{
	m_context->marker_seen(m_cookie, name);
}


void
sdl_smiltext_renderer::smiltext_changed()
{
	m_engine.lock();
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
			gint start_index = m_text_storage.size();
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

void
sdl_smiltext_renderer::redraw_body(const rect &dirty, gui_window *window)
{
#ifdef  TBD
#endif//TBD
	PangoRectangle ink_rect;
	PangoRectangle log_rect;

	m_engine.lock();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("sdl_smiltext_renderer.redraw(%p, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (m_is_changed) {
		// compute the changes
		_sdl_smiltext_changed();
		m_is_changed = false;
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

	AM_DBG lib::logger::get_logger()->debug("sdl_smiltext_render(%p): ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):offsetp=(%d,%d):",(void *)this,r.left(),r.top(),r.width(),r.height(),data==NULL?"(null)":data,p.x,p.y,offset.x,offset.y);
	if ( ! (m_pango_layout && window))
		return; // nothing to do

	int L = r.left()+p.x,
		T = r.top()+p.y,
		W = r.width(),
		H = r.height();

	pango_layout_set_width(m_pango_layout, m_wrap ? W*PANGO_SCALE : -1);
	SDL_Surface* sdl_surface = SDLPango_CreateSurfaceDraw (m_sdl_pango_context);
	SDLPango_Draw(m_sdl_pango_context, sdl_surface, 0, 0);
	double alpha_media = 1.0;
	L -= offset.x;
	T -= offset.y;
	SDL_Rect sdl_dst_rect = {L,T,W,H}; //X {dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height() };	
	sdl_ambulant_window* saw = asdlw->get_sdl_ambulant_window();
	saw->copy_to_sdl_surface (sdl_surface, NULL, &sdl_dst_rect, 255 * alpha_media);
	SDL_FreeSurface(sdl_surface);
}
#endif// defined(WITH_SDL_PANGO)

} // namespace sdl

} // namespace gui

} //namespace ambulant

#endif // defined(WITH_SDL2) && !defined(WITH_SDL_TTF)
