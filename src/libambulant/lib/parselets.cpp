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

using namespace ambulant;

//////////////////////
// and_p

std::ptrdiff_t 
lib::and_p::parse(const_iterator& it, const const_iterator& end) {
	const_iterator test_it = it;
	std::ptrdiff_t sd = 0;
	std::list<parselet*>::iterator rit;
	for(rit = m_rules.begin(); rit !=  m_rules.end(); rit++) {
		std::ptrdiff_t d = (*rit)->parse(test_it, end);
		if(d<0) return -1;
		sd += d;
	}
	it = test_it;
	return sd;
}

lib::and_p::~and_p() {
	std::list<parselet*>::iterator rit;
	for(rit = m_rules.begin(); rit !=  m_rules.end(); rit++) 
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
		if(cur > dmax) {
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
	if((d = h_p.parse(tit, end))>0) return (m_result = tu_h, it = tit, 1);
	else if((d = min_p.parse(tit, end))>0) return (m_result = tu_min, it = tit, 3);
	else if((d = ms_p.parse(tit, end))>0) return (m_result = tu_ms, it = tit, 2);
	else if((d = s_p.parse(tit, end))>0) return (m_result = tu_s, it = tit, 1);
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
	m_result.hours = hours.m_result;
	sd += d;
	
	// :
	d = literal_p<':'>().parse(tit, end);
	if(d == -1) return -1;
	sd += d;
	
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
	d = fraction.parse(it, end);
	if(d == -1) return -1;
	m_result.fraction = fraction.m_result;
	sd += d;
	return sd;
}