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

#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/transition_info.h"

namespace ambulant {

namespace common {
class surface;
};
	
namespace smil2 {

class transition_engine {
  public:
	transition_engine();
	virtual ~transition_engine();
	void init(common::surface *dst, bool is_outtrans, lib::transition_info *info);
	
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
	lib::transition_info *m_info;
	lib::transition_info::time_type m_begin_time;
	double m_progress;
	double m_time2progress;
};

class transition_blitclass_r1r2 : public transition_engine {
  protected:
	lib::screen_rect<int> m_oldrect, m_newrect;
};

class transition_blitclass_r1r2r3r4 : public transition_engine {
  protected:
	lib::screen_rect<int> m_oldsrcrect, m_olddstrect, m_newsrcrect, m_newdstrect;
};

class transition_blitclass_rlistr2 : public transition_engine {
  protected:
};

class transition_blitclass_polyr2 : public transition_engine {
  protected:
};

class transition_blitclass_polylistr2 : public transition_engine {
  protected:
};

class transition_blitclass_fade : public transition_engine {
  protected:
};

/////////////////////////////

class transition_engine_fade : virtual public transition_blitclass_fade {
  protected:
	void compute();
};

class transition_engine_barwipe : virtual public transition_blitclass_r1r2 {
  protected:
    void compute();
};

class transition_engine_boxwipe : virtual public transition_blitclass_r1r2 {
  protected:
    void compute();
};

class transition_engine_barndoorwipe : virtual public transition_blitclass_r1r2 {
  protected:
    void compute();
};

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_TRANSITION_H
