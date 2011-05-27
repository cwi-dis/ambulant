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

#include "ambulant/gui/d2/d2_img.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/colors.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/test_attrs.h"

#include <wincodec.h>
#include <d2d1.h>
#include <d2d1helper.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF((FLOAT) r.left(), (FLOAT) r.top(), (FLOAT) r.right(), (FLOAT) r.bottom());
}

extern const char d2_img_playable_tag[] = "img";
extern const char d2_img_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirect2D");
extern const char d2_img_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererWicImg");
extern const char d2_img_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererImg");

IWICImagingFactory *gui::d2::d2_img_renderer::s_wic_factory = NULL;

common::playable_factory *
gui::d2::create_d2_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirect2D"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererWicImg"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererImg"), true);
	return new common::single_playable_factory<
		gui::d2::d2_img_renderer,
		d2_img_playable_tag,
		d2_img_playable_renderer_uri,
		d2_img_playable_renderer_uri2,
		d2_img_playable_renderer_uri3 >(factory, mdp);
}

gui::d2::d2_img_renderer::d2_img_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	d2_renderer<renderer_playable>(context, cookie, node, evp, fp, mdp),
	m_original(NULL),
	m_d2bitmap(NULL),
	m_databuf(NULL),
	m_factory(fp)
{
	if (s_wic_factory == NULL) {
		// init wic factory
		HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&s_wic_factory)
			);
		assert(SUCCEEDED(hr));
	}

	AM_DBG lib::logger::get_logger()->debug("d2_img_renderer::ctr(0x%x)", this);
}

gui::d2::d2_img_renderer::~d2_img_renderer() {
	AM_DBG lib::logger::get_logger()->debug("d2_img_renderer::dtr(0x%x)", this);
	if (m_original) {
		m_original->Release();
		m_original = NULL;
	}
	if (m_databuf) {
		free(m_databuf);
		m_databuf = NULL;
	}
	discard_d2d();
}

void gui::d2::d2_img_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("d2_img_renderer::start(0x%x)", this);
	if (s_wic_factory == NULL) {
		lib::logger::get_logger()->error("Windows Imaging Component not initialized");
		return;
	}
	start_transition(t);

	HRESULT hr = S_OK;
	IWICBitmapDecoder *decoder = NULL;
	IWICFormatConverter *converter = NULL;
	IWICBitmapFrameDecode *frame = NULL;
	IWICBitmapSource *source = NULL;

	net::url url = m_node->get_url("src");
	if (url.is_local_file()) {
		// Local file, let WIC access it directly
		std::string filename_str(url.get_file());
		lib::textptr filename(filename_str.c_str());
		hr = s_wic_factory->CreateDecoderFromFilename(
			filename.wstr(),
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&decoder);
		if (!SUCCEEDED(hr)) {
			lib::logger::get_logger()->trace("%s: cannot CreateDecoderFromFilename: error 0x%x", url.get_url().c_str(), hr);
			lib::logger::get_logger()->error("%s: cannot open image", url.get_url().c_str());
			goto fail;
		}
	} else {
		// Remote file, go through data buffer, etc.
		size_t size;
		assert(m_databuf == NULL);
		if ( !net::read_data_from_url(url, m_factory->get_datasource_factory(), &m_databuf, &size)) {
			m_context->stopped(m_cookie);
			return;
		}
		IWICStream *stream = NULL;
		hr = s_wic_factory->CreateStream(&stream);
		if (!SUCCEEDED(hr)) {
			ambulant::lib::logger::get_logger()->error("Cannot create Windows Imaging Component stream");
			goto fail;
		}
		hr = stream->InitializeFromMemory((BYTE *)m_databuf, size);
		if (!SUCCEEDED(hr)) {
			ambulant::lib::logger::get_logger()->error("Cannot create Windows Imaging Component stream from buffer");
			stream->Release();
			goto fail;
		}
		hr = s_wic_factory->CreateDecoderFromStream(
			stream,
			NULL,
			WICDecodeMetadataCacheOnDemand,
			&decoder);
		// We release the stream now, before testing success. It will be increffed
		// in case things worked fine.
		stream->Release();
		if (!SUCCEEDED(hr)) {
			ambulant::lib::logger::get_logger()->error("Cannot create Windows Imaging Component decoder from stream");
			goto fail;
		}
	}
	// Get the first from the bitmap file
	hr = decoder->GetFrame(0, &frame);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: GetFrame() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}

	// Store this frame as a WIC bitmap source
	hr = frame->QueryInterface(IID_IWICBitmapSource, (void**)&source);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: QueryInterface() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}

	// Convert to required pixel format
	hr = s_wic_factory->CreateFormatConverter(&converter);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: CreateFormatConverter() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}
	hr = converter->Initialize(
		source,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.0,
		WICBitmapPaletteTypeCustom);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: FormatConverter::Initialize() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}

	// Get access to the converter as a bitmap source, and keep this
	// for future reference.
//	assert(m_original == NULL);
	hr = converter->QueryInterface(IID_PPV_ARGS(&m_original));
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("WIC image renderer: FormatConverter::QueryInterface() returned 0x%x", hr);
		lib::logger::get_logger()->error("%s: cannot render image", url.get_url().c_str());
		goto fail;
	}
	
	// Make sure the DD surface is created, next redraw
//	assert(m_d2bitmap == NULL);

	// Inform the scheduler, ask for a redraw
	m_context->started(m_cookie);
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	//m_dest->need_redraw(); // show already did this

fail:
	// Release things
	if (converter) converter->Release();
	if (source) source->Release();
	if (frame) frame->Release();
	if (decoder) decoder->Release();

	// Notify scheduler that we're done playing
	m_context->stopped(m_cookie);
}

