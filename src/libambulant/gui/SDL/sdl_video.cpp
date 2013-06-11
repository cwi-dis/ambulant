/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

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

//X #ifdef  WITH_SDL2 
//X #include "ambulant/gui/sdl/sdl_factory.h"
//X #include "ambulant/gui/sdl/sdl_includes.h"
//X #include "ambulant/gui/sdl/sdl_renderer.h"
#define __STDC_CONSTANT_MACROS //XXXX Grrr.. for ‘UINT64_C’ not declared

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#ifndef AV_NUM_DATA_POINTERS
// needed for older versions of ffmpeg (< 0.9)
#define AV_NUM_DATA_POINTERS 4
#endif// !  AV_NUM_DATA_POINTERS
};
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

sdl_video_renderer::sdl_video_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	// No transitions yet
    // sdl_renderer<common::video_renderer>(context, cookie, node, evp, factory, mdp),
  	common::video_renderer(context, cookie, node, evp, factory, mdp),

	m_data(NULL),
	m_datasize(0),
	m_img_displayed(0),
	m_sws_ctx(0)
{
	SDL_Init(SDL_INIT_VIDEO);
}

sdl_video_renderer::~sdl_video_renderer()
{	
	m_lock.enter();
	if (m_data)
		free(m_data);
	if (m_sws_ctx) {
		sws_freeContext (m_sws_ctx);
		m_sws_ctx = NULL;
	}
	m_lock.leave();
}

net::pixel_order
sdl_video_renderer::pixel_layout()
{
	return SDL_PIXEL_LAYOUT;
}

void
sdl_video_renderer::_push_frame(char* frame, size_t size)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer::_push_frame: frame=0x%x, size=%d, this=0x%x", (void*) frame, size, (void*) this);
	if (m_data)
		free(m_data);
	m_data = frame;
	m_datasize = size;
}


void
//sdl_video_renderer::redraw_body(const lib::rect &dirty, common::gui_window* w)
sdl_video_renderer::redraw(const lib::rect &dirty, common::gui_window* w)
{
	//XXXX locking at this point may result in deadly embrace with internal lock,
	//XXXX but as far as we know this has never happened
	m_lock.enter();
	if (m_data){
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw(0x%x)",(void*) this);
		const lib::point p = m_dest->get_global_topleft();
		const lib::rect &r = m_dest->get_rect();
		lib::rect dst_rect_whole = r;

		dst_rect_whole.translate(p);
		// XXXX WRONG! This is the info for the region, not for the node!
		const common::region_info *info = m_dest->get_info();
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw: info=0x%x", info);
		// background drawing
		if (info && (info->get_bgopacity() > 0.5)) {
				// XXXX Fill with background color TBD
				lib::color_t bgcolor = info->get_bgcolor();
		}
		lib::rect src_rect; // lib::rect(lib::point(0,0), lib::size(width, height)), dst_rect;
		lib::rect croprect = m_dest->get_crop_rect(m_size);
		lib::rect dst_rect = m_dest->get_fit_rect(croprect, m_size, &src_rect, m_alignment);
		if (src_rect.w == 0 || src_rect.h == 0 || dst_rect.w == 0 || dst_rect.h == 0) {
				// either nothing to redraw from source or to destination)
				return;
		}
		dst_rect.translate(p);
		int dst_width = dst_rect.w;
		int dst_height = dst_rect.h;
		int src_width = src_rect.w;
		int src_height = src_rect.h;
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw_body(0x%x): dst_width=%d, dst_height=%d, src_width=%d, src_height=%d",(void *)this, dst_width, dst_height, src_width, src_height);

		ambulant_sdl_window* asw = (ambulant_sdl_window*) w;
		SDL_Renderer* renderer = asw->get_sdl_ambulant_window()->get_sdl_renderer();
		SDL_Surface* surface = NULL;
  
		// prevent warnings: Forcing full internal H chroma due to odd output size
		int flags = SWS_FAST_BILINEAR | SWS_FULL_CHR_H_INT;
		m_sws_ctx = sws_getCachedContext(m_sws_ctx, src_width, src_height, SDL_SWS_PIX_FMT, dst_width, dst_height, SDL_SWS_PIX_FMT, flags, NULL, NULL, NULL);
		uint8_t* pixels[AV_NUM_DATA_POINTERS];
		int dst_stride[AV_NUM_DATA_POINTERS];
		int src_stride[AV_NUM_DATA_POINTERS];
		dst_stride[0] = dst_width*SDL_BPP;
		src_stride[0] = m_size.w*SDL_BPP;
		for (int i = 1; i < AV_NUM_DATA_POINTERS; i++) {
				pixels[i] = NULL;
				dst_stride[i] = src_stride[i] = 0;
		}
		pixels[0] = (uint8_t*) malloc(dst_stride[0]*dst_height); 
		int rv = sws_scale(m_sws_ctx,(const uint8_t* const*) &m_data, src_stride, 0, src_height, pixels, dst_stride);
		AM_DBG { 		static int old_src, old_dst; if (old_src != src_width || old_dst != dst_width) { old_src = src_width; old_dst = dst_width; lib::logger::get_logger()->debug("ambulant_sdl_video::redraw(0x%x) src=%dx%d dst=%dx%d rv=%d src_stride=%d", this, src_width, src_height, dst_width, dst_height,rv,src_stride[0]); }}
		dst_rect.h = dst_height = rv;
		Uint32 rmask, gmask, bmask, amask;
		// we use ARGB
		amask = 0xff000000;
		rmask = 0x00ff0000;
		gmask = 0x0000ff00;
		bmask = 0x000000ff;     
		// convert to SDL
		surface = SDL_CreateRGBSurfaceFrom(pixels[0], dst_width, dst_height, 32, dst_stride[0], rmask, gmask, bmask, amask);
		SDL_Rect sdl_dst_rect = {dst_rect.left(), dst_rect.top(), dst_width, dst_height};
		SDL_Rect sdl_src_rect = {src_rect.left(), src_rect.top(), src_width, src_height};
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_video::redraw(0x%x) sdl_dst_rect={%d,%d,%d,%d}", this, sdl_dst_rect.x, sdl_dst_rect.y, sdl_dst_rect.w, sdl_dst_rect.h);
		sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
//		saw->dump_sdl_surface(surface, "surf");  // use this for debugging
		saw->copy_to_sdl_surface (surface, NULL, &sdl_dst_rect, 255 * (info?info->get_mediaopacity():1.0));
		SDL_FreeSurface(surface);
		free (pixels[0]);
	}
	m_lock.leave();
}

//#endif//WITH_SDL2
