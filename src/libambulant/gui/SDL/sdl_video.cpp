/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

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

#ifdef  WITH_SDL2 
//X #include "ambulant/gui/sdl/sdl_factory.h"
//X #include "ambulant/gui/sdl/sdl_includes.h"
//X #include "ambulant/gui/sdl/sdl_renderer.h"
#define __STDC_CONSTANT_MACROS //XXXX Grrr.. for ‘UINT64_C’ not declared
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_video.h"
#include "ambulant/gui/SDL/sdl_window.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/factory.h"
#include <stdlib.h>
#include "ambulant/common/playable.h"
#include "ambulant/smil2/test_attrs.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
};

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
   	m_img_displayed(0)
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
	//XXXXX m_lock.enter(); TMP disabled
	if (m_data){
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw(0x%x)",(void*) this);

		const lib::point p = m_dest->get_global_topleft();
		const lib::rect &r = m_dest->get_rect();
		lib::rect dstrect_whole = r;
		dstrect_whole.translate(p);
		int L = dstrect_whole.left(),
			T = dstrect_whole.top(),
			W = dstrect_whole.width(),
			H = dstrect_whole.height();

		// XXXX WRONG! This is the info for the region, not for the node!
		const common::region_info *info = m_dest->get_info();
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw: info=0x%x", info);
		// background drawing
		if (info && (info->get_bgopacity() > 0.5)) {
			// First find our whole area (which we have to clear to
			// background color)
			// XXXX Fill with background color
			lib::color_t bgcolor = info->get_bgcolor();
//			AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw: clearing to 0x%x", (long)bgcolor);
//X			GdkColor bgc;
//X			bgc.red = redc(bgcolor)*0x101;
//X			bgc.blue = bluec(bgcolor)*0x101;
//X			bgc.green = greenc(bgcolor)*0x101;
//X			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (asdlw->get_ambulant_pixmap()));
//X			gdk_gc_set_rgb_fg_color (gc, &bgc);
//X			gdk_draw_rectangle (GDK_DRAWABLE (asdlw->get_ambulant_pixmap()), gc, TRUE, L, T, W, H);
//X			g_object_unref (G_OBJECT (gc));
		}
		int width = m_size.w;
		int height = m_size.h;
		AM_DBG lib::logger::get_logger()->debug("sdl_video_renderer.redraw_body(0x%x): width = %d, height = %d",(void *)this, width, height);
		ambulant_sdl_window* asw = (ambulant_sdl_window*) w;
/*
		static SDL_Texture* s_texture = NULL; //XXXX member !
		if (s_texture == NULL) {			
			ambulant_sdl_window* asw = (ambulant_sdl_window*) w;
			s_texture = SDL_CreateTexture(asw->get_sdl_ambulant_window()->get_sdl_renderer(), SDL_PIXELFORMAT, SDL_TEXTUREACCESS_STREAMING, width, height);
		}
		assert(s_texture);
*/
		SDL_Renderer* renderer = asw->get_sdl_ambulant_window()->get_sdl_renderer();
		SDL_Surface* surface = NULL;
//		SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT, SDL_TEXTUREACCESS_STREAMING, width, height);
		static struct SwsContext* s_sws_ctx = NULL; //XXX member !
		s_sws_ctx = sws_getCachedContext(s_sws_ctx, width, height, SDL_SWS_PIX_FMT, width, height, SDL_SWS_PIX_FMT, SWS_BICUBIC, NULL, NULL, NULL);
		uint8_t* pixels[AV_NUM_DATA_POINTERS];
		int pitch[AV_NUM_DATA_POINTERS];
		int stride[AV_NUM_DATA_POINTERS];
		pitch[0] = stride[0] = width*SDL_BPP;
		for (int i = 1; i < AV_NUM_DATA_POINTERS; i++) {
			pixels[i] = NULL;
			pitch[i] = stride[i] = 0;
		}
		pixels[0] = (uint8_t*) malloc(stride[0]*height); 
//		SDL_LockTexture(texture, NULL/*SDL_Rect*/, (void**)&pixels, &pitch[0]);
		int rv = sws_scale(s_sws_ctx,(const uint8_t* const*) &m_data, stride, 0, height, pixels, pitch);
		Uint32 rmask, gmask, bmask, amask;
		// we use ARGB
		amask = 0xff000000;
		rmask = 0x00ff0000;
		gmask = 0x0000ff00;
		bmask = 0x000000ff;

		surface = SDL_CreateRGBSurfaceFrom(pixels[0], W, H, 32, pitch[0], rmask, gmask, bmask, 0);
//		SDL_UnlockTexture(texture);
//		SDL_DestroyTexture(texture);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);		
//TBD	sws_freeContext(sws_ctx);
		SDL_Rect rect;
		rect.x = L;
		rect.y = T;
		rect.w = W;
		rect.h = H;
		SDL_RenderCopy(renderer, texture, NULL, &rect);
		SDL_DestroyTexture(texture);
		SDL_FreeSurface(surface);
		free (pixels[0]);
	}
	//XXXXX m_lock.leave();
}

#endif//WITH_SDL2
