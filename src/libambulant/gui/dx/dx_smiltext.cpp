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

#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_smiltext.h"
#include "ambulant/gui/dx/dx_transition.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"
#include "ambulant/common/factory.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/textptr.h"
#ifdef _UNICODE
#define STR_TO_TSTR(s) ambulant::lib::textptr(s).c_wstr()
#else
#define STR_TO_TSTR(s) (s)
#endif

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
using ambulant::lib::logger;

gui::dx::dx_smiltext_run::dx_smiltext_run(smil2::smiltext_run r)
:	smil2::smiltext_run(r),
		m_left(0),
		m_top(0),
		m_width(0),
		m_ascent(0),
		m_descent(0)
{        
}

gui::dx::dx_smiltext_run::~dx_smiltext_run()
{
}

gui::dx::dx_smiltext_renderer::dx_smiltext_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories* factory,
	dx_playables_context *dxplayer)
:   dx_renderer_playable(context, cookie, node, evp, dxplayer),
	m_text(0),
	m_df(factory->get_datasource_factory()),
	m_engine(smil2::smiltext_engine(node, evp, this, true)),
	m_params(m_engine.get_params())
{
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer(0x%x)", this);
}
void gui::dx::dx_smiltext_renderer::set_surface(common::surface *dest) {
	m_dest = dest;
	
	lib::rect rc = dest->get_rect();
	lib::size bounds(rc.width(), rc.height());
	net::url url = m_node->get_url("src");
	dx_window *dxwindow = static_cast<dx_window*>(m_dest->get_gui_window());
	viewport *v = dxwindow->get_viewport();
	m_text = new text_renderer(url, bounds, v); //XXX 
}

gui::dx::dx_smiltext_renderer::~dx_smiltext_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_smiltext_renderer(0x%x)", this);
}

void
gui::dx::dx_smiltext_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::start(0x%x)", this);
		
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	renderer_playable::start(t);
}

void 
gui::dx::dx_smiltext_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::stop(0x%x)", this);
	delete m_text;
	m_text = 0;
	m_dest->renderer_done(this);
	m_activated = false;
	m_dxplayer->stopped(this);
}

void
gui::dx::dx_smiltext_renderer::smiltext_changed() {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::smiltext_changed(0x%x)", this);
	dx_smiltext_runs runs;
	m_lock.enter();
	if (m_engine.is_changed()) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;
		// Always re-compute and re-render everything when new text is added.
		for (i = m_engine.begin(); i != m_engine.end(); i++) {
			AM_DBG lib::logger::get_logger()->debug("dx_smiltext_changed(): command=%d",(*i).m_command);
			// Add the new characters
			dx_smiltext_run run(*i);
			if (run.m_command == smil2::stc_break)
				;
			else if (run.m_command == smil2::stc_data) {
				AM_DBG lib::logger::get_logger()->debug("dx_smiltext_changed(): bg_col=0x%x, color=0x%x, data=%s", (*i).m_bg_color, (*i).m_color, (*i).m_data.c_str());
			} else {
				AM_DBG lib::logger::get_logger()->debug("smiltext_engine produced unknown command 0x%x", run.m_command);
			}
			runs.push_back(run);
		}
		dx_smiltext_runs_itr it; 		
		// For each word: set font and other attributes, then get text extent
		//KB TBD eliminate next loop by moving all the work in the constructor
		for (it=runs.begin(); it != runs.end(); it++) {
			if (it->m_command == smil2::stc_data) {
				it->m_data += ' '; 
				it->dx_smiltext_run_set_attr();
				it->dx_smiltext_run_get_extent();
			}
		}
		// layout line-by-line
		lib::rect r = m_dest->get_rect();
		// assure last run in runs is stc_break
		it = runs.end(); it--;
		if (it->m_command != smil2::stc_break) {
			dx_smiltext_run linebreak(*it);
			linebreak.m_command = smil2::stc_break;
			runs.push_back(linebreak);
		}
		horizontal_layout(&runs, &r);
		vertical_layout(&runs, &r);
		// render the stuff
		for (it=runs.begin(); it != runs.end(); it++) {
			dx_smiltext_run run(*it);
			AM_DBG lib::logger::get_logger()->debug("dx_smiltext_changed()@render: p(%d,%d) %s", run.m_left, run.m_top, run.m_data.c_str());
			dx_smiltext_render(&*it);
		}
        m_engine.done();
	}
	m_lock.leave();
	m_dest->need_redraw();
}

void
gui::dx::dx_smiltext_run::dx_smiltext_run_set_attr() {

	DWORD family = FF_DONTCARE | DEFAULT_PITCH;
	const char *fontname = m_font_family;
	if (m_font_family) {
		if (strcmp(m_font_family, "serif") == 0) {
			family = FF_ROMAN | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(m_font_family, "sans-serif") == 0) {
			family = FF_SWISS | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(m_font_family, "monospace") == 0) {
			family = FF_DONTCARE | FIXED_PITCH;
			fontname = NULL;
		} else if (strcmp(m_font_family, "cursive") == 0) {
			family = FF_SCRIPT | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(m_font_family, "fantasy") == 0) {
			family = FF_DECORATIVE | VARIABLE_PITCH;
			fontname = NULL;
		}
	}
	int weight;
	switch(m_font_weight) {
		default:
		case smil2::stw_normal:
			weight = FW_NORMAL;
			break;
		case smil2::stw_bold:
			weight = FW_BOLD;
			break;
	}
	DWORD italic;
	switch(m_font_style) {
		default:
		case smil2::sts_normal:
			italic = false;
			break;
		case smil2::sts_italic:
		case smil2::sts_oblique:
		case smil2::sts_reverse_oblique:
		//XXXX no oblique for Windows GDI.
		//XXXX See: http://www.codeproject.com/useritems/oblique_txt.asp
			italic = true;
			break;
	}
	m_dx_font = ::CreateFont(
			-(int)m_font_size,	// height of font
			0,					// average character width
			0,					// angle of escapement
			0,					// base-line orientation angle
			weight,				// font weight
			italic,				// italic attribute option
			0,					// underline attribute option
			0,					// strikeout attribute option
			ANSI_CHARSET,		// character set identifier
			OUT_DEFAULT_PRECIS, // output precision
			CLIP_DEFAULT_PRECIS, // clipping precision
			DEFAULT_QUALITY,	// output quality
			family,				// pitch and family
			STR_TO_TSTR(fontname));	// typeface name
}

