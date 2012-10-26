/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
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

class dx_gui_region {
  public:
	// Creates an empty region
	dx_gui_region()
	:	m_hrgn(CreateRectRgn(0, 0, 0, 0)) {
		s_counter++;
	}

	// Creates a rect region
	dx_gui_region(const lib::rect& rect)
	:	m_hrgn(CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom())) {
		s_counter++;
	}

	~dx_gui_region() {
		DeleteObject((HGDIOBJ)m_hrgn);
		s_counter--;
	}

	// Clone factory function
	dx_gui_region *clone() const {
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

	bool overlaps(const lib::rect& rect) const {
		RECT rc = {rect.left(), rect.top(), rect.right(), rect.bottom()};
		return RectInRegion(m_hrgn, &rc) != 0;
	}

	lib::rect get_bounding_box() const {
		RECT rect;
		GetRgnBox(m_hrgn, &rect);
		return lib::rect(lib::point(rect.left, rect.top),
			lib::size(rect.right-rect.left, rect.bottom-rect.top));
	}

	bool operator ==(const dx_gui_region& r) const {
		return EqualRgn(m_hrgn, handle(r)) != 0;
	}

	// assignment
	dx_gui_region& operator =(const lib::rect& rect) {
		SetRectRgn(m_hrgn, rect.left(), rect.top(), rect.right(), rect.bottom());
		return *this;
	}

	// assignment
	dx_gui_region& operator =(const dx_gui_region& r) {
		if(this == &r) return *this;
		CombineRgn(m_hrgn, handle(r), 0, RGN_COPY);
		return *this;
	}

	// intersection
	dx_gui_region& operator &=(const dx_gui_region& r) {
		if(this == &r) return *this;
		CombineRgn(m_hrgn, m_hrgn, handle(r), RGN_AND);
		return *this;
	}

	// intersection
	dx_gui_region& operator &=(const lib::rect& rect) {
		HRGN hrgn = CreateRectRgn(rect.left(), rect.top(), rect.right(), rect.bottom());
		CombineRgn(m_hrgn, m_hrgn, hrgn, RGN_AND);
		DeleteObject((HGDIOBJ) hrgn);
		return *this;
	}

	// union
	dx_gui_region& operator |=(const dx_gui_region& r) {
		if(this == &r) return *this;
		CombineRgn(m_hrgn, m_hrgn, handle(r), RGN_OR);
		return *this;
	}

	// difference (this - intersection)
	dx_gui_region& operator -=(const dx_gui_region& r) {
		if(this == &r) {clear(); return *this;}
		CombineRgn(m_hrgn, m_hrgn, handle(r), RGN_DIFF);
		return *this;
	}

	// xor (union - intersection)
	dx_gui_region& operator ^=(const dx_gui_region& r) {
		if(this == &r) { clear(); return *this;}
		CombineRgn(m_hrgn, m_hrgn, handle(r), RGN_XOR);
		return *this;
	}

	// translation
	dx_gui_region& operator +=(const lib::point& pt) {
		OffsetRgn(m_hrgn, pt.x, pt.y);
		return *this;
	}

	// mem-verifier
	static int s_counter;

  private:
	dx_gui_region(HRGN hrgn) : m_hrgn(hrgn) {s_counter++;}
	HRGN handle() { return m_hrgn;}
	HRGN handle() const { return m_hrgn;}
	static HRGN handle(dx_gui_region& r) { return ((dx_gui_region*)&r)->handle();}
	static HRGN handle(const dx_gui_region& r) { return ((const dx_gui_region*)&r)->handle();}
	HRGN m_hrgn;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_RGN_H
