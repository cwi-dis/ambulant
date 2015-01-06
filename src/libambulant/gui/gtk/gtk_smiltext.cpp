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

#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_smiltext.h"
#include "ambulant/gui/gtk/gtk_util.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace gtk {

extern const char gtk_smiltext_playable_tag[] = "smilText";
extern const char gtk_smiltext_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererGtk");
extern const char gtk_smiltext_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererSmilText");

common::playable_factory *
create_gtk_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererGtk"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSmilText"), true);
	return new common::single_playable_factory<
		gtk_smiltext_renderer,
		gtk_smiltext_playable_tag,
		gtk_smiltext_playable_renderer_uri,
		gtk_smiltext_playable_renderer_uri2,
		gtk_smiltext_playable_renderer_uri2>(factory, mdp);
}

gtk_smiltext_renderer::gtk_smiltext_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	gtk_renderer<renderer_playable>(context, cookie, node, evp, fp, mdp),
	m_engine(smil2::smiltext_engine(node, evp, this, false)),
	m_params(m_engine.get_params()),
	m_motion_done(false),
	m_pango_attr_list(NULL),
	m_bg_layout(NULL),
	m_bg_pango_attr_list(NULL),
	m_pango_layout(NULL),
	m_pango_context(NULL),
	m_transparent(GTK_TRANSPARENT_COLOR),
	m_alternative(GTK_ALTERNATIVE_COLOR),
	m_alpha_media(1.0),
	m_alpha_media_bg(1.0),
	m_alpha_chroma(1.0),
	m_chroma_low(0x000000),		//black
	m_chroma_high(0xFFFFFF),	//white
	m_align(smil2::sta_left),
	m_writing_mode(smil2::stw_lr_tb),// Left to Right, Top to Bottom
	m_needs_conditional_space(false),
	m_needs_conditional_newline(false),
	m_wrap(true),
	m_is_changed(false),
	m_start(lib::point(0,0)),
	m_origin(lib::point(0,0))
{
#ifdef	TBD
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append);
#endif//TBD
}

gtk_smiltext_renderer::~gtk_smiltext_renderer()
{
	m_engine.lock();
	if ( m_pango_attr_list != NULL) {
		pango_attr_list_unref( m_pango_attr_list);
		m_pango_attr_list = NULL;
	}
	if (m_pango_context != NULL) {
		g_object_unref (G_OBJECT (m_pango_context));
		m_pango_context = NULL;
	}
	if (m_pango_layout != NULL) {
		g_object_unref(G_OBJECT (m_pango_layout));
		m_pango_layout = NULL;
	}
	if ( m_bg_pango_attr_list != NULL) {
		pango_attr_list_unref(m_bg_pango_attr_list);
		m_bg_pango_attr_list = NULL;
	}
	if (m_bg_layout) {
		g_object_unref(m_bg_layout);
		m_bg_layout = NULL;
	}
	m_engine.unlock();
}

void
gtk_smiltext_renderer::start(double t)
{
	gui::gtk::gtk_renderer<common::renderer_playable>::start(t);
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	m_motion_done = false;
	renderer_playable::start(t);
	m_context->started(m_cookie);
}

void
gtk_smiltext_renderer::seek(double t)
{
	m_engine.seek(t);
	//renderer_playable::seek(t);
}

bool
gtk_smiltext_renderer::stop()
{
	m_engine.stop();
	renderer_playable::stop();
	m_context->stopped(m_cookie);
	return true;
}

void
gtk_smiltext_renderer::marker_seen(const char *name)
{
	m_context->marker_seen(m_cookie, name);
}


void
gtk_smiltext_renderer::smiltext_changed()
{
	m_engine.lock();
	if (m_engine.is_changed()) {
		m_is_changed = true;
	}
	m_engine.unlock();
	m_dest->need_redraw(); // cannot be called while locked
}

