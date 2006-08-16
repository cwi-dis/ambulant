// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

//
//  MyDocument.m
//  cocoambulant
//
//  Created by Jack Jansen on Thu Sep 04 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "MyDocument.h"
#import "MyAmbulantView.h"
#import "ambulant/common/preferences.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// Help class for fullscreen windows: normally, windows
// with style NSBorderlessWindowMask don't get any keyboard input.
// By overriding canBecomeKeyWindow we fix that.
@interface FullScreenWindow : NSWindow
{
}
- (BOOL)canBecomeKeyWindow;
@end
@implementation FullScreenWindow
- (BOOL)canBecomeKeyWindow
{
	return YES;
}
@end

void
document_embedder::show_file(const ambulant::net::url& href)
{
	CFStringRef cfhref = CFStringCreateWithCString(NULL, href.get_url().c_str(), kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithString(NULL, cfhref, NULL);
	OSErr status;
	
	if ((status=LSOpenCFURLRef(url, NULL)) != 0) {
		ambulant::lib::logger::get_logger()->trace("Opening URL <%s>: LSOpenCFURLRef error %d", href.get_url().c_str(), status);
		ambulant::lib::logger::get_logger()->error(gettext("Cannot open: %s"), href.get_url().c_str());
	}
}

void
document_embedder::close(ambulant::common::player *p)
{
	[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
}

void
document_embedder::open(ambulant::net::url newdoc, bool start, ambulant::common::player *old)
{
	if (old) {
		AM_DBG NSLog(@"performSelectorOnMainThread: close: on 0x%x", (void*)m_mydocument);
		[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
	}
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *str_url = [NSString stringWithCString: newdoc.get_url().c_str()];
	NSURL *url = [NSURL URLWithString: str_url];
	NSDocumentController *docController = [NSDocumentController sharedDocumentController];
	NSDocument *doc = [docController openDocumentWithContentsOfURL:url display:YES];
	if (!doc) {
		ambulant::lib::logger::get_logger()->error(gettext("Cannot open: %s"), newdoc.get_url().c_str());
	}
	[pool release];
	// [doc retain] ??
	
}

#ifdef WITH_AUX_DOCUMENT
bool
document_embedder::aux_open(const ambulant::net::url& auxdoc)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (auxdoc.get_url() == "") {
		NSLog(@"aux_open: closing");
		[m_mydocument closeAuxDocument];
		return true;
	}
	NSString *str_url = [NSString stringWithCString: auxdoc.get_url().c_str()];
	NSURL *url = [NSURL URLWithString: str_url];
	NSLog(@"aux_open: open %@", url);
	BOOL rv = [m_mydocument openAuxDocument: url];
	[pool release];
	return (bool)rv;
}
#endif // WITH_AUX_DOCUMENT

@implementation MyDocument

- (id)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
    
    }
	saved_window = nil;
    return self;
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
	[[view window] makeFirstResponder: view];
	[[view window] setAcceptsMouseMovedEvents: YES];

	if ([self fileURL] == nil) {
		[self askForURL: self];
	} else {
		[self openTheDocument];
	}
	[self validateButtons: self];
}

- (void)askForURL: (id)sender
{
	AM_DBG NSLog(@"Show sheet to ask for URL");
	[self showWindows];
	[NSApp beginSheet: ask_url_panel
		modalForWindow:[self windowForSheet] 
		modalDelegate:self 
		didEndSelector:@selector(askForURLDidEnd:returnCode:contextInfo:) 
		contextInfo:nil];
}

- (IBAction)closeURLPanel:(id)sender
{
	[ask_url_panel orderOut:self];
	[NSApp endSheet:ask_url_panel returnCode:([sender tag] == 1) ? NSOKButton : NSCancelButton];
}

- (void)askForURLDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	if (returnCode == NSOKButton && [[url_field stringValue] length] > 0) {
		AM_DBG NSLog(@"ask_for_url: User said OK: %@", [url_field stringValue]);
		[self setFileURL: [NSURL URLWithString: [url_field stringValue]]];
		[self openTheDocument];
	} else {
		AM_DBG NSLog(@"ask_for_url: User said cancel");
		[self close];
	}
}

