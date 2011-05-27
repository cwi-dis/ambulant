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

#define MY_PIXEL_LAYOUT net::pixel_argb

#ifdef ENABLE_FAST_DDVIDEO
// ?? Cannot pick up IID_IDirectDrawSurface7. Redefine self.
const GUID myIID_IDirectDrawSurface7 = {
	0x06675a80,0x3b9b,0x11d2, {0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b}
};
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
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	common::video_renderer(context, cookie, node, evp, factory, mdp),
	m_frame(NULL),
	m_bitmap(NULL),
	m_bitmap_dataptr(NULL),
	m_ddsurf(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer(): 0x%x created", (void*)this);
}

net::pixel_order
dx_dsvideo_renderer::pixel_layout()
{
	return MY_PIXEL_LAYOUT;
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
#ifdef ENABLE_FAST_DDVIDEO
	// Error message will be given later, if needed
	(void)m_ddsurf->QueryInterface(myIID_IDirectDrawSurface7, (void**)&m_ddsurf7);
#endif
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
	if (m_frame)
		free(m_frame);
	m_frame = NULL;
	m_lock.leave();
}

void
dx_dsvideo_renderer::_push_frame(char* frame, size_t size)
{
	if (m_frame) free(m_frame);
	m_frame = frame;
	m_frame_size = size;
	AM_DBG lib::logger::get_logger()->debug("dx_dsvideo_renderer::_push_frame(0x%x, %d)", frame, size);
	assert(size == (m_size.w * m_size.h * 4));
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
#pragma warning(push)
#pragma warning(disable:4800)
	bool ok = (bool)::BitBlt(hdc, 0, 0, m_size.w, m_size.h, bmp_hdc, 0, 0, SRCCOPY);
	assert(ok);
	SelectObject(bmp_hdc, hbmp_old);
	ok = (bool)DeleteDC(bmp_hdc);
#pragma warning(pop)
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
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) {
		AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::redraw NOT: no viewport %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}
	if (m_ddsurf == NULL ) _init_ddsurf(window);
	if (m_ddsurf == NULL) {
		m_lock.leave();
		return;
	}
	// First thing we try is to change the DD surface memory pointer in-place.
	// If that fails we copy.
	LPVOID oldDataPointer = NULL;
#ifdef ENABLE_FAST_DDVIDEO
	static bool warned = false;	// Give the warning message only once per run.
	DDSURFACEDESC2 desc;
	HRESULT hr;
	if (m_ddsurf7 == NULL) {
		if (!warned) lib::logger::get_logger()->trace("dx_dsvideo: No fast video: DirectDrawSurface7 not available");
		warned = true;
	} else {
		memset(&desc, 0, sizeof desc);
		desc.dwSize = sizeof desc;
		hr = m_ddsurf7->GetSurfaceDesc(&desc);
	}
	if (m_ddsurf7 == NULL) {
		// pass, error message given earlier.
	} else
	if (FAILED(hr)) {
		if (!warned) lib::logger::get_logger()->trace("dx_dsvideo: No fast video: GetSurfaceDesc failed, error 0x%x", hr);
		warned = true;
	} else
	if ((desc.dwFlags & DDSD_PIXELFORMAT) != DDSD_PIXELFORMAT) {
		if (!warned) lib::logger::get_logger()->trace("dx_dsvideo: No fast video: GetSurfaceDesc: incomplete information (dwFlags 0x%x)", desc.dwFlags);
		warned = true;
	} else
	if ( (desc.ddpfPixelFormat.dwFlags&DDPF_RGB) == 0
			|| desc.ddpfPixelFormat.dwRGBBitCount != 32) {
		if (!warned) lib::logger::get_logger()->trace("dx_dsvideo: Surface incompatible with fast video (dwFlags 0x%x, dwRGBBitCount %d, lpSurface 0x%x)",
			desc.ddpfPixelFormat.dwFlags, desc.ddpfPixelFormat.dwRGBBitCount, desc.lpSurface);
		warned = true;
	} else {
		// All is fine. Remember memory buffer (so we can set it back later, and as flag).
		oldDataPointer = desc.lpSurface;
	}
	if (oldDataPointer) {
		// Replace memory buffer pointer inside surface.
		assert(m_ddsurf7);
		memset(&desc, 0, sizeof desc);
		desc.dwSize = sizeof desc;
		desc.dwFlags = DDSD_LPSURFACE;
		desc.lpSurface = m_frame;
		hr = m_ddsurf7->SetSurfaceDesc(&desc, 0);
		if (FAILED(hr)) {
			if (!warned) lib::logger::get_logger()->trace("dx_dsvideo: Cannot set lpSurface, no fast video (error 0x%x)", hr);
			warned = true;
			oldDataPointer = NULL;
		}
		if (FAILED(hr)) {
			lib::logger::get_logger()->trace("dx_dsvideo: Unlock: error 0x%x", hr);
		}
	}
#endif // ENABLE_FAST_DXVIDEO
	if (!oldDataPointer) {
		// Could not replace data pointer. Use bitblit to copy data.
		if (m_bitmap == NULL) _init_bitmap();
		//bo 17-03-2008, for some reasons, somtimes, m_frame == null;
		if (m_frame == NULL){
			m_lock.leave();
			return;
		}
		memcpy(m_bitmap_dataptr, m_frame, m_frame_size);
		_copy_to_ddsurf();
	}
	//const rect &r = m_dest->get_rect();
	//AM_DBG logger::get_logger()->debug("dx_dsvideo_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	lib::rect img_rect1;
	lib::rect img_reg_rc;
	// lib::size srcsize = m_image->get_size();

	lib::rect croprect = m_dest->get_crop_rect(m_size);
	img_reg_rc = m_dest->get_fit_rect(croprect, m_size, &img_rect1, m_alignment);
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
#ifdef ENABLE_FAST_DDVIDEO
	if (oldDataPointer) {
		// Replace memory buffer pointer inside surface.
		assert(m_ddsurf7);
		memset(&desc, 0, sizeof desc);
		desc.dwSize = sizeof desc;
		desc.dwFlags = DDSD_LPSURFACE;
		desc.lpSurface = oldDataPointer;
		hr = m_ddsurf7->SetSurfaceDesc(&desc, 0);
		if (FAILED(hr)) {
			lib::logger::get_logger()->trace("dx_dsvideo: Cannot reset lpSurface??");
		}
	}
#endif
	m_lock.leave();
}

} // namespace dx

} // namespace gui

} //namespace ambulant

