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

#import "LogController.h"
#import "MyAppDelegate.h"

@implementation LogController

+ (id)sharedLogController {
	static LogController *s_LogController = nil;

	if (!s_LogController) {
		s_LogController = [[LogController allocWithZone:[self zone]] init];
	}
	return s_LogController;
}

- (id)init {
	self = [self initWithWindowNibName:@"Log"];
	if (self) {
		[self setWindowFrameAutosaveName:@"Log"];
//		  needsUpdate = NO;
	}
	return self;
}

- (void) awakeFromNib
{
	if (level_popup) [level_popup selectItemAtIndex: level];
}

- (void) insertText: (NSString *)data
{
	NSRange endRange;

	endRange.location = [[text textStorage] length];
	endRange.length = 0;
	[text replaceCharactersInRange:endRange withString:data];
	endRange.length = [data length];
	[text scrollRangeToVisible:endRange];
}

- (void) setLogLevel: (id) sender
{
	int lvl = (int)[sender indexOfSelectedItem];
	MyAppDelegate *app = (MyAppDelegate*) [[NSApplication sharedApplication] delegate];
	[app setLogLevel: lvl];
}

- (void) setLogLevelUI: (int) newlevel
{
	level = newlevel;
	if (level_popup) [level_popup selectItemAtIndex: level];
}

@end
