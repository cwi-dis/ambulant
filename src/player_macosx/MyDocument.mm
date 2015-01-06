// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
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

//
//	MyDocument.m
//	cocoambulant
//
//	Created by Jack Jansen on Thu Sep 04 2003.
//	Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "MyDocument.h"
#import "MyAmbulantView.h"
#import "ambulant/common/preferences.h"

#ifndef CGFLOAT_DEFINED
typedef float CGFloat;
#endif

//#define	AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

/*"""AM_DBG"""*/ void dumpResponderChain(NSResponder *r) {
    std::string x = "";
    while (r) {
        NSLog(@"%snext responder %@", x.c_str(), r);
        x += " ";
        r = [r nextResponder];
    }
}

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
document_embedder::done(ambulant::common::player *p)
{
	[m_mydocument performSelectorOnMainThread: @selector(validateButtons:) withObject: nil waitUntilDone: NO];
}

void
document_embedder::open(ambulant::net::url newdoc, bool start, ambulant::common::player *old)
{
#ifdef WITH_OVERLAY_WINDOW
	if (newdoc.get_protocol() == "ambulant_aux") {
		std::string aux_url = newdoc.get_url();
		aux_url = aux_url.substr(13);
		ambulant::net::url auxdoc = ambulant::net::url::from_url(aux_url);
		aux_open(auxdoc);
	}
#endif

	if (old) {
		AM_DBG NSLog(@"performSelectorOnMainThread: close: on %p", (void*)m_mydocument);
		[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *str_url = [NSString stringWithUTF8String: newdoc.get_url().c_str()];
	NSURL *url = [NSURL URLWithString: str_url];
	NSDocumentController *docController = [NSDocumentController sharedDocumentController];
	NSError *error;
	NSDocument *doc = [docController openDocumentWithContentsOfURL:url display:YES error:&error];
	if (!doc) {
		ambulant::lib::logger::get_logger()->error(gettext("Cannot open: %s, error: %s"), newdoc.get_url().c_str(), [[error localizedDescription] UTF8String]);
	}
	[pool release];
	// [doc retain] ??

}

#ifdef WITH_OVERLAY_WINDOW
bool
document_embedder::aux_open(const ambulant::net::url& auxdoc)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (auxdoc.get_url() == "") {
		AM_DBG NSLog(@"aux_open: closing");
		[m_mydocument closeAuxDocument];
		return true;
	}
	NSString *str_url = [NSString stringWithUTF8String: auxdoc.get_url().c_str()];
	NSURL *url = [NSURL URLWithString: str_url];
	AM_DBG NSLog(@"aux_open: open %@", url);
	BOOL rv = [m_mydocument openAuxDocument: url];
	[pool release];
	return (bool)rv;
}
#endif // WITH_OVERLAY_WINDOW

@implementation ScalerView
- (void) awakeFromNib
{
	autoScale = YES;
	autoCenter = YES;
	scaleFactor = 1.0;
    resizingWindow = false;
}

- (BOOL)isFlipped
{
	return true;
}

- (MyAmbulantView *)getPlayer
{
	if ([[self subviews] count] == 0) return nil;
	return [[self subviews] objectAtIndex: 0];
}

- (void) viewDidMoveToSuperview
{
	AM_DBG NSLog(@"ScalerView %@ moved to superview %@", self, self.superview);
	// Resize the player view (if it exists) to fit our size
	
	MyAmbulantView *playerView = [self getPlayer];
	if (playerView == nil) return;
	NSRect origPlayerBounds = playerView.bounds;
	
	// Compute the aspect-ratio-maintaing scale factor that is needed to make the content as big as possible
	// given our size.
	CGFloat scaleX = self.bounds.size.width / playerView.bounds.size.width;
	CGFloat scaleY = self.bounds.size.height / playerView.bounds.size.height;
	scaleFactor = fmin(scaleX, scaleY);
    AM_DBG NSLog(@"scale is %f", scaleFactor);

	// Set the frame of the player view to the new size, while maintaing the inner coordinate
	// system of the player view.
    CGSize newPlayerFrame = CGSizeMake(playerView.bounds.size.width*scaleFactor, playerView.bounds.size.height*scaleFactor);
    [playerView setFrameSize: NSSizeFromCGSize(newPlayerFrame)];
	[playerView setBounds: origPlayerBounds];
	
	// Center the player view within our bounds.
	CGFloat extraX = self.bounds.size.width - playerView.frame.size.width;
	CGFloat extraY = self.bounds.size.height - playerView.frame.size.height;
	[playerView setFrameOrigin: NSMakePoint(extraX/2, extraY/2)];
}

- (void) recomputeZoom
{
	MyAmbulantView *playerView = [self getPlayer];
	if (playerView == nil) return;
	AM_DBG NSLog(@"recomputeZoom, self.bounds %f,%f,%f,%f",
		self.bounds.origin.x, self.bounds.origin.y, self.bounds.size.width, self.bounds.size.height);
	AM_DBG NSLog(@"recomputeZoom,  playerview.frame %f,%f,%f,%f",
		playerView.frame.origin.x, playerView.frame.origin.y, playerView.frame.size.width, playerView.frame.size.height);

	// Two possibilities: either we resize the window to match the player view size, or we
	// scale the player view to match our size. Find out which is the case.
	bool resizeWindow = true;
	NSWindow *window = [self window];
	int32_t shieldLevel = CGShieldingWindowLevel();
	if ([window level] >= shieldLevel)
		resizeWindow = false;
		
	if (resizeWindow) {
        // We are in window mode. We want to resize the window to fit the document.
        // First, we want to determine how many pixels are used in the window that are not part of this view,
        // this is the area used for buttons and such.
		NSView* contentView = [window contentView];
		CGFloat extraWidth = contentView.bounds.size.width - self.frame.size.width;
		CGFloat extraHeight = contentView.bounds.size.height - self.frame.size.height;
        
        // We expect here that the player view is rooted at (0,0). We also make its frame equal to its bounds.
        // Note that I'm not 100% sure these asserts are needed: we could just assume that the player view takes
        // care of its bounds/frame factors, and we only look at frame.
        // For now, we set the player view frame to be identical to its bounds*scale.
        assert(playerView.bounds.origin.x == 0);
        assert(playerView.bounds.origin.y == 0);
		NSRect playerBounds = playerView.bounds;
        NSRect playerFrame = playerView.bounds;
        playerFrame.size.width *= scaleFactor;
        playerFrame.size.height *= scaleFactor;
 		[playerView setFrame: playerFrame];
		[playerView setBounds: playerBounds];
		assert(playerView.bounds.origin.x == playerView.frame.origin.x);
		assert(playerView.bounds.origin.y == playerView.frame.origin.y);
		assert(fabs(playerView.bounds.size.width*scaleFactor-playerView.frame.size.width) < 2.0);
		assert(fabs(playerView.bounds.size.height*scaleFactor-playerView.frame.size.height) < 2.0);
        
        // Now we set our own size
        [self setFrameSize: playerFrame.size];
 		[playerView setFrame: playerFrame];
        [self setBounds: playerFrame];
		assert(playerView.bounds.origin.x == playerView.frame.origin.x);
		assert(playerView.bounds.origin.y == playerView.frame.origin.y);
		assert(fabs(playerView.bounds.size.width*scaleFactor-playerView.frame.size.width) < 2.0);
		assert(fabs(playerView.bounds.size.height*scaleFactor-playerView.frame.size.height) < 2.0);
        
		// Compute the new window size and set it. We need to cater for the extra pixels that are part of
        // the content view but not part of us (toolbars and such).
        CGFloat windowWidth = self.frame.size.width + extraWidth;
        CGFloat windowHeight = self.frame.size.height + extraHeight;
		CGSize newWindowSize = CGSizeMake(windowWidth, windowHeight);
        resizingWindow = true;
		[window setContentSize: NSSizeFromCGSize(newWindowSize)];
        resizingWindow = false;
        
		[window makeKeyAndOrderFront: self];

        AM_DBG NSLog(@"recomputeZoom after, self.bounds %f,%f,%f,%f",
            self.bounds.origin.x, self.bounds.origin.y, self.bounds.size.width, self.bounds.size.height);
        AM_DBG NSLog(@"recomputeZoom after,  self.frame %f,%f,%f,%f",
            self.frame.origin.x, self.frame.origin.y, self.frame.size.width, self.frame.size.height);
	} else {
        // We're in full-screen mode (or some other mode where our size is determined by
        // somethings outside our control). We adapt our scaling.
        float x_factor = self.bounds.size.width / playerView.bounds.size.width;
        float y_factor = self.bounds.size.height / playerView.bounds.size.height;
        float max_factor = fmin(x_factor, y_factor);
		if (scaleFactor > max_factor) scaleFactor = max_factor;
        NSRect playerBounds = playerView.bounds;
        NSRect playerFrame = playerView.bounds;
        playerFrame.size.width *= scaleFactor;
        playerFrame.size.height *= scaleFactor;
 		[playerView setFrame: playerFrame];
		[playerView setBounds: playerBounds];
		assert(playerView.bounds.origin.x == playerView.frame.origin.x);
		assert(playerView.bounds.origin.y == playerView.frame.origin.y);
		assert(fabs(playerView.bounds.size.width*scaleFactor-playerView.frame.size.width) < 2.0);
		assert(fabs(playerView.bounds.size.height*scaleFactor-playerView.frame.size.height) < 2.0);
        float x = (self.bounds.size.width - playerView.frame.size.width) / 2;
        float y = (self.bounds.size.height - playerView.frame.size.height) / 2;
        [playerView setFrameOrigin: NSMakePoint(x, y)];
    }
}

- (IBAction) zoomImageToActualSize: (id) sender
{
    // Reset the scale factor to 1
    scaleFactor = 1.0;
       
	[self recomputeZoom];
}

- (IBAction) zoomOut: (id) sender
{
    scaleFactor /= 1.1892;
    if (scaleFactor < 0.1) scaleFactor = 0.1;
       
	[self recomputeZoom];
}

- (IBAction) zoomIn: (id) sender
{
    scaleFactor *= 1.1892;
    if(scaleFactor > 10) scaleFactor = 10;
       
	[self recomputeZoom];
}

- (void) resizeWithOldSuperviewSize:(NSSize)oldSize
{
    AM_DBG NSLog(@"ScalerView.resizeWithOldSuperviewSize: y=%f", self.frame.origin.y);
    AM_DBG NSLog(@"ScalerView.resizeWithOldSuperviewSize: oldsize %f,%f bounds.size %f,%f", oldSize.width, oldSize.height, self.bounds.size.width, self.bounds.size.height);
	if (resizingWindow) {
        // The very first call to this method, during recomputeZoom, should *not* recompute our frame.
        return;
    }
	
	// First thing to do is remember the player view inner coordinates (the SMIL coordinate system),
	// because resizeWithOldSuperviewSize: will muck with it.
	MyAmbulantView *playerView = [self getPlayer];
	NSRect origPlayerBounds = playerView.bounds;
	
	// Now we let Apple do its stuff so our NSView is resized correctlty wrt. our parent view
    [super resizeWithOldSuperviewSize: oldSize];
    AM_DBG NSLog(@"ScalerView.resizeWithOldSuperviewSize: new bounds.size %f,%f", self.bounds.size.width, self.bounds.size.height);
    AM_DBG NSLog(@"ScalerView.resizeWithOldSuperviewSize: new frame.size %f,%f", self.frame.size.width, self.frame.size.height);
	if (playerView == nil) return;
	
	// Compute the aspect-ratio-maintaing scale factor that is needed to make the content as big as possible
	// given our size.
	CGFloat scaleX = self.bounds.size.width / playerView.bounds.size.width;
	CGFloat scaleY = self.bounds.size.height / playerView.bounds.size.height;
	CGFloat scale = fmin(scaleX, scaleY);
    AM_DBG NSLog(@"scale is %f", scale);

	// Set the frame of the player view to the new size, while maintaing the inner coordinate
	// system of the player view.
    CGSize newPlayerFrame = CGSizeMake(playerView.bounds.size.width*scale, playerView.bounds.size.height*scale);
    [playerView setFrameSize: NSSizeFromCGSize(newPlayerFrame)];
	[playerView setBounds: origPlayerBounds];
	
	// Center the player view within our bounds.
	CGFloat extraX = self.bounds.size.width - playerView.frame.size.width;
	CGFloat extraY = self.bounds.size.height - playerView.frame.size.height;
	[playerView setFrameOrigin: NSMakePoint(extraX/2, extraY/2)];
	AM_DBG NSLog(@"ScalerView.resizeWithOldSuperviewSize after, self.bounds %f,%f,%f,%f",
		self.bounds.origin.x, self.bounds.origin.y, self.bounds.size.width, self.bounds.size.height);
	AM_DBG NSLog(@"ScalerView.resizeWithOldSuperviewSize after,  self.frame %f,%f,%f,%f",
		self.frame.origin.x, self.frame.origin.y, self.frame.size.width, self.frame.size.height);
	AM_DBG NSLog(@"ScalerView.resizeWithOldSuperviewSize after, playerview.bounds %f,%f,%f,%f",
		playerView.bounds.origin.x, playerView.bounds.origin.y, playerView.bounds.size.width, playerView.bounds.size.height);
	AM_DBG NSLog(@"ScalerView.resizeWithOldSuperviewSize after,  playerview.frame %f,%f,%f,%f",
		playerView.frame.origin.x, playerView.frame.origin.y, playerView.frame.size.width, playerView.frame.size.height);
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

@end

@implementation MyDocument

- (id)init
{
	self = [super init];
	if (self) {
		// Add your subclass-specific initialization here.
		// If an error occurs here, send a [self release] message and return nil.

	}
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
    if (hud_controls) [hud_controls retain];

	if ([self fileURL] == nil) {
		[self askForURL: self];
	} else {
		[self openTheDocument];
	}
	[self validateButtons: self];
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
	return YES;
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
	AM_DBG NSLog(@"openTheDocument called\n");
	
	// Pause other players (actually also sent to ourselves, but we are not inited yet)
	NSDocumentController *dc = [NSDocumentController sharedDocumentController];
	NSArray *docs = [dc documents];
	[docs makeObjectsPerformSelector:@selector(pause:) withObject: self];
	
	// Create the mainloop (and player, etc) objects
	NSString *url;
	url = [[self fileURL] absoluteString];
	embedder = new document_embedder(self);
	myMainloop = new mainloop([url UTF8String], view, embedder);
	// and play self
	[self autoPlay: self];
}

- (void)showWindows
{
	[super showWindows];
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	if (prefs->m_fullscreen)
		[self goFullScreen: self];

}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
	// Insert code here to write your document from the given data.	 You can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.
	return nil;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
	// Insert code here to read your document from the given data.	You can also choose to override -loadFileWrapperRepresentation:ofType: or -readFromFile:ofType: instead.
	return YES;
}

- (BOOL) validateUIItem:(id)UIItem
{
//	AM_DBG NSLog(@"Validating %@", UIItem);
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
    if (play_button_2) [play_button_2 setEnabled: enabled];
	enabled = [self validateUIItem: stop_button];
	[stop_button setEnabled: enabled];
    if (stop_button_2) [stop_button_2 setEnabled: enabled];
	enabled = [self validateUIItem: pause_button];
	[pause_button setEnabled: enabled];
    if (pause_button_2) [pause_button_2 setEnabled: enabled];
}

- (IBAction)pause:(id)sender
{
	AM_DBG NSLog(@"pause for %@, myMainloop=%p", self, myMainloop);
	if (myMainloop) myMainloop->pause();
	[self validateButtons: nil];
}

- (IBAction)play:(id)sender
{
	if (!myMainloop) return;

	myMainloop->play();
	[self validateButtons: nil];
}

- (IBAction)autoPlay:(id)sender
{
	if (!myMainloop) return;
#ifdef WITH_REMOTE_SYNC
    if (myMainloop->uses_external_sync()) return;
#endif // WITH_REMOTE_SYNC
    [self play: self];
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
#ifdef WITH_OVERLAY_WINDOW
	delete myAuxMainloop;
	myAuxMainloop = NULL;
	if (myAuxWindow) {
		[myAuxWindow release];
		myAuxWindow = NULL;
	}
#endif
	delete myMainloop;
	myMainloop = NULL;
	delete embedder;
	embedder = NULL;
	[self countDoc: -1];
	[super close];
}

- (void)close: (id)dummy
{
	[self close];
}

- (void)fixMouse: (id)dummy
{
	mainloop *ml = myMainloop;
#ifdef WITH_OVERLAY_WINDOW
	if (myAuxMainloop) {
		ml = myAuxMainloop;
	}
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
#ifdef WITH_OVERLAY_WINDOW
	if (myAuxMainloop) {
		ml = myAuxMainloop;
	}
#endif
	if (ml) ml->before_mousemove(0);
    if (saved_window) {
        // If we are in fullscreen mode we want to show the HUD.
        [self showHUD: self];
    }
}

- (void)keyDown: (NSEvent *)ev
{
	mainloop *ml = myMainloop;
#ifdef WITH_OVERLAY_WINDOW
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
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_fullscreen = false;
	prefs->save_preferences();
	[self _goWindowMode];
}

- (void)_goWindowMode
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

    NSView* ourView = view;
    if (scaler_view) ourView = scaler_view;
	// Attach our view to the normal window.
	NSWindow *mScreenWindow = [ourView window];
	NSView *savedcontentview = [saved_window contentView];
	[ourView setFrame: saved_view_rect];
	[savedcontentview addSubview: ourView];
	[savedcontentview setNeedsDisplay:YES];
	[saved_window makeFirstResponder: view];
	[saved_window setAcceptsMouseMovedEvents: YES];

	// Tell our controller that the normal window is in use again.
	NSWindowController* winController = [[self windowControllers] objectAtIndex:0];
	[winController setWindow:saved_window];

	// Get rid of the fullscreen window
	[mScreenWindow close];
	mScreenWindow = nil;
	[saved_window makeKeyAndOrderFront:self];

	// And clear saved_window, which signals we're in normal mode again.
	[saved_window release];
	saved_window = nil;
#ifdef XXXJACK_RESIZE_FULLSCREEN
    if (scaler_view) [scaler_view recomputeZoom];
#endif
}

