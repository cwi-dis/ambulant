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

#include "ambulant/gui/dx/dx_dsvideo.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/img_decoder.h"
#include "ambulant/lib/win32/win32_error.h"
using ambulant::lib::win32::win_report_error;

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace dx {

class video_dib_surface {
  public:
	HBITMAP get_handle() { return NULL;}
};

dx_dsvideo_renderer::dx_dsvideo_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory)
:	common::video_renderer(context, cookie, node, evp, factory),
	m_bitmap(NULL),
	m_bitmap_dataptr(NULL),
	m_ddsurf(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer(): 0x%x created", (void*)this);
}

void
dx_dsvideo_renderer::_init_bitmap()
{
	// First check that we actually have the necessary data (x/y size)
	if (m_size.h == 0 || m_size.w == 0) return;
	assert(m_bitmap == NULL);
	assert(m_bitmap_dataptr == NULL);
	// Create a HBITMAP for which we also have the data pointer (so we can
	// copy frames into it later)
	void *pBits = NULL;
	BITMAPINFO *pbmpi = get_bmp_info(m_size.w, -(int)m_size.h, 32);
	m_bitmap = CreateDIBSection(NULL, pbmpi, DIB_RGB_COLORS, &pBits, NULL, 0);
	if(m_bitmap==NULL || pBits==NULL) {
		lib::logger::get_logger()->error("CreateDIBSection() failed");
		return; // failed
	}
	m_bitmap_dataptr = (char *)pBits;
}

void
dx_dsvideo_renderer::_init_ddsurf(gui_window *window)
{
	if (m_size.h == 0 || m_size.w == 0) return;
	assert(m_ddsurf == NULL);
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	assert(v);
	m_ddsurf = v->create_surface(m_size);
;
}

dx_dsvideo_renderer::~dx_dsvideo_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~dx_dsvideo_renderer(0x%x)", (void *)this);
	if (m_ddsurf)
		m_ddsurf->Release();
	m_ddsurf = NULL;
	if (m_bitmap)
			DeleteObject(m_bitmap);
	m_bitmap = NULL;
	m_bitmap_dataptr = NULL;
	m_lock.leave();
}
	
void
dx_dsvideo_renderer::show_frame(const char* frame, int size)
{
	if (m_bitmap == NULL) _init_bitmap();
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer::show_frame: size=%d", size);
	assert(size == (int)(m_size.w * m_size.h * 4));
	memcpy(m_bitmap_dataptr, frame, size);
	if (m_dest) m_dest->need_redraw();
	m_lock.leave();
}

void
dx_dsvideo_renderer::_copy_to_ddsurf()
{
	if (m_bitmap == NULL || m_ddsurf == NULL) return;
	HRESULT hr;
	HDC hdc;
	hr = m_ddsurf->GetDC(&hdc);
	AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer::_copy_to_ddsurf(), Called GetDC");
	if (FAILED(hr)) {
		win_report_error("DirectDrawSurface::GetDC()", hr);
		return;
	}
	
	HDC bmp_hdc = CreateCompatibleDC(hdc);
	assert(bmp_hdc);
	HBITMAP hbmp_old = (HBITMAP) SelectObject(bmp_hdc, m_bitmap);
	bool ok = ::BitBlt(hdc, 0, 0, m_size.w, m_size.h, bmp_hdc, 0, 0, SRCCOPY);
	assert(ok);
	SelectObject(bmp_hdc, hbmp_old);
	ok = DeleteDC(bmp_hdc);
	assert(ok);
	hr = m_ddsurf->ReleaseDC(hdc);
	if (FAILED(hr)) {
		win_report_error("DirectDrawSurface::ReleaseDC()", hr);
	}
	AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer::_copy_to_ddsurf(), Called ReleaseDC");
}

