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

/////////////////////////////
// region_dim
//
// A representation for a region dimension.
//
// The respresentation implemented is capable to
// represent auto, absolute or relative coordinates.
//
// Objects of this class may be used to permanently hold
// the dimensions of a region (as they are specified in the 
// original document or by the animated one).
// Conversions to absolute coordinates should be done
// only dynamically and as late as possible.
// An entity having access to the whole layout tree
// (animated or not) should be responsible for all 
// on the fly conversions.
//
// For simplicity we assume that all absolute coordinates
// have been converted to uniform units.
/////////////////////////////

#ifndef AMBULANT_COMMON_REGION_DIM_H
#define AMBULANT_COMMON_REGION_DIM_H

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"

// std::runtime_error
#include <stdexcept>

// floor
#include <math.h>

namespace ambulant {

namespace common {

using namespace lib;

class region_dim {
  
	union dim_value_holder_t {
		int int_val;
		double dbl_val;
	};
	
  public:
	//////////////////////
	// region_dim constructors
	
	// default constructor
	// constructs an auto dim
	region_dim()
	:	m_type(rdt_auto) { m_holder.dbl_val = 0;}
    
	// constructs an absolute dim (assumed in pixels)
    region_dim(int value)
    :	m_type(rdt_absolute) { m_holder.int_val = value;}
	
	// constructs a relative dim (proportion or percent)
    region_dim(double value)
    :	m_type(rdt_relative) { m_holder.dbl_val = value;}
	
	// constructs a region dim from an other dim
    region_dim(const region_dim& other)
    :	m_type(other.m_type) {
		if(other.absolute())
			m_holder.int_val = other.get_as_int();
		else if(other.relative())
 			m_holder.dbl_val = other.get_as_dbl();
   } 
     
	// constructs a region dim from the provided str
	region_dim(const std::string& s) 
	:	m_type(rdt_auto) {
		m_holder.dbl_val = 0;
		if(s.empty()) return;	
		char *endptr;
		int ivalue = strtol(s.c_str(), &endptr, 10);
		if(*endptr == '\0' || strcmp(endptr, "px") == 0) {
			m_holder.int_val = ivalue;
			m_type = rdt_absolute;
		} else if (*endptr == '%') {
			m_holder.dbl_val = ivalue / 100.0;
			m_type = rdt_relative;
		} 
	}
     
	//////////////////////
	// region_dim destructor
	
    ~region_dim(){}
    
	//////////////////////
	// region_dim assignments (construct from existing)
	
	// sets this to other
    const region_dim& operator=(const region_dim& other) { 
		if(&other != this) {
			m_type = other.m_type;
			m_holder = other.m_holder;
		}
		return *this;
    }
    
	// sets this to the absolute value provided
    const region_dim& operator=(int value) { 
		m_type = rdt_absolute;
		m_holder.int_val = value;
		return *this;
    }
    
	// sets this to the relative value provided
    const region_dim& operator=(double value) { 
		m_type = rdt_relative;
		m_holder.dbl_val = value;
		return *this;
    }
   
	//////////////////////
	// type queries
	
	bool relative() const { return m_type == rdt_relative;}
	bool absolute() const { return m_type == rdt_absolute;}
	bool defined() const { return m_type != rdt_auto;}
	bool isauto() const { return m_type == rdt_auto;}
	
	// Value getter function
	int get_as_int() const { 
		if(absolute()) return m_holder.int_val; 
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
		throw std::runtime_error("Illegal call. Region dim is not absolute");
#else
		abort();
#endif
		return 0;
	}
	
	double get_as_dbl() const { 
		if(relative()) return m_holder.dbl_val;
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
		throw std::runtime_error("Illegal call. Region dim is not relative");
#else
		abort();
#endif
		return 0;
	}
		
	int get(int ref) const {
		switch(m_type) {
			case rdt_absolute: return get_as_int();
			case rdt_relative: return int(floor(ref*get_as_dbl() + 0.5));
			case rdt_auto: break;
		}
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
		throw std::runtime_error("Illegal call. Region dim is undefined");
#else
		abort();
		return 0;
#endif
	}
	
	bool operator== (const region_dim& other) const {
		if (m_type != other.m_type) return false;
		if (m_type == rdt_absolute) return m_holder.int_val == other.m_holder.int_val;
		if (m_type == rdt_relative) return m_holder.dbl_val == other.m_holder.dbl_val;
		return true;
	}

	bool operator!= (const region_dim& other) const { return !(*this == other); }
		
	region_dim& operator+=(const region_dim& rhs) {
		assert(m_type == rhs.m_type);
		if(absolute())
			m_holder.int_val += rhs.get_as_int();
		else if(relative())
 			m_holder.dbl_val += rhs.get_as_dbl();
		return *this;
	}
	
	region_dim& operator-=(const region_dim& rhs) {
		assert(m_type == rhs.m_type);
		if(absolute())
			m_holder.int_val -= rhs.get_as_int();
		else if(relative())
 			m_holder.dbl_val -= rhs.get_as_dbl();
		return *this;
	}
	
	region_dim operator+(const region_dim& rhs) const { region_dim t(*this); t+=rhs; return t;}
	
	region_dim operator-(const region_dim& rhs) const { region_dim t(*this); t-=rhs; return t;}
		
