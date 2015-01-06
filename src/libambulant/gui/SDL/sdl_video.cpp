/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

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
#include "ambulant/gui/SDL/sdl_video.h"
#include "ambulant/gui/SDL/sdl_window.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/factory.h"
#include <stdlib.h>
#include "ambulant/common/playable.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::sdl;

extern const char sdl_video_playable_tag[] = "video";
extern const char sdl_video_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererSdl");
extern const char sdl_video_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererVideo");
extern const char sdl_video_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererOpen");

common::playable_factory *
gui::sdl::create_sdl_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererGtk"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererOpen"), true);
	return new common::single_playable_factory<
		sdl_video_renderer,
		sdl_video_playable_tag,
		sdl_video_playable_renderer_uri,
		sdl_video_playable_renderer_uri2,
		sdl_video_playable_renderer_uri3>(factory, mdp);
}

sdl_video_renderer::sdl_video_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:
    sdl_renderer<common::video_renderer>(context, cookie, node, evp, factory, mdp),

	m_img_displayed(0),
	m_data(NULL),
	m_datasize(0),
	m_pixel_order(net::pixel_unknown),
	m_data_surface(NULL),
	m_Rmask(0),
	m_Gmask(0),
	m_Bmask(0),
	m_Amask(0),
#ifdef WITH_DYNAMIC_PIXEL_LAYOUT
	m_bits_per_pixel(0),
	m_bytes_per_pixel(0)
#else //WITH_DYNAMIC_PIXEL_LAYOUT
	m_bits_per_pixel(32),
	m_bytes_per_pixel(SDL_BPP)
#endif//WITH_DYNAMIC_PIXEL_LAYOUT
{
	SDL_Init(SDL_INIT_VIDEO);
}

sdl_video_renderer::~sdl_video_renderer()
{	
	m_lock.enter();
	if (m_data)
		free(m_data);
	m_lock.leave();
}

net::pixel_order
sdl_video_renderer::pixel_layout()
{
#ifdef WITH_DYNAMIC_PIXEL_LAYOUT
	if (m_pixel_order == net::pixel_unknown) {
		ambulant_sdl_window* asw = (ambulant_sdl_window*)  m_dest->get_gui_window();
		sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();

		m_pixel_order = get_pixel_order_from_SDL_PixelFormat (saw->get_window_pixel_format());
	}
	return m_pixel_order;
#else //WITH_DYNAMIC_PIXEL_LAYOUT
	return SDL_PIXEL_LAYOUT;
#endif//WITH_DYNAMIC_PIXEL_LAYOUT
}

net::pixel_order
sdl_video_renderer::get_pixel_order_from_SDL_PixelFormat (SDL_PixelFormat* sdl_pf)
{
	net::pixel_order rv = net::pixel_unknown;
	if (sdl_pf != NULL) {
		Uint32 sdl_window_pixel_format = sdl_pf->format, Rmask, Gmask, Bmask, Amask;

		switch (sdl_window_pixel_format) {
		case  SDL_PIXELFORMAT_RGBA8888:
			rv =  net::pixel_rgba;
			m_bytes_per_pixel = 4;
			break;
		case  SDL_PIXELFORMAT_RGBX8888:
			rv =  net::pixel_rgbx;
			m_bytes_per_pixel = 4;
			break;
		case  SDL_PIXELFORMAT_BGRA8888:
			rv = net::pixel_bgrx;
			m_bytes_per_pixel = 4;
			break;
		case  SDL_PIXELFORMAT_BGRX8888:
			rv = net::pixel_bgra;
			m_bytes_per_pixel = 4;
			break;
		case  SDL_PIXELFORMAT_ARGB8888:
			rv = net::pixel_argb;
			m_bytes_per_pixel = 4;
			break;
		case  SDL_PIXELFORMAT_RGB888:
			rv = net::pixel_xrgb;
			m_bytes_per_pixel = 4;
			break;
		case  SDL_PIXELFORMAT_ABGR8888:
			rv = net::pixel_abgr;
			m_bytes_per_pixel = 4;
			break;
		case  SDL_PIXELFORMAT_BGR888:
			rv = net::pixel_xbgr;
			m_bytes_per_pixel = 4;
			break;
		case  SDL_PIXELFORMAT_RGB24:
			rv = net::pixel_rgb;
			m_bytes_per_pixel = 3;
			break;
		case  SDL_PIXELFORMAT_BGR24:
			rv = net::pixel_bgr;
			m_bytes_per_pixel = 3;
			break;
		}
		// Find parameters for callling SDL_CreateRGBSurface() later, when we have data
		bool ok = SDL_PixelFormatEnumToMasks(sdl_pf->format, &m_bits_per_pixel, &m_Rmask, &m_Gmask, &m_Bmask, &m_Amask);
		if ( ! ok) {
			lib::logger::get_logger()->trace ("%s: %s failed, error: %s",__PRETTY_FUNCTION__, "SDL_PixelFormatEnumToMasks", SDL_GetError());
			return rv;
		}
	}
	return rv;
}

void
sdl_video_renderer::_push_frame(char* frame, size_t size)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer::_push_frame: frame=%p, size=%d, this=%p", (void*) frame, size, (void*) this);
	if (m_data)
		free(m_data);
	m_data = frame;
	m_datasize = size;
}

