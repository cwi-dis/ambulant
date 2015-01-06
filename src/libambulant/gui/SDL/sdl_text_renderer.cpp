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

#if defined(WITH_SDL_IMAGE)

#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_text_renderer.h"
#include "ambulant/gui/SDL/sdl_window.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#if defined(WITH_SDL_TTF)
#define FONT "Times 6"
#ifdef AMBULANT_PLATFORM_MACOS
#define DEFAULT_FONT_FILE1 "/Library/Fonts/Arial.ttf"
#define DEFAULT_FONT_FILE2 "/Library/Fonts/Times New Roman.ttf"
#define DEFAULT_FONT_FILE3 "/Library/Fonts/Courier New.ttf"
#else // assume AMBULANT_PLATFORM_LINUX
#ifndef ANDROID
#define DEFAULT_FONT_FILE1 "/usr/share/fonts/liberation/LiberationSans-Regular.ttf"
#define DEFAULT_FONT_FILE2 "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf" 
#define DEFAULT_FONT_FILE3 "/usr/local/etc/ginga/files/font/vera.ttf"
#else // ANDROID
#define DEFAULT_FONT_FILE1 "/system/fonts/DroidSans.ttf"
#define DEFAULT_FONT_FILE2 "/system/fonts/Roboto-Regular.ttf" 
#define DEFAULT_FONT_FILE3 "/system/fonts/DroidSerif-Regular.ttf"
#endif // ANDROID
#endif // AMBULANT_PLATFORM_XXX
#endif// defined(WITH_SDL_TTF)

#define DEFAULT_FONT_HEIGHT 16

using namespace ambulant;
using namespace gui::sdl;

extern const char sdl_text_playable_tag[] = "text";
extern const char sdl_text_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererSdl");
extern const char sdl_text_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererText");

common::playable_factory *
gui::sdl::create_sdl_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSdl"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererText"), true);
#if defined(WITH_SDL_TTF)
	TTF_Init();
#elif ! defined(WITH_SDL_PANGO)
	lib::logger::get_logger()->trace("No %s renderer available", "text");
#endif// ! defined(WITH_SDL_PANGO)
	return new common::single_playable_factory<
		sdl_text_renderer,
		sdl_text_playable_tag,
		sdl_text_playable_renderer_uri,
		sdl_text_playable_renderer_uri2,
		sdl_text_playable_renderer_uri2>(factory, mdp);
}

sdl_text_renderer::sdl_text_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	sdl_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
	m_text_storage(NULL),
	m_text_color(0),
	m_text_size(DEFAULT_FONT_HEIGHT),
	m_text_font(NULL),
#if defined(WITH_SDL_TTF)
	m_ttf_font(NULL),
	m_ttf_style(TTF_STYLE_NORMAL),
#endif// defined(WITH_SDL_TTF)
	m_sdl_surface(NULL)
{
	smil2::params *params = smil2::params::for_node(node);
	AM_DBG lib::logger::get_logger()->debug("sdl_text_renderer(%p) params=%p",this,params);
	if (params) {
#if defined(WITH_SDL_TTF)
		int ttf_font_style = TTF_STYLE_NORMAL;
		const char* font_style = params->get_str("font-style");
		const char* font_weight = params->get_str("font-weight");
		if (font_style != NULL && (strcmp(font_style, "italic") == 0 || strcmp(font_style, "oblique") == 0 || strcmp(font_style, "reverseOblique") == 0)) {
			m_ttf_style |= TTF_STYLE_ITALIC;
		}
		if (font_weight != NULL && strcmp(font_weight, "bold") == 0) {
			m_ttf_style |= TTF_STYLE_BOLD;
		}
#endif//defined(WITH_SDL_TTF)
		m_text_font = params->get_str("font-family");
		m_text_color = params->get_color("color", 0);
		m_text_size = params->get_float("font-size", DEFAULT_FONT_HEIGHT);
		delete params;
	}
}

sdl_text_renderer::~sdl_text_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~sdl_text_renderer(%p)", this);
	m_lock.enter();
	if (m_text_storage != NULL) {
		free(m_text_storage);
		m_text_storage =  NULL;
	}
	SDL_FreeSurface (m_sdl_surface);
	m_lock.leave();
}