void
gtk_smiltext_renderer::_gtk_smiltext_changed()
{
AM_DBG lib::logger::get_logger()->debug("gtk_smiltext_changed(0x%x)",this);
	if ( ! m_pango_context) {
		// initialize the pango context, layout...
		m_pango_context = gdk_pango_context_get();
		PangoLanguage* language = gtk_get_default_language();
		pango_context_set_language (m_pango_context, language);
		pango_context_set_base_dir (m_pango_context, PANGO_DIRECTION_LTR);
		const common::region_info *ri = m_dest->get_info();
		if (ri) {
			m_alpha_media = ri->get_mediaopacity();
			m_alpha_media_bg = ri->get_mediabgopacity();
			m_alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance, &m_chroma_low, &m_chroma_high);
		}
#ifndef WITH_GTK_ANTI_ALIASING
		if (m_alpha_media != 1.0 || m_alpha_media_bg != 1.0 || m_alpha_chroma != 1.0) {
			cairo_font_options_t* cairo_font_options = cairo_font_options_create();
			//
			// anti-aliasing by pango/cairo is disabled
			// when blending is necessary, because this
			// sometimes results in ugly glyphs.
			//
			cairo_font_options_set_antialias (cairo_font_options, CAIRO_ANTIALIAS_NONE);
			pango_cairo_context_set_font_options (m_pango_context, cairo_font_options);
			cairo_font_options_destroy (cairo_font_options);
		}
#endif // WITH_GTK_ANTI_ALIASING
		m_writing_mode = m_engine.begin()->m_writing_mode;
		switch (m_writing_mode) {
		default:
		case smil2::stw_lr_tb:
			pango_context_set_base_dir (m_pango_context, PANGO_DIRECTION_LTR);
			break;
		case smil2::stw_rl_tb:
			pango_context_set_base_dir (m_pango_context, PANGO_DIRECTION_RTL);
			break;
		}
	}
	if ( ! m_pango_layout) {
		m_pango_layout = pango_layout_new (m_pango_context);
		pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_LEFT);
	}
	if ( ! m_pango_attr_list)
		m_pango_attr_list = pango_attr_list_new();
	if ( ! m_bg_layout	&& (m_alpha_media != 1.0  || m_alpha_media_bg != 1.0 || m_alpha_chroma != 1.0)) {
		// prepare for blending: layout is setup twice:
		// m_bg_layout has textColor as m_transparent
		// m_pango_layout has textBackGroundColor as m_transparent
		// when blending, pixels in m_transparent color are ignored
		m_bg_layout = pango_layout_new(m_pango_context);
		pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_LEFT);
		m_bg_pango_attr_list = pango_attr_list_new();
	}
	if (m_is_changed) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;
		m_is_changed = false;
		if (m_engine.is_cleared()) {
			// Completely new text. Clear our copy and render everything.
			m_text_storage = "";
			i = m_engine.begin();
			smil2::smiltext_align align = i->m_align;
			if (align != m_align) {
				switch (align) {
				default:
				case smil2::sta_left:
					pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_LEFT);
					if (m_bg_layout)
						pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_LEFT);
					break;
				case smil2::sta_center:
					pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_CENTER);
					if (m_bg_layout)
						pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_CENTER);
					break;
				case smil2::sta_right:
					pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_RIGHT);
					if (m_bg_layout)
						pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_RIGHT);
					break;
				}
				m_align = align;
			}
			m_needs_conditional_space = false;
			m_needs_conditional_newline = false;
			if ( ! i->m_wrap || m_params.m_mode == smil2::stm_crawl)
				m_wrap = false;
		} else {
			// Only additions. Don't clear and only copy the new stuff.
			i = m_engine.newbegin();
		}
		while (i != m_engine.end()) {
			// Add the new characters
			gint start_index = m_text_storage.size();
			switch (i->m_command) {
			default:
				assert(0);
				break;
			case smil2::stc_break:
				if (m_params.m_mode == smil2::stm_crawl) {
					m_text_storage += " ";
				} else	{
					m_text_storage += "\n";
				}
				m_needs_conditional_space = false;
				m_needs_conditional_newline = false;
				break;
			case smil2::stc_condbreak:
				if (m_needs_conditional_newline) {
					if (m_params.m_mode == smil2::stm_crawl) {
						m_text_storage += " ";
					} else	{
						m_text_storage += "\n\n";
					}
					m_needs_conditional_space = false;
					m_needs_conditional_newline = false;
				}
				break;
			case smil2::stc_condspace:
				if (m_needs_conditional_space) {
					m_text_storage += " ";
					m_needs_conditional_newline = true;
					m_needs_conditional_space = false;
				}
				break;
			case smil2::stc_data:
				char lastch = *(i->m_data.rbegin());
				if (lastch == '\r' || lastch == '\n' || lastch == '\f' || lastch == '\v') {
					m_needs_conditional_newline = false;
					m_needs_conditional_space = false;
				} else if (lastch == ' ' || lastch == '\t') {
					m_needs_conditional_newline = true;
					m_needs_conditional_space = false;
				} else {
					m_needs_conditional_newline = true;
					m_needs_conditional_space = true;
				}
				if (m_params.m_mode != smil2::stm_crawl) {
					m_text_storage +=  i->m_data;
					break;
				}
				/* crawling: remove embedded newlines */
				std::string newdata = i->m_data;
				std::string::size_type nl = 0;
				while (nl != std::string::npos) {
					nl = newdata.find("\n", nl);
					newdata[nl] = ' ';
				}
				m_text_storage += newdata;
				break;
			}
			// Set font attributes
			_gtk_set_font_attr(m_pango_attr_list,
				i->m_font_families[0].c_str(),
				i->m_font_style,
				i->m_font_weight,
				i->m_font_size,
				start_index,
				m_text_storage.size());
			if (m_bg_pango_attr_list) {
				_gtk_set_font_attr(m_bg_pango_attr_list,
					i->m_font_families[0].c_str(),
					i->m_font_style,
					i->m_font_weight,
					i->m_font_size,
					start_index,
					m_text_storage.size());
			}
			// text foreground/background color settings.
			if ( ! i->m_transparent) {
				// Set foreground color attribute
				color_t fg_color = i->m_color == m_transparent ? m_alternative : i->m_color;
				_gtk_set_color_attr(
					m_pango_attr_list,
					fg_color,
					pango_attr_foreground_new,
					start_index,
					m_text_storage.size());
				if (m_bg_layout) {
					_gtk_set_color_attr(
						m_bg_pango_attr_list,
						m_transparent,
						pango_attr_foreground_new,
						start_index,
						m_text_storage.size());
				}
			}
			if ( ! i->m_bg_transparent) {
				// Set background color attribute
				// Select altenative color for m_transparent
				color_t bg_color = i->m_bg_color == m_transparent ? m_alternative : i->m_bg_color;
				_gtk_set_color_attr(m_pango_attr_list,
					m_bg_layout ?
					m_transparent : bg_color,
					pango_attr_background_new,
					start_index, m_text_storage.size());
				if (m_bg_layout) {
					_gtk_set_color_attr(m_bg_pango_attr_list,
						bg_color,
						pango_attr_background_new,
						start_index,
						m_text_storage.size());
				}
			}
			// Set the foreground attributes and text
			pango_layout_set_attributes(m_pango_layout, m_pango_attr_list);
			pango_layout_set_text(m_pango_layout, m_text_storage.c_str(), -1);
			pango_layout_context_changed(m_pango_layout);
			if (m_bg_layout) {
				// Set the background attributes and text
				pango_layout_set_attributes(m_bg_layout, m_bg_pango_attr_list);
				pango_layout_set_text(m_bg_layout, m_text_storage.c_str(), -1);
				pango_layout_context_changed(m_bg_layout);
			}
			i++;
		}
		m_engine.done();
	}
	bool finished = m_engine.is_finished();
