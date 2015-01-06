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

namespace ambulant {

using namespace lib;

namespace gui {

namespace d2 {

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF((FLOAT)r.left(), (FLOAT)r.top(), (FLOAT)r.right(), (FLOAT)r.bottom());
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

void
d2_dsvideo_renderer::redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget* rt)
{
	recreate_d2d();
	m_lock.enter();
	if(!m_d2bitmap) {
		// No bits available
		AM_DBG lib::logger::get_logger()->debug("d2_img_renderer::redraw NOT: no image or cannot play %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		m_lock.leave();
		return;
	}
	assert(rt);
	if (rt == NULL)
		return;

	_frame_was_displayed();
	lib::rect img_rect1;
	lib::rect img_reg_rc;

	double alpha_media = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
	}
	lib::rect croprect = m_dest->get_crop_rect(m_size);
	img_reg_rc = m_dest->get_fit_rect(croprect, m_size, &img_rect1, m_alignment);
	// Use one type of rect to do op
	lib::rect img_rect(img_rect1);


	// Translate img_reg_rc_dirty to viewport coordinates
	lib::point topleft = m_dest->get_global_topleft();
	img_reg_rc.translate(topleft);


	// Finally blit img_rect_dirty to img_reg_rc_dirty
	AM_DBG lib::logger::get_logger()->debug("d2_dsvideo_renderer::redraw %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
	rt->DrawBitmap(
		m_d2bitmap,
		d2_rectf(img_reg_rc),
		(FLOAT)alpha_media,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		d2_rectf(img_rect));

	m_lock.leave();
}


void
d2_dsvideo_renderer::recreate_d2d()
{
	m_lock.enter();
	// Check that we actually have work to do 
	if (m_d2bitmap || m_frame == NULL) {
		m_lock.leave();
		return;
	}
	D2D1_SIZE_U size = {m_size.w, m_size.h};
	D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties();
	props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);
	HRESULT hr = rt->CreateBitmap(size, m_frame, m_size.w*4, props, &m_d2bitmap);
	if (!SUCCEEDED(hr))
		lib::logger::get_logger()->trace("CreateBitmapFromWicBitmap: error 0x%x", hr);
	rt->Release();
	m_lock.leave();
}

void
d2_dsvideo_renderer::discard_d2d()
{
	m_lock.enter();
	if (m_d2bitmap) {
		m_d2bitmap->Release();
		m_d2bitmap = NULL;
	}
	m_lock.leave();
}

} // namespace d2

} // namespace gui

} //namespace ambulant