	// define comparisons
	bool operator<(const region_dim& rhs) const {
		if(isauto()) return true;
		return  absolute()?(m_holder.dbl_val<rhs.m_holder.dbl_val):
			m_holder.int_val<rhs.m_holder.int_val;}
	bool operator<=(const region_dim& rhs) const {
		if(isauto()) return true;
		return  absolute()?m_holder.dbl_val<= rhs.m_holder.dbl_val:
			m_holder.int_val<=rhs.m_holder.int_val;}
	bool operator>(const region_dim& rhs) const {
		if(isauto()) return true;
		return  absolute()?(m_holder.dbl_val>rhs.m_holder.dbl_val):
			m_holder.int_val>rhs.m_holder.int_val;}
	bool operator>=(const region_dim& rhs) const {
		if(isauto()) return true;
		return  absolute()?m_holder.dbl_val>=rhs.m_holder.dbl_val:
			m_holder.int_val>=rhs.m_holder.int_val;}

  private: 
	// region dimension types
	enum region_dim_type {rdt_auto, rdt_relative, rdt_absolute};
	
	// region dimension data
	region_dim_type m_type;
	dim_value_holder_t m_holder;
};

// A structure holding all layout attributes of a region
// A region node may hold along its other attributes this data structure.
struct region_dim_spec {
	region_dim left, width, right;
	region_dim top, height, bottom;
	region_dim_spec() {}
	region_dim_spec(const std::string& coords, const char *shape = 0);
	bool operator== (region_dim_spec& other) const {
		return left==other.left && width==other.width && right==other.right
		    && top == other.top && height==other.height && bottom==other.bottom;
	}
	bool operator!= (region_dim_spec& other) const { return !(*this == other); }
	void convert(const lib::screen_rect<int>& rc);
};


// Sets the region dimensions from the bounding box specified by the coords attribute
inline region_dim_spec::region_dim_spec(const std::string& coords, const char *shape) {
	if(coords.empty()) return;
	std::list<std::string> list;
	lib::split_trim_list(coords, list, ',');
	std::list<std::string>::iterator it = list.begin();
	if((!shape || (shape && strcmp(shape, "rect")==0)) && list.size() == 4) {
		left = region_dim(*it++);
		top = region_dim(*it++);
		width = region_dim(*it++) - left;
		height = region_dim(*it++) - top;
	} else if((shape && strcmp(shape, "circle")==0) && list.size() == 3) {
		region_dim x(*it++);
		region_dim y(*it++);
		region_dim r(*it++);
		left = x-r;
		top = y-r;
		width = r+r;
		height = width;
	} else if((shape && strcmp(shape, "poly")==0) && list.size() >= 6 && (list.size() % 2) == 0) {
		region_dim l, t, r, b;
		while(it!=list.end()) {
			region_dim x(*it++);
			l = std::min<region_dim>(l, x);
			r = std::max<region_dim>(r, x);
			region_dim y(*it++);
			t = std::min<region_dim>(t, y);
			b = std::max<region_dim>(b, y);
		}
		left = l;
		top = t;
		width = r-l;
		height = b-t;
	} 
}

// Converts those coordinates that are relative to absolute 
inline void region_dim_spec::convert(const lib::screen_rect<int>& rc) {
	int w = rc.width(), h = rc.height();
	
	if(!left.isauto()) left = left.get(w);
	if(!right.isauto()) right = right.get(w);
	if(!width.isauto()) width = width.get(w);
	
	if(!top.isauto()) top = top.get(h);
	if(!bottom.isauto()) bottom = bottom.get(h);
	if(!height.isauto()) height = height.get(h);
}

// A structure holding attributes of a regPoint or regAlign
// A region node may hold along its other attributes this data structure.
struct regpoint_spec {
	region_dim left;
	region_dim top;
	
	// Default constructor initializes everything to auto
	regpoint_spec() {}
	
	// Specific constructor to give percentage values
	regpoint_spec(double hor, double vert)
	:   left(hor),
		top(vert) {}
	
	bool operator== (regpoint_spec& other) const {
		return left==other.left  && top == other.top;
	}
	
	bool operator!= (regpoint_spec& other) const { return !(*this == other); }
};

} // namespace common
 
} // namespace ambulant



///////////////////////////////
#ifndef AMBULANT_NO_IOSTREAMS_HEADERS

// std::ostream for debug output
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/

#endif // AMBULANT_NO_IOSTREAMS_HEADERS
///////////////////////////////

#ifndef AMBULANT_NO_OSTREAM

// debug region_dim print out
inline std::ostream& operator<<(std::ostream& os, const ambulant::common::region_dim& rd) { 
	if(rd.relative())
		return os << rd.get_as_dbl() * 100.0 << '%' ;
	else if(rd.absolute())
		return os << rd.get_as_int();
	return os <<  "<auto>";
}

// debug region_dim_spec printout
inline std::ostream& operator<<(std::ostream& os, const ambulant::common::region_dim_spec& rds) { 
	os << '('  << rds.left << ", " << rds.width  << ", "  << rds.right;
	os << ", " << rds.top  << ", " << rds.height << ", "  << rds.bottom;
	return os << ')';
}

#endif

#endif // AMBULANT_COMMON_REGION_DIM_H
