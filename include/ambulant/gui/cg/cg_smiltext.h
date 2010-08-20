/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
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

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_CG_CG_SMILTEXT_H
#define AMBULANT_GUI_CG_CG_SMILTEXT_H

#ifdef WITH_SMIL30

#include "ambulant/gui/cg/cg_renderer.h"
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/smil2/smiltext.h"
#ifdef __OBJC__
#ifdef WITH_UIKIT
#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>
#include <CoreText/CTLine.h>
#include <UIKit/UIKit.h>
#define VIEW_SUPERCLASS UIView
//XXXX inline CGRect CGRectFromViewRect(ambulant::lib::rect rect) { return rect; }
//XXXX inline CGRect ViewRectFromCGRect(CGRect rect) { return rect; }
//XXXX inline CGPoint CGPointFromViewPoint(CGPoint point) { return point; }
//XXXX inline CGSize CGSizeFromViewSize(CGSize size) { return size; }
#else
#include <AppKit/AppKit.h>
#define VIEW_SUPERCLASS NSView
//XXXX inline CGRect CGRectFromViewRect(NSRect rect) { return *(CGRect*)&rect; }
//XXXX inline NSRect ViewRectFromCGRect(CGRect rect) { return *(NSRect*)&rect; }
//XXXX inline CGPoint CGPointFromViewPoint(NSPoint point) { return *(CGPoint*)&point; }
//XXXX inline CGSize CGSizeFromViewSize(NSSize size) { return *(CGSize*)&size; }
#endif
#endif // __OBJC__

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace cg {

class cg_smiltext_renderer :
	public cg_renderer<renderer_playable>,
	public smil2::smiltext_notification
{
  public:
	cg_smiltext_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp);
	~cg_smiltext_renderer();

	void redraw_body(const rect &dirty, gui_window *window);
	void start(double t);
	void seek(double t);
//	void stop();
	bool stop();
	void pause(pause_display d=display_show);
	void resume();
	// Callbacks from the engine
	void smiltext_changed();
	void marker_seen(const char *name);
  private:
	unsigned int _compute_rate(smil2::smiltext_align align, lib::size size, lib::rect r,  unsigned int dur); // Must go to engine
  private:
	CFMutableAttributedStringRef m_text_storage;
	CGColorSpaceRef m_rgb_colorspace;
	CTRunRef m_text_container;
	CTFrameRef m_frame;
	smil2::smiltext_engine m_engine;
	bool m_needs_conditional_newline;
	bool m_needs_conditional_space;
	smil2::smiltext_params m_params;
	CTParagraphStyleRef m_cur_paragraph_style;
	smil2::smiltext_align m_cur_para_align;
	smil2::smiltext_writing_mode m_cur_para_writing_mode;
	bool m_cur_para_wrap;
	bool m_render_offscreen;			// True if m_params does not allows rendering in-place
	lib::timer::time_type m_epoch;
	bool m_any_semiopaque_bg;			// True if any backgroundOpacity != 1.0 is used
	critical_section m_lock;
};

} // namespace cg

} // namespace gui

} // namespace ambulant

#endif // WITH_SMIL30

#endif // AMBULANT_GUI_CG_CG_SMILTEXT_H