void
dx_dsvideo_renderer::redraw(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	// XXXX Should do this only when needed (skip extra bitblit)
	if (m_ddsurf == NULL ) _init_ddsurf(window);
	if (m_ddsurf == NULL) {
		m_lock.leave();
		return;
	}
	// XXXX Should do this only when needed:
	_copy_to_ddsurf();
	//const rect &r = m_dest->get_rect();
	//AM_DBG logger::get_logger()->debug("dx_dsvideo_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
#if 1
	lib::rect img_rect1;
	lib::rect img_reg_rc;
	// lib::size srcsize = m_image->get_size();

#ifdef WITH_SMIL30
	lib::rect croprect = m_dest->get_crop_rect(m_size);
	img_reg_rc = m_dest->get_fit_rect(croprect, m_size, &img_rect1, m_alignment);
#else
	// Get fit rectangles
	img_reg_rc = m_dest->get_fit_rect(m_size, &img_rect1, m_alignment);
#endif
	// Use one type of rect to do op
	lib::rect img_rect(img_rect1);
	
	// A complete repaint would be:  
	// {img, img_rect } -> img_reg_rc
	
	// We have to paint only the intersection.
	// Otherwise we will override upper layers 
	lib::rect img_reg_rc_dirty = img_reg_rc & dirty;
	if(img_reg_rc_dirty.empty()) {
		// this renderer has no pixels for the dirty rect
		AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer::redraw NOT: empty dirty region %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		m_lock.leave();
		return;
	}	
	
	// Find the part of the image that is mapped to img_reg_rc_dirty
	lib::rect img_rect_dirty = reverse_transform(&img_reg_rc_dirty, 
		&img_rect, &img_reg_rc);
		
	// Translate img_reg_rc_dirty to viewport coordinates 
	lib::point topleft = m_dest->get_global_topleft();
	img_reg_rc_dirty.translate(topleft);
	
	
	// Finally blit img_rect_dirty to img_reg_rc_dirty
	AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::redraw %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
	
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) {
		AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::redraw NOT: no viewport %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}

	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}

	if(tr && tr->is_outtrans()) {
		// First draw the background color, if applicable
		const common::region_info *ri = m_dest->get_info();
		if(ri)
			v->clear(img_reg_rc_dirty,ri->get_bgcolor(), ri->get_bgopacity());
		// Next, take a snapshot of the relevant pixels as they are now, before we draw the image
		IDirectDrawSurface *bgimage = v->create_surface(m_size);
		lib::rect dirty_screen = img_rect_dirty;
		dirty_screen.translate(topleft);
		RECT bgrect_image, bgrect_screen;
		set_rect(img_rect_dirty, &bgrect_image);
		set_rect(dirty_screen, &bgrect_screen);
#ifdef DDBLT_WAIT
#define WAITFLAG DDBLT_WAIT
#else
#define WAITFLAG DDBLT_WAITNOTBUSY
#endif
		bgimage->Blt(&bgrect_image, v->get_surface(), &bgrect_screen, WAITFLAG, NULL);
		// Then draw the image
		v->draw(m_ddsurf, img_rect_dirty, img_reg_rc_dirty, FALSE, (dx_transition*)0);
		// And finally transition in the background bits saved previously
		v->draw(bgimage, img_rect_dirty, img_reg_rc_dirty, false, tr);
		bgimage->Release();
	} else {
		v->draw(m_ddsurf, img_rect_dirty, img_reg_rc_dirty, FALSE, tr);
	}
#endif // 1
#if 0	
	dx_window *cwindow = (dx_window *)window;
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
			AM_DBG logger::get_logger()->debug("dx_dsvideo_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("dx_dsvideo_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
#endif

	if (m_image) {

		// Now find both source and destination area for the bitblit.
		NSSize cocoa_srcsize = [m_image size];
		size srcsize = size((int)cocoa_srcsize.width, (int)cocoa_srcsize.height);
		rect srcrect = rect(size(0, 0));
		rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());
		
		NSRect cocoa_srcrect = NSMakeRect(0, 0, srcrect.width(), srcrect.height()); // XXXX 0, 0 is wrong
		NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
		AM_DBG logger::get_logger()->debug("cocoa_dsvideo_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", cocoa_srcsize.width, cocoa_srcsize.height, NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect), NSMaxX(cocoa_dstrect), NSMaxY(cocoa_dstrect));
#ifdef WITH_SMIL30
		double alfa = 1.0;
		const common::region_info *ri = m_dest->get_info();
		if (ri) alfa = ri->get_mediaopacity();
		[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceAtop fraction: alfa];
#else
		[m_image drawInRect: cocoa_dstrect fromRect: cocoa_srcrect operation: NSCompositeSourceAtop fraction: 1.0];
#endif
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
#endif
	m_lock.leave();
}

} // namespace dx

} // namespace gui

} //namespace ambulant