- (void)openTheDocument
{
    NSString *url = [[self fileURL] absoluteString];
#if 0
	// XXX This is incorrect
	if ( [[self fileURL] isFileURL] && ![[self fileURL] fragment]) {
		NSString *escapedurl = (NSString *)CFURLCreateStringByAddingPercentEscapes(NULL,
			(CFStringRef)url, NULL, NULL, kCFStringEncodingUTF8);
		//[url release];
		url = escapedurl;
	}
#endif
	bool use_mms = ([[url pathExtension] compare: @".mms"] == 0);
	embedder = new document_embedder(self);
	myMainloop = new mainloop([url UTF8String], view, use_mms, embedder);
	[self play: self];
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // Insert code here to write your document from the given data.  You can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.
    return nil;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    // Insert code here to read your document from the given data.  You can also choose to override -loadFileWrapperRepresentation:ofType: or -readFromFile:ofType: instead.
    return YES;
}

- (BOOL) validateUIItem:(id)UIItem
{
	AM_DBG NSLog(@"Validating %@", UIItem);
	SEL theAction = [UIItem action];
	if (!myMainloop) {
		// No document: no checkmarks and grayed for all items
		[UIItem setState: NSOffState];
		return NO;
	}
	
	if (theAction == @selector(play:)) {
		if (myMainloop->is_play_active()) {
			AM_DBG NSLog(@"play - On");
			[UIItem setState: NSOnState];
		} else {
			AM_DBG NSLog(@"play - Off");
			[UIItem setState: NSOffState];
		}
		return myMainloop->is_play_enabled();
	} else if (theAction == @selector(stop:)) {
		if (myMainloop->is_stop_active()) {
			AM_DBG NSLog(@"stop - On");
			[UIItem setState: NSOnState];
		} else {
			AM_DBG NSLog(@"stop - Off");
			[UIItem setState: NSOffState];
		}
		return myMainloop->is_stop_enabled();
	} else if (theAction == @selector(pause:)) {
		if (myMainloop->is_pause_active()) {
			AM_DBG NSLog(@"pause - On");
			[UIItem setState: NSOnState];
		} else {
			AM_DBG NSLog(@"pause - On");
			[UIItem setState: NSOffState];
		}
		return myMainloop->is_pause_enabled();
	} else if (theAction == @selector(toggleFullScreen:)) {
		if (saved_window) {
			[UIItem setState: NSOnState];
		} else {
			[UIItem setState: NSOffState];
		}
		return YES;
	}
	return NO;
}

- (BOOL) validateMenuItem:(id)menuItem
{
	return [self validateUIItem: menuItem];
}

- (void) validateButtons: (id)dummy
{
	if (!play_button || !stop_button || !pause_button) return;
	BOOL enabled;
	enabled = [self validateUIItem: play_button];
	[play_button setEnabled: enabled];
	enabled = [self validateUIItem: stop_button];
	[stop_button setEnabled: enabled];
	enabled = [self validateUIItem: pause_button];
	[pause_button setEnabled: enabled];
}

- (IBAction)pause:(id)sender
{
    if (myMainloop) myMainloop->pause();
	[self validateButtons: nil];
}

- (IBAction)play:(id)sender
{
	if (!myMainloop) return;
	[NSThread detachNewThreadSelector: @selector(startPlay:) toTarget: self withObject: NULL];
	[self validateButtons: nil];
}

- (void)startPlay: (id)dummy
{
	// XXXX Jack thinks that this extra thread is no longer needed (20060124)
	if (!myMainloop) return;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    assert([NSThread isMultiThreaded]);
	myMainloop->play();
	// We don't use refcounting on myMainloop, because
	// otherwise our player infrastructure will be destructed in this
	// thread, and at that time the window (and the ambulantWidget) is
	// gone. So the main thread does the cleanup and zaps myMainloop.
	while (myMainloop && (myMainloop->is_play_active()||myMainloop->is_pause_active())) {
		AM_DBG NSLog(@"validating in separate thread");
		[self validateButtons: nil];
		sleep(1);
	}
	AM_DBG NSLog(@"validating in separate thread - final");
	[self validateButtons: nil];
    [pool release];
	// myMainloop->release();
}

- (IBAction)stop:(id)sender
{
    AM_DBG NSLog(@"Stop");
	if (myMainloop) myMainloop->stop();
	[self validateButtons: nil];
}

- (void *)view
{
    return view;
}