bool gui::d2::d2_img_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("d2_img_renderer::stop(0x%x)", this);
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_context->stopped(m_cookie);
	return true;
}

bool gui::d2::d2_img_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void
gui::d2::d2_img_renderer::recreate_d2d()
{
	if (m_d2bitmap) return;
	if (m_original == NULL) return;
	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);
	if (rt == NULL) return;

	HRESULT hr = rt->CreateBitmapFromWicBitmap(m_original, NULL, &m_d2bitmap);
	if (!SUCCEEDED(hr))
		lib::logger::get_logger()->trace("CreateBitmapFromWicBitmap: error 0x%x", hr);
#ifdef	AM_DMP

	m_d2player->dump (rt, "d2_image-recreate2");

#endif//AM_DMP

}

void
gui::d2::d2_img_renderer::discard_d2d()
{
	if (m_d2bitmap) {
//		m_d2bitmap->Release();
		m_d2bitmap = NULL;
	}
}

void gui::d2::d2_img_renderer::redraw_body(const lib::rect& dirty, common::gui_window *window, ID2D1RenderTarget* rt) {
	recreate_d2d();
	if(!m_d2bitmap) {
		// No bits available
		AM_DBG lib::logger::get_logger()->debug("d2_img_renderer::redraw NOT: no image or cannot play %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}
	assert(rt);
	if (rt == NULL) return;
	lib::rect img_rect1;
	lib::rect img_reg_rc;
	UINT w, h;
	HRESULT hr = m_original->GetSize(&w, &h);
	assert(hr == S_OK);
	if (hr != S_OK) return;
	lib::size srcsize(w, h);
	AM_DBG lib::logger::get_logger()->debug("d2_img_renderer::redraw(0x%x) rt=0x%x", this, rt);

	// This code could be neater: it could share quite a bit with the
	// code below (for non-tiled images). Also, support for tiled images
	// is specifically geared toward background images: stuff like the
	// dirty region and transitions are ignored.
	// Also, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") &&	 m_dest->is_tiled()) {
		AM_DBG lib::logger::get_logger()->debug("d2_img_renderer.redraw: drawing tiled image");
		img_reg_rc = m_dest->get_rect();
		img_reg_rc.translate(m_dest->get_global_topleft());
		common::tile_positions tiles = m_dest->get_tiles(srcsize, img_reg_rc);
		common::tile_positions::iterator it;
		for(it=tiles.begin(); it!=tiles.end(); it++) {
			img_rect1 = (*it).first;
			img_reg_rc = (*it).second;
			rt->DrawBitmap(
				m_d2bitmap,
				d2_rectf(img_reg_rc),
				1.0f,
				D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				d2_rectf(img_rect1));
		}

		if (m_erase_never) m_dest->keep_as_background();
		return;
	}
#ifdef	AM_DMP

//	m_d2player->dump (rt, "d2_image-redraw2");

#endif//AM_DMP


	lib::rect croprect = m_dest->get_crop_rect(srcsize);
//	AM_DBG lib::logger::get_logger()->debug("get_crop_rect(%d,%d) -> (%d, %d, %d, %d)", srcsize.w, srcsize.h, croprect.left(), croprect.top(), croprect.width(), croprect.height());
	img_reg_rc = m_dest->get_fit_rect(croprect, srcsize, &img_rect1, m_alignment);
	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			lib::compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		} else alpha_chroma = alpha_media;
	}
	img_reg_rc.translate(m_dest->get_global_topleft());

	lib::rect img_rect(img_rect1);
	rt->DrawBitmap(
		m_d2bitmap,
		d2_rectf(img_reg_rc),
		alpha_media,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		d2_rectf(img_rect));


#ifdef NOTYET
	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}

	if(tr && tr->is_outtrans() && m_original) {
		// First draw the background color, if applicable
		const common::region_info *ri = m_dest->get_info();
		if(ri)
			v->clear(img_reg_rc_dirty,ri->get_bgcolor(), ri->get_bgopacity());
		// Next, take a snapshot of the relevant pixels as they are now, before we draw the image
		UINT w, h;
		HRESULT hr = m_original->GetSize(&w, &h);
		assert(hr == 0);
		lib::size image_size(w, h);
		IDirectDrawSurface *bgimage = v->create_surface(image_size);
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
		bgimage->Blt(&bgrect_image, m_ddsurf, &bgrect_screen, WAITFLAG, NULL);
		// Then draw the image
		v->draw(m_ddsurf, img_rect_dirty, img_reg_rc_dirty, 0 /*m_image->is_transparent()*/, (dx_transition*)0);
		// And finally transition in the background bits saved previously
		v->draw(bgimage, img_rect_dirty, img_reg_rc_dirty, false, tr);
		bgimage->Release();
	} else {
		if (alpha_chroma != 1.0) {
			IDirectDrawSurface* screen_ddsurf = v->get_surface();
			IDirectDrawSurface* image_ddsurf = m_ddsurf;
			lib::rect rct0 (lib::point(0, 0), img_reg_rc_dirty.size());
			v->blend_surface(
				img_reg_rc_dirty,
				image_ddsurf,
				rct0,
				0 /*m_image->is_transparent()*/,
				alpha_chroma,
				alpha_media,
				chroma_low,
				chroma_high);
		} else {
			v->draw(m_ddsurf, img_rect_dirty, img_reg_rc_dirty,0 /* m_image->is_transparent()*/, tr);
		}
	}
#endif
	if (m_erase_never) m_dest->keep_as_background();
}
