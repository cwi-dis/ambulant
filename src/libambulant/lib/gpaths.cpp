/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/lib/gpaths.h"
#include "ambulant/lib/parselets.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/lib/logger.h"
#include <algorithm>

#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

//////////////////
// path_descr

lib::path_descr::path_descr(const std::string& strpath) 
:	m_strpath(strpath), 
	m_errors(0) {
	
	// The range of the string to parse
	std::string::iterator b = m_strpath.begin();
	std::string::iterator e = m_strpath.end();
	
	// The range of svg commands to search for
	std::string::const_iterator cb = path_seg_cmds.begin();
	std::string::const_iterator ce = path_seg_cmds.end();
	
	// Extract the SVG path segments commands and arguments from strpath
	lib::number_list_p parser;
	std::string::iterator it = std::find_first_of(b, e, cb, ce);
	while(it != e) {
		// locate the end of this cmd and the start of the next
		b = it; b++;
		std::string::iterator next = std::find_first_of(b, e, cb, ce);
		
		// the cmd and the args string
		char cmd = *it;
		std::string args = lib::trim(std::string(++it, next));
		
		// store cmd
		m_cmds += cmd;
		
		// parse the cmd args string and store the numbers into m_args
		std::string::size_type ix = path_seg_cmds.find(cmd);
		assert(ix != std::string::npos);
		size_t numbers_expected = path_seg_cmds_size[ix];
		if(numbers_expected>0) {
			// The cmd requires a set of numbers
			if(!parser.matches(args) || parser.size() != numbers_expected) {
				lib::logger::get_logger()->warn("Illegal path segment %c %s", cmd, args.c_str());
				for(size_t i=0;i<numbers_expected;i++)
					m_args.push_back(0);
				m_errors++;
			} else {
				m_args.insert(m_args.end(), parser.begin(), parser.end());
			}
		} else {
			// The cmd does not require any arguments
			if(!args.empty()) {
				lib::logger::get_logger()->warn("Illegal path segment %c %s", cmd, args.c_str());
				m_errors++;
			}
		}
		
		// Update start of next command
		it = next;
	}
}

//////////////////
// generic path_builder

// static
lib::path_builder::path_seg_handler_t
lib::path_builder::path_seg_hanlders[] = {
	&path_builder::close_path,
	&path_builder::moveto,
	&path_builder::lineto,
	&path_builder::curveto,
	&path_builder::quadratic_bezier_curveto,
	&path_builder::elliptic_arc,
	&path_builder::horizontal_lineto,
	&path_builder::vertical_lineto,
	&path_builder::smooth_curveto,
	&path_builder::truetype_quadratic_bezier_curveto
};

// static
lib::logger *lib::path_builder::plogger = 0;

lib::path_builder::path_builder(const lib::path_descr* pd) 
:	m_pd(pd),
	m_cpx(0), m_cpy(0),
	m_mx(0), m_my(0) {
	if(!plogger) plogger = lib::logger::get_logger();
}

void lib::path_builder::build_path() {
	const std::string& cmds = m_pd->get_cmds();
	const std::list<double>& args = m_pd->get_args();
	std::string::const_iterator curr_cmd_it = cmds.begin();
	arg_iterator curr_arg_it = args.begin();
	while(curr_cmd_it != cmds.end()) {
		std::string::size_type ix = lib::path_seg_cmds.find(*curr_cmd_it);
		if(ix != std::string::npos) {
			path_seg_handler_t handler = path_seg_hanlders[ix/2];
			if(handler) (this->*handler)((ix%2==0), curr_arg_it);
		} else {
			plogger->error("Failed to find command '%c'", *curr_cmd_it);
		}
		curr_cmd_it++;
		size_t nargs = lib::path_seg_cmds_size[ix];
		while(nargs>0) {curr_arg_it++;nargs--;}
	}
}

// z
void lib::path_builder::close_path(bool abs, arg_iterator it) { 
	char cmd = abs?'Z':'z';
	m_cpx = m_mx; m_cpy = m_my;
	plogger->trace("%c [CP: %.0f, %.0f]", cmd, m_cpx, m_cpy);
}

