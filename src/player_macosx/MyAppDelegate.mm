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

#import "MyAppDelegate.h"
#import "MyDocument.h"
#import "MyAppDelegate.h"
#import "LogController.h"
#import "mypreferences.h"
#import <CoreFoundation/CoreFoundation.h>
#include <locale.h>
#include <crt_externs.h>

//#define	AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// XXXX Should go elsewhere
#include "ambulant/version.h"
#include "ambulant/config/config.h"
#include "ambulant/lib/amstream.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/net/url.h"
#include "ambulant/lib/profile.h"
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif
#include "ambulant/smil2/test_attrs.h"
#include <stdarg.h>

#ifndef NSINTEGER_DEFINED
typedef int NSInteger;
typedef unsigned int NSUInteger;
#endif

class nslog_ostream : public ambulant::lib::ostream {
	bool is_open() const {return true;}
	void close() {}
	int write(const unsigned char *buffer, int nbytes) {NSLog(@"ostream use of buffer, size not implemented for Cocoa"); return 0;}
	int write(const char *cstr);
	void flush() {}
};

int
nslog_ostream::write(const char *cstr)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	LogController *log = [LogController sharedLogController];
	NSString *nsstr = [NSString stringWithUTF8String: cstr];
	if (log) [log performSelectorOnMainThread: @selector(insertText:) withObject: nsstr waitUntilDone: NO];
	[pool release];
	return 0;
}

static void
show_message(int level, const char *format)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *message = [[NSString stringWithUTF8String: format] retain];
	MyAppDelegate *delegate = (MyAppDelegate*)[[NSApplication sharedApplication] delegate];
	[delegate performSelectorOnMainThread: @selector(showMessage:)
		withObject: message waitUntilDone: NO];
//	[message release];
	[pool release];
}

static bool
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
    AM_DBG NSLog(@"applicationShouldOpenUntitledFile called\n");

#ifdef WITH_SPLASH_SCREEN
	// For Ta2 (and other embedded apps) we always show the splash screen
	// (which takes the place of the welcome document)
	[self playWelcome: self];
#else
	// Test whether we want to run the welcome document (on first run only)
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	if ( ![defaults boolForKey: @"welcomeDocumentSeen"] ) {
		[self playWelcome: self];
		[defaults setBool: YES forKey: @"welcomeDocumentSeen"];
	}
#endif

	return NO;
}

- (NSString *)typeForContentsOfURL:(NSURL *)inAbsoluteURL error:(NSError **)outError
{
    AM_DBG NSLog(@"typeForContentsOfURL called\n");
    outError = nil;
    return [NSString stringWithUTF8String: "SMIL document"];
}

- (void) applicationWillFinishLaunching:(NSNotification *)aNotification
{
	AM_DBG NSLog(@"applicationWillFinishLaunching called\n");
	// First get our bundle, various initializations need it.
	NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];

	// Install our preferences handler
	mypreferences::install_singleton();
    
	// Install our logger
	if (initialize_logger() == 0 && getenv("AMBULANT_LOGGER_NOWINDOW") == NULL) {
		// Show the logger window immedeately if log level is DEBUG
		[self showLogWindow: self];
	}
	// Initialize profiler (if enabled)
	ambulant::lib::profile::initialize();

	// Initialize the gettext library. We support both the MacOS System Preferences
	// setting and the unix-style LANG variable.
	// In addition we support an LC_MESSAGES file either in the bundle or in the
	// standard system location.
	CFLocaleRef userLocaleRef = CFLocaleCopyCurrent();
	NSString *userLocaleName = (NSString *)CFLocaleGetIdentifier(userLocaleRef);
	const char *locale = [userLocaleName UTF8String];
	const char *unix_locale = getenv("LANG");
	if (unix_locale == NULL || *unix_locale == '\0') {
		setlocale(LC_MESSAGES, locale);
		setenv("LANG", locale, 1);
	} else if (strcmp(locale, unix_locale) != 0) {
		ambulant::lib::logger::get_logger()->trace("MacOS System Preferences locale: \"%s\", Unix locale: LANG=\"%s\"", locale, unix_locale);
	}
#if ENABLE_NLS
	NSString *resourcePath = [thisBundle resourcePath];
	char bundleLocaleDir[1024];
	snprintf(bundleLocaleDir, sizeof(bundleLocaleDir), "%s/locale", [resourcePath UTF8String]);
	if (access(bundleLocaleDir, 0) >= 0) {
		bindtextdomain (PACKAGE, bundleLocaleDir);
	} else {
		bindtextdomain (PACKAGE, LOCALEDIR);
	}
	textdomain (PACKAGE);
#endif
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: compile time version %s, runtime version %s"), AMBULANT_VERSION, ambulant::get_version());
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: built on %s for Macintosh/%s/%s"), __DATE__,
		"CoreGraphics",
#ifdef __ppc64__
		"ppc64"
#elif defined(__ppc__)
		"ppc"
#elif defined(__x86_64__)
		"x86_64"
#elif defined(__i386__)
		"i386"
#elif defined(__armv6__)
		"armv6"
#else
		"unknown-architecture"
#endif
	);
#if ENABLE_NLS
	ambulant::lib::logger::get_logger()->debug(gettext("Ambulant Player: localization enabled (english; user requested %s)"), locale);
#endif
	// Initialize ordered list of language preferences
	ambulant::smil2::test_attrs::clear_languages();
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
	// Should also work in earlier systems
	const NSArray *langNames = (const NSArray *)CFPreferencesCopyAppValue(CFSTR("AppleLanguages"), kCFPreferencesCurrentApplication);
