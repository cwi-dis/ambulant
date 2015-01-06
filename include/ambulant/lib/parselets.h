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

#ifndef AMBULANT_LIB_PARSELETS_H
#define AMBULANT_LIB_PARSELETS_H

#include "ambulant/config/config.h"

// used by nfa_p
// support for nfa parsers may be removed
#include "ambulant/lib/nfa.h"

// used by and_p and options_p
#include <list>

// used by number_list_p
#include <vector>

#include <math.h>

// used by wallclock_p
#include <time.h>

// This module defines a set of simple parsers.
// All the parsers offer a common minimal interface
// in order to simplify composition.
//
// Reserving the name "parser" for the final composition
// we call an atomic parser a "parselet".

namespace ambulant {

namespace lib {

template <class CharType>
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

	bool matches(const string_type& s) {
		const_iterator b = s.begin(), e = s.end();
		return parse(b, e) == (std::ptrdiff_t)s.length();
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

class int_p : public parselet {
  public:
	typedef int_p self_type;
	typedef int result_type;
	result_type m_result;
    result_type m_min;
    result_type m_max;
    
    int_p (int min=INT_MIN, int max=INT_MAX) {
        m_min = min;
        m_max = max;
    }
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		if(it == end || *it < '0' || *it > '9') return -1;
		m_result = 0;
		std::ptrdiff_t len = 0;
		do {
			m_result = m_result*10 + int(*it - '0');
			len++;it++;
		} while(it != end && *it >= '0' && *it <= '9');
        if (m_result >= m_min && m_result <= m_max) {
            return len;
        }
		return -1;
	}
};

class fraction_p : public int_p {
  public:
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		std::ptrdiff_t d = int_p::parse(it, end);
		if (d < 0) return d;
		std::ptrdiff_t digits = d;
		while (digits < 3) {
			digits++;
			m_result *= 10;
		}
		while (digits > 3) {
			digits--;
			m_result /= 10;
		}
		return d;
	}
};

template <class CharType, class IsNameStartCh, class IsNameCh >
class name_p :  public basic_parselet<CharType> {
  public:
	typedef name_p<CharType, IsNameStartCh, IsNameCh> self_type;
	typedef typename basic_parselet<CharType>::string_type result_type;
	typedef typename basic_parselet<CharType>::const_iterator const_iterator;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		if(it == end || !IsNameStartCh()(*it)) return -1;
		m_result += *it;
		std::ptrdiff_t d = 1;
		const_iterator test_it = ++it;
		while(test_it != end && IsNameCh()(*test_it))
			{ m_result += *test_it++; d++;}
		return (it = test_it, d);
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
		if(d != -1) {
			m_result = std::string(it, it_test);
			it = it_test;
		}
		return d;
	}
	result_type m_result;
	const lib::nfa_expr& m_expr;
};


// Parses: FirstType SecondType
template<class FirstType, class SecondType>
class cat_pair_p  : public parselet {
  public:
	typedef FirstType first_type;
	typedef SecondType second_type;
	typedef typename FirstType::result_type first_result_type;
	typedef typename SecondType::result_type second_result_type;
	//typedef typename cat_pair_p<FirstType, SecondType> self_type;
	typedef std::pair<typename FirstType::result_type,
		typename SecondType::result_type> result_type;

	cat_pair_p(const FirstType& f, const SecondType& s) : m_first(f), m_second(s) {}

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

	first_result_type get_first_result() const { return m_result.first;}
	second_result_type get_second_result() const { return m_result.second;}

	first_type m_first;
	second_type m_second;
	result_type m_result;
};

// Parses: FirstType | SecondType
template<class FirstType, class SecondType>
class or_pair_p : public parselet {
  public:
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

		if( (fd == -1 && sd != -1) || (fd != -1 && sd != -1 && fd<sd)) {
			it = sit;
			m_result.first = std::make_pair(false, m_first.m_result);
			m_result.second = std::make_pair(true, m_second.m_result);
			return sd;
		} else if( (fd != -1 && sd == -1) || (fd != -1 && sd != -1 && fd>=sd)) {
			it = fit;
			m_result.first = std::make_pair(true, m_first.m_result);
			m_result.second = std::make_pair(false, m_second.m_result);
			return fd;
		}
		return -1;
	}

	bool matched_first() const { return m_result.first.first;}
	bool matched_second() const { return m_result.second.first;}
	first_result_type get_first_result() const { return m_result.first.second;}
	second_result_type get_second_result() const { return m_result.second.second;}
};

