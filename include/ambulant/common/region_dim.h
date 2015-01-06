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

#ifndef AMBULANT_COMMON_REGION_DIM_H
#define AMBULANT_COMMON_REGION_DIM_H

#include "ambulant/config/config.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/logger.h"

// std::runtime_error
#include <stdexcept>

// floor
#include <math.h>

namespace ambulant {

namespace common {

using namespace lib;

/// A representation for a region dimension.
///
/// The respresentation implemented is capable of
/// representing auto, absolute or relative coordinates.
///
/// Objects of this class may be used to permanently hold
/// the dimensions of a region (as they are specified in the
/// original document or by the animated one).
/// Conversions to absolute coordinates should be done
/// only dynamically, and as late as possible.
/// An entity having access to the whole layout tree
/// (animated or not) should be responsible for all
/// on the fly conversions.
///
/// For simplicity we assume that all absolute coordinates
/// have been converted to uniform units.
class region_dim {

	/// A type holding either an int (for absolute values) or double (for relative values).
	union dim_value_holder_t {
		int int_val;
		double dbl_val;
	};

  public:
	//////////////////////
	// region_dim constructors

	/// Default constructor,
	// constructs an auto region_dim.
	region_dim()
	:	m_type(rdt_auto) { m_holder.dbl_val = 0;}

	/// Constructs an absolute dim (assumed in pixels).
	region_dim(int value)
	:	m_type(rdt_absolute) { m_holder.int_val = value;}

	/// Constructs a relative dim (proportion or percent).
	region_dim(double value)
	:	m_type(rdt_relative) { m_holder.dbl_val = value;}

	/// Constructs a region_dim from another region_dim.
	region_dim(const region_dim& other)
	:	m_type(other.m_type) {
		if(other.absolute())
			m_holder.int_val = other.get_as_int();
		else if(other.relative())
			m_holder.dbl_val = other.get_as_dbl();
	}

