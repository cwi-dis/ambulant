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

#include "ambulant/gui/d2/d2_fill.h"
#include "ambulant/gui/d2/d2_window.h"
#include "ambulant/gui/d2/d2_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

#include <d2d1.h>
#include <d2d1helper.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace d2 {

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF((float) r.left(), (float) r.top(), (float) r.right(), (float) r.bottom());
}

extern const char d2_fill_playable_tag[] = "brush";
extern const char d2_fill_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirect2D");
extern const char d2_fill_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererFill");

common::playable_factory *
create_d2_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirect2D"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererFill"), true);
	return new common::single_playable_factory<
		d2_fill_renderer,
		d2_fill_playable_tag,
		d2_fill_playable_renderer_uri,
		d2_fill_playable_renderer_uri2,
		d2_fill_playable_renderer_uri2>(factory, mdp);
}

d2_fill_renderer::~d2_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~d2_fill_renderer(0x%x)", (void *)this);
	m_lock.leave();
}

void
d2_fill_renderer::start(double where)
{
	start_transition(where);
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("d2_fill_renderer.start(0x%x)", (void *)this);
	if (m_activated) {
		logger::get_logger()->trace("d2_fill_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	m_activated = true;
	if (!m_dest) {
		logger::get_logger()->trace("d2_fill_renderer.start(0x%x): no surface", (void *)this);
		return;
	}
	m_dest->show(this);
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
	m_lock.leave();
}

void
d2_fill_renderer::redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget* rt)
{
	assert(rt);
	if (rt == NULL)
		return;
	recreate_d2d();
	if (m_brush == NULL) return;
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("d2_fill_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	d2_window *cwindow = (d2_window *)window;

	// First find our whole area (which we have to clear to background color)
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	AM_DBG logger::get_logger()->debug("d2_fill_renderer.redraw(0x%x, global_ltrb=(%d,%d,%d,%d)", (void *)this, dstrect_whole.left(), dstrect_whole.top(), dstrect_whole.right(), dstrect_whole.bottom());
	
	assert(rt);
	if (rt == NULL)
		return;
	D2D1_RECT_F rr = d2_rectf(dstrect_whole);
	rt->FillRectangle(rr, m_brush);
	m_lock.leave();
}

void 
d2_fill_renderer::recreate_d2d()
{
	if (m_brush) return;
	m_lock.enter();
	HRESULT hr = S_OK;

	// Get color and alpha info from the SMIL node
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		m_lock.leave();
		return;
	}

	color_t color = lib::to_color(color_attr);
	AM_DBG lib::logger::get_logger()->debug("d2_fill_renderer.redraw: clearing to 0x%x", (long)color);
	double alfa = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();

	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);

	// Create the corresponding D2D brush
	hr = rt->CreateSolidColorBrush(D2D1::ColorF(redf(color), greenf(color), bluef(color), alfa), &m_brush);
	if (!SUCCEEDED(hr)) lib::logger::get_logger()->trace("CreateSolidColorBrush: error 0x%x", hr);

	rt->Release();
	m_lock.leave();
}

void
d2_fill_renderer::discard_d2d()
{
	m_lock.enter();
	if (m_brush) {
		m_brush->Release();
		m_brush = NULL;
	}
	m_lock.leave();
}

d2_background_renderer::~d2_background_renderer()
{
	if (m_d2player) m_d2player->unregister_resources(this);
	discard_d2d();
#ifdef D2D_NOTYET
	if (m_bgimage)
		[m_bgimage release];
#endif
	m_bgimage = NULL;
}

void
d2_background_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	recreate_d2d();
	// XXX Incorrect for bgimage and no color:
	if (!m_mustrender || m_brush == NULL) return;

	const rect &r =	 m_dst->get_rect();
	rect dstrect = dirty & r;
	AM_DBG logger::get_logger()->debug("d2_bg_renderer::drawbackground(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	d2_window *cwindow = (d2_window *)window;
	AM_DBG lib::logger::get_logger()->debug("d2_bg_renderer::drawbackground: clearing to 0x%x opacity %f", (long)m_src->get_bgcolor(), m_src->get_bgopacity());
	dstrect.translate(m_dst->get_global_topleft());

	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);

	D2D1_RECT_F rr = d2_rectf(dstrect);
	rt->FillRectangle(rr, m_brush);
#ifdef D2D_NOTYET
	// background images not yet implemented
	if (m_bgimage) {
		AM_DBG lib::logger::get_logger()->debug("d2_background_renderer::redraw(): drawing image");
		NSSize srcsize = [m_bgimage size];
		NSRect srcrect = NSMakeRect(0, 0, srcsize.width, srcsize.height);
		[m_bgimage drawInRect: d2_dstrect_whole fromRect: srcrect
			operation: NSCompositeSourceAtop fraction: (float)1.0];
	}
#endif
	rt->Release();
}

void
d2_background_renderer::highlight(common::gui_window *window)
{
#ifdef D2D_NOTYET
	// Highlighting not yet implemented
	const rect &r =	 m_dst->get_rect();
	AM_DBG logger::get_logger()->debug("d2_bg_renderer::highlight(0x%x)", (void *)this);

	d2_window *cwindow = (d2_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect d2_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	color_t hicolor = 0x0000ff;
	AM_DBG lib::logger::get_logger()->debug("d2_bg_renderer::highlight: framing with color 0x%x", (long)hicolor);
	NSColor *d2_bgcolor = [NSColor colorWithCalibratedRed:redf(hicolor)
				green:greenf(hicolor)
				blue:bluef(hicolor)
				alpha:(float)1.0];
	[d2_bgcolor set];
	NSFrameRect(d2_dstrect_whole);
#endif
}

void
d2_background_renderer::keep_as_background()
{
	AM_DBG lib::logger::get_logger()->debug("d2_background_renderer::keep_as_background() called");
	if (m_bgimage) {
		AM_DBG lib::logger::get_logger()->debug("d2_background_renderer::keep_as_background: delete old m_image");
		m_bgimage = NULL;
	}
	d2_window *cwindow = (d2_window *)m_dst->get_gui_window();
#ifdef D2D_NOTYET
	// keep_as_background not yet implemented
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect_whole = m_dst->get_rect();
	dstrect_whole.translate(m_dst->get_global_topleft());
	NSRect d2_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];

	m_bgimage = [[view getOnScreenImageForRect: d2_dstrect_whole] retain];
#endif
}


void 
d2_background_renderer::recreate_d2d()
{
	if (m_brush) return;
	HRESULT hr = S_OK;
	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);
	// Fill with  color
	assert(m_src);
	double opacity = m_src->get_bgopacity();
	if (opacity > 0) {
		// First find our whole area (which we have to clear to background color)
		// XXXX Fill with background color
		m_mustrender = true;
		color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("d2_bg_renderer::drawbackground: clearing to 0x%x opacity %f", (long)bgcolor, opacity);
		hr = rt->CreateSolidColorBrush(D2D1::ColorF(redf(bgcolor), greenf(bgcolor), bluef(bgcolor), opacity), &m_brush);
		if (!SUCCEEDED(hr)) lib::logger::get_logger()->trace("CreateSolidColorBrush: error 0x%x", hr);
	} else {
		m_mustrender = false;
	}
	rt->Release();
}

void
d2_background_renderer::discard_d2d()
{
	if (m_brush) {
		m_brush->Release();
		m_brush = NULL;
	}
}

} // namespace d2

} // namespace gui

} //namespace ambulant