AM_DBG	lib::logger::get_logger()->debug("gtk_smiltext_changed(0x%x), m_text_storage=%s",this,m_text_storage.c_str());
	if (finished)
		m_context->stopped(m_cookie);
}

void
gtk_smiltext_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	PangoRectangle ink_rect;
	PangoRectangle log_rect;
	m_engine.lock();
	const rect &r = m_dest->get_rect();
AM_DBG logger::get_logger()->debug("gtk_smiltext_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (m_is_changed) {
		// compute the changes
		_gtk_smiltext_changed();
		m_is_changed = false;
		// get the extents of the lines
		pango_layout_set_width(m_pango_layout, m_wrap ? r.w*PANGO_SCALE : -1);
		if (m_bg_layout)
			pango_layout_set_width(m_bg_layout, m_wrap ? r.w*PANGO_SCALE : -1);
		// get extents of first line (contains space for all lines in layout)
		PangoLayoutIter* iter_p = pango_layout_get_iter(m_pango_layout);
		PangoLayoutLine* line_p = pango_layout_iter_get_line(iter_p);
		pango_layout_iter_get_layout_extents (iter_p, &ink_rect, &log_rect);
		AM_DBG { std::string line(m_text_storage, line_p->start_index, line_p->length); logger::get_logger()->debug("pango line extents %s: x=%d y=%d width=%d height=%d",line.c_str(), log_rect.x, log_rect.y, log_rect.width, log_rect.height); }
		pango_layout_iter_free(iter_p);
		m_log_rect.x = log_rect.x/PANGO_SCALE;
		m_log_rect.y = log_rect.y/PANGO_SCALE;
		m_log_rect.w = log_rect.width/PANGO_SCALE;
		m_log_rect.h = log_rect.height/PANGO_SCALE;
	}
