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

#ifndef AMBULANT_GUI_GTK_GTK_TRANSITION_H
#define AMBULANT_GUI_GTK_GTK_TRANSITION_H

#include "ambulant/smil2/transition.h"
#include "ambulant/common/layout.h"
#include "ambulant/gui/gtk/gtk_factory.h"

namespace ambulant {

namespace gui {

namespace gtk {

class gtk_transition_debug { // TMP
  public:
	void paint_rect(ambulant_gtk_window* aqw,
			common::surface * dst,
			lib::color_t color);
};

class gtk_transition_blitclass_fade : virtual public smil2::transition_blitclass_fade {
  public:
  protected:
	void update();
};

class gtk_transition_blitclass_rect : virtual public smil2::transition_blitclass_rect {
  protected:
	void update();
};

class gtk_transition_blitclass_r1r2r3r4 : virtual public smil2::transition_blitclass_r1r2r3r4 {
  protected:
	void update();
};

class gtk_transition_blitclass_rectlist : virtual public smil2::transition_blitclass_rectlist {
  protected:
	void update();
};

class gtk_transition_blitclass_poly : virtual public smil2::transition_blitclass_poly {
  protected:
	void update();
};

class gtk_transition_blitclass_polylist : virtual public smil2::transition_blitclass_polylist {
  protected:
	void update();
};


// Series 1: edge wipes

class gtk_transition_engine_barwipe :
	virtual public gtk_transition_blitclass_rect,
	virtual public smil2::transition_engine_barwipe {};

class gtk_transition_engine_boxwipe :
	virtual public gtk_transition_blitclass_rect,
	virtual public smil2::transition_engine_boxwipe {};

class gtk_transition_engine_fourboxwipe :
	virtual public gtk_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_fourboxwipe {};

class gtk_transition_engine_barndoorwipe :
	virtual public gtk_transition_blitclass_rect,
	virtual public smil2::transition_engine_barndoorwipe {};

class gtk_transition_engine_diagonalwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_diagonalwipe {};

class gtk_transition_engine_miscdiagonalwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscdiagonalwipe {};

class gtk_transition_engine_veewipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_veewipe {};

class gtk_transition_engine_barnveewipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnveewipe {};

class gtk_transition_engine_zigzagwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_zigzagwipe {};

class gtk_transition_engine_barnzigzagwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnzigzagwipe {};

class gtk_transition_engine_bowtiewipe :
	virtual public gtk_transition_blitclass_polylist,
	virtual public smil2::transition_engine_bowtiewipe {};

// series 2: iris wipes
class gtk_transition_engine_iriswipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_iriswipe {};

class gtk_transition_engine_pentagonwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_pentagonwipe {};

class gtk_transition_engine_arrowheadwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_arrowheadwipe {};

class gtk_transition_engine_trianglewipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_trianglewipe {};

class gtk_transition_engine_hexagonwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_hexagonwipe {};

class gtk_transition_engine_eyewipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_eyewipe {};

class gtk_transition_engine_roundrectwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_roundrectwipe {};

class gtk_transition_engine_ellipsewipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_ellipsewipe {};

class gtk_transition_engine_starwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_starwipe {};

class gtk_transition_engine_miscshapewipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscshapewipe {};


// series 3: clock-type wipes

class gtk_transition_engine_clockwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_clockwipe {};

class gtk_transition_engine_singlesweepwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_singlesweepwipe {};

class gtk_transition_engine_doublesweepwipe :
	virtual public gtk_transition_blitclass_polylist,
	virtual public smil2::transition_engine_doublesweepwipe {};

class gtk_transition_engine_saloondoorwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_saloondoorwipe {};

class gtk_transition_engine_windshieldwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_windshieldwipe {};

class gtk_transition_engine_fanwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_fanwipe {};

class gtk_transition_engine_doublefanwipe :
	virtual public gtk_transition_blitclass_poly,
	virtual public smil2::transition_engine_doublefanwipe {};

class gtk_transition_engine_pinwheelwipe :
	virtual public gtk_transition_blitclass_polylist,
	virtual public smil2::transition_engine_pinwheelwipe {};

// series 4: matrix wipe types

class gtk_transition_engine_snakewipe :
	virtual public gtk_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_snakewipe {};

class gtk_transition_engine_waterfallwipe :
	virtual public gtk_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_waterfallwipe {};

class gtk_transition_engine_spiralwipe :
	virtual public gtk_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_spiralwipe {};

class gtk_transition_engine_parallelsnakeswipe :
	virtual public gtk_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_parallelsnakeswipe {};

class gtk_transition_engine_boxsnakeswipe :
	virtual public gtk_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_boxsnakeswipe {};

// series 5: SMIL-specific types

class gtk_transition_engine_pushwipe :
	virtual public gtk_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_pushwipe {};

class gtk_transition_engine_slidewipe :
	virtual public gtk_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_slidewipe {};

class gtk_transition_engine_fade :
	virtual public gtk_transition_blitclass_fade,
	virtual public smil2::transition_engine_fade {};

smil2::transition_engine *gtk_transition_engine(
	common::surface *dst, bool is_outtrans, const lib::transition_info *info);

} // namespace gtk

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_GTK_GTK_TRANSITION_H
