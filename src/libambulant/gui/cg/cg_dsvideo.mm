// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/gui/cg/cg_dsvideo.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

cg_dsvideo_renderer::cg_dsvideo_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory)
:	common::video_renderer(context, cookie, node, evp, factory),
	m_image(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer(): 0x%x created", (void*)this);
}

cg_dsvideo_renderer::~cg_dsvideo_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cg_dsvideo_renderer(0x%x)", (void *)this);
	if (m_image) CGImageRelease(m_image);
	m_image = NULL;
	m_lock.leave();
}
	
void
cg_dsvideo_renderer::show_frame(const char* frame, int size)
{
	m_lock.enter();
	if (m_image) {
		CGImageRelease(m_image);
		m_image = NULL;
	}
	AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer::show_frame: size=%d, w*h*3=%d", size, m_size.w * m_size.h * 4);
	assert(size == (int)(m_size.w * m_size.h * 4));
	// XXXX Who keeps reference to frame?
	CGSize nssize = CGSizeMake(m_size.w, m_size.h);
	m_image = NULL; // [[NSImage alloc] initWithSize: nssize];
	CFDataRef cfdata = CFDataCreate(NULL, (const UInt8 *)frame, size);
	assert(cfdata);
	CGDataProviderRef provider = CGDataProviderCreateWithCFData(cfdata);
	assert(provider);
	CFRelease(cfdata);
	CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
	assert(genericColorSpace);
	CGBitmapInfo bitmapInfo = 0; // XXXJACK may need to cater for endianness
	m_image = CGImageCreate( m_size.w, m_size.h, 8, 32, m_size.w*4, genericColorSpace, bitmapInfo, provider, NULL, true, kCGRenderingIntentDefault);
	CGDataProviderRelease(provider);
	CGColorSpaceRelease(genericColorSpace);
	if (!m_image) {
		logger::get_logger()->trace("cg_dsvideo_renderer::show_frame: cannot create CGImage");
		m_lock.leave();
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer::show_frame: created CGImage 0x%x", m_image);
#if UIKIT_NOT_YET
	NSBitmapImageRep *bitmaprep = [[NSBitmapImageRep alloc]
		initWithBitmapDataPlanes: NULL
		pixelsWide: m_size.w
		pixelsHigh: m_size.h
		bitsPerSample: 8
		samplesPerPixel: 3
		hasAlpha: NO
		isPlanar: NO
		colorSpaceName: NSDeviceRGBColorSpace
#ifdef __BIG_ENDIAN__
		bitmapFormat: NSAlphaFirstBitmapFormat
#else
		bitmapFormat: (NSBitmapFormat)0
#endif
		bytesPerRow: m_size.w * 4
		bitsPerPixel: 32];
	if (!bitmaprep) {
		logger::get_logger()->trace("cg_dsvideo_renderer::show_frame: cannot allocate NSBitmapImageRep");
		logger::get_logger()->error(gettext("Out of memory while showing video"));
		m_lock.leave();
		return;
	}
#if 1
	memcpy([bitmaprep bitmapData], frame, size);
#else
	swab(frame, [bitmaprep bitmapData], size);
#endif
	[m_image addRepresentation: bitmaprep];
	[m_image setFlipped: true];
	[bitmaprep release];
#endif
	if (m_dest) m_dest->need_redraw();
	m_lock.leave();
}

void
cg_dsvideo_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	cg_window *cwindow = (cg_window *)window;
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
			AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("cg_dsvideo_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
#endif
	if (m_image) {
		// Now find both source and destination area for the bitblit.
		CGSize cg_srcsize = CGSizeMake(m_size.w, m_size.h);
		rect srcrect = rect(size(0, 0));
		rect dstrect = m_dest->get_fit_rect(m_size, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());
		
		CGRect cg_srcrect = CGRectMake(0, 0, srcrect.width(), srcrect.height()); // XXXX 0, 0 is wrong
		CGRect cg_dstrect = [view CGRectForAmbulantRect: &dstrect];
		AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cg_srcsize.width, cg_srcsize.height, CGRectGetMinX(cg_dstrect), CGRectGetMinY(cg_dstrect), CGRectGetMaxX(cg_dstrect), CGRectGetMaxY(cg_dstrect));
		// XXX Crop the image, if needed.
		CGImageRef cropped_image = m_image;
		CGContextRef myContext = [view getCGContext];
		double alfa = 1.0;
#ifdef WITH_SMIL30
		const common::region_info *ri = m_dest->get_info();
		if (ri) alfa = ri->get_mediaopacity();
#endif
		CGContextDrawImage (myContext, cg_dstrect, cropped_image); // ignoring alfa, for now
		// XXX  release cropped_image
	} else {
	}
#if 0
	if (surf) [surf unlockFocus];
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->debug("cg_dsvideo_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<cg_dsvideo_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cg_dsvideo_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("cg_dsvideo_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay);
	}
#endif

	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant

