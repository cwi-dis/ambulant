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
	common::sound_alignment sa;
	common::region_dim_spec panzoom;
};

// An animate_node is the base class for all animation node flavors
class animate_node : public time_node {
  public:
	animate_node(context_type *ctx, const node *n, animate_attrs *aattrs);
	~animate_node();

	// Scheduler interface
	virtual void prepare_interval();
	virtual void get_values();

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
	static animate_node* new_soundalign_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	static animate_node* new_panzoom_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
	static animate_node* new_opacity_animation(context_type *ctx, const node *n, animate_attrs *aattrs);
  protected:
	// The set of animation related attributes of this node.
	// Attributes parsing helper
	animate_attrs *m_aattrs;
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_ANIMATE_N_H
