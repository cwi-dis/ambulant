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

#include "ambulant/smil2/smiltext.h"
#include "ambulant/smil2/time_attrs.h"
#include "ambulant/lib/callback.h"
#include <math.h>
#define round(x) ((int)((x)+0.5))

//#define	AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

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
	m_process_lf(true),
	m_auto_rate(false),
	m_update_event(NULL),
	m_is_locked(false)
{
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine(0x%x).smiltext_engine(%s)", this, m_node->get_sig().c_str());
	// Initialize the iterators to the correct place
	m_this = this;
	m_tree_iterator++;
	m_newbegin = m_runs.end();
	m_newbegin_valid = false;

	// Initialize the global para
	// Initialize the default formatting and apply node attributes
	smiltext_run stdrun;
	stdrun.m_command = stc_data;
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
	if ((m_params.m_mode == stm_crawl || m_params.m_mode == stm_scroll) && m_params.m_rate == 0)
		// textRate="auto", must compute rate
		m_auto_rate = true;
	m_run_stack.push(stdrun);
}

smiltext_engine::~smiltext_engine()
{
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine::~smiltext_engine(0x%x)",this);
	m_lock.enter();
#ifndef	WITH_GCD_EVENT_PROCESSOR
	if (m_update_event&&m_event_processor)
		m_event_processor->cancel_event(m_update_event, lib::ep_med);
#endif//WITH_GCD_EVENT_PROCESSOR
//	delete m_update_event;
	m_update_event = NULL;
	m_client = NULL;
	m_node = NULL;
	m_this = NULL;
	m_lock.leave();
}

/// Start the engine.
void
smiltext_engine::start(double t) {
	// XXX Need to allow for "t"
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine(0x%x).start(%s, %f)", this, m_node->get_sig().c_str(), t);
	m_epoch = m_event_processor->get_timer()->elapsed() - lib::timer::time_type(t*1000);
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
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine::stop(0x%x)",this);
	lock();
#ifndef	WITH_GCD_EVENT_PROCESSOR
	if (m_update_event&&m_event_processor)
		m_event_processor->cancel_event(m_update_event, lib::ep_med);
#endif//WITH_GCD_EVENT_PROCESSOR
//	delete m_update_event;
	m_update_event = NULL;
	m_tree_iterator = m_node->end();
	m_runs.clear();
	m_newbegin = m_runs.end();
	m_newbegin_valid = false;
	unlock();
	m_node = NULL;
}

lib::xml_string
smiltext_engine::_split_into_lines(lib::xml_string data, size_t lf_pos, size_t limit) {
	if (lf_pos > 0) {
		smiltext_run run = m_run_stack.top();
		run.m_command = stc_data;
		run.m_data = data.substr(0, lf_pos);
		_insert_run_at_end(run);
	}
	while (lf_pos++ < limit) {
		smiltext_run run = m_run_stack.top();
		run.m_command = stc_break;
		_insert_run_at_end(run);
		if (data[lf_pos] != '\n') {
			break;
		}
	}
	return data.substr(lf_pos);
}

void
smiltext_engine::_split_into_words(lib::xml_string data, smil2::smiltext_xml_space xml_space) {
	// same semantics as isspace, used by tree_builder
	std::string spacechar(" \f\n\r\t\v");
	size_t first_nonspace, first_char;
	while (data.length() > 0) {
		// find non-space after any leading space
		first_nonspace = data.find_first_not_of(spacechar);
		if (first_nonspace == std::string::npos)
			first_nonspace = data.length();
		// skip leading space if applicable
		first_char = xml_space == stx_preserve ? 0 : first_nonspace;
		// find trailing space
		size_t first_trailing_space = data.find_first_of(spacechar, first_nonspace);
		if (first_trailing_space == std::string::npos)
			first_trailing_space = data.length();
		if ((first_trailing_space - first_char) > 0) {
			size_t	lf_pos;
			if (m_process_lf && xml_space == stx_preserve
				&& (lf_pos = data.find('\n')) != std::string::npos
				&& lf_pos <	 first_nonspace) {
				// found line-feed characters in leading space
				data = _split_into_lines(data, lf_pos, first_nonspace);
				continue;
			}
			smiltext_run run = m_run_stack.top();
			smiltext_run pre_space = m_run_stack.top();
			smiltext_run post_space = m_run_stack.top();
			pre_space.m_command = stc_condspace;
			post_space.m_command = stc_condspace;
			pre_space.m_data = " ";
			post_space.m_data = " ";
			run.m_command = stc_data;
			run.m_data = data.substr(first_char, first_trailing_space-first_char);
			AM_DBG lib::logger::get_logger()->debug("smiltext_engine::_split_into_words(): bg_col=0x%x, color=0x%x, data=%s", run.m_bg_color, run.m_color, run.m_data.c_str());
			// add a conditional space before and after the word,
			// when it was there originally
			if (first_char > 0)
				_insert_run_at_end(pre_space);
			_insert_run_at_end(run);
			if (first_trailing_space != data.length())
				_insert_run_at_end(post_space);
			data = data.substr(first_trailing_space);
		} else {
			data = data.substr(data.length());
		}
	}
}

