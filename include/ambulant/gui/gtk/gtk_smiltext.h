/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_GTK_GTK_SMILTEXT_H
#define AMBULANT_GUI_GTK_GTK_SMILTEXT_H

#ifdef WITH_SMIL30

#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/smil2/smiltext.h"

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace gtk {

class gtk_smiltext_renderer : 
	public gtk_renderer<renderer_playable>,
	public smil2::smiltext_notification
{
  public:
	gtk_smiltext_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp);
        ~gtk_smiltext_renderer();
	
    	void redraw_body(const rect &dirty, gui_window *window);
    	void start(double t);
	void seek(double t);
	void stop();
	// Callback from the engine
	void smiltext_changed();
  private:
	void gtk_smiltext_render(const lib::rect r, ambulant_gtk_window* window);
//JUNK	ambulant_gtk_window* m_gtk_window;
	std::string m_text_storage;
	smil2::smiltext_engine m_engine;
	const smil2::smiltext_params& m_params;
//JUNK	bool m_render_offscreen; // True if m_params does not allow rendering in-place
	lib::timer::time_type m_epoch;
	critical_section m_lock;
// pango specific stuff
	PangoAttrList* m_attr;
};

} // namespace gtk

} // namespace gui
 
} // namespace ambulant

#endif // WITH_SMIL30

#endif // AMBULANT_GUI_GTK_GTK_SMILTEXT_H