- (void)close
{
	[self stop: self];
	play_button = nil;
	stop_button = nil;
	pause_button = nil;
#ifdef WITH_AUX_MAINLOOP
	delete myAuxMainloop;
	myAuxMainloop = NULL;
#endif
	delete myMainloop;
	myMainloop = NULL;
	delete embedder;
	embedder = NULL;
	[super close];
}

- (void)close: (id)dummy
{
	[self close];
}

- (void)fixMouse: (id)dummy
{
	mainloop *ml = myMainloop;
#ifdef WITH_AUX_MAINLOOP
	if (myAuxMainloop) ml = myAuxMainloop;
#endif
	if (!ml) return;
	int cursor = ml->after_mousemove();
	AM_DBG NSLog(@"Fixing mouse to %d", cursor);
	if (cursor == 0) {
		if ([NSCursor currentCursor] != [NSCursor arrowCursor]) {
			[[NSCursor arrowCursor] set];
			// XXX This is a bit of a hack: hovering over an anchor
			// stats the status line, but the reset "never happens"
			// so we clear the status line here. This should really
			// be done more intelligently in smil_player.
			if (status_line) [status_line setStringValue: @""];
		}
	} else if (cursor == 1) {
		if ([NSCursor currentCursor] != [NSCursor pointingHandCursor])
			[[NSCursor pointingHandCursor] set];
	} else {
		NSLog(@"Warning: unknown cursor index %d", cursor);
	}
}

- (void)resetMouse: (id)dummy
{
	mainloop *ml = myMainloop;
#ifdef WITH_AUX_MAINLOOP
	if (myAuxMainloop) ml = myAuxMainloop;
#endif
	if (ml) ml->before_mousemove(0);
}

- (void)keyDown: (NSEvent *)ev
{
	mainloop *ml = myMainloop;
#ifdef WITH_AUX_MAINLOOP
	if (myAuxMainloop) ml = myAuxMainloop;
#endif
	NSString *chars = [ev characters];
	
	if (chars && [chars length] == 1 && ml) {
		unichar ch = [chars characterAtIndex:0];
		// First, escape will exit fullscreen mode
		if (ch == '\033') {
			[self goWindowMode: self];
			return;
		}
		ambulant::common::preferences* prefs = ambulant::common::preferences::get_preferences();

		if (prefs->m_tabbed_links) {
			if (ch == '\t') {
				ml->on_focus_advance();
				return;
			}
			if (ch == '\r' || ch == '\n') {
				ml->on_focus_activate();
				return;
			}
		}
		ml->on_char(ch);
	} else {
		AM_DBG NSLog(@"MyDocument::keyDown: dropping %@", chars);
	}
}

- (void) setStatusLine: (NSString *)message
{
	if (status_line) [status_line setStringValue: message];
	[message release];
}

- (IBAction)goWindowMode:(id)sender
{
	if (!saved_window) {
		NSLog(@"goWindowMode: already in window mode");
		return;
	}
    // Get the screen information.
    NSScreen* screen = [[view window] screen];
	if (screen == NULL) screen = [NSScreen mainScreen]; 
    NSDictionary* screenInfo = [screen deviceDescription]; 
    NSNumber* screenID = [screenInfo objectForKey:@"NSScreenNumber"];

    // Release the screen.
    CGDirectDisplayID displayID = (CGDirectDisplayID)[screenID longValue]; 
    CGDisplayErr err = CGDisplayRelease(displayID);
    if (err != CGDisplayNoErr) {
		NSLog(@"goFullScreen: CGDisplayRelease failed");
		return;
	}
	
	// Attach our view to the normal window. 
	NSWindow *mScreenWindow = [view window];
	NSView *savedcontentview = [saved_window contentView];
	[savedcontentview addSubview: view];
	[view setFrame: saved_view_rect];
	[savedcontentview setNeedsDisplay:YES];
	[saved_window makeFirstResponder: view];
	[saved_window setAcceptsMouseMovedEvents: YES];

	// Tell our controller that the normal window is in use again.
	NSWindowController* winController = [[self windowControllers]
											 objectAtIndex:0];
	[winController setWindow:saved_window];

	// Get rid of the fullscreen window
	[mScreenWindow close];
	[saved_window makeKeyAndOrderFront:self];
	
	// And clear saved_window, which signals we're in normal mode again.
	[saved_window release];
	saved_window = nil;
}

