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

#include "ambulant/lib/gpaths.h"
#include "ambulant/lib/parselets.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/lib/logger.h"
#include <algorithm>

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

//////////////////
// path_descr

lib::gpath_descr::gpath_descr(const std::string& strpath)
:	m_strpath(strpath),
	m_errors(0) {

	// The range of the string to parse
	std::string::iterator b = m_strpath.begin();
	std::string::iterator e = m_strpath.end();

	// The range of svg commands to search for
	std::string::const_iterator cb = gpath_seg_cmds.begin();
	std::string::const_iterator ce = gpath_seg_cmds.end();

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
		std::string::size_type ix = gpath_seg_cmds.find(cmd);
		assert(ix != std::string::npos);
		size_t numbers_expected = gpath_seg_cmds_size[ix];
		if(numbers_expected>0) {
			// The cmd requires a set of numbers
			bool matches = parser.matches(args);
			size_t nc = parser.size()/numbers_expected;
			size_t nr = parser.size()%numbers_expected;
			if(!matches || nr != 0 || nc == 0) {
				lib::logger::get_logger()->trace("Illegal path segment %c %s", cmd, args.c_str());
				for(size_t i=0;i<numbers_expected;i++)
					m_args.push_back(0);
				m_errors++;
			} else {
				if(nc == 1)
					m_args.insert(m_args.end(), parser.begin(), parser.end());
				else {
					// multiple commands
					const std::vector<double>& argsv =  parser.get_result();
					(void)argsv; // Suppress compiler warning
					std::vector<double>::const_iterator bit = parser.begin();
					std::vector<double>::const_iterator eit = bit;
					eit += numbers_expected;
					m_args.insert(m_args.end(), bit, eit);
					for(size_t i=1;i<nc;i++) {
						m_cmds += cmd;
						bit += numbers_expected;
						eit += numbers_expected;
						m_args.insert(m_args.end(), bit, eit);
					}
				}
			}
		} else {
			// The cmd does not require any arguments
			if(!args.empty()) {
				lib::logger::get_logger()->trace("Illegal path segment %c %s", cmd, args.c_str());
				m_errors++;
			}
		}

		// Update start of next command
		it = next;
	}
	AM_DBG lib::logger::get_logger()->debug("%s --> %s", strpath.c_str(), m_cmds.c_str());
}


//////////////////
// the base gpath_builder

// static
lib::gpath_builder::path_seg_handler_t
lib::gpath_builder::path_seg_hanlders[] = {
	&gpath_builder::close_path,
	&gpath_builder::moveto,
	&gpath_builder::lineto,
	&gpath_builder::curveto,
	&gpath_builder::quadratic_bezier_curveto,
	&gpath_builder::elliptic_arc,
	&gpath_builder::horizontal_lineto,
	&gpath_builder::vertical_lineto,
	&gpath_builder::smooth_curveto,
	&gpath_builder::truetype_quadratic_bezier_curveto
};

// static
lib::logger *lib::gpath_builder::plogger = 0;

lib::gpath_builder::gpath_builder()
:	m_cpx(0), m_cpy(0),
	m_mx(0), m_my(0) {
	if(!plogger) plogger = lib::logger::get_logger();
}

lib::gpath *lib::gpath_builder::build_path(const lib::gpath_descr* pd) {
	reset();
	const std::string& cmds = pd->get_cmds();
	const std::list<double>& args = pd->get_args();
	std::string::const_iterator curr_cmd_it = cmds.begin();
	arg_iterator curr_arg_it = args.begin();
	while(curr_cmd_it != cmds.end()) {
		std::string::size_type ix = lib::gpath_seg_cmds.find(*curr_cmd_it);
		if(ix != std::string::npos) {
			path_seg_handler_t handler = path_seg_hanlders[ix/2];
			if(handler) (this->*handler)((ix%2==0), curr_arg_it);
		} else {
			plogger->trace("gpath_builder: Failed to find command '%c'", *curr_cmd_it);
		}
		curr_cmd_it++;
		size_t nargs = lib::gpath_seg_cmds_size[ix];
		while(nargs>0) {curr_arg_it++;nargs--;}
	}
	return get_builded_path(pd);
}

void lib::gpath_builder::reset() {
	m_cpx =  m_cpy = 0;
	m_mx = m_my = 0;
}

// z
void lib::gpath_builder::close_path(bool abs, arg_iterator it) {
	m_cmd = abs?'Z':'z';
}

// m %f{2}
void lib::gpath_builder::moveto(bool abs, arg_iterator it) {
	m_cmd = abs?'M':'m';
	m_x = *it++;
	m_y = *it++;
}

// l %f{2}
void lib::gpath_builder::lineto(bool abs, arg_iterator it) {
	m_cmd = abs?'L':'l';
	m_x = *it++;
	m_y = *it++;
}

// c %f{6}
void lib::gpath_builder::curveto(bool abs, arg_iterator it) {
	m_cmd = abs?'C':'c';
	m_x1 = *it++;
	m_y1 = *it++;
	m_x2 = *it++;
	m_y2 = *it++;
	m_x = *it++;
	m_y = *it++;
}

