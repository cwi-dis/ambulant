//
//  AmbulantWebView.mm
//  AmbulantWebKitPlugin
//
//  Created by Jack Jansen on 20-12-05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import "AmbulantWebView.h"
#import <WebKit/WebKit.h>

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
    if (!m_mainloop) {
        NSDictionary *webPluginAttributesObj = [m_arguments objectForKey:WebPlugInAttributesKey];
        NSString *urlString = [webPluginAttributesObj objectForKey:@"src"];
        if (urlString != nil && [urlString length] != 0) {
            NSURL *baseUrl = [m_arguments objectForKey:WebPlugInBaseURLKey];
            NSURL *url = [NSURL URLWithString:urlString relativeToURL:baseUrl];
			if (url) {
				m_mainloop = new mainloop([[url absoluteString] UTF8String], self, false, NULL /*embedder*/);			
			}
		}
    }
	if (m_mainloop) {
		[self startPlayer];
	}
}

- (void)webPlugInStop
{
	[self stopPlayer];
}

- (void)webPlugInDestroy
{
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

@end
