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

#include "ambulant/lib/parselets.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/string_util.h"
#include <math.h>

using namespace ambulant;

//////////////////////
// and_p

std::ptrdiff_t 
lib::list_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator test_it = it;
	std::ptrdiff_t sd = 0;
	std::list<parselet*>::iterator rit;
	for(rit = m_result.begin(); rit !=  m_result.end(); rit++) {
		std::ptrdiff_t d = (*rit)->parse(test_it, end);
		if(d == -1) return -1;
		sd += d;
	}
	it = test_it;
	return sd;
}

lib::list_p::~list_p() {
	std::list<parselet*>::iterator rit;
	for(rit = m_result.begin(); rit !=  m_result.end(); rit++) 
		delete *rit;
}
	
//////////////////////
// options_p

std::ptrdiff_t 
lib::options_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator test_it = it;
	
	std::ptrdiff_t dmax = -1;
	
	std::ptrdiff_t *aptrdiff = new int[m_options.size()];
		
	std::list<parselet*>::iterator rit;
	int ix = 0;
	for(rit = m_options.begin(); rit !=  m_options.end(); rit++, ix++)
		aptrdiff[ix] = (*rit)->parse(test_it, end);
	
	for(rit = m_options.begin(), ix=0; rit !=  m_options.end(); rit++, ix++) {
		std::ptrdiff_t cur = aptrdiff[ix];
		if(cur != -1 && (dmax == -1 || cur > dmax)) {
			m_result = *rit;
			dmax = cur;
		}
	}
	for(std::ptrdiff_t d=0;d<dmax;d++) it++;
	return dmax;
}

lib::options_p::~options_p() {
	std::list<parselet*>::iterator rit;
	for(rit = m_options.begin(); rit !=  m_options.end(); rit++) 
		delete *rit;
}


//////////////////////
// time_unit_p

std::ptrdiff_t 
lib::time_unit_p::parse(const_iterator& it, const const_iterator& end) {
	literal_cstr_p h_p("h");
	literal_cstr_p min_p("min");
	literal_cstr_p s_p("s");
	literal_cstr_p ms_p("ms");
	const_iterator tit = it;
	std::ptrdiff_t d;
	if((d = h_p.parse(tit, end)) != -1) return (m_result = tu_h, it = tit, 1);
	else if((d = min_p.parse(tit, end)) != -1) return (m_result = tu_min, it = tit, 3);
	else if((d = ms_p.parse(tit, end)) != -1) return (m_result = tu_ms, it = tit, 2);
	else if((d = s_p.parse(tit, end)) != -1) return (m_result = tu_s, it = tit, 1);
	return -1;
}

//////////////////////
// length_unit_p

std::ptrdiff_t 
lib::length_unit_p::parse(const_iterator& it, const const_iterator& end) {
	literal_cstr_p px_p("px");
	literal_cstr_p percent_p("%");
	const_iterator tit = it;
	std::ptrdiff_t d;
	if((d = px_p.parse(tit, end)) != -1) return (m_result = px, it = tit, 2);
	else if((d = percent_p.parse(tit, end)) != -1) return (m_result = percent, it = tit, 1);
	return -1;
}

//////////////////////
// full_clock_value_p

std::ptrdiff_t 
lib::full_clock_value_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
		
	// hours
	int_p hours;
	d = hours.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.hours = hours.m_result;
	
	// :
	d = literal_p<':'>().parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	
	// minutes
	int_p minutes;
	d = minutes.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.minutes = minutes.m_result;
	
	// :
	d = literal_p<':'>().parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	
	// seconds
	int_p seconds;
	d = seconds.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.seconds = seconds.m_result;
	
	/////////////////
	// optional part
	// . 
	d = literal_p<'.'>().parse(tit, end);
	if(d == -1) {
		m_result.fraction = -1;
		it = tit;
		return sd;
	}
	sd += d;
	
	// fraction
	int_p fraction;
	d = fraction.parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	m_result.fraction = fraction.m_result;
	it = tit;
	return sd;
}

//////////////////////
// partial_clock_value_p

std::ptrdiff_t 
lib::partial_clock_value_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	// minutes
	int_p minutes;
	d = minutes.parse(tit, end);
	if(d == -1) return -1;
	m_result.minutes = minutes.m_result;
	sd += d;
	
	// :
	d = literal_p<':'>().parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	
	// seconds
	int_p seconds;
	d = seconds.parse(tit, end);
	if(d == -1) return -1;
	m_result.seconds = seconds.m_result;
	sd += d;
	
	/////////////////
	// optional part
	// . 
	d = literal_p<'.'>().parse(tit, end);
	if(d == -1) {
		m_result.fraction = -1;
		it = tit;
		return sd;
	}
	sd += d;
	
	// fraction
	int_p fraction;
	d = fraction.parse(tit, end);
	if(d == -1) return -1;
	m_result.fraction = fraction.m_result;
	sd += d;
	it = tit;
	return sd;
}