- (IBAction)goFullScreen:(id)sender
{
#ifdef SAVE_FULLSCREEN_IN_PREFS
	ambulant::common::preferences *prefs = ambulant::common::preferences::get_preferences();
	prefs->m_fullscreen = true;
	prefs->save_preferences();
#endif
	if (saved_window) {
		NSLog(@"goFullScreen: already in fullscreen mode");
		return;
	}
	// Get the screen information.
	NSScreen* screen = [[view window] screen];
	if (screen == NULL) screen = [NSScreen mainScreen];
	NSDictionary* screenInfo = [screen deviceDescription];
	NSNumber* screenID = [screenInfo objectForKey:@"NSScreenNumber"];
	AM_DBG NSLog(@"%p.goFullScreen: view=%@ window=%@ screenID = %@", (void*)self, view, [view window], screenID);

	// Capture the screen.
	CGDirectDisplayID displayID = (CGDirectDisplayID)[screenID longValue];
	CGDisplayErr err = CGDisplayCapture(displayID);
	if (err != CGDisplayNoErr) {
		NSLog(@"goFullScreen: CGDisplayCapture failed");
		return;
	}

	// Create the full-screen window.
	NSRect winRect = [screen frame];
	// The (x, y) coordinates are nonzero for a non-primary screen, it appears that
	// the rect is for the virtual combination of all screens, with (0, 0) rooted
	// at the origin of the main screen.
	winRect.origin.x = 0;
	winRect.origin.y = 0;
	NSWindow *mScreenWindow;
	mScreenWindow = [[FullScreenWindow alloc] initWithContentRect:winRect
			styleMask:NSBorderlessWindowMask
			backing:NSBackingStoreBuffered
			defer:NO
			screen:screen];

	// Establish the window attributes.
	[mScreenWindow setDelegate:id<NSWindowDelegate>(self)];
	[mScreenWindow setBackgroundColor: [NSColor blackColor]];

	// Remember the old window, and move our view to the fullscreen
	// window.
	saved_window = [view window];
	[saved_window retain];

	// Create the outer view on the fullscreen window, and insert the
	// ambulant view within it.
	NSView *fsmainview = [[NSView alloc] initWithFrame: winRect];

	NSView *contentview = view;
	saved_view_rect = [contentview frame];
    if (scaler_view) {
        // For CG-based player
        contentview = scaler_view;
		saved_view_rect = [contentview frame];
        contentview.frame = winRect;
    }
	[fsmainview addSubview: contentview];
	NSRect contentRect = [contentview frame];
	CGFloat xExtra = NSWidth(winRect) - NSWidth(contentRect);
	CGFloat yExtra = NSHeight(winRect) - NSHeight(contentRect);
	NSPoint frameOrigin = NSMakePoint(xExtra/2, yExtra/2);
	[contentview setFrameOrigin: frameOrigin];

	[mScreenWindow setContentView: fsmainview];
	[fsmainview setNeedsDisplay:YES];
	[fsmainview release];
	[mScreenWindow makeFirstResponder: view];
	[mScreenWindow setAcceptsMouseMovedEvents: YES];

	// Make the screen window the current document window.
	// Be sure to retain the previous window if you want to	 use it again.
	NSWindowController* winController = [[self windowControllers] objectAtIndex:0];
	[winController setWindow:mScreenWindow];

	// The window has to be above the level of the shield window.
	int32_t shieldLevel = CGShieldingWindowLevel();
	[mScreenWindow setLevel:shieldLevel];


	// Show the window.
	[mScreenWindow makeKeyAndOrderFront:self];
#ifdef XXXJACK_RESIZE_FULLSCREEN
    if (scaler_view) [scaler_view recomputeZoom];
#endif
}

