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
#import "mypreferences.h"
#import <CoreFoundation/CoreFoundation.h>
#include <locale.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// XXXX Should go elsewhere
#include "ambulant/version.h"
#include "ambulant/config/config.h"
#include "ambulant/lib/amstream.h"
#include "ambulant/lib/byte_buffer.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"
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
//	[nsstr autorelease];
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

bool
initialize_logger()
{
	// Connect logger to our message displayer and output processor
	ambulant::lib::logger::get_logger()->set_show_message(show_message);
	ambulant::lib::logger::get_logger()->set_ostream(new nslog_ostream);
	// Tell the logger about the output level preference
	int level = ambulant::common::preferences::get_preferences()->m_log_level;
	ambulant::lib::logger::get_logger()->set_level(level);
	// And tell the UI too
	LogController *log = [LogController sharedLogController];
	if (log) [log setLogLevelUI: level];	
}

@implementation MyAppDelegate
- (BOOL) applicationShouldOpenUntitledFile: (id) sender
{
	return NO;
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Install our preferences handler
	mypreferences::install_singleton();
	// Install our logger
	if (initialize_logger() ) {
		// Show the logger window immedeately if log level is DEBUG
		[self showLogWindow: self];
	}
	// Initialize the gettext library
	CFLocaleRef userLocaleRef = CFLocaleCopyCurrent();
	NSString *userLocaleName = (NSString *)CFLocaleGetIdentifier(userLocaleRef);
	const char *locale = [userLocaleName cString];
	const char *unix_locale = getenv("LANG");
	if (unix_locale == NULL || *unix_locale == '\0') {
		setlocale(LC_MESSAGES, locale);
		setenv("LANG", locale, 1);
	} else if (strcmp(locale, unix_locale) != 0) {
		ambulant::lib::logger::get_logger()->warn("MacOS System Preferences locale: \"%s\", Unix locale: LANG=\"%s\"", locale, unix_locale);
	}
#if ENABLE_NLS
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Macintosh/Cocoa"), __DATE__);
#if ENABLE_NLS
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english; user requested %s)"), locale);
#endif

	// Initialize the default system test settings
	NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
	NSString *systemTestSettingsPath = [thisBundle pathForResource:@"systemTestSettings" ofType:@"xml"];
	if (systemTestSettingsPath) {
		std::string path([systemTestSettingsPath cString]);
		mainloop::set_preferences(path);
	}
	// Test whether we want to run the welcome document (on first run only)
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
		ambulant::lib::logger::get_logger()->error(gettext("No Welcome.smil in application bundle"));
	}
	// Ask for notification when preferences change.
#if 0
	// This doesn't work;-(
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self
		selector:@selector(preferencesChanged:)
		name:NSUserDefaultsDidChangeNotification
		object:nil];
	// And these don't work either:-(
	[defaults addObserver:self forKeyPath:@"observingKeyPath" options:NSKeyValueObservingOptionNew context:nil];
	[[NSUserDefaultsController sharedUserDefaultsController] addObserver:self forKeyPath:@"values.log_level" options:NSKeyValueObservingOptionNew context:nil];
	[[NSUserDefaultsController sharedUserDefaultsController] addObserver:self forKeyPath:@"log_level" options:NSKeyValueObservingOptionNew context:nil];
#endif
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
			ambulant::lib::logger::get_logger()->error(gettext("Welcome.smil could not be opened"));
		}
	} else {
		ambulant::lib::logger::get_logger()->error(gettext("No Welcome.smil in application bundle"));
	}
}

- (IBAction)showHomepage:(id)sender
{
	AM_DBG NSLog(@"Show Homepage");
	CFURLRef url = CFURLCreateWithString(NULL, (CFStringRef)@"http://www.ambulantplayer.org", NULL);
	OSErr status;
	
	if ((status=LSOpenCFURLRef(url, NULL)) != 0) {
		ambulant::lib::logger::get_logger()->trace("Opening http://www.ambulantplayer.org: LSOpenCFURLRef error %d",  status);
		ambulant::lib::logger::get_logger()->error(gettext("Cannot open http://www.ambulantplayer.org"));
	}
}

- (IBAction)showLogWindow:(id)sender
{
	AM_DBG NSLog(@"Show Log Window");
	LogController *log = [LogController sharedLogController];
	if (log) [log showWindow: sender];
}

- (IBAction)applyPreferences:(id)sender
{
	NSLog(@"Apply Preferences Window");
	ambulant::common::preferences::get_preferences()->load_preferences();
	// Set log level
	int level = ambulant::common::preferences::get_preferences()->m_log_level;
	ambulant::lib::logger::get_logger()->trace("Log level set to %s",
		ambulant::lib::logger::get_level_name(level));
	ambulant::lib::logger::get_logger()->set_level(level);
	// And reflect in UI
	LogController *log = [LogController sharedLogController];
	if (log) [log setLogLevelUI: level];	
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

- (void)setLogLevel: (int)level
{
	ambulant::common::preferences::get_preferences()->m_log_level = level;
	ambulant::common::preferences::get_preferences()->save_preferences();
	ambulant::lib::logger::get_logger()->set_level(0);
	ambulant::lib::logger::get_logger()->trace("Log level set to %s",
		ambulant::lib::logger::get_level_name(level));
	ambulant::lib::logger::get_logger()->set_level(level);
}
@end

