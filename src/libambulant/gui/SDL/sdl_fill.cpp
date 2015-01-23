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

#ifdef  WITH_SDL_IMAGE

#include "ambulant/common/factory.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/smil2/test_attrs.h"

#include "ambulant/gui/SDL/sdl_renderer.h"

#include "ambulant/gui/SDL/sdl_window.h"
#include "ambulant/gui/SDL/sdl_fill.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::sdl;

extern const char sdl_fill_playable_tag[] = "brush";
extern const char sdl_fill_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererSdl");
extern const char sdl_fill_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererFill");

common::playable_factory *
ambulant::gui::sdl::create_sdl_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSdl"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererFill"), true);
	return new common::single_playable_factory<
		sdl_fill_renderer,
		sdl_fill_playable_tag,
		sdl_fill_playable_renderer_uri,
		sdl_fill_playable_renderer_uri2,
		sdl_fill_playable_renderer_uri2>(factory, mdp);
}

sdl_fill_renderer::~sdl_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~sdl_fill_renderer(%p)", (void *)this);
	SDL_FreeSurface(m_surface);
	m_lock.leave();
}

void
sdl_fill_renderer::redraw_body(const lib::rect &dirty, common::gui_window *window) {

	// <brush> drawing

	AM_DBG lib::logger::get_logger()->debug("sdl_fill_renderer::redraw_body(%p)", (void *)this);
	int err = 0;
	const common::region_info *info = m_dest->get_info();
	lib::rect r = m_dest->get_rect();
	ambulant_sdl_window* asw = (ambulant_sdl_window*) window;
	sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
	// First find our whole area to be cleared to <brush> color
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	int L = dstrect_whole.left(),
		T = dstrect_whole.top(),
		W = dstrect_whole.width(),
		H = dstrect_whole.height();
	if (m_surface == NULL || m_W != W || m_H != H) {
		if (m_surface != NULL) {
			SDL_FreeSurface (m_surface);
			m_W = W;
			m_H = H;
		}
		// from http://wiki.libsdl.org/: using the default masks for the depth
		m_surface = SDL_CreateRGBSurface(0,W,H,32,0,0,0,0);
		assert(m_surface != NULL);
	}
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		return;
	}
	// Fill with <brush> color
	color_t color = lib::to_color(color_attr);
	lib::color_t bgcolor = info ? info->get_bgcolor() : lib::rrggbb_to_color(0xffffff);
	Uint8 alpha = info ? info->get_mediaopacity()* 255 : 255;
	Uint8 bgalpha = info ? info->get_bgopacity()* 255 : 255;
	AM_DBG lib::logger::get_logger()->debug("sdl_fill_renderer.redraw_body: clearing to %p", (long)color);
	SDL_Rect sdl_src_rect = {0, 0, W, H};
	SDL_Rect sdl_dst_rect = {L, T, W, H};
	SDL_Color sdl_bgcolor = {redc(bgcolor), greenc(bgcolor), bluec(bgcolor), bgalpha};
	// Draw the background for the region
	err = SDL_FillRect (m_surface, &sdl_src_rect, SDL_MapRGBA(m_surface->format, sdl_bgcolor.r, sdl_bgcolor.g, sdl_bgcolor.b, sdl_bgcolor.a));
	assert (err==0);
	// Draw the foreground for the region
	SDL_Color sdl_fgcolor = {redc(color), greenc(color), bluec(color), alpha};
	err = SDL_FillRect (m_surface, &sdl_src_rect, SDL_MapRGBA(m_surface->format, sdl_fgcolor.r, sdl_fgcolor.g, sdl_fgcolor.b, sdl_fgcolor.a));
	assert (err==0);
	AM_DBG lib::logger::get_logger()->debug("sdl_fill_renderer.redraw_body(%p, local_ltrb=(%d,%d,%d,%d)",(void *)this, L,T,W,H);
	err = saw->copy_to_sdl_surface (m_surface, &sdl_src_rect, &sdl_dst_rect, 255);
	assert (err==0);
}

