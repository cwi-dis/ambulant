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

/// Type that represents the allowable SMIL 2.0 fit values.
enum fit_t {fit_default, fit_hidden, fit_fill, fit_meet,
#ifdef USE_SMIL21
			fit_meetbest,
#endif
			fit_scroll, fit_slice };

/// Type that represents a SMIL 2.0 z-index value.
typedef int zindex_t;

/// Interface to a class that holds all SMIL 2.0 layout information for a region.
/// This is the read-only interface, used to construct windows and such.
class region_info {
  public:
	
	/// Return the name of the region.
	virtual std::string get_name() const = 0;
	
	/// Return the rectangle of the region.
	virtual basic_rect<int> get_rect() const = 0;
	
	/// Return the rectangle of the region.
	virtual screen_rect<int> get_screen_rect() const = 0;
	
	/// Return the fit attribute for the region.
	virtual fit_t get_fit() const = 0;
	
	/// Return the background color for the region.
	virtual color_t get_bgcolor() const = 0;
	
	/// Return true if the region is transparent.
	virtual bool get_transparent() const = 0;
	
	/// Return the zindex for the region.
	virtual zindex_t get_zindex() const = 0;
	
	/// Return true if showbackground=always.
	virtual bool get_showbackground() const = 0;
	
	/// Return true if this object represents subregion positioning on a body node.
	virtual bool is_subregion() const = 0;
	
	/// Return audio volume
	virtual double get_soundlevel() const = 0;
};

/// Interface to animate region information.
/// This is the read/write interface used by animator objects to fiddle the
/// parameters of the region (or node).
class animation_destination : public region_info {
  public:
  
	/// Get one of the six dimensions of a region.
	/// If fromdom is true get the original DOM value, otherwise get the current
	/// value (as animated by previous set_ calls). The name which is the
	/// SMIL attribute name.
	virtual region_dim get_region_dim(const std::string& which, bool fromdom = false) const = 0;
  
	/// Get one of the region colors.
	/// If fromdom is true get the original DOM value, otherwise get the current
	/// value (as animated by previous set_ calls). The name which is the
	/// SMIL attribute name.
	virtual color_t get_region_color(const std::string& which, bool fromdom = false) const = 0;
  
	/// Get the z-index of a region.
	/// If fromdom is true get the original DOM value, otherwise get the current
	/// value (as animated by previous set_ calls).
	virtual zindex_t get_region_zindex(bool fromdom = false) const = 0;
  
	/// Get the audio volume of a region.
	/// If fromdom is true get the original DOM value, otherwise get the current
	/// value (as animated by previous set_ calls).
	virtual double get_region_soundlevel(bool fromdom = false) const = 0;
  
	/// Set one of the six dimensions of a region to a new value.
	/// The name which is the SMIL attribute name.
	virtual void set_region_dim(const std::string& which, const region_dim& rd) = 0;
	
	/// Set one of the region colors to a new value.
	/// The name which is the SMIL attribute name.
	virtual void set_region_color(const std::string& which, lib::color_t clr) = 0;
	
	/// Set the region z-index to a new value.
	virtual void set_region_zindex(common::zindex_t z) = 0;
	
	/// Set the region audio volume to a new value.
	virtual void set_region_soundlevel(double level) = 0;
};

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_REGION_INFO_H
