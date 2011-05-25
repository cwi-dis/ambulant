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

/*
 * @$Id$
 */

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_fill.h"
#include "ambulant/gui/cocoa/cocoa_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

extern const char cocoa_fill_playable_tag[] = "brush";
extern const char cocoa_fill_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCocoa");
extern const char cocoa_fill_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererFill");

common::playable_factory *
create_cocoa_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCocoa"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererFill"), true);
	return new common::single_playable_factory<
		cocoa_fill_renderer,
		cocoa_fill_playable_tag,
		cocoa_fill_playable_renderer_uri,
		cocoa_fill_playable_renderer_uri2,
		cocoa_fill_playable_renderer_uri2>(factory, mdp);
}

cocoa_fill_renderer::~cocoa_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~cocoa_fill_renderer(0x%x)", (void *)this);
	m_lock.leave();
}

void
cocoa_fill_renderer::start(double where)
{
	start_transition(where);
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cocoa_fill_renderer.start(0x%x)", (void *)this);
	if (m_activated) {
		logger::get_logger()->trace("cocoa_fill_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	m_activated = true;
	if (!m_dest) {
		logger::get_logger()->trace("cocoa_fill_renderer.start(0x%x): no surface", (void *)this);
		return;
	}
	m_dest->show(this);
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
	m_lock.leave();
}

void
cocoa_fill_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_fill_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	// First find our whole area (which we have to clear to background color)
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	AM_DBG logger::get_logger()->debug("cocoa_fill_renderer.redraw(0x%x, global_ltrb=(%d,%d,%d,%d)", (void *)this, dstrect_whole.left(), dstrect_whole.top(), dstrect_whole.right(), dstrect_whole.bottom());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		m_lock.leave();
		return;
	}
	color_t color = lib::to_color(color_attr);
	AM_DBG lib::logger::get_logger()->debug("cocoa_fill_renderer.redraw: clearing to 0x%x", (long)color);
	double alfa = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
	NSColor *nscolor = [NSColor colorWithCalibratedRed:redf(color)
			green:greenf(color)
			blue:bluef(color)
			alpha:(float)alfa];
	[nscolor set];
	NSRectFillUsingOperation(cocoa_dstrect_whole, NSCompositeSourceAtop);
	m_lock.leave();
}

cocoa_background_renderer::~cocoa_background_renderer()
{
	if (m_bgimage)
		[m_bgimage release];
	m_bgimage = NULL;
}

void
cocoa_background_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	const rect &r =	 m_dst->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_bg_renderer::drawbackground(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	AM_DBG lib::logger::get_logger()->debug("cocoa_bg_renderer::drawbackground: clearing to 0x%x opacity %f", (long)m_src->get_bgcolor(), m_src->get_bgopacity());
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	double opacity = m_src->get_bgopacity();
	if (m_src && opacity > 0) {
		// First find our whole area (which we have to clear to background color)
		// XXXX Fill with background color
		color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("cocoa_bg_renderer::drawbackground: clearing to 0x%x opacity %f", (long)bgcolor, opacity);
		NSColor *cocoa_bgcolor = [NSColor colorWithCalibratedRed:redf(bgcolor)
					green:greenf(bgcolor)
					blue:bluef(bgcolor)
					alpha:(float)opacity];
		[cocoa_bgcolor set];
		NSRectFillUsingOperation(cocoa_dstrect_whole, NSCompositeSourceAtop);
	}
	if (m_bgimage) {
		AM_DBG lib::logger::get_logger()->debug("cocoa_background_renderer::redraw(): drawing image");
		NSSize srcsize = [m_bgimage size];
		NSRect srcrect = NSMakeRect(0, 0, srcsize.width, srcsize.height);
		[m_bgimage drawInRect: cocoa_dstrect_whole fromRect: srcrect
			operation: NSCompositeSourceAtop fraction: (float)1.0];
	}
}

void
cocoa_background_renderer::highlight(common::gui_window *window)
{
	const rect &r =	 m_dst->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_bg_renderer::highlight(0x%x)", (void *)this);

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	color_t hicolor = 0x0000ff;
	AM_DBG lib::logger::get_logger()->debug("cocoa_bg_renderer::highlight: framing with color 0x%x", (long)hicolor);
	NSColor *cocoa_bgcolor = [NSColor colorWithCalibratedRed:redf(hicolor)
				green:greenf(hicolor)
				blue:bluef(hicolor)
				alpha:(float)1.0];
	[cocoa_bgcolor set];
	NSFrameRect(cocoa_dstrect_whole);
}

void
cocoa_background_renderer::keep_as_background()
{
	AM_DBG lib::logger::get_logger()->debug("cocoa_background_renderer::keep_as_background() called");
	if (m_bgimage) {
		AM_DBG lib::logger::get_logger()->debug("cocoa_background_renderer::keep_as_background: delete old m_image");
		[m_bgimage release];
		m_bgimage = NULL;
	}
	cocoa_window *cwindow = (cocoa_window *)m_dst->get_gui_window();
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];

	m_bgimage = [[view getOnScreenImageForRect: cocoa_dstrect_whole] retain];
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

