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
	
  private:
	schedulable *m_tn;
	const time_attrs& m_attrs;
	static lib::logger *clogger;
};

} // namespace smil2
 
} // namespace ambulant


#endif // AMBULANT_SMIL2_TIME_CALC_H