void
gui::dx::dx_smiltext_run::dx_smiltext_run_get_extent() {
	if (m_command != smil2::stc_data) 
		return;
	HDC hdc = CreateCompatibleDC(0);//KB TBD OPT
	if (hdc== NULL || m_data.length() == 0) {
		return;
	}
	::SelectObject(hdc, m_dx_font);
	TEXTMETRIC tm;
	BOOL res = ::GetTextMetrics(hdc, &tm);
	if (res == 0)
		win_report_last_error("GetTextMetric()");
	m_ascent  = tm.tmAscent;
	m_descent = tm.tmDescent;

	lib::textptr tp(m_data.c_str(), m_data.length());
	SIZE SZ;
	res = ::GetTextExtentPoint32(hdc, tp, (int)tp.length(), &SZ);
	if (res == 0)
		win_report_last_error("GetTextExtentPoint32()");
	m_width  = SZ.cx;
//	m_height = SZ.cy;

	DeleteDC(hdc); //KB TBD OPT
}

void
gui::dx::dx_smiltext_renderer::horizontal_layout(dx_smiltext_runs* runs, lib::rect*r) {
    dx_smiltext_runs_itr it;

	// find horizontal fit in region; insert line breaks when necessary
	lib::point p(r->left(), r->top());
	AM_DBG lib::logger::get_logger()->debug("horizontal_line_layout()@init: p(%d,%d) r(%d,%d,%d,%d) %s", p.x, p.y, r->left(),r->top(), r->width(), r->height(), runs->begin()->m_data.c_str());
	for (it = runs->begin(); it != runs->end(); it++) {
		if (it->m_command == smil2::stc_data) {
			if (p.x + it->m_width > r->right()) {
				// word doesn't fit on the current line
				p.x = r->left();
				AM_DBG lib::logger::get_logger()->debug("horizontal_line_layout()@fit: p(%d,%d) %s", p.x, p.y, it->m_data.c_str());
				dx_smiltext_run linebreak(*it);
				linebreak.m_command = smil2::stc_break;
				runs->insert(it, linebreak);
			}
			it->m_left = p.x;
			p.x += it->m_width;
		} else if (it->m_command == smil2::stc_break) {
			p.x = r->left();
		}
	}
}

void
gui::dx::dx_smiltext_renderer::vertical_layout(dx_smiltext_runs* runs, lib::rect*r) {
    dx_smiltext_runs_itr it, substr_begin, substr_end;
	lib::point p(r->left(), r->top());
	int max_ascent = 0, max_descent = 0;
	// find max. ascent/desent of all words in a line and adjust the
	// top of all words in a line for the biggest word in that line
	for (substr_begin = it = runs->begin(); it != runs->end(); it++) {
		if (it->m_command == smil2::stc_data) {
			if (max_ascent < it->m_ascent)
				max_ascent = it->m_ascent;
			if (max_descent < it->m_descent)
				max_descent = it->m_descent;
		} else  if (it->m_command == smil2::stc_break) {
			// compute top of all words in the line that ends here
			dx_smiltext_runs_itr it2;
			substr_end = it;
			for (it2 = substr_begin; it2 != substr_end; it2++) {
				if (it2->m_command == smil2::stc_data) {
					it2->m_top = p.y;
					it2->m_top += (max_ascent - it2->m_ascent);
				}
			}
			p.y += (max_ascent + max_descent);
			substr_begin = it;
			substr_begin++;
			max_ascent = max_descent = 0;
		}
	}
}

void
gui::dx::dx_smiltext_renderer::dx_smiltext_render(dx_smiltext_run* run) {
	//XXX KB TBD
	if (run->m_command != smil2::stc_data)
		return;
	if (m_text == NULL)
		return;
	//m_text->set_text_font("monospace");
	if ( ! run->m_transparent)
		m_text->set_text_color(run->m_color);
	if ( ! run->m_bg_transparent)
		m_text->set_text_bgcolor(run->m_bg_color);
	int length = (int) run->m_data.length();
	m_text->set_text_data(run->m_data.c_str(), length);
	m_text->render(run->m_left, run->m_top, run->m_dx_font); 
}

void
gui::dx::dx_smiltext_renderer::user_event(const lib::point& pt, int what) {
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
}

void
gui::dx::dx_smiltext_renderer::redraw(const lib::rect& dirty, common::gui_window *window) {
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;
	
	if(!m_text || !m_text->can_play()) {
		// No bits available
		AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::redraw with no text");
		return;
	}
	lib::rect smiltext_rc = dirty;
	lib::rect reg_rc = dirty;
	
	// Translate img_reg_rc_dirty to viewport coordinates 
	lib::point pt = m_dest->get_global_topleft();
	reg_rc.translate(pt);
		
	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}
		
	// Finally blit img_rect_dirty to img_reg_rc_dirty
	v->draw(m_text->get_ddsurf(), smiltext_rc, reg_rc, true, tr);

	if (m_erase_never) m_dest->keep_as_background();
}

 

