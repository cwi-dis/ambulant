//
//  MyDocument.m
//  cocoambulant
//
//  Created by Jack Jansen on Thu Sep 04 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "MyDocument.h"
#import "MyAmbulantView.h"

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
    myWindowFactory = new ambulant::gui::cocoa::cocoa_window_factory((void *)view);
    myMainloop = new mainloop();
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

- (BOOL) validateMenuItem:(id)menuItem
{
	SEL theAction = [menuItem action];
	if (theAction == @selector(play:)) {
		if (myMainloop->is_running() && myMainloop->get_speed() == 1.0)
			[menuItem setState: NSOnState];
		else
			[menuItem setState: NSOffState];
		return !myMainloop->is_running() || myMainloop->get_speed() == 0.0;
	} else if (theAction == @selector(stop:)) {
		if (!myMainloop->is_running())
			[menuItem setState: NSOnState];
		else
			[menuItem setState: NSOffState];
		return myMainloop->is_running();
	} else if (theAction == @selector(pause:)) {
		if (myMainloop->is_running() && myMainloop->get_speed() == 0.0)
			[menuItem setState: NSOnState];
		else
			[menuItem setState: NSOffState];
		return myMainloop->is_running();
	}
	return NO;
}

- (IBAction)pause:(id)sender
{
    myMainloop->set_speed(1.0 - myMainloop->get_speed());
}

- (IBAction)play:(id)sender
{
	if (myMainloop->is_running())
		myMainloop->set_speed(1.0);
	else
		[NSThread detachNewThreadSelector: @selector(startPlay:) toTarget: self withObject: NULL];
}

- (void)startPlay: (id)dummy
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *filename = [self fileName];
    
    if (![NSThread isMultiThreaded]) {
        NSLog(@"startPlay: still not multi-threaded!");
    }
    myMainloop->run([filename UTF8String], myWindowFactory);
    [pool release];
}

- (IBAction)stop:(id)sender
{
    NSLog(@"Stop");
}

- (void *)view
{
    return view;
}
@end