- (IBAction) toggleFullScreen: (id)sender
{
	if (saved_window) {
        [hud_controls removeFromSuperview];
		[self goWindowMode: sender];
	} else {
		[self goFullScreen: sender];
    }
}

#ifdef WITH_OVERLAY_WINDOW
- (BOOL)openAuxDocument: (NSURL *)auxUrl
{
//	embedder = new document_embedder(self);
	delete myAuxMainloop;
	if (myAuxView) {
		[myAuxView removeFromSuperview];
		// No need to release myAuxView: removeFromSuperView did that for us
		myAuxView = NULL;
	}
	myAuxView = [[MyAmbulantView alloc] initWithFrame: [view bounds]];
	if (myAuxWindow == NULL) {
		// Determine where on the screen the overlay window should be
		NSPoint baseOrigin = NSMakePoint([view frame].origin.x, [view frame].origin.y);
		NSPoint screenOrigin = [[view window] convertBaseToScreen: baseOrigin];

		// Create the window
		// Note that it is NOT a fullscreen window, but that class gives us
		// keyboard events, which normally borderless windows don't get.
		myAuxWindow = [[FullScreenWindow alloc] initWithContentRect:
			NSMakeRect(screenOrigin.x,screenOrigin.y,[view frame].size.width,[view frame].size.height)
			styleMask:NSBorderlessWindowMask
			backing:NSBackingStoreBuffered
			defer:YES];
		[myAuxWindow setDelegate: id<NSWindowDelegate>(self)];
		[myAuxWindow setBackgroundColor: [NSColor clearColor]];
		[myAuxWindow setOpaque:NO];
		[myAuxWindow setIgnoresMouseEvents: NO];
		[myAuxWindow setAcceptsMouseMovedEvents: YES];
		[myAuxWindow setHasShadow:NO];
	}
	// Add the view
	[[myAuxWindow contentView] addSubview: myAuxView];
//	[myAuxWindow setInitialFirstResponder: myAuxView];

	// Connect the aux window to the main window and put it up front
	[[view window] addChildWindow: myAuxWindow ordered: NSWindowAbove];
//	[myAuxWindow makeKeyAndOrderFront: self];
	AM_DBG NSLog(@"openAuxDocument %@", auxUrl);
	AM_DBG NSLog(@"Orig view %p, auxView %p", (void*)view, (void*)myAuxView);
	myAuxMainloop = new mainloop([[auxUrl absoluteString] UTF8String], myAuxView, NULL);
	myAuxMainloop->play();
	[myAuxWindow makeFirstResponder: myAuxView];
	return true;
}

