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
			fit_meetbest, fit_scroll, fit_slice };

/// Type that represents a SMIL 2.0 z-index value.
typedef int zindex_t;

/// Type that represents where audio is panned
enum sound_alignment {sa_default, sa_both, sa_left, sa_right};

/// Type that represents image tiling
enum tiling { tiling_default, tiling_none, tiling_inherit,
		tiling_horizontal, tiling_vertical, tiling_both};

/// Interface to a class that holds all SMIL 2.0 layout information for a region.
/// This is the read-only interface, used to construct windows and such.
class region_info {
  public:
	virtual ~region_info(){}

	/// Return the name of the region.
	virtual std::string get_name() const = 0;

	/// Return the rectangle of the region.
	virtual rect get_rect(const lib::rect *default_rect = NULL) const = 0;

	/// Return the fit attribute for the region.
	virtual fit_t get_fit() const = 0;

	/// Return the background color for the region.
	virtual color_t get_bgcolor() const = 0;

	/// Return the background opacity of the region.
	virtual double get_bgopacity() const = 0;

	/// Return true if the background is transparent for the region.
	virtual bool get_transparent() const = 0;

	/// Return the zindex for the region.
	virtual zindex_t get_zindex() const = 0;

	/// Return true if showbackground=always.
	virtual bool get_showbackground() const = 0;

	/// Return true if this object represents subregion positioning on a body node.
	virtual bool is_subregion() const = 0;

	/// Return audio volume
	virtual double get_soundlevel() const = 0;

	/// Return audio placement
	virtual sound_alignment get_soundalign() const = 0;

	/// Return image tiling within region
	virtual tiling get_tiling() const = 0;

	/// Return the background image
	virtual const char *get_bgimage() const = 0;

	/// Return the image crop area
	virtual rect get_crop_rect(const size& srcsize) const = 0;

	/// Return the media opacity of the region.
	virtual double get_mediaopacity() const = 0;

	/// Return the media background opacity of the region.
	virtual double get_mediabgopacity() const = 0;

	/// Return whether the region.has a valid chromakey
	virtual bool is_chromakey_specified() const = 0;

	/// Return the chromakey of the region.
	virtual lib::color_t get_chromakey() const = 0;

	/// Return the chromakey tolerance of the region.
	virtual lib::color_t get_chromakeytolerance() const = 0;

	/// Return the chromakey opacity of the region.
	virtual double get_chromakeyopacity() const = 0;
};

/// Interface to animate region information.
/// This is the read/write interface used by animator objects to fiddle the
/// parameters of the region (or node).
class animation_destination : public region_info {
  public:
	virtual ~animation_destination() {}

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

	/// Get the audio volume of a region.
	/// If fromdom is true get the original DOM value, otherwise get the current
	/// value (as animated by previous set_ calls).
	virtual sound_alignment get_region_soundalign(bool fromdom = false) const = 0;

	/// Get the region image cropping parameters.
	/// If fromdom is true get the original DOM value, otherwise get the current
	/// value (as animated by previous set_ calls).
	virtual const region_dim_spec& get_region_panzoom(bool fromdom = false) const = 0;

	/// Get the background opacity of a region.
	/// If fromdom is true get the original DOM value, otherwise get the current
	/// value (as animated by previous set_ calls).
	virtual double get_region_opacity(const std::string& which, bool fromdom = false) const = 0;

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

	/// Set the region audio volume to a new value.
	virtual void set_region_soundalign(sound_alignment sa) = 0;

	/// Set the region image cropping parameters
	virtual void set_region_panzoom(const region_dim_spec& rds) = 0;

	/// Set the region background opacity to a new value.
	virtual void set_region_opacity(const std::string& which, double level) = 0;
};

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_REGION_INFO_H