void
sdl_fill_renderer::start(double where)
{
	start_transition(where);
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("sdl_fill_renderer.start(0x%x)", (void *)this);
	if (m_activated) {
		logger::get_logger()->trace("sdl_fill_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	m_activated = true;
	if (!m_dest) {
		logger::get_logger()->trace("sdl_fill_renderer.start(0x%x): no surface", (void *)this);
		return;
	}
	m_dest->show(this);
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
	m_lock.leave();
}

/*** sdl_background_renderer ***
 */
sdl_background_renderer::~sdl_background_renderer()
{
}

void
sdl_background_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	if ( !	(m_src && m_dst))
		return;
	const lib::rect &r = m_dst->get_rect();
	AM_DBG lib::logger::get_logger()->debug("sdl_background_renderer::redraw(%p)", (void *)this);
	double opacity = m_src->get_bgopacity();
	if (opacity > 0.0) {
	// First find our whole area to be cleared to background color
		ambulant_sdl_window* asw = (ambulant_sdl_window*) window;
		sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
#ifdef WITH_SDL_TEXTURE
		SDL_Renderer* background_renderer =  saw != NULL ? saw->get_sdl_renderer() : NULL;
		if (background_renderer == NULL) {
			return;
		}
#endif//WITH_SDL_TEXTURE
		lib::rect dstrect_whole = r;
		dstrect_whole.translate(m_dst->get_global_topleft());
		int L = dstrect_whole.left(),
			T = dstrect_whole.top(),
			W = dstrect_whole.width(),
			H = dstrect_whole.height();
		// Fill with background color
		lib::color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("sdl_background_renderer::redraw: clearing to %x, asw=%p local_ltwh(%d,%d,%d,%d)",(long)bgcolor,(void*)asw,L,T,W,H);

		Uint8 red = redc(bgcolor), green = greenc(bgcolor), blue = bluec(bgcolor);
		// Set and draw the background color for the region
		SDL_Rect sdl_dst_rect = {L, T, W, H};
		int err = 0;
#ifdef WITH_SDL_TEXTURE
		err = SDL_SetRenderDrawColor (background_renderer, red, green, blue, opacity*255);
		assert (err==0);
//		err = SDL_RenderClear (background_renderer);
		AM_DBG lib::logger::get_logger()->debug("sdl_background_renderer::redraw(%p) l,t,w,h=(%d,%d, %d,%d) rgba=(%d,%d,%d)", (void *)this, L,T,W,H,red,green,blue);
		err = SDL_RenderFillRect (background_renderer, &sdl_dst_rect);
#else//WITH_SDL_TEXTURE
		if (bgcolor != m_bgcolor || m_map == 0) {
			m_bgcolor = bgcolor;
			m_sdl_color = SDL_Color_from_ambulant_color(bgcolor);
			m_map = SDL_MapRGBA (saw->get_sdl_surface()->format, m_sdl_color.r, m_sdl_color.g, m_sdl_color.b, m_sdl_color.a);
		}
		err = SDL_FillRect (saw->get_sdl_surface(), &sdl_dst_rect, m_map);
#endif//WITH_SDL_TEXTURE
		assert (err==0);
//		SDL_Rect sdl_src_rect = {0, 0, W, H};
//TBD?		err = asw->copy_sdl_surface (m_surface, &sdl_src_rect, &sdl_dst_rect, 255);
//		assert (err==0);
/*TBD*
		if (opacity == 1.0) {
		} else {  //XXXX adapted from sdl_transition. May be some code to be factored out
			// Method:
			// 1. Get the current on-screen image as a pixmap
			// 2. Create a new pixmap and draw a coloured rectangle on it
			// 3. Blend these 2 pixmaps together by getting their pixbufs
			// 4. Draw the resulting pixbuf to become the new on-screen image
			GdkPixmap* opm = asdlw->get_ambulant_pixmap();
			gint width; gint height;
			gdk_drawable_get_size(GDK_DRAWABLE (opm), &width, &height);
			GdkPixmap* npm = gdk_pixmap_new(opm, width, height, -1);
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (opm));
			GdkGC *ngc = gdk_gc_new (GDK_DRAWABLE (npm));
			gdk_gc_set_rgb_fg_color (ngc, &bgc);
			gdk_draw_rectangle (GDK_DRAWABLE (npm), ngc, TRUE, L, T, W, H);
			GdkPixbuf* old_pixbuf = gdk_pixbuf_get_from_drawable(NULL, opm, NULL, L, T, 0, 0, W, H);
			GdkPixbuf* new_pixbuf = gdk_pixbuf_get_from_drawable(NULL, npm, NULL, L, T, 0, 0, W, H);
			int alpha = static_cast<int>(round(255*opacity));
			gdk_pixbuf_composite(new_pixbuf, old_pixbuf,0,0,W,H,0,0,1,1,GDK_INTERP_BILINEAR, alpha);
			gdk_draw_pixbuf(opm, gc, old_pixbuf, 0, 0, L, T, W, H, GDK_RGB_DITHER_NONE,0,0);
			g_object_unref (G_OBJECT (old_pixbuf));
			g_object_unref (G_OBJECT (new_pixbuf));
			g_object_unref (G_OBJECT (ngc));
			g_object_unref (G_OBJECT (gc));
		}
		//sdl_widget_modify_bg (SDL_WIDGET (asdlw->get_ambulant_widget()->get_sdl_widget()), SDL_STATE_NORMAL, &bgc );
		if (m_background_pixmap) {
			AM_DBG lib::logger::get_logger()->debug("sdl_background_renderer::redraw: drawing pixmap");
		//	paint.drawPixmap(L, T, *m_background_pixmap);
		}
TBD*/
	}
}

void sdl_background_renderer::highlight(gui_window *window)
{
}

void
sdl_background_renderer::keep_as_background()
{
	AM_DBG lib::logger::get_logger()->debug("sdl_background_renderer::keep_as_background(%p) called", (void *)this);
	const lib::rect &r = m_dst->get_rect();
/*TBD
	ambulant_sdl_window* asdlw = (ambulant_sdl_window*) m_dst->get_gui_window();
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	if (m_background_pixmap) {
		delete m_background_pixmap;
		m_background_pixmap = NULL;
	}
	m_background_pixmap = asdlw->get_pixmap_from_screen(dstrect_whole);
//XXXX	dumpPixmap(m_background_pixmap, "/tmp/keepbg");
TBD*/
}

#endif//WITH_SDL_IMAGE
