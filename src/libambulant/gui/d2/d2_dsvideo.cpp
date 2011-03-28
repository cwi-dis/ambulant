// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
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

#include "ambulant/gui/d2/d2_dsvideo.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/lib/win32/win32_error.h"
using ambulant::lib::win32::win_report_error;

#include <wincodec.h>
#include <d2d1.h>
#include <d2d1helper.h>

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

namespace d2 {

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF(r.left(), r.top(), r.right(), r.bottom());
}

d2_dsvideo_renderer::d2_dsvideo_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	d2_renderer(context, cookie, node, evp, factory, mdp),
	m_frame(NULL),
	m_d2bitmap(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("d2_dsvideo_renderer(): 0x%x created", (void*)this);
}

net::pixel_order
d2_dsvideo_renderer::pixel_layout()
{
	return MY_PIXEL_LAYOUT;
}

#ifdef JNK
void
d2_dsvideo_renderer::_init_bitmap()
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
d2_dsvideo_renderer::_init_ddsurf(gui_window *window)
{
	if (m_size.h == 0 || m_size.w == 0) return;
	assert(m_ddsurf == NULL);
	d2_window *d2window = static_cast<d2_window*>(window);
	viewport *v = d2window->get_viewport();
	assert(v);
	m_ddsurf = v->create_surface(m_size);
#ifdef ENABLE_FAST_DDVIDEO
	// Error message will be given later, if needed
	(void)m_ddsurf->QueryInterface(myIID_IDirectDrawSurface7, (void**)&m_ddsurf7);
#endif
}
#endif

d2_dsvideo_renderer::~d2_dsvideo_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~d2_dsvideo_renderer(0x%x)", (void *)this);
	if (m_d2bitmap) {
		m_d2bitmap->Release();
		m_d2bitmap = NULL;
	}
	if (m_frame)
		free(m_frame);
	m_frame = NULL;
	m_lock.leave();
}

void
d2_dsvideo_renderer::_push_frame(char* frame, size_t size)
{
	if (m_frame) free(m_frame);
	m_frame = frame;
	m_frame_size = size;
	AM_DBG lib::logger::get_logger()->debug("d2_dsvideo_renderer::_push_frame(0x%x, %d)", frame, size);
	assert(size == (m_size.w * m_size.h * 4));
	discard_d2d();
}

#ifdef JNK
void
d2_dsvideo_renderer::_copy_to_ddsurf()
{
	if (m_bitmap == NULL || m_ddsurf == NULL) return;
	HRESULT hr;
	HDC hdc;
	hr = m_ddsurf->GetDC(&hdc);
	AM_DBG lib::logger::get_logger()->debug("d2_dsvideo_renderer::_copy_to_ddsurf(), Called GetDC");
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
	AM_DBG lib::logger::get_logger()->debug("d2_dsvideo_renderer::_copy_to_ddsurf(), Called ReleaseDC");
}
#endif

void
d2_dsvideo_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	recreate_d2d();
	if(!m_d2bitmap) {
		// No bits available
		AM_DBG lib::logger::get_logger()->debug("d2_img_renderer::redraw NOT: no image or cannot play %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		m_lock.leave();
		return;
	}
	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);
	lib::rect img_rect1;
	lib::rect img_reg_rc;

	double alpha_media;
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
//???		alpha_media_bg = ri->get_mediabgopacity();
//???		m_bgopacity = ri->get_bgopacity();
#ifdef JNK
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			lib::compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		} else alpha_chroma = alpha_media;
#endif
	}
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
		AM_DBG lib::logger::get_logger()->debug("d2_dsvideo_renderer::redraw NOT: empty dirty region %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
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
	AM_DBG lib::logger::get_logger()->debug("d2_dsvideo_renderer::redraw %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
	rt->DrawBitmap(
		m_d2bitmap,
		d2_rectf(img_reg_rc),
		alpha_media,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		d2_rectf(img_rect));

	m_lock.leave();
}


void
d2_dsvideo_renderer::recreate_d2d()
{
	if (m_d2bitmap) return;
	if (m_frame == NULL) return;
	D2D1_SIZE_U size = {m_size.w, m_size.h};
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
 	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);
	HRESULT hr = rt->CreateBitmap(size, m_frame, m_size.w*4, props, &m_d2bitmap);
	if (!SUCCEEDED(hr))
		lib::logger::get_logger()->trace("CreateBitmapFromWicBitmap: error 0x%x", hr);
}

void
d2_dsvideo_renderer::discard_d2d()
{
	if (m_d2bitmap) {
		m_d2bitmap->Release();
		m_d2bitmap = NULL;
	}
}

} // namespace d2

} // namespace gui

} //namespace ambulant

