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

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif//AM_DBG
#import "AmbulantAppDelegate.h"
#import "AmbulantViewController.h"
#import "AmbulantWebViewController.h"
#import <CoreFoundation/CoreFoundation.h>

#include "ambulant/lib/logger.h"
#include "iOSpreferences.h"
#include <fstream>

#pragma mark -
#pragma mark Logger and messages

#ifndef NDEBUG
#define WITH_CONSOLE_LOGGING
#endif
// Define WITH_CONSOLE_LOGGING to have Ambulant logger output go to the iOS system console.
// This allows it to be read in the debugger. But: it will also be saved on the device,
// so it should not be enabled for production builds.
#ifdef WITH_CONSOLE_LOGGING
#include <syslog.h>
class nslog_ostream : public ambulant::lib::ostream {
	std::string m_curstring;
  public:
	nslog_ostream() {
		openlog("iAmbulant.app", LOG_CONS, 0);
	}
	~nslog_ostream() {
		closelog();
	}
	bool is_open() const {return true;}
	void close() {}
	int write(const unsigned char *buffer, int nbytes) {NSLog(@"ostream use of buffer, size not implemented for Cocoa"); return 0;}
	int write(const char *cstr) {
		m_curstring += std::string(cstr);
		if (cstr && *cstr && cstr[strlen(cstr)-1] == '\n') {
			NSLog(@"%s", m_curstring.c_str());
			syslog(LOG_INFO, "%s", m_curstring.c_str());
			m_curstring = "";
		}
		return 0;
	}
	void flush() {}
};
#endif // WITH_CONSOLE_LOGGING

static void
show_message(int level, const char *format)
{
	static NSString* old_message = NULL;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *message = [[NSString stringWithUTF8String: format] retain];
	id appDelegate = [[UIApplication sharedApplication] delegate];
	
	// do not repeat the same error succesively
	if (old_message == NULL || ! [old_message isEqualToString:message]) {
		[appDelegate performSelectorOnMainThread: @selector(showAlert:)
			withObject: message
			waitUntilDone: NO];
		if (old_message != NULL) {
			[old_message release];
		}
		old_message = message;
	} else {
		// the message was not given to 'showAlert:'
		[message release];
	}
	[pool release];
}

static bool
initialize_logger()
{
	// Connect logger to our message displayer and output processor
	ambulant::lib::logger::get_logger()->set_show_message(show_message);
	ambulant::lib::logger::get_logger()->trace("starting:%s", "iAmbulant");
#ifdef WITH_CONSOLE_LOGGING
	ambulant::lib::logger::get_logger()->set_ostream(new nslog_ostream);
#endif
	// Tell the logger about the output level preference
#ifdef _DEBUG
	int level = ambulant::lib::logger::LEVEL_DEBUG; //ambulant::common::preferences::get_preferences()->m_log_level;
#else
	int level = ambulant::common::preferences::get_preferences()->m_log_level;
#endif
	ambulant::lib::logger::get_logger()->set_level(level);
	return level;
}

@implementation AmbulantAppDelegate

@synthesize autoCenter;
@synthesize autoResize;
@synthesize nativeRenderer;
@synthesize autoHideHUD;
@synthesize shortTapForHUD;

#pragma mark -
#pragma mark Application lifecycle

- (BOOL)
application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{    
    // Override point for customization after application launch.
    // Add the view controller's view to the window and display.

	AM_DBG NSLog(@"AmbulantAppDelegate application didFinishLaunchingWithOptions: %@", launchOptions);
	// Install ambulent preferences handler
	ambulant::iOSpreferences::install_singleton();
	
	// Install ambulant logger
	initialize_logger();

    // Setup preferences that are important for UI
	ambulant::iOSpreferences::get_preferences()->load_preferences();
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	autoCenter = prefs->m_auto_center;
	autoResize = prefs->m_auto_resize;
	autoHideHUD = prefs->m_hud_auto_hide;
	shortTapForHUD = prefs->m_hud_short_tap;
	nativeRenderer = ! prefs->m_prefer_ffmpeg;
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter]
        addObserver:self
        selector:@selector(orientationChanged:)
        name:UIDeviceOrientationDidChangeNotification
        object: nil];
    return YES;
}


- (void)
applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
	AM_DBG NSLog(@"AmbulantAppDelegate applicationWillResignActive");
	if (viewController) [viewController pause];
}

- (BOOL)
isValid: (NSURL*) url {
	return YES;
}

//XX need to use: application:openURL:sourceApplication:annotation: {}

