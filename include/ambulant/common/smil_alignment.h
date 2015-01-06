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

#ifndef AMBULANT_COMMON_ALIGNMENT_H
#define AMBULANT_COMMON_ALIGNMENT_H

#include "ambulant/lib/node.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/region_dim.h"

namespace ambulant {

namespace common {

/// Implementation of the alignment interface for SMIL 2.0.
class smil_alignment : public alignment {
  public:
	smil_alignment(regpoint_spec& image_fixpoint, regpoint_spec& surface_fixpoint)
	:   m_image_fixpoint(image_fixpoint),
		m_surface_fixpoint(surface_fixpoint) {}
	~smil_alignment() {}

	lib::point get_image_fixpoint(lib::size image_size) const;
	lib::point get_surface_fixpoint(lib::size surface_size) const;

  private:
	regpoint_spec m_image_fixpoint;
	regpoint_spec m_surface_fixpoint;
};


} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_ALIGNMENT_H
