/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#ifndef AMBULANT_GUI_ARTS_FACTORY_H
#define AMBULANT_GUI_ARTS_FACTORY_H

#include "ambulant/common/factory.h"
#include "ambulant/common/playable.h"

namespace ambulant {
namespace gui {
namespace arts {
          
AMBULANTAPI common::playable_factory *create_arts_playable_factory(common::factories *f);

} // end namespace arts
} // end namespace gui
} // end namespace ambulant


#endif // AMBULANT_GUI_ARTS_FACTORY_H
