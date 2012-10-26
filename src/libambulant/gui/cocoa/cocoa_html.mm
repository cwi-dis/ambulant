// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_html.h"
//#include "ambulant/gui/cocoa/cocoa_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/lib/callback.h"
#include "ambulant/smil2/test_attrs.h"
#include <WebKit/WebKit.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// Helper classes to allow creating the WebView and loading the URL in the
// main thread. This appears to be needed because otherwise the WebView
// will not load data (because it will use the unused NSRunLoop from the
// current thread).
@interface WebViewController : NSObject
{
	WebView *view;
}
- (WebView *)view;
- (void)create: (NSRectHolder *)rect;
- (void)load: (NSURL *)url;
@end

@implementation WebViewController
- (WebView *)view
{
	return view;
}

- (void)create: (NSRectHolder *)rect
{
	view = [[WebView alloc] initWithFrame: [rect rect] frameName: nil groupName: nil];
	AM_DBG NSLog(@"Created new html viewer %@", view);
}

- (void)load: (NSURL *)url
{
		NSURLRequest *req = [NSURLRequest requestWithURL: url];
		[[view mainFrame] loadRequest: req];
		AM_DBG NSLog(@"viewer %@: loading %@", view, url);
}

@end

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

// ref_counted container for the WebViewController class. Cleans up
// the WebViewController and the WebView when its refcount drops to zero.
class wvc_container : public lib::ref_counted_obj {
	WebViewController *m_wvc;
	int m_generation;
  public:
	wvc_container(WebViewController *it)
	:	m_wvc(it),
		m_generation(0)
	{
	}
	~wvc_container() {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		[[m_wvc view] release];
		[m_wvc release];
		m_wvc = NULL;
		[pool release];
	}
	WebViewController *show() {
		m_generation++;
		return m_wvc;
	}
	void hide_generation(int gen) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		if (m_generation == gen) {
			[[m_wvc view] performSelectorOnMainThread: @selector(removeFromSuperviewWithoutNeedingDisplay) withObject: nil waitUntilDone: NO];
			m_generation++;
			AM_DBG lib::logger::get_logger()->debug("wvc_container: %d: hiding HTML view", gen);
		} else {
			AM_DBG lib::logger::get_logger()->debug("wvc_container: %d: not hiding HTML view", gen);
		}
		[pool release];
	}
	void hide(event_processor *evp) {
		typedef lib::scalar_arg_callback_event<wvc_container, int> hide_cb;
		hide_cb *cb = new hide_cb(this, &wvc_container::hide_generation, m_generation);
		evp->add_event(cb, 1, lib::ep_med);
	}
};

// The (unique) address of my_renderer_id is used to distinguish this renderer class from any
// other classes using the renderer_private_data interfaces.
static common::renderer_private_id my_renderer_id = (common::renderer_private_id)"cocoa_html_browser";

// Helper routine - Get a WebView from a surface, or create
// one if it doesn't exist.
static wvc_container *
_get_html_view(common::surface *surf)
{
	wvc_container *wvc = reinterpret_cast<wvc_container *>(surf->get_renderer_private_data(my_renderer_id));
	if (wvc == NULL) {
		rect amrect = surf->get_rect();
		const lib::point p = surf->get_global_topleft();
		amrect.translate(p);
		NSRectHolder *crect = [[NSRectHolder alloc] initWithRect: NSMakeRect(amrect.left(), amrect.top(), amrect.width(), amrect.height())];
		[crect autorelease];
		WebViewController *ctrl = [[[WebViewController alloc] init] retain];
		[ctrl performSelectorOnMainThread: @selector(create:) withObject: crect waitUntilDone: YES];

		wvc = new wvc_container(ctrl);
	}
	surf->set_renderer_private_data(my_renderer_id, (common::renderer_private_data *)wvc);
	return wvc;
}

extern const char cocoa_html_playable_tag[] = "text";
extern const char cocoa_html_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCocoa");
extern const char cocoa_html_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererHtml");

common::playable_factory *
create_cocoa_html_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCocoa"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererHtml"), true);
	return new common::single_playable_factory<
		cocoa_html_renderer,
		cocoa_html_playable_tag,
		cocoa_html_playable_renderer_uri,
		cocoa_html_playable_renderer_uri2,
		cocoa_html_playable_renderer_uri2>(factory, mdp);
}

void
cocoa_html_renderer::start(double where) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	renderer_playable::start(where);
	if (m_dest) {
//		assert(m_html_view == NULL);
		m_html_view = _get_html_view(m_dest);
		WebViewController *wvc = m_html_view->show();
		WebView *view = [wvc view];

		AM_DBG lib::logger::get_logger()->debug("cocoa_html_renderer: view=0x%x", view);
		net::url url = m_node->get_url("src");
		if (view) {
			AM_DBG lib::logger::get_logger()->debug("cocoa_html_renderer: display %s", url.get_url().c_str());
			// Hook the HTML view into the hierarchy. It is retained there,
			// so we release it.
			cocoa_window *amwindow = (cocoa_window *)m_dest->get_gui_window();
			assert(amwindow);
			AmbulantView *mainview = (AmbulantView *)amwindow->view();
			assert(mainview);
			if ([view superview]) {
				assert([view superview] == mainview);
			} else {
				[mainview addSubview: view];
			}
			// Setup an URL loader and tell the frame about it
			WebFrame *frame = [view mainFrame];
			assert(frame);
			NSString *cstr = [NSString stringWithUTF8String: url.get_url().c_str()];
			NSURL *curl = [NSURL URLWithString: cstr];
			[wvc performSelectorOnMainThread: @selector(load:) withObject: curl waitUntilDone: NO];
		}
	}
	m_lock.leave();
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
	[pool release];
}

bool
cocoa_html_renderer::stop() {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	m_lock.enter();
	if (m_html_view) {
		AM_DBG lib::logger::get_logger()->debug("cocoa_html_renderer: stop display");
		// Unhook the view from the view hierarchy.
		m_html_view->hide(m_event_processor);
		m_html_view = NULL;
		//		lib::logger::get_logger()->debug("cocoa_html_renderer: %f%% done", [view estimatedProgress]);
		//		if ([[view mainFrame] dataSource] == nil) lib::logger::get_logger()->debug("cocoa_html_renderer: not complete yet!");
		//		// [view removeFromSuperviewWithoutNeedingDisplay];
	}
	renderer_playable::stop();
	m_context->stopped(m_cookie);
	m_lock.leave();
	[pool release];
	return true; // Don't re-use this renderer
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

