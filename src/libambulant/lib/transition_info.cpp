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

#include "ambulant/lib/logger.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/transition_info.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace lib;

transition_info *
transition_info::from_node(node *n)
{
	// Placeholder
	transition_info *rv = new transition_info();
#if 0
	const char *ctype = n->get_attribute("type");
	if (!ctype) {
		lib::logger::get_logger()->error("transition: no type");
		delete rv;
		return NULL;
	}
	std::string type = ctype;
	if (type == "barWipe") m_type = barWipe;
	else if (type == "boxWipe") m_type = boxWipe;
	else if (type == "fourBoxWipe") m_type = fourBoxWipe;
	else if (type == "barnDoorWipe") m_type = barnDoorWipe;
	else if (type == "diagonalWipe") m_type = diagonalWipe;
	else if (type == "bowTieWipe") m_type = bowTieWipe;
	else if (type == "miscDiagonalWipe") m_type = miscDiagonalWipe;
	else if (type == "veeWipe") m_type = veeWipe;
	else if (type == "barnVeeWipe") m_type = barnVeeWipe;
	else if (type == "zigZagWipe") m_type = zigZagWipe;
	else if (type == "barnZigZagWipe") m_type = barnZigZagWipe;
	else if (type == "irisWipe") m_type = irisWipe;
	else if (type == "triangleWipe") m_type = triangleWipe;
	else if (type == "arrowHeadWipe") m_type = arrowHeadWipe;
	else if (type == "pentagonWipe") m_type = pentagonWipe;
	else if (type == "hexagonWipe") m_type = hexagonWipe;
	else if (type == "ellipseWipe") m_type = ellipseWipe;
	else if (type == "eyeWiperoundRectWipe") m_type = eyeWiperoundRectWipe;
	else if (type == "starWipe") m_type = starWipe;
	else if (type == "miscShapeWipe") m_type = miscShapeWipe;
	else if (type == "clockWipe") m_type = clockWipe;
	else if (type == "pinWheelWipe") m_type = pinWheelWipe;
	else if (type == "singleSweepWipe") m_type = singleSweepWipe;
	else if (type == "fanWipe") m_type = fanWipe;
	else if (type == "doubleFanWipe") m_type = doubleFanWipe;
	else if (type == "doubleSweepWipe") m_type = doubleSweepWipe;
	else if (type == "saloonDoorWipe") m_type = saloonDoorWipe;
	else if (type == "windshieldWipe") m_type = windshieldWipe;
	else if (type == "snakeWipe") m_type = snakeWipe;
	else if (type == "spiralWipe") m_type = spiralWipe;
	else if (type == "parallelSnakesWipe") m_type = parallelSnakesWipe;
	else if (type == "boxSnakesWipe") m_type = boxSnakesWipe;
	else if (type == "waterfallWipe") m_type = waterfallWipe;
	else if (type == "pushWipe") m_type = pushWipe;
	else if (type == "slideWipe") m_type = slideWipe;
	else if (type == "fade") m_type = fade;
	else {
		lib::logger::get_logger()->error("transition: unknown type=\"%s\"", ctype);
		delete rv;
		return NULL;
	}
	const char *csubtype = n->get_attribute("subtype");
	if (csubtype)
		rv->m_subtype = csubtype;
	else
		rv->m_subtype = "";
#else
	rv->m_type = barnDoorWipe;
	rv->m_subtype = "";
#endif
	rv->m_dur = 2000;
	rv->m_startProgress = 0.0;
	rv->m_endProgress = 1.0;
	rv->m_reverse = false;
	return rv;
}

std::string
ambulant::lib::repr(transition_type t)
{
	switch(t) {
	case barWipe: return "barWipe";
	case boxWipe: return "boxWipe";
	case fourBoxWipe: return "fourBoxWipe";
	case barnDoorWipe: return "barnDoorWipe";
	case diagonalWipe: return "diagonalWipe";
	case bowTieWipe: return "bowTieWipe";
	case miscDiagonalWipe: return "miscDiagonalWipe";
	case veeWipe: return "veeWipe";
	case barnVeeWipe: return "barnVeeWipe";
	case zigZagWipe: return "zigZagWipe";
	case barnZigZagWipe: return "barnZigZagWipe";
	case irisWipe: return "irisWipe";
	case triangleWipe: return "triangleWipe";
	case arrowHeadWipe: return "arrowHeadWipe";
	case pentagonWipe: return "pentagonWipe";
	case hexagonWipe: return "hexagonWipe";
	case ellipseWipe: return "ellipseWipe";
	case eyeWipe: return "eyeWipe";
	case roundRectWipe: return "roundRectWipe";
	case starWipe: return "starWipe";
	case miscShapeWipe: return "miscShapeWipe";
	case clockWipe: return "clockWipe";
	case pinWheelWipe: return "pinWheelWipe";
	case singleSweepWipe: return "singleSweepWipe";
	case fanWipe: return "fanWipe";
	case doubleFanWipe: return "doubleFanWipe";
	case doubleSweepWipe: return "doubleSweepWipe";
	case saloonDoorWipe: return "saloonDoorWipe";
	case windshieldWipe: return "windshieldWipe";
	case snakeWipe: return "snakeWipe";
	case spiralWipe: return "spiralWipe";
	case parallelSnakesWipe: return "parallelSnakesWipe";
	case boxSnakesWipe: return "boxSnakesWipe";
	case waterfallWipe: return "waterfallWipe";
	case pushWipe: return "pushWipe";
	case slideWipe: return "slideWipe";
	case fade: return "fade";
	default: return "<unknown transition type>";
	}
}