- (void)closeAuxDocument
{
	AM_DBG NSLog(@"closeAuxDocument");
	delete myAuxMainloop;
	myAuxMainloop = NULL;
	if (myAuxView) {
		[myAuxView removeFromSuperview];
		myAuxView = NULL;
		[[view window] makeFirstResponder: view];
	}
	if (myAuxWindow) {
		[myAuxWindow release];
		myAuxWindow = NULL;
	}
}
#endif // WITH_OVERLAY_WINDOW

- (IBAction)showHUD: (id)sender
{
    if (hud_controls == nil) return;
    
    NSView *container = [[view window] contentView];
    if (hud_controls.superview != container) {
        AM_DBG NSLog(@"Showing HUD");
        float x_pos = (container.bounds.size.width- hud_controls.frame.size.width) / 2;
        float y_pos = 20;
        [hud_controls setFrameOrigin: NSMakePoint(x_pos, y_pos)];
        [container addSubview:hud_controls];
    }
    [NSObject cancelPreviousPerformRequestsWithTarget: self selector: @selector(hideHUD:) object: self];
    [self performSelector: @selector(hideHUD:) withObject: self afterDelay: 5.0];
}

- (IBAction)hideHUD: (id)sender
{
    [hud_controls removeFromSuperview];
}

- (int) countDoc: (int) incr
{
	static int n_doc;
	AM_DBG NSLog(@"countDoc: n_doc=%d incr=%d", n_doc, incr);
	int rv = n_doc;
	n_doc += incr;
	return rv;
}
@end
