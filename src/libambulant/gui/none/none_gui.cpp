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

#include "ambulant/gui/none/none_gui.h"
#include "ambulant/common/renderer.h"
#include "ambulant/common/region_info.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;
using namespace lib;
using namespace common;

void
gui::none::none_playable::start(double where)
{
#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_OPERATORS_IN_NAMESPACE)
	lib::logger::ostream os = lib::logger::get_logger()->trace_stream();
	os << "none_playable.start(" << (void *)this;
	os << ", node=" << *m_node;
	os << ")" << lib::endl;
#endif
	stopped_callback();
}

void
gui::none::none_playable::stop()
{
	lib::logger::get_logger()->trace("none_playable.stop(0x%x)", (void *)this);
}

void
gui::none::none_background_renderer::drawbackground(
	const region_info *src, 
	const screen_rect<int> &dirty, 
	surface *dst, abstract_window *window)
{
	lib::logger::get_logger()->trace("none_background_renderer.redraw(0x%x) from 0x%x to 0x%x", (void *)this, (void*)src, (void*)dst);
}

playable *
gui::none::none_playable_factory::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
	return new none_playable(context, cookie, node);
}

abstract_bg_rendering_source *
gui::none::none_window_factory::new_background_renderer()
{
	return new none_background_renderer();
}

abstract_window *
gui::none::none_window_factory::new_window(const std::string &name, size bounds, renderer *region)
{
	return new none_window(name, bounds, region);
}