- (IBAction)goFullScreen:(id)sender
{
	if (saved_window) {
		NSLog(@"goFullScreen: already in fullscreen mode");
		return;
	}
    // Get the screen information.
    NSScreen* screen = [[view window] screen];
	if (screen == NULL) screen = [NSScreen mainScreen]; 
    NSDictionary* screenInfo = [screen deviceDescription]; 
    NSNumber* screenID = [screenInfo objectForKey:@"NSScreenNumber"];
	NSLog(@"goFullScreen: screenID = %@", screenID);
 
    // Capture the screen.
    CGDirectDisplayID displayID = (CGDirectDisplayID)[screenID longValue]; 
    CGDisplayErr err = CGDisplayCapture(displayID);
    if (err != CGDisplayNoErr) {
		NSLog(@"goFullScreen: CGDisplayCapture failed");
		return;
	}

	// Create the full-screen window.
	NSRect winRect = [screen frame];
#if 1
	// The (x, y) coordinates are nonzero for a non-primary screen, it appears that
	// the rect is for the virtual combination of all screens, with (0, 0) rooted
	// at the origin of the main screen.
	winRect.origin.x = 0;
	winRect.origin.y = 0;
#endif
	NSWindow *mScreenWindow;
	mScreenWindow = [[FullScreenWindow alloc] initWithContentRect:winRect
			styleMask:NSBorderlessWindowMask 
			backing:NSBackingStoreBuffered 
			defer:NO 
			screen:screen];

	// Establish the window attributes.
	[mScreenWindow setDelegate:self];
	[mScreenWindow setBackgroundColor: [NSColor blackColor]];

	// Remember the old window, and move our view to the fullscreen
	// window.
	saved_window = [view window];
	[saved_window retain];

	// Create the outer view on the fullscreen window, and insert the
	// ambulant view within it.
	NSView *fsmainview = [[NSView alloc] initWithFrame: winRect];
	
	id contentview = view;
	saved_view_rect = [contentview frame];
	[fsmainview addSubview: contentview];
	NSRect contentRect = [contentview frame];
	float xExtra = NSWidth(winRect) - NSWidth(contentRect);
	float yExtra = NSHeight(winRect) - NSHeight(contentRect);
	[contentview setFrameOrigin: NSMakePoint(xExtra/2, yExtra/2)];
	
	[mScreenWindow setContentView: fsmainview];
	[fsmainview setNeedsDisplay:YES];
	[fsmainview release];
	[mScreenWindow makeFirstResponder: contentview];
	[mScreenWindow setAcceptsMouseMovedEvents: YES];

	// Make the screen window the current document window.
	// Be sure to retain the previous window if you want to  use it again.
	NSWindowController* winController = [[self windowControllers]
											 objectAtIndex:0];
	[winController setWindow:mScreenWindow];

	// The window has to be above the level of the shield window.
	int32_t     shieldLevel = CGShieldingWindowLevel();
	[mScreenWindow setLevel:shieldLevel];

	// Show the window.
	[mScreenWindow makeKeyAndOrderFront:self];
}

- (IBAction) toggleFullScreen: (id)sender
{
	if (saved_window)
		[self goWindowMode: sender];
	else
		[self goFullScreen:sender];
}

#ifdef WITH_AUX_DOCUMENT
- (BOOL)openAuxDocument: (NSURL *)auxUrl
{
//	embedder = new document_embedder(self);
	delete myAuxMainloop;
	if (myAuxView) {
		[myAuxView removeFromSuperview];
		myAuxView = NULL;
	}
	myAuxView = [[MyAmbulantView alloc] initWithFrame: [view bounds]];
	[view addSubview: myAuxView];
	[[view window] makeFirstResponder: myAuxView];
	NSLog(@"openAuxDocument %@", auxUrl);
	myAuxMainloop = new mainloop([[auxUrl absoluteString] UTF8String], myAuxView, false, NULL);
	myAuxMainloop->play();
	return true;
}

- (void)closeAuxDocument
{
	NSLog(@"closeAuxDocument");
	delete myAuxMainloop;
	myAuxMainloop = NULL;
	if (myAuxView) {
		[myAuxView removeFromSuperview];
		myAuxView = NULL;
		[[view window] makeFirstResponder: view];
	}
}
#endif // WITH_AUX_DOCUMENT
@end