// q %f{4}
void lib::gpath_builder::quadratic_bezier_curveto(bool abs, arg_iterator it) {
	m_cmd = abs?'Q':'q';
	m_x1 = *it++;
	m_y1 = *it++;
	m_x = *it++;
	m_y = *it++;
}

// a %f{7}
void lib::gpath_builder::elliptic_arc(bool abs, arg_iterator it) {
	m_cmd = abs?'A':'a';
	m_r1 = *it++;
	m_r2 = *it++;
	m_angle = *it++;
	m_arc_flag = *it++;
	m_sweep_flag = *it++;
	m_x = *it++;
	m_y = *it++;
}

// h %f{1}
void lib::gpath_builder::horizontal_lineto(bool abs, arg_iterator it) {
	m_cmd = abs?'H':'h';
	m_x = *it++;
}

// v %f{1}
void lib::gpath_builder::vertical_lineto(bool abs, arg_iterator it) {
	m_cmd = abs?'V':'v';
	m_y = *it++;
}

// s %f{4}
void lib::gpath_builder::smooth_curveto(bool abs, arg_iterator it) {
	m_cmd = abs?'S':'s';
	m_x2 = *it++;
	m_y2 = *it++;
	m_x = *it++;
	m_y = *it++;
}

// t %f{2}
void lib::gpath_builder::truetype_quadratic_bezier_curveto(bool abs, arg_iterator it) {
	m_cmd = abs?'T':'t';
	m_x = *it++;
	m_y = *it++;
}

/////////////////////////
// polyline_builder implementation

// z
void lib::polyline_builder::close_path(bool abs, arg_iterator it) {
	gpath_builder::close_path(abs, it);
	m_cpx = m_mx; m_cpy = m_my;
	AM_DBG plogger->debug("%c [CP: %.0f, %.0f]", m_cmd, m_cpx, m_cpy);
}

// m %f{2}
void lib::polyline_builder::moveto(bool abs, arg_iterator it) {
	gpath_builder::moveto(abs, it);
	if(abs) {m_cpx = m_x; m_cpy = m_y;}
	else {m_cpx += m_x; m_cpy += m_y;}
	m_mx = m_cpx; m_my = m_cpy;
	AM_DBG plogger->debug("%c %.0f %.0f [CP: %.0f, %.0f]", m_cmd, m_x, m_y, m_cpx, m_cpy);
}

// l %f{2}
void lib::polyline_builder::lineto(bool abs, arg_iterator it) {
	gpath_builder::lineto(abs, it);
	if(abs) {m_cpx = m_x; m_cpy = m_y;}
	else {m_cpx += m_x; m_cpy += m_y;}
	AM_DBG plogger->debug("%c %.0f %.0f [CP: %.0f, %.0f]", m_cmd, m_x, m_y, m_cpx, m_cpy);
}

// c %f{6}
void lib::polyline_builder::curveto(bool abs, arg_iterator it) {
	gpath_builder::curveto(abs, it);
	if(abs) {m_cpx = m_x; m_cpy = m_y;}
	else {m_cpx += m_x; m_cpy += m_y;}
	AM_DBG plogger->debug("%c %.0f %.0f %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]", m_cmd, m_x1, m_y1, m_x2, m_y2, m_x, m_y, m_cpx, m_cpy);
}

// q %f{4}
void lib::polyline_builder::quadratic_bezier_curveto(bool abs, arg_iterator it) {
	gpath_builder::quadratic_bezier_curveto(abs, it);
	if(abs) {m_cpx = m_x; m_cpy = m_y;}
	else {m_cpx += m_x; m_cpy += m_y;}
	AM_DBG plogger->debug("%c %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]", m_cmd, m_x1, m_y1, m_x, m_y, m_cpx, m_cpy);
}

// a %f{7}
void lib::polyline_builder::elliptic_arc(bool abs, arg_iterator it) {
	gpath_builder::elliptic_arc(abs, it);
	if(abs) {m_cpx = m_x; m_cpy = m_y;}
	else {m_cpx += m_x; m_cpy += m_y;}
	AM_DBG plogger->debug("%c %.0f %.0f %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]",
		m_cmd,  m_r1, m_r2, m_arc_flag, m_sweep_flag, m_x, m_y, m_cpx, m_cpy);
}

// h %f{1}
void lib::polyline_builder::horizontal_lineto(bool abs, arg_iterator it) {
	gpath_builder::horizontal_lineto(abs, it);
	if(abs) {m_cpx = m_x;}
	else {m_cpx += m_x;}
	AM_DBG plogger->debug("%c %.0f [CP: %.0f, %.0f]", m_cmd, m_x, m_cpx, m_cpy);
}

// v %f{1}
void lib::polyline_builder::vertical_lineto(bool abs, arg_iterator it) {
	gpath_builder::vertical_lineto(abs, it);
	if(abs) {m_cpy = m_y;}
	else {m_cpy += m_y;}
	AM_DBG plogger->debug("%c %.0f [CP: %.0f, %.0f]", m_cmd, m_y, m_cpx, m_cpy);
}

