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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define FONT "Times 6"
#define DEFAULT_FONT_FILE "/usr/local/etc/ginga/files/font/vera.ttf"
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
	m_text_size(0),

	m_ttf_font(NULL),
	m_sdl_surface(NULL)
{
	smil2::params *params = smil2::params::for_node(node);
	AM_DBG lib::logger::get_logger()->debug("sdl_text_renderer(0x%x) params=0x%x",this,params);
	if (params) {
		m_text_font = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		m_text_color = params->get_color("color", 0);
		m_text_size = params->get_float("font-size", 0.0);
		delete params;
	}
	m_sdl_color.r = m_sdl_color.g = m_sdl_color.b = 0; // black
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
#ifdef  JNK
	PangoContext *context;
	PangoLanguage *language;
	PangoFontDescription *font_desc;
	PangoLayout *layout;
#endif//JNK
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
		ambulant_sdl_window* asdlw = (ambulant_sdl_window*) w;
		if (m_ttf_font == NULL) {
			m_ttf_font =  TTF_OpenFont(DEFAULT_FONT_FILE, DEFAULT_FONT_HEIGHT);
//			assert(m_ttf_font);
			if (m_ttf_font == NULL) {
			  lib::logger::get_logger()->error("TTF_OpenFont(%s, %d): %s", DEFAULT_FONT_FILE, DEFAULT_FONT_HEIGHT, TTF_GetError());
			  return;
			}
			TTF_SetFontStyle(m_ttf_font, (int)TTF_STYLE_BOLD);
			TTF_SetFontOutline(m_ttf_font, 0);
			TTF_SetFontKerning(m_ttf_font, 1);
			TTF_SetFontHinting(m_ttf_font, (int)TTF_HINTING_NORMAL);
		}
		m_sdl_surface = TTF_RenderText_Solid (m_ttf_font, m_text_storage, m_sdl_color);
		assert (m_sdl_surface);

	}
	SDL_Rect sdl_dst_rect = {L,T,W,H}; //X {dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height() };
	asdlw->copy_sdl_surface (m_sdl_surface, NULL, &sdl_dst_rect, 255 * alpha_media);
#ifdef  JNK
		// initialize the pango context, layout...
		context = gdk_pango_context_get();
		language = sdl_get_default_language();
		pango_context_set_language (context, language);
		pango_context_set_base_dir (context, PANGO_DIRECTION_LTR);
		// We initialize the font as Sans 10
		font_desc = pango_font_description_from_string ("sans 10");
		// in case we have some specific font style and type
		if (m_text_font) {
			// TBD: smil font name/style to pango font name/style conversion
			pango_font_description_set_family(font_desc, m_text_font);
		}

		// in case we have some point size (taken from sdl_smiltext.cpp)
		if (m_text_size) {
			// smil font size to pango font size conversion
			double pango_font_size = m_text_size*PANGO_SCALE;
			pango_font_description_set_absolute_size(font_desc, pango_font_size);
		}

		pango_context_set_font_description (context, font_desc);
		layout = pango_layout_new(context);
		pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);

		// include the text
		pango_layout_set_text (layout, m_text_storage, -1);
		// according to the documentation, Pango sets the width in thousandths of a device unit (why? I don't know)
		pango_layout_set_width(layout, W*1000);
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
	}
#endif//JNK
}

#endif//WITH_SDL2