// m %f{2} 
void lib::path_builder::moveto(bool abs, arg_iterator it) { 
	char cmd = abs?'M':'m';
	double x = *it++;
	double y = *it++;
	if(abs) {m_cpx = x; m_cpy = y;}
	else {m_cpx += x; m_cpy += y;}
	m_mx = m_cpx; m_my = m_cpy;
	plogger->trace("%c %.0f %.0f [CP: %.0f, %.0f]", cmd, x, y, m_cpx, m_cpy);
}

// l %f{2}
void lib::path_builder::lineto(bool abs, arg_iterator it) {
	char cmd = abs?'L':'l';
	double x = *it++;
	double y = *it++;
	if(abs) {m_cpx = x; m_cpy = y;}
	else {m_cpx += x; m_cpy += y;}
	plogger->trace("%c %.0f %.0f [CP: %.0f, %.0f]", cmd, x, y, m_cpx, m_cpy);
}

// c %f{6}
void lib::path_builder::curveto(bool abs, arg_iterator it) {
	char cmd = abs?'C':'c';
	double x1 = *it++;
	double y1 = *it++;
	double x2 = *it++;
	double y2 = *it++;
	double x = *it++;
	double y = *it++;
	if(abs) {m_cpx = x; m_cpy = y;}
	else {m_cpx += x; m_cpy += y;}
	plogger->trace("%c %.0f %.0f %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]", cmd, x1, y1, x2, y2, x, y, m_cpx, m_cpy);
}

// q %f{4}
void lib::path_builder::quadratic_bezier_curveto(bool abs, arg_iterator it) {
	char cmd = abs?'Q':'q';
	double x1 = *it++;
	double y1 = *it++;
	double x = *it++;
	double y = *it++;
	if(abs) {m_cpx = x; m_cpy = y;}
	else {m_cpx += x; m_cpy += y;}
	plogger->trace("%c %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]", cmd, x1, y1, x, y, m_cpx, m_cpy);
}

// a %f{7}
void lib::path_builder::elliptic_arc(bool abs, arg_iterator it) {
	char cmd = abs?'A':'a';
	double r1 = *it++;
	double r2 = *it++;
	double angle = *it++;
	int arc_flag = int(*it++);
	int seep_flag = int(*it++);
	double x = *it++;
	double y = *it++;
	if(abs) {m_cpx = x; m_cpy = y;}
	else {m_cpx += x; m_cpy += y;}
	plogger->trace("%c %.0f %.0f %d %d %.0f %.0f [CP: %.0f, %.0f]", cmd,  r1, r2, arc_flag, seep_flag, x, y, m_cpx, m_cpy);
}

// h %f{1}
void lib::path_builder::horizontal_lineto(bool abs, arg_iterator it) {
	char cmd = abs?'H':'h';
	double x = *it++;
	if(abs) {m_cpx = x;}
	else {m_cpx += x;}
	plogger->trace("%c %.0f [CP: %.0f, %.0f]", cmd, x, m_cpx, m_cpy);
}

// v %f{1}
void lib::path_builder::vertical_lineto(bool abs, arg_iterator it) {
	char cmd = abs?'V':'v';
	double y = *it++;
	if(abs) {m_cpy = y;}
	else {m_cpy += y;}
	plogger->trace("%c %.0f [CP: %.0f, %.0f]", cmd, y, m_cpx, m_cpy);
}

// s %f{4}
void lib::path_builder::smooth_curveto(bool abs, arg_iterator it) {
	char cmd = abs?'S':'s';
	double x2 = *it++;
	double y2 = *it++;
	double x = *it++;
	double y = *it++;
	if(abs) {m_cpx = x; m_cpy = y;}
	else {m_cpx += x; m_cpy += y;}
	plogger->trace("%c %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]", cmd, x2, y2, x, y, m_cpx, m_cpy); 
}

// t %f{2}
void lib::path_builder::truetype_quadratic_bezier_curveto(bool abs, arg_iterator it) {
	char cmd = abs?'T':'t';
	double x = *it++;
	double y = *it++;
	if(abs) {m_cpx = x; m_cpy = y;}
	else {m_cpx += x; m_cpy += y;}
	plogger->trace("%c %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]", cmd, x, y, m_cpx, m_cpy); 
}