void
smiltext_engine::_update() {
	if (m_this != this)
		return; // object has been released
	lock();
	if (m_this != this || m_node == NULL || m_run_stack.empty()) {
		unlock();
		return;
	}
	assert(m_node);
	lib::timer::time_type next_update_needed = 0;
	AM_DBG lib::logger::get_logger()->debug("smiltext_engine::_update(0x%x)",this);
	m_update_event = NULL;
	for( ; !m_tree_iterator.is_end(); m_tree_iterator++) {
		assert(!m_run_stack.empty());
		const lib::node *item = (*m_tree_iterator).second;

		if (!(*m_tree_iterator).first) {
			// Pop the stack, if needed
			bool stack_popped = false;
			const lib::xml_string &tag = item->get_local_name();
			if ( tag == "span") {
				m_run_stack.pop();
				stack_popped = true;
			} else if (tag == "div" || tag == "p") {
				smiltext_run run = m_run_stack.top();
				// insert conditional line break
				run.m_data = "";
				run.m_command = stc_condbreak;
				_insert_run_at_end(run);

				m_run_stack.pop();
				stack_popped = true;
			}
			if (stack_popped && m_word_mode) {
				smil2::smiltext_runs::reverse_iterator rlast = m_runs.rbegin();
				if ((*rlast).m_command == stc_condspace) {
					// KB From: http://ubuntuforums.org/archive/index.php/t-330552.html
					// m_runs.erase(last.base*()); // dumps core

					smil2::smiltext_runs::iterator last = rlast.base();
					last--;
					m_runs.erase(last);
				}
			}
			continue;
		}

		if (item->is_data_node()) {
			// Characters. Add them to the run with current font/size/etc
			smiltext_run run = m_run_stack.top();
			lib::xml_string data = item->get_data();
			// XXX <pre>!
			if (m_word_mode) {
				_split_into_words(data, run.m_xml_space);
				continue;
			}
			if (run.m_xml_space == stx_preserve) {
				run.m_command = stc_data;
				run.m_data = data;
				_insert_run_at_end(run);
			} else {
				size_t first_nonblank = data.find_first_not_of(" \t\r\n\f\v");
				size_t last_nonblank = data.find_last_not_of(" \t\r\n\f\v");
				size_t last_position = data.size();
				if (first_nonblank != 0 && first_nonblank != std::string::npos) {
					// String starts with whitespace. Add a conditional space.
					run.m_command = stc_condspace;
					run.m_data = "";
					_insert_run_at_end(run);
				}
				// Strip the blanks from the string and add it to the output.
				if (first_nonblank != std::string::npos && last_nonblank != std::string::npos)
					data = data.substr(first_nonblank, last_nonblank-first_nonblank+1);
				run.m_command = stc_data;
				run.m_data = data;
				_insert_run_at_end(run);

				if (last_nonblank != last_position-1) {
					// String ends with whitespace. Add a conditional space.
					run.m_command = stc_condspace;
					run.m_data = "";
					_insert_run_at_end(run);
				}
			}
		} else {
			// Element. Check what it is.
			lib::xml_string tag = item->get_local_name();
			if (tag == "tev" || tag == "clear") {
				if (!m_auto_rate) { // Ignore tev and clear for autoscroll/autocrawl
					const char *time_str = item->get_attribute("begin");
					double time = 0;
					if (time_str) {
						time_attr_parser tp(item, "begin", lib::logger::get_logger());
						sync_value_struct svs;
						svs.type = sv_indefinite;
						svs.offset = 0;
						svs.iparam = 0;
						if (tp.parse_sync(time_str, svs)) {
							AM_DBG lib::logger::get_logger()->debug("smilText: %s: begin=\"%s\"", item->get_sig().c_str(), repr(svs).c_str());
							if (svs.type == sv_offset) {
								time = svs.offset / 1000.0;
							} else if (svs.type == sv_event) {
								lib::logger::get_logger()->trace("smilText: %s: events not yet implemented", item->get_sig().c_str());
							} else {
								lib::logger::get_logger()->trace("smilText: %s: begin=\"%s\" not allowed", item->get_sig().c_str(), repr(svs).c_str());
								lib::logger::get_logger()->error(gettext("Error in smilText timing"));
							}
						}
					} else if ((time_str = item->get_attribute("next"))) {
						time_attr_parser tp(item, "next", lib::logger::get_logger());
						sync_value_struct svs;
						svs.type = sv_indefinite;
						svs.offset = 0;
						svs.iparam = 0;
						if (tp.parse_sync(time_str, svs)) {
							AM_DBG lib::logger::get_logger()->debug("%s: next=\"%s\"", item->get_sig().c_str(), repr(svs).c_str());
							if (svs.type == sv_offset) {
								time = svs.offset / 1000.0;
							} else {
								lib::logger::get_logger()->trace("smilText: %s: next=\"%s\" not allowed", item->get_sig().c_str(), repr(svs).c_str());
								lib::logger::get_logger()->error(gettext("Error in smilText timing"));
							}
						}
						time = m_tree_time + time;
					} else	{
						lib::logger::get_logger()->trace("smiltext: tev without begin or next attribute ignored");
						continue;
					}
					lib::timer::time_type ttime = m_epoch + round(time*1000);
					lib::timer::time_type now = m_event_processor->get_timer()->elapsed();
					// If this node is still in the future we stop here and schedule the next update
					// Otherwise, we continue processing.
					if (ttime > now) {
						next_update_needed = ttime-now;
						break;
					}
					m_tree_time = time;
					if (tag == "clear" || m_params.m_mode == stm_replace) {
						m_runs.clear();
						m_newbegin = m_runs.end();
						m_newbegin_valid = false;
					}
				}
			} else if (tag == "br" ) {
				if (m_params.m_mode != stm_crawl) {
					// insert line break
					smiltext_run run = m_run_stack.top();
					run.m_data = "";
					run.m_command = stc_break;
					_insert_run_at_end(run);
				}
			} else if ( tag == "span" ) {
				smiltext_run run = m_run_stack.top();
				_get_formatting(run, item);
				m_run_stack.push(run);
			} else if ( tag == "p" || tag == "div") {
				smiltext_run run = m_run_stack.top();
				_get_formatting(run, item);
				m_run_stack.push(run);
				if (m_params.m_mode != stm_crawl ) {
					// insert conditional line break
					smiltext_run r = m_run_stack.top();
					r.m_data = "";
					r.m_command = stc_condbreak;
					_insert_run_at_end(r);
				}
			} else {
				lib::logger::get_logger()->trace("smilText: unknown tag <%s>", tag.c_str());
			}
			//
			// Finally, if the node has an ID we raise a marker event.
			// There's magic to make tevEvent and markerEvent be unified.
			const char *id = item->get_attribute("id");
			if (id) {
				m_client->marker_seen(id);
			}
		}
	}
	// Insert empty data node at end, so a final clear will work
	if (m_tree_iterator.is_end() && !m_newbegin_valid) {
		smiltext_run run = m_run_stack.top();
		run.m_command = stc_data;
		run.m_data = "";
		_insert_run_at_end(run);

	}
	if ((m_params.m_rate > 0 || m_auto_rate)
		&& (m_params.m_mode == stm_crawl || m_params.m_mode == stm_scroll)) {
		// We need to schedule another update event to keep the scroll/crawl going.
		// In principle we do a callback per pixel scrolled, but clamp at 25 per second.
		unsigned int delay = m_auto_rate ? 40 : 1000 / m_params.m_rate;
		if (delay < 40) delay = 40;
		AM_DBG lib::logger::get_logger()->debug("delay=%d: next_update_needed=%d", delay, next_update_needed);
		if (next_update_needed > delay || next_update_needed == 0) {
			next_update_needed = delay;
		}
	}
	if (next_update_needed > 0) {
		m_update_event = new update_callback(this, &smiltext_engine::_update);
		AM_DBG lib::logger::get_logger()->debug("add_event(0x%x): next_update_needed=%d m_update_event=0x%x", this, next_update_needed, m_update_event);
		m_event_processor->add_event(m_update_event, next_update_needed, lib::ep_med);
	}
	smiltext_notification* client = m_client;
	unlock();
	if (client)
		client->smiltext_changed();
}

