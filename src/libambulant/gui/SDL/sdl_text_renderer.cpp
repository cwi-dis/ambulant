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

#ifdef  WITH_SDL2 // TBD

//#include "ambulant/gui/SDL/sdl_includes.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_text_renderer.h"
#include "ambulant/gui/SDL/sdl_window.h"

#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

//#define WITH_SDLPANGO // doesn't work yet...
#ifdef  WITH_SDLPANGO
#include <pango-1.0/pango/pango.h>
#define __PANGO_H__ // this reveals some useful functions we need to use
#include <SDL_Pango.h>
#endif//WITH_SDLPANGO

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define FONT "Times 6"
#define DEFAULT_FONT_FILE1 "/usr/share/fonts/liberation/LiberationSans-Regular.ttf"
#define DEFAULT_FONT_FILE2 "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf" 
#define DEFAULT_FONT_FILE3 "/usr/local/etc/ginga/files/font/vera.ttf"
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
	m_text_font(NULL),
	m_text_size(DEFAULT_FONT_HEIGHT),

	m_ttf_font(NULL),
	m_ttf_style(TTF_STYLE_NORMAL),
	m_sdl_surface(NULL)
{
	smil2::params *params = smil2::params::for_node(node);
	AM_DBG lib::logger::get_logger()->debug("sdl_text_renderer(0x%x) params=0x%x",this,params);
	if (params) {
		int ttf_font_style = TTF_STYLE_NORMAL;
		const char* font_style = params->get_str("font-style");
		const char* font_weight = params->get_str("font-weight");
		if (font_style != NULL && (strcmp(font_style, "italic") == 0 || strcmp(font_style, "oblique") == 0 || strcmp(font_style, "reverseOblique") == 0)) {
			m_ttf_style |= TTF_STYLE_ITALIC;
		}
		if (font_weight != NULL && strcmp(font_weight, "bold") == 0) {
			m_ttf_style |= TTF_STYLE_BOLD;
		}		 
		m_text_font = params->get_str("font-family");
		m_text_color = params->get_color("color", 0);
		m_text_size = params->get_float("font-size", DEFAULT_FONT_HEIGHT);
		delete params;
	}
}

sdl_text_renderer::~sdl_text_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~sdl_text_renderer(0x%x)", this);
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
#ifdef  WITH_SDLPANGO
	PangoContext *context;
	PangoLanguage *language;
	PangoFontDescription *font_desc;
	PangoLayout *layout;
#endif//WITH_SDLPANGO
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
	AM_DBG lib::logger::get_logger()->debug(
		"sdl_text_renderer.redraw(0x%x):"
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
#ifndef  WITH_SDLPANGO
		if (m_ttf_font == NULL) { // Fedora 16
			m_ttf_font = TTF_OpenFont(DEFAULT_FONT_FILE1, m_text_size);
			if (m_ttf_font == NULL) { // Ubuntu 12.04
				m_ttf_font = TTF_OpenFont(DEFAULT_FONT_FILE2, m_text_size);
			}
			if (m_ttf_font == NULL) { // local
				m_ttf_font = TTF_OpenFont(DEFAULT_FONT_FILE3, m_text_size);
			}
//			assert(m_ttf_font);
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
		m_sdl_surface = TTF_RenderText_Solid (m_ttf_font, m_text_storage, sdl_color);
		assert (m_sdl_surface);
#else //WITH_SDLPANGO
		// initialize the pango context, layout...
//X		context = gdk_pango_context_get();
		SDLPango_Context* sdl_pango_context = SDLPango_CreateContext();
		context = *(PangoContext**) sdl_pango_context;
		language = pango_language_get_default();
//X		pango_context_set_language (context, language);
		SDLPango_SetLanguage (sdl_pango_context, pango_language_to_string (language));
//X		pango_context_set_base_dir (context, PANGO_DIRECTION_LTR);
		SDLPango_SetBaseDirection (sdl_pango_context, SDLPANGO_DIRECTION_LTR);
//X	We initialize the font as Sans 10
//X		font_desc = pango_font_description_from_string ("sans 10");
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
//X		pango_context_set_font_description (context, font_desc);
// no equivalent in SDLPango, hope it does so by itself
//X		layout = pango_layout_new(context);
		layout = SDLPango_GetPangoLayout(sdl_pango_context);
		pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);
		SDLPango_SetDefaultColor (sdl_pango_context, MATRIX_WHITE_BACK);               
	// include the text
//X		pango_layout_set_text (layout, m_text_storage, -1);
		// according to the documentation, Pango sets the width in thousandths of a device unit (why? I don't know)
		pango_layout_set_width(layout, W*1000);
		SDLPango_SetText (sdl_pango_context, m_text_storage, -1);//m_text_size);
		m_sdl_surface = SDLPango_CreateSurfaceDraw (sdl_pango_context);
		SDLPango_Draw(sdl_pango_context, m_sdl_surface, 0, 0);

#ifndef WITH_SDLPANGO
		// Foreground Color of the text
		GdkColor sdl_color;
		sdl_color.red = redc(m_text_color)*0x101;
		sdl_color.blue = bluec(m_text_color)*0x101;
		sdl_color.green = greenc(m_text_color)*0x101;
		GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (asdlw->get_ambulant_pixmap()));
		gdk_gc_set_rgb_fg_color (gc, &sdl_color);
		gdk_draw_layout(GDK_DRAWABLE (asdlw->get_ambulant_pixmap()),gc , L, T, layout);
		pango_font_description_free(font_desc);
		g_object_unref (G_OBJECT (context));
		g_object_unref(layout);
		g_object_unref (G_OBJECT (gc));
#endif// ! WITH_SDLPANGO
		SDLPango_FreeContext (sdl_pango_context);
#endif//WITH_SDLPANGO
	} // m_text_storage != NULL && m_sdl_surface == NULL)
	SDL_Rect sdl_dst_rect = {L,T,W,H}; //X {dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height() };
	asdlw->copy_sdl_surface (m_sdl_surface, NULL, &sdl_dst_rect, 255 * alpha_media);
}

#endif//WITH_SDL2
