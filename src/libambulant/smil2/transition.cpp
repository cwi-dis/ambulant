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
#include "ambulant/common/layout.h"
#include "ambulant/smil2/transition.h"
#include <math.h>

// It seems M_PI isn't defined for VC++
#ifndef M_PI
#define M_PI 3.1415926
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define MATRIX_HSTEPS 8
#define MATRIX_VSTEPS 8

using namespace ambulant;
using namespace smil2;

transition_engine::transition_engine()
:   m_dst(NULL),
	m_info(NULL),
	m_begin_time(0),
	m_stepcount(0)
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::transition_engine()");
}

transition_engine::~transition_engine()
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::~transition_engine()");
	// XXX Free m_info?
}

void
transition_engine::init(common::surface *dst, bool outtrans, lib::transition_info *info)
{
	m_dst = dst;
	m_outtrans = outtrans;
	m_info = info;
	AM_DBG lib::logger::get_logger()->trace("transition_engine::init()");
	m_progress = m_info->m_startProgress;
	lib::transition_info::time_type dur = m_info->m_dur;
	if (dur <= 0) {
		lib::logger::get_logger()->error("transition_engine: incorrect transition duration %f", float(dur));
		dur = 1;
	}
	m_time2progress = (m_info->m_endProgress - m_info->m_startProgress) / dur;
}

void
transition_engine::begin(lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::begin(%d)", now);
	assert(m_info);
	m_begin_time = now;
}

void
transition_engine::end()
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::end()");
	assert(m_info);
}

void
transition_engine::step(lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->trace("transition_engine::step(%d)", now);
	assert(m_info);
	m_progress = (now-m_begin_time) * m_time2progress;
	if (m_progress > m_info->m_endProgress) m_progress = 1.0;
	AM_DBG lib::logger::get_logger()->trace("transition_engine::step: delta_t=%d, progress=%f%%", now-m_begin_time, m_progress*100);
	compute();
	update();
}

bool
transition_engine::is_done()
{
	assert(m_info);
	return m_progress >= m_info->m_endProgress;
}

lib::transition_info::time_type
transition_engine::next_step_delay()
{
	assert(m_info);
	if (m_stepcount) {
		double dt = 1.0 / (m_stepcount*m_time2progress);
		AM_DBG lib::logger::get_logger()->trace("transition_engine::next_step_delay: m_stepcount=%d, dt=%f", m_stepcount, dt);
		return (lib::transition_info::time_type)dt;
	}
	return 20; // Show something 50 times per second
}

// **********************

// Series 1: edge wipes
void
transition_engine_barwipe::compute()
{
	// XXX Only does horizontal left-to-right at the moment
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int xcur = dstrect.m_left + int(m_progress*(dstrect.m_right - dstrect.m_left) + 0.5);
	m_stepcount = dstrect.m_right - dstrect.m_left;
//	int ycur = dstrect.m_top + int(m_progress*(dstrect.m_bottom - dstrect.m_top) + 0.5);
	m_newrect = lib::screen_rect<int>(
		lib::point(dstrect.m_left, dstrect.m_top),
		lib::point(xcur, dstrect.m_bottom));
}

void
transition_engine_boxwipe::compute()
{
	// XXX Only does box from topleft right now
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int xcur = dstrect.m_left + int(m_progress*(dstrect.m_right - dstrect.m_left) + 0.5);
	int ycur = dstrect.m_top + int(m_progress*(dstrect.m_bottom - dstrect.m_top) + 0.5);
	m_stepcount = dstrect.m_right - dstrect.m_left;
	m_newrect = lib::screen_rect<int>(
		lib::point(dstrect.m_left, dstrect.m_top),
		lib::point(xcur, ycur));
}


void
transition_engine_fourboxwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int xmid = (dstrect.m_left + dstrect.m_right)/2;
	int ymid = (dstrect.m_top + dstrect.m_bottom)/2;
	int half_width = int(m_progress*(xmid - dstrect.m_left) + 0.5);
	int half_height = int(m_progress*(ymid - dstrect.m_top) + 0.5);
	m_stepcount = (dstrect.m_right - dstrect.m_left)/2;
	clear();
	m_newrectlist.push_back(lib::screen_rect<int>(
		lib::point(dstrect.m_left, dstrect.m_top),
		lib::point(dstrect.m_left+half_width, dstrect.m_top+half_height)));
	m_newrectlist.push_back(lib::screen_rect<int>(
		lib::point(dstrect.m_right-half_width, dstrect.m_top),
		lib::point(dstrect.m_right, dstrect.m_top+half_height)));
	m_newrectlist.push_back(lib::screen_rect<int>(
		lib::point(dstrect.m_left, dstrect.m_bottom-half_height),
		lib::point(dstrect.m_left+half_width, dstrect.m_bottom)));
	m_newrectlist.push_back(lib::screen_rect<int>(
		lib::point(dstrect.m_right-half_width, dstrect.m_bottom-half_height),
		lib::point(dstrect.m_right, dstrect.m_bottom)));
}

