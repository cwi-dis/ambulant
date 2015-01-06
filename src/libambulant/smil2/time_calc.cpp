// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/smil2/time_calc.h"
#include <cmath>

#define AM_DBG if(0)
//#define AM_DBG if(1)

using namespace ambulant;
using namespace smil2;

// static
lib::logger* time_calc::clogger = 0;

time_calc::time_calc(schedulable *tn)
:	m_tn(tn),
	m_attrs(*tn->get_time_attrs()),
	m_paused(false), m_uses_dur(false) {
	if(!clogger) clogger = lib::logger::get_logger();
}

// This function calculates the simple duration of this node.
// See spec: "Defining the simple duration"
// The last calculated simple duration is stored in the variable m_last_cdur.
// This function will call the function "get_implicit_dur()" if and only if
// the implicit duration of a node is required by the timing model.
time_calc::time_type
time_calc::calc_dur() {
	if(!m_attrs.has_dur_specifier() && m_attrs.specified_end()) {
		AM_DBG clogger->debug("%s[%s].calc_dur(): %s", m_attrs.get_tag().c_str(),
			m_attrs.get_id().c_str(), "indefinite");
		return time_type::indefinite;
	}
	dur_type dt = m_attrs.get_dur_type();
	time_type cdur = time_type::unresolved;
	if(m_paused) dt = dt_indefinite; // do the calcs as if indefinite
	if(dt == dt_definite) {
		m_uses_dur = true;
		cdur = time_manipulated(m_attrs.get_dur());
	} else if(dt == dt_indefinite) {
		cdur = time_type::indefinite;
	} else if(dt == dt_unspecified) {
		m_uses_dur = true;
		cdur = m_tn->get_implicit_dur();
	} else if(dt == dt_media) {
		m_uses_dur = true;
		cdur = m_tn->get_implicit_dur();
	}
	AM_DBG clogger->debug("%s[%s].calc_dur(): %s", m_attrs.get_tag().c_str(),
		m_attrs.get_id().c_str(), ::repr(cdur).c_str());
	return cdur;
}

time_calc::time_type
time_calc::time_manipulated(time_type d) const {
	if(!d.is_definite()) return d;
	double speed = fabs(m_attrs.get_speed());
	u_value_type speed100 = (u_value_type)(::floor(0.5 + speed * 100));
	if(speed100 == 0) speed100 = 1;
	unsigned long ud = (speed100 * d() ) / 100;
	d = time_type(ud);
	if(m_attrs.auto_reverse()) d += d;
	return d;
}

// See spec: "Intermediate Active Duration Computation"
// This function calculates the "intermediate active duration" of this node.
// This quantity is used by the active duration calculation.
time_calc::time_type
time_calc::calc_intermediate_ad() {
	time_type cdur = calc_dur();
	time_type adrad = m_attrs.specified_rcount()?calc_active_rad():time_type::indefinite;
	time_type rdur = m_attrs.specified_rdur()?m_attrs.get_rdur():time_type::indefinite;
	time_type iad;
	if(cdur() == 0) iad = time_type(0);
	else if(!m_attrs.specified_rcount() && !m_attrs.specified_rdur())
		iad = cdur;
	else
		iad = std::min(std::min(adrad, rdur), time_type::indefinite);
	return iad;
}

// See spec: "Intermediate Active Duration Computation"
// This function calculates the accumulated sum of the specified
// number of simple durations of the iterations of this element.
time_calc::time_type
time_calc::calc_active_rad() {
	assert(m_attrs.specified_rcount());
	double rcount = m_attrs.get_rcount();

	// Calculate based on the specified dur if definite
	dur_type dt = m_attrs.get_dur_type();
	if(dt == dt_definite) {
		if(m_attrs.is_rcount_indefinite())
			return time_type::indefinite;
		else {
			time_type dur = time_manipulated(m_attrs.get_dur());
			return (value_type) ::floor(rcount*dur() + 0.5);
		}
	}

	// Calculate based on the the implicit dur if definite
	time_type idur = m_tn->get_implicit_dur();
	if(idur.is_definite()) {
		if(m_attrs.is_rcount_indefinite())
			return time_type::indefinite;
		else
			return (value_type) ::floor(rcount*idur() + 0.5);
	}

	// Best estimate based on the previous iteration.
	time_type last_cdur = m_tn->get_last_dur();
	if(last_cdur.is_definite()) {
		if(m_attrs.is_rcount_indefinite())
			return time_type::indefinite;
		else
			return (value_type) ::floor(rcount*last_cdur() + 0.5);
	}

	// Cann't be resolved yet.
	return time_type::unresolved;
}

