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

#ifndef AMBULANT_GUI_CG_CG_TRANSITION_H
#define AMBULANT_GUI_CG_CG_TRANSITION_H

#include "ambulant/smil2/transition.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace gui {

namespace cg {

class cg_transition_blitclass_fade : virtual public smil2::transition_blitclass_fade
{
  protected:
	void update();
};

class cg_transition_blitclass_rect : virtual public smil2::transition_blitclass_rect {
  protected:
	void update();
};

class cg_transition_blitclass_r1r2r3r4 : virtual public smil2::transition_blitclass_r1r2r3r4 {
  protected:
	void update();
};

class cg_transition_blitclass_rectlist : virtual public smil2::transition_blitclass_rectlist {
  protected:
	void update();
};

class cg_transition_blitclass_poly : virtual public smil2::transition_blitclass_poly {
  protected:
	void update();
};

class cg_transition_blitclass_polylist : virtual public smil2::transition_blitclass_polylist {
  protected:
	void update();
};

// Series 1: edge wipes

class cg_transition_engine_barwipe :
	virtual public cg_transition_blitclass_rect,
	virtual public smil2::transition_engine_barwipe {};

class cg_transition_engine_boxwipe :
	virtual public cg_transition_blitclass_rect,
	virtual public smil2::transition_engine_boxwipe {};

class cg_transition_engine_fourboxwipe :
	virtual public cg_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_fourboxwipe {};

class cg_transition_engine_barndoorwipe :
	virtual public cg_transition_blitclass_rect,
	virtual public smil2::transition_engine_barndoorwipe {};

class cg_transition_engine_diagonalwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_diagonalwipe {};

class cg_transition_engine_miscdiagonalwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscdiagonalwipe {};

class cg_transition_engine_veewipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_veewipe {};

class cg_transition_engine_barnveewipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnveewipe {};

class cg_transition_engine_zigzagwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_zigzagwipe {};

class cg_transition_engine_barnzigzagwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnzigzagwipe {};

class cg_transition_engine_bowtiewipe :
	virtual public cg_transition_blitclass_polylist,
	virtual public smil2::transition_engine_bowtiewipe {};

// series 2: iris wipes
class cg_transition_engine_iriswipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_iriswipe {};

class cg_transition_engine_pentagonwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_pentagonwipe {};

class cg_transition_engine_arrowheadwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_arrowheadwipe {};

class cg_transition_engine_trianglewipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_trianglewipe {};

class cg_transition_engine_hexagonwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_hexagonwipe {};

class cg_transition_engine_eyewipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_eyewipe {};

class cg_transition_engine_roundrectwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_roundrectwipe {};

class cg_transition_engine_ellipsewipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_ellipsewipe {};

class cg_transition_engine_starwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_starwipe {};

class cg_transition_engine_miscshapewipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscshapewipe {};


// series 3: clock-type wipes

class cg_transition_engine_clockwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_clockwipe {};

class cg_transition_engine_singlesweepwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_singlesweepwipe {};

class cg_transition_engine_doublesweepwipe :
	virtual public cg_transition_blitclass_polylist,
	virtual public smil2::transition_engine_doublesweepwipe {};

class cg_transition_engine_saloondoorwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_saloondoorwipe {};

class cg_transition_engine_windshieldwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_windshieldwipe {};

class cg_transition_engine_fanwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_fanwipe {};

class cg_transition_engine_doublefanwipe :
	virtual public cg_transition_blitclass_poly,
	virtual public smil2::transition_engine_doublefanwipe {};

class cg_transition_engine_pinwheelwipe :
	virtual public cg_transition_blitclass_polylist,
	virtual public smil2::transition_engine_pinwheelwipe {};

// series 4: matrix wipe types

class cg_transition_engine_snakewipe :
	virtual public cg_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_snakewipe {};

class cg_transition_engine_waterfallwipe :
	virtual public cg_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_waterfallwipe {};

class cg_transition_engine_spiralwipe :
	virtual public cg_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_spiralwipe {};

class cg_transition_engine_parallelsnakeswipe :
	virtual public cg_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_parallelsnakeswipe {};

class cg_transition_engine_boxsnakeswipe :
	virtual public cg_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_boxsnakeswipe {};

// series 5: SMIL-specific types

class cg_transition_engine_pushwipe :
	virtual public cg_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_pushwipe {};

class cg_transition_engine_slidewipe :
	virtual public cg_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_slidewipe {};

class cg_transition_engine_fade :
	virtual public cg_transition_blitclass_fade,
	virtual public smil2::transition_engine_fade {};

smil2::transition_engine *cg_transition_engine(
	common::surface *dst, bool is_outtrans, const lib::transition_info *info);

} // namespace cg

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_CG_CG_TRANSITION_H
