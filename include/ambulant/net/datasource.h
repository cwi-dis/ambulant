/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
 
// XXXX Notes by Jack:
// XXXX 2. We need a naming convention for formal arg/attribute matches.
// XXXX 3. Do we have a naming convention for classes that are "semi-private"
//         such as caching_information? Another way to show this semi-privateness
//         such as "friend" or so?
// XXXX 4. What is the naming convention again to distinguish between classes
//         and other things (methods, attributes)?
// XXXX 5. I have no idea what the arguments to set_consumer are, my C++ is lacking here:-(

#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

#include "ambulant/lib/callback.h"
#include "url.h"

// temporary debug messages
#include <iostream>

namespace ambulant {

namespace net {

namespace detail {

class caching_information {
	// Objects of this class store caching information
	// regarding a single object. The object is private
	// to active_datasource (and maybe passive_datasource too?).
	
	// ... to be provided
};

} // namespace detail

// forward declaration.
class passive_datasource;

class active_datasource {
  public:
	active_datasource(passive_datasource *creator) {
		std::cout << "active_datasource" << std::endl;
	}
	~active_datasource() {
		std::cout << "~active_datasource" << std::endl;
	}
	
	void set_consumer(/* ... */) {}
	void more() {}
	
};

// Not sure we need this.
class timed_active_datasource : public active_datasource {
  public:
	typedef int position; // Not good, need better type for this
	
	void set_position(position pos);
	position get_position();
};
	
class passive_datasource {
	url m_url;
	active_datasource *m_prefetched;
	detail::caching_information *m_cacheinfo;
	
  public:
    passive_datasource(url the_url)
    :	m_url(the_url),
    	m_prefetched(NULL),
    	m_cacheinfo(NULL) {
		std::cout << "passive_datasource" << std::endl;
    }
    // ... more constructors to follow
    
    ~passive_datasource() {
		delete m_cacheinfo;
		std::cout << "~passive_datasource" << std::endl;
    }
    
    void start_prefetch() {}
    // ... more arglists to follow
    
    void prefetch() {}
    
    active_datasource *activate() {
    	if (!m_prefetched)
    		prefetch();
    	active_datasource *rv = m_prefetched;
    	m_prefetched = NULL;
    	return rv;
    }
    
    // ... need methods to set/get caching information
    
};

} // namespace net
 
} // namespace ambulant

#endif // AMBULANT_NET_DATASOURCE_H

