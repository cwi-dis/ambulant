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
#include "ambulant/gui/cocoa/cocoa_dsvideo.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_dsvideo_renderer::cocoa_dsvideo_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory)
:	common::active_video_renderer(context, cookie, node, evp, factory),
	m_image(NULL)
{
	if (!m_src) {
		lib::logger::get_logger()->error("qt_active_video_renderer::qt_active_video_renderer: no datasource");
		//m_context->stopped(m_cookie, 0);
		return;
	}
	if (m_src->has_audio()) {
		m_audio_ds = m_src->get_audio_datasource();
	
		if (m_audio_ds) {
			AM_DBG lib::logger::get_logger()->debug("qt_active_video_renderer::qt_active_video_renderer: creating audio renderer !");
			m_audio_renderer = factory->rf->new_aux_audio_playable(context, cookie, node, evp, m_audio_ds);
			AM_DBG lib::logger::get_logger()->debug("qt_active_video_renderer::qt_active_video_renderer: audio renderer created(0x%x)!", (void*) m_audio_renderer);
			//m_audio_renderer = new gui::sdl::sdl_active_audio_renderer(&m_playable_notification, cookie, node, evp, df, m_audio_ds);
			//lib::logger::get_logger()->debug("active_video_renderer::active_video_renderer() (this =0x%x) got audio renderer (0x%x)", (void *) this, (void*) m_audio_renderer);
		} else {
			m_audio_renderer = NULL;
		}
		
		//lib::logger::get_logger()->debug("active_video_renderer::active_video_renderer() video has audio", (void *) m_src);
	}
}

cocoa_dsvideo_renderer::~cocoa_dsvideo_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cocoa_dsvideo_renderer(0x%x)", (void *)this);
	if (m_image)
		[m_image release];
	m_image = NULL;
	m_lock.leave();
}
	
void
cocoa_dsvideo_renderer::show_frame(char* frame, int size)
{
	m_lock.enter();
	if (m_image) {
		[m_image release];
		m_image = NULL;
	}
	AM_DBG lib::logger::get_logger()->debug("cocoa_dsvideo_renderer::show_frame: size=%d, w*h*3=%d", size, m_size.w * m_size.h * 4);
	assert(size == m_size.w * m_size.h * 4);
	// XXXX Who keeps reference to frame?
	NSSize nssize = NSMakeSize(m_size.w, m_size.h);
	m_image = [[NSImage alloc] initWithSize: nssize];
	if (!m_image) {
		logger::get_logger()->trace("cocoa_dsvideo_renderer::show_frame: cannot allocate NSImage");
		logger::get_logger()->error(gettext("Out of memory while showing video"));
		m_lock.leave();
		return;
	}
	NSBitmapImageRep *bitmaprep = [[NSBitmapImageRep alloc]
		initWithBitmapDataPlanes: NULL
		pixelsWide: m_size.w
		pixelsHigh: m_size.h
		bitsPerSample: 8
		samplesPerPixel: 3
		hasAlpha: NO
		isPlanar: NO
		colorSpaceName: NSDeviceRGBColorSpace
		bytesPerRow: m_size.w * 4
		bitsPerPixel: 32];
	if (!bitmaprep) {
		logger::get_logger()->trace("cocoa_dsvideo_renderer::show_frame: cannot allocate NSBitmapImageRep");
		logger::get_logger()->error(gettext("Out of memory while showing video"));
		m_lock.leave();
		return;
	}
	memcpy([bitmaprep bitmapData], frame, size);
	[m_image addRepresentation: bitmaprep];
	[m_image setFlipped: true];
	[bitmaprep release];
	if (m_dest) m_dest->need_redraw();
	m_lock.leave();
}

void
cocoa_dsvideo_renderer::redraw(const screen_rect_int &dirty, gui_window *window)
{
	m_lock.enter();
	const screen_rect_int &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
#if 0
	// See whether we're in a transition
	NSImage *surf = NULL;
	if (m_trans_engine && m_trans_engine->is_done()) {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if ([surf isValid]) {
			[surf lockFocus];
			AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("cocoa_dsvideo_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
#endif

	if (m_image) {
		// Now find both source and destination area for the bitblit.
		NSSize cocoa_srcsize = [m_image size];
		size srcsize = size((int)cocoa_srcsize.width, (int)cocoa_srcsize.height);
		rect srcrect = rect(size(0, 0));
		screen_rect_int dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());
		
		NSRect cocoa_srcrect = NSMakeRect(0, 0, srcrect.width(), srcrect.height()); // XXXX 0, 0 is wrong
		NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
		AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cocoa_srcsize.width, cocoa_srcsize.height, NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect), NSMaxX(cocoa_dstrect), NSMaxY(cocoa_dstrect));
		[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceAtop fraction: 1.0];
	} else {
	}
#if 0
	if (surf) [surf unlockFocus];
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<cocoa_dsvideo_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cocoa_dsvideo_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay);
	}
#endif

	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

