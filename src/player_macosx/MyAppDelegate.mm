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

#import "MyAppDelegate.h"
#import "MyDocument.h"
#import "LogController.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// XXXX Should go elsewhere
#include "ambulant/lib/amstream.h"
#include "ambulant/lib/byte_buffer.h"
#include "ambulant/lib/logger.h"
#include <stdarg.h>

class nslog_ostream : public ambulant::lib::ostream {
	bool is_open() const {return true;}
	void close() {}
	int write(const unsigned char *buffer, int nbytes) {NSLog(@"ostream use of buffer, size not implemented for Cocoa");}
	int write(const char *cstr);
	void write(ambulant::lib::byte_buffer& bb) {NSLog(@"ostream use of byte_buffer not implemented for Cocoa");}
	void flush() {}
};

int
nslog_ostream::write(const char *cstr)
{
	LogController *log = [LogController sharedLogController];
	NSString *nsstr = [NSString stringWithCString: cstr];
	[nsstr autorelease];
	if (log) [log insertText: nsstr];
}

void
show_message(const char *format, va_list args)
{
	NSString *message = [NSString stringWithCString: format];
	MyAppDelegate *delegate = [[NSApplication sharedApplication] delegate];
	[delegate performSelectorOnMainThread: @selector(showMessage:) 
		withObject: message waitUntilDone: YES];
}

void
initialize_logger()
{
	ambulant::lib::logger::get_logger()->set_show_message(show_message);
	ambulant::lib::logger::get_logger()->set_ostream(new nslog_ostream);
}

@implementation MyAppDelegate
- (BOOL) applicationShouldOpenUntitledFile: (id) sender
{
	return NO;
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
	initialize_logger();
	NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
	NSString *systemTestSettingsPath = [thisBundle pathForResource:@"systemTestSettings" ofType:@"xml"];
	if (systemTestSettingsPath) {
		std::string path([systemTestSettingsPath cString]);
		mainloop::set_preferences(path);
	}

	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	if ( [defaults boolForKey: @"welcomeDocumentSeen"] ) return;
	NSString *welcomePath = [thisBundle pathForResource:@"Welcome" ofType:@"smil"];
	if (welcomePath) {
		id sender = [aNotification object];
		AM_DBG NSLog(@"Will play %@", welcomePath);
		AM_DBG NSLog(@"Application is %@", sender);
		NSDocumentController *controller = [NSDocumentController sharedDocumentController];
		AM_DBG NSLog(@"DocumentController is %@", controller);
		MyDocument *welcomeDoc = [controller openDocumentWithContentsOfFile: welcomePath display: YES];
		if (welcomeDoc) {
			[welcomeDoc play: sender];
			[defaults setBool: YES forKey: @"welcomeDocumentSeen"];
		}
	} else {
		NSLog(@"No Welcome.smil in bundle");
	}
}

- (IBAction)loadFilter:(id)sender
{
	NSOpenPanel *panel = [NSOpenPanel openPanel];
	int result = [panel runModalForDirectory: nil file: nil types: nil];
	if (result != NSOKButton) return;
	NSString *filename = [[panel filenames] objectAtIndex: 0];
	std::string path([filename cString]);
	mainloop::set_preferences(path);
}

- (IBAction)playWelcome:(id)sender
{
	AM_DBG NSLog(@"Play Welcome");
	NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
	NSString *welcomePath = [thisBundle pathForResource:@"Welcome" ofType:@"smil"];
	if (welcomePath) {
		NSDocumentController *controller = [NSDocumentController sharedDocumentController];
		MyDocument *welcomeDoc = [controller openDocumentWithContentsOfFile: welcomePath display: YES];
		if (welcomeDoc) {
			[welcomeDoc play: self];
		} else {
			NSLog(@"Could not open document Welcome.smil");
		}
	} else {
		NSLog(@"No Welcome.smil in bundle");
	}
}

- (IBAction)showHomepage:(id)sender
{
	NSLog(@"Show Homepage");
	CFURLRef url = CFURLCreateWithString(NULL, (CFStringRef)@"http://www.ambulantplayer.org", NULL);
	OSErr status;
	
	if ((status=LSOpenCFURLRef(url, NULL)) != 0) {
		ambulant::lib::logger::get_logger()->error("Cannot open http://www.ambulantplayer.org: LSOpenCFURLRef error %d",  status);
	}
}

- (IBAction)showLogWindow:(id)sender
{
	NSLog(@"Show Log Window");
	LogController *log = [LogController sharedLogController];
	if (log) [log showWindow: sender];
}

- (IBAction)openURL:(id)sender
{
	NSLog(@"open URL");
	CFURLRef url = CFURLCreateWithString(NULL, (CFStringRef)@"http://www.ambulantplayer.org", NULL);
	OSErr status;
	
	if ((status=LSOpenCFURLRef(url, NULL)) != 0) {
		ambulant::lib::logger::get_logger()->error("Cannot open http://www.ambulantplayer.org: LSOpenCFURLRef error %d",  status);
	}
}

- (void)showMessage:(NSString *)message
{
	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"OK"];
	[alert addButtonWithTitle:@"Show Log Window"];
	[alert setMessageText:message];
	[alert setInformativeText:@"The Log window may have more detailed information"];
	[alert setAlertStyle:NSWarningAlertStyle];
	if ([alert runModal] == NSAlertSecondButtonReturn) {
		[self showLogWindow: self];
	}
	[alert release];
	[message release];
}

@end

