//
//  AmbulantAppDelegate.mm
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright CWI 2010. All rights reserved.
//

#import "AmbulantAppDelegate.h"
#import "AmbulantViewController.h"
#import "AmbulantWebViewController.h"
#import <CoreFoundation/CoreFoundation.h>

#include "ambulant/lib/logger.h"
#include "iOSpreferences.h"
#include <fstream>

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

static void
show_message(int level, const char *format)
{
	static NSString* old_message = NULL;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *message = [[NSString stringWithUTF8String: format] retain];
	AmbulantAppDelegate *delegate = [[UIApplication sharedApplication] delegate];
	if (old_message != NULL && ! [old_message isEqualToString:message]) {
		[delegate performSelectorOnMainThread: @selector(showAlert:)
								   withObject: message
								waitUntilDone: NO];
	//	[message release];
		if (old_message != NULL) {
			[old_message release];
		}
		[old_message release];
		old_message = message;
	} else {
		[message release];
	}
	[pool release];
}

static bool
initialize_logger()
{
	// Connect logger to our message displayer and output processor
	ambulant::lib::logger::get_logger()->set_show_message(show_message);
	ambulant::lib::logger::get_logger()->trace("starting:%s", "player_iphone");
//	if (getenv("AMBULANT_LOGGER_NOWINDOW") == NULL)
//		ambulant::lib::logger::get_logger()->set_ostream(new nslog_ostream);
	// Tell the logger about the output level preference
	int level = ambulant::lib::logger::LEVEL_DEBUG; //ambulant::common::preferences::get_preferences()->m_log_level;
	ambulant::lib::logger::get_logger()->set_level(level);
	// And tell the UI too
	// LogController *log = [LogController sharedLogController];
	// if (log) [log setLogLevelUI: level];
	return level;
}

@implementation AmbulantAppDelegate

@synthesize window;
@synthesize tabBarController;
@synthesize viewController;
@synthesize webViewController;


#pragma mark -
#pragma mark Application lifecycle

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

- (void)
openWebLink: (NSString*) url {
	AM_DBG NSLog(@"AmbulantAppDelegate openWebLink: %@", url);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[viewController pause];
#ifdef JNK
	webViewController = [[AmbulantWebViewController alloc]
	 initWithNibName: @"AmbulantWebView"
			  bundle: nil];
	webViewController.urlField = url;
	[window addSubview:webViewController.view];
	webViewController.modalTransitionStyle = 
	UIModalTransitionStyleFlipHorizontal;
	
	[viewController presentModalViewController: webViewController animated: YES];
	
//	[[viewController navigationController] pushViewController:
//	 webViewController animated:YES];
	[webViewController release];
#endif//JNK
	if (url != NULL) {
		NSURL* nsurl = [NSURL URLWithString: url];
		if ([[UIApplication sharedApplication] canOpenURL: nsurl]) {
			[[UIApplication sharedApplication] openURL: nsurl];
		}
	}
	[pool release];
}

- (BOOL)
application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{    
    // Override point for customization after application launch.
    // Add the view controller's view to the window and display.

	AM_DBG NSLog(@"AmbulantAppDelegate application didFinishLaunchingWithOptions");
	// Install ambulent preferences handler
	ambulant::iOSpreferences::install_singleton();
	
	// Install ambulant logger
	initialize_logger();
    [window addSubview:viewController.view];
	[window addSubview:tabBarController.view];
	tabBarController.view.alpha = 0.0;
	tabBarController.view.hidden = true;
    [window makeKeyAndVisible];

    return YES;
}

- (void)
//application:(UIApplication *)application 
showAmbulantPlayer: (void*) id
{
	[ UIView animateWithDuration: 1.0 animations: ^
	 {
		 tabBarController.view.hidden = true;
		 tabBarController.view.alpha = 0.0;
		 viewController.view.alpha = 1.0;
	 } ];
}

- (void)
//application:(UIApplication *)application 
showPresentationViews:(void *)id
{
	[ UIView animateWithDuration: 1.0 animations: ^
	 {
		 tabBarController.view.hidden = false;
		 tabBarController.view.alpha = 1.0;
		 viewController.view.alpha = 0.0;
	 } ];
}