void
sdl_text_renderer::redraw_body(const lib::rect &r, common::gui_window* w) {
// No m_lock needed, protected by base class
#if defined(WITH_SDL_PANGO)
	PangoContext *context;
	PangoLanguage *language;
	PangoFontDescription *font_desc;
	PangoLayout *layout;
#endif//defined(WITH_SDL_PANGO)
	double alpha_media = 1.0;
	ambulant_sdl_window* asdlw = (ambulant_sdl_window*) w;

	const lib::point p = m_dest->get_global_topleft();
	//XXX need to be fixed in renderer_playable_dsl
	if (m_data && !m_text_storage) {
		m_text_storage = (char*) malloc(m_data_size+1);
		strncpy(m_text_storage,
			(const char*) m_data,
			m_data_size);
		m_text_storage[m_data_size] = '\0';
	}
	AM_DBG lib::logger::get_logger()->debug (
		"sdl_text_renderer.redraw(%p):"
		"ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):"
		"font-family=(%s)",
		(void *)this, r.left(), r.top(), r.width(), r.height(),
		m_text_storage == NULL ? "(null)": (const char*) m_text_storage,
		p.x, p.y, m_text_font == NULL ? "(null)": (const char*) m_text_font);
	int L = r.left()+p.x,
	    T = r.top()+p.y,
	    W = r.width(),
	    H = r.height();

	if (m_text_storage != NULL && m_sdl_surface == NULL) {
#if defined(WITH_SDL_TTF)
		if (m_ttf_font == NULL) { // Fedora 16
			m_ttf_font = TTF_OpenFont(DEFAULT_FONT_FILE1, m_text_size);
			if (m_ttf_font == NULL) { // Ubuntu 12.04
				m_ttf_font = TTF_OpenFont(DEFAULT_FONT_FILE2, m_text_size);
			}
			if (m_ttf_font == NULL) { // local
				m_ttf_font = TTF_OpenFont(DEFAULT_FONT_FILE3, m_text_size);
			}
			if (m_ttf_font == NULL) {
				lib::logger::get_logger()->error("TTF_OpenFont(%s, %d): %s", DEFAULT_FONT_FILE1, m_text_size, TTF_GetError());
				return;
			}
			TTF_SetFontStyle(m_ttf_font, m_ttf_style);
			TTF_SetFontOutline(m_ttf_font, 0);
			TTF_SetFontKerning(m_ttf_font, 1);
			TTF_SetFontHinting(m_ttf_font, (int)TTF_HINTING_NORMAL);
		}
		SDL_Color sdl_color = {redc(m_text_color),greenc(m_text_color),bluec(m_text_color)};
		m_sdl_surface = TTF_RenderText_Blended_Wrapped (m_ttf_font, m_text_storage, sdl_color, W);
		assert (m_sdl_surface);
#elif defined(WITH_SDL_PANGO)
		// initialize the pango context, layout...
		SDLPango_Context* sdl_pango_context = SDLPango_CreateContext();
		context = *(PangoContext**) sdl_pango_context;
		language = pango_language_get_default();
		SDLPango_SetLanguage (sdl_pango_context, pango_language_to_string (language));
		SDLPango_SetBaseDirection (sdl_pango_context, SDLPANGO_DIRECTION_LTR);
		font_desc = SDLPango_GetPangoFontDescription(sdl_pango_context);
		// in case we have some specific font style and type
		if (m_text_font) {
		// TBD: smil font name/style to pango font name/style conversion
			pango_font_description_set_family(font_desc, m_text_font);
		}
		// in case we have some point size (taken from gtk_smiltext.cpp)
		if (m_text_size) {
			// smil font size to pango font size conversion
			double pango_font_size = m_text_size*PANGO_SCALE;
			pango_font_description_set_absolute_size(font_desc, pango_font_size);
		}
// no equivalent in SDLPango, hope it does so by itself
//		layout = pango_layout_new(context);
		layout = SDLPango_GetPangoLayout(sdl_pango_context);
		pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);
		SDLPango_Matrix color_matrix = *MATRIX_TRANSPARENT_BACK_BLACK_LETTER;
		// 1st column of SDLPango_Matrix contains background color, 2nd foregroumd color
		if (m_text_color != 0 /*black */ ) {
			color_matrix.m[0][1] = redc(m_text_color);
			color_matrix.m[1][1] = greenc(m_text_color);
			color_matrix.m[2][1] = bluec(m_text_color);
			color_matrix.m[3][1] = 255; // alpha pixel
		}
		SDLPango_SetDefaultColor (sdl_pango_context, &color_matrix);               
		// include the text
		// according to the documentation, Pango sets the width in thousandths of a device unit (why? I don't know)
		pango_layout_set_width(layout, W*1000);
		SDLPango_SetText (sdl_pango_context, m_text_storage, -1);//m_text_size);
		m_sdl_surface = SDLPango_CreateSurfaceDraw (sdl_pango_context);
		SDLPango_Draw(sdl_pango_context, m_sdl_surface, 0, 0);

		SDLPango_FreeContext (sdl_pango_context);
#endif//defined(WITH_SDL_PANGO)
		// optimize for blitting om following redraw() with the same content
		m_sdl_surface = SDL_ConvertSurface(m_sdl_surface, asdlw->get_sdl_ambulant_window()->get_sdl_surface()->format, 0);
	} // m_text_storage != NULL && m_sdl_surface == NULL)
	SDL_Rect sdl_dst_rect = {L,T,W,H}; //X {dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height() };
	SDL_Rect sdl_src_rect = {0,0,W,H};
	sdl_ambulant_window* saw = asdlw->get_sdl_ambulant_window();
	saw->copy_to_sdl_surface (m_sdl_surface, &sdl_src_rect, &sdl_dst_rect, 255 * alpha_media);
}

#endif//defined(WITH_SDL_IMAGE)
