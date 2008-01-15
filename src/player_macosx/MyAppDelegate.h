/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* MyAppDelegate */

#import <Cocoa/Cocoa.h>

@interface MyAppDelegate : NSObject
{
}
- (BOOL) applicationShouldOpenUntitledFile: (id) sender;
- (void) applicationWillFinishLaunching:(NSNotification *)aNotification;
- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)replyEvent;
- (IBAction)loadFilter:(id)sender;
- (IBAction)playWelcome:(id)sender;
- (IBAction)showHomepage:(id)sender;
- (IBAction)showLogWindow:(id)sender;
- (void)applyPreferences:(id)sender;
- (void)showMessage:(NSString *)message;
- (void)setLogLevel: (int)level;
@end