void
sdl_video_renderer::redraw_body(const lib::rect &dirty, common::gui_window* w)
//sdl_video_renderer::redraw(const lib::rect &dirty, common::gui_window* w)
{
	//XXXX locking at this point may result in deadly embrace with internal lock,
	//XXXX but as far as we know this has never happened
	m_lock.enter();
	if (m_data){
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw_body(%p)",(void*) this);
		ambulant_sdl_window* asw = (ambulant_sdl_window*) w;
		sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();

		_frame_was_displayed();
		const lib::rect &r = m_dest->get_rect();
		lib::rect dst_rect_whole = r;
		lib::rect clip_rect = dirty;
		const lib::point p = m_dest->get_global_topleft();
		clip_rect.translate(p);
		
		// XXXX WRONG! This is the info for the region, not for the node!
		const common::region_info *info = m_dest->get_info();
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw_body: info=%p", info);
		// background drawing
		if (info && (info->get_bgopacity() > 0.5)) {
			// XXXX Fill with background color TBD
			lib::color_t bgcolor = info->get_bgcolor();
		}
		lib::rect src_rect = lib::rect(lib::size(0,0));
		lib::rect croprect = m_dest->get_crop_rect(m_size);
		lib::rect dst_rect = m_dest->get_fit_rect(croprect, m_size, &src_rect, m_alignment);
		dst_rect.translate(p);
		if (src_rect.w == 0 || src_rect.h == 0 || dst_rect.w == 0 || dst_rect.h == 0) {
			// either nothing to redraw from source or to destination)
			return;
		}
		SDL_Rect sdl_dst_rect = SDL_Rect_from_ambulant_rect(dst_rect);
		SDL_Rect sdl_src_rect = SDL_Rect_from_ambulant_rect(src_rect);
		SDL_Rect sdl_clip_rect = SDL_Rect_from_ambulant_rect(clip_rect);
		// we use ARGB
		Uint32 rmask, gmask, bmask, amask;
		amask = 0xff000000;
		rmask = 0x00ff0000;
		gmask = 0x0000ff00;
		bmask = 0x000000ff;     
		
        int dst_width = dst_rect.w;
		int dst_height = dst_rect.h;
		int src_width = src_rect.w;
		int src_height = src_rect.h;
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw_body(%p): dst_width=%d, dst_height=%d, src_width=%d, src_height=%d",(void *)this, dst_width, dst_height, src_width, src_height);
		bool sdl_surface_created = false;
		if (m_data_surface != NULL && (m_size.w != m_data_surface->w  || m_size.h != m_data_surface->h)) {
			// source size changed
			SDL_FreeSurface (m_data_surface);
			m_data_surface = NULL;
		}
		if (m_data_surface == NULL) {
			//XXXX is this correct ? should it be rounded up on word boundary ?
			int pitch = m_size.w*m_bytes_per_pixel;
			m_data_surface = SDL_CreateRGBSurfaceFrom(m_data, m_size.w, m_size.h, m_bits_per_pixel, pitch, m_Rmask, m_Gmask, m_Bmask, m_Amask);
			if (m_data_surface != NULL) {
				m_data_surface->format->format &= ~0xFF;
				m_data_surface->format->format |= m_bytes_per_pixel;
				lib::logger::get_logger()->trace ("%s: m_data_surface.format=%s", __PRETTY_FUNCTION__, SDL_GetPixelFormatName(m_data_surface->format->format));
				sdl_surface_created = true;
			} else {
				lib::logger::get_logger()->trace ("%s: %s failed, error: %s",__PRETTY_FUNCTION__, "SDL_CreateRGBSurfaceFrom", SDL_GetError());
			}
		}			
		if (m_data_surface != NULL) {
			m_data_surface->pixels = m_data;
			m_data_surface->w = m_size.w;
			m_data_surface->h = m_size.h;
			saw->copy_to_sdl_surface (m_data_surface, &sdl_src_rect, &sdl_dst_rect, 255 * (info?info->get_mediaopacity():1.0), &sdl_clip_rect);
			AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_video::redraw_body(%p) sdl_src_rect={%d,%d,%d,%d},  sdl_dst_rect={%d,%d,%d,%d}, sdl_clip_rect={%d,%d,%d,%d}, croprect={%d,%d,%d,%d}", this, sdl_src_rect.x, sdl_src_rect.y, sdl_src_rect.w, sdl_src_rect.h, sdl_dst_rect.x, sdl_dst_rect.y, sdl_dst_rect.w, sdl_dst_rect.h, sdl_clip_rect.x, sdl_clip_rect.y, sdl_clip_rect.w, sdl_clip_rect.h, croprect.top(), croprect.left(), croprect.width(), croprect.height());
#ifndef WITH_DYNAMIC_PIXEL_LAYOUT
			if (sdl_surface_created) {
				SDL_FreeSurface(m_data_surface);
				m_data_surface = NULL;
			}
#endif// ! WITH_DYNAMIC_PIXEL_LAYOUT
			AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_video::redraw_body(%p) sdl_dst_rect={%d,%d,%d,%d}", this, sdl_dst_rect.x, sdl_dst_rect.y, sdl_dst_rect.w, sdl_dst_rect.h);
		}
	}
	m_lock.leave();
}

#endif // defined(WITH_SDL_IMAGE)