- (PresentationViewController*)
getPresentationView: (void*) id withIndex: (NSUInteger) index
{
	return (PresentationViewController*) [tabBarController.viewControllers objectAtIndex: index];
}

- (void)
showPresentationView: (void*) id withIndex: (NSUInteger) index
{
	tabBarController.selectedIndex = 1;
	[self showPresentationViews: id];
}

- (void)
applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
	AM_DBG NSLog(@"AmbulantAppDelegate applicationWillResignActive");
	if (viewController != NULL && viewController.myMainloop != NULL) {
		viewController.myMainloop->pause();
	}
}

- (BOOL)
isValid: (NSURL*) url {
	/* Validate the given 'url'
	 */
	return YES;
}

- (BOOL)
application:(UIApplication* ) application handleOpenURL: (NSURL*) url {
	AM_DBG NSLog(@"AmbulantAppDelegate application handleOpenURL");
	AM_DBG NSLog(@"AmbulantAppDelegate handleOpenURL: %@", [url absoluteURL]);
	BOOL validated = NO;
	if ([self isValid:url] && viewController != NULL) { 		validated = YES;
		viewController.URLEntryField.text = [[[NSMutableString alloc] initWithString: @"http://"]
							stringByAppendingString: [[url resourceSpecifier] substringFromIndex:2]];
		viewController.playURL = (NSMutableString*) viewController.URLEntryField.text;
		if (viewController.myMainloop) {
			viewController.myMainloop->stop();
			delete viewController.myMainloop;
			viewController.myMainloop = NULL;
		}
	}
	return validated;
}

- (void)
applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers,
	 and store enough application state information to restore your application to its current state
	 in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate:
	 when the user quits.
     */
	AM_DBG NSLog(@"AmbulantAppDelegate applicationDidEnterBackground");
	//XXXX TBD: store state
	if (viewController != NULL && viewController.myMainloop != NULL) {
		viewController.myMainloop->pause();
	}
	
}


- (void)
applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state:
	 here you can undo many of the changes made on entering the background.
	 */
	AM_DBG NSLog(@"AmbulantAppDelegate applicationWillEnterForeground");
	//XXXX TBD: restore state
	if (viewController != NULL && viewController.myMainloop != NULL) {
		viewController.myMainloop->play();
	} else {
		if (viewController != NULL) {
			[self.viewController handleURLEntered];
		}
	}
	ambulant::iOSpreferences::delete_preferences_singleton();
}


- (void)
applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. 
	 If the application was previously in the background, optionally refresh the user interface.
     */
	AM_DBG NSLog(@"AmbulantAppDelegate applicationDidBecomeActive");
/* AmulantIOS is not a restartable app. */
//XXXX TBD: restore state
//	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	if (viewController != NULL && viewController.myMainloop != NULL) {
		//	viewController.myMainloop->restart(true);
		if ( ! viewController.myMainloop->is_play_active()) {
			viewController.myMainloop->play();
		}
	} else {
		if (viewController != NULL && self.viewController.playURL != NULL) {
			[self.viewController handleURLEntered];
		}
	}
}

- (void)
applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
	AM_DBG NSLog(@"AmbulantAppDelegate applicationWillTerminate: viewController.retainCount()=%d", [viewController retainCount]);
	if (viewController && viewController.myMainloop) {
		delete viewController.myMainloop;
	}
    [viewController release];
}


#pragma mark -
#pragma mark Memory management

- (void)
applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
	AM_DBG NSLog(@"AmbulantAppDelegate applicationDidReceiveMemoryWarning");
	if (viewController && viewController.myMainloop) {
		viewController.myMainloop->pause();
	}
	ambulant::lib::logger::get_logger()->error("Memory low, try reboot iPhone");
	if (viewController && viewController.myMainloop) {
		viewController.myMainloop->play();
	}
}


- (void)
dealloc {
	AM_DBG NSLog(@"AmbulantAppDelegate dealloc");
	if (viewController && viewController.myMainloop) {
		delete viewController.myMainloop;
	}
    [viewController release];	
    [window release];
    [super dealloc];
}


@end
