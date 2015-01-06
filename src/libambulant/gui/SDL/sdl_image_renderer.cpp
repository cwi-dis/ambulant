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
#include "ambulant/gui/SDL/sdl_image_renderer.h"
#include "ambulant/gui/SDL/sdl_transition.h"
#include "ambulant/gui/SDL/sdl_window.h"
//#include "ambulant/gui/SDL/sdl_util.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/smil2/test_attrs.h"
#include "SDL_image.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui::sdl;

extern const char sdl_image_playable_tag[] = "img";
extern const char sdl_image_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererSdl");
extern const char sdl_image_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererImg");

common::playable_factory *
gui::sdl::create_sdl_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSdl"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererImg"), true);
	return new common::single_playable_factory<
		sdl_image_renderer,
		sdl_image_playable_tag,
		sdl_image_playable_renderer_uri,
		sdl_image_playable_renderer_uri2,
		sdl_image_playable_renderer_uri2>(factory, mdp);
}

sdl_image_renderer::~sdl_image_renderer() {
	AM_DBG lib::logger::get_logger()->debug("sdl_image_renderer::~sdl_image_renderer(%p), %s, m_image=%p", this, this->get_sig().c_str(), m_image);
	m_lock.enter();
	if (m_image != NULL) { 
		SDL_FreeSurface(m_image);
		m_image = NULL;
	}
	m_lock.leave();
}

void
sdl_image_renderer::redraw_body(const rect &dirty, gui_window* w) {

	m_lock.enter();
	const point p = m_dest->get_global_topleft();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("sdl_image_renderer.redraw_body(%p): m_image=%p, ltrb=(%d,%d,%d,%d), p=(%d,%d) sig=%s", (void *)this, &m_image,r.left(), r.top(), r.right(), r.bottom(),p.x,p.y,m_node->get_sig().c_str());

// XXXX WRONG! This is the info for the region, not for the node!
	const common::region_info *info = m_dest->get_info();
	AM_DBG logger::get_logger()->debug("sdl_image_renderer.redraw_body: gui_window=%p info=%p",w,info);
	ambulant_sdl_window* asw = (ambulant_sdl_window*) w;
	sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();

	if (m_data && !m_image_loaded && m_data_size > 0) {
		AM_DBG logger::get_logger()->debug("sdl_image_renderer.redraw_body(%p): load data for %s", this, this->get_sig().c_str());
		SDL_RWops* rwops = SDL_RWFromMem (m_data, m_data_size);
		assert (rwops != NULL);
		m_image = IMG_Load_RW(rwops, 1);
//		rwops->close (rwops);
		if (m_image == NULL) {
			logger::get_logger()->debug("sdl_image_renderer.redraw_body(%p): IMG_Load_RW failed. %s", this, this->get_sig().c_str());
		} else {
			// convert the image to RGBA compatible format (necessary for blending)
#ifdef WITH_DYNAMIC_PIXEL_LAYOUT
			SDL_PixelFormat* new_pixel_format = saw->get_window_pixel_format();
#else //WITH_DYNAMIC_PIXEL_LAYOUT
			SDL_PixelFormat* new_pixel_format = (SDL_PixelFormat*) malloc (sizeof(SDL_PixelFormat));
			*new_pixel_format = *saw->get_sdl_surface()->format;
#endif//WITH_DYNAMIC_PIXEL_LAYOUT
			SDL_Surface* new_image = SDL_ConvertSurface (m_image, new_pixel_format, 0);
			if (new_image != NULL && new_image != m_image) {
				AM_DBG lib::logger::get_logger()->debug("sdl_image_renderer::redraw_body(%p), %s, SDL_FreeSurface(m_image=%p)", this, this->get_sig().c_str(), m_image);
				SDL_FreeSurface (m_image);
				m_image = new_image;
			}
			// enable alpha blending for this image
//			asw->ambulant_sdl_window::dump_sdl_surface(m_image, "image");
			SDL_SetSurfaceBlendMode(m_image, SDL_BLENDMODE_BLEND);
			AM_DBG lib::logger::get_logger()->debug("sdl_image_renderer::redraw_body(%p), %s, m_image=%p", this, this->get_sig().c_str(), m_image);
			m_image_loaded = true;
		}
	}
	if ( ! m_image_loaded) {
		// Initially the image may not yet be loaded
		m_lock.leave();
		return;
	} 
	int width = m_image->w;
	int height = m_image->h;
	size srcsize = size(width, height);
	rect srcrect;
	rect dstrect;

	// While rendering background images only, check for tiling. This code is
	// convoluted, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") && m_dest->is_tiled()) {
		//TBD
		AM_DBG lib::logger::get_logger()->debug("sdl_image_renderer.redraw: drawing tiled image");
		dstrect = m_dest->get_rect();
		dstrect.translate(m_dest->get_global_topleft());
		common::tile_positions tiles = m_dest->get_tiles(srcsize, dstrect);
		common::tile_positions::iterator it;
		for(it=tiles.begin(); it!=tiles.end(); it++) {
			srcrect = (*it).first;
			dstrect = (*it).second;
			int S_L = srcrect.left(),
				S_T = srcrect.top(),
				S_W = srcrect.width(),
				S_H = srcrect.height();
			int D_L = dstrect.left(),
				D_T = dstrect.top(),
				D_W = dstrect.width(),
				D_H = dstrect.height();
			AM_DBG lib::logger::get_logger()->debug("sdl_image_renderer.redraw_body(%p): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H);
#ifdef TBD // tiled bg
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (asw->get_ambulant_pixmap()));
			gdk_pixbuf_render_to_drawable(
				m_image,
				GDK_DRAWABLE (asw->get_ambulant_pixmap()),
				gc,
				S_L, S_T,
				D_L, D_T,
				D_W, D_H,
				GDK_RGB_DITHER_NONE,
				0,
				0);
			gdk_pixbuf_unref (m_image);
			g_object_unref (G_OBJECT (gc));
#endif//TBD
		}
		m_lock.leave();
		return;
	}
	srcrect = rect(size(0,0));
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	dstrect = m_dest->get_fit_rect(croprect, srcsize, &srcrect, m_alignment);
	dstrect.translate(m_dest->get_global_topleft());

	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		} else alpha_chroma = alpha_media;
	}
	if (srcrect.w == 0 || srcrect.h == 0 || dstrect.w == 0 || dstrect.h == 0) {
		// either nothing to redraw from source or to destination)
		return;
	}
	lib::rect clip_rect = dirty;
	clip_rect.translate(m_dest->get_global_topleft());
	SDL_Rect sdl_srcrect = SDL_Rect_from_ambulant_rect (srcrect);
	SDL_Rect sdl_dstrect = SDL_Rect_from_ambulant_rect (dstrect);
	SDL_Rect sdl_cliprect = SDL_Rect_from_ambulant_rect (clip_rect);
	
	if (alpha_chroma != 1.0) { //TBD chroma keying
	} else {
	}
//		saw->dump_sdl_surface(m_image, "uimg");  // use this for debugging
	saw->copy_to_sdl_surface (m_image, &sdl_srcrect, &sdl_dstrect, 255 * alpha_media, &sdl_cliprect);
//	saw->dump_sdl_surface(saw->get_sdl_surface(), "surf");  // use this for debugging
	AM_DBG lib::logger::get_logger()->debug("sdl_image_renderer.redraw_body(%p done.", this);
	m_lock.leave();
}

#endif// defined(WITH_SDL_IMAGE)
