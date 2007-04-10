/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_TRANSITION_INFO_H
#define AMBULANT_LIB_TRANSITION_INFO_H

#include "ambulant/lib/colors.h"
#include "ambulant/lib/node.h"

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

std::string repr(transition_type t);

/// Stores all information regarind a specific transition.
class AMBULANTAPI transition_info {
  public:
	typedef int time_type;
	typedef double progress_type;
	
	transition_type m_type;    ///< SMIL 2.1 transition type.
	std::string m_subtype;     ///< SMIL 2.1 transition subtype.
	time_type m_dur;           ///< Transition duration (Unit???)
	progress_type m_startProgress; ///< Starting point of transition.
	progress_type m_endProgress;   ///< Ending point of transition.
	bool m_reverse;            ///< True if transition orientation is reversed.
	lib::color_t m_color;      ///< Color, if transition needs it.
	transition_scope m_scope;  ///< Should transition be region or whole-window?
	// We should also have the <param> contents here

	transition_info() { }
	transition_info(transition_info* info) { *this = *info; }
	
	/// Factory function: obtain transition_info object from DOM node.
	static transition_info *from_node(const node *n);

 private:
	static time_type get_trans_dur(const node *n);
	static progress_type get_progress(const node *n, const char* progress, progress_type default_value);
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_TRANSITION_INFO_H
