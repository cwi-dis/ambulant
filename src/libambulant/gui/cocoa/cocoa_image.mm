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
#include "ambulant/common/region_info.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_active_image_renderer::~cocoa_active_image_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->trace("~cocoa_active_image_renderer(0x%x)", (void *)this);
	if (m_image)
		[m_image release];
	m_image = NULL;
	m_lock.leave();
}
	
void
cocoa_active_image_renderer::redraw(const screen_rect<int> &dirty, abstract_window *window)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	if (m_data && !m_image) {
		AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw: creating image");
		m_nsdata = [NSData dataWithBytesNoCopy: m_data length: m_data_size freeWhenDone: NO];
		m_image = [[NSImage alloc] initWithData: m_nsdata];
		if (!m_image)
			logger::get_logger()->error("cocoa_active_image_renderer.redraw: could not create image");
		[m_image setFlipped: true];
		// XXXX Could free data and m_data again here...
	}

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	// XXXX WRONG! This is the info for the region, not for the node!
	const abstract_smil_region_info *info = m_dest->get_info();
	if (info && !info->get_transparent()) {
		// First find our whole area (which we have to clear to background color)
		screen_rect<int> dstrect_whole = r;
		dstrect_whole.translate(m_dest->get_global_topleft());
		NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
		// XXXX Fill with background color
		color_t bgcolor = info->get_bgcolor();
		/*AM_DBG*/ lib::logger::get_logger()->trace("cocoa_active_image_renderer.redraw: clearing to 0x%x", (long)bgcolor);
		NSColor *cocoa_bgcolor = [NSColor colorWithCalibratedRed:redf(bgcolor)
					green:greenf(bgcolor)
					blue:bluef(bgcolor)
					alpha:1.0];
		[cocoa_bgcolor set];
		NSRectFill(cocoa_dstrect_whole);
	}

	if (m_image) {
		// Now find both source and destination area for the bitblit.
		NSSize cocoa_srcsize = [m_image size];
		size srcsize = size((int)cocoa_srcsize.width, (int)cocoa_srcsize.height);
		rect srcrect = rect(size(0, 0));
		screen_rect<int> dstrect = m_dest->get_fit_rect(srcsize, &srcrect);
		dstrect.translate(m_dest->get_global_topleft());
		
		NSRect cocoa_srcrect = NSMakeRect(0, 0, srcrect.width(), srcrect.height()); // XXXX 0, 0 is wrong
		NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
		AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cocoa_srcsize.width, cocoa_srcsize.height, NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect), NSMaxX(cocoa_dstrect), NSMaxY(cocoa_dstrect));
		[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeCopy fraction: 1.0];
	} else {
	}
	
	m_lock.leave();
}


} // namespace cocoa

} // namespace gui

} //namespace ambulant

