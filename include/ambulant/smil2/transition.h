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

#ifndef AMBULANT_SMIL2_TRANSITION_H
#define AMBULANT_SMIL2_TRANSITION_H

#ifdef USE_SMIL21
#include "ambulant/lib/event_processor.h"
#endif                                   
#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/transition_info.h"
#include <vector>

namespace ambulant {

namespace common {
class surface;
};
	
namespace smil2 {

class transition_engine {
  public:
	transition_engine();
	virtual ~transition_engine();
	void init(common::surface *dst, bool is_outtrans, const lib::transition_info *info);
	
	void begin(lib::transition_info::time_type now);
	void end();
	
	void step(lib::transition_info::time_type now);
	bool is_done();
	lib::transition_info::time_type next_step_delay();
  protected:
//	virtual void resized() {};
	virtual void compute() = 0;
	virtual void update() = 0;

	common::surface *m_dst;
	bool m_outtrans;
	const lib::transition_info *m_info;
	lib::transition_info::time_type m_begin_time;
	int m_stepcount;
	double m_progress;
	double m_old_progress;
	double m_progress_per_milli;
};

class transition_blitclass_rect : public transition_engine {
  protected:
	lib::screen_rect<int> m_newrect;
};

class transition_blitclass_r1r2r3r4 : public transition_engine {
  protected:
	lib::screen_rect<int> m_oldsrcrect, m_olddstrect, m_newsrcrect, m_newdstrect;
};

class transition_blitclass_rectlist : public transition_engine {
  public:
	~transition_blitclass_rectlist() { clear(); }
  protected:
	void clear() { m_newrectlist.clear(); };
	std::vector< lib::screen_rect<int> > m_newrectlist;
};

class transition_blitclass_poly : public transition_engine {
  public:
	~transition_blitclass_poly() { clear(); }
  protected:
	void clear() { m_newpolygon.clear(); };
	std::vector<lib::point> m_newpolygon;
};

class transition_blitclass_polylist : public transition_engine {
  public:
	~transition_blitclass_polylist() { clear(); }
  protected:
	void clear() { m_newpolygonlist.clear(); };
	std::vector< std::vector<lib::point> > m_newpolygonlist;
	lib::screen_rect<int> m_oldrect;
};

class transition_blitclass_fade : public transition_engine {
  protected:
};

/////////////////////////////

// Series 1: edge wipes

class transition_engine_barwipe : virtual public transition_blitclass_rect {
  protected:
    void compute();
};

class transition_engine_boxwipe : virtual public transition_blitclass_rect {
  protected:
    void compute();
};

class transition_engine_fourboxwipe : virtual public transition_blitclass_rectlist {
  protected:
    void compute();
};

class transition_engine_barndoorwipe : virtual public transition_blitclass_rect {
  protected:
    void compute();
};

class transition_engine_diagonalwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_miscdiagonalwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_veewipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_barnveewipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_zigzagwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_barnzigzagwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_bowtiewipe : virtual public transition_blitclass_polylist {
  protected:
    void compute();
};

// series 2: iris wipes

class transition_engine__iris : virtual public transition_blitclass_poly {
  protected:
	virtual lib::dpoint *get_template(int *size) = 0;
    void compute();
};

class transition_engine_iriswipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_pentagonwipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_arrowheadwipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_trianglewipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_hexagonwipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_eyewipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_roundrectwipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_ellipsewipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_starwipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

class transition_engine_miscshapewipe : virtual public transition_engine__iris {
  protected:
    lib::dpoint *get_template(int *size);
	static lib::dpoint m_template[];
};

// series 3: clock-type wipes

namespace detail {
// Helper class - compute edges and points for angles
enum edgetype {
	edge_topright,
	edge_right,
	edge_bottom,
	edge_left,
	edge_topleft
};

class angle_computer {
  public:
	angle_computer()
	:   m_initialized(false) {}
	angle_computer(lib::screen_rect<int> rect);
	~angle_computer() {}
	
	bool matches(lib::screen_rect<int> rect);
	
	void angle2poly(std::vector<lib::point> &outpoly, double angle, bool clockwise);
  private:
	void recompute_angles();
	edgetype angle2edge(double angle, lib::point &edgepoint);
	bool m_initialized;
	lib::screen_rect<int> m_rect;
	// more...
	int m_xmid, m_ymid;
	double m_angle_topleft, m_angle_topright, m_angle_botleft, m_angle_botright;
	double m_xdist, m_ydist;
};
}; // namespace detail

class transition_engine_clockwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
  private:
	detail::angle_computer m_angle_computer;
};

class transition_engine_singlesweepwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_doublesweepwipe : virtual public transition_blitclass_polylist {
  protected:
    void compute();
};

class transition_engine_saloondoorwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_windshieldwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_fanwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_doublefanwipe : virtual public transition_blitclass_poly {
  protected:
    void compute();
};

class transition_engine_pinwheelwipe : virtual public transition_blitclass_polylist {
  protected:
    void compute();
};

// series 4: matrix wipe types

class transition_engine_snakewipe : virtual public transition_blitclass_rectlist {
  protected:
    void compute();
};

class transition_engine_waterfallwipe : virtual public transition_blitclass_rectlist {
  protected:
    void compute();
};

class transition_engine_spiralwipe : virtual public transition_blitclass_rectlist {
  protected:
    void compute();
};

class transition_engine_parallelsnakeswipe : virtual public transition_blitclass_rectlist {
  protected:
    void compute();
};

class transition_engine_boxsnakeswipe : virtual public transition_blitclass_rectlist {
  protected:
    void compute();
};

// series 5: SMIL-specific types

class transition_engine_pushwipe : virtual public transition_blitclass_r1r2r3r4 {
  protected:
    void compute();
};

class transition_engine_slidewipe : virtual public transition_blitclass_r1r2r3r4 {
  protected:
    void compute();
};

class transition_engine_fade : virtual public transition_blitclass_fade {
  protected:
	void compute();
};
#ifdef USE_SMIL21

class abstract_audio_transition_engine {
  public:
	virtual void init(const lib::event_processor* evp, bool outtrans, const lib::transition_info* info) = 0;
	virtual const double get_volume(const double soundlevel) = 0;
	virtual const bool is_done(lib::transition_info::time_type now) = 0;
};

class audio_transition_engine :  virtual public abstract_audio_transition_engine {
  public:
	audio_transition_engine();
	void init(const lib::event_processor* evp, bool outtrans, const lib::transition_info* info);
	const double get_volume(const double soundlevel);
	const bool is_done(lib::transition_info::time_type now);

  protected:
	lib::transition_info::time_type m_start_time;
	lib::transition_info::time_type m_dur;
	lib::transition_info::progress_type m_startProgress;
	lib::transition_info::progress_type m_endProgress;
	bool m_outtrans;

  private:
	const lib::event_processor* m_event_processor;
};
#endif                                   

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_TRANSITION_H