- (BOOL)
application:(UIApplication* ) application handleOpenURL: (NSURL*) url {
	AM_DBG NSLog(@"AmbulantAppDelegate application handleOpenURL");
	AM_DBG ambulant::lib::logger::get_logger()->trace("AmbulantAppDelegate handleOpenURL: %s", [[url absoluteString] UTF8String]);
	
	BOOL validated = NO;
	if ([self isValid:url] && viewController != NULL) { 
		validated = YES;
        NSString *urlstr = [url absoluteString];
        if ([urlstr hasPrefix: @"ambulant:"]) {
            urlstr = [urlstr substringFromIndex: 9]; // Length of "ambulant:"
        }
        [viewController doPlayURL: urlstr fromNode: nil];
	}
	return validated;
}

- (void)
applicationWillEnterForeground:(UIApplication *)application {
	ambulant::iOSpreferences::delete_preferences_singleton();
}


- (void)
applicationDidBecomeActive:(UIApplication *)application {
    // We are finally becoming active. There are a few possible cases:
    // 1. Our player view already has a document (presumably throughOpenURL). Play it.
    // 2. No document in the player view, and Welcome.smil never seen. Show it.
    // 3. The most recent history item was unfinished, and we didn't crash on
    //    the previous run. Show it.
    // 4. Otherwise we show the history.
    bool showPlayer = [viewController canPlay];

    if (!showPlayer) {
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        if ( ![defaults boolForKey: @"welcomeDocumentSeen"] ) {
            [self playWelcome: self];
            [defaults setBool: YES forKey: @"welcomeDocumentSeen"];
            showPlayer = [viewController canPlay];
        }
    }
    if (!showPlayer) {
        ambulant::iOSpreferences::get_preferences()->load_preferences();
        ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		if (prefs->m_normal_exit) {
			// restart where left
			PlaylistItem* last_item = prefs->m_history != NULL ? prefs->m_history->get_last_item() : NULL;
			NSString *startPath = last_item != NULL ? [[last_item url] absoluteString] : NULL;
			NSString *startNodeRepr = last_item != NULL ? [last_item position_node] : NULL;
            if (startPath && startNodeRepr) {
                [viewController doPlayURL: startPath fromNode: startNodeRepr];
            }
		}
        showPlayer = [viewController canPlay];
    }
       
    // Note: the order of adding the views to the window is important.
    // Only the first viewController will be informed about rotation, see
    // <http://developer.apple.com/library/ios/#qa/qa2010/qa1688.html>. 
    window.rootViewController = viewController;
    if (showPlayer) {
        viewController.view.alpha = 1.0;
        viewController.view.hidden = false;

        tabBarController.view.alpha = 0.0;
        tabBarController.view.hidden = true;
    } else {
        viewController.view.alpha = 0.0;
        viewController.view.hidden = true;

        tabBarController.view.alpha = 1.0;
        tabBarController.view.hidden = false;
    }
    [window addSubview:viewController.view];
    [window addSubview:tabBarController.view];
    [window makeKeyAndVisible];
}

- (void)
applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers,
	// and store enough application state information to restore your application to its current state
	// in case it is terminated later. 
    // If your application supports background execution, called instead of applicationWillTerminate:
	// when the user quits.
 	AM_DBG NSLog(@"AmbulantAppDelegate applicationDidEnterBackground");
	// save current state
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	prefs->save_preferences();
	if (viewController) [viewController pause];
	
}


- (void)
applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate.
    // See also applicationDidEnterBackground:.
	AM_DBG NSLog(@"AmbulantAppDelegate applicationWillTerminate: viewController.retainCount()=%d", [viewController retainCount]);
    [viewController willTerminate];
    [viewController release];
}


#pragma mark -
#pragma mark Showing views
- (void) showAmbulantPlayer: (id)sender
{
	[ UIView animateWithDuration: 1.0 animations: ^
	 {
		 tabBarController.view.hidden = true;
		 tabBarController.view.alpha = 0.0;
		 viewController.view.alpha = 1.0;
		 viewController.view.hidden = false;
	 } ];
	[viewController play];
}

- (void) showPresentationViews: (id)sender
{
	[viewController pause];
	[UIView animateWithDuration: 1.0
        animations:
        ^{
            tabBarController.view.hidden = false;
            tabBarController.view.alpha = 1.0;
            viewController.view.alpha = 0.0;
        }
    ];
}

- (void)
showPresentationViewWithIndex: (NSUInteger) index
{
	tabBarController.selectedIndex = index;
	[self showPresentationViews: self];
}

- (void)
openWebLink: (NSString*) url {
	AM_DBG NSLog(@"AmbulantAppDelegate openWebLink: %@", url);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[viewController pause];
	if (url != NULL) {
		NSURL* nsurl = [NSURL URLWithString: url];
		if ([[UIApplication sharedApplication] canOpenURL: nsurl]) {
			[[UIApplication sharedApplication] openURL: nsurl];
		}
	}
	[pool release];
}