void
transition_engine_barndoorwipe::compute()
{
	// XXX Only does horizontal right now
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int xmid = (dstrect.m_left + dstrect.m_right)/2;
//	int ymid = (dstrect.m_top + dstrect.m_bottom)/2;
	int half_width = int(m_progress*(xmid - dstrect.m_left) + 0.5);
//	int half_height = int(m_progress*(ymid - dstrect.m_top) + 0.5);
	m_stepcount = (dstrect.m_right - dstrect.m_left)/2;
	m_newrect = lib::screen_rect<int>(
		lib::point(xmid-half_width, dstrect.m_top),
		lib::point(xmid+half_width, dstrect.m_bottom));
}

void
transition_engine_diagonalwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int xmin = dstrect.left() - 2*dstrect.width();
	int xcur = xmin + (int)(m_progress*2*dstrect.width());
	m_stepcount = 2*dstrect.width();
	
	m_newpolygon.push_back(lib::point(xcur, dstrect.top()));
	m_newpolygon.push_back(lib::point(xcur+2*dstrect.width(), dstrect.top()));
	m_newpolygon.push_back(lib::point(xcur+dstrect.width(), dstrect.bottom()));
	m_newpolygon.push_back(lib::point(xcur, dstrect.bottom()));
}

void
transition_engine_miscdiagonalwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype miscDiagonalWipe not yet implemented");
}

void
transition_engine_veewipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype veeWipe not yet implemented");
}

void
transition_engine_barnveewipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype barnVeeWipe not yet implemented");
}

void
transition_engine_zigzagwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype zigZagWipe not yet implemented");
}

void
transition_engine_barnzigzagwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype barnZigZagWipe not yet implemented");
}

void
transition_engine_bowtiewipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype bowTieWipe not yet implemented");
}

// series 2: iris wipes

void
transition_engine__iris::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int pointcount;
	lib::dpoint *pointp = get_template(&pointcount);
	clear();
	m_stepcount = dstrect.width()/2;
	double radius = sqrt( 
		(0.5*dstrect.width())*(0.5*dstrect.width()) + 
		(0.5*dstrect.height())*(0.5*dstrect.height()));
	while (pointcount--) {
		lib::dpoint rel_point(pointp->x * m_progress, pointp->y * m_progress);
		lib::point abs_point(
			(int)(rel_point.x * radius + (dstrect.left() + dstrect.right())/2.0 ),
			(int)(rel_point.y * radius + (dstrect.top() + dstrect.bottom())/2.0 ));
		m_newpolygon.push_back(abs_point);
		pointp++;
	}
}

lib::dpoint ambulant::smil2::transition_engine_iriswipe::m_template[] = {
	lib::dpoint(-1, -1),
	lib::dpoint(1, -1),
	lib::dpoint(1, 1),
	lib::dpoint(1, 1)
};

lib::dpoint *
transition_engine_iriswipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_pentagonwipe::m_template[] = {
	lib::dpoint(0.000000, 1.236068),
	lib::dpoint(1.175571, 0.381966),
	lib::dpoint(0.726543, -1.000000),
	lib::dpoint(-0.726543, -1.000000),
	lib::dpoint(-1.175571, 0.381966)
};

lib::dpoint *
transition_engine_pentagonwipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_arrowheadwipe::m_template[] = {
	lib::dpoint(0.000000, 2.000000),
	lib::dpoint(1.732051, -1.000000),
	lib::dpoint(-1.732051, -1.000000)
};

lib::dpoint *
transition_engine_arrowheadwipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_trianglewipe::m_template[] = {
	lib::dpoint(0.000000, 2.000000),
	lib::dpoint(1.732051, -1.000000),
	lib::dpoint(0.000000, -0.500000),
	lib::dpoint(-1.732051, -1.000000)
};

