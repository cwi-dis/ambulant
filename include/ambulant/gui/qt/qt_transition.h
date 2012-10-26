/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
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

#ifndef AMBULANT_GUI_QT_QT_TRANSITION_H
#define AMBULANT_GUI_QT_QT_TRANSITION_H

#include "ambulant/smil2/transition.h"
#include "ambulant/common/layout.h"
#include "ambulant/gui/qt/qt_factory_impl.h"
#include "ambulant/gui/qt/qt_includes.h" // TMP
namespace ambulant {

namespace gui {

namespace qt {

class qt_transition_debug { // TMP
  public:
	void paint_rect(ambulant_qt_window* aqw,
			common::surface * dst,
			lib::color_t color);
};

class qt_transition_blitclass_fade : virtual public smil2::transition_blitclass_fade {
  protected:
	void update();
};

class qt_transition_blitclass_rect : virtual public smil2::transition_blitclass_rect {
  protected:
	void update();
};

class qt_transition_blitclass_r1r2r3r4 : virtual public smil2::transition_blitclass_r1r2r3r4 {
  protected:
	void update();
};

class qt_transition_blitclass_rectlist : virtual public smil2::transition_blitclass_rectlist {
  protected:
	void update();
};

class qt_transition_blitclass_poly : virtual public smil2::transition_blitclass_poly {
  protected:
	void update();
};

class qt_transition_blitclass_polylist : virtual public smil2::transition_blitclass_polylist {
  protected:
	void update();
};


// Series 1: edge wipes

class qt_transition_engine_barwipe :
	virtual public qt_transition_blitclass_rect,
	virtual public smil2::transition_engine_barwipe {};

class qt_transition_engine_boxwipe :
	virtual public qt_transition_blitclass_rect,
	virtual public smil2::transition_engine_boxwipe {};

class qt_transition_engine_fourboxwipe :
	virtual public qt_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_fourboxwipe {};

class qt_transition_engine_barndoorwipe :
	virtual public qt_transition_blitclass_rect,
	virtual public smil2::transition_engine_barndoorwipe {};

class qt_transition_engine_diagonalwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_diagonalwipe {};

class qt_transition_engine_miscdiagonalwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscdiagonalwipe {};

class qt_transition_engine_veewipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_veewipe {};

class qt_transition_engine_barnveewipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnveewipe {};

class qt_transition_engine_zigzagwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_zigzagwipe {};

class qt_transition_engine_barnzigzagwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnzigzagwipe {};

class qt_transition_engine_bowtiewipe :
	virtual public qt_transition_blitclass_polylist,
	virtual public smil2::transition_engine_bowtiewipe {};

// series 2: iris wipes
class qt_transition_engine_iriswipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_iriswipe {};

class qt_transition_engine_pentagonwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_pentagonwipe {};

class qt_transition_engine_arrowheadwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_arrowheadwipe {};

class qt_transition_engine_trianglewipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_trianglewipe {};

class qt_transition_engine_hexagonwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_hexagonwipe {};

class qt_transition_engine_eyewipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_eyewipe {};

class qt_transition_engine_roundrectwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_roundrectwipe {};

class qt_transition_engine_ellipsewipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_ellipsewipe {};

class qt_transition_engine_starwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_starwipe {};

class qt_transition_engine_miscshapewipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscshapewipe {};


// series 3: clock-type wipes

class qt_transition_engine_clockwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_clockwipe {};

class qt_transition_engine_singlesweepwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_singlesweepwipe {};

class qt_transition_engine_doublesweepwipe :
	virtual public qt_transition_blitclass_polylist,
	virtual public smil2::transition_engine_doublesweepwipe {};

class qt_transition_engine_saloondoorwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_saloondoorwipe {};

class qt_transition_engine_windshieldwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_windshieldwipe {};

class qt_transition_engine_fanwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_fanwipe {};

class qt_transition_engine_doublefanwipe :
	virtual public qt_transition_blitclass_poly,
	virtual public smil2::transition_engine_doublefanwipe {};

class qt_transition_engine_pinwheelwipe :
	virtual public qt_transition_blitclass_polylist,
	virtual public smil2::transition_engine_pinwheelwipe {};

// series 4: matrix wipe types

class qt_transition_engine_snakewipe :
	virtual public qt_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_snakewipe {};

class qt_transition_engine_waterfallwipe :
	virtual public qt_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_waterfallwipe {};

class qt_transition_engine_spiralwipe :
	virtual public qt_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_spiralwipe {};

class qt_transition_engine_parallelsnakeswipe :
	virtual public qt_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_parallelsnakeswipe {};

class qt_transition_engine_boxsnakeswipe :
	virtual public qt_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_boxsnakeswipe {};

// series 5: SMIL-specific types

class qt_transition_engine_pushwipe :
	virtual public qt_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_pushwipe {};

class qt_transition_engine_slidewipe :
	virtual public qt_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_slidewipe {};

class qt_transition_engine_fade :
	virtual public qt_transition_blitclass_fade,
	virtual public smil2::transition_engine_fade {};

smil2::transition_engine *qt_transition_engine(
	common::surface *dst, bool is_outtrans, const lib::transition_info *info);

} // namespace qt

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_QT_QT_TRANSITION_H
