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

#ifndef AMBULANT_LIB_PARSELETS_H
#define AMBULANT_LIB_PARSELETS_H

#include "ambulant/config/config.h"

// used by nfa_p
// support for nfa parsers may be removed
#include "ambulant/lib/nfa.h"

// used by and_p and options_p
#include <list>

// This module defines a set of simple parsers.
// All the parsers offer a common minimal interface 
// in order to simplify composition.
//
// Reserving the name "parser" for the final composition
// we call an atomic parser a "parselet".

namespace ambulant {

namespace lib {

template <typename CharType>
class basic_parselet {
  public:
 	typedef CharType char_type;
 	typedef std::basic_string<char_type> string_type;
	typedef typename string_type::size_type size_type;   
	typedef typename string_type::iterator iterator;
	typedef typename string_type::const_iterator const_iterator;
	struct empty {};
	
	virtual ~basic_parselet() {}
	virtual std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) = 0;
	
	bool matches(const string_type& s) const {
		return parse(s.begin(), s.end()) == s.length();
	}
};

typedef basic_parselet<char> parselet;


//////////////////
// Generic parselets

template<char ch>
class literal_p : public parselet {
  public:
	typedef literal_p<ch> self_type;
	typedef char_type result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		return (it == end || *it != ch)? -1 : (m_result = *it++, 1); 
	}
};

template<char ch1, char ch2>
class range_p : public parselet {
  public:
	typedef range_p<ch1, ch2> self_type;
	typedef char_type result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		return (it == end || *it < ch1 || *it > ch2)?-1:(m_result = *it++, 1);
	}
};

class literal_cstr_p : public parselet {
   public:
	typedef literal_cstr_p self_type;
	typedef string_type result_type;
	result_type m_result;
	
	const char *m_psz;
	literal_cstr_p(const char *psz) : m_psz(psz) {}
	
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		const_iterator test_it = it;
		std::ptrdiff_t d = 0;
		const char *p = m_psz;
		while(test_it != end && *p && *test_it == *p) 
			{m_result+=*test_it;test_it++; p++; d++;}
		return *p?-1:(it = test_it, d);
	}
};

class int_p : public parselet {
   public:
	typedef int_p self_type;
	typedef int result_type;
	result_type m_result;

	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		if(it == end || *it < '0' || *it > '9') return -1; 	
		m_result = 0;
		std::ptrdiff_t len = 0;
		do {
			m_result = m_result*10 + int(*it - '0');
			len++;it++;
		} while(it != end && *it >= '0' && *it <= '9');
		return len;
	}
};

class epsilon_p : public parselet {
  public:
	typedef epsilon_p self_type;
	typedef empty result_type;
	std::ptrdiff_t parse(const_iterator& it, 
		const const_iterator& end) { return 0;}
	result_type m_result;
};

class nfa_p : public parselet {
  public:
	typedef nfa_p self_type;
	typedef string_type result_type;
	
	nfa_p(const lib::nfa_expr& expr) 
	:	m_expr(expr) {}
	
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		const_iterator it_test = it;
		std::ptrdiff_t d = m_expr.parse(it_test, end);
		if(d>=0) {
			m_result = std::string(it, it_test);
			it = it_test;
		}
		return d;
	}
	result_type m_result;
	const lib::nfa_expr& m_expr;
};

class delimiter_p : public parselet {
   public:
	typedef delimiter_p self_type;
	typedef char_type result_type;
	result_type m_result;
	string_type m_delims;
	
	delimiter_p(const string_type& delims)
	:	m_delims(delims) {}
	
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		if(it == end) return -1;
		size_type ix = m_delims.find_first_of(*it);
		if(ix != string_type::npos) {
			m_result = *it;
			it++;
			return 1;
		}
		return -1;
	}
};

template<typename FirstType, typename SecondType>
struct cat_pair_p  : public parselet {
	typedef FirstType first_type;
	typedef SecondType second_type;
	typedef cat_pair_p<FirstType, SecondType> self_type;
	typedef std::pair<typename FirstType::result_type,
		typename SecondType::result_type> result_type;

	cat_pair_p(const first_type& f, const second_type& s) : m_first(f), m_second(s) {}
	
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		const_iterator it_test = it;
		std::ptrdiff_t fd = m_first.parse(it_test, end);
		if(fd == -1) return -1;
		std::ptrdiff_t sd = m_second.parse(it_test, end);
		if(sd == -1) return -1;
		it = it_test;
		m_result = std::make_pair(m_first.m_result, m_second.m_result);
		return fd + sd;
	}
	first_type m_first;
	second_type m_second;
	result_type m_result;
};

template<typename FirstType, typename SecondType>
struct or_pair_p : public parselet {
	typedef FirstType first_type;
	typedef SecondType second_type;
	typedef or_pair_p<FirstType, SecondType> self_type;
	typedef typename FirstType::result_type first_result_type;
	typedef typename SecondType::result_type second_result_type;
	typedef std::pair< 
		std::pair<bool, first_result_type>, 
		std::pair<bool, second_result_type>
		> result_type; 
	result_type m_result;
	
	first_type m_first;
	second_type m_second;	
	or_pair_p(const first_type& f, const second_type& s) : m_first(f), m_second(s) {}
	
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		const_iterator fit = it;
		std::ptrdiff_t fd = m_first.parse(fit, end);		
		
		const_iterator sit = it;
		std::ptrdiff_t sd = m_second.parse(sit, end);
		
		if( (fd < 0 && sd >= 0) || (fd >= 0 && sd >= 0 && fd<sd)) {
			it = sit;
			m_result.first = std::make_pair(false, m_first.m_result);
			m_result.second = std::make_pair(true, m_second.m_result);
			return sd;
		} else if( (fd >= 0 && sd < 0) || (fd >= 0 && sd >= 0 && fd>=sd)) {
			it = fit;
			m_result.first = std::make_pair(true, m_first.m_result);
			m_result.second = std::make_pair(false, m_second.m_result);
			return fd;
		} 
		return -1;
	}
};

template<typename FirstType, typename SecondType>
cat_pair_p<FirstType, SecondType> cat_p(const FirstType& f, const SecondType& s) {
	return cat_pair_p<FirstType, SecondType>(f, s);
}
template<typename FirstType, typename SecondType>
or_pair_p<FirstType, SecondType> or_p(const FirstType& f, const SecondType& s) {
	return or_pair_p<FirstType, SecondType>(f, s);
}

template<typename P>
or_pair_p<P, epsilon_p> optional(const P& p) {
	return or_p(p, epsilon_p());
}

////////////////////////////////
// Composite parselets
// More as a pattern than for real use

class and_p : public parselet {	
  public:
	typedef and_p self_type;
	std::list<parselet*> m_rules;
	~and_p();
	void push_back(parselet* p) { m_rules.push_back(p);}
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
};

class options_p : public parselet {
  public:
	std::list<parselet*> m_options;
	typedef parselet* result_type;
	result_type m_result;
	~options_p();
	void push_back(parselet* p) { m_options.push_back(p);}
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
};

////////////////////////////////
// Domain parselets

// time_unit ::= "h" | "min" | "s" | "ms"
class time_unit_p : public parselet {
  public:
	typedef time_unit_p self_type;
	typedef enum {tu_h, tu_min, tu_s, tu_ms} result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
};

class full_clock_value_p : public parselet {
  public:
	typedef full_clock_value_p self_type;
	typedef struct {
		int hours;
		int minutes;
		int seconds;
		int fraction;
	}  result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_PARSELETS_H
