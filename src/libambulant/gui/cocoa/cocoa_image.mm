/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_image.h"
//#include "ambulant/gui/cocoa/cocoa_transition.h"
//#include "ambulant/common/region_info.h"
//#include "ambulant/common/smil_alignment.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

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
cocoa_image_renderer::redraw_body(const screen_rect<int> &dirty, gui_window *window)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_image_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	if (m_data && !m_image) {
		AM_DBG logger::get_logger()->debug("cocoa_image_renderer.redraw: creating image");
		m_nsdata = [NSData dataWithBytesNoCopy: m_data length: m_data_size freeWhenDone: NO];
		m_image = [[NSImage alloc] initWithData: m_nsdata];
		if (!m_image)
			logger::get_logger()->error("%s: could not create NSImage", m_node->get_url("src").get_url().c_str());
		[m_image setFlipped: true];
		// XXXX Could free data and m_data again here...
	}

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	if (!m_image) {
		m_lock.leave();
		return;
	}
	
	// Now find both source and destination area for the bitblit.
	NSSize cocoa_srcsize = [m_image size];
	size srcsize = size((int)cocoa_srcsize.width, (int)cocoa_srcsize.height);
	rect srcrect;
	NSRect cocoa_srcrect;
	screen_rect<int> dstrect;
	NSRect cocoa_dstrect;
#ifdef USE_SMIL21
	// While rendering background images only, check for tiling. This code is
	// convoluted, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundRepeat") && m_dest->is_tiled()) {
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
			[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceAtop fraction: 1.0];
		}
		m_lock.leave();
		return;
	}
#endif
	dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
	cocoa_srcrect = NSMakeRect(0, 0, srcrect.width(), srcrect.height());
	dstrect.translate(m_dest->get_global_topleft());
	cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];

	AM_DBG logger::get_logger()->debug("cocoa_image_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cocoa_srcsize.width, cocoa_srcsize.height, NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect), NSMaxX(cocoa_dstrect), NSMaxY(cocoa_dstrect));
	[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceAtop fraction: 1.0];
	
	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

