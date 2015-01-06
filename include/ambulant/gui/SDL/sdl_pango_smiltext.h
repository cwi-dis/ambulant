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

#ifndef AMBULANT_GUI_SDL_SDL_SMILTEXT_H
#define AMBULANT_GUI_SDL_SDL_SMILTEXT_H

#if defined(WITH_SDL_IMAGE) && defined(WITH_SDL_PANGO)
//TBD: offscreen rendering

#include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_window.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/smil2/smiltext.h"

#include <pango-1.0/pango/pango.h>
#define __PANGO_H__ // this reveals some useful functions we need to use
#include <SDL_Pango.h>

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace sdl {

class sdl_smiltext_renderer :
	public sdl_renderer<renderer_playable>,
	public smil2::smiltext_notification
{
  public:
	sdl_smiltext_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp);
	~sdl_smiltext_renderer();

	void redraw_body(const rect &dirty, gui_window *window);

	void start(double t);
	void seek(double t);
	bool stop();
	// Callback from the engine
	void smiltext_changed();
	void marker_seen(const char *name);
  private:
	void _sdl_smiltext_changed();
	void _sdl_smiltext_render(const lib::rect r, const lib::point offset,
		ambulant_sdl_window* window);

	std::string m_text_storage;
	smil2::smiltext_engine m_engine;
	const smil2::smiltext_params& m_params;
//TBD 	bool m_render_offscreen; // True if m_params does not allow rendering in-place
	lib::timer::time_type m_epoch;

// pango specific stuff
	void _sdl_set_color_attr(
		PangoAttrList* pal, lib::color_t smiltext_color,
		PangoAttribute* (*pango_attr_color)(guint16 r,guint16 g,guint16 b),
		unsigned int start_index, unsigned int end_index);
	void _sdl_set_font_attr(PangoAttrList* pal, const char* smiltext_font_family,
		smil2::smiltext_font_style smiltext_font_style,
		smil2::smiltext_font_weight smiltext_font_weight,
		int smiltext_font_size,
		unsigned int start_index, unsigned int end_index);

	SDLPango_Context* m_sdl_pango_context;
	PangoAttrList* m_pango_attr_list;
	PangoContext* m_pango_context;
	PangoLayout* m_pango_layout;
	PangoAttrList* m_bg_pango_attr_list;
	PangoLayout* m_bg_layout;

	const color_t m_transparent; // needed for blending
	const color_t m_alternative; // when m_transparent to be drawn
	double	m_alpha_media;
	double	m_alpha_media_bg;
	double	m_alpha_chroma;
	color_t m_chroma_low;
	color_t m_chroma_high;
	smil2::smiltext_align m_align;
	smil2::smiltext_writing_mode m_writing_mode;
	bool m_needs_conditional_space;
	bool m_needs_conditional_newline;
	bool m_wrap;
	bool m_is_changed;
	bool m_motion_done;
	lib::point m_start;
	lib::point m_origin;
	lib::rect  m_log_rect;
};

} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif // defined(WITH_SDL_IMAGE) && defined(WITH_SDL_PANGO)

#endif // AMBULANT_GUI_SDL_SDL_SMILTEXT_H