//////////////////////
// timecount_value_p

std::ptrdiff_t 
lib::timecount_value_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	// parse value
	dec_p p1;
	d = p1.parse(tit, end);
	if(d == -1) return -1;
	m_result.value = p1.m_result;
	sd += d;
	
	// parse optional units	
	time_unit_p p2;
	d = p2.parse(tit, end);
	if(d == -1) return (m_result.unit = time_unit_p::tu_s, it = tit, sd);
	sd += d;
	return (m_result.unit = p2.m_result, it = tit, sd);
}

//////////////////////
// clock_value_p and converter to ms

inline int fraction_to_ms(int f) {
	return (f<=0)?0:int(::floor(1000.0*f + 0.5));
}

std::ptrdiff_t 
lib::clock_value_p::parse(const_iterator& it, const const_iterator& end) {
	lib::clock_value_sel_p p = make_or_trio_p(full_clock_value_p(), 
			partial_clock_value_p(), timecount_value_p());
	const_iterator test_it = it;
	std::ptrdiff_t d = p.parse(test_it, end);
	if(d == -1) return -1;
	if(p.matched_first()) {
		full_clock_value_p::result_type r = p.get_first_result();
		m_result = r.hours * H_MS + r.minutes * MIN_MS + r.seconds * S_MS + fraction_to_ms(r.fraction);
	} else if(p.matched_second()) {
		partial_clock_value_p::result_type r = p.get_second_result();
		m_result = r.minutes * MIN_MS + r.seconds * S_MS + fraction_to_ms(r.fraction);
	} else if(p.matched_third()) {
		timecount_value_p::result_type r = p.get_third_result();
		switch(r.unit) {
			case time_unit_p::tu_h:
				m_result = long(::floor(r.value * H_MS + 0.5));
				break;
			case time_unit_p::tu_min:
				m_result = long(::floor(r.value * MIN_MS + 0.5));
				break;
			case time_unit_p::tu_s:
				m_result = long(::floor(r.value * S_MS + 0.5));
				break;
			case time_unit_p::tu_ms:
				m_result = long(::floor(r.value + 0.5));
				break;
		}
	} else {
		logger::get_logger()->error("clock_value_p logic error");
	}	
	it = test_it;
	return d;
}

//////////////////////
// offset_value_p and converter to ms
// offset-value ::= (( S? "+" | "-" S? )? ( Clock-value )

std::ptrdiff_t 
lib::offset_value_p::parse(const_iterator& it, const const_iterator& end) {
		const_iterator test_it = it;
		std::ptrdiff_t rd = 0;
		std::ptrdiff_t d;
		
		delimiter_p space(" \t\r\n");
		star_p<delimiter_p> opt_space_inst = make_star(space);

		d = opt_space_inst.parse(test_it, end);
		rd += (d == -1)?0:d;
		
		delimiter_p sign_inst("+-");
		d = sign_inst.parse(test_it, end);
		int sign = (d == -1)?1:( (sign_inst.m_result == '+')?1:-1 );
		rd += (d == -1)?0:d;
	
		d = opt_space_inst.parse(test_it, end);
		rd += (d == -1)?0:d;
		
		clock_value_p c;
		d = c.parse(test_it, end);
		if(d == -1) return -1;
		rd += d;
		it = test_it;
		m_result = sign*c.m_result;
		return rd;
}

//////////////////////
// coord_p

std::ptrdiff_t 
lib::coord_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator tit = it;
	std::ptrdiff_t d;
	std::ptrdiff_t sd = 0;
	
	// we need this if we want to allow optional space between value and units
	delimiter_p space(" \t\r\n");
	star_p<delimiter_p> opt_space_inst = make_star(space);
	
	// parse value
	dec_p p1;
	d = p1.parse(tit, end);
	if(d == -1) return -1;
	m_result.value = p1.m_result;
	sd += d;
	
	// allow optional space between value and units
	d = opt_space_inst.parse(tit, end);
	sd += (d == -1)?0:d;
	
	// parse optional units	
	length_unit_p p2;
	d = p2.parse(tit, end);
	if(d == -1) return (m_result.unit = length_unit_p::px, it = tit, sd);
	sd += d;
	return (m_result.unit = p2.m_result, it = tit, sd);
}


