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

#ifndef AMBULANT_COMMON_REGION_INFO_H
#define AMBULANT_COMMON_REGION_INFO_H

#include "ambulant/config/config.h"

#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/colors.h"
#include "ambulant/common/region_dim.h"

namespace ambulant {

namespace common {

	using namespace ambulant::lib;

enum fit_t { fit_fill, fit_hidden, fit_meet, fit_scroll, fit_slice };
typedef int zindex_t;

// This interface allows access to all information 
class region_info {
  public:
	virtual std::string get_name() const = 0;
	virtual basic_rect<int> get_rect() const = 0;
	virtual screen_rect<int> get_screen_rect() const = 0;
	virtual fit_t get_fit() const = 0;
	virtual color_t get_bgcolor() const = 0;
	virtual bool get_transparent() const = 0;
	virtual zindex_t get_zindex() const = 0;
	virtual bool get_showbackground() const = 0;
	virtual bool is_subregion() const = 0;
};

class animation_destination : public region_info {
  public:
	// The following functions get the dom or display values of the region attributes
	virtual region_dim get_region_dim(const std::string& which, bool fromdom = false) const = 0;
	virtual color_t get_region_color(const std::string& which, bool fromdom = false) const = 0;
	virtual zindex_t get_region_zindex(bool fromdom = false) const = 0;
  
	// The following functions set the display values for the region attributes
	virtual void set_region_dim(const std::string& which, const region_dim& rd) = 0;
	virtual void set_region_color(const std::string& which, lib::color_t clr) = 0;
	virtual void set_region_zindex(common::zindex_t z) = 0;
};

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_REGION_INFO_H
