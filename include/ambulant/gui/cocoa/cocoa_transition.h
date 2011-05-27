/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_COCOA_COCOA_TRANSITION_H
#define AMBULANT_GUI_COCOA_COCOA_TRANSITION_H

#include "ambulant/smil2/transition.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace gui {

namespace cocoa {

class cocoa_transition_blitclass_fade : virtual public smil2::transition_blitclass_fade {
  protected:
	void update();
};

class cocoa_transition_blitclass_rect : virtual public smil2::transition_blitclass_rect {
  protected:
	void update();
};

class cocoa_transition_blitclass_r1r2r3r4 : virtual public smil2::transition_blitclass_r1r2r3r4 {
  protected:
	void update();
};

class cocoa_transition_blitclass_rectlist : virtual public smil2::transition_blitclass_rectlist {
  protected:
	void update();
};

class cocoa_transition_blitclass_poly : virtual public smil2::transition_blitclass_poly {
  protected:
	void update();
};

class cocoa_transition_blitclass_polylist : virtual public smil2::transition_blitclass_polylist {
  protected:
	void update();
};

// Series 1: edge wipes

class cocoa_transition_engine_barwipe :
	virtual public cocoa_transition_blitclass_rect,
	virtual public smil2::transition_engine_barwipe {};

class cocoa_transition_engine_boxwipe :
	virtual public cocoa_transition_blitclass_rect,
	virtual public smil2::transition_engine_boxwipe {};

class cocoa_transition_engine_fourboxwipe :
	virtual public cocoa_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_fourboxwipe {};

class cocoa_transition_engine_barndoorwipe :
	virtual public cocoa_transition_blitclass_rect,
	virtual public smil2::transition_engine_barndoorwipe {};

class cocoa_transition_engine_diagonalwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_diagonalwipe {};

class cocoa_transition_engine_miscdiagonalwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscdiagonalwipe {};

class cocoa_transition_engine_veewipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_veewipe {};

class cocoa_transition_engine_barnveewipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnveewipe {};

class cocoa_transition_engine_zigzagwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_zigzagwipe {};

class cocoa_transition_engine_barnzigzagwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnzigzagwipe {};

class cocoa_transition_engine_bowtiewipe :
	virtual public cocoa_transition_blitclass_polylist,
	virtual public smil2::transition_engine_bowtiewipe {};

// series 2: iris wipes
class cocoa_transition_engine_iriswipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_iriswipe {};

class cocoa_transition_engine_pentagonwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_pentagonwipe {};

class cocoa_transition_engine_arrowheadwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_arrowheadwipe {};

class cocoa_transition_engine_trianglewipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_trianglewipe {};

class cocoa_transition_engine_hexagonwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_hexagonwipe {};

class cocoa_transition_engine_eyewipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_eyewipe {};

class cocoa_transition_engine_roundrectwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_roundrectwipe {};

class cocoa_transition_engine_ellipsewipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_ellipsewipe {};

class cocoa_transition_engine_starwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_starwipe {};

class cocoa_transition_engine_miscshapewipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscshapewipe {};


// series 3: clock-type wipes

class cocoa_transition_engine_clockwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_clockwipe {};

class cocoa_transition_engine_singlesweepwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_singlesweepwipe {};

class cocoa_transition_engine_doublesweepwipe :
	virtual public cocoa_transition_blitclass_polylist,
	virtual public smil2::transition_engine_doublesweepwipe {};

class cocoa_transition_engine_saloondoorwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_saloondoorwipe {};

class cocoa_transition_engine_windshieldwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_windshieldwipe {};

class cocoa_transition_engine_fanwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_fanwipe {};

class cocoa_transition_engine_doublefanwipe :
	virtual public cocoa_transition_blitclass_poly,
	virtual public smil2::transition_engine_doublefanwipe {};

class cocoa_transition_engine_pinwheelwipe :
	virtual public cocoa_transition_blitclass_polylist,
	virtual public smil2::transition_engine_pinwheelwipe {};

// series 4: matrix wipe types

class cocoa_transition_engine_snakewipe :
	virtual public cocoa_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_snakewipe {};

class cocoa_transition_engine_waterfallwipe :
	virtual public cocoa_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_waterfallwipe {};

class cocoa_transition_engine_spiralwipe :
	virtual public cocoa_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_spiralwipe {};

class cocoa_transition_engine_parallelsnakeswipe :
	virtual public cocoa_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_parallelsnakeswipe {};

class cocoa_transition_engine_boxsnakeswipe :
	virtual public cocoa_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_boxsnakeswipe {};

// series 5: SMIL-specific types

class cocoa_transition_engine_pushwipe :
	virtual public cocoa_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_pushwipe {};

class cocoa_transition_engine_slidewipe :
	virtual public cocoa_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_slidewipe {};

class cocoa_transition_engine_fade :
	virtual public cocoa_transition_blitclass_fade,
	virtual public smil2::transition_engine_fade {};

smil2::transition_engine *cocoa_transition_engine(
	common::surface *dst, bool is_outtrans, const lib::transition_info *info);

} // namespace cocoa

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_TRANSITION_H