lib::dpoint *
transition_engine_trianglewipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_hexagonwipe::m_template[] = {
	lib::dpoint(0.000000, 1.154701),
	lib::dpoint(1.000000, 0.577350),
	lib::dpoint(1.000000, -0.577350),
	lib::dpoint(0.000000, -1.154701),
	lib::dpoint(-1.000000, -0.577350),
	lib::dpoint(-1.000000, 0.577350)
};

lib::dpoint *
transition_engine_hexagonwipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_eyewipe::m_template[] = {
	lib::dpoint(-1, -1),
	lib::dpoint(1, -1),
	lib::dpoint(1, 1),
	lib::dpoint(1, 1)
};

lib::dpoint *
transition_engine_eyewipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_roundrectwipe::m_template[] = {
	lib::dpoint(-1, -1),
	lib::dpoint(1, -1),
	lib::dpoint(1, 1),
	lib::dpoint(1, 1)
};

lib::dpoint *
transition_engine_roundrectwipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_ellipsewipe::m_template[] = {
	lib::dpoint(-1, -1),
	lib::dpoint(1, -1),
	lib::dpoint(1, 1),
	lib::dpoint(1, 1)
};

lib::dpoint *
transition_engine_ellipsewipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_starwipe::m_template[] = {
	lib::dpoint(-1, -1),
	lib::dpoint(1, -1),
	lib::dpoint(1, 1),
	lib::dpoint(1, 1)
};

lib::dpoint *
transition_engine_starwipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_miscshapewipe::m_template[] = {
	lib::dpoint(-1, -1),
	lib::dpoint(1, -1),
	lib::dpoint(1, 1),
	lib::dpoint(1, 1)
};

lib::dpoint *
transition_engine_miscshapewipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

// series 3: clock-type wipes

detail::angle_computer::angle_computer(lib::screen_rect<int> rect)
:   m_initialized(true),
	m_rect(rect)
{
	recompute_angles();
}

bool
detail::angle_computer::matches(lib::screen_rect<int> rect)
{
	if (!m_initialized) return false;
	return rect == m_rect;
}

void
detail::angle_computer::recompute_angles()
{
	AM_DBG lib::logger::get_logger()->trace("angle_computer::recompute_angles()");
	int l = m_rect.left(), r = m_rect.right(), t = m_rect.top(), b = m_rect.bottom();
	m_xmid = (l+r)/2;
	m_ymid = (t+b)/2;
	m_xdist = (r-l)/2;
	m_ydist = (b-t)/2;
	m_angle_topleft = atan2(m_ydist, -m_xdist);
	m_angle_topright = atan2(m_ydist, m_xdist);
	m_angle_botleft = atan2(-m_ydist, -m_xdist);
	m_angle_botright = atan2(-m_ydist, m_xdist);
	AM_DBG lib::logger::get_logger()->trace("angle_computer::recompute_angles: tl=%d, tr=%d, bl=%d, br=%d",
		(int)(m_angle_topleft * 180 / M_PI),
		(int)(m_angle_topright * 180 / M_PI),
		(int)(m_angle_botleft * 180 / M_PI),
		(int)(m_angle_botright * 180 / M_PI));
}

void
detail::angle_computer::angle2poly(std::vector<lib::point> &outpolygon, double angle, bool clockwise)
{
	AM_DBG lib::logger::get_logger()->trace("angle_computer::angle2poly()");
	assert(clockwise);
	// Compute where a line with this angle intersects our rectangle
	int l = m_rect.left(), r = m_rect.right(), t = m_rect.top(), b = m_rect.bottom();
	lib::point edgepoint;
	detail::edgetype edge = angle2edge(angle, edgepoint);

	// We always have the hitpoint, the center and the top edge.
	// Then we progress over all cornerpoints that are part of the figure.
	outpolygon.push_back(edgepoint);
	outpolygon.push_back(lib::point(m_xmid, m_ymid));
	outpolygon.push_back(lib::point(m_xmid, t));
	if (edge == edge_topright) return;
	outpolygon.push_back(lib::point(r, t));
	if (edge == edge_right) return;
	outpolygon.push_back(lib::point(r, b));
	if (edge == edge_bottom) return;
	outpolygon.push_back(lib::point(l, b));
	if (edge == edge_left) return;
	outpolygon.push_back(lib::point(l, t));
	assert(edge == edge_topleft);
	return;
}

