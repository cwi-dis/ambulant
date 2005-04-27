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

#ifndef AMBULANT_SMIL2_ANIMATE_N_H
#define AMBULANT_SMIL2_ANIMATE_N_H

#include "ambulant/config/config.h"
#include "ambulant/lib/colors.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/time_node.h"
#include "ambulant/smil2/animate_a.h"

namespace ambulant {

namespace common {
	class animation_destination;
}

namespace smil2 {

// Animation registers
// One register per linear animateable attribute type
// Each animator type uses at most one register
struct animate_registers {
	common::region_dim rd;
	lib::color_t cl;
	lib::point pt;
	common::zindex_t zi;
	int iv;
	double dv;
#ifdef USE_SMIL21
	common::sound_alignment sa;
#endif
};

// An animate_node is the base class for all animation node flavors
class animate_node : public time_node {
  public:	
	animate_node(context_type *ctx, const node *n, animate_attrs *aattrs);
	~animate_node();
	
	// Scheduler interface
	virtual void prepare_interval();
	
	// Animation engine interface
	const lib::node *get_animation_target() const { return m_aattrs->get_target();}
	const std::string& get_animation_attr() const { return m_aattrs->get_target_attr();}
	virtual void read_dom_value(common::animation_destination *dst, animate_registers& regs) const;
	virtual bool set_animated_value(common::animation_destination *dst, animate_registers& regs) const;
	virtual void apply_self_effect(animate_registers& regs) const;
	
	// Timegraph building interface
	static animate_node* new_instance(context_type *ctx, const node *n, const node* tparent);
	
  private:
	// Internal helpers for timegraph building interface
	static animate_node* new_regdim_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	static animate_node* new_color_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	static animate_node* new_zindex_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	static animate_node* new_position_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	// XXXX Need to add soundlevel animation (similar to zindex)
#ifdef USE_SMIL21
	static animate_node* new_soundalign_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
#endif
  protected:
	// The set of animation related attributes of this node.
	// Attributes parsing helper
	animate_attrs *m_aattrs;
};

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_ANIMATE_N_H