// s %f{4}
void lib::polyline_builder::smooth_curveto(bool abs, arg_iterator it) {
	gpath_builder::smooth_curveto(abs, it);
	if(abs) {m_cpx = m_x; m_cpy = m_y;}
	else {m_cpx += m_x; m_cpy += m_y;}
	AM_DBG plogger->debug("%c %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]", m_cmd, m_x2, m_y2, m_x, m_y, m_cpx, m_cpy);
}

// t %f{2}
void lib::polyline_builder::truetype_quadratic_bezier_curveto(bool abs, arg_iterator it) {
	gpath_builder::truetype_quadratic_bezier_curveto(abs, it);
	if(abs) {m_cpx = m_x; m_cpy = m_y;}
	else {m_cpx += m_x; m_cpy += m_y;}
	AM_DBG plogger->debug("%c %.0f %.0f %.0f %.0f [CP: %.0f, %.0f]", m_cmd, m_x, m_y, m_cpx, m_cpy);
}

// Returns the pivot points of the path as a set of polylines
void lib::polyline_builder::build_polyline_paths(const lib::gpath_descr* pd, std::vector<gpath*>& paths) {
	reset();
	const std::string& cmds = pd->get_cmds();
	const std::list<double>& args = pd->get_args();
	std::string::const_iterator curr_cmd_it = cmds.begin();

	// Record the current point before the first not M|m command
	// Stop at the following M|m command
	bool started = false;
	std::vector<lib::point> points;

	arg_iterator curr_arg_it = args.begin();
	while(curr_cmd_it != cmds.end()) {
		char cmd = *curr_cmd_it;
		std::string::size_type ix = gpath_seg_cmds.find(cmd);
		path_seg_handler_t handler = path_seg_hanlders[ix/2];
		if(started && (cmd == 'm' || cmd == 'M')) {
			paths.push_back(new polyline_path(points));
			points.clear();
			started = false;
		}
		if(!started && cmd != 'm' && cmd != 'M') {
			started = true;
			points.push_back(lib::point(int(floor(m_cpx+0.5)), int(floor(m_cpy+0.5))));
		}
		(this->*handler)((ix%2==0), curr_arg_it);
		if(started)
			points.push_back(lib::point(int(floor(m_cpx+0.5)), int(floor(m_cpy+0.5))));
		curr_cmd_it++;
		size_t nargs = lib::gpath_seg_cmds_size[ix];
		while(nargs>0) {curr_arg_it++;nargs--;}
	}
	if(started && points.size()>0) {
		paths.push_back(new polyline_path(points));
	}
}

// This builder returns the pivot points of the first connected part of the path as a polyline
lib::gpath* lib::polyline_builder::get_builded_path(const gpath_descr* pd) {
	std::vector<gpath*> paths;
	build_polyline_paths(pd, paths);
	if(paths.size() == 0) return 0;
	std::vector<lib::gpath*>::iterator it = paths.begin();
	gpath *first_path = *it++;
	for(;it!=paths.end();it++) delete *it;
	return first_path;
}


/////////////////////////
// polyline_path implementation
// The simplest possible path
// A path consisting of a sequence of connected line segments

lib::polyline_path::polyline_path(const std::vector<lib::point>& points) {
	if(points.size()== 0) {
		m_curve[0] = point(0, 0);
		return;
	}
	double s = 0;
	std::vector<lib::point>::const_iterator it = points.begin();
	lib::point cp = *it++;
	m_curve[0] = cp;
	for(;it!=points.end();it++) {
		lib::point np = *it;
		double dx = np.x-cp.x;
		double dy = np.y-cp.y;
		s += sqrt(dx*dx+dy*dy);
		m_curve[s] = np;
		cp = np;
	}
}

double lib::polyline_path::length() const {
	return (*m_curve.rbegin()).first;
}

lib::basic_point<int> lib::polyline_path::at(double s) const {
	if(s<=0) return (*m_curve.begin()).second;
	if(s>=length()) return (*m_curve.rbegin()).second;
	map_type::const_iterator eit = m_curve.upper_bound(s);
	map_type::const_iterator bit = eit;bit--;
	if(eit == m_curve.end()) return (*bit).second;
	double ds = s - (*bit).first;
	double d = (*eit).first - (*bit).first;
	lib::point v1 = (*bit).second;
	lib::point v2 = (*eit).second;
	double x = v1.x + ((v2.x-v1.x)*ds)/d;
	double y = v1.y + ((v2.y-v1.y)*ds)/d;
	return lib::point(int(floor(x+0.5)), int(floor(y+0.5)));
}

void lib::polyline_path::translate(const lib::point& pt) {
	map_type::iterator it;
	for(it = m_curve.begin();it!=m_curve.end();it++)
		(*it).second += pt;
}

void lib::polyline_path::get_pivot_points(std::vector<lib::point>& v) {
	map_type::iterator it;
	for(it = m_curve.begin();it!=m_curve.end();it++)
		v.push_back((*it).second);
}
