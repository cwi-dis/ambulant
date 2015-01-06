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

#import "MyAmbulantView.h"

void
set_statusline(void *view, const char *msg)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *message = [[NSString stringWithCString: msg encoding: NSUTF8StringEncoding] retain];
	[(id)view performSelectorOnMainThread: @selector(setStatusLine:)
		withObject: message waitUntilDone: NO];
	[pool release];
}


@implementation MyAmbulantView

- (void) keyDown: (NSEvent *)theEvent
{
	if (document) {
		[document keyDown: theEvent];
	} else {
		NSLog(@"MyAmbulantView: keyDown: %@", theEvent);
	}
}

- (IBAction) toggleFullScreen:(id)sender
{
	if (document) {
		[document toggleFullScreen: sender];
	}
}


- (void) resetMouse: (id)sender
{
	if (document) {
		[document resetMouse: sender];
    }
}

- (void) fixMouse: (id)sender
{
	if (document) {
		[document fixMouse: sender];
    }
}

- (void) setStatusLine: (NSString *)message
{
	if (document) [document setStatusLine: message];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}
@end
