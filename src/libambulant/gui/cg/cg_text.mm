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

#include "ambulant/gui/cg/cg_text.h"
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

extern const char cg_text_playable_tag[] = "text";
extern const char cg_text_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCoreGraphics");
extern const char cg_text_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererText");

common::playable_factory *
create_cg_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCoreGraphics"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererText"), true);
	return new common::single_playable_factory<
		cg_text_renderer,
		cg_text_playable_tag,
		cg_text_playable_renderer_uri,
		cg_text_playable_renderer_uri2,
		cg_text_playable_renderer_uri2>(factory, mdp);
}


cg_text_renderer::cg_text_renderer(
    playable_notification *context,
    playable_notification::cookie_type cookie,
    const lib::node *node,
    event_processor *evp,
    common::factories *factory,
    common::playable_factory_machdep *mdp)
:	cg_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
	m_font_name(NULL),
	m_font_size(0.0)
{
	smil2::params *params = smil2::params::for_node(node);

	if (params) {
		m_font_name = params->get_str("font-family");
        if (m_font_name == NULL)
            m_font_name = "Helvetica";
		m_font_size = params->get_float("font-size", 14.0);
		m_text_color = params->get_color("color", 0);
		delete params;
	} else {
		m_font_name = "Helvetica";
		m_font_size = 14.0;
		m_text_color = 0;
	}
}

cg_text_renderer::~cg_text_renderer()
{
	m_lock.enter();
	m_lock.leave();
}

void
cg_text_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	if (!m_data) {
		m_lock.leave();
		return;
	}
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cg_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	CGContextRef ctx = [view getCGContext];
	rect dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	CGRect cg_dstrect = CGRectFromAmbulantRect(dstrect);

	// Set the text matrix. 
	CGAffineTransform matrix = [view transformForRect: &cg_dstrect flipped: YES translated: NO];
	CGContextSetTextMatrix(ctx, matrix);

	// Set the color
	double alfa = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
	CGFloat components[] = {redf(m_text_color), greenf(m_text_color), bluef(m_text_color), alfa};
	CGColorSpaceRef genericColorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextSetFillColorSpace(ctx, genericColorSpace);
	CGContextSetFillColor(ctx, components);

	// Set the font
	AM_DBG lib::logger::get_logger()->debug("cg_text: select font %s, size %f", m_font_name, m_font_size);
	CGContextSelectFont(ctx, m_font_name, m_font_size, kCGEncodingMacRoman);

	// Calculate sizes
	float lineheight = m_font_size;
	float x = CGRectGetMinX(cg_dstrect);
	float y = CGRectGetMinY(cg_dstrect) + lineheight;
	float w = CGRectGetWidth(cg_dstrect);

    // Now loop, computing how many characters fit and then drawing them on a line.
	size_t lbegin, lend;
	const char *cdata = (char *)m_data;
	lbegin = 0;
	lend = 0;
	while(_calc_fit(ctx, w, lbegin, lend) ) {
		AM_DBG lib::logger::get_logger()->debug("cg_text: draw line at (%f, %f)", x, y);
		CGContextSetTextPosition(ctx, x, y);
		AM_DBG{ CGAffineTransform mtx = CGContextGetTextMatrix(ctx); lib::logger::get_logger()->debug("cg_text: textmatrix: (%f, %f) (%f, %f) (%f, %f)", mtx.a, mtx.b, mtx.c, mtx.d, mtx.tx, mtx.ty); }
		AM_DBG{ CGAffineTransform mtx = CGContextGetCTM(ctx); lib::logger::get_logger()->debug("cg_text: matrix: (%f, %f) (%f, %f) (%f, %f)", mtx.a, mtx.b, mtx.c, mtx.d, mtx.tx, mtx.ty); }
		CGContextSetTextDrawingMode(ctx, kCGTextFill);
		CGContextShowText(ctx, cdata+lbegin, lend-lbegin);
		lbegin = lend;
		y += lineheight;
	}
    
	CGColorSpaceRelease(genericColorSpace);
	m_lock.leave();
}

bool
cg_text_renderer::_calc_fit(CGContextRef ctx, float width, size_t& lbegin, size_t& lend)
{
	const char *cdata = (const char *)m_data;

	// Find beginning point: we skip spacing characters at begin-of-line
    // XXXJACK is this correct according to the SMIL spec?
	if (lbegin > 0)
		while (lbegin < m_data_size && isspace(cdata[lbegin])) lbegin++;
	if (cdata[lbegin] == '\0' || lbegin >= m_data_size) return false;
    
    // We will draw at least one character per line
	lend = lbegin+1;
    
    // We search for the next space, and if the text up to there still fits on the line
    // we will draw at least up to there. Loop until it no longer fits.
	int lendcand = lend;
	do {
		while (cdata[lendcand] != '\0' && lendcand < m_data_size && !isspace(cdata[lendcand])) lendcand++;
		if (!_fits(ctx, width, cdata+lbegin, lendcand-lbegin))
			return true;
		lend = lendcand;
		if (cdata[lend] == '\r' || cdata[lend] == '\n')
			return true;
		while (isspace(cdata[lendcand]) && lendcand < m_data_size) lendcand++;
	} while(cdata[lendcand] != '\0' && lendcand < m_data_size);
    
	return true;
}

bool
cg_text_renderer::_fits(CGContextRef ctx, float maxwidth, const char *data, size_t datalen)
{
    // Compute whether the text fits by drawing it invisibly and checking that it doesn't
    // extend past maxwidth. 
	CGPoint beginpos = CGContextGetTextPosition(ctx);
	CGContextSetTextDrawingMode(ctx, kCGTextInvisible);
	CGContextShowText(ctx, data, datalen);
	CGPoint endpos = CGContextGetTextPosition(ctx);
	float width = endpos.x - beginpos.x;
	return width <= maxwidth;
}


} // namespace cg

} // namespace gui

} //namespace ambulant
