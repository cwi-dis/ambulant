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
cocoa_active_image_renderer::redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d), window_topleft=(%d, %d))", (void *)this, r.left(), r.top(), r.right(), r.bottom(), window_topleft.x, window_topleft.y);
	
	if (m_data && !m_image) {
		AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw: creating image");
		m_nsdata = [NSData dataWithBytesNoCopy: m_data length: m_data_size freeWhenDone: NO];
		m_image = [[NSImage alloc] initWithData: m_nsdata];
		if (!m_image)
			logger::get_logger()->error("cocoa_active_image_renderer.redraw: could not create image");
		// XXXX Could free data and m_data again here...
	}

	cocoa_passive_window *cwindow = (cocoa_passive_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	screen_rect<int> window_rect = r;
	window_rect.translate(window_topleft);
	NSRect dstrect = [view NSRectForAmbulantRect: &window_rect];

	if (m_image) {
		NSSize srcsize = [m_image size];
		NSRect srcrect = NSMakeRect(0, 0, srcsize.width, srcsize.height);
		AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", srcsize.width, srcsize.height, NSMinX(dstrect), NSMinY(dstrect), NSMaxX(dstrect), NSMaxY(dstrect));
		[m_image drawInRect: dstrect fromRect: srcrect operation: NSCompositeCopy fraction: 1.0];
	}
	m_lock.leave();
}


} // namespace cocoa

} // namespace gui

} //namespace ambulant