detail::edgetype
detail::angle_computer::angle2edge(double angle, lib::point &edgepoint)
{
	AM_DBG lib::logger::get_logger()->trace("angle_computer::angle2edge(%f) %d degrees", angle, (int)(angle*180/M_PI));
	// Normalize angle to [-pi, pi) range
	while (angle < -M_PI)
		angle = angle + 2*M_PI;
	while (angle >= M_PI)
		angle = angle -2*M_PI;
	// Now find between which cornerpoint angles we are
	detail::edgetype edge;
	if (angle >= m_angle_topright && angle <= M_PI/2) {
		edge = edge_topright;
		AM_DBG lib::logger::get_logger()->trace("angle_computer::angle2edge: topright");
	} else if (angle >= m_angle_botright && angle <= m_angle_topright) {
		edge = edge_right;
		AM_DBG lib::logger::get_logger()->trace("angle_computer::angle2edge: right");
	} else if (angle >= m_angle_botleft && angle <= m_angle_botright) {
		edge = edge_bottom;
		AM_DBG lib::logger::get_logger()->trace("angle_computer::angle2edge: bottom");
	} else if (angle >= m_angle_topleft || angle <= m_angle_botleft) {
		edge = edge_left;
		AM_DBG lib::logger::get_logger()->trace("angle_computer::angle2edge: left");
	} else if (angle >= M_PI/2 && angle <= m_angle_topleft) {
		edge = edge_topleft;
		AM_DBG lib::logger::get_logger()->trace("angle_computer::angle2edge: topleft");
	} else
		lib::logger::get_logger()->error("anglecomputer: impossible angle %f", angle);
	// Next compute the intersection point
	int l = m_rect.left(), r = m_rect.right(), t = m_rect.top(), b = m_rect.bottom();
	if (edge == edge_topleft || edge == edge_topright)
		edgepoint = lib::point((int)(m_xmid + m_xdist/tan(angle) + 0.5), t);
	else if (edge == edge_right)
		edgepoint = lib::point(r, (int)(m_ymid - m_ydist*tan(angle) + 0.5));
	else if (edge == edge_bottom)
		edgepoint = lib::point((int)(m_xmid - m_xdist/tan(angle) + 0.5), b);
	else
		edgepoint = lib::point(l, (int)(m_ymid + m_ydist*tan(angle) + 0.5));
	return edge;
}

void
transition_engine_clockwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	if (!m_angle_computer.matches(dstrect))
		m_angle_computer = detail::angle_computer(dstrect);
	clear();
	m_stepcount = 2*dstrect.width() + 2*dstrect.height();
	double angle = M_PI/2 - (m_progress*2*M_PI);
	AM_DBG lib::logger::get_logger()->trace("transition_engine_clockwipe::compute: progress %f angle %f (%d)", m_progress, angle, (int)(angle*180/M_PI));
	m_angle_computer.angle2poly(m_newpolygon, angle, true);
}

void
transition_engine_singlesweepwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype singleSweepWipe not yet implemented");
}

void
transition_engine_doublesweepwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype doubleSweepWipe not yet implemented");
}

void
transition_engine_saloondoorwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype saloonDoorWipe not yet implemented");
}

void
transition_engine_windshieldwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype windshieldWipe not yet implemented");
}

void
transition_engine_fanwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype fanWipe not yet implemented");
}

void
transition_engine_doublefanwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype doubleFanWipe not yet implemented");
}

void
transition_engine_pinwheelwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype pinWheelWipe not yet implemented");
}

// series 4: matrix wipe types

void
transition_engine_snakewipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int index = (int)(m_progress*MATRIX_HSTEPS*MATRIX_VSTEPS);
	int hindex = index % MATRIX_HSTEPS;
	int vindex = index / MATRIX_HSTEPS;
	int vindexpos = (dstrect.m_top + vindex*(dstrect.m_bottom-dstrect.m_top)/MATRIX_VSTEPS);
	int vindex2pos = (dstrect.m_top + (vindex+1)*(dstrect.m_bottom-dstrect.m_top)/MATRIX_VSTEPS);
	m_stepcount = MATRIX_HSTEPS*MATRIX_VSTEPS;
	clear();
	if (vindex)
		m_newrectlist.push_back(lib::screen_rect<int>(
			lib::point(dstrect.m_left, dstrect.m_top),
			lib::point(dstrect.m_right, vindexpos)));
	if (hindex) {
		if (vindex & 1) {
			int hindexpos = (dstrect.m_right - hindex*(dstrect.m_right-dstrect.m_left)/MATRIX_VSTEPS);
			m_newrectlist.push_back(lib::screen_rect<int>(
				lib::point(hindexpos, vindexpos),
				lib::point(dstrect.m_right, vindex2pos)));
		} else {
			int hindex2pos = (dstrect.m_left + hindex*(dstrect.m_right-dstrect.m_left)/MATRIX_VSTEPS);
			m_newrectlist.push_back(lib::screen_rect<int>(
				lib::point(dstrect.m_left, vindexpos),
				lib::point(hindex2pos, vindex2pos)));
		}
	}
}

void
transition_engine_waterfallwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int index = (int)(m_progress*MATRIX_HSTEPS*MATRIX_VSTEPS);
	int hindex = index / MATRIX_HSTEPS;
	int vindex = index % MATRIX_HSTEPS;
	int hindexpos = (dstrect.m_left + hindex*(dstrect.m_right-dstrect.m_left)/MATRIX_VSTEPS);
	int hindex2pos = (dstrect.m_top + (hindex+1)*(dstrect.m_right-dstrect.m_left)/MATRIX_VSTEPS);
	int vindexpos = (dstrect.m_top + vindex*(dstrect.m_bottom-dstrect.m_top)/MATRIX_VSTEPS);
	m_stepcount = MATRIX_HSTEPS*MATRIX_VSTEPS;
	clear();
	if (hindex)
		m_newrectlist.push_back(lib::screen_rect<int>(
			lib::point(dstrect.m_left, dstrect.m_top),
			lib::point(hindexpos, dstrect.m_bottom)));
	if (vindex)
		m_newrectlist.push_back(lib::screen_rect<int>(
			lib::point(hindexpos, dstrect.m_top),
			lib::point(hindex2pos, vindexpos)));
}

void
transition_engine_spiralwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype spiralWipe not yet implemented");
}

void
transition_engine_parallelsnakeswipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype parallelSnakesWipe not yet implemented");
}

void
transition_engine_boxsnakeswipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype boxSnakesWipe not yet implemented");
}

// series 5: SMIL-specific types

void
transition_engine_pushwipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int half_width = int(m_progress*(dstrect.m_right - dstrect.m_left) + 0.5);
//	int half_height = int(m_progress*(ymid - dstrect.m_top) + 0.5);
	m_stepcount = dstrect.m_right - dstrect.m_left;
	m_oldsrcrect = lib::screen_rect<int>(
		lib::point(dstrect.m_left, dstrect.m_top),
		lib::point(dstrect.m_right-half_width, dstrect.m_bottom));
	m_olddstrect = lib::screen_rect<int>(
		lib::point(dstrect.m_left+half_width, dstrect.m_top),
		lib::point(dstrect.m_right, dstrect.m_bottom));
	m_newsrcrect = lib::screen_rect<int>(
		lib::point(dstrect.m_right-half_width, dstrect.m_top),
		lib::point(dstrect.m_right, dstrect.m_bottom));
	m_newdstrect = lib::screen_rect<int>(
		lib::point(dstrect.m_left, dstrect.m_top),
		lib::point(dstrect.m_left+half_width, dstrect.m_bottom));
}

void
transition_engine_slidewipe::compute()
{
	lib::screen_rect<int> dstrect = m_dst->get_rect();
	int half_width = int(m_progress*(dstrect.m_right - dstrect.m_left) + 0.5);
//	int half_height = int(m_progress*(ymid - dstrect.m_top) + 0.5);
	m_stepcount = dstrect.m_right - dstrect.m_left;
	m_oldsrcrect = lib::screen_rect<int>(
		lib::point(dstrect.m_left+half_width, dstrect.m_top),
		lib::point(dstrect.m_right, dstrect.m_bottom));
	m_olddstrect = lib::screen_rect<int>(
		lib::point(dstrect.m_left+half_width, dstrect.m_top),
		lib::point(dstrect.m_right, dstrect.m_bottom));
	m_newsrcrect = lib::screen_rect<int>(
		lib::point(dstrect.m_right-half_width, dstrect.m_top),
		lib::point(dstrect.m_right, dstrect.m_bottom));
	m_newdstrect = lib::screen_rect<int>(
		lib::point(dstrect.m_left, dstrect.m_top),
		lib::point(dstrect.m_left+half_width, dstrect.m_bottom));
}

void
transition_engine_fade::compute()
{
	m_stepcount = 256;
}
