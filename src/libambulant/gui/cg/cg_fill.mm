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

#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_fill.h"
//#include "ambulant/gui/cg/cg_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/test_attrs.h"
//#include <CoreGraphics/CoreGraphics.h>

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

extern const char cg_fill_playable_tag[] = "brush";
extern const char cg_fill_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCoreGraphics");
extern const char cg_fill_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererFill");

common::playable_factory *
create_cg_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCoreGraphics"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererFill"), true);
	return new common::single_playable_factory<
		cg_fill_renderer,
		cg_fill_playable_tag,
		cg_fill_playable_renderer_uri,
		cg_fill_playable_renderer_uri2,
		cg_fill_playable_renderer_uri2>(factory, mdp);
}

cg_fill_renderer::~cg_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~cg_fill_renderer(0x%x)", (void *)this);
	m_lock.leave();
}
   
void
cg_fill_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cg_fill_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	// First find our whole area (which we have to clear to background color)
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	AM_DBG logger::get_logger()->debug("cg_fill_renderer.redraw(0x%x, dstrect_whole=(%d,%d,%d,%d)", (void *)this, dstrect_whole.left(), dstrect_whole.top(), dstrect_whole.right(), dstrect_whole.bottom());
	CGRect cg_dstrect_whole = CGRectFromAmbulantRect(dstrect_whole);
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		m_lock.leave();
		return;
	}
	color_t color = lib::to_color(color_attr);
	AM_DBG lib::logger::get_logger()->debug("cg_fill_renderer.redraw: clearing to 0x%x", (long)color);
	double alfa = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
	CGFloat components[] = {redf(color), greenf(color), bluef(color), alfa};
	CGContextRef myContext = [view getCGContext];
	CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextSetFillColorSpace(myContext, genericColorSpace);
	CGColorSpaceRelease(genericColorSpace);
	CGContextSetFillColor(myContext, components);
	CGContextFillRect(myContext, cg_dstrect_whole);
	m_lock.leave();
}

void
cg_fill_renderer::start(double t)
{
    if (m_transition_renderer != NULL) {
        m_transition_renderer->start(t);
    }
    if (m_context != NULL) {
        m_context->started(m_cookie);
        m_context->stopped(m_cookie);
    }
    renderer_playable::start(t);
}
    
cg_background_renderer::~cg_background_renderer()
{
	if (m_bgimage)
		CGImageRelease(m_bgimage);
	m_bgimage = NULL;
}

void
cg_background_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	const rect &r =	 m_dst->get_rect();
	AM_DBG logger::get_logger()->debug("cg_bg_renderer::drawbackground(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	CGContextRef myContext = [view getCGContext];
	AM_DBG lib::logger::get_logger()->debug("cg_bg_renderer::drawbackground: clearing to 0x%x opacity %f", (long)m_src->get_bgcolor(), m_src->get_bgopacity());
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	CGRect cg_dstrect_whole = CGRectFromAmbulantRect(dstrect_whole);
	double opacity = m_src->get_bgopacity();
	if (m_src && opacity > 0) {
		// First find our whole area (which we have to clear to background color)
		// XXXX Fill with background color
		color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("cg_bg_renderer::drawbackground: clearing to 0x%x opacity %f", (long)bgcolor, opacity);
		CGFloat components[] = {redf(bgcolor), greenf(bgcolor), bluef(bgcolor), opacity};
		CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
		CGContextSetFillColorSpace(myContext, genericColorSpace);
		CGColorSpaceRelease(genericColorSpace);
		CGContextSetFillColor(myContext, components);
		CGContextFillRect(myContext, cg_dstrect_whole);
	}
	if (m_bgimage) {
		AM_DBG lib::logger::get_logger()->debug("cg_background_renderer::redraw(): drawing image");
		CGContextDrawImage(myContext, cg_dstrect_whole, m_bgimage);
	}
}

void
cg_background_renderer::highlight(common::gui_window *window)
{
	const rect &r =	 m_dst->get_rect();
	AM_DBG logger::get_logger()->debug("cg_bg_renderer::highlight(0x%x)", (void *)this);

	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	CGRect cg_dstrect_whole = CGRectFromAmbulantRect(dstrect_whole);
	color_t hicolor = 0x0000ff;
	AM_DBG lib::logger::get_logger()->debug("cg_bg_renderer::highlight: framing with color 0x%x", (long)hicolor);
	CGFloat components[] = {redf(hicolor), greenf(hicolor), bluef(hicolor), 1.0};
	CGContextRef myContext = [view getCGContext];
	CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextSetStrokeColorSpace(myContext, genericColorSpace);
	CGColorSpaceRelease(genericColorSpace);
	CGContextSetStrokeColor(myContext, components);
	CGContextStrokeRect(myContext, cg_dstrect_whole);
}

void
cg_background_renderer::keep_as_background()
{
	AM_DBG lib::logger::get_logger()->debug("cg_background_renderer::keep_as_background() called");
	if (m_bgimage) {
		AM_DBG lib::logger::get_logger()->debug("cg_background_renderer::keep_as_background: delete old m_image");
		CGImageRelease(m_bgimage);
		m_bgimage = NULL;
	}
	rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
}

} // namespace cg

} // namespace gui

} //namespace ambulant

