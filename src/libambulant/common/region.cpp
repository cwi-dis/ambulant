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

#include "ambulant/lib/logger.h"
#include "ambulant/common/region.h"
#include "ambulant/common/renderer.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::passive_region *
lib::passive_region::subregion(const std::string &name, screen_rect<int> bounds)
{
	point topleft = m_window_topleft + bounds.left_top();
	passive_region *rv = new passive_region(name, this, bounds, topleft);
	m_children.push_back(rv);
	return rv;
}

lib::active_region *
lib::passive_region::activate(const node *node)
{
	active_region *rv = new lib::active_region(this, node);
	return rv;
}

void
lib::passive_region::show(active_region *cur)
{
	m_cur_active_region = cur;
	AM_DBG lib::logger::get_logger()->trace("passive_region.show(0x%x, active=0x%x)", (void *)this, (void *)m_cur_active_region);
}

void
lib::passive_region::redraw(const screen_rect<int> &r, abstract_window *window)
{
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x, ltrb=(%d, %d, %d, %d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	screen_rect<int> our_outer_rect = r & m_outer_bounds;
	screen_rect<int> our_rect = m_outer_bounds.innercoordinates(our_outer_rect);
	if (our_rect.empty())
		return;
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x, our_ltrb=(%d, %d, %d, %d))", (void *)this, our_rect.left(), our_rect.top(), our_rect.right(), our_rect.bottom());
		
	if (m_cur_active_region) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) ->active 0x%x", (void *)this, (void *)m_cur_active_region);
		m_cur_active_region->redraw(our_rect, window);
	} else {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) no active region", (void *)this);
	}
	std::vector<passive_region *>::iterator i;
	for(i=m_children.begin(); i<m_children.end(); i++) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) -> child 0x%x", (void *)this, (void *)(*i));
		(*i)->redraw(our_rect, window);
	}
}

void
lib::passive_region::need_redraw(const screen_rect<int> &r)
{
	if (!m_parent)
		return;   // Audio region or some such
	screen_rect<int> parent_rect = r & m_inner_bounds;
	m_parent->need_redraw(m_outer_bounds.outercoordinates(parent_rect));
}

lib::passive_root_layout::passive_root_layout(const std::string &name, size bounds, window_factory *wf)
:   passive_region(name, NULL, screen_rect<int>(point(0, 0), size(bounds.w, bounds.h)), point(0, 0))
{
	m_gui_window = wf->new_window(name, bounds, this);
}
		
lib::passive_root_layout::~passive_root_layout()
{
	delete m_gui_window;
	m_gui_window = NULL;
}

void
lib::passive_root_layout::need_redraw(const screen_rect<int> &r)
{
	m_gui_window->need_redraw(r);
}

void
lib::active_region::show(abstract_rendering_source *renderer)
{
	m_renderer = renderer;
	m_source->show(this);
	AM_DBG lib::logger::get_logger()->trace("active_region.show(0x%x, \"%s\", renderer=0x%x)", (void *)this, m_source->m_name.c_str(), (void *)renderer);
	need_redraw();
}

void
lib::active_region::redraw(const screen_rect<int> &r, abstract_window *window)
{
	if (m_renderer) {
		AM_DBG lib::logger::get_logger()->trace("active_region.redraw(0x%x) -> renderer 0x%x", (void *)this, (void *)m_renderer);
		m_renderer->redraw(r, window);
	} else {
		// At this point we should have a renderer that draws the default background
		// When that is implemented this trace message should turn into an error (or fatal).
		AM_DBG lib::logger::get_logger()->trace("active_region.redraw(0x%x) no renderer", (void *)this);
	}
}

void
lib::active_region::need_redraw(const screen_rect<int> &r)
{
	m_source->need_redraw(r);
}

void
lib::active_region::need_redraw()
{
	need_redraw(m_source->m_inner_bounds);
}

void
lib::active_region::renderer_done()
{
	m_renderer = NULL;
	AM_DBG lib::logger::get_logger()->trace("active_region.done(0x%x, \"%s\")", (void *)this, m_source->m_name.c_str());
	need_redraw();
}
