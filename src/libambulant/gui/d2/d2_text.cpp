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

#include "ambulant/gui/d2/d2_text.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace d2 {

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF((float) r.left(), (float) r.top(), (float) r.right(), (float) r.bottom());
}

extern const char d2_text_playable_tag[] = "text";
extern const char d2_text_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirect2D");
extern const char d2_text_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererText");

IDWriteFactory *ambulant::gui::d2::d2_text_renderer::s_write_factory = NULL;

common::playable_factory *
create_d2_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirect2D"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererText"), true);
	return new common::single_playable_factory<
		d2_text_renderer,
		d2_text_playable_tag,
		d2_text_playable_renderer_uri,
		d2_text_playable_renderer_uri2,
		d2_text_playable_renderer_uri2>(factory, mdp);
}

d2_text_renderer::d2_text_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
:	d2_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
	m_text_format(NULL),
	m_brush(NULL)
{
	if (s_write_factory == NULL) {
		HRESULT hr;
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(s_write_factory),
			reinterpret_cast<IUnknown**>(&s_write_factory));
		if (!SUCCEEDED(hr)) {
			lib::logger::get_logger()->error("Cannot create DirectWrite factory: error 0x%x", hr);
		}
	}
}

void
d2_text_renderer::init_with_node(const lib::node *node)
{
	lib::textptr font_name("Helvetica");
	float font_size = 14.0;
	m_text_color = 0;

	smil2::params *params = smil2::params::for_node(node);
	if (params) {
		lib::textptr fn = params->get_str("font-family");
		if (fn != (lib::textptr::const_char_ptr) NULL) {
			font_name = fn;
		}
		font_size = params->get_float("font-size", 14.0);
		m_text_color = params->get_color("color", 0);
		delete params;
	}
	if (m_text_format) {
		m_text_format->Release();
		m_text_format = NULL;
	}
	if (m_brush) {
		m_brush->Release();
		m_brush = NULL;
	}
	if (!s_write_factory) return;
	HRESULT hr;
	hr = s_write_factory->CreateTextFormat(
		font_name.wstr(),
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		font_size,
		L"",
		&m_text_format);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->error("Cannot create DirectWrite TextFormat: error 0x%x", hr);
	}
}

d2_text_renderer::~d2_text_renderer()
{
	m_lock.enter();
	m_lock.leave();
}

void
d2_text_renderer::redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget* rt)
{
	// XXXJACK: if our color.opacity has been animated we should re-create
	recreate_d2d();

	m_lock.enter();
	if (!m_data || !m_text_format || !m_brush) {
		m_lock.leave();
		return;
	}
	assert(rt);
	if (rt == NULL)
		return;
	rect destrect = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("d2_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, destrect.left(), destrect.top(), destrect.right(), destrect.bottom());

	destrect.translate(m_dest->get_global_topleft());
	
	assert(strlen((char*)m_data) <= m_data_size);
	lib::textptr text_data((char *)m_data);

	rt->DrawText(
		text_data.c_wstr(),
		wcslen(text_data.c_wstr()),
		m_text_format,
		d2_rectf(destrect),
		m_brush);

	m_lock.leave();
}

void
d2_text_renderer::recreate_d2d()
{
	if (m_brush) return;
	m_lock.enter();
	HRESULT hr = S_OK;
	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);

	double alfa = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
	hr = rt->CreateSolidColorBrush(D2D1::ColorF(redf(m_text_color), greenf(m_text_color), bluef(m_text_color), alfa), &m_brush);
	if (!SUCCEEDED(hr)) lib::logger::get_logger()->trace("CreateSolidColorBrush: error 0x%x", hr);
	rt->Release();
	m_lock.leave();
}

void
d2_text_renderer::discard_d2d()
{
	if (m_brush) {
		m_brush->Release();
		m_brush = NULL;
	}
}

} // namespace d2

} // namespace gui

} //namespace ambulant