// Compute the shifted position of what we want to draw w.r.t. the visible origin
	switch (m_params.m_mode) {
	default:
	case smil2::stm_append:
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			break;
		case smil2::stc_initial:
			break;
		case smil2::stc_final:
			break;
		case smil2::stc_both:
			break;
		}
		break;
	case smil2::stm_crawl:
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			m_motion_done = m_origin.x > (int)m_log_rect.width() + (int)r.width();
			break;
		case smil2::stc_initial:
			m_start.x = -(int)r.width();
			m_motion_done = m_origin.x > (int)m_log_rect.width() + (int) r.width();
			break;
		case smil2::stc_final:
			break;
		case smil2::stc_both:
			m_start.x = -(int)r.width();
			break;
		}
		break;
	case smil2::stm_jump:
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			break;
		case smil2::stc_initial:
			break;
		case smil2::stc_final:
			break;
		case smil2::stc_both:
			break;
		}
		break;
	case smil2::stm_scroll:
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			if (m_origin.y > (int)m_log_rect.height()  + (int)r.height())
				m_motion_done = true;
			break;
		case smil2::stc_initial:
			m_start.y = -(int)r.height();
			if (m_origin.y > (int)m_log_rect.height()  + (int)r.height())
				m_motion_done = true;
			break;
		case smil2::stc_final:
			break;
		case smil2::stc_both:
			m_start.y = -(int)r.height();
			break;
		}
		break;
	}
	if ( ! m_motion_done ) {
		if (m_params.m_rate == 0 && m_engine.is_auto_rate()) {
			// all information to compute the rate is now available
			unsigned int dur = m_engine.get_dur();
			if (dur) {
				lib::size size(m_log_rect.w, m_log_rect.h);
				unsigned int rate = smil2::smiltext_layout_engine::compute_rate(m_params, m_align, size, r, m_engine.get_dur());
				m_engine.set_rate(rate);
			}
		}
		m_origin.x = m_start.x;
		m_origin.y = m_start.y;
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		switch (m_params.m_mode) {
		default: // no smilText motion, don't come here again
			m_motion_done = true;
			break;
		case smil2::stm_crawl:
			m_origin.x += (int) now * m_params.m_rate / 1000;
			break;
		case smil2::stm_scroll:
			m_origin.y += (int) now * m_params.m_rate / 1000;
			break;
		}

	} else {
		m_context->stopped(m_cookie);
	}
	AM_DBG logger::get_logger()->debug("gtk_smiltext_renderer.redraw: logical_origin(%d,%d) log_rect(%d,%d) r(%d,%d)", m_origin.x, m_origin.y, m_log_rect.w, m_log_rect.h, r.w, r.h);

	_gtk_smiltext_render(r, m_origin,(ambulant_gtk_window*)window);
	m_engine.unlock();
}

