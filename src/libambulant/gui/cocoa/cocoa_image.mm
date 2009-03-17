// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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
#include "ambulant/gui/cocoa/cocoa_image.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

extern const char cocoa_image_playable_tag[] = "img";
extern const char cocoa_image_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCocoa");
extern const char cocoa_image_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererImg");

common::playable_factory *
create_cocoa_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
    smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCocoa"), true);
    smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererImg"), true);
	return new common::single_playable_factory<
        cocoa_image_renderer, 
        cocoa_image_playable_tag, 
        cocoa_image_playable_renderer_uri,
        cocoa_image_playable_renderer_uri2,
        cocoa_image_playable_renderer_uri2>(factory, mdp);
}

cocoa_image_renderer::~cocoa_image_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cocoa_image_renderer(0x%x)", (void *)this);
	if (m_image)
		[m_image release];
	m_image = NULL;
	m_lock.leave();
}
	
void
cocoa_image_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	const common::region_info *ri = m_dest->get_info();
	AM_DBG logger::get_logger()->debug("cocoa_image_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	if (m_data && !m_image) {
		AM_DBG logger::get_logger()->debug("cocoa_image_renderer.redraw: creating image");
		m_nsdata = [NSData dataWithBytesNoCopy: m_data length: m_data_size freeWhenDone: NO];
		m_image = [[NSImage alloc] initWithData: m_nsdata];
		if (!m_image)
			logger::get_logger()->error("%s: could not create NSImage", m_node->get_url("src").get_url().c_str());
		[m_image setFlipped: true];
		// XXXX Could free data and m_data again here...
		// Now we need to remember the real image size, which will be trampled soon.
		NSImageRep *bestrep = [m_image bestRepresentationForDevice: nil];
		assert(bestrep);
#ifdef WITH_SMIL30
		// If we need to do chroma keying we do it now. We have to go via a CGImage to do it, though...
		// XXX By doing it here and not later we disable animation on the chromakey attributes...
		if (ri->is_chromakey_specified()) {
			double opacity = ri->get_chromakeyopacity();
			if (opacity != 0.0 && opacity != 1.0) {
				lib::logger::get_logger()->trace("%s: only chromaKeyOpacity values 0.0 and 1.0 supported on MacOS", m_node->get_sig().c_str());
			}
			if (![bestrep respondsToSelector: @selector(CGImage)]) {
				lib::logger::get_logger()->trace("%s: chromaKey only supported on 10.5, and/or not for this image type", m_node->get_sig().c_str());
				opacity = 1.0;
			}
			if (opacity < 0.5) {
				lib::color_t chromakey = ri->get_chromakey();
				lib::color_t tolerance = ri->get_chromakeytolerance();
				float components[8] = {
					redc(chromakey)-redc(tolerance), redc(chromakey)+redc(tolerance),
					greenc(chromakey)-greenc(tolerance), greenc(chromakey)+greenc(tolerance),
					bluec(chromakey)-bluec(tolerance), bluec(chromakey)+bluec(tolerance),
					0.0, 0.0
				};
				CGImageRef orig_cgi = (CGImageRef)[(NSBitmapImageRep *)bestrep CGImage]; // XXX 10.5 only!
				assert(orig_cgi);
				CGImageRef new_cgi = CGImageCreateWithMaskingColors(orig_cgi, components);
				NSImageRep *newrep = [[NSBitmapImageRep alloc] initWithCGImage: new_cgi];
				[m_image removeRepresentation: bestrep];
				bestrep = newrep;
				[m_image addRepresentation: bestrep];
			}
		}
#endif
		m_size = lib::size([bestrep pixelsWide], [bestrep pixelsHigh]);
		AM_DBG lib::logger::get_logger()->debug("cocoa_image_renderer: image size in pixels: %d x %d", m_size.w, m_size.h);
		AM_DBG lib::logger::get_logger()->debug("cocoa_image_renderer: image size in units: %f x %f", [bestrep size].width, [bestrep size].height);
		[bestrep setSize:NSMakeSize(m_size.w, m_size.h)];
	}

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	if (!m_image) {
		AM_DBG logger::get_logger()->debug("cocoa_image_renderer.redraw: nothing to draw");
		m_lock.leave();
		return;
	}
	
	// Now find both source and destination area for the bitblit.
	NSSize cocoa_srcsize = [m_image size];
	size srcsize = size((int)cocoa_srcsize.width, (int)cocoa_srcsize.height);
	rect srcrect;
	NSRect cocoa_srcrect;
	rect dstrect;
	NSRect cocoa_dstrect;
	// While rendering background images only, check for tiling. This code is
	// convoluted, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") && m_dest->is_tiled()) {
		AM_DBG lib::logger::get_logger()->debug("cocoa_image_renderer.redraw: drawing tiled image");
		dstrect = m_dest->get_rect();
		dstrect.translate(m_dest->get_global_topleft());
		common::tile_positions tiles = m_dest->get_tiles(srcsize, dstrect);
		common::tile_positions::iterator it;
		for(it=tiles.begin(); it!=tiles.end(); it++) {
			srcrect = (*it).first;
			dstrect = (*it).second;
			cocoa_srcrect = NSMakeRect(srcrect.left(), srcrect.top(), srcrect.width(), srcrect.height());
			cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
			AM_DBG logger::get_logger()->debug("cocoa_image_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cocoa_srcsize.width, cocoa_srcsize.height, NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect), NSMaxX(cocoa_dstrect), NSMaxY(cocoa_dstrect));
			[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceOver fraction: 1.0];
		}
		m_lock.leave();
		return;
	}
#ifdef WITH_SMIL30
	lib::rect croprect = m_dest->get_crop_rect(m_size);
	AM_DBG logger::get_logger()->debug("cocoa_image::redraw, clip 0x%x (%d %d) -> (%d, %d, %d, %d)", m_dest, m_size.w, m_size.h, croprect.x, croprect.y, croprect.w, croprect.h);

	dstrect = m_dest->get_fit_rect(croprect, srcsize, &srcrect, m_alignment);
	cocoa_srcrect = NSMakeRect(srcrect.left(), srcrect.top(), srcrect.width(), srcrect.height());
#else
	dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
	cocoa_srcrect = NSMakeRect(0, 0, srcrect.width(), srcrect.height());
#endif
	dstrect.translate(m_dest->get_global_topleft());
	cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
	AM_DBG logger::get_logger()->debug("cocoa_image_renderer.redraw: draw image (%f, %f, %f %f) -> (%f, %f, %f, %f)",
		NSMinX(cocoa_srcrect), NSMinY(cocoa_srcrect), NSMaxX(cocoa_srcrect), NSMaxY(cocoa_srcrect),
		NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect), NSMaxX(cocoa_dstrect), NSMaxY(cocoa_dstrect));
#ifdef WITH_SMIL30
	double alfa = 1.0;
	if (ri) alfa = ri->get_mediaopacity();
	[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceOver fraction: alfa];
#else
	[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceOver fraction: 1.0];
#endif
	
	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

