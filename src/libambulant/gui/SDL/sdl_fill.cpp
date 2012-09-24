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

//#include "ambulant/gui/sdl/sdl_includes.h"
//#include "ambulant/gui/sdl/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_fill.h"
#include "ambulant/gui/SDL/sdl_window.h"
//#include "ambulant/gui/sdl/sdl_transition.h"
//#include "ambulant/gui/sdl/sdl_image_renderer.h"
//#include "ambulant/gui/sdl/sdl_text_renderer.h"
#include "ambulant/smil2/test_attrs.h"

#ifdef  WITH_SDL2 // TBD

///#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::sdl;

extern const char sdl_fill_playable_tag[] = "brush";
extern const char sdl_fill_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererSdl");
extern const char sdl_fill_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererFill");

/*JNK? */
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
	AM_DBG lib::logger::get_logger()->debug("~sdl_fill_renderer(0x%x)", (void *)this);
	m_intransition = NULL;
	m_outtransition = NULL;
//TBD	if (m_trans_engine) delete m_trans_engine;
	m_trans_engine = NULL;
 
	m_lock.leave();
}

void
sdl_fill_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("sdl_fill_renderer.start(0x%x)", (void *)this);
	if (m_is_showing) {
		logger::get_logger()->trace("sdl_fill_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	m_is_showing = true;
	if (!m_dest) {
		logger::get_logger()->trace("sdl_fill_renderer.start(0x%x): no surface", (void *)this);
		return;
	}
	if (m_intransition) {
//		m_trans_engine = sdl_transition_engine(m_dest, false, m_intransition);
//		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
	}
	m_dest->show(this);
	assert(m_context);
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
	m_lock.leave();
}

void
sdl_fill_renderer::start_outtransition(lib::transition_info *info)
{
	m_lock.enter();
	m_outtransition = info;
	if (m_outtransition) {
		// XXX Schedule beginning of out transition
		//lib::event *ev = new transition_callback(this, &transition_outbegin);
		//m_event_processor->add_event(ev, XXXX);
	}
	m_lock.leave();
}

bool
sdl_fill_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_fill_renderer.stop(0x%x)", (void *)this);
	if (!m_is_showing) {
		logger::get_logger()->trace("sdl_fill_renderer.stop(0x%x): already stopped", (void*)this);
	} else {
		m_is_showing = false;
		if (m_dest) m_dest->renderer_done(this);
		m_dest = NULL;
	}
	assert(m_context);
	m_context->stopped(m_cookie);
	m_lock.leave();
	return true;
}

