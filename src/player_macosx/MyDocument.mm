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

#ifndef AM_DBG
#define AM_DBG if(0)
#endif


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
	(void)doc; // Suppress warning
	[pool release];
	// [doc retain] ??
	
}


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
    NSString *url = [[self fileURL] path];
	if ( [[self fileURL] isFileURL] ) {
		NSString *escapedurl = (NSString *)CFURLCreateStringByAddingPercentEscapes(NULL,
			(CFStringRef)url, NULL, NULL, kCFStringEncodingUTF8);
		//[url release];
		url = escapedurl;
	}
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
	if (myMainloop) delete myMainloop;
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
	if (!myMainloop) return;
	int cursor = myMainloop->get_cursor();
	AM_DBG NSLog(@"Fixing mouse to %d", cursor);
	if (cursor == 0) {
		if ([NSCursor currentCursor] != [NSCursor arrowCursor])
			[[NSCursor arrowCursor] set];
	} else if (cursor == 1) {
		if ([NSCursor currentCursor] != [NSCursor pointingHandCursor])
			[[NSCursor pointingHandCursor] set];
	} else {
		NSLog(@"Warning: unknown cursor index %d", cursor);
	}
}

- (void)resetMouse: (id)dummy
{
	if (myMainloop) myMainloop->set_cursor(0);
}

- (void)keyDown: (NSEvent *)ev
{
	NSString *chars = [ev characters];
	
	if (chars && [chars length] == 1 && myMainloop) {
		myMainloop->on_char([chars characterAtIndex:0]);
	} else {
		/*AM_DBG*/ NSLog(@"MyDocument::keyDown: dropping %@", chars);
	}
}
		
@end
