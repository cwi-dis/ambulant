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
#include "ambulant/lib/parselets.h"
#include "ambulant/lib/transition_info.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace lib;

transition_info *
transition_info::from_node(const node *n)
{
	// Placeholder
	if (n == NULL) return NULL;
	transition_info *rv = new transition_info();

	const char *ctype = n->get_attribute("type");
	if (!ctype) {
		lib::logger::get_logger()->error(gettext("transition: no `type' attribute"));
		delete rv;
		return NULL;
	}
	std::string type = ctype;
	if (type == "barWipe") rv->m_type = barWipe;
	else if (type == "boxWipe") rv->m_type = boxWipe;
	else if (type == "fourBoxWipe") rv->m_type = fourBoxWipe;
	else if (type == "barnDoorWipe") rv->m_type = barnDoorWipe;
	else if (type == "diagonalWipe") rv->m_type = diagonalWipe;
	else if (type == "bowTieWipe") rv->m_type = bowTieWipe;
	else if (type == "miscDiagonalWipe") rv->m_type = miscDiagonalWipe;
	else if (type == "veeWipe") rv->m_type = veeWipe;
	else if (type == "barnVeeWipe") rv->m_type = barnVeeWipe;
	else if (type == "zigZagWipe") rv->m_type = zigZagWipe;
	else if (type == "barnZigZagWipe") rv->m_type = barnZigZagWipe;
	else if (type == "irisWipe") rv->m_type = irisWipe;
	else if (type == "triangleWipe") rv->m_type = triangleWipe;
	else if (type == "arrowHeadWipe") rv->m_type = arrowHeadWipe;
	else if (type == "pentagonWipe") rv->m_type = pentagonWipe;
	else if (type == "hexagonWipe") rv->m_type = hexagonWipe;
	else if (type == "ellipseWipe") rv->m_type = ellipseWipe;
	else if (type == "eyeWipe") rv->m_type = eyeWipe;
	else if (type == "roundRectWipe") rv->m_type = roundRectWipe;
	else if (type == "starWipe") rv->m_type = starWipe;
	else if (type == "miscShapeWipe") rv->m_type = miscShapeWipe;
	else if (type == "clockWipe") rv->m_type = clockWipe;
	else if (type == "pinWheelWipe") rv->m_type = pinWheelWipe;
	else if (type == "singleSweepWipe") rv->m_type = singleSweepWipe;
	else if (type == "fanWipe") rv->m_type = fanWipe;
	else if (type == "doubleFanWipe") rv->m_type = doubleFanWipe;
	else if (type == "doubleSweepWipe") rv->m_type = doubleSweepWipe;
	else if (type == "saloonDoorWipe") rv->m_type = saloonDoorWipe;
	else if (type == "windshieldWipe") rv->m_type = windshieldWipe;
	else if (type == "snakeWipe") rv->m_type = snakeWipe;
	else if (type == "spiralWipe") rv->m_type = spiralWipe;
	else if (type == "parallelSnakesWipe") rv->m_type = parallelSnakesWipe;
	else if (type == "boxSnakesWipe") rv->m_type = boxSnakesWipe;
	else if (type == "waterfallWipe") rv->m_type = waterfallWipe;
	else if (type == "pushWipe") rv->m_type = pushWipe;
	else if (type == "slideWipe") rv->m_type = slideWipe;
	else if (type == "fade") rv->m_type = fade;
#ifdef USE_SMIL21
	else if (type == "audioFade") rv->m_type = audioFade;
	else if (type == "audioVisualFade") rv->m_type = audioVisualFade;
#endif
	else {
		lib::logger::get_logger()->error(gettext("transition: unknown type=\"%s\""), ctype);
		delete rv;
		return NULL;
	}

	const char *csubtype = n->get_attribute("subtype");
	if (csubtype)
		rv->m_subtype = csubtype;
	else
		rv->m_subtype = "";

	rv->m_dur = get_trans_dur(n);

	rv->m_startProgress = get_progress(n, "startProgress", 0.0);
	rv->m_endProgress = get_progress(n, "endProgress", 1.0);
	rv->m_reverse = false;

#ifdef USE_SMIL21
	rv->m_scope = scope_region;
	const char *scope = n->get_attribute("scope");
	if (scope) {
		if (strcmp(scope, "region") == 0) rv->m_scope = scope_region;
		else if (strcmp(scope, "screen") == 0) rv->m_scope = scope_screen;
		else lib::logger::get_logger()->error(gettext("transition: unknown scope=\"%s\""), scope);
	}
#endif
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
#ifdef USE_SMIL21
	case audioFade: return "audioFade";
	case audioVisualFade: return "audioVisualFade";
#endif
	default: return "<unknown transition type>";
	}
}

transition_info::time_type
transition_info::get_trans_dur(const node *n)
{
	if(!n) return 0;
	const char *p = n->get_attribute("dur");
	if(!p) return 1000;  // 1 second
	std::string sdur = trim(p);
	clock_value_p pl;
	std::string::const_iterator b = sdur.begin();
	std::string::const_iterator e = sdur.end();
	std::ptrdiff_t d = pl.parse(b, e);
	if(d == -1) {
		lib::logger::get_logger()->error(gettext("transition: illegal value for ""%s"" attribute (""%s"")"), "dur", p);
		return 1000;
	}
	return pl.m_result;
}

transition_info::progress_type
transition_info::get_progress(const node *n, const char* progress, progress_type default_value)
{
	if(!n) return 0;
	const char *p = n->get_attribute(progress);
	if(!p) return default_value;
	std::string sdur = trim(p);
	number_p np;
	std::string::const_iterator b = sdur.begin();
	std::string::const_iterator e = sdur.end();
	std::ptrdiff_t d = np.parse(b, e);
	if(d == -1) {
		lib::logger::get_logger()->error(gettext("transition: illegal value for ""%s"" attribute (""%s"")"), progress, p);
		return default_value;
	}
	return np.m_result;
}
