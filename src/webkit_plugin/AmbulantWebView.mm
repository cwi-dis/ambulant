// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#import "AmbulantWebView.h"
#import <WebKit/WebKit.h>
#include "ambulant/config/config.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/preferences.h"

#ifndef AM_DBG
#define AM_DBG if(getenv("AMBULANT_PLUGIN_DEBUG"))
#endif

void
set_statusline(void *view, const char *msg)
{
}

// Subclasses of various Ambulant classes

class my_cocoa_window_factory : public ambulant::gui::cocoa::cocoa_window_factory
{
  public:
	my_cocoa_window_factory(void *view)
	:	cocoa_window_factory(view) {};

  protected:
	virtual void init_window_size(
		ambulant::gui::cocoa::cocoa_window *window, const std::string &name,
		ambulant::lib::size bounds) {};
};

@implementation AmbulantWebView

- (AmbulantWebView *)init
{
	self = [super init];
	m_arguments = NULL;
	m_mainloop = NULL;
	container = nil;
	return self;
}

+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments
{
	AmbulantWebView *view = [[[self alloc] init] autorelease];
	AM_DBG NSLog(@" view: %@ arguments: %@", self, arguments);
	[view setArguments:arguments];
	return view;
}

- (void)setArguments:(NSDictionary *)arguments
{
	[m_arguments release];
	m_arguments = [arguments retain];
}

- (void)webPlugInInitialize
{
}

- (void)webPlugInStart
{
#if 1
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_prefer_ffmpeg = true;
	prefs->m_use_plugins = true;
#endif
	NSDictionary *webPluginAttributesObj = [m_arguments objectForKey:WebPlugInAttributesKey];
	if (!m_mainloop) {
		container = [m_arguments objectForKey:WebPlugInContainerKey];
		if (container) {
			[container webPlugInContainerShowStatus: @"Ambulant Plugin: Loaded"];
			ambulant::common::plugin_engine *pe = ambulant::common::plugin_engine::get_plugin_engine();
			void *edptr = pe->get_extra_data("python_extra_data");
			if (edptr) {
				*(id*)edptr = container;
				AM_DBG NSLog(@"AmbulantWebKitPlugin: python_extra_data=0x%x, set to 0x%x", edptr, (void *)container);
			} else {
				AM_DBG (@"AmbulantWebKitPlugin: Cannot find python_extra_data, cannot communicate webPlugInContainer");
			}
			edptr = pe->get_extra_data("webkit_extra_data");
			if (edptr) {
				*(id*)edptr = [container webFrame];
				AM_DBG NSLog(@"AmbulantWebKitPlugin: webkit_extra_data=0x%x, set to 0x%x", edptr, (void *)[container webFrame]);
			} else {
				AM_DBG NSLog(@"AmbulantWebKitPlugin: Cannot find webkit_extra_data, cannot communicate WebFrame pointer");
			}
		}
		NSString *urlString = [webPluginAttributesObj objectForKey:@"src"];
		if (urlString != nil && [urlString length] != 0) {
			NSURL *baseUrl = [m_arguments objectForKey:WebPlugInBaseURLKey];
			NSURL *url = [NSURL URLWithString:urlString relativeToURL:baseUrl];
			if (url) {
				m_mainloop = new mainloop([[url absoluteString] UTF8String], self, NULL /*embedder*/);
			}
		}
	}
	NSString *autostartString = [webPluginAttributesObj objectForKey:@"autostart"];
	BOOL autostart = autostartString == nil || [autostartString isEqualToString: @"true"];
	if (m_mainloop && autostart) {
		[self startPlayer];
	}
}

- (void)webPlugInStop
{
	[self stopPlayer];
}

- (void)webPlugInDestroy
{
	AM_DBG NSLog(@" webPluginDestroy %@", self);
	[m_arguments release];
	m_arguments = NULL;
	if (m_mainloop) delete m_mainloop;
	m_mainloop = NULL;
	container = nil;
}

- (void)webPlugInSetIsSelected:(BOOL)isSelected
{
}

// Scripting support

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector
{
	if (selector == @selector(startPlayer)
		|| selector == @selector(stopPlayer)
		|| selector == @selector(restartPlayer)
		|| selector == @selector(pausePlayer)
		|| selector == @selector(resumePlayer)
	) {
		return NO;
	}
	return YES;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)property
{
	return YES;
}

- (id)objectForWebScript
{
	return self;
}

- (void)startPlayer
{
	if (m_mainloop)
		m_mainloop->play();
}

- (void)stopPlayer
{
	if (m_mainloop)
		m_mainloop->stop();
}

- (void)restartPlayer
{
	if (m_mainloop) {
		m_mainloop->restart();
// XXXJACK		m_mainloop->play();
	}
}

- (void)pausePlayer
{
	if (m_mainloop)
		m_mainloop->pause();
}

- (void)resumePlayer
{
	if (m_mainloop)
		m_mainloop->play();
}

- (bool)ignoreResize
{
	return true;
}

- (id)container
{
	return container;
}
@end