// private methods
void
gtk_smiltext_renderer::_gtk_set_font_attr (PangoAttrList* pal,
	const char* smiltext_font_family,
	smil2::smiltext_font_style smiltext_font_style,
	smil2::smiltext_font_weight smiltext_font_weight,
	int smiltext_font_size,
	unsigned int start_index, unsigned int end_index)
{
	PangoFontDescription* pango_font_description = pango_font_description_new();

	// smiltext font style to pango font style conversion
	PangoStyle pango_font_style;
	switch (smiltext_font_style) {
	default:
	case smil2::sts_normal:
	case smil2::sts_reverse_oblique:	// Not supported by pango
		pango_font_style = PANGO_STYLE_NORMAL;
		break;
	case smil2::sts_italic:
		pango_font_style = PANGO_STYLE_ITALIC;
		break;
	case smil2::sts_oblique:
		pango_font_style = PANGO_STYLE_OBLIQUE;
		break;
	}
	pango_font_description_set_style(pango_font_description, pango_font_style);
	// smiltext font weight to pango font weight conversion
	PangoWeight pango_font_weight;
	switch (smiltext_font_weight) {
	default:
	case smil2::stw_normal:
		pango_font_weight = PANGO_WEIGHT_NORMAL;
		break;
	case smil2::stw_bold:
		pango_font_weight = PANGO_WEIGHT_BOLD;
		break;
	}
	pango_font_description_set_weight(pango_font_description, pango_font_weight);
	// smiltext font size to pango font size conversion
	double pango_font_size = smiltext_font_size*PANGO_SCALE;
	pango_font_description_set_absolute_size(pango_font_description, pango_font_size);
	// smiltext font family to pango font family conversion
	const char* pango_font_family = smiltext_font_family;
	pango_font_description_set_family(pango_font_description, pango_font_family);
	PangoAttribute* pango_attribute = pango_attr_font_desc_new(pango_font_description);
	// add the new PangoAttribute to the PangoAttrList for the range given
	pango_attribute->start_index = start_index;
	pango_attribute->end_index	 = end_index;
	pango_attr_list_insert(pal, pango_attribute);

	pango_font_description_free(pango_font_description);
}

void
gtk_smiltext_renderer::_gtk_set_color_attr(PangoAttrList* pal,
	lib::color_t smiltext_color,
	PangoAttribute* (*pango_attr_color_new)(guint16 r, guint16 g, guint16 b),
	unsigned int start_index, unsigned int end_index)
{
	// smiltext color to pango color conversion
	guint16 pango_red = redc(smiltext_color)*0x101;
	guint16 pango_green = greenc(smiltext_color)*0x101;
	guint16 pango_blue = bluec(smiltext_color)*0x101;
	// Create new PangoAttribute containing the specified color
	PangoAttribute* pango_attribute = pango_attr_color_new(pango_red, pango_green, pango_blue);
	// add the new PangoAttribute to the PangoAttrList for the range given
	pango_attribute->start_index = start_index;
	pango_attribute->end_index = end_index;
	pango_attr_list_insert(pal, pango_attribute);
}