template<class FirstType, class SecondType>
cat_pair_p<FirstType, SecondType> make_cat_p(const FirstType& f, const SecondType& s) {
	return cat_pair_p<FirstType, SecondType>(f, s);
}
template<class FirstType, class SecondType>
or_pair_p<FirstType, SecondType> make_or_p(const FirstType& f, const SecondType& s) {
	return or_pair_p<FirstType, SecondType>(f, s);
}

// Parses: P?
template<class P>
class optional_p : public or_pair_p<P, epsilon_p> {
  public:
	typedef typename or_pair_p<P, epsilon_p>::first_result_type first_result_type;
	optional_p(const P& p) : or_pair_p<P, epsilon_p>(p, epsilon_p()) {}
	bool occured() const { return or_pair_p<P, epsilon_p>::matched_first();}
	first_result_type get_result() const { return or_pair_p<P, epsilon_p>::get_first_result();}
};

template<class P>
optional_p<P> make_optional(const P& p) {
	return optional_p<P>(p);
}

// Parses: P+
template<class P>
class plus_p : public parselet {
  public:
	typedef plus_p<P> self_type;
	typedef typename P::result_type atom_result_type;
	typedef std::list<atom_result_type> result_type;
	result_type m_result;
	P m_p;
	plus_p(const P& p) : m_p(p) {}
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		const_iterator test_it = it;
		std::ptrdiff_t rd = m_p.parse(test_it, end);
		if(rd == -1) return -1;
		it = test_it;
		m_result.push_back(m_p.m_result);
		while(test_it != end) {
			std::ptrdiff_t d = m_p.parse(test_it, end);
			if(d == -1) break;
			rd += d;
			it = test_it;
			m_result.push_back(m_p.m_result);
		}
		return rd;
	}
};

template<class P>
plus_p<P> make_plus(const P& p) {
	return plus_p<P>(p);
}

// Parses: P*
template<class P>
class star_p : public parselet {
  public:
	typedef star_p<P> self_type;
	typedef typename P::result_type atom_result_type;
	typedef std::list<atom_result_type> result_type;
	result_type m_result;
	P m_p;
	star_p(const P& p) : m_p(p) {}
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		const_iterator test_it = it;
		std::ptrdiff_t rd = m_p.parse(test_it, end);
		if(rd == -1) return 0;
		it = test_it;
		m_result.push_back(m_p.m_result);
		while(test_it != end) {
			std::ptrdiff_t d = m_p.parse(test_it, end);
			if(d == -1) break;
			rd += d;
			it = test_it;
			m_result.push_back(m_p.m_result);
		}
		return rd;
	}
};

template<class P>
star_p<P> make_star(const P& p) {
	return star_p<P>(p);
}

// Parses: FirstType | SecondType | ThirdType
template<class FirstType, class SecondType, class ThirdType>
class or_trio_p : public parselet {
  public:
	typedef FirstType first_type;
	typedef SecondType second_type;
	typedef ThirdType third_type;
	typedef or_trio_p<FirstType, SecondType, ThirdType> self_type;
	typedef typename FirstType::result_type first_result_type;
	typedef typename SecondType::result_type second_result_type;
	typedef typename ThirdType::result_type third_result_type;
	typedef struct  {
		std::pair<bool, first_result_type> first;
		std::pair<bool, second_result_type> second;
		std::pair<bool, third_result_type> third;
	} result_type;
	result_type m_result;