- (void) playWelcome: (id)sender
{
    NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
    assert(thisBundle);
    NSString *startPath = [thisBundle pathForResource:@"Welcome" ofType:@"smil"];
    if (startPath == NULL) {
        ambulant::lib::logger::get_logger()->error("Document Welcome.smil missing from application bundle");
        return;
    }
    NSURL *startURL = [NSURL fileURLWithPath: startPath];
    assert(viewController);
    [viewController doPlayURL: [startURL absoluteString] fromNode: nil];
}

#pragma mark -
#pragma mark Messages and inter-view services


- (void)
showAlert: (NSString*) message {
	NSString* title_ = @"Ambulant";
	NSString* _detail;

	if (message == nil) _detail = @"";
	else _detail = message;
	UIAlertView *alert =
	[[UIAlertView alloc] initWithTitle: title_
							   message: _detail
							  delegate:nil	
					 cancelButtonTitle:@"OK"
					 otherButtonTitles:nil];
	[alert show];
	[alert release];
}

- (PresentationViewController*)
getPresentationViewWithIndex: (NSUInteger) index
{
	UINavigationController* navigationController = [tabBarController.viewControllers objectAtIndex: index];
	return (PresentationViewController*) [navigationController.viewControllers objectAtIndex:0];
}

- (void)
document_stopped: (id) sender
{
	[viewController stopped]; // to activate the 'Play" button
}

- (void) settingsHaveChanged:(SettingsViewController *)controller {
	AM_DBG NSLog(@"AmbulantViewController showSettings(%p)", self);
	// check we have the settings view
	if (controller.view.tag != 40) {
		return;
	}
	// get the values entered by the user
	autoCenter = [controller autoCenter];
	autoResize = [controller autoResize];
	nativeRenderer = [controller nativeRenderer];
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	prefs->m_auto_center = autoCenter;
	prefs->m_auto_resize = autoResize;
	prefs->m_prefer_ffmpeg = ! nativeRenderer;
	prefs->save_preferences();
    if (viewController) {
        [viewController settingsHaveChanged];
    }
}

- (void) auxViewControllerDidFinish: (UIViewController *)controller {
	AM_DBG NSLog(@"auxViewControllerDidFinish: controller=%p", controller);
    // XXX Needed?[viewController orientationChanged: nil];
	// Most view auxiliary view controllers may change some of items stored in preferences
	ambulant::iOSpreferences::get_preferences()->save_preferences();
	[self showAmbulantPlayer: self];
    [viewController play];
}

- (void) setHistoryViewController:(PresentationViewController *)controller
{
	AM_DBG NSLog(@"AmbulantViewController setHistoryViewController(%p) controller=%p", self, controller);
    history = controller;
	if (currentPVC == NULL) {
		currentPVC = history;
	}
}

- (BOOL) canSelectNextPresentation
{
	return currentPVC && ![currentPVC isHistory];
}

- (void) selectNextPresentation
{
    if (currentPVC) [currentPVC selectNextPresentation];
}

- (void) playURL: (NSString*) whatString {
    // XXXJACK: Change interface to get PlayListItem, which has the position_offset as well.
	AM_DBG NSLog(@"AmbulantViewController (%p)", self);
	AM_DBG NSLog(@"Selected: %@",whatString);
	currentPVC = nil;
    [viewController doPlayURL: whatString fromNode: nil];
    if ([viewController canPlay]) {
        [self showAmbulantPlayer: self];
        [history updatePlaylist];
    }
}

- (void) playPresentation: (PlaylistItem*) item fromPresentationViewController: (PresentationViewController*) controller {
    // XXXJACK: Change interface to get PlayListItem, which has the position_offset as well.
	AM_DBG NSLog(@"AmbulantViewController (%p)", self);
	AM_DBG NSLog(@"Selected: %@",item);
	currentPVC = controller;
    [viewController doPlayURL: [[item url] absoluteString] fromNode: [item position_node]];
    if ([viewController canPlay]) {
        [self showAmbulantPlayer: self];
        [history updatePlaylist];
    }
}

- (void) orientationChanged:(NSNotification *)notification {
    [window setNeedsLayout];
}

- (BOOL) canShowRotatedUIViews
{
    return UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch {
    if ([touch.view isKindOfClass:[UIControl class]]) {
        // we touched a button, slider, or other UIControl
        return NO; // ignore the touch
    }
    return YES; // handle the touch
}

#pragma mark -
#pragma mark Memory management

- (void)
applicationDidReceiveMemoryWarning:(UIApplication *)application {
    // Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
	AM_DBG NSLog(@"AmbulantAppDelegate applicationDidReceiveMemoryWarning");
	if (viewController) [viewController pause];
	ambulant::lib::logger::get_logger()->error("Memory low, try reboot iPhone");
}


- (void)
dealloc {
	AM_DBG NSLog(@"AmbulantAppDelegate dealloc");
	[tabBarController release];
    [viewController release];	
    [window release];
    [super dealloc];
}


@end
