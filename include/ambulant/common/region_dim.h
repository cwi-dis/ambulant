
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

#ifndef AMBULANT_LIB_REGION_DIM_H
#define AMBULANT_LIB_REGION_DIM_H

// std::runtime_error
#include <stdexcept>

// std::ostream for debug output
#include <ostream>

// floor
#include <math.h>

namespace ambulant {

namespace lib {

class region_dim {
  public:
  
	//////////////////////
	// region_dim constructors
	
	// default constructor
	// constructs an auto dim
	region_dim()
	:	m_type(rdt_auto), m_holder(0) {}
    
	// constructs an absolute dim (assumed in pixels)
    region_dim(int value)
    :	m_type(rdt_absolute), m_holder(new region_dim_holder_t<int>(value)){}
	
	// constructs a relative dim (proportion or percent)
    region_dim(double value)
    :	m_type(rdt_relative), m_holder(new region_dim_holder_t<double>(value)){}
	
	// constructs a region dim from an other dim
    region_dim(const region_dim& other)
    :	m_type(other.m_type), m_holder(other.m_holder?other.m_holder->clone():0) {}
     
     
	//////////////////////
	// region_dim destructor
	
    ~region_dim(){ if(m_holder != 0) delete m_holder;}
    
	//////////////////////
	// region_dim assignments (construct from existing)
	
	// sets this to other
    const region_dim& operator=(const region_dim& other) { 
		if(&other != this) {
			if(m_holder != 0) delete m_holder;
			m_type = other.m_type;
			m_holder = other.m_holder?other.m_holder->clone():0;
		}
		return *this;
    }
    
	// sets this to the absolute value provided
    const region_dim& operator=(int value) { 
		m_type = rdt_absolute;
		if(m_holder != 0) delete m_holder;
		m_holder = new region_dim_holder_t<int>(value);
		return *this;
    }
    
	// sets this to the relative value provided
    const region_dim& operator=(double value) { 
		m_type = rdt_relative;
		if(m_holder != 0) delete m_holder;
		m_holder = new region_dim_holder_t<double>(value);
		return *this;
    }
   
	//////////////////////
	// type queries
	
	bool relative() const { return m_type == rdt_relative;}
	bool absolute() const { return m_type == rdt_absolute;}
	bool defined() const { return m_type != rdt_auto;}
	bool isauto() const { return m_type == rdt_auto;}
	
	// Value getter function
	// throws std::runtime_error on illegal template argument.
	template<class T>
	T get() const {
		if(m_holder == 0)
			throw std::runtime_error("Illegal call. Region dim is undefined");
		region_dim_holder_t<T> *holder =
			(dynamic_cast< region_dim_holder_t<T>* >(m_holder));
		if(holder == 0)
			throw std::runtime_error("Illegal template argument");
		return holder->m_value; 
	}
	
	int get(int ref) const {
		switch(m_type) {
			case rdt_absolute: return get<int>();
			case rdt_relative: return int(floor(ref*get<double>() + 0.5));
		}
		throw std::runtime_error("Illegal call. Region dim is undefined");
	}
	
	// debug print out
	friend std::ostream& operator<<(std::ostream& os, const region_dim& rd) { 
		if(rd.relative())
			return os << rd.get<double>();
		else if(rd.absolute())
			return os << rd.get<int>();
		return os <<  "<auto>";
	}
	
  private: 
	
	// region_dim_holder base  
	struct region_dim_holder {
		virtual ~region_dim_holder() {}
		virtual region_dim_holder* clone() = 0;
	};
	
	// region_dim_holder
	template<class T>
	struct region_dim_holder_t : public region_dim_holder {
		region_dim_holder_t(const T& value) : m_value(value) {}
		virtual region_dim_holder* clone() {
			return new region_dim_holder_t<T>(m_value);
		}
		T m_value;
	};
	
	// region dimension types
	enum region_dim_type {rdt_auto, rdt_relative, rdt_absolute};
	
	// region dimension data
	region_dim_type m_type;
	region_dim_holder *m_holder;
};

// A structure holding all layout attributes of a region
// A region node may hold along its other attributes this data structure.
struct region_dim_spec {
	region_dim left, width, right;
	region_dim top, height, bottom;
};

} // namespace lib
 
} // namespace ambulant

// debug region_dim_spec printout
inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::region_dim_spec& rds) { 
	os << '('  << rds.left << ", " << rds.width  << ", "  << rds.right;
	os << ", " << rds.top  << ", " << rds.height << ", "  << rds.bottom;
	return os << ')';
}

#endif // AMBULANT_LIB_REGION_DIM_H
