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

#ifndef AMBULANT_GUI_QT_QT_TRANSITION_H
#define AMBULANT_GUI_QT_QT_TRANSITION_H

#include "ambulant/smil2/transition.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/layout.h"
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
	common::surface *dst, bool is_outtrans, lib::transition_info *info);
	
} // namespace qt

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_QT_QT_TRANSITION_H
