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
#include "ambulant/common/plugin_engine.h"
#include "ambulant/net/url.h"
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif

#include <stdarg.h>

class nslog_ostream : public ambulant::lib::ostream {
	bool is_open() const {return true;}
	void close() {}
	int write(const unsigned char *buffer, int nbytes) {NSLog(@"ostream use of buffer, size not implemented for Cocoa"); return 0;}
	int write(const char *cstr);
	void write(ambulant::lib::byte_buffer& bb) {NSLog(@"ostream use of byte_buffer not implemented for Cocoa");}
	void flush() {}
};

int
nslog_ostream::write(const char *cstr)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	LogController *log = [LogController sharedLogController];
	NSString *nsstr = [NSString stringWithCString: cstr];
	if (log) [log insertText: nsstr];
//	[nsstr release];
	[pool release];
	return 0;
}

void
show_message(int level, const char *format)
{
	NSString *message = [[NSString stringWithCString: format] retain];
	MyAppDelegate *delegate = [[NSApplication sharedApplication] delegate];
	[delegate performSelectorOnMainThread: @selector(showMessage:) 
		withObject: message waitUntilDone: YES];
//	[message release];
}

bool
initialize_logger()
{
	// Connect logger to our message displayer and output processor
	ambulant::lib::logger::get_logger()->set_show_message(show_message);
	if (getenv("AMBULANT_LOGGER_NOWINDOW") == NULL)
    	ambulant::lib::logger::get_logger()->set_ostream(new nslog_ostream);
	// Tell the logger about the output level preference
	int level = ambulant::common::preferences::get_preferences()->m_log_level;
	ambulant::lib::logger::get_logger()->set_level(level);
	// And tell the UI too
	LogController *log = [LogController sharedLogController];
	if (log) [log setLogLevelUI: level];
	return level;	
}

@implementation MyAppDelegate
- (BOOL) applicationShouldOpenUntitledFile: (id) sender
{
	return NO;
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// First get our bundle, various initializations need it.
	NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
	
	// Install our preferences handler
	mypreferences::install_singleton();
	
	// Install our logger
	if (initialize_logger() == 0 && getenv("AMBULANT_LOGGER_NOWINDOW") == NULL) {
		// Show the logger window immedeately if log level is DEBUG
		[self showLogWindow: self];
	}
	
	// Initialize the gettext library. We support both the MacOS System Preferences
	// setting and the unix-style LANG variable.
	// In addition we support an LC_MESSAGES file either in the bundle or in the
	// standard system location.
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
	NSString *resourcePath = [thisBundle resourcePath];
	char bundleLocaleDir[1024];
	snprintf(bundleLocaleDir, sizeof(bundleLocaleDir), "%s/locale", [resourcePath cString]);
	if (access(bundleLocaleDir, 0) >= 0) {
		bindtextdomain (PACKAGE, bundleLocaleDir);
	} else {
		bindtextdomain (PACKAGE, LOCALEDIR);
	}
    textdomain (PACKAGE);
#endif
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Macintosh/Cocoa"), __DATE__);
#if ENABLE_NLS
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english; user requested %s)"), locale);
#endif

#if 0
	// Initialize the plugins, so we can parser the system test settings file
	{
		ambulant::common::factories fact;
		fact.pf = ambulant::lib::global_parser_factory::get_parser_factory();
#ifdef WITH_XERCES_BUILTIN
		fact.pf->add_factory(new ambulant::lib::xerces_factory());
#endif
		ambulant::common::plugin_engine *pe = ambulant::common::plugin_engine::get_plugin_engine();
		pe->add_plugins(&fact);
	}
#endif

	// Initialize the default system test settings
	NSString *systemTestSettingsPath = [thisBundle pathForResource:@"systemTestSettings" ofType:@"xml"];
	if (systemTestSettingsPath) {
		std::string path([systemTestSettingsPath cString]);
		mainloop::set_preferences(path);
		// And initialize the location where we find other datafiles (bit of a hack)
		NSString *nsresourcedir = [systemTestSettingsPath stringByDeletingLastPathComponent];
		std::string resourcedir([nsresourcedir cString]);
		ambulant::net::url::set_datafile_directory(resourcedir);
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
//	[message release];
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

