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

#ifndef AMBULANT_GUI_D2_SMILTEXT_H
#define AMBULANT_GUI_D2_SMILTEXT_H

#include "ambulant/config/config.h"
#include "ambulant/gui/d2/d2_renderer.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/smil2/smiltext.h"

interface IDWriteFactory;
interface IDWriteTextFormat;
interface IDWriteTextLayout;
interface IDWriteFontCollection;
interface ID2D1SolidColorBrush;

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace d2 {

common::playable_factory *create_d2_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

class d2_range_params;

class d2_smiltext_renderer : 
	public d2_renderer<renderer_playable>,
	public smil2::smiltext_notification
{
  public:
	d2_smiltext_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp);
	~d2_smiltext_renderer();

	void init_with_node(const lib::node *n);
	void redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget*);

	void start(double t);
	void seek(double t);
	bool stop();
	void pause(pause_display d=display_show);
	void resume();

	// Callbacks from the SMILText engine
	void smiltext_changed();
	void marker_seen(const char *name);

	// Direct2D resource management
	void recreate_d2d();
	void discard_d2d();
  private:
    bool _collect_text();
	void _recreate_layout();
	void _discard_range_params();
	unsigned int _compute_rate(smil2::smiltext_align align, lib::size size, lib::rect r,  unsigned int dur); // Must go to engine

	lib::color_t m_text_color;

	static IDWriteFactory *s_write_factory;

	IDWriteTextFormat *m_text_format;	// The default formatting instructions
	std::wstring m_data;	// The text to show
	std::vector<d2_range_params*> m_range_params;	// Per-range parameters
	IDWriteTextLayout *m_text_layout;	// The layout engine
	std::vector<int> m_run_begins;	// Starting point of each run
	ID2D1SolidColorBrush *m_brush;	// Default brush
	
	smil2::smiltext_engine m_engine;
	bool m_needs_conditional_newline;
	bool m_needs_conditional_space;
	const smil2::smiltext_params &m_params;
	void *m_cur_paragraph_style;
	smil2::smiltext_align m_cur_para_align;
	smil2::smiltext_writing_mode m_cur_para_writing_mode;
	bool m_cur_para_wrap;
	bool m_render_offscreen;			// True if m_params does not allows rendering in-place
	lib::timer::time_type m_epoch;
	bool m_any_semiopaque_bg;			// True if any backgroundOpacity != 1.0 is used

	critical_section m_lock;
};

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_SMILTEXT_H
