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

// std::runtime_error
#include <stdexcept>

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
	bool operator== (region_dim_spec& other) const {
		return left==other.left && width==other.width && right==other.right
		    && top == other.top && height==other.height && bottom==other.bottom;
	}
	
	bool operator!= (region_dim_spec& other) const { return !(*this == other); }
};

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
