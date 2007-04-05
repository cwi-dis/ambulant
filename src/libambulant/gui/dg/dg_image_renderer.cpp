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

#include "ambulant/config/config.h"
#include <objbase.h>
#include <windows.h>

#include "ambulant/gui/dg/dg_image_renderer.h"

#include "ambulant/lib/colors.h"
#include "ambulant/lib/memfile.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define AM_JPG
#define AM_PNG
#define AM_GIF
#define AM_BMP

#ifdef AM_JPG
#include "ambulant/gui/dg/jpg_decoder.h"
#endif

#ifdef AM_PNG
#include "ambulant/gui/dg/png_decoder.h"
#endif

#ifdef AM_GIF
#include "ambulant/gui/dg/gif_decoder.h"
#endif

#ifdef AM_BMP
#include "ambulant/gui/dg/bmp_decoder.h"
#endif

#include "ambulant/gui/dg/dg_viewport.h"

using namespace ambulant;
using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
using ambulant::lib::logger;

typedef gui::dg::img_decoder<lib::memfile> img_decoder_class;

static img_decoder_class*
create_img_decoder(lib::memfile *src, HDC hdc) {

	img_decoder_class* decoder = 0;

#ifdef AM_JPG
	typedef gui::dg::jpg_decoder<lib::memfile> jpg_decoder_class;
	decoder = new jpg_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;
#endif

#ifdef AM_PNG
	typedef gui::dg::png_decoder<lib::memfile> png_decoder_class;
	decoder = new png_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;
#endif

#ifdef AM_GIF
	typedef gui::dg::gif_decoder<lib::memfile> gif_decoder_class;
	decoder = new gif_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;	
#endif

#ifdef AM_BMP
	typedef gui::dg::bmp_decoder<lib::memfile> bmp_decoder_class;
	decoder = new bmp_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;
#endif
	return 0;
}

gui::dg::image_renderer::image_renderer(const net::url& u, net::datasource *src, viewport* v)
:	m_url(u),
	m_dibsurf(0),
	m_transparent(false),
	m_transp_color(CLR_INVALID) {
	open(src, v);
}

gui::dg::image_renderer::~image_renderer() {
	if(m_dibsurf) delete m_dibsurf;
}

void gui::dg::image_renderer::open(net::datasource *src, viewport* v) {
	lib::memfile mf(src);
	if(!mf.read()) {
		lib::logger::get_logger()->show("%s: Cannot read data", m_url.get_url().c_str());
		return;
	}
	
	// Decode the image
	HDC hdc = ::GetDC(NULL);
	img_decoder_class* decoder = create_img_decoder(&mf, hdc);
	::DeleteDC(hdc);
	if(!decoder) {
		lib::logger::get_logger()->show("%s: Cannot create image decoder", m_url.get_url().c_str());
		return;
	}
	
	m_dibsurf = decoder->decode();
	if(!m_dibsurf) {
		lib::logger::get_logger()->show("%s: Cannot decode image", m_url.get_url().c_str());
		delete decoder;
		return;
	}
	m_transparent = decoder->is_transparent();
	m_transp_color = decoder->get_transparent_color();
	delete decoder;
	
	m_size.w = DWORD(m_dibsurf->get_pixmap()->get_width());
	m_size.h = DWORD(m_dibsurf->get_pixmap()->get_height());
}
 