#else
	const NSArray *langNames = (const NSArray *)CFLocaleCopyPreferredLanguages();
#endif
	NSUInteger nLangs = [langNames count];
	double factor = 1.0 / nLangs;
	unsigned int i;
	for (i=0; i < [langNames count]; i++) {
		NSString *langName = [langNames objectAtIndex: i];
		AM_DBG NSLog(@"Language %d: %@", i, langName);
		std::string cLang([langName UTF8String]);
		ambulant::smil2::test_attrs::add_language(cLang, float(1.0-(factor*i)));
	}
	[langNames release];

	// Initialize the plugins, so we can parser the system test settings file
	{
		ambulant::common::factories *fact = new ambulant::common::factories();
		fact->set_parser_factory(ambulant::lib::global_parser_factory::get_parser_factory());
#ifdef WITH_XERCES_BUILTIN
		fact->get_parser_factory()->add_factory(new ambulant::lib::xerces_factory());
#endif
		ambulant::common::plugin_engine *pe = ambulant::common::plugin_engine::get_plugin_engine();
		pe->add_plugins(fact, NULL);
	}

	// Initialize the default system test settings
	NSString *systemTestSettingsPath = [thisBundle pathForResource:@"systemTestSettings" ofType:@"xml"];
	if (systemTestSettingsPath) {
		std::string path([systemTestSettingsPath UTF8String]);
		mainloop::load_test_attrs(path);
		// And initialize the location where we find other datafiles (bit of a hack)
		NSString *nsresourcedir = [systemTestSettingsPath stringByDeletingLastPathComponent];
		std::string resourcedir([nsresourcedir UTF8String]);
		ambulant::net::url::set_datafile_directory(resourcedir);
	}
	// Install our "open URL" handler.
	NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];
	// There seem to be two similar events gurl/gurl (official one) and GURL/GURL (used by "open" command line tool).
	[appleEventManager setEventHandler:self andSelector:@selector(handleGetURLEvent:withReplyEvent:)
		forEventClass:kAEInternetSuite
		andEventID:kAEISGetURL];
	[appleEventManager setEventHandler:self andSelector:@selector(handleGetURLEvent:withReplyEvent:)
		forEventClass:'GURL'
		andEventID:'GURL'];
}

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
	AM_DBG NSLog(@"applicationDidFinishLaunching called\n");
}

- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification
{
	AM_DBG NSLog(@"applicationDidChangeScreenParameters");
}

- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)replyEvent
{
	AM_DBG NSLog(@"GetURLEvent called\n");
	NSAppleEventDescriptor *obj = [event paramDescriptorForKeyword: keyDirectObject];
	if (!obj) {
		NSLog(@"handleGetURLEvent: No direct object!\n");
		return;
	}
	NSString *urlstr = [obj stringValue];
    if ([urlstr hasPrefix: @"ambulant://"]) {
        // Kees' format: replace http by ambulant
        urlstr = [NSString stringWithFormat: @"http:%@", [urlstr substringFromIndex: 9]]; // Length of "ambulant:"
    }
    if ([urlstr hasPrefix: @"ambulant:"]) {
        // Jack's format: prepend ambulant:
        urlstr = [urlstr substringFromIndex: 9]; // Length of "ambulant:"
    }
	AM_DBG NSLog(@"handleGetURLEvent: %@\n", urlstr);
	NSURL *url = [NSURL URLWithString:urlstr];
	NSDocumentController *controller = [NSDocumentController sharedDocumentController];
	NSError *error;
	MyDocument *doc = [controller openDocumentWithContentsOfURL:url display:YES error:&error];
	if (doc) {
        [doc autoPlay: self];
    } else {
		NSLog(@"handleGetURLEvent: error: %@\n", error);
    }
}

- (IBAction)loadFilter:(id)sender
{
	NSOpenPanel *panel = [NSOpenPanel openPanel];
#if 0
	NSInteger result = [panel runModalForDirectory: nil file: nil types: nil];
	if (result != NSOKButton) return;
	NSString *filename = [[panel filenames] objectAtIndex: 0];
	std::string path([filename UTF8String]);
	mainloop::load_test_attrs(path);
#else
    NSInteger result = [panel runModal];
    if (result != NSOKButton) return;
    NSURL *nsurl = [[panel URLs] objectAtIndex: 0];
    std::string path([[nsurl path] UTF8String]);
    mainloop::load_test_attrs(path);
#endif
}

- (IBAction)playWelcome:(id)sender
{
	AM_DBG NSLog(@"Play Welcome");
	NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
#ifdef WITH_SPLASH_SCREEN
	NSString *welcomePath = [thisBundle pathForResource:@ WITH_SPLASH_SCREEN ofType:nil];
#else
	NSString *welcomePath = [thisBundle pathForResource:@"Welcome" ofType:@"smil"];
#endif // WITH_SPLASH_SCREEN
	if (welcomePath) {
		NSDocumentController *controller = [NSDocumentController sharedDocumentController];
		NSError *err;
		MyDocument *welcomeDoc = [controller
			openDocumentWithContentsOfURL: [NSURL fileURLWithPath: welcomePath]
			display: YES
			error: &err];
		if (welcomeDoc) {
			[welcomeDoc play: self];
		} else {
			ambulant::lib::logger::get_logger()->error("%s: %s", gettext("Welcome.smil could not be opened"), [[err localizedDescription] UTF8String]);
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
	AM_DBG NSLog(@"Apply Preferences Window");
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

