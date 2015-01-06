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

#ifndef AMBULANT_SMIL2_TIME_CALC_H
#define AMBULANT_SMIL2_TIME_CALC_H

#include "ambulant/config/config.h"
#include "ambulant/smil2/time_attrs.h"

namespace ambulant {

namespace smil2 {

class time_calc : public time_traits {
  public:
	time_calc(schedulable *tn);

	interval_type calc_first_interval(time_mset& begin_list, time_mset& end_list,
		time_type begin_before);

	interval_type calc_next_interval(time_mset& begin_list, time_mset& end_list,
		time_type begin_before, time_type begin_after, bool prev_was_zero_dur);

	// Calculates the simple duration of this node
	time_type calc_dur();

	// Calculates the active duration (AD) of this node
	// Uses simple duration calculation: calc_dur()
	time_type calc_ad(time_type b, time_type e);
	time_type calc_ad(time_type b);

	// AD helpers
	time_type calc_preliminary_ad(time_type b, time_type e);
	time_type calc_intermediate_ad();
	time_type calc_active_rad();
	time_type calc_preliminary_ad(time_type b);

	time_type calc_end(time_type b, time_type e);
	time_type calc_end(time_type b);

	time_type time_manipulated(time_type d) const;

	// Calculates and returns the current inteval end.
	// This uses calc_ad() function.
	time_type calc_interval_end(interval_type& i, time_mset& end_list);

	void set_paused_mode(bool b) { m_paused = b;}

	bool uses_dur() const { return m_uses_dur;}
  private:
	schedulable *m_tn;
	const time_attrs& m_attrs;
	bool m_paused;
	bool m_uses_dur;
	static lib::logger *clogger;
};

} // namespace smil2

} // namespace ambulant


#endif // AMBULANT_SMIL2_TIME_CALC_H