	/// constructs a region_dim from the provided string.
	/// Does very little error checking.
	region_dim(const std::string& s)
	:	m_type(rdt_auto) {
		m_holder.dbl_val = 0;
		if(s.empty()) return;
		char *endptr;
		int ivalue = (int)strtol(s.c_str(), &endptr, 10);
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

	const region_dim& operator=(const region_dim& other) { ///< operator
		if(&other != this) {
			m_type = other.m_type;
			m_holder = other.m_holder;
		}
		return *this;
	}

	const region_dim& operator=(int value) { ///< operator
		m_type = rdt_absolute;
		m_holder.int_val = value;
		return *this;
	}

	const region_dim& operator=(double value) { ///< operator
		m_type = rdt_relative;
		m_holder.dbl_val = value;
		return *this;
	}

	//////////////////////
	// type queries

	/// Return true if this region_dim is relative.
	bool relative() const { return m_type == rdt_relative;}

	/// Return true if this region_dim is absolute.
	bool absolute() const { return m_type == rdt_absolute;}

	/// Return true if this region_dim is not auto.
	bool defined() const { return m_type != rdt_auto;}

	/// Return true if this region_dim is auto.
	bool isauto() const { return m_type == rdt_auto;}

	/// Get value as absolute integer (or abort).
	int get_as_int() const {
		if(absolute()) return m_holder.int_val;
		throw std::runtime_error("Illegal call. Region dim is not absolute");
		return 0;
	}

	/// Get value as relative double (or abort).
	double get_as_dbl() const {
		if(relative()) return m_holder.dbl_val;
		throw std::runtime_error("Illegal call. Region dim is not relative");
		return 0;
	}

	/// Get value as absolute int.
	/// Relative values are interpreted with respect to ref,
	/// auto values will abort.
	int get(int ref) const {
		switch(m_type) {
			case rdt_absolute: return get_as_int();
			case rdt_relative: return int(floor(ref*get_as_dbl() + 0.5));
			case rdt_auto: break;
		}
		throw std::runtime_error("Illegal call. Region dim is undefined");
	}

	bool operator== (const region_dim& other) const { ///< operator
		if (m_type != other.m_type) return false;
		if (m_type == rdt_absolute) return m_holder.int_val == other.m_holder.int_val;
		if (m_type == rdt_relative) return m_holder.dbl_val == other.m_holder.dbl_val;
		return true;
	}

	bool operator!= (const region_dim& other) const { return !(*this == other); } ///< operator

	region_dim& operator+=(const region_dim& rhs) {  ///< operator
		if(m_type != rhs.m_type)
			lib::logger::get_logger()->trace("region animation: cannot mix percentages and absolute values");
		else if(absolute())
			m_holder.int_val += rhs.get_as_int();
		else if(relative())
			m_holder.dbl_val += rhs.get_as_dbl();
		return *this;
	}

	region_dim& operator-=(const region_dim& rhs) { ///< operator
		if(m_type != rhs.m_type)
			lib::logger::get_logger()->trace("region animation: cannot mix percentages and absolute values");
		else if(absolute())
			m_holder.int_val -= rhs.get_as_int();
		else if(relative())
			m_holder.dbl_val -= rhs.get_as_dbl();
		return *this;
	}

	region_dim& operator*=(int n) { ///< operator
		if(absolute())
			m_holder.int_val *= n;
		else if(relative())
			m_holder.dbl_val *= n;
		return *this;
	}

	region_dim& operator/=(int n) { ///< operator
		if(absolute())
			m_holder.int_val /= n;
		else if(relative())
			m_holder.dbl_val /= n;
		return *this;
	}

	region_dim operator+(const region_dim& rhs) const { region_dim t(*this); t+=rhs; return t;} ///< operator

	region_dim operator-(const region_dim& rhs) const { region_dim t(*this); t-=rhs; return t;} ///< operator

	region_dim operator*(int n) const { region_dim t(*this); t*=n; return t;} ///< operator

	region_dim operator/(int n) const { region_dim t(*this); t/=n; return t;} ///< operator

	// define comparisons
	bool operator<(const region_dim& rhs) const { ///< operator
		if(isauto()) return true;
		return  absolute()?(m_holder.dbl_val<rhs.m_holder.dbl_val):
			m_holder.int_val<rhs.m_holder.int_val;}
	bool operator<=(const region_dim& rhs) const { ///< operator
		if(isauto()) return true;
		return  absolute()?m_holder.dbl_val<= rhs.m_holder.dbl_val:
			m_holder.int_val<=rhs.m_holder.int_val;}
	bool operator>(const region_dim& rhs) const { ///< operator
		if(isauto()) return true;
		return  absolute()?(m_holder.dbl_val>rhs.m_holder.dbl_val):
			m_holder.int_val>rhs.m_holder.int_val;}
	bool operator>=(const region_dim& rhs) const { ///< operator
		if(isauto()) return true;
		return  absolute()?m_holder.dbl_val>=rhs.m_holder.dbl_val:
			m_holder.int_val>=rhs.m_holder.int_val;}

    std::string repr() { ///< Represent as string.
        if (isauto()) return "auto";
        std::stringstream ss;
        if (absolute()) {
            ss << m_holder.int_val;
        } else {
            ss << m_holder.dbl_val;
            ss << "%";
        }
        return ss.str();
    }
            
  private:
	// region dimension types
	enum region_dim_type {rdt_auto, rdt_relative, rdt_absolute};

	// region dimension data
	region_dim_type m_type;
	dim_value_holder_t m_holder;
};

/// A structure holding all layout attributes of a SMIL region.
struct region_dim_spec {
	region_dim left; 	///< Constraint in pixels or percentage, or auto.
	region_dim width; 	///< Constraint in pixels or percentage, or auto.
	region_dim right; 	///< Constraint in pixels or percentage, or auto.
	region_dim top; 	///< Constraint in pixels or percentage, or auto.
	region_dim height; 	///< Constraint in pixels or percentage, or auto.
	region_dim bottom; 	///< Constraint in pixels or percentage, or auto.


	/// Default constructor, sets all values to auto.
	region_dim_spec() {}
	/// Construct as copy of another region_dim_spec.
	region_dim_spec(const region_dim_spec &other) {
		left = other.left; width = other.width; right = other.right;
		top = other.top; height = other.height; bottom = other.bottom;
	}

