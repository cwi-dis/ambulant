/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DG_WINDOW_H
#define AMBULANT_GUI_DG_WINDOW_H

#include "ambulant/config/config.h"

#include <string>

#include "ambulant/common/layout.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/common/region.h"

namespace ambulant {
namespace common {
class passive_region;
}}

typedef ambulant::common::surface_impl region; // XXX Or should it be surface??

namespace ambulant {

namespace gui {

namespace dg {

class viewport;

class dg_window : public common::gui_window {
  public:
  	dg_window(const std::string& name, 
  		lib::size bounds,
  		region *rgn,
  		common::window_factory *wf,
  		viewport* m_viewport);
  	~dg_window();
  	
	void need_redraw(const lib::rect& r);
	void redraw(const lib::rect& r);
	
	const std::string& get_name() const { return m_name;}
	region *get_region() { return m_rgn;}
	void need_redraw();
	void redraw_now() { /* Redraws are synchronous on Windows */ }
	void need_events(bool onoff) { /* Always get them on windows */ }
	
	viewport *get_viewport() { return m_viewport;}
  private:
	// gui_window:
	// passive_region *m_region;
	region *m_rgn; 
	std::string m_name; // for easy access
	lib::rect m_viewrc;	
	common::window_factory *m_wf;
    viewport* m_viewport;
};

} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_WINDOW_H
