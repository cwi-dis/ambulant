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

#ifndef AMBULANT_GUI_COCOA_COCOA_GUI_H
#define AMBULANT_GUI_COCOA_COCOA_GUI_H

#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
#endif

namespace ambulant {

namespace gui {

namespace cocoa {

class cocoa_window : public common::gui_window {
  public:
  	cocoa_window(const std::string &name, lib::size bounds, void *_view, common::gui_events *handler)
  	:	common::gui_window(handler),
  		m_view(_view) {};
	~cocoa_window();
  		
	void need_redraw(const lib::screen_rect<int> &r);
	void need_events(bool want);
	
	void redraw(const lib::screen_rect<int> &r);
	void user_event(const lib::point &where, int what = 0);

	void *view() { return m_view; }
	
  private:
    void *m_view;
};

;
class cocoa_window_factory : public common::window_factory {
  public:
  	cocoa_window_factory(void *view)
  	:	m_defaultwindow_view(view) {}
  	
	common::gui_window *new_window(const std::string &name, lib::size bounds, common::gui_events *handler);
	common::renderer *new_background_renderer(const common::region_info *src);
  private:
    void *m_defaultwindow_view;
};

class cocoa_renderer_factory : public common::playable_factory {
  public:
  	cocoa_renderer_factory(net::datasource_factory *df)
	:   m_datasource_factory(df) {}
  	
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
  private:
    net::datasource_factory *m_datasource_factory;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#ifdef __OBJC__
@interface AmbulantView : NSView
{
    ambulant::gui::cocoa::cocoa_window *ambulant_window;
	NSImage *transition_surface;
	int transition_count;
}

- (id)initWithFrame:(NSRect)frameRect;

- (void)setAmbulantWindow: (ambulant::gui::cocoa::cocoa_window *)window;
- (void)ambulantWindowClosed;
- (bool)isAmbulantWindowInUse;
- (BOOL)isFlipped;

- (NSRect) NSRectForAmbulantRect: (const ambulant::lib::screen_rect<int> *)arect;
- (ambulant::lib::screen_rect<int>) ambulantRectForNSRect: (const NSRect *)nsrect;

- (void)mouseDown: (NSEvent *)theEvent;
- (void)mouseMoved: (NSEvent *)theEvent;

- (void)dumpToImageID: (char *)ident;
- (void)dump: (id)image toImageID: (char *)ident;
- (BOOL)wantsDefaultClipping;

- (void) incrementTransitionCount;
- (void) decrementTransitionCount;
- (NSImage *)getTransitionSurface;
- (NSImage *)getTransitionOldSource;
- (NSImage *)getTransitionNewSource;
@end

#endif // __OBJC__
#endif // AMBULANT_GUI_COCOA_COCOA_GUI_H
