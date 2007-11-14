//
//  AmbulantWebView.mm
//  AmbulantWebKitPlugin
//
//  Created by Jack Jansen on 20-12-05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import "AmbulantWebView.h"
#import <WebKit/WebKit.h>
#include "ambulant/config/config.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/preferences.h"

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

+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments
{
    AmbulantWebView *view = [[[self alloc] init] autorelease];
	NSLog(@"arguments: %@", arguments);
    [view setArguments:arguments];
    return view;
}

- (void)dealloc
{   
    [m_arguments release];
    [super dealloc];
}

- (void)setArguments:(NSDictionary *)arguments
{
    [arguments copy];
    [m_arguments release];
    m_arguments = arguments;
}

- (void)webPlugInInitialize
{
}

- (void)webPlugInStart
{
#if 1
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_prefer_ffmpeg = false;
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
			} else {
				NSLog(@"AmbulantWebKitPlugin: Cannot find python_extra_data, cannot communicate webPlugInContainer");
			}
			edptr = pe->get_extra_data("webkit_extra_data");
			if (edptr) {
				*(id*)edptr = [container webFrame];
			} else {
				NSLog(@"AmbulantWebKitPlugin: Cannot find webkit_extra_data, cannot communicate WebFrame pointer");
			}
		}
        NSString *urlString = [webPluginAttributesObj objectForKey:@"src"];
        if (urlString != nil && [urlString length] != 0) {
            NSURL *baseUrl = [m_arguments objectForKey:WebPlugInBaseURLKey];
            NSURL *url = [NSURL URLWithString:urlString relativeToURL:baseUrl];
			if (url) {
				m_mainloop = new mainloop([[url absoluteString] UTF8String], self, false, NULL /*embedder*/);			
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
