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

#ifndef AMBULANT_COMMON_REGION_EVAL_H
#define AMBULANT_COMMON_REGION_EVAL_H

#include "ambulant/config/config.h"

// should go to config.h
#ifdef min
#undef min
#endif

#include "ambulant/config/config.h"
#   if __GNUC__ == 2 && __GNUC_MINOR__ <= 97
#include "ambulant/compat/limits"
#else
#include <limits>
#endif
#include <cassert>
#include "ambulant/lib/gtypes.h"
#include "ambulant/common/region_dim.h"

namespace ambulant {

namespace common {

using namespace lib;

namespace detail {
// const representing auto
const int auto_int = std::numeric_limits<int>::min();
}

// A simple utility for evaluating regions positioning attributes.
// This utility may be used as a helper while building the layout tree. 
// This utility is just a calculator and its not appropriate for holding region pos attrs.  

class region_evaluator {

  public:
	// argument w and h are the width and height of the parent region in pixels
	// e.g parent.get_width(), parent.get_height()
	region_evaluator(int w, int h) : m_refw(w), m_refh(h) {
		reset();
	}
	
	// copy constructor
	region_evaluator(const region_evaluator& re) 
	:	m_refw(re.get_ref_width()), m_refh(re.get_ref_height()) {
		reset();
	}
	
	void reset() {
		m_horz[0] = m_horz[1] = m_horz[2] = detail::auto_int;
		m_vert[0] = m_vert[1] = m_vert[2] = detail::auto_int;
		m_eval = false;
	}
	
	// set defined attributes from region dimensions spec
	void set(const region_dim_spec& rds) {
		set_left(rds.left);set_width(rds.width);set_right(rds.right);
		set_top(rds.top);set_height(rds.height);set_bottom(rds.bottom);
	}
	
	// set defined attributes from region dimensions array
	void set(const region_dim *prd) {
		set_horz(prd[0], 0); set_horz(prd[1], 1); set_horz(prd[2], 2);
		set_vert(prd[3], 0); set_vert(prd[4], 1); set_vert(prd[5], 2);
	}
	
	// set defined attributes from region dimensions
	void set_left(const region_dim& rd) { set_horz(rd, 0);}
	void set_width(const region_dim& rd) { set_horz(rd, 1);}
	void set_right(const region_dim& rd) { set_horz(rd, 2);}
	
	void set_top(const region_dim& rd) { set_vert(rd, 0);}
	void set_height(const region_dim& rd) { set_vert(rd, 1);}
	void set_bottom(const region_dim& rd) { set_vert(rd, 2);}
	
	// set defined attributes in pixels
	void set_left(int v) { m_horz[0] = v;}
	void set_width(int v) { m_horz[1] = v;}
	void set_right(int v) { m_horz[2] = v;}
	
	void set_top(int v) { m_vert[0] = v;}
	void set_height(int v) { m_vert[1] = v;}
	void set_bottom(int v) { m_vert[2] = v;}
	
	// set defined attributes in proportions (%)
	void set_left(double p) { set_horz(p, 0);}
	void set_width(double p) { set_horz(p, 1);}
	void set_right(double p) { set_horz(p, 2);}
	
	void set_top(double p) { set_vert(p, 0);}
	void set_height(double p) { set_vert(p, 1);}
	void set_bottom(double p) { set_vert(p, 2);}
		
	// get evaluated attributes in pixels
	int get_left() { 
		if(!m_eval) evaluate();
		return m_horz[0];
	}
	int get_width() { 
		if(!m_eval) evaluate();
		return m_horz[1];
	}
	int get_right() { 
		if(!m_eval) evaluate();
		return  m_horz[2];
	}
	
	int get_top() { 
		if(!m_eval) evaluate();
		return m_vert[0];
	}
	int get_height() { 
		if(!m_eval) evaluate();
		return m_vert[1];
	}
	int get_bottom() { 
		if(!m_eval) evaluate();
		return m_vert[2];
	}
	
	int get_ref_width() const { 
		return m_refw;
	}
	
	int get_ref_height() const { 
		return m_refh;
	}
	
	
	basic_rect<int> get_rect() {
		return basic_rect<int>(basic_point<int>(get_left(), get_top()),
			basic_size<int>(get_width(), get_height()));
	}
	
	screen_rect<int> get_screen_rect() {
		return screen_rect<int>(basic_point<int>(get_left(), get_top()),
			basic_point<int>(get_left()+get_width(), get_top()+get_height()));
	}
	
	basic_point<int> get_origin() {
		return basic_point<int>(get_left(), get_top());
	}
	
	basic_size<int> get_size() {
		return basic_size<int>(get_width(), get_height());
	}
	
	// get clipped box
	// ...
	
  private:	
	bool is_defined(int v) { return v != detail::auto_int;}
	bool is_auto(int v) { return v == detail::auto_int;}
	void eval_third(int dim[], int w);
	int count_defined(int dim[]);
	void eval_linear_dim(int dim[], int w);
	void set_horz(double p, int ix) {
		m_horz[ix] = int(floor(m_refw*p + 0.5));
	}
	void set_vert(double p, int ix) {
		m_vert[ix] = int(floor(m_refh*p + 0.5));
	}
	
	void set_horz(const region_dim& rd, int i) {
		assert(i>=0 && i<3);
		if(rd.absolute()) m_horz[i] = rd.get_as_int(); 
		else if(rd.relative()) m_horz[i] = rd.get(m_refw);
		else m_horz[i] = detail::auto_int;
	}
	
	void set_vert(const region_dim& rd, int i) {
		assert(i>=0 && i<3);
		if(rd.absolute()) m_vert[i] = rd.get_as_int(); 
		else if(rd.relative()) m_vert[i] = rd.get(m_refh);
		else m_vert[i] = detail::auto_int;
	}
	
	void evaluate() {
		if(!m_eval) {
			eval_linear_dim(m_horz, m_refw);
			eval_linear_dim(m_vert, m_refh);
			m_eval = true;
		}
	}
	
	int m_horz[3];
	int m_vert[3];
	int m_refw;
	int m_refh;
	bool m_eval;
};


inline void region_evaluator::eval_third(int dim[], int w) {
	if(is_auto(dim[0])) dim[0] = w - dim[1] - dim[2];
	else if(is_auto(dim[1])) dim[1] = w - dim[0] - dim[2];
	else if(is_auto(dim[2])) dim[2] = w - dim[0] - dim[1];
}

inline int region_evaluator::count_defined(int dim[]) {
	int nd = 0;
	if(is_defined(dim[0])) nd++;
	if(is_defined(dim[1])) nd++;
	if(is_defined(dim[2])) nd++;
	return nd;
}

inline void region_evaluator::eval_linear_dim(int dim[], int w) {
	int nd = count_defined(dim);
	switch(nd) {
		case 0:
			dim[0] = dim[2] = 0;
			dim[1] = w;
			return;
		case 2:
			eval_third(dim, w);
			return;
		case 3:
			dim[2] = w - dim[0] - dim[1];
			return;
	}
	assert(nd == 1);
	if(is_auto(dim[0])) {
		dim[0] = 0;
		eval_third(dim, w);
	} else {
		dim[2] = 0;
		dim[1] = w - dim[0];
	}
}
 
} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_REGION_EVAL_H
