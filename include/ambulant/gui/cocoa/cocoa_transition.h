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

class cocoa_transition_blitclass_r1r2 : virtual public smil2::transition_blitclass_r1r2 {
  protected:
	void update();
};

class cocoa_transition_blitclass_r1r2r3r4 : virtual public smil2::transition_blitclass_r1r2r3r4 {
  protected:
	void update();
};

class cocoa_transition_blitclass_rlistr2 : virtual public smil2::transition_blitclass_rlistr2 {
  protected:
	void update();
};

class cocoa_transition_blitclass_polyr2 : virtual public smil2::transition_blitclass_polyr2 {
  protected:
	void update();
};

class cocoa_transition_blitclass_polylistr2 : virtual public smil2::transition_blitclass_polylistr2 {
  protected:
	void update();
};

class cocoa_transition_engine_barwipe :
	virtual public cocoa_transition_blitclass_r1r2,
	virtual public smil2::transition_engine_barwipe {};

class cocoa_transition_engine_boxwipe :
	virtual public cocoa_transition_blitclass_r1r2,
	virtual public smil2::transition_engine_boxwipe {};

class cocoa_transition_engine_fourboxwipe :
	virtual public cocoa_transition_blitclass_rlistr2,
	virtual public smil2::transition_engine_fourboxwipe {};

class cocoa_transition_engine_barndoorwipe :
	virtual public cocoa_transition_blitclass_r1r2,
	virtual public smil2::transition_engine_barndoorwipe {};

class cocoa_transition_engine_diagonalwipe :
	virtual public cocoa_transition_blitclass_polyr2,
	virtual public smil2::transition_engine_diagonalwipe {};

class cocoa_transition_engine_miscdiagonalwipe :
	virtual public cocoa_transition_blitclass_polyr2,
	virtual public smil2::transition_engine_miscdiagonalwipe {};

class cocoa_transition_engine_veewipe :
	virtual public cocoa_transition_blitclass_polyr2,
	virtual public smil2::transition_engine_veewipe {};

class cocoa_transition_engine_barnveewipe :
	virtual public cocoa_transition_blitclass_polyr2,
	virtual public smil2::transition_engine_barnveewipe {};

class cocoa_transition_engine_zigzagwipe :
	virtual public cocoa_transition_blitclass_polyr2,
	virtual public smil2::transition_engine_zigzagwipe {};

class cocoa_transition_engine_barnzigzagwipe :
	virtual public cocoa_transition_blitclass_polyr2,
	virtual public smil2::transition_engine_barnzigzagwipe {};

class cocoa_transition_engine_bowtiewipe :
	virtual public cocoa_transition_blitclass_polylistr2,
	virtual public smil2::transition_engine_bowtiewipe {};

class cocoa_transition_engine_doublesweepwipe :
	virtual public cocoa_transition_blitclass_polylistr2,
	virtual public smil2::transition_engine_doublesweepwipe {};

class cocoa_transition_engine_saloondoorwipe :
	virtual public cocoa_transition_blitclass_polyr2,
	virtual public smil2::transition_engine_saloondoorwipe {};

class cocoa_transition_engine_windshieldwipe :
	virtual public cocoa_transition_blitclass_polyr2,
	virtual public smil2::transition_engine_windshieldwipe {};

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
	common::surface *dst, bool is_outtrans, lib::transition_info *info);
	
} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_TRANSITION_H