	/// Constructor using SMIL anchor coords string.
	/// For non-rectangular coords values this will set the region_dim_spec
	/// to the bounding box for the shape.
	region_dim_spec(const std::string& coords, const char *shape = 0);

	bool operator== (region_dim_spec& other) const { ///< operator
		return left==other.left && width==other.width && right==other.right
			&& top == other.top && height==other.height && bottom==other.bottom;
	}

	bool operator!= (region_dim_spec& other) const { return !(*this == other); } ///< operator

	/// Convert all relative parameters to absolute.
	void convert(const lib::rect& rc);
	region_dim_spec& operator+=(const region_dim_spec& other) { ///< operator
		left += other.left; right += other.right; top += other.top;
		bottom += other.bottom; width += other.width; height += other.height;
		return *this;
	}
	region_dim_spec& operator-=(const region_dim_spec& other) { ///< operator
		left -= other.left; right -= other.right; top -= other.top;
		bottom -= other.bottom; width -= other.width; height -= other.height;
		return *this;
	}
	region_dim_spec& operator*=(int t) { ///< operator
		left *= t; right *= t; top *= t;
		bottom *= t; width *= t; height *= t;
		return *this;
	}
	region_dim_spec& operator/=(int t) { ///< operator
		left /= t; right /= t; top /= t;
		bottom /= t; width /= t; height /= t;
		return *this;
	}
	region_dim_spec operator+(const region_dim_spec& other) const { ///< operator
		region_dim_spec rv(*this); rv += other; return rv;
	}
	region_dim_spec operator-(const region_dim_spec& other) const { ///< operator
		region_dim_spec rv(*this); rv -= other; return rv;
	}
	region_dim_spec operator*(int t) const { ///< operator
		region_dim_spec rv(*this); rv *= t; return rv;
	}
	region_dim_spec operator/(int t) const { ///< operator
		region_dim_spec rv(*this); rv /= t; return rv;
	}
	bool operator<(const region_dim_spec& other) const { ///< operator
		abort();
	}
};


// Sets the region dimensions from the bounding box specified by the coords attribute.
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
	} else if(((shape && strcmp(shape, "panZoomRect")==0)) && list.size() == 4) {
		left = region_dim(*it++);
		top = region_dim(*it++);
		width = region_dim(*it++);
		height = region_dim(*it++);
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
inline void region_dim_spec::convert(const lib::rect& rc) {
	int w = rc.width(), h = rc.height();

	if(!left.isauto()) left = left.get(w);
	if(!right.isauto()) right = right.get(w);
	if(!width.isauto()) width = width.get(w);

	if(!top.isauto()) top = top.get(h);
	if(!bottom.isauto()) bottom = bottom.get(h);
	if(!height.isauto()) height = height.get(h);
}

/// A structure holding attributes of a SMIL regPoint or regAlign.
/// A region node may hold along its other attributes this data structure.
struct regpoint_spec {

	region_dim left;	///< Coordinate, may be absolute or relative.
	region_dim top;	///< Coordinate, may be absolute or relative.

	/// Default constructor initializes everything to auto.
	regpoint_spec() {}

	/// Specific constructor giving percentage values.
	regpoint_spec(double hor, double vert)
	:	left(hor),
		top(vert) {}

	bool operator== (regpoint_spec& other) const { ///< operator
		return left==other.left  && top == other.top;
	}

	bool operator!= (regpoint_spec& other) const { return !(*this == other); } ///< operator
};

} // namespace common

} // namespace ambulant

#if !defined(AMBULANT_PLATFORM_WIN32)
#define sprintf_s snprintf
#endif

inline std::string repr(const ambulant::common::region_dim& rd) {
	char sz[16] = "<auto>";
	if(rd.relative()) {
		sprintf_s(sz, sizeof sz, "%d%c", int(floor(0.5+rd.get_as_dbl() * 100.0)), '%');
	} else if(rd.absolute()) {
		sprintf_s(sz, sizeof sz, "%d", rd.get_as_int());
	}
	return sz;
}

///////////////////////////////
// std::ostream for debug output
#include <ostream>


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

#endif // AMBULANT_COMMON_REGION_DIM_H
