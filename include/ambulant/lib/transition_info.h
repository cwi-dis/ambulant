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

#ifndef AMBULANT_LIB_TRANSITION_INFO_H
#define AMBULANT_LIB_TRANSITION_INFO_H

#include "ambulant/lib/colors.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/timer.h"

namespace ambulant {

namespace lib {

/// Type of transition, as per the SMIL 2.1 standard.
enum transition_type {
	barWipe,
	boxWipe,
	fourBoxWipe,
	barnDoorWipe,
	diagonalWipe,
	bowTieWipe,
	miscDiagonalWipe,
	veeWipe,
	barnVeeWipe,
	zigZagWipe,
	barnZigZagWipe,
	irisWipe,
	triangleWipe,
	arrowHeadWipe,
	pentagonWipe,
	hexagonWipe,
	ellipseWipe,
	eyeWipe,roundRectWipe,
	starWipe,
	miscShapeWipe,
	clockWipe,
	pinWheelWipe,
	singleSweepWipe,
	fanWipe,
	doubleFanWipe,
	doubleSweepWipe,
	saloonDoorWipe,
	windshieldWipe,
	snakeWipe,
	spiralWipe,
	parallelSnakesWipe,
	boxSnakesWipe,
	waterfallWipe,
	pushWipe,
	slideWipe,
	fade,
	audioFade,
	audioVisualFade,
};

/// Determines area to which transition will apply.
enum transition_scope {
	scope_region,  ///< Apply to the media region only.
	scope_screen   ///< Apply to the whole window.
};

/// Stores all information regarind a specific transition.
class AMBULANTAPI transition_info {
  public:
	/// How a transition represents time
	typedef timer::time_type time_type;
	/// How a transition represents progress (value between 0 and 1).
	typedef double progress_type;
	
	transition_type m_type;    ///< SMIL 2.1 transition type.

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

	std::string m_subtype;     ///< SMIL 2.1 transition subtype.

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	time_type m_dur;           ///< Transition duration (Unit???)
	progress_type m_startProgress; ///< Starting point of transition.
	progress_type m_endProgress;   ///< Ending point of transition.
	bool m_reverse;            ///< True if transition orientation is reversed.
	lib::color_t m_color;      ///< Color, if transition needs it.
	transition_scope m_scope;  ///< Should transition be region or whole-window?
	// We should also have the <param> contents here

	transition_info() { }
	/// Construct transition_info as a copy of another transition_info.
	transition_info(transition_info* info) { *this = *info; }

	/// Factory function: obtain transition_info object from DOM node.
	static transition_info *from_node(const node *n);

  private:
	static time_type get_trans_dur(const node *n);
	static progress_type get_progress(const node *n, const char* progress, progress_type default_value);
};

} // namespace lib

} // namespace ambulant

inline std::string repr(ambulant::lib::transition_type t)
{
	switch(t) {
	case ambulant::lib::barWipe: return "barWipe";
	case ambulant::lib::boxWipe: return "boxWipe";
	case ambulant::lib::fourBoxWipe: return "fourBoxWipe";
	case ambulant::lib::barnDoorWipe: return "barnDoorWipe";
	case ambulant::lib::diagonalWipe: return "diagonalWipe";
	case ambulant::lib::bowTieWipe: return "bowTieWipe";
	case ambulant::lib::miscDiagonalWipe: return "miscDiagonalWipe";
	case ambulant::lib::veeWipe: return "veeWipe";
	case ambulant::lib::barnVeeWipe: return "barnVeeWipe";
	case ambulant::lib::zigZagWipe: return "zigZagWipe";
	case ambulant::lib::barnZigZagWipe: return "barnZigZagWipe";
	case ambulant::lib::irisWipe: return "irisWipe";
	case ambulant::lib::triangleWipe: return "triangleWipe";
	case ambulant::lib::arrowHeadWipe: return "arrowHeadWipe";
	case ambulant::lib::pentagonWipe: return "pentagonWipe";
	case ambulant::lib::hexagonWipe: return "hexagonWipe";
	case ambulant::lib::ellipseWipe: return "ellipseWipe";
	case ambulant::lib::eyeWipe: return "eyeWipe";
	case ambulant::lib::roundRectWipe: return "roundRectWipe";
	case ambulant::lib::starWipe: return "starWipe";
	case ambulant::lib::miscShapeWipe: return "miscShapeWipe";
	case ambulant::lib::clockWipe: return "clockWipe";
	case ambulant::lib::pinWheelWipe: return "pinWheelWipe";
	case ambulant::lib::singleSweepWipe: return "singleSweepWipe";
	case ambulant::lib::fanWipe: return "fanWipe";
	case ambulant::lib::doubleFanWipe: return "doubleFanWipe";
	case ambulant::lib::doubleSweepWipe: return "doubleSweepWipe";
	case ambulant::lib::saloonDoorWipe: return "saloonDoorWipe";
	case ambulant::lib::windshieldWipe: return "windshieldWipe";
	case ambulant::lib::snakeWipe: return "snakeWipe";
	case ambulant::lib::spiralWipe: return "spiralWipe";
	case ambulant::lib::parallelSnakesWipe: return "parallelSnakesWipe";
	case ambulant::lib::boxSnakesWipe: return "boxSnakesWipe";
	case ambulant::lib::waterfallWipe: return "waterfallWipe";
	case ambulant::lib::pushWipe: return "pushWipe";
	case ambulant::lib::slideWipe: return "slideWipe";
	case ambulant::lib::fade: return "fade";
	case ambulant::lib::audioFade: return "audioFade";
	case ambulant::lib::audioVisualFade: return "audioVisualFade";
	default: return "<unknown transition type>";
	}
}

#endif // AMBULANT_LIB_TRANSITION_INFO_H
