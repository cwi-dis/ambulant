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

	if ([self fileName] == nil) {
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
		[self setFileName: [url_field stringValue]];
		[self openTheDocument];
	} else {
		AM_DBG NSLog(@"ask_for_url: User said cancel");
		[self close];
	}
}

- (void)openTheDocument
{
    ambulant::gui::cocoa::cocoa_window_factory *myWindowFactory;
    myWindowFactory = new ambulant::gui::cocoa::cocoa_window_factory((void *)view);
    NSString *filename = [self fileName];
	bool use_mms = ([[filename pathExtension] compare: @".mms"] == 0);
	embedder = new document_embedder(self);
	myMainloop = new mainloop([filename UTF8String], myWindowFactory, use_mms, embedder);
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
	if (!myMainloop) return NO;
	SEL theAction = [UIItem action];
	AM_DBG NSLog(@"Validating %@, is_running=%d, get_speed=%f", UIItem, myMainloop->is_running(),myMainloop->get_speed());
	if (theAction == @selector(play:)) {
		if (myMainloop->is_running() && myMainloop->get_speed() == 1.0) {
			AM_DBG NSLog(@"play - On");
			[UIItem setState: NSOnState];
		} else {
			AM_DBG NSLog(@"play - Off");
			[UIItem setState: NSOffState];
		}
		//AM_DBG NSLog(@"play - enabled=%d", (!myMainloop->is_running() || myMainloop->get_speed() == 0.0));
		//return !myMainloop->is_running() || myMainloop->get_speed() == 0.0;
		return YES;
	} else if (theAction == @selector(stop:)) {
		if (!myMainloop->is_running()) {
			AM_DBG NSLog(@"stop - On");
			[UIItem setState: NSOnState];
		} else {
			AM_DBG NSLog(@"stop - On");
			[UIItem setState: NSOffState];
		}
		AM_DBG NSLog(@"play - enabled=%d", (myMainloop->is_running()));
		return myMainloop->is_running();
	} else if (theAction == @selector(pause:)) {
		if (myMainloop->is_running() && myMainloop->get_speed() == 0.0) {
			AM_DBG NSLog(@"pause - On");
			[UIItem setState: NSOnState];
		} else {
			AM_DBG NSLog(@"pause - On");
			[UIItem setState: NSOffState];
		}
		AM_DBG NSLog(@"play - enabled=%d", (myMainloop->is_running()));
		return myMainloop->is_running();
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
    if (myMainloop) myMainloop->set_speed(1.0 - myMainloop->get_speed());
	[self validateButtons: nil];
}

- (IBAction)play:(id)sender
{
	if (!myMainloop) return;
	if (myMainloop->is_running())
		myMainloop->set_speed(1.0);
	else {
		// Removed: see comment in [startPlay:]
		// myMainloop->add_ref();
		[NSThread detachNewThreadSelector: @selector(startPlay:) toTarget: self withObject: NULL];
	}
	[self validateButtons: nil];
}

- (void)startPlay: (id)dummy
{
	if (!myMainloop) return;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    if (![NSThread isMultiThreaded]) {
        NSLog(@"startPlay: still not multi-threaded!");
    }
    myMainloop->play();
	// We don't use refcounting on myMainloop, because
	// otherwise our player infrastructure will be destructed in this
	// thread, and at that time the window (and the ambulantWidget) is
	// gone. So the main thread does the cleanup and zaps myMainloop.
	while (myMainloop && myMainloop->is_running()) {
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
	if (myMainloop) myMainloop->release();
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
@end