// Fill a run with the formatting parameters from a node.
void
smiltext_engine::_get_formatting(smiltext_run& dst, const lib::node *src)
{
	const char *style = src->get_attribute("textStyle");
	if (style) {
		const lib::node_context *ctx = src->get_context();
		assert(ctx);
		if (ctx == NULL) {
			return;
		}
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
	if (bg_color && strcmp(bg_color,"transparent") != 0) {
		dst.m_bg_transparent = false;
		dst.m_bg_color = lib::to_color(bg_color);
	}
	const char *color = src->get_attribute("textColor");
	if (color) {
		dst.m_transparent = false;
		dst.m_color = lib::to_color(color);
	}
	dst.m_font_families.clear();
	const char *font_family = src->get_attribute("textFontFamily");
	if (font_family) {
		const char *endpos;
		do {
			endpos = strchr(font_family, ',');
			size_t len = endpos? (endpos-font_family) : strlen(font_family);
			std::string family(font_family, len);
			dst.m_font_families.push_back(family);
			font_family = endpos + 1;
		} while(endpos);
	}
	dst.m_font_families.push_back("sansSerif");
	const char *font_size = src->get_attribute("textFontSize");
	if (font_size) {
		if (strcmp(font_size, "xx-small") == 0) dst.m_font_size = 8;
		else if (strcmp(font_size, "x-small") == 0) dst.m_font_size = 10;
		else if (strcmp(font_size, "small") == 0) dst.m_font_size = 12;
		else if (strcmp(font_size, "medium") == 0) dst.m_font_size = 14;
		else if (strcmp(font_size, "normal") == 0) dst.m_font_size = 14;
		else if (strcmp(font_size, "large") == 0) dst.m_font_size = 16;
		else if (strcmp(font_size, "x-large") == 0) dst.m_font_size = 18;
		else if (strcmp(font_size, "xx-large") == 0) dst.m_font_size = 20;
		else if (strcmp(font_size, "smaller") == 0) dst.m_font_size -= 2;
		else if (strcmp(font_size, "larger") == 0) dst.m_font_size += 2;
		else if (strcmp(font_size, "inherit") == 0) /* no-op */ ;
		else {
			// Check to see whether it is a numeric value ending in 'em', 'ex' or 'px'
			char *lastp;
			double num_size = strtod(font_size, &lastp);
			if (num_size > 0) {
				if (strcmp(lastp, "px") != 0 && *lastp != '\0') {
					num_size = 0;
				}
			}
			if (num_size) {
				dst.m_font_size = (int)num_size;
			} else {
				lib::logger::get_logger()->trace("%s: textFontSize=\"%s\": incorrect size", src->get_sig().c_str(), font_size);
			}
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
	const char *wrap = src->get_attribute("textWrapOption");
	if (wrap) {
		if (strcmp(wrap, "wrap") == 0) dst.m_wrap = true;
		else if (strcmp(wrap, "noWrap") == 0) dst.m_wrap = false;
		else if (strcmp(wrap, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: textWrapOption=\"%s\": must be wrap or noWrap", src->get_sig().c_str(), wrap);
		}
	}
	const char *writing_mode = src->get_attribute("textWritingMode");
	if (writing_mode) {
		if (src->get_local_name() == "span") {
			lib::logger::get_logger()->trace("%s: textWritingMode only allowed on smilText, div, p", src->get_sig().c_str());
		}
		else if (strcmp(writing_mode, "lr") == 0) dst.m_writing_mode = stw_lr_tb;
		else if (strcmp(writing_mode, "lr-tb") == 0) dst.m_writing_mode = stw_lr_tb;
		else if (strcmp(writing_mode, "rl") == 0) dst.m_writing_mode = stw_rl_tb;
		else if (strcmp(writing_mode, "rl-tb") == 0) dst.m_writing_mode = stw_rl_tb;
		else if (strcmp(writing_mode, "tb-lr") == 0) dst.m_writing_mode = stw_tb_lr;
		else if (strcmp(writing_mode, "tb-rl") == 0) dst.m_writing_mode = stw_tb_rl;
		else if (strcmp(writing_mode, "inherit") == 0) /* no-op */;
		else {
			lib::logger::get_logger()->trace("%s: textWritingMode=\"%s\": unknown writing mode", src->get_sig().c_str(), writing_mode);
		}
	}
	const char *direction = src->get_attribute("textDirection");
	if (direction) {
		if (src->get_local_name() != "span" && src->get_local_name() != "textStyle") {
			lib::logger::get_logger()->trace("%s: textWritingMode only allowed on span or textStyle", src->get_sig().c_str());
		}
		else if (strcmp(direction, "ltr") == 0) dst.m_direction = stw_ltr;
		else if (strcmp(direction, "rtl") == 0) dst.m_direction = stw_rtl;
		else if (strcmp(direction, "ltro") == 0) dst.m_direction = stw_ltro;
		else if (strcmp(direction, "rtlo") == 0) dst.m_direction = stw_rtlo;
		else if (strcmp(direction, "inherit") == 0) /* no-op */;
		else {
			lib::logger::get_logger()->trace("%s: textDirection=\"%s\": unknown text direction", src->get_sig().c_str(), writing_mode);
		}
	}
	// xml:space attribute
	const char *xml_space = src->get_attribute("space");
	if (xml_space) {
		if (strcmp(xml_space, "preserve") == 0)		dst.m_xml_space = stx_preserve;
		else if (strcmp(xml_space, "default") == 0) dst.m_xml_space = stx_default;
		else if (strcmp(xml_space, "inherit") == 0) /* no-op */ ;
		else {
			lib::logger::get_logger()->trace("%s: xml:space=\"%s\": must be default or preserve", src->get_sig().c_str(), xml_space);
		}
	}
}

// Fill a run with the default formatting.
void
smiltext_engine::_get_default_formatting(smiltext_run& dst)
{
//	dst.m_font_family = "sansSerif";
	dst.m_font_style = sts_normal;
	dst.m_font_weight = stw_normal;
	dst.m_font_size = 12;
	dst.m_transparent = false;
	dst.m_color = lib::color_t(0);
	dst.m_bg_transparent = true;
	dst.m_bg_color = lib::color_t(0);
	dst.m_xml_space = stx_default;
	dst.m_align = sta_start;
	dst.m_writing_mode = stw_lr_tb;
	dst.m_wrap = true;
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
	const char *rate = src->get_attribute("textRate");
	if (rate) {
		if (strcmp(rate, "auto") == 0) {
			params.m_rate = 0;
		} else {
			int rate_i = atoi(rate); // XXXX
			params.m_rate = rate_i;
		}
	}
	const char *text_place = src->get_attribute("textPlace");
	if (text_place) {
		if (strcmp(text_place, "start") == 0) params.m_text_place = stp_from_start;
		else if (strcmp(text_place, "end") == 0) params.m_text_place = stp_from_end;
		else if (strcmp(text_place, "center") == 0) params.m_text_place = stp_from_center;
		else if (strcmp(text_place, "inherit") == 0) /* no-op */;
		else {
			lib::logger::get_logger()->trace("%s: textPlace=\"%s\": unknown textPlace", src->get_sig().c_str(), text_place);
		}
	}
	const char *text_conceal = src->get_attribute("textConceal");
	if (text_conceal) {
		if (strcmp(text_conceal, "none") == 0) params.m_text_conceal = stc_none;
		else if (strcmp(text_conceal, "initial") == 0) params.m_text_conceal = stc_initial;
		else if (strcmp(text_conceal, "final") == 0) params.m_text_conceal = stc_final;
		else if (strcmp(text_conceal, "both") == 0) params.m_text_conceal = stc_both;
		else if (strcmp(text_conceal, "inherit") == 0) /* no-op */;
		else {
			lib::logger::get_logger()->trace("%s: textConceal=\"%s\": unknown textConceal", src->get_sig().c_str(), text_conceal);
		}
	}
}

// Fill default smiltext_params
void
smiltext_engine::_get_default_params(smiltext_params& params)
{
	params.m_mode = stm_append;
	params.m_rate = 0;
	params.m_text_place = stp_from_start;
	params.m_text_conceal = stc_none;
}

// Called as soon as textRate is known. Turns off m_auto_rate.
void
smiltext_engine::set_rate(unsigned int new_rate)
{
	if (m_auto_rate && new_rate > 0) {
		smiltext_params params = get_params();
		params.m_rate = new_rate;
		m_params = params;
		m_auto_rate = false;
	}
}

// Return the simple duration of a <smilText/> element
int
smiltext_engine::get_dur() {
	// XXXJACK: this is incorrect: dur is not an integer, it is a time (could be float, could
	// be timecode, whatever).
	// Also, we should really get this from time_node or time_attrs.
	const char *durs = m_node->get_attribute("dur");
	if (durs == NULL) return 0;
	int dur = atoi(durs);
	return dur < 0 ? 0 : dur;
}

// get the lock for m_runs
void
smiltext_engine::lock() {
	m_lock.enter();
	assert ( ! m_is_locked);
	m_is_locked = true;
}

// release the lock for m_runs
void
smiltext_engine::unlock() {
	assert (m_is_locked);
	m_is_locked = false;
	m_lock.leave();
}

// Returns an iterator pointing to the first smiltext_run.
// Must be called while locked.
smiltext_runs::const_iterator
smiltext_engine::begin() {
	assert (m_is_locked);
	return m_runs.begin();
}

// Returns an iterator pointing to the first unseen smiltext_run.
// Must be called while locked.
smiltext_runs::const_iterator
smiltext_engine::newbegin() {
	assert (m_is_locked);
	return m_newbegin_valid ? m_newbegin : (smiltext_runs::const_iterator)m_runs.end();
}

// Returns an iterator pointing to the end of the smiltext_runs.
// Must be called while locked.
smiltext_runs::const_iterator
smiltext_engine::end() {
	assert (m_is_locked);
	return m_runs.end();
}

// Called when the client has processed all runs.
// Must be called while locked.
void
smiltext_engine::done() {
	assert (m_is_locked);
	m_newbegin = m_runs.end();
	m_newbegin_valid = false;
}

// Returns true if all text has been received.
// Must be called while locked.
bool
smiltext_engine::is_finished() {
	assert (m_is_locked);
	return m_tree_iterator.is_end();
}

// Returns true if the text has changed since the last done() call.
// Must be called while locked.
bool
smiltext_engine::is_changed() {
	assert (m_is_locked);
	return m_newbegin_valid;
}

// Returns true if the text has been cleared and should be re-rendered from scratch.
// Must be called while locked.
bool
smiltext_engine::is_cleared() {
	assert (m_is_locked);
	return m_newbegin_valid && m_newbegin == m_runs.begin();
}

////////////////////////////////////////////////////////////////////////
//	smiltext_layout_engine						  //
////////////////////////////////////////////////////////////////////////
smiltext_layout_engine::smiltext_layout_engine(const lib::node *n, lib::event_processor *ep, smiltext_layout_provider* provider, smiltext_notification* client, bool process_lf)
  : m_engine(smiltext_engine(n, ep, client, true)),
	m_process_lf(process_lf),
	m_event_processor(ep),
	m_provider(provider),
	m_params(m_engine.get_params()),
	m_needs_conditional_newline(false),
	m_needs_conditional_space(false),
	m_crawling(m_params.m_mode == smil2::stm_crawl),
	m_scrolling(m_params.m_mode == smil2::stm_scroll),
	m_shifted_origin(0,0)
{
}

void
smiltext_layout_engine::start(double t) {
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	m_crawling = m_params.m_mode == smil2::stm_crawl;
	m_scrolling = m_params.m_mode == smil2::stm_scroll;
	m_shifted_origin = lib::point(0,0);
}

void
smiltext_layout_engine::seek(double t) {
	m_engine.seek(t);
}

void
smiltext_layout_engine::stop() {
	m_engine.stop();
}

bool
smiltext_layout_engine::is_finished()
{
	m_engine.lock();
	bool rv = m_engine.is_finished();
	m_engine.unlock();
	return rv;
}

void
smiltext_layout_engine::set_dest_rect( const lib::rect& r) {
	m_dest_rect = r;
}


smiltext_layout_word::smiltext_layout_word(const smiltext_run run, smiltext_metrics stm, int n_nl)
:	m_run(run),
	m_leading_newlines(n_nl),
	m_metrics(stm)
{
	m_bounding_box = lib::rect(
		lib::point(0,0),
		lib::size(stm.get_width(), stm.get_height()));
}

void
smiltext_layout_engine::smiltext_changed() {
	m_engine.lock();
	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::smiltext_changed(0x%x)", this);
	if (m_engine.is_changed()) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;

		if (m_engine.is_cleared()) {
			// Completely new text, clear the copy.
			m_needs_conditional_space = false;
			m_needs_conditional_newline = false;
				m_words.clear();
			i = m_engine.begin();
		} else {
			// Only additions. Don't clear, store new stuff.
			i = m_engine.newbegin();
		}
		int n_nl = 0; // # of newlines
		while (i != m_engine.end()) {
			switch (i->m_command) {
			default:
				assert(0);
				break;
			case smil2::stc_break:
				n_nl += 1;
				m_needs_conditional_space = false;
				m_needs_conditional_newline = false;
				break;
			case smil2::stc_condbreak:
				if (m_needs_conditional_newline) {
					n_nl += 2;
					m_needs_conditional_space = false;
					m_needs_conditional_newline = false;
				}
				break;
			case smil2::stc_condspace:
				if (m_needs_conditional_space) {
					// conditional spaces are placed between words.
					// Later, they are either converted into data or ignored,
					// the latter only when appearing immediately before a newline,
					// or at the end of a run.
					// create a space word, copying attributes from previous word
					m_space = *i;
					smiltext_metrics stm = m_provider->get_smiltext_metrics (m_space);
					smiltext_layout_word word_info(m_space, stm, n_nl);
					m_words.push_back(word_info);
					m_needs_conditional_newline = true;
					m_needs_conditional_space = false;
				}
				break;
			case smil2::stc_data:
				if ((*i).m_data.begin() == (*i).m_data.end()) {
					m_needs_conditional_newline = false;
					m_needs_conditional_space = false;
				} else {
					char lastch = *((*i).m_data.rbegin());
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
				}
				smiltext_metrics stm =
					m_provider->get_smiltext_metrics (*i);
				smiltext_layout_word word_info(*i, stm, n_nl);
				m_words.push_back(word_info);
				n_nl = 0;
				AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::smiltext_changed(0x%x) data=%s, H=%d,W=%d", this, i->m_data.c_str(), stm.get_width(), stm.get_height());
				break;
			}
			i++;
		}
	}
	m_engine.unlock();
}

void
smiltext_layout_engine::_get_initial_values(
	lib::rect rct,
	smiltext_layout_word* stlw_p,
	int* x_start_p,
	int* y_start_p,
	int* x_dir_p,
	int* y_dir_p)
{
	switch (stlw_p->m_run.m_writing_mode) {
	default:
	case stw_lr_tb:
		*x_start_p = rct.left();
		*x_dir_p = 1;
		break;
	case stw_rl_tb:
		// in right-to-left mode, everything is computed
		//	w.r.t. the right-top corner of the bounding box
		*x_start_p = rct.right();
		*x_dir_p = -1;
		break;
	}
	*y_start_p = rct.top();
	/* implementation of textPlace attribute */
	switch (m_params.m_text_place) {
	default:
	case stp_from_start:
		// Note: this assumes secondary writing direction is top-to-bottom
		*y_dir_p = 1;
		*y_start_p = rct.top();
		break;
	case stp_from_end:
		// Note: this assumes secondary writing direction is top-to-bottom
		//*y_dir_p = -1;
		*y_dir_p = 1;
		*y_start_p = rct.bottom() - stlw_p->m_bounding_box.h;
		break;
	case stp_from_center:
		*y_dir_p = 1;
		*y_start_p = (rct.bottom() - rct.top() - stlw_p->m_bounding_box.h) / 2;
		break;
	}
}

void
smiltext_layout_engine::redraw(const lib::rect& r) {
	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(0x%x) r=(L=%d,T=%d,W=%d,H=%d", this,r.left(),r.top(),r.width(),r.height());
	m_engine.lock();
	if (m_words.empty()) {
		m_engine.unlock();
		return;
	}
	const lib::rect rect = m_provider->get_rect();
	int x_start = 0, y_start = 0, x_dir = 1, y_dir = 1;
	_get_initial_values(rect, &*m_words.begin(),
				&x_start, &y_start, &x_dir, &y_dir);
	smil2::smiltext_align align = m_words.begin()->m_run.m_align;
	smil2::smiltext_writing_mode writing_mode = m_words.begin()->m_run.m_writing_mode;
	bool wrap_lines = m_words.begin()->m_run.m_wrap;
	if (writing_mode == stw_rl_tb)
		switch (align) {
		case sta_start:
			align = sta_right;
			break;
		case sta_end:
			align = sta_left;
		default:
			break;
		}
	// Compute the shifted position of what we want to draw w.r.t. the visible origin
	if (m_crawling) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		m_shifted_origin.x = (int) now * m_params.m_rate / 1000 * x_dir;
		if (m_shifted_origin.x < 0)
		AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw(0x%x): strange: shifted_x=%d, m_epoch=%ld, elpased=%ld !", this, m_shifted_origin.x, m_epoch, elapsed);
		switch (align) {
		default:
		case sta_start:
		case sta_left:
			break;
		case sta_center:
			x_start = r.left() + (r.right() - r.left())
							/ 2;
			/* KB no vertical centering ?
			y_start = r.top() + (r.bottom() - r.top())
							/ 2
				- (m_words.begin()->m_bounding_box.height())
						/ 2;
			*/
			break;
		case sta_right:
		case sta_end:
			if (writing_mode == stw_rl_tb)
				x_start = r.left();
			else	x_start = r.right();
			break;
		}
		if (m_params.m_text_conceal == smil2::stc_initial
			|| m_params.m_text_conceal == smil2::stc_both) {
			if (writing_mode == stw_rl_tb)
				x_start = r.left();
			else	x_start = r.right();
		}

	}
	if (m_scrolling) {
		if (m_params.m_text_conceal == smil2::stc_initial
			|| m_params.m_text_conceal == smil2::stc_both) {
			y_start = r.bottom();
		}
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		m_shifted_origin.y = (int) now * m_params.m_rate / 1000 * y_dir;
		AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw: m_rate=%d y_dir=%d now=%lf m_shifted_origin(%d,%d)", m_params.m_rate, y_dir, now, m_shifted_origin.x, m_shifted_origin.y);
	}
	AM_DBG lib::logger::get_logger()->debug("smiltext_layout_engine::redraw: m_shifted_origin(%d,%d)", m_shifted_origin.x, m_shifted_origin.y);

	bool linefeed_processing = ! m_crawling;

	// textAlign, textWritingMode, textPlace, etc. is  implemented
	// by giving x_start|y_start|x_dir|y_dir proper initial values
	int prev_max_ascent = 0, prev_max_descent = 0;
	std::vector<smiltext_layout_word>::iterator
		bol,// begin of line
		eol,// end of line
		first_word,
		last_word_seen,
		word;
	first_word = m_words.begin();
	last_word_seen = first_word;

	for (bol = m_words.begin(); bol != m_words.end(); bol = eol) {
		unsigned int max_ascent = 0, max_descent = 0;
		int x = x_start;
		int y = y_start;
		bool is_first_word = true;
		align = bol->m_run.m_align;
		// find end of line
		for (word = bol; word != m_words.end(); word++) {
			if (word != bol && word->m_leading_newlines != 0)
				break;
			// for each word on this line see if it fits
			// for rtl, x==word->m_bounding_box.right()
			if (word->m_run.m_writing_mode == stw_rl_tb) {
				word->m_bounding_box.x = x - word->m_bounding_box.w;
			} else {
				word->m_bounding_box.x = x;
			}
			word->m_bounding_box.y = y;
			// first word in a line is shown always,
			// because otherwise nothing would be shown:
			// if it doesn't fit on this line it won't fit
			// on the next line in the rectangle either
			if (linefeed_processing
				&& ! is_first_word
				&& wrap_lines
				&&! _smiltext_fits(word->m_bounding_box,rect))
			{
				if (word->m_leading_newlines == 0)
					word->m_leading_newlines++;
				break;
			}
			is_first_word = false;
			// compute x-position of next word
			x += word->m_metrics.get_width()* x_dir;
			// find max. height (ascent+descent) of all words
			if (word->m_metrics.get_ascent() > max_ascent)
				max_ascent = word->m_metrics.get_ascent();
			if (word->m_metrics.get_descent() > max_descent)
				max_descent = word->m_metrics.get_descent();
			last_word_seen = word;
		}
		eol = word;
		std::vector<smiltext_layout_word>::iterator lwl = word - 1;  // last word on line
		// alignment processing
		int x_align = 0, x_min, x_max;
		if (bol->m_run.m_writing_mode == stw_rl_tb) {
			x_min = lwl->m_bounding_box.left();
			x_max = bol->m_bounding_box.right();
		} else {
			x_min = bol->m_bounding_box.left();
			x_max = lwl->m_bounding_box.right();
		}
		switch (align) {
		default:
		case sta_start:
		case sta_left:
			if (writing_mode == stw_rl_tb && x_min > r.left())
				x_align = r.left() - x_min;
			break;
		case sta_center:
			if (writing_mode == stw_lr_tb) {
				if (x_max < r.right())
					x_align = (r.right() - x_max) / 2;
			} else {
				if (x_min > r.left())
					x_align = (r.left() - x_min) / 2;
			}
			break;
		case sta_end:
		case sta_right:
			if (writing_mode == stw_lr_tb
				&& x_max < r.right())
				x_align = r.right() - x_max;
			break;
		}
		if (linefeed_processing) {
			// if a run starts with blank lines, take the
			// height of the first line as line distance
			if (prev_max_ascent == 0)
				prev_max_ascent = max_ascent;
			if (prev_max_descent == 0)
				prev_max_descent = max_descent;
			// compute y-position of next line
			y_start += (prev_max_ascent + prev_max_descent) * bol->m_leading_newlines * y_dir;
			if (m_params.m_mode == smil2::stm_jump
				&& (y_start - m_shifted_origin.y + (int) max_ascent + (int) max_descent) > r.bottom())
			{
				int rate = 1;
				if (m_params.m_rate > 1)
					rate = m_params.m_rate;
				m_shifted_origin.y += rate * (max_ascent + max_descent);
			}
			// ignore previous space after adding a newline
			if (last_word_seen->m_run.m_command == stc_condspace)
				last_word_seen->m_run.m_command = stc_ignore;
		}
		for (word = bol; word != eol; word++) {
			// alignment correction
			if ( ! m_crawling)
				word->m_bounding_box.x += x_align;
			// baseline correction
			word->m_bounding_box.y = y_start + max_ascent - word->m_metrics.get_ascent();
			last_word_seen = word;
		}
		prev_max_ascent = max_ascent;
		prev_max_descent = max_descent;
	}
	// ignore previous space at end of run
	if (last_word_seen->m_run.m_command == stc_condspace)
		last_word_seen->m_run.m_command = stc_ignore;
	std::vector<smiltext_layout_word>::iterator last_word =	 m_words.end() - 1;
	if (m_crawling || m_scrolling) {
		if (m_engine.is_auto_rate()) {
			// now we have all information to compute the rate
			int dur = m_engine.get_dur();
			if (dur) {
				lib::rect smiltext_rect = first_word->m_bounding_box | last_word->m_bounding_box;
				unsigned int rate = compute_rate(m_params, m_words.begin()->m_run.m_align, smiltext_rect.size(), rect, dur);
				m_engine.set_rate(rate);
				m_params = m_engine.get_params();
			}
//*KB*/ lib::logger::get_logger()->debug("smiltext_layout_engine::redraw: dur=%d rate%d", dur, rate);
		}
	}
	// layout done, render the run
	for (word = m_words.begin(); word != m_words.end(); word++) {
		word->m_bounding_box -= m_shifted_origin;
		if (word->m_run.m_command == stc_condspace)
			word->m_run.m_command = stc_data;
		if (word->m_run.m_command != stc_data || _smiltext_disjunct (word->m_bounding_box, rect))
			continue; // nothing to de displayed
		m_provider->render_smiltext(word->m_run, word->m_bounding_box);
	}
	m_engine.unlock();
}

// return true if r1 completely fits horizontally in r2
bool
smiltext_layout_engine::_smiltext_fits(const lib::rect& r1, const lib::rect& r2) {
	return r2.left() <=	 r1.left() && r1.right() <= r2.right();
}

// return true if r1 and r2 have a zero intersection
bool
smiltext_layout_engine::_smiltext_disjunct(const lib::rect& r1, const lib::rect& r2) {
	return (r1 & r2) == lib::rect();
}

unsigned int
smiltext_layout_engine::compute_rate(const smiltext_params params, const smiltext_align align, const lib::size size, const lib::rect r,	 const unsigned int dur) {
	/* First find the distance to travel during scroll for various values
	* for textConceal and textPlace (w=window height, t=text height)

	+ ---------------------------------------------------+
	| textConceal |	 none	 | initial |  final	 |	both |
	|-------------|			 |		   |		 |		 |
	| textPlace	  |			 |		   |		 |		 |
	|----------------------------------------------------|
	|	start	  | t>w?t-w:0|	  t	   |	t	 |	w+t	 |
	| ---------------------------------------------------|
	|	center	 |t>w?t-w/2:w/2|  t	   | w/2+t	 |	w+t	 |
	| ---------------------------------------------------|
	|	end		  |	   t	 |	  t	   |  w+t	 |	w+t	 |
	+ ---------------------------------------------------+
	*/
	if (dur == 0) return 0;

	unsigned int dst = 0, win = 0, txt = 0;

	switch (params.m_mode) {

	case smil2::stm_crawl:
		win = r.w;
		txt = size.w;
		// Convert any sta_left/sta_right values into sta_start/sta_end
		// as defined by textWritingMode
		smiltext_align _align;
		switch (align) {
		default:
			_align = align;
			break;
		case smil2::sta_left:
			//TBD adapt for textWritingMode
			_align = smil2::sta_start;
			break;
		case smil2::sta_right:
			//TBD adapt for textWritingMode
			_align = smil2::sta_end;
			break;
		}
		switch (params.m_text_conceal) {
		default:
		case smil2::stc_none:
			switch (_align) {
			default:
			case smil2::sta_start:
				dst = txt > win ? txt - win : 0;
				break;
			case smil2::sta_end:
				dst = txt;
				break;
			case smil2::sta_center:
				dst = txt > win/2 ? txt - win/2 : 0;
				break;
			}
			break;
		case smil2::stc_initial: // ignore textAlign
			dst = txt;
			break;
		case smil2::stc_final:
			switch (_align) {
			default:
			case smil2::sta_start:
				dst = txt;
				break;
			case smil2::sta_end:
				dst = win+txt;
				break;
			case smil2::sta_center:
				dst = win/2+txt;
				break;
			}
			break;
		case smil2::stc_both: // ignore textAlign
			dst = win+txt;
			break;
		}
		break;

	case smil2::stm_scroll:
		win = r.h;
		txt = size.h;
		switch (params.m_text_conceal) {
		default:
		case smil2::stc_none:
			switch (params.m_text_place) {
			default:
			case smil2::stp_from_start:
				dst = txt > win ? txt - win : 0;
				break;
			case smil2::stp_from_end:
				dst = txt;
				break;
			case smil2::stp_from_center:
				dst = txt > win/2 ? txt - win/2 : 0;
				break;
			}
			break;
		case smil2::stc_initial: // ignore textPlace
			dst = txt;
			break;
		case smil2::stc_final:
			switch (params.m_text_place) {
			default:
			case smil2::stp_from_start:
				dst = txt;
				break;
			case smil2::stp_from_end:
				dst = win+txt;
				break;
			case smil2::stp_from_center:
				dst = win/2+txt;
				break;
			}
			break;
		case smil2::stc_both: // ignore textPlace
			dst = win+txt;
			break;
		}
		break;

	default:
		break;
	}
	return (dst+dur-1)/dur;
}
 
} // namespace smil2
} // namespace ambulant
