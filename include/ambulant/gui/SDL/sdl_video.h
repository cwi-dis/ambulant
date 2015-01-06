/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_GUI_SDL_VIDEO_RENDERER_H
#define AMBULANT_GUI_SDL_VIDEO_RENDERER_H

#ifdef WITH_SDL_IMAGE

#include "ambulant/config/config.h"
#include "ambulant/common/video_renderer.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/gui/SDL/sdl_renderer.h"

#include "SDL.h"
// Constants used to define bitmaps as used in ambulant, ffmpeg and SDL2.
// These constants should match.
//
// Note: preferably this information should be obtained dynamically 
// in order to optimally match available hardware
#if 0
#define SDL_PIXEL_LAYOUT net::pixel_rgba
#define SDL_PIXELFORMAT SDL_PIXELFORMAT_RGBA8888
#define SDL_SWS_PIX_FMT PIX_FMT_RGB32_1
#define SDL_BPP 4
#elif 0
#define SDL_PIXEL_LAYOUT net::pixel_bgra
#define SDL_PIXELFORMAT SDL_PIXELFORMAT_BGRA8888
#define SDL_SWS_PIX_FMT PIX_FMT_RGB32_1
#define SDL_BPP 4
#elif 1
#define SDL_PIXEL_LAYOUT net::pixel_argb
#define SDL_PIXELFORMAT SDL_PIXELFORMAT_ARGB8888
#define SDL_SWS_PIX_FMT PIX_FMT_RGB32
#define SDL_BPP 4
#elif 0
#define SDL_PIXEL_LAYOUT net::pixel_abgr
#define SDL_PIXELFORMAT SDL_PIXELFORMAT_ARGB8888
#define SDL_SWS_PIX_FMT PIX_FMT_RGB32
#define SDL_BPP 4
#elif 0
#define SDL_PIXEL_LAYOUT net::pixel_rgb
#define SDL_PIXELFORMAT SDL_PIXELFORMAT_RGB24
#define SDL_SWS_PIX_FMT PIX_FMT_RGB24
#define SDL_BPP 3
#elif 0
#define SDL_PIXEL_LAYOUT net::pixel_bgr
#define SDL_PIXELFORMAT SDL_PIXELFORMAT_BGR24
#define SDL_SWS_PIX_FMT PIX_FMT_RGB24
#define SDL_BPP 3
#else
#error No bitmap representation defined
#endif

namespace ambulant {

namespace gui {

namespace sdl {


class sdl_video_renderer : public sdl_renderer<common::video_renderer>  {
  public:
	sdl_video_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp);

	~sdl_video_renderer();
	net::pixel_order pixel_layout();
	void redraw_body(const lib::rect &r, common::gui_window* w);

  protected:
	void _push_frame(char* frame, size_t size);
private:
	void render_frame();
	net::pixel_order get_pixel_order_from_SDL_PixelFormat (SDL_PixelFormat*);
	long int m_img_displayed;
	char* m_data;
	unsigned int m_datasize;
	net::pixel_order m_pixel_order;
	SDL_Surface* m_data_surface; // video frame pixel data
	Uint32 m_Rmask, m_Gmask, m_Bmask, m_Amask;
	int m_bits_per_pixel, m_bytes_per_pixel;
};


} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif // WITH_SDL_IMAGE

#endif // AMBULANT_GUI_SDL_VIDEO_RENDERER_H
