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

#ifndef AMBULANT_GUI_SDL_SDL_TRANSITION_H
#define AMBULANT_GUI_SDL_SDL_TRANSITION_H

#ifdef  WITH_SDL_IMAGE

#include "ambulant/smil2/transition.h"
#include "ambulant/common/layout.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_window.h"

#include "SDL.h"
namespace ambulant {

namespace gui {

namespace sdl {

class sdl_transition_debug { // TMP
  public:
	void paint_rect(ambulant_sdl_window* aqw,
			common::surface * dst,
			lib::color_t color);
};

class sdl_transition_blitclass_fade : virtual public smil2::transition_blitclass_fade {
  public:
  protected:
	void update();
};

class sdl_transition_blitclass_rect : virtual public smil2::transition_blitclass_rect {
  protected:
	void update();
};

class sdl_transition_blitclass_r1r2r3r4 : virtual public smil2::transition_blitclass_r1r2r3r4 {
  protected:
	void update();
};

class sdl_transition_blitclass_rectlist : virtual public smil2::transition_blitclass_rectlist {
  protected:
	void update();
};

class sdl_transition_blitclass_poly : virtual public smil2::transition_blitclass_poly {
  protected:
	void update();
};

class sdl_transition_blitclass_polylist : virtual public smil2::transition_blitclass_polylist {
  protected:
	void update();
};


// Series 1: edge wipes

class sdl_transition_engine_barwipe :
	virtual public sdl_transition_blitclass_rect,
	virtual public smil2::transition_engine_barwipe {};

class sdl_transition_engine_boxwipe :
	virtual public sdl_transition_blitclass_rect,
	virtual public smil2::transition_engine_boxwipe {};

class sdl_transition_engine_fourboxwipe :
	virtual public sdl_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_fourboxwipe {};

class sdl_transition_engine_barndoorwipe :
	virtual public sdl_transition_blitclass_rect,
	virtual public smil2::transition_engine_barndoorwipe {};

class sdl_transition_engine_diagonalwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_diagonalwipe {};

class sdl_transition_engine_miscdiagonalwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscdiagonalwipe {};

class sdl_transition_engine_veewipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_veewipe {};

class sdl_transition_engine_barnveewipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnveewipe {};

class sdl_transition_engine_zigzagwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_zigzagwipe {};

class sdl_transition_engine_barnzigzagwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_barnzigzagwipe {};

class sdl_transition_engine_bowtiewipe :
	virtual public sdl_transition_blitclass_polylist,
	virtual public smil2::transition_engine_bowtiewipe {};

// series 2: iris wipes
class sdl_transition_engine_iriswipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_iriswipe {};

class sdl_transition_engine_pentagonwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_pentagonwipe {};

class sdl_transition_engine_arrowheadwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_arrowheadwipe {};

class sdl_transition_engine_trianglewipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_trianglewipe {};

class sdl_transition_engine_hexagonwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_hexagonwipe {};

class sdl_transition_engine_eyewipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_eyewipe {};

class sdl_transition_engine_roundrectwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_roundrectwipe {};

class sdl_transition_engine_ellipsewipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_ellipsewipe {};

class sdl_transition_engine_starwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_starwipe {};

class sdl_transition_engine_miscshapewipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_miscshapewipe {};


// series 3: clock-type wipes

class sdl_transition_engine_clockwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_clockwipe {};

class sdl_transition_engine_singlesweepwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_singlesweepwipe {};

class sdl_transition_engine_doublesweepwipe :
	virtual public sdl_transition_blitclass_polylist,
	virtual public smil2::transition_engine_doublesweepwipe {};

class sdl_transition_engine_saloondoorwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_saloondoorwipe {};

class sdl_transition_engine_windshieldwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_windshieldwipe {};

class sdl_transition_engine_fanwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_fanwipe {};

class sdl_transition_engine_doublefanwipe :
	virtual public sdl_transition_blitclass_poly,
	virtual public smil2::transition_engine_doublefanwipe {};

class sdl_transition_engine_pinwheelwipe :
	virtual public sdl_transition_blitclass_polylist,
	virtual public smil2::transition_engine_pinwheelwipe {};

// series 4: matrix wipe types

class sdl_transition_engine_snakewipe :
	virtual public sdl_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_snakewipe {};

class sdl_transition_engine_waterfallwipe :
	virtual public sdl_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_waterfallwipe {};

class sdl_transition_engine_spiralwipe :
	virtual public sdl_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_spiralwipe {};

class sdl_transition_engine_parallelsnakeswipe :
	virtual public sdl_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_parallelsnakeswipe {};

class sdl_transition_engine_boxsnakeswipe :
	virtual public sdl_transition_blitclass_rectlist,
	virtual public smil2::transition_engine_boxsnakeswipe {};

// series 5: SMIL-specific types

class sdl_transition_engine_pushwipe :
	virtual public sdl_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_pushwipe {};

class sdl_transition_engine_slidewipe :
	virtual public sdl_transition_blitclass_r1r2r3r4,
	virtual public smil2::transition_engine_slidewipe {};

class sdl_transition_engine_fade :
	virtual public sdl_transition_blitclass_fade,
	virtual public smil2::transition_engine_fade {};

smil2::transition_engine *sdl_transition_engine(
	common::surface *dst, bool is_outtrans, const lib::transition_info *info);

} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif // WITH_SDL_IMAGE

#endif // AMBULANT_GUI_SDL_SDL_TRANSITION_H
