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

//////////////////////////////////////
// Defines basic SVG path related objects
//////////////////////////////////////

 
#ifndef AMBULANT_LIB_GPATHS_H
#define AMBULANT_LIB_GPATHS_H

#include "ambulant/config/config.h"
#include <string>
#include <list>

namespace ambulant {

namespace lib {

// An svgpath_descr can be used both for aproximating the path for animations
// and for SVG drawing 

// The svgpath_descr code uses a number parselet for arg parsing
// This can be extented for using expotential notation etc.

// The allowed SVG commands signature (20 commands)
const std::string path_seg_cmds = "ZzMmLlCcQqAaHhVvSsTt";

// The number of the expected arguments for each command
const size_t path_seg_cmds_size[] = {0, 0, 2, 2, 2, 2, 6, 6, 4, 4, 7, 7, 1, 1, 4, 4, 2, 2};

// The number of the SVG commands (20 commands)
const size_t path_seg_cmds_count = 20;

// The number of relative SVG commands (10 commands)
const size_t path_seg_cmds_count_rel = 10;

class path_descr {
  public:
	path_descr(const std::string& s);
	
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

class logger;

class path_builder {
  public:
	path_builder(const path_descr* pd);
	void build_path();
	
  protected:
	typedef std::list<double>::const_iterator arg_iterator;
	
	// The following may be overriden by specific path builders
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
	
	// The current point (init to 0, 0)
	// Each path command appends a segment and updates the current point 
	double m_cpx, m_cpy;
	
	
	// The last move point (init to 0, 0)
	double m_mx, m_my;
	
	// The class logger
	static lib::logger *plogger;
	
  private:
	const path_descr* m_pd;

	typedef void (path_builder::*path_seg_handler_t)(bool abs, arg_iterator it);

	// The segments handlers table 
	static path_seg_handler_t path_seg_hanlders[];
};

  
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_GPATHS_H
