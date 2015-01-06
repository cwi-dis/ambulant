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

#ifndef AMBULANT_GUI_D2_D2_TRANSITION_H
#define AMBULANT_GUI_D2_D2_TRANSITION_H

#include "ambulant/smil2/transition.h"
#include "ambulant/common/layout.h"
#include "ambulant/gui/d2/d2_renderer.h"

namespace ambulant {

namespace gui {

namespace d2 {

class d2_transition_blitclass_fade : virtual public smil2::transition_blitclass_fade
{
  protected:
	void update();
};

class d2_transition_blitclass_rect : virtual public smil2::transition_blitclass_rect {
  protected:
	void update();
};

class d2_transition_blitclass_r1r2r3r4 : virtual public smil2::transition_blitclass_r1r2r3r4 {
  protected:
	void update();
};

class d2_transition_blitclass_rectlist : virtual public smil2::transition_blitclass_rectlist {
  protected:
	void update();
};

class d2_transition_blitclass_poly : virtual public smil2::transition_blitclass_poly {
  protected:
	void update();
};

class d2_transition_blitclass_polylist : virtual public smil2::transition_blitclass_polylist {
  protected:
	void update();
};

// Series 1: edge wipes

class d2_transition_engine_barwipe :
	virtual public d2_transition_blitclass_rect,
	virtual public smil2::transition_engine_barwipe {};

class d2_transition_engine_boxwipe :
	virtual public d2_transition_blitclass_rect,
	virtual public smil2::transition_engine_boxwipe {};

class d2_transition_engine_fourboxwipe :
	virtual public d2_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_fourboxwipe {};

class d2_transition_engine_barndoorwipe :
	virtual public d2_transition_blitclass_rect,
	virtual public smil2::transition_engine_barndoorwipe {};

class d2_transition_engine_diagonalwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_diagonalwipe {};

class d2_transition_engine_miscdiagonalwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscdiagonalwipe {};

class d2_transition_engine_veewipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_veewipe {};

class d2_transition_engine_barnveewipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnveewipe {};

class d2_transition_engine_zigzagwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_zigzagwipe {};

class d2_transition_engine_barnzigzagwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnzigzagwipe {};

class d2_transition_engine_bowtiewipe :
	virtual public d2_transition_blitclass_polylist,
	virtual public smil2::transition_engine_bowtiewipe {};

// series 2: iris wipes
class d2_transition_engine_iriswipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_iriswipe {};

class d2_transition_engine_pentagonwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_pentagonwipe {};

class d2_transition_engine_arrowheadwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_arrowheadwipe {};

class d2_transition_engine_trianglewipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_trianglewipe {};

class d2_transition_engine_hexagonwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_hexagonwipe {};

class d2_transition_engine_eyewipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_eyewipe {};

class d2_transition_engine_roundrectwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_roundrectwipe {};

class d2_transition_engine_ellipsewipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_ellipsewipe {};

class d2_transition_engine_starwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_starwipe {};

class d2_transition_engine_miscshapewipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscshapewipe {};


// series 3: clock-type wipes

class d2_transition_engine_clockwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_clockwipe {};

class d2_transition_engine_singlesweepwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_singlesweepwipe {};

class d2_transition_engine_doublesweepwipe :
	virtual public d2_transition_blitclass_polylist,
	virtual public smil2::transition_engine_doublesweepwipe {};

class d2_transition_engine_saloondoorwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_saloondoorwipe {};

class d2_transition_engine_windshieldwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_windshieldwipe {};

class d2_transition_engine_fanwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_fanwipe {};

class d2_transition_engine_doublefanwipe :
	virtual public d2_transition_blitclass_poly,
	virtual public smil2::transition_engine_doublefanwipe {};

class d2_transition_engine_pinwheelwipe :
	virtual public d2_transition_blitclass_polylist,
	virtual public smil2::transition_engine_pinwheelwipe {};

// series 4: matrix wipe types

class d2_transition_engine_snakewipe :
	virtual public d2_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_snakewipe {};

class d2_transition_engine_waterfallwipe :
	virtual public d2_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_waterfallwipe {};

class d2_transition_engine_spiralwipe :
	virtual public d2_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_spiralwipe {};

class d2_transition_engine_parallelsnakeswipe :
	virtual public d2_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_parallelsnakeswipe {};

class d2_transition_engine_boxsnakeswipe :
	virtual public d2_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_boxsnakeswipe {};

// series 5: SMIL-specific types

class d2_transition_engine_pushwipe :
	virtual public d2_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_pushwipe {};

class d2_transition_engine_slidewipe :
	virtual public d2_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_slidewipe {};

class d2_transition_engine_fade :
	virtual public d2_transition_blitclass_fade,
	virtual public smil2::transition_engine_fade {};

smil2::transition_engine *d2_transition_engine(
	common::surface *dst, bool is_outtrans, const lib::transition_info *info);

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_D2_TRANSITION_H