void
sdl_fill_renderer::redraw(const rect &dirty, gui_window *window)
{

	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("sdl_fill_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)",(void *)this,r.left(),r.top(),r.right(),r.bottom());
	ambulant_sdl_window* asdlw = (ambulant_sdl_window*) window;
	SDL_Surface* surf = NULL;
//X GdkPixmap *surf = NULL;
	if (m_trans_engine && m_trans_engine->is_done()) {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	// See whether we're in a transition
	if (m_trans_engine) {
/*TBD
		GdkPixmap *qpm = asdlw->get_ambulant_pixmap();
		surf = asdlw->get_ambulant_surface();
		if (surf == NULL)
			surf = asdlw->new_ambulant_surface();
		if (surf != NULL) {
			asdlw->set_ambulant_surface(surf);
			// Copy the background pixels
			rect dstrect = r;
			dstrect.translate(m_dest->get_global_topleft());
			AM_DBG logger::get_logger()->debug("sdl_fill.redraw: bitBlt to=0x%x (%d,%d) from=0x%x (%d,%d,%d,%d)",surf, dstrect.left(), dstrect.top(), qpm,dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
//			bitBlt(surf, dstrect.left(),dstrect.top(),
//				qpm,dstrect.left(),dstrect.top(),dstrect.width(),dstrect.height());
//			bitBlt(surf, dstrect.left(), dstrect.top(),
//				qpm,  dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
			AM_DBG logger::get_logger()->debug("sdl_fill_renderer.redraw: drawing to transition surface");
		}
TBD*/
	}

	redraw_body(dirty, window);

/*TBD
	if (surf != NULL) {
		asdlw->reset_ambulant_surface();
	}
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->debug("sdl_fill_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef no_arg_callback<sdl_fill_renderer>transition_callback;
		event *ev = new transition_callback(this, &sdl_fill_renderer::transition_step);
		transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
//		delay = 500;
		AM_DBG logger::get_logger()->debug("sdl_fill_renderer.redraw: now=%d, schedule step for %d",m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay, ep_low);
	}
TBD*/
	m_lock.leave();
}


void
sdl_fill_renderer::transition_step()
{
	if (m_dest) m_dest->need_redraw();
}

bool
sdl_fill_renderer::user_event(const point &where, int what)
{
	if (!user_event_sensitive(where)) return false;
	if (what == user_event_click) m_context->clicked(m_cookie, 0);
	else if (what == user_event_mouse_over) m_context->pointed(m_cookie, 0);
	else assert(0);
	return true;
}


void
sdl_fill_renderer::redraw_body(const lib::rect &dirty, common::gui_window *window) {

	// <brush> drawing

	AM_DBG lib::logger::get_logger()->debug("sdl_fill_renderer::redraw_body(0x%x)", (void *)this);
	int err = 0;
	const common::region_info *info = m_dest->get_info();
	lib::rect r = m_dest->get_rect();
	ambulant_sdl_window* asw = (ambulant_sdl_window*) window;
	sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
	SDL_Renderer* renderer =  saw != NULL ? saw->get_sdl_renderer() : NULL;
	if (renderer == NULL) {
		return;
	}
	// First find our whole area to be cleared to <brush> color
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	int L = dstrect_whole.left(),
		T = dstrect_whole.top(),
		W = dstrect_whole.width(),
		H = dstrect_whole.height();
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
	AM_DBG lib::logger::get_logger()->debug("sdl_fill_renderer.redraw_body: clearing to 0x%x", (long)color);
	SDL_Rect sdl_dst_rect = {L, T, W, H};
	// Set and draw the background color for the region
	err = SDL_SetRenderDrawColor (renderer, redc(bgcolor), greenc(bgcolor), bluec(bgcolor), bgalpha);
	assert (err==0);
	err = SDL_RenderFillRect (renderer, &sdl_dst_rect);
	assert (err==0);
	// Set and draw the  foreground color for the region
	err = SDL_SetRenderDrawColor (renderer, redc(color), greenc(color), bluec(color), alpha);
	assert (err==0);
	err = SDL_RenderFillRect (renderer, &sdl_dst_rect);
	assert (err==0);
	AM_DBG lib::logger::get_logger()->debug("sdl_fill_renderer.redraw_body(0x%x, local_ltrb=(%d,%d,%d,%d)",(void *)this, L,T,W,H);
	SDL_Rect sdl_src_rect = {0, 0, W, H};
//TBD	err = asw->copy_sdl_surface (m_surface, &sdl_src_rect, &sdl_dst_rect, 255);
//	assert (err==0);
}

sdl_background_renderer::~sdl_background_renderer()
{
}

void
sdl_background_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	if ( !	(m_src && m_dst))
		return;
	const lib::rect &r = m_dst->get_rect();
	AM_DBG lib::logger::get_logger()->debug("sdl_background_renderer::redraw(0x%x)", (void *)this);
	double opacity = m_src->get_bgopacity();
	if (opacity > 0.0) {
	// First find our whole area to be cleared to background color
		ambulant_sdl_window* asw = (ambulant_sdl_window*) window;
		sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
		SDL_Renderer* background_renderer =  saw != NULL ? saw->get_sdl_renderer() : NULL;
		if (background_renderer == NULL) {
			return;
		}
		lib::rect dstrect_whole = r;
		dstrect_whole.translate(m_dst->get_global_topleft());
		int L = dstrect_whole.left(),
			T = dstrect_whole.top(),
			W = dstrect_whole.width(),
			H = dstrect_whole.height();
		// Fill with background color
		lib::color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("sdl_background_renderer::redraw: clearing to %x, asw=0x%x local_ltwh(%d,%d,%d,%d)",(long)bgcolor,(void*)asw,L,T,W,H);

		Uint8 red = redc(bgcolor), green = bluec(bgcolor), blue = greenc(bgcolor);
//TMP		if (background_renderer == NULL) { // TMP disable SDL_Renderer* caching
			background_renderer = saw->get_sdl_renderer();
			if (background_renderer == NULL) {
				return;
			}
//TMP  	}
		// Set and draw the background color for the region
		SDL_Rect sdl_dst_rect = {L, T, W, H};
		int err = 0;
		err = SDL_SetRenderDrawColor (background_renderer, red, green, blue, opacity*255);
		assert (err==0);
//		err = SDL_RenderClear (background_renderer);
		err = SDL_RenderFillRect (background_renderer, &sdl_dst_rect);
		assert (err==0);
		SDL_Rect sdl_src_rect = {0, 0, W, H};
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
/*TBD*/
}

void sdl_background_renderer::highlight(gui_window *window)
{
}

void
sdl_background_renderer::keep_as_background()
{
	AM_DBG lib::logger::get_logger()->debug("sdl_background_renderer::keep_as_background(0x%x) called", (void *)this);
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

#endif//WITH_SDL2
