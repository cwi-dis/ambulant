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

#include "ambulant/gui/cg/atsui_text.h"
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"

//#include <CoreGraphics/CoreGraphics.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

atsui_text_renderer::atsui_text_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
:	cg_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
	m_text_storage(NULL),
	m_style(NULL),
	m_layout_manager(NULL)
{
	OSStatus err;
	ATSUAttributeTag tags[1];
	ByteCount sizes[1];
	ATSUAttributeValuePtr values[1];
	smil2::params *params = smil2::params::for_node(node);
	color_t text_color = lib::to_color(0, 0, 0);
	err = ATSUCreateStyle(&m_style);
	assert(err == 0);
	if (params) {
		const char *fontname = params->get_str("font-family");
		if (fontname) {
			ATSUFontID font;

			err = ATSUFindFontFromName(fontname, strlen(fontname), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font);
			if (err) {
				lib::logger::get_logger()->trace("%s: font \"%s\" not found", m_node->get_sig().c_str(), fontname);
			} else {
				tags[0] = kATSUFontTag;
				sizes[0] = sizeof(ATSUFontID);
				values[0] = &font;
				err = ATSUSetAttributes(m_style, 1, tags, sizes, values);
				assert(err == 0);
			}
		}

//		const char *fontstyle = params->get_str("font-style");
		float fontsize = params->get_float("font-size", 0.0);
		if (fontsize) {
			Fixed ffontsize = FloatToFixed(fontsize);
			tags[0] = kATSUSizeTag;
			sizes[0] = sizeof(Fixed);
			values[0] = &ffontsize;
			err = ATSUSetAttributes(m_style, 1, tags, sizes, values);
			assert(err == 0);
		}
		m_text_color = params->get_color("color", text_color);
		delete params;
	}
}

atsui_text_renderer::~atsui_text_renderer()
{
	m_lock.enter();
	if (m_style) ATSUDisposeStyle(m_style);
	m_style = NULL;
	if (m_layout_manager) ATSUDisposeTextLayout(m_layout_manager);
	m_layout_manager = NULL;
	if (m_text_storage) free(m_text_storage);
	m_text_storage = NULL;
	m_lock.leave();
}

void
atsui_text_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	OSStatus err;
	ATSUAttributeTag tags[1];
	ByteCount sizes[1];
	ATSUAttributeValuePtr values[1];

	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("atsui_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	if (m_data && !m_text_storage) {
		// Convert through NSString. We hope this will get the unicode intricacies right.
		NSString *the_string = [[NSString alloc] initWithBytesNoCopy: m_data
				length: (unsigned int)m_data_size
				encoding: NSUTF8StringEncoding
				freeWhenDone: NO];
		m_text_storage_length = [the_string length];
		m_text_storage = (UniChar *)malloc(m_text_storage_length*sizeof(UniChar));
		if (m_text_storage == NULL) {
			lib::logger::get_logger()->trace("%s: out of memory", m_node->get_sig().c_str());
		}
		[the_string getCharacters: m_text_storage];
		[the_string release];
	}

	if (m_text_storage && !m_layout_manager) {
		// Only now can we set the color: the alfa comes from the region.
		double alfa = 1.0;
		const common::region_info *ri = m_dest->get_info();
		if (ri) alfa = ri->get_mediaopacity();
		ATSURGBAlphaColor acolor = {redf(m_text_color), greenf(m_text_color), bluef(m_text_color), alfa};
		tags[0] = kATSURGBAlphaColorTag;
		sizes[0] = sizeof(ATSURGBAlphaColor);
		values[0] = &acolor;
		err = ATSUSetAttributes(m_style, 1, tags, sizes, values);
		assert(err == 0);

		err = ATSUCreateTextLayout(&m_layout_manager);
		assert(err == 0);

		err = ATSUSetTextPointerLocation(m_layout_manager, m_text_storage, kATSUFromTextBeginning, kATSUToTextEnd, m_text_storage_length);
		assert(err == 0);

		err = ATSUSetRunStyle(m_layout_manager, m_style, kATSUFromTextBeginning, kATSUToTextEnd);
		assert(err == 0);
	}
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	CGRect cg_dstrect = CGRectFromAmbulantRect(dstrect);
	if (m_layout_manager) {
		// Setup context
		CGContextRef ctx = [view getCGContext];
		tags[0] = kATSUCGContextTag;
		sizes[0] = sizeof(CGContextRef);
		values[0] = &ctx;
		err = ATSUSetLayoutControls(m_layout_manager, 1, tags, sizes, values);
		assert(err == 0);

		// Find line breaks.
		Fixed flinewidth = FloatToFixed(CGRectGetWidth(cg_dstrect));
		tags[0] = kATSULineWidthTag;
		sizes[0] = sizeof(Fixed);
		values[0] = &flinewidth;
		err = ATSUSetLayoutControls(m_layout_manager, 1, tags, sizes, values);
		assert(err == 0);

		ItemCount nlines;
		err = ATSUBatchBreakLines(m_layout_manager, kATSUFromTextBeginning, m_text_storage_length, flinewidth, &nlines);
		assert(err == 0);
//		err = ATSUGetSoftLineBreaks(layout, kATSUFromTextBeginning, kATSUToTextEnd, 0, NULL, &numSoftBreaks);
//		assert(err == 0);
		UniCharArrayOffset *breaks = (UniCharArrayOffset *) malloc((nlines+1) * sizeof(UniCharArrayOffset));
		assert(breaks);
		err = ATSUGetSoftLineBreaks(m_layout_manager, kATSUFromTextBeginning, kATSUToTextEnd, nlines, breaks, NULL);
		assert(err == 0);
		breaks[nlines++] = m_text_storage_length;

		// Draw each line
		UniCharArrayOffset lbegin, lend;
		int i;
		Fixed x = CGRectGetMinX(cg_dstrect);
		Fixed y = CGRectGetMaxY(cg_dstrect);
		lbegin = 0;
		for(i=0; i<nlines; i++) {
			lend = breaks[i];
			ATSUTextMeasurement ascent, descent;
			ATSUGetLineControl(m_layout_manager, lbegin, kATSULineAscentTag, sizeof(ATSUTextMeasurement), &ascent, NULL);
			ATSUGetLineControl(m_layout_manager, lbegin, kATSULineDescentTag, sizeof(ATSUTextMeasurement), &descent, NULL);

			y -= Fix2X(ascent);
			err = ATSUDrawText(m_layout_manager, lbegin, lend-lbegin, X2Fix(x), X2Fix(y));
			assert(err == 0);
			y -= Fix2X(descent);

			lbegin = lend;
		}
		free(breaks);
	}
	m_lock.leave();
}

} // namespace cg

} // namespace gui

} //namespace ambulant
