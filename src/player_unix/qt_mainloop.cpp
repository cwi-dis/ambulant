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

#include "qt_mainloop.h"
#ifdef WITH_ARTS
#include "ambulant/gui/arts/arts.h"
#endif
#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_gui.h"
#endif
#include "ambulant/lib/document.h"

using namespace ambulant;
using namespace lib;
using namespace gui;
using namespace qt;
 
void*
qt_mainloop::run(void* view) {
	qt_gui* qt_view = (qt_gui*) view;
	qt_window_factory *wf;

	AM_DBG logger::get_logger()->trace(
		"qt_mainloop::run(qt_gui=0x%x)",
		view);

	document *doc = document::create_from_file(qt_view->filename());

	common::global_renderer_factory *rf =
		new common::global_renderer_factory(); 
#ifdef WITH_SDL
	AM_DBG logger::get_logger()->trace("add factory for SDL");
	rf->add_factory( new sdl::sdl_renderer_factory() );
AM_DBG logger::get_logger()->trace("add factory for SDL done");
#endif
#ifdef WITH_ARTS
	rf->add_factory(new arts::arts_renderer_factory());
#endif 
	rf->add_factory(new qt_renderer_factory());
 
	wf = new qt_window_factory(qt_view, 
				   qt_view->get_o_x(),
			 	   qt_view->get_o_y());
			 
	common::abstract_player *a;
#ifdef WITH_MMS_PLAYER
	a = new mms::mms_player(doc, wf, rf);
#else
	a = new smil2::smil_player(doc, wf, rf);
#endif
				
	a->start();

	while(!a->is_done())
		sleep(1);

	// XXXX Should we call a callback in the parent?

	return (void*) 1;
}
