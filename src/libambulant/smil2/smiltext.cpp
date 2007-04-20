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

#include "ambulant/smil2/smiltext.h"
#include "ambulant/lib/callback.h"

#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef WITH_SMIL30

namespace ambulant {
namespace smil2 {

typedef lib::no_arg_callback<smiltext_engine> update_callback;

smiltext_engine::smiltext_engine(const lib::node *n, lib::event_processor *ep, smiltext_notification *client, bool word_mode)
:	m_node(n),
	m_tree_iterator(n->begin()),
	m_event_processor(ep),
	m_client(client),
	m_word_mode(word_mode),
	m_newbegin_valid(false),
	m_update_event(NULL)
{
	lib::logger::get_logger()->debug("smiltext_engine(0x%x).smiltext_engine(%s)", this, m_node->get_sig().c_str());
	// Initialize the iterators to the correct place
	m_tree_iterator++;
	m_newbegin = m_runs.end();
	m_newbegin_valid = false;
	
	// Initialize the global para
	// Initialize the default formatting and apply node attributes
	smiltext_run stdrun;
	stdrun.m_command = (smiltext_command)0;//6228874;
	_get_default_formatting(stdrun);
	_get_default_params(m_params);
	const char *rgn = n->get_attribute("region");
	if (rgn) {
		const lib::node *rgn_node = n->get_context()->get_node(rgn);
		if (rgn_node) {
			_get_formatting(stdrun, rgn_node);
			_get_params(m_params, rgn_node);
		}
	}
	_get_formatting(stdrun, n);
	_get_params(m_params, n);
	m_run_stack.push(stdrun);
}

smiltext_engine::~smiltext_engine()
{
	if (m_update_event&&m_event_processor)
		m_event_processor->cancel_event(m_update_event, lib::ep_med);
//	delete m_update_event;
	m_update_event = NULL;
	m_client = NULL;
	m_node = NULL;
}

/// Start the engine.
void
smiltext_engine::start(double t) {
	// XXX Need to allow for "t"
	lib::logger::get_logger()->debug("smiltext_engine(0x%x).start(%s)", this, m_node->get_sig().c_str());
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_tree_time = 0;
	_update();
}

/// Seek the engine
void
smiltext_engine::seek(double t) {
	// To be provided
}

/// Stop the engine.
void
smiltext_engine::stop() {
	if (m_update_event&&m_event_processor)
		m_event_processor->cancel_event(m_update_event, lib::ep_med);
//	delete m_update_event;
	m_update_event = NULL;
	m_tree_iterator = m_node->end();
	m_runs.clear();
	m_newbegin = m_runs.end();
	m_newbegin_valid = false;
	m_node = NULL;
}

void
smiltext_engine::_split_into_words(lib::xml_string data) {
	size_t first_nonblank;
	std::string blanks(" \f\n\r\t\v");
	while ((first_nonblank = data.find_first_not_of(blanks)) != std::string::npos) {
		size_t first_blank = data.find_first_of(blanks, first_nonblank);
		if (first_blank == std::string::npos)
			first_blank = data.length();
		if (first_blank > 0) {
			smiltext_run run = m_run_stack.top();
			run.m_command = stc_data;
			run.m_data = data.substr(first_nonblank, first_blank-first_nonblank);
			smiltext_runs::const_iterator where = m_runs.insert( m_runs.end(), run);
			if (!m_newbegin_valid) {
				m_newbegin = where;
				m_newbegin_valid = true;
			}
//			m_runs.push_back(run);
			data = data.substr(first_blank);
		} else {
			data = data.substr(data.length());
		}
	}
}

void
smiltext_engine::_update() {
	assert(m_node);
	lib::timer::time_type next_update_needed = 0;
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine::_update()");
	m_update_event = NULL;
	for( ; !m_tree_iterator.is_end(); m_tree_iterator++) {
		assert(!m_run_stack.empty());
		const lib::node *item = (*m_tree_iterator).second;
		if (!(*m_tree_iterator).first) {
			// Pop the stack, if needed
			const lib::xml_string &tag = item->get_local_name();
			if (tag == "span" || tag == "pre")
				m_run_stack.pop();
			continue;
		}
		
		if (item->is_data_node()) {
			// Characters. Add them to the run with current font/size/etc
			smiltext_run run = m_run_stack.top();
			// Trim all space characters. BUT if there is whitespace at the
			// end leave one space there.
			lib::xml_string data = item->get_data();
			// XXX <pre>!
			if (m_word_mode) {
				_split_into_words(data);
				continue;
			}
			size_t first_nonblank = data.find_first_not_of(" \t\r\n");
			size_t last_nonblank = data.find_last_not_of(" \t\r\n");
			bool space_at_end = last_nonblank < data.size()-1;
			if (first_nonblank != std::string::npos && last_nonblank != std::string::npos) {
				run.m_data = data.substr(first_nonblank, last_nonblank-first_nonblank+1);
				if (space_at_end) run.m_data += ' ';
				if (run.m_data != "") {
					smiltext_runs::const_iterator where = m_runs.insert(m_runs.end(), run);
					if (!m_newbegin_valid) {
						m_newbegin = where;
						m_newbegin_valid = true;
					}
				}
			}
		} else {
			// Element. Check what it is.
			lib::xml_string tag = item->get_local_name();
			if (tag == "tev" || tag == "clear") {
				const char *time_str = item->get_attribute("next");
				if (!time_str) {
					lib::logger::get_logger()->trace("smiltext: tev without next attribute ignored");
					continue;
				}
				double time = atof(time_str); // XXXJACK
				if (time_str[0] == '+')
					m_tree_time = m_tree_time + time;
				else
					m_tree_time = time;
				lib::timer::time_type ttime = m_epoch + int(time*1000);
				lib::timer::time_type now = m_event_processor->get_timer()->elapsed();
				if (ttime > now) {
					next_update_needed = ttime-now;
					break;
				}
				// else this time has already passed and we continue the loop
				if (tag == "clear") {
					m_runs.clear();
					m_newbegin = m_runs.end();
					m_newbegin_valid = false;
				}
			} else if (tag == "span") {
				smiltext_run run = m_run_stack.top();
				_get_formatting(run, item);
				m_run_stack.push(run);
			} else if (tag == "br") {
				smiltext_run run = m_run_stack.top();
				run.m_data = "";
				run.m_command = stc_break;
				smiltext_runs::const_iterator where = m_runs.insert( m_runs.end(), run);
				if (!m_newbegin_valid) {
					m_newbegin = where;
					m_newbegin_valid = true;
				}
			} else {
				lib::logger::get_logger()->trace("smiltext: unknown tag <%s>", tag.c_str());
			}
		}
	}
	if (m_client)
		m_client->smiltext_changed();
	if (m_params.m_rate > 0 && m_update_event == NULL) {
		// We need to schedule another update event to keep the scrolling/crawling going.
		// In principle we do a callback per pixel scrolled, but clamp at 25 per second.
		unsigned int delay = 1000 / m_params.m_rate;
		if (delay < 40) delay = 40;
		if (next_update_needed > delay || next_update_needed == 0) {
			next_update_needed = delay;
		}
	}
	if (next_update_needed > 0) {
		m_update_event = new update_callback(this, &smiltext_engine::_update);
		m_event_processor->add_event(m_update_event, next_update_needed, lib::ep_med);
	}
}

// Fill a run with the formatting parameters from a node.
void
smiltext_engine::_get_formatting(smiltext_run& dst, const lib::node *src)
{
	const char *style = src->get_attribute("textStyle");
	if (style) {
		const lib::node_context *ctx = src->get_context();
		assert(ctx);
		const lib::node *stylenode = ctx->get_node(style);
		if (stylenode) {
			// XXX check that stylenode.tag == textStyle
			_get_formatting(dst, stylenode);
		} else {
			lib::logger::get_logger()->trace("%s: textStyle=\"%s\": ID not found", src->get_sig().c_str(), style);
		}
	}
	const char *align = src->get_attribute("textAlign");
	if (align) {
		if (strcmp(align, "start") == 0) dst.m_align = sta_start;
		else if (strcmp(align, "end") == 0) dst.m_align = sta_end;
		else if (strcmp(align, "left") == 0) dst.m_align = sta_left;
		else if (strcmp(align, "right") == 0) dst.m_align = sta_right;
		else if (strcmp(align, "center") == 0) dst.m_align = sta_center;
		else if (strcmp(align, "inherit") == 0) /* no-op */;
		else {
			lib::logger::get_logger()->trace("%s: textAlign=\"%s\": unknown alignment", src->get_sig().c_str(), align);
		}
	}
	const char *bg_color = src->get_attribute("textBackgroundColor");
	if (bg_color) {
		dst.m_bg_transparent = false;
		dst.m_bg_color = lib::to_color(bg_color);
	}
	const char *color = src->get_attribute("textColor");
	if (color) {
		dst.m_transparent = false;
		dst.m_color = lib::to_color(color);
	}
	const char *direction = src->get_attribute("textDirection");
	if (direction) {
		if (strcmp(direction, "ltr") == 0) dst.m_direction = std_ltr;
		else if (strcmp(direction, "rtl") == 0) dst.m_direction = std_rtl;
		else if (strcmp(direction, "inherit") == 0) /* no-op */;
		else {
			lib::logger::get_logger()->trace("%s: textDirection=\"%s\": unknown direction", src->get_sig().c_str(), direction);
		}
		
	}
	const char *font_family = src->get_attribute("textFontFamily");
	if (font_family) {
		dst.m_font_family = font_family;
	}
	const char *font_size = src->get_attribute("textFontSize");
	if (font_size) {
		if (strcmp(font_size, "xx-small") == 0) dst.m_font_size = 8;
		else if (strcmp(font_size, "x-small") == 0) dst.m_font_size = 10;
		else if (strcmp(font_size, "small") == 0) dst.m_font_size = 12;
		else if (strcmp(font_size, "normal") == 0) dst.m_font_size = 14;
		else if (strcmp(font_size, "large") == 0) dst.m_font_size = 16;
		else if (strcmp(font_size, "x-large") == 0) dst.m_font_size = 18;
		else if (strcmp(font_size, "xx-large") == 0) dst.m_font_size = 20;
		else if (strcmp(font_size, "smaller") == 0) dst.m_font_size -= 2;
		else if (strcmp(font_size, "larger") == 0) dst.m_font_size += 2;
		else if (strcmp(font_size, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textFontSize=\"%s\": unknown size", src->get_sig().c_str(), font_size);
		}
	}
	const char *font_weight = src->get_attribute("textFontWeight");
	if (font_weight) {
		if (strcmp(font_weight, "normal") == 0) dst.m_font_weight = stw_normal;
		else if (strcmp(font_weight, "bold") == 0) dst.m_font_weight = stw_bold;
		else if (strcmp(font_weight, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textFontWeight=\"%s\": unknown weight", src->get_sig().c_str(), font_weight);
		}
	}
	const char *font_style = src->get_attribute("textFontStyle");
	if (font_style) {
		if (strcmp(font_style, "normal") == 0) dst.m_font_style = sts_normal;
		else if (strcmp(font_style, "italic") == 0) dst.m_font_style = sts_italic;
		else if (strcmp(font_style, "oblique") == 0) dst.m_font_style = sts_oblique;
		else if (strcmp(font_style, "reverseOblique") == 0) dst.m_font_style = sts_reverse_oblique;
		else if (strcmp(font_style, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textFontStyle=\"%s\": unknown style", src->get_sig().c_str(), font_style);
		}
	}
}

// Fill a run with the default formatting.
void
smiltext_engine::_get_default_formatting(smiltext_run& dst)
{
	dst.m_font_family = "monospace";
	dst.m_font_style = sts_normal;
	dst.m_font_weight = stw_normal;
	dst.m_font_size = 12;
	dst.m_transparent = false;
	dst.m_color = lib::color_t(0);
	dst.m_bg_transparent = true;
	dst.m_bg_color = lib::color_t(0);
	dst.m_align = sta_start;
	dst.m_direction = std_ltr;
}

// Fill smiltext_params from a node
void
smiltext_engine::_get_params(smiltext_params& params, const lib::node *src)
{
	const char *mode = src->get_attribute("textMode");
	if (mode) {
		if (strcmp(mode, "replace") == 0) params.m_mode = stm_replace;
		else if (strcmp(mode, "append") == 0) params.m_mode = stm_append;
		else if (strcmp(mode, "scroll") == 0) params.m_mode = stm_scroll;
		else if (strcmp(mode, "crawl") == 0) params.m_mode = stm_crawl;
		else if (strcmp(mode, "jump") == 0) params.m_mode = stm_jump;
		else if (strcmp(mode, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textMode=\"%s\": unknown mode", src->get_sig().c_str(), mode);
		}
	}
	const char *loop = src->get_attribute("textLoop");
	if (loop) {
		if (strcmp(loop, "true") == 0) params.m_loop = true;
		else if (strcmp(loop, "false") == 0) params.m_loop = false;
		else if (strcmp(loop, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textLoop=\"%s\": must be true or false", src->get_sig().c_str(), loop);
		}
	}
	const char *rate = src->get_attribute("textRate");
	if (rate) {
		int rate_i = atoi(rate); // XXXX
		params.m_rate = rate_i;
	}
	const char *wrap = src->get_attribute("textWrapOption");
	if (wrap) {
		if (strcmp(wrap, "wrap") == 0) params.m_wrap = true;
		else if (strcmp(wrap, "noWrap") == 0) params.m_wrap = false;
		else if (strcmp(wrap, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textWrapOption=\"%s\": must be wrap or noWrap", src->get_sig().c_str(), wrap);
		}
	}
}

// Fill default smiltext_params
void
smiltext_engine::_get_default_params(smiltext_params& params)
{
	params.m_mode = stm_append;
	params.m_loop = false;
	params.m_rate = 0;
	params.m_wrap = true;
}


}
}
#endif // WITH_SMIL30