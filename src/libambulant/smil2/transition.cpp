// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/logger.h"
#include "ambulant/lib/node.h"
#include "ambulant/common/layout.h"
#include "ambulant/smil2/transition.h"
#include <math.h>
#define round(x) ((int)((x)+0.5))

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
:	m_dst(NULL),
	m_info(NULL),
	m_begin_time(0),
	m_stepcount(0),
	m_progress(0),
	m_old_progress(0),
	m_progress_per_milli(0)
{
	AM_DBG lib::logger::get_logger()->debug("transition_engine::transition_engine()");
}

transition_engine::~transition_engine()
{
	AM_DBG lib::logger::get_logger()->debug("transition_engine::~transition_engine()");
}

void
transition_engine::init(common::surface *dst, bool outtrans, const lib::transition_info *info)
{
	m_dst = dst;
	m_outtrans = outtrans;
	m_info = info;
	AM_DBG lib::logger::get_logger()->debug("transition_engine::init()");
	m_progress = m_info->m_startProgress;
	lib::transition_info::time_type dur = m_info->m_dur;
	if (dur <= 0) {
		lib::logger::get_logger()->trace("transition: duration %f, should be greater than zero", float(dur));
		dur = 1;
	}
	m_progress_per_milli = (m_info->m_endProgress - m_info->m_startProgress) / dur;
}

void
transition_engine::begin(lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->debug("transition_engine::begin(%d)", now);
	assert(m_info);
	m_begin_time = now;
}

void
transition_engine::end()
{
	AM_DBG lib::logger::get_logger()->debug("transition_engine::end()");
	assert(m_info);
}

void
transition_engine::step(lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->debug("transition_engine::step(%d)", now);
	assert(m_info);
	//lib::transition_info::time_type orig_now = now;
	if (now < m_begin_time) {
		// sometimes clock steps back, then the calculation of 'm_progress' results in a huge value
		// and no transition effect would be visible, without the next line.
		now = m_begin_time;
	}
	m_progress = (now-m_begin_time) * m_progress_per_milli + m_info->m_startProgress;
	//printf("m_begin_time=%ld, now=%ld, orig_now=%ld, m_progress_per_milli=%f, m_progress=%f\n",m_begin_time, now, orig_now, m_progress_per_milli, m_progress);
	if (m_progress <= m_old_progress)
		m_progress = m_old_progress;
	else
		m_old_progress = m_progress;
	if (m_progress > m_info->m_endProgress) m_progress = 1.0;
	AM_DBG lib::logger::get_logger()->debug("transition_engine::step: delta_t=%d, progress=%f%%", now-m_begin_time, m_progress*100);
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
		double dt = m_info->m_dur / m_stepcount;
		AM_DBG lib::logger::get_logger()->debug("transition_engine::next_step_delay: m_stepcount=%d, dt=%f", m_stepcount, dt);
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
	lib::rect dstrect = m_dst->get_rect();
	int wcur = round(m_progress*dstrect.width());
	m_stepcount = dstrect.width();
//	int ycur = dstrect.top() + round(m_progress*(dstrect.bottom() - dstrect.top()));
	m_newrect = lib::rect(dstrect.left_top(), lib::size(wcur, dstrect.height()));
}

void
transition_engine_boxwipe::compute()
{
	// XXX Only does box from topleft right now
	lib::rect dstrect = m_dst->get_rect();
	int wcur = round(m_progress*dstrect.width());
	int hcur = round(m_progress*dstrect.height());
	m_stepcount = dstrect.width();
	m_newrect = lib::rect(dstrect.left_top(), lib::size(wcur, hcur));
}


void
transition_engine_fourboxwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	int xmid = (dstrect.left() + dstrect.right())/2;
	int ymid = (dstrect.top() + dstrect.bottom())/2;
	int half_width = round(m_progress*(xmid - dstrect.left()));
	int half_height = round(m_progress*(ymid - dstrect.top()));
	lib::size half_size(half_width, half_height);
	m_stepcount = (dstrect.width())/2;
	clear();
	m_newrectlist.push_back(lib::rect(
		dstrect.left_top(),
		half_size));
	m_newrectlist.push_back(lib::rect(
		lib::point(dstrect.right()-half_width, dstrect.top()),
		half_size));
	m_newrectlist.push_back(lib::rect(
		lib::point(dstrect.left(), dstrect.bottom()-half_height),
		half_size));
	m_newrectlist.push_back(lib::rect(
		lib::point(dstrect.right()-half_width, dstrect.bottom()-half_height),
		half_size));
}

void
transition_engine_barndoorwipe::compute()
{
	// XXX Only does horizontal right now
	lib::rect dstrect = m_dst->get_rect();
	int xmid = (dstrect.left() + dstrect.right())/2;
//	int ymid = (dstrect.top() + dstrect.bottom())/2;
	int half_width = round(m_progress*(xmid - dstrect.left()));
//	int half_height = round(m_progress*(ymid - dstrect.top()));
	m_stepcount = (dstrect.width())/2;
	m_newrect = lib::rect(
		lib::point(xmid-half_width, dstrect.top()),
		lib::size(2*half_width, dstrect.height()));
}

void
transition_engine_diagonalwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	int xmin = dstrect.left() - 2*dstrect.width();
	int xcur = xmin + (int)(m_progress*2*dstrect.width());
	clear();
	m_stepcount = 2*dstrect.width();

	m_newpolygon.push_back(lib::point(xcur, dstrect.top()));
	m_newpolygon.push_back(lib::point(xcur+2*dstrect.width(), dstrect.top()));
	m_newpolygon.push_back(lib::point(xcur+dstrect.width(), dstrect.bottom()));
	m_newpolygon.push_back(lib::point(xcur, dstrect.bottom()));
}

void
transition_engine_miscdiagonalwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype miscDiagonalWipe not yet implemented");
}

void
transition_engine_veewipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype veeWipe not yet implemented");
}

void
transition_engine_barnveewipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype barnVeeWipe not yet implemented");
}

void
transition_engine_zigzagwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype zigZagWipe not yet implemented");
}

void
transition_engine_barnzigzagwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype barnZigZagWipe not yet implemented");
}

void
transition_engine_bowtiewipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	int x0 = dstrect.left();
	int y0 = dstrect.top();
	int x1 = dstrect.right();
	int y1 = dstrect.bottom();
	int xmid = (x0 + x1)/2;
	int ymid = (y0 + y1)/2;
	int width = x1 - x0;
	int height = y1 - y0;
	m_newpolygonlist.clear();
	if (m_progress <= 0.5) {
		int xleft, xright, ytop, ybot;
		std::vector<lib::point> poly;
		double deltax = m_progress*width;
		xleft  = static_cast<int>(round(xmid - deltax));
		xright = static_cast<int>(round(xmid + deltax));
		double deltay = m_progress*height;
		ytop   = static_cast<int>(round(y0   + deltay));
		ybot   = static_cast<int>(round(y1   - deltay));
		poly.push_back(lib::point(xleft,  y0));
		poly.push_back(lib::point(xright, y0));
		poly.push_back(lib::point(xmid,   ytop));
		m_newpolygonlist.push_back(poly);
		poly.clear();
		poly.push_back(lib::point(xright, y1));
		poly.push_back(lib::point(xleft,  y1));
		poly.push_back(lib::point(xmid,   ybot));
		m_newpolygonlist.push_back(poly);
	} else {
		lib::transition_info::progress_type value = m_progress - 0.5;
		int xleft, xright, ytop, ybot;
		std::vector<lib::point> poly;
		double deltax = value*width;
		xleft  = static_cast<int>(round(xmid - deltax));
		xright = static_cast<int>(round(xmid + deltax));
		double deltay = value*height;
		ytop   = static_cast<int>(round(y0   + deltay));
		ybot   = static_cast<int>(round(y1   - deltay));
		poly.push_back(lib::point(x0,  y0));
		poly.push_back(lib::point(x1,  y0));
		poly.push_back(lib::point(x1,  ytop));
		poly.push_back(lib::point(xright, ymid));
		poly.push_back(lib::point(x1,  ybot));
		poly.push_back(lib::point(x1,  y1));
		poly.push_back(lib::point(x0,  y1));
		poly.push_back(lib::point(x0,  ybot));
		poly.push_back(lib::point(xleft, ymid));
		poly.push_back(lib::point(x0,  ytop));
		m_newpolygonlist.push_back(poly);
	}
}

// series 2: iris wipes

void
transition_engine__iris::compute()
{
	lib::rect dstrect = m_dst->get_rect();
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
	lib::dpoint(-1, 1)
};

lib::dpoint *
transition_engine_iriswipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_pentagonwipe::m_template[] = {
	lib::dpoint(0.000000, -1.236068),
	lib::dpoint(1.175571, -0.381966),
	lib::dpoint(0.726543, 1.000000),
	lib::dpoint(-0.726543, 1.000000),
	lib::dpoint(-1.175571, -0.381966)
};

lib::dpoint *
transition_engine_pentagonwipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_arrowheadwipe::m_template[] = {
	lib::dpoint(0.000000, 0.500000),
	lib::dpoint(-1.732051, 1.000000),
	lib::dpoint(0.000000, -2.000000),
	lib::dpoint(1.732051, 1.000000)
};

lib::dpoint *
transition_engine_arrowheadwipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_trianglewipe::m_template[] = {
	lib::dpoint(0.000000, -2.000000),
	lib::dpoint(1.732051, 1.000000),
	lib::dpoint(-1.732051, 1.000000)
};

lib::dpoint *
transition_engine_trianglewipe::get_template(int *size)
{
	*size = sizeof(m_template) / sizeof(m_template[0]);
	return m_template;
}

lib::dpoint ambulant::smil2::transition_engine_hexagonwipe::m_template[] = {
	/* horizontal (113) [default]  */
	lib::dpoint(1.154701, 0.000000),
	lib::dpoint(0.577350, 1.000000),
	lib::dpoint(-0.577350, 1.000000),
	lib::dpoint(-1.154701, 0.000000),
	lib::dpoint(-0.577350, -1.000000),
	lib::dpoint(0.577350, -1.000000)
	/* vertical (114) 
    lib::dpoint(0.000000, 1.154701),
    lib::dpoint(-1.000000, 0.577350),
    lib::dpoint(-1.000000, -0.577350),
    lib::dpoint(0.000000, -1.154701),
    lib::dpoint(1.000000, -0.577350),
    lib::dpoint(1.000000, 0.577350)/
	 */
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

detail::angle_computer::angle_computer(lib::rect rect)
:   m_initialized(true),
	m_rect(rect)
{
	recompute_angles();
}

bool
detail::angle_computer::matches(lib::rect rect)
{
	if (!m_initialized) return false;
	return rect == m_rect;
}

void
detail::angle_computer::recompute_angles()
{
	AM_DBG lib::logger::get_logger()->debug("angle_computer::recompute_angles()");
	int l = m_rect.left(), r = m_rect.right(), t = m_rect.top(), b = m_rect.bottom();
	m_xmid = (l+r)/2;
	m_ymid = (t+b)/2;
	m_xdist = (r-l)/2;
	m_ydist = (b-t)/2;
	m_angle_topleft = atan2(m_ydist, -m_xdist);
	m_angle_topright = atan2(m_ydist, m_xdist);
	m_angle_botleft = atan2(-m_ydist, -m_xdist);
	m_angle_botright = atan2(-m_ydist, m_xdist);
	AM_DBG lib::logger::get_logger()->debug("angle_computer::recompute_angles: tl=%d, tr=%d, bl=%d, br=%d",
		(int)(m_angle_topleft * 180 / M_PI),
		(int)(m_angle_topright * 180 / M_PI),
		(int)(m_angle_botleft * 180 / M_PI),
		(int)(m_angle_botright * 180 / M_PI));
}

void
detail::angle_computer::angle2poly(std::vector<lib::point> &outpolygon, double angle, bool clockwise)
{
	AM_DBG lib::logger::get_logger()->debug("angle_computer::angle2poly()");
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
	AM_DBG lib::logger::get_logger()->debug("angle_computer::angle2edge(%f) %d degrees", angle, (int)(angle*180/M_PI));
	// Normalize angle to [-pi, pi) range
	while (angle < -M_PI)
		angle = angle + 2*M_PI;
	while (angle >= M_PI)
		angle = angle -2*M_PI;
	// Now find between which cornerpoint angles we are
	detail::edgetype edge;
	if (angle >= m_angle_topright && angle <= M_PI/2) {
		edge = edge_topright;
		AM_DBG lib::logger::get_logger()->debug("angle_computer::angle2edge: topright");
	} else if (angle >= m_angle_botright && angle <= m_angle_topright) {
		edge = edge_right;
		AM_DBG lib::logger::get_logger()->debug("angle_computer::angle2edge: right");
	} else if (angle >= m_angle_botleft && angle <= m_angle_botright) {
		edge = edge_bottom;
		AM_DBG lib::logger::get_logger()->debug("angle_computer::angle2edge: bottom");
	} else if (angle >= m_angle_topleft || angle <= m_angle_botleft) {
		edge = edge_left;
		AM_DBG lib::logger::get_logger()->debug("angle_computer::angle2edge: left");
	} else if (angle >= M_PI/2 && angle <= m_angle_topleft) {
		edge = edge_topleft;
		AM_DBG lib::logger::get_logger()->debug("angle_computer::angle2edge: topleft");
	} else
		lib::logger::get_logger()->trace("transition_engine: impossible angle %f", angle);
	// Next compute the intersection point
	int l = m_rect.left(), r = m_rect.right(), t = m_rect.top(), b = m_rect.bottom();
	if (edge == edge_topleft || edge == edge_topright)
		edgepoint = lib::point(round(m_xmid + m_xdist/tan(angle)), t);
	else if (edge == edge_right)
		edgepoint = lib::point(r, round(m_ymid - m_ydist*tan(angle)));
	else if (edge == edge_bottom)
		edgepoint = lib::point(round(m_xmid - m_xdist/tan(angle)), b);
	else
		edgepoint = lib::point(l, round(m_ymid + m_ydist*tan(angle)));
	return edge;
}

void
transition_engine_clockwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	// First check whether we're done.
	clear();
	if (m_progress > 0.999) {
		int l = dstrect.left(), r = dstrect.right(), t = dstrect.top(), b = dstrect.bottom();
		m_newpolygon.push_back(lib::point(l, t));
		m_newpolygon.push_back(lib::point(r, t));
		m_newpolygon.push_back(lib::point(r, b));
		m_newpolygon.push_back(lib::point(l, b));
		return;
	}
	if (!m_angle_computer.matches(dstrect))
		m_angle_computer = detail::angle_computer(dstrect);
	m_stepcount = 2*dstrect.width() + 2*dstrect.height();
	double angle = M_PI/2 - (m_progress*2*M_PI);
	AM_DBG lib::logger::get_logger()->debug("transition_engine_clockwipe::compute: progress %f angle %f (%d)", m_progress, angle, (int)(angle*180/M_PI));
	m_angle_computer.angle2poly(m_newpolygon, angle, true);
}

void
transition_engine_singlesweepwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype singleSweepWipe not yet implemented");
}

void
transition_engine_doublesweepwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype doubleSweepWipe not yet implemented");
}

void
transition_engine_saloondoorwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype saloonDoorWipe not yet implemented");
}

void
transition_engine_windshieldwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype windshieldWipe not yet implemented");
}

void
transition_engine_fanwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype fanWipe not yet implemented");
}

void
transition_engine_doublefanwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype doubleFanWipe not yet implemented");
}

void
transition_engine_pinwheelwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype pinWheelWipe not yet implemented");
}

// series 4: matrix wipe types

void
transition_engine_snakewipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	int index = (int)(m_progress*MATRIX_HSTEPS*MATRIX_VSTEPS);
	int hindex = index % MATRIX_HSTEPS;
	int vindex = index / MATRIX_HSTEPS;
	int vindexpos = (dstrect.top() + vindex*(dstrect.height())/MATRIX_VSTEPS);
	int vindex2pos = (dstrect.top() + (vindex+1)*(dstrect.height())/MATRIX_VSTEPS);
	m_stepcount = MATRIX_HSTEPS*MATRIX_VSTEPS;
	clear();
	if (vindex)
		m_newrectlist.push_back(lib::rect(
			dstrect.left_top(),
			lib::size(dstrect.width(), vindexpos-dstrect.top())));
	if (hindex) {
		if (vindex & 1) {
			int hindexpos = (dstrect.right() - hindex*(dstrect.width())/MATRIX_VSTEPS);
			m_newrectlist.push_back(lib::rect(
				lib::point(hindexpos, vindexpos),
				lib::size(dstrect.width(), vindex2pos-vindexpos)));
		} else {
			int hindex2pos = (dstrect.left() + hindex*(dstrect.width())/MATRIX_VSTEPS);
			m_newrectlist.push_back(lib::rect(
				lib::point(dstrect.left(), vindexpos),
				lib::size(hindex2pos-dstrect.left(), vindex2pos-vindexpos)));
		}
	}
}

void
transition_engine_waterfallwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	int index = (int)(m_progress*MATRIX_HSTEPS*MATRIX_VSTEPS);
	int hindex = index / MATRIX_HSTEPS;
	int vindex = index % MATRIX_HSTEPS;
	int hindexpos = (dstrect.left() + hindex*(dstrect.width())/MATRIX_VSTEPS);
	int hindex2pos = (dstrect.top() + (hindex+1)*(dstrect.width())/MATRIX_VSTEPS);
	int vindexpos = (dstrect.top() + vindex*(dstrect.height())/MATRIX_VSTEPS);
	m_stepcount = MATRIX_HSTEPS*MATRIX_VSTEPS;
	clear();
	if (hindex)
		m_newrectlist.push_back(lib::rect(
			dstrect.left_top(),
			lib::size(hindexpos-dstrect.left(), dstrect.height())));
	if (vindex)
		m_newrectlist.push_back(lib::rect(
			lib::point(hindexpos, dstrect.top()),
			lib::size(hindex2pos-hindexpos, vindexpos-dstrect.top())));
}

void
transition_engine_spiralwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype spiralWipe not yet implemented");
}

void
transition_engine_parallelsnakeswipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype parallelSnakesWipe not yet implemented");
}

void
transition_engine_boxsnakeswipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	lib::logger::get_logger()->trace("transitiontype boxSnakesWipe not yet implemented");
}

// series 5: SMIL-specific types

void
transition_engine_pushwipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	int half_width = round(m_progress*dstrect.width());
//	int half_height = round(m_progress*(ymid - dstrect.top()));
	m_stepcount = dstrect.width();
	m_oldsrcrect = lib::rect(
		dstrect.left_top(),
		lib::size(dstrect.width()-half_width, dstrect.height()));
	m_olddstrect = lib::rect(
		lib::point(dstrect.left()+half_width, dstrect.top()),
		lib::size(dstrect.width()-half_width, dstrect.height()));
	m_newsrcrect = lib::rect(
		lib::point(dstrect.right()-half_width, dstrect.top()),
		lib::size(half_width, dstrect.height()));
	m_newdstrect = lib::rect(
		dstrect.left_top(),
		lib::size(half_width, dstrect.height()));
}

void
transition_engine_slidewipe::compute()
{
	lib::rect dstrect = m_dst->get_rect();
	int half_width = round(m_progress*dstrect.width());
//	int half_height = round(m_progress*(ymid - dstrect.top()));
	m_stepcount = dstrect.width();
	m_oldsrcrect = lib::rect(
		lib::point(dstrect.left()+half_width, dstrect.top()),
		lib::size(dstrect.width()-half_width, dstrect.height()));
	m_olddstrect = lib::rect(
		lib::point(dstrect.left()+half_width, dstrect.top()),
		lib::size(dstrect.width()-half_width, dstrect.height()));
	m_newsrcrect = lib::rect(
		lib::point(dstrect.right()-half_width, dstrect.top()),
		lib::size(half_width, dstrect.height()));
	m_newdstrect = lib::rect(
		dstrect.left_top(),
		lib::size(half_width, dstrect.height()));
}

void
transition_engine_fade::compute()
{
	m_stepcount = 256;
}


audio_transition_engine::audio_transition_engine()
:	m_start_time(0),
	m_dur(0),
	m_startProgress(0),
	m_endProgress(1),
	m_outtrans(false),
	m_event_processor(NULL)
{
}

void audio_transition_engine::init(const lib::event_processor* evp,bool outtrans, const lib::transition_info* info) {
	m_event_processor = evp;
	m_outtrans	= outtrans;
	m_start_time	= m_event_processor->get_timer()->elapsed();
	m_dur		= info->m_dur;
	m_startProgress = info->m_startProgress;
	m_endProgress = info->m_endProgress;
	AM_DBG lib::logger::get_logger()->debug("audio_transition_engine::audio_transition_engine(0x%x): m_start_time=%d  m_dur=%d m_startProgress=%f m_endProgress=%f",this,m_start_time,m_dur,m_startProgress,m_endProgress);
}

const double
audio_transition_engine::get_volume(const double soundlevel) {
	double progress;
	lib::transition_info::time_type now;
	now = m_event_processor->get_timer()->elapsed();
	if (m_dur == 0 || is_done(now))
		// no transition or transition finished
		return soundlevel;
	progress = ((double) (now - m_start_time) / m_dur)
		* (m_endProgress - m_startProgress);

	progress += m_startProgress;
	if (progress > m_endProgress) progress = m_endProgress;
	if (progress < m_startProgress) progress = m_startProgress;
	AM_DBG lib::logger::get_logger()->debug("audio_transition_engine::get_transition_volume(0x%x): soundlevel=%f m_outtrans=%d now=%d dur=%d progress=%f",this,soundlevel,m_outtrans,now,m_dur,progress);
	double level = soundlevel;
	if (m_outtrans)
		level *= (1.0 - progress);
	else
		level *= progress;
	return level;
}


const bool
audio_transition_engine::is_done(lib::transition_info::time_type now) {
	return now >= m_start_time + m_dur;
}