// See spec: "Active duration algorithm"
// This function calculates the preliminary active duration of an element,
// before accounting for min and max  semantics
time_calc::time_type
time_calc::calc_preliminary_ad(time_type b, time_type e) {
	if(!m_attrs.has_dur_specifier() && m_attrs.specified_end()) {
		assert(calc_dur() == time_type::indefinite);
		if(e.is_definite())
			return e - b;
		else if(e.is_indefinite())
			return time_type::indefinite;
		else if(e.is_unresolved())
			return time_type::unresolved;
	} else if(!m_attrs.specified_end() || (m_attrs.specified_end() && m_attrs.end_is_indefinite())) {
		return calc_intermediate_ad();
	}
	// else
	assert( (m_attrs.specified_end() && !m_attrs.end_is_indefinite()) && m_attrs.has_dur_specifier());
	return std::min(calc_intermediate_ad(), e - b);
}

// See spec: "Active duration algorithm"
// This function calculates the preliminary active duration of this node,
// before accounting for min and max  semantics.
// Simplified version of the above applicable when "end" is not specified.
time_calc::time_type
time_calc::calc_preliminary_ad(time_type b) {
	assert(!m_attrs.specified_end());
	return calc_intermediate_ad();
}

// See spec: "Active duration algorithm"
// This function calculates the active duration of this node.
time_calc::time_type
time_calc::calc_ad(time_type b, time_type e) {
	time_type minval = m_attrs.specified_min()?m_attrs.get_min():0;
	time_type maxval = m_attrs.specified_max()?m_attrs.get_max():time_type::indefinite;
	time_type pad = calc_preliminary_ad(b, e);
	time_type cad = std::min(maxval, std::max(minval, pad));
	AM_DBG clogger->debug("%s[%s].calc_ad(): %s", m_attrs.get_tag().c_str(),
		m_attrs.get_id().c_str(), ::repr(cad).c_str());
	return cad;
}

// See spec: "Active duration algorithm"
// This function calculates the active duration of this node.
// Simplified version of the above applicable when "end" is not specified.
time_calc::time_type
time_calc::calc_ad(time_type b) {
	assert(!m_attrs.specified_end());
	time_type minval = m_attrs.specified_min()?m_attrs.get_min():0;
	time_type maxval = m_attrs.specified_max()?m_attrs.get_max():time_type::indefinite;
	time_type pad = calc_preliminary_ad(b);
	time_type cad = std::min(maxval, std::max(minval, pad));
	AM_DBG clogger->debug("%s[%s].calc_ad(): %s", m_attrs.get_tag().c_str(),
		m_attrs.get_id().c_str(), ::repr(cad).c_str());
	return cad;
}

// Calculates and returns the active end of this node.
// This uses calc_ad() function.
time_calc::time_type
time_calc::calc_end(time_type b, time_type e) {
	return b + calc_ad(b, e);
}

// Calculates and returns the active end of this node when "end" is not specified.
// This uses calc_ad() function.
time_calc::time_type
time_calc::calc_end(time_type b) {
	assert(!m_attrs.specified_end());
	return b + calc_ad(b);
}

// Calculates and returns the current inteval end.
// This uses calc_ad() function.
time_calc::time_type
time_calc::calc_interval_end(interval_type& i, time_mset& end_list)  {
	time_type begin = i.begin;
	time_type end;
	if(!m_attrs.specified_end()) {
		end = calc_end(begin);
	} else {
		time_mset::iterator eit = end_list.lower_bound(begin);
		if(eit != end_list.end()) end = *eit;
		else end = time_type::unresolved;
		end = calc_end(begin, end);
	}
	AM_DBG clogger->debug("%s[%s].calc_interval_end(): %s", m_attrs.get_tag().c_str(),
		m_attrs.get_id().c_str(), ::repr(end).c_str());
	return end;
}