#if GTK_MAJOR_VERSION >= 3
void
gtk_smiltext_renderer::_gtk_smiltext_render(
	const lib::rect r, 
	const lib::point offset,
	ambulant_gtk_window* window)
{
	// Determine current position and size.
	const lib::point p = m_dest->get_global_topleft();
	const char* data = m_text_storage.c_str();

	AM_DBG lib::logger::get_logger()->debug("gtk_smiltext_render(0x%x): ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):offsetp=(%d,%d):",(void *)this,r.left(),r.top(),r.width(),r.height(),data==NULL?"(null)":data,p.x,p.y,offset.x,offset.y);
	if ( ! (m_pango_layout && window))
		return; // nothing to do

	int L = r.left()+p.x,
		T = r.top()+p.y,
		W = r.width(),
		H = r.height();

	// include the text
	pango_layout_set_width(m_pango_layout, m_wrap ? W*PANGO_SCALE : -1);
	if (m_bg_layout) { //XXX TBD
		g_object_unref(m_bg_layout);
		m_bg_layout = NULL;
	}

	cairo_t *cr = cairo_create (window->get_target_surface());
	/* clip */
	cairo_rectangle (cr, L, T, W, H);
	cairo_clip (cr);
	/* set the correct source color */
	/* draw the text */
	cairo_move_to (cr, L-offset.x, T-offset.y);
	pango_cairo_show_layout (cr, m_pango_layout);
	cairo_destroy (cr);
}
#else // GTK_MAJOR_VERSION < 3
void
gtk_smiltext_renderer::_gtk_smiltext_render(
	const lib::rect r, 
	const lib::point offset,
	ambulant_gtk_window* window)
{
	// Determine current position and size.
	const lib::point p = m_dest->get_global_topleft();
	const char* data = m_text_storage.c_str();

	AM_DBG lib::logger::get_logger()->debug("gtk_smiltext_render(0x%x): ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):offsetp=(%d,%d):",(void *)this,r.left(),r.top(),r.width(),r.height(),data==NULL?"(null)":data,p.x,p.y,offset.x,offset.y);
	if ( ! (m_pango_layout && window))
		return; // nothing to do
	int L = r.left()+p.x,
		T = r.top()+p.y,
		W = r.width(),
		H = r.height();
	GdkRectangle gdk_rectangle;
	gdk_rectangle.x = L;
	gdk_rectangle.y = T;
	gdk_rectangle.width = W;
	gdk_rectangle.height = H;

	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (window->get_ambulant_pixmap()));
	gdk_gc_set_clip_rectangle(gc, &gdk_rectangle);
	// include the text
	pango_layout_set_width(m_pango_layout, m_wrap ? W*PANGO_SCALE : -1);

	if (m_bg_layout) {
		// blending
		pango_layout_set_width(m_bg_layout, m_wrap ? W*PANGO_SCALE : -1);

		GdkPixmap* text_pixmap = gdk_pixmap_new((window->get_ambulant_pixmap()),W,H,-1);
		GdkPixmap* bg_pixmap = gdk_pixmap_new((window->get_ambulant_pixmap()),W,H,-1);
		GdkGC* text_gc = gdk_gc_new (GDK_DRAWABLE (text_pixmap));
		GdkGC* bg_gc =	gdk_gc_new (GDK_DRAWABLE (bg_pixmap));
		gdk_rectangle.x = gdk_rectangle.y = 0;
		gdk_gc_set_clip_rectangle(text_gc, &gdk_rectangle);
		gdk_gc_set_clip_rectangle(bg_gc, &gdk_rectangle);
		GdkPixmap* pixmap = window->get_ambulant_pixmap();

		int PW = -1, PH = -1;

		if (pixmap != NULL)
			gdk_drawable_get_size (pixmap, &PW, &PH);
		if (pixmap == NULL || PW < L+W || PH  < T+H ) {
			g_object_unref (G_OBJECT (text_pixmap));
			g_object_unref (G_OBJECT (bg_pixmap));
			g_object_unref (G_OBJECT (text_gc));
			g_object_unref (G_OBJECT (bg_gc));
			g_object_unref (G_OBJECT (gc));
			lib::logger::get_logger()->trace("smilText: gdk_pixbuf_get_from_drawable failed, pixmap.size()=(%d,%d), (L,T,W,H)=(%d,%d,%d,%d)", PW,PH,L,T,W,H);
			lib::logger::get_logger()->error(gettext("Geometry error in smil document at %s"), m_node->get_sig().c_str());
			return;
		}
		GdkPixbuf* screen_pixbuf = gdk_pixbuf_get_from_drawable (NULL, pixmap, NULL, L, T, 0, 0, W, H);
		GdkColor gdk_transparent;
		gdk_transparent.red = redc(m_transparent)*0x101;
		gdk_transparent.blue = bluec(m_transparent)*0x101;
		gdk_transparent.green = greenc(m_transparent)*0x101;
		gdk_gc_set_rgb_bg_color (bg_gc, &gdk_transparent);
		gdk_gc_set_rgb_fg_color (bg_gc, &gdk_transparent);
		gdk_gc_set_rgb_bg_color (text_gc, &gdk_transparent);
		gdk_gc_set_rgb_fg_color (text_gc, &gdk_transparent);

		// clear text_pixmap and bg_pixmap to transparent color
		gdk_draw_rectangle (bg_pixmap, bg_gc, TRUE, 0, 0, W, H);
		gdk_draw_rectangle (text_pixmap, text_gc, TRUE, 0, 0, W, H);

		// draw m_bg_layout containing smilText runs with text in
		// m_transparent color and background in required color
		gdk_draw_layout(GDK_DRAWABLE (bg_pixmap), bg_gc , 0-offset.x, 0-offset.y, m_bg_layout);
		g_object_unref (G_OBJECT (bg_gc));
//DBG	gdk_pixmap_dump(bg_pixmap, "bg");
		GdkPixbuf* bg_pixbuf = gdk_pixbuf_get_from_drawable(
			NULL,
			bg_pixmap,
			NULL,
			0,0,0,0,W,H);
		lib::rect rc(lib::point(L,T),lib::size(W,H));
		// blend the screen pixbuf with th background pixbuf
		gdk_pixbuf_blend (
			screen_pixbuf,
			rc, bg_pixbuf,
			rc,
			m_alpha_chroma,
			m_alpha_media_bg,
			m_chroma_low,
			m_chroma_high,
			m_transparent);
//DBG	gdk_pixmap_dump( window->get_ambulant_pixmap(), "screen0");

		// draw m_pango_layout containing smilText runs with text in
		// required colors and background in m_transparant color
		gdk_draw_layout(GDK_DRAWABLE (text_pixmap), text_gc , 0-offset.x, 0-offset.y, m_pango_layout);
		g_object_unref (G_OBJECT (text_gc));
//DBG	gdk_pixmap_dump(text_pixmap, "text");
		GdkPixbuf* text_pixbuf = gdk_pixbuf_get_from_drawable(
			NULL,
			text_pixmap,
			NULL,
			0,0,0,0,W,H);
		gdk_pixbuf_blend (
			screen_pixbuf,
			rc,
			text_pixbuf,
			rc,
			m_alpha_chroma,
			m_alpha_media,
			m_chroma_low,
			m_chroma_high,
			m_transparent);
		// draw the blended pixbuf on the screen
		gdk_draw_pixbuf(
			window->get_ambulant_pixmap(),
			gc,
			screen_pixbuf,
			0, 0,
			L, T, W, H,
			GDK_RGB_DITHER_NONE,
			0, 0);
//DBG	gdk_pixmap_dump( window->get_ambulant_pixmap(), "screen1");
		g_object_unref (G_OBJECT (text_pixbuf));
		g_object_unref (G_OBJECT (bg_pixbuf));
		g_object_unref (G_OBJECT (screen_pixbuf));
		g_object_unref (G_OBJECT (bg_pixmap));
		g_object_unref (G_OBJECT (text_pixmap));
	} else {
		gdk_draw_layout(
			GDK_DRAWABLE (window->get_ambulant_pixmap()),
			gc,
			L-offset.x,
			T-offset.y,
			m_pango_layout);
	}
	g_object_unref (G_OBJECT (gc));
}
#endif // GTK_MAJOR_VERSION < 3

} // namespace gtk

} // namespace gui

} //namespace ambulant