	first_type m_first;
	second_type m_second;
	third_type m_third;
	or_trio_p(const first_type& first, const second_type& second, const third_type& third)
	:	m_first(first), m_second(second), m_third(third) {}

	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end) {
		const_iterator ita[3] = {it, it, it};
		std::ptrdiff_t d[3] = {
			m_first.parse(ita[0], end),
			m_second.parse(ita[1], end),
			m_third.parse(ita[2], end)};
		std::ptrdiff_t dmax = -1;
		for(int i=0;i<3;i++) {
			if(d[i] != -1 && (dmax == -1 || d[i] > dmax)) {
				dmax = d[i];
			}
		}
		if(dmax == -1) return -1;
		if(d[0] == dmax) {
			it = ita[0];
			m_result.first = std::make_pair(true, m_first.m_result);
			m_result.second = std::make_pair(false, m_second.m_result);
			m_result.third = std::make_pair(false, m_third.m_result);
		} else if(d[1] == dmax) {
			it = ita[1];
			m_result.first = std::make_pair(false, m_first.m_result);
			m_result.second = std::make_pair(true, m_second.m_result);
			m_result.third = std::make_pair(false, m_third.m_result);
		} else if(d[2] == dmax) {
			it = ita[2];
			m_result.first = std::make_pair(false, m_first.m_result);
			m_result.second = std::make_pair(false, m_second.m_result);
			m_result.third = std::make_pair(true, m_third.m_result);
		} else {
			m_result.first = std::make_pair(false, m_first.m_result);
			m_result.second = std::make_pair(false, m_second.m_result);
			m_result.third = std::make_pair(false, m_third.m_result);
		}
		return dmax;
	}
	bool matched_first() const { return m_result.first.first;}
	bool matched_second() const { return m_result.second.first;}
	bool matched_third() const { return m_result.third.first;}
	first_result_type get_first_result() const { return m_result.first.second;}
	second_result_type get_second_result() const { return m_result.second.second;}
	third_result_type get_third_result() const { return m_result.third.second;}
};

// let the compiler deduce the type from the args
template<class T1, class T2, class T3>
or_trio_p<T1, T2, T3> make_or_trio_p(const T1& t1, const T2& t2, const T3& t3) {
	return or_trio_p<T1, T2, T3>(t1, t2, t3);
}

////////////////////////////////
// Composite parselets
// More as a pattern than for real use

class list_p : public parselet {
  public:
	typedef list_p self_type;
	typedef std::list<parselet*> result_type;
	result_type m_result;
	~list_p();
	void push_back(parselet* p) { m_result.push_back(p);}
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
// Number parser

// Parses any decimal number not using scientific notation
// includes: (+|-)?d+(.d*)? | .d+
class number_p : public parselet {
  public:
	typedef number_p self_type;
	typedef double result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
	result_type get_result() const { return m_result;}
};

// Parses a list of numbers
// The list is sepatated with white space
// The parser stops to the first not number sequence or at end
class number_list_p : public parselet {
  public:
	typedef number_list_p self_type;
	typedef std::vector<double> result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& endit);

	// result related helpers
	const result_type& get_result() const { return m_result;}
	size_t size() const {return m_result.size();}
	result_type::iterator begin() {return m_result.begin();}
	result_type::iterator end() {return m_result.end();}
	result_type::const_iterator begin() const {return m_result.begin();}
	result_type::const_iterator end() const {return m_result.end();}
};

////////////////////////////////
// Domain parselets

// the following teplates must be extended
// to use std::isalpha and std::isdigit

// NameStartChar ::= (Letter | '_' | ':')
template <class CharType>
struct xml_name_start_ch {
	bool operator()(CharType ch) {
		return isalpha(ch) || ch == '_' || ch == ':';
	}
};

template <class CharType>
struct xml_ncname_start_ch {
	bool operator()(CharType ch) {
		return xml_name_start_ch<CharType>()(ch) && ch != ':';
	}
};

// NameChar ::= Letter | Digit | '.' | '-' | '_' | ':'
template <class CharType>
struct xml_name_ch {
	bool operator()(CharType ch) {
		return isalpha(ch) || isdigit(ch) ||
			ch == '.' || ch == '-' || ch == '_' || ch == ':';
	}
};

template <class CharType>
struct xml_ncname_ch {
	bool operator()(char ch) {
		return xml_name_ch<CharType>()(ch) && ch != ':';
	}
};

// Space ::= #x20 | #x9 | #xD | #xA
template <class CharType>
struct xml_space_ch {
	bool operator()(CharType ch) {
		return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
	}
};

typedef name_p< char, xml_name_start_ch<char>,  xml_name_ch<char> > xml_name_p;
typedef name_p< char, xml_ncname_start_ch<char>,  xml_ncname_ch<char> > xml_ncname_p;
typedef name_p< char, xml_name_ch<char>,  xml_name_ch<char> > xml_nmtoken_p;

// time_unit ::= "h" | "min" | "s" | "ms"
class time_unit_p : public parselet {
  public:
	typedef time_unit_p self_type;
	typedef enum {tu_h, tu_min, tu_s, tu_ms} result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
};

// length_unit ::= "px" | "%"
class length_unit_p : public parselet {
  public:
	typedef length_unit_p self_type;
	typedef enum {px, percent} result_type;
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

class partial_clock_value_p : public parselet {
  public:
	typedef partial_clock_value_p self_type;
	typedef struct {
		int minutes;
		int seconds;
		int fraction;
	}  result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
};

class timecount_value_p : public parselet {
  public:
	typedef full_clock_value_p self_type;
	typedef struct {
		number_p::result_type value;
		time_unit_p::result_type unit;
	}  result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
};

typedef  or_trio_p<full_clock_value_p, partial_clock_value_p, timecount_value_p>
	clock_value_sel_p;

const int S_MS = 1000;
const int MIN_MS = 60*S_MS;
const int H_MS = 60*MIN_MS;

// Though clock_value_sel_p does the job of parsing a clock value
// implement a handy class that will convert any matching clock alt to ms.
class clock_value_p : public parselet {
  public:
	typedef clock_value_p self_type;
	typedef long result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);

	// This will return a valid value when the previous call
	// to parse returned a value != -1.
	result_type get_value() const { return m_result;}
};

// offset-value ::= (( S? "+" | "-" S? )? ( Clock-value )
class offset_value_p : public parselet {
  public:
	typedef clock_value_p self_type;
	typedef long result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
	result_type get_value() const { return m_result;}
};

class coord_p : public parselet {
  public:
	typedef coord_p self_type;
	typedef struct {
		number_p::result_type value;
		length_unit_p::result_type unit;
	}  result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
	length_unit_p::result_type get_units() const { return m_result.unit;}
	int get_px() const { return int(floor(m_result.value+0.5));}
	double get_percent() const {
		return std::max<double>(0.0, std::min<double>(100.0, m_result.value));
	}
};

// region_dim ::= S? (int|dec) ((px)? | %)
class region_dim_p : public parselet {
  public:
	typedef coord_p self_type;
	typedef struct {
		int int_val;
		double dbl_val;
		bool relative;
	}  result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
	int get_px_val() const { return m_result.int_val;}
	double get_relative_val() const { return m_result.dbl_val;}
	bool is_relative() const { return m_result.relative;}
};

// point_p ::= S? (? d+ S? , S? d+  S? )?
class point_p : public parselet {
  public:
	typedef coord_p self_type;
	typedef struct {
		int_p::result_type x;
		int_p::result_type y;
	}  result_type;
	result_type m_result;
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
	int get_x() const { return m_result.x;}
	int get_y() const { return m_result.y;}
};

//parses smtpe=d+:d+:d+[:d+.d+]
class smpte_p : public parselet {
  public:
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
	long int get_time(); //returns the parsed time converted to ms.
	int m_result[5];
	int m_frame_rate;
	bool m_drop;
};


class npt_p : public parselet {
  public:
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
	long int get_time(); //returns the parsed time converted to ms.
	long int m_result;
	int m_frame_rate;
	bool m_drop;
};

class mediaclipping_p : public parselet {
  public:
	std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
	long int get_time(); //returns the parsed time converted to ms.
	long int m_result;
	int m_frame_rate;
	bool m_drop;
};


class wallclock_p : public parselet {
public:
    std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
    long int get_time(); //returns the parsed time converted to ms.
    long int m_result;
};
   
    
class datetime_p : public parselet {
public:
    std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
    struct tm get_time(); //returns the parsed time converted to ms.
    struct tm m_result;
};


class walltime_p : public parselet {
public:
    std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
    struct tm get_time(); //returns the parsed time converted to ms.
    struct tm m_result;
};

class date_p : public parselet {
public:
    std::ptrdiff_t parse(const_iterator& it, const const_iterator& end);
    struct tm get_time(); //returns the parsed date
    struct tm m_result;
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_PARSELETS_H