// See spec: "Getting the first interval"
// Calculates the first valid interval for this node.
// An interval is valid if:
// a) has a definite begin
// b) it is a valid child of its parent (e.g. begin < begin_before && end >= parent.begin)
// When no such interval exists this function returns unresolved (e.g. failure).
// When more than one intervals are valid the one that begins earlier is returned.
// The returned interval has been modulated by any max/min attributes or any time manipulations.
//
// Please note that the interval calculated by this algorithm
// does not embed all the available info provided by the model.
// For example the fact that the parent simple duration
// or the cut short last parent simple dur will
// cut short the interval calculated by this algorithm.
// The interval calculated may have an unresolved or indefinite end
// whereas the parent may have a definite simple dur.
// The scheduler should take into account this extra info.
//
// The calculation uses active and simple duration algorithms
// and requires this node's begin and end lists and the parent
// simple dur.
//
time_calc::interval_type
time_calc::calc_first_interval(time_mset& begin_list, time_mset& end_list,
	time_type begin_before) {
	// define failure as an alias for the invalid unresolved interval.
	interval_type failure = interval_type::unresolved;

	// The begin of the first interval can be in [-infinity, parent.simple_end)
	time_type begin_after = time_type::minus_infinity;

	// Check the begin instance list
	if(begin_list.empty()) {
		AM_DBG clogger->debug("%s[%s].calc_first_interval(): begin list is empty",
			m_attrs.get_tag().c_str(),
			m_attrs.get_id().c_str());
		return failure;
	}
	AM_DBG clogger->debug("%s[%s].calc_first_interval(): begin list(%s, ...)",
		m_attrs.get_tag().c_str(),
		m_attrs.get_id().c_str(),
		::repr(*begin_list.lower_bound(begin_after)).c_str());

	while(true) {
		time_mset::iterator bit = begin_list.lower_bound(begin_after);
		if(bit == begin_list.end()) return failure;
		time_type temp_begin = *bit;
		time_type temp_end;
		if(temp_begin > begin_before) return failure;
		if(!m_attrs.specified_end()) {
			temp_end = calc_end(temp_begin);
		} else {
			time_mset::iterator eit = end_list.lower_bound(temp_begin);
			if(eit != end_list.end()) {
				temp_end = *eit;
				end_list.erase(eit); // don't use it again e.g. next for equal values
			}  else {
				// either the list is empty or no time >= temp_begin
				if(m_attrs.end_has_event_conditions() || end_list.empty())
					temp_end = time_type::unresolved;
				else {
					AM_DBG clogger->debug("%s[%s].calc_first_interval(): %s",
						m_attrs.get_tag().c_str(), m_attrs.get_id().c_str(), ::repr(failure).c_str());
					return failure;
				}
			}
			temp_end = calc_end(temp_begin, temp_end);
		}
		// Handle the zero duration intervals at the parent begin time as a special case
		// see: http://www.w3.org/2001/07/REC-SMIL20-20010731-errata
		if(temp_end()>0 || (temp_begin()== 0 && temp_end()==0)) {
			interval_type ret(temp_begin, temp_end);
			AM_DBG clogger->debug("%s[%s].calc_first_interval(): %s",
				m_attrs.get_tag().c_str(), m_attrs.get_id().c_str(), ::repr(ret).c_str());
			return ret;
		}
		// else
		begin_after = temp_end;
	}
}

// See spec: "End of interval - Getting the next interval"
// Calculates the next acceptable interval for this node.
// The variable this->m_interval holds the just ended interval.
//
// This calculation uses active and simple duration algorithms
// and requires this node's begin and end lists, this node's
// just ended interval, and the parent simple dur.
time_calc::interval_type
time_calc::calc_next_interval(time_mset& begin_list, time_mset& end_list,
	time_type begin_before, time_type begin_after, bool prev_was_zero_dur) {
	// define failure as an alias for the invalid unresolved interval.
	interval_type failure = interval_type::unresolved;

	if(begin_list.empty())
		return failure;

	time_mset::iterator bit;
	if(!prev_was_zero_dur)
		// the first value in the begin list that is >= begin_after
		bit = begin_list.lower_bound(begin_after);
	else
		// the first value in the begin list that is > begin_after
		// see: http://www.w3.org/2001/07/REC-SMIL20-20010731-errata
		bit = begin_list.upper_bound(begin_after);

	// If no such value or begins after parent end abort
	if(bit == begin_list.end() || *bit >= begin_before)
		return failure;

	// OK, we have a begin value - get an end
	time_type temp_begin = *bit;
	time_type temp_end;

	if(!m_attrs.specified_end()) {
		// this calculates the active end with no end constraint
		temp_end = calc_end(temp_begin);
	} else {
		// the first value in the end list that is >= temp_begin
		time_mset::iterator eit = end_list.lower_bound(temp_begin);

		// Allow for non-0-duration interval that begins immediately
		// after a 0-duration interval.
		if(eit != end_list.end()) {
			while(*eit == begin_after && eit != end_list.end()) ++eit;
			if(eit != end_list.end()) temp_end = *eit;
		}
		if(eit == end_list.end()) { // no such value
			if(m_attrs.end_has_event_conditions() || end_list.empty())
				temp_end = time_type::unresolved;
			else
				return failure;
		}
		temp_end = calc_end(temp_begin, temp_end);
	}
	return interval_type(temp_begin, temp_end);
}

