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
#include "ambulant/config/config.h"
#include <objbase.h>
#include <ddraw.h>
#include <windows.h>

#include "ambulant/gui/dx/dx_image_renderer.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/dx/jpg_decoder.h"
#include "ambulant/gui/dx/gif_decoder.h"
#include "ambulant/gui/dx/png_decoder.h"
#include "ambulant/gui/dx/bmp_decoder.h"
#include "ambulant/gui/dx/dx_viewport.h"

#include "ambulant/lib/colors.h"
#include "ambulant/lib/memfile.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"

using namespace ambulant;
using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
using ambulant::lib::logger;

using lib::uint16;
using lib::uint32;
using lib::uchar;

typedef gui::dx::img_decoder<lib::memfile, lib::color_trible> img_decoder_class;

static img_decoder_class*
create_img_decoder(lib::memfile *src, HDC hdc) {
	typedef gui::dx::jpg_decoder<lib::memfile, lib::color_trible> jpg_decoder_class;
	typedef gui::dx::gif_decoder<lib::memfile, lib::color_trible> gif_decoder_class;
	typedef gui::dx::png_decoder<lib::memfile, lib::color_trible> png_decoder_class;
	typedef gui::dx::bmp_decoder<lib::memfile, lib::color_trible> bmp_decoder_class;

	img_decoder_class* decoder = 0;

	decoder = new jpg_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;

	decoder = new gif_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;

	decoder = new png_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;

	decoder = new bmp_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;

	return 0;
}

gui::dx::image_renderer::image_renderer(const net::url& u, net::datasource *src, viewport* v)
:	m_url(u),
	m_ddsurf(0),
	m_transparent(false) {
	open(src, v);
}

gui::dx::image_renderer::~image_renderer() {
	if(m_ddsurf) m_ddsurf->Release();
}

void gui::dx::image_renderer::open(net::datasource *src, viewport* v) {
	lib::memfile mf(src);
	if(!mf.read())
		return;

	// Decode the image
	HDC hdc = ::GetDC(NULL);
	img_decoder_class* decoder = create_img_decoder(&mf, hdc);
	if ( ! ::ReleaseDC(NULL, hdc))
		lib::logger::get_logger()->warn("%s failed", "dx::image_renderer::open: ::ReleaseDC(NULL, hdc)");
	if(!decoder) {
		lib::logger::get_logger()->show("%s: Cannot create image decoder", m_url.get_url().c_str());
		return;
	}

	dib_surface<lib::color_trible>* dibsurf = decoder->decode();
	if(!dibsurf) {
		lib::logger::get_logger()->warn("%s: Cannot decode image", m_url.get_url().c_str());
		delete decoder;
		return;
	}
	m_transparent = decoder->is_transparent();
	lib::color_t tarnsp_color = decoder->get_transparent_color();
	delete decoder;

	m_size.w = DWORD(dibsurf->get_pixmap()->get_width());
	m_size.h = DWORD(dibsurf->get_pixmap()->get_height());
	m_ddsurf = v->create_surface(m_size);
	if(!m_ddsurf) {
		delete dibsurf;
		return;
	}

	//////////////
	// Create image surface

	HRESULT hr;
	hr = m_ddsurf->GetDC(&hdc);
	if (FAILED(hr)) {
		win_report_error("DirectDrawSurface::GetDC()", hr);
		delete dibsurf;
		return;
	}
	HDC bmp_hdc = CreateCompatibleDC(hdc);
	HBITMAP hbmp_old = (HBITMAP) SelectObject(bmp_hdc, dibsurf->get_handle());
	::BitBlt(hdc, 0, 0, m_size.w, m_size.h, bmp_hdc, 0, 0, SRCCOPY);
	SelectObject(bmp_hdc, hbmp_old);
	DeleteDC(bmp_hdc);
	m_ddsurf->ReleaseDC(hdc);
	delete dibsurf;

	//////////////
	// If the image is transparent set the color
	if(m_transparent) {
		DWORD ddTranspColor = v->convert(tarnsp_color);
		DWORD dwFlags = DDCKEY_SRCBLT;
		DDCOLORKEY ck;
		ck.dwColorSpaceLowValue = ddTranspColor;
		ck.dwColorSpaceHighValue = ddTranspColor;
		hr = m_ddsurf->SetColorKey(dwFlags, &ck);
		if (FAILED(hr)) {
			win_report_error("SetColorKey()", hr);
		}
	}
}


