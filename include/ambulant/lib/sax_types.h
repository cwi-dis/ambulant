
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_SAX_TYPES_H
#define AMBULANT_LIB_SAX_TYPES_H

#include <string>

// pair
#include <utility>

// list
#include <list>

namespace ambulant {

namespace lib {

typedef std::basic_string<char> xml_string;

// first is the namespace uri and second is the local name
typedef std::pair<xml_string, xml_string> q_name_pair;

// first is the qualified name of the attribute and second 
// is the value of the attribute
typedef std::pair<q_name_pair, xml_string> q_attribute_pair;

// a list of attributes
typedef std::list<q_attribute_pair> q_attributes_list;

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_SAX_TYPES_H
