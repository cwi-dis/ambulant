
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_REGION_H
#define AMBULANT_LIB_REGION_H

namespace ambulant {

namespace lib {

class active_region;

class passive_region {
  public:
	passive_region() {}
	~passive_region() {}
	
	active_region *activate();
};

class active_region {
  public:
	active_region() {}
	~active_region() {}
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REGION_H
