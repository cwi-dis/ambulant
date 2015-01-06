/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

//////////////////////////////////////
// Defines basic SVG path related objects
//
// basic usage pattern:
//
//  std::string pathspec = "m0 0  L50 0 L -50 50 L 0 50 Z";
//  // parse the path spec
//  lib::gpath_descr pd(pathspec);
//
//  // build a representation of the path
// 	lib::xxx_path_builder builder;
//	lib::gpath *path = builder.build_path(&pd);
//
//  // ... use the path
//  path->length();
//  lib::point pt1 = path->at(1.0);
//  lib::point pt2 = path->at(2.0);
//  path->translate(lib::point(0,30));
//
//	// cleanup
// delete path;
//////////////////////////////////////


#ifndef AMBULANT_LIB_GPATHS_H
#define AMBULANT_LIB_GPATHS_H

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include <string>
#include <list>
#include <vector>
#include <map>

namespace ambulant {

namespace lib {

// An svgpath_descr can be used both for aproximating the path for animations
// and for SVG drawing

// The svgpath_descr code uses a number parselet for arg parsing
// This can be extented for using expotential notation etc.

// The allowed SVG commands signature (20 commands)
const std::string gpath_seg_cmds = "ZzMmLlCcQqAaHhVvSsTt";

// The number of the expected arguments for each command
const size_t gpath_seg_cmds_size[] = {0, 0, 2, 2, 2, 2, 6, 6, 4, 4, 7, 7, 1, 1, 4, 4, 2, 2};

// The number of the SVG commands (20 commands)
const size_t gpath_seg_cmds_count = 20;

// The number of relative SVG commands (10 commands)
const size_t gpath_seg_cmds_count_rel = 10;

class logger;

class gpath_descr {
  public:
	gpath_descr(const std::string& s);

	const std::string& get_cmds() const { return m_cmds;}
	const std::list<double>& get_args() const { return m_args;}

	bool is_valid() const { return m_errors == 0;}
	size_t get_errors_size() const { return m_errors;}

  private:
	// The SVG path to be parsed
	std::string m_strpath;

	// The SVG path segments commands
	std::string m_cmds;

	// The parsed SVG path segments arguments
	// for each cmd push back into this list
	// as many numbers as specified in svgpath_cmd_args[cmd_index]
	// e.g. args.size() == sum(cmds.count(c[i])*svgpath_cmd_args[i] for i=0, 19
	std::list<double> m_args;

	// The number of errors seen during parsing
	size_t m_errors;
};


// The parametric interface of a path
// Returns a point for each parameter value s in [0, path::length())
class gpath {
  public:
	virtual ~gpath(){}

	// Returns the length of the path
	virtual double length() const = 0;

	// Returns the point of the path at length s
	// for s <= 0 returns path::at(0)
	// for s >= length() returns path::at(length())
	virtual lib::point at(double s) const = 0;

	// Translates the path by (pt.x, pt.y)
	virtual void translate(const lib::point& pt) = 0;

	// Returns the pivot points of this path
	// The points returned may be used as a first linear aproximation of the path
	virtual void get_pivot_points(std::vector<lib::point>& v) = 0;
};


// Abstract base path builder
class gpath_builder {
  public:
	gpath_builder();
	virtual ~gpath_builder(){}

	// Builds and returns the path described by gpath_descr
	gpath *build_path(const gpath_descr* pd);

  protected:
	typedef std::list<double>::const_iterator arg_iterator;

	// The following should be overriden by specific path builders
	virtual void close_path(bool abs, arg_iterator it); // z
	virtual void moveto(bool abs, arg_iterator it); // m
	virtual void lineto(bool abs, arg_iterator it); // l
	virtual void curveto(bool abs, arg_iterator it); // c
	virtual void quadratic_bezier_curveto(bool abs, arg_iterator it); // q
	virtual void elliptic_arc(bool abs, arg_iterator it); // a
	virtual void horizontal_lineto(bool abs, arg_iterator it); // h
	virtual void vertical_lineto(bool abs, arg_iterator it); // v
	virtual void smooth_curveto(bool abs, arg_iterator it); // s
	virtual void truetype_quadratic_bezier_curveto(bool abs, arg_iterator it); // t

	// Returns the path constructed by the derived gpath_builder
	virtual gpath *get_builded_path(const gpath_descr* pd) = 0;

	// Resets state for re-scanning
	virtual void reset();

	// The current point (init to 0, 0)
	// Each path command appends a segment and updates the current point
	double m_cpx, m_cpy;

	// The last move point (init to 0, 0)
	double m_mx, m_my;

	// A path segment register
	char m_cmd;
	double m_x1, m_y1;
	double m_x2, m_y2;
	double m_x, m_y;
	double m_r1, m_r2;
	double m_angle;
	double m_arc_flag;
	double m_sweep_flag;

	// The class logger
	static logger *plogger;

	typedef void (gpath_builder::*path_seg_handler_t)(bool abs, arg_iterator it);

	// The segments handlers table
	static path_seg_handler_t path_seg_hanlders[];
};


class polyline_path : public gpath {
  public:
	polyline_path(const std::vector<point>& points);
	virtual ~polyline_path() {}

	// gpath interface
	double length() const;
	point at(double s) const;
	void translate(const point& pt);
	void get_pivot_points(std::vector<lib::point>& v);
  private:
	typedef std::map<double, lib::point> map_type;
	map_type m_curve;
};


// A simple path builder that uses polyline paths
class polyline_builder : public gpath_builder {
  protected:
	// The following are overriden by this gpath_builder
	virtual void close_path(bool abs, arg_iterator it); // z
	virtual void moveto(bool abs, arg_iterator it); // m
	virtual void lineto(bool abs, arg_iterator it); // l
	virtual void curveto(bool abs, arg_iterator it); // c
	virtual void quadratic_bezier_curveto(bool abs, arg_iterator it); // q
	virtual void elliptic_arc(bool abs, arg_iterator it); // a
	virtual void horizontal_lineto(bool abs, arg_iterator it); // h
	virtual void vertical_lineto(bool abs, arg_iterator it); // v
	virtual void smooth_curveto(bool abs, arg_iterator it); // s
	virtual void truetype_quadratic_bezier_curveto(bool abs, arg_iterator it); // t

	// Returns the path constructed by this gpath_builder
	virtual gpath *get_builded_path(const gpath_descr* pd);

	// Builds and returns the pivot points of the path as a set of polylines
	void build_polyline_paths(const gpath_descr* pd, std::vector<gpath*>& paths);
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_GPATHS_H
