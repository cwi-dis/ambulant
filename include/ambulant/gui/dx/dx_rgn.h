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

#ifndef AMBULANT_GUI_DX_RGN_H
#define AMBULANT_GUI_DX_RGN_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace gui {

namespace dx {

class dx_gui_region : public common::gui_region {
  public:
	// Creates an empty region
	dx_gui_region() 
	:	m_hrgn(CreateRectRgn(0, 0, 0, 0)) { 
		s_counter++;
	}
	
	// Creates a rect region
	dx_gui_region(const lib::screen_rect<int>& rect) 
	:	m_hrgn(CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom())) { 
		s_counter++;
	}
			
	// Creates a region that is equal in size and shape to the provided region
	dx_gui_region(const gui_region& rgn) 
	:	m_hrgn(CreateRectRgn(0, 0, 0, 0)) {
		CombineRgn(m_hrgn, handle(rgn), 0, RGN_COPY);
		s_counter++;
	}
	
	~dx_gui_region() { 
		DeleteObject((HGDIOBJ)m_hrgn); 
		s_counter--;
	}
	
	// Clone factory function
	gui_region *clone() const {
		HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
		CombineRgn(hrgn, m_hrgn, 0, RGN_COPY);
		return new dx_gui_region(hrgn);
	}
	
    void clear() {
		SetRectRgn(m_hrgn, 0, 0, 0, 0);
    }
    
	bool is_empty() const {
		RECT rc;
		return (GetRgnBox(m_hrgn, &rc) == NULLREGION) || rc.top == rc.bottom || rc.left == rc.right;
	}
	
	bool contains(const lib::point& pt) const {
		return PtInRegion(m_hrgn, pt.x, pt.y) != 0;
	}
	
	bool overlaps(const lib::screen_rect<int>& rect) const {
		RECT rc = {rect.left(), rect.top(), rect.right(), rect.bottom()};
		return RectInRegion(m_hrgn, &rc) != 0;
	}
	
	lib::screen_rect<int> get_bounding_box() const {
		RECT rect;
		GetRgnBox(m_hrgn, &rect);
		return lib::screen_rect<int>(lib::point(rect.left, rect.top), 
			lib::point(rect.right, rect.bottom));
	}
	
	bool operator ==(const common::gui_region& r) const {
		return EqualRgn(m_hrgn, handle(r)) != 0;
	}
	
	// assignment
	common::gui_region& operator =(const lib::screen_rect<int>& rect) {
		SetRectRgn(m_hrgn, rect.left(), rect.top(), rect.right(), rect.bottom());
		return *this;
	}
	
	// assignment
	common::gui_region& operator =(const common::gui_region& r) {
		if(this == &r) return *this;
		CombineRgn(m_hrgn, handle(r), 0, RGN_COPY);
		return *this;
	}
	
	// intersection
	common::gui_region& operator &=(const common::gui_region& r) {
		if(this == &r) return *this;
		CombineRgn(m_hrgn, m_hrgn, handle(r), RGN_AND);
		return *this;
	}
	
	// intersection
	common::gui_region& operator &=(const lib::screen_rect<int>& rect) {
		HRGN hrgn = CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom());
		CombineRgn(m_hrgn, m_hrgn, hrgn, RGN_AND);
		DeleteObject((HGDIOBJ) hrgn);
		return *this;
	}
	
	// union
	common::gui_region& operator |=(const common::gui_region& r) {
		if(this == &r) return *this;
		CombineRgn(m_hrgn, m_hrgn, handle(r), RGN_OR);
		return *this;
	}
	
	// difference (this - intersection)
	common::gui_region& operator -=(const common::gui_region& r) {
		if(this == &r) {clear(); return *this;}
		CombineRgn(m_hrgn, m_hrgn, handle(r), RGN_DIFF);
		return *this;
	}
	
	// xor (union - intersection)
	common::gui_region& operator ^=(const common::gui_region& r) {
		if(this == &r) { clear(); return *this;}
		CombineRgn(m_hrgn, m_hrgn, handle(r), RGN_XOR);
		return *this;
	}
	
	// translation
	common::gui_region& operator +=(const lib::point& pt) {
		OffsetRgn(m_hrgn, pt.x, pt.y);
		return *this;
	}
	
	// mem-verifier
	static int s_counter;
        
  private:	
	dx_gui_region(HRGN hrgn) : m_hrgn(hrgn) {s_counter++;}
	HRGN handle() { return m_hrgn;}
	HRGN handle() const { return m_hrgn;}
	static HRGN handle(common::gui_region& r) { return ((dx_gui_region*)&r)->handle();}
	static HRGN handle(const common::gui_region& r) { return ((const dx_gui_region*)&r)->handle();}
	HRGN m_hrgn;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_RGN_H
