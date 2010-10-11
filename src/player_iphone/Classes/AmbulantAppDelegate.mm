//
//  AmbulantAppDelegate.mm
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "AmbulantAppDelegate.h"
#import "AmbulantViewController.h"
#import "AmbulantWebViewController.h"
#import <CoreFoundation/CoreFoundation.h>

#include "ambulant/lib/logger.h"
#include "cg_preferences.h"

static void
show_message(int level, const char *format)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *message = [[NSString stringWithUTF8String: format] retain];
	AmbulantAppDelegate *delegate = [[UIApplication sharedApplication] delegate];
	[delegate performSelectorOnMainThread: @selector(showAlert:)
							   withObject: message	waitUntilDone: NO];
	//	[message release];
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
@synthesize viewController;
@synthesize webViewController;


#pragma mark -
#pragma mark Application lifecycle

- (void) showAlert: (NSString*) message {
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

- (void) openWebLink: (NSString*) url {
	NSLog(@"Starting AmbulantWebView");
	[viewController pause];
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
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{    
    // Override point for customization after application launch.
    // Add the view controller's view to the window and display.

	// Install ambulent preferences handler
	ambulant::gui::cg::cg_preferences::install_singleton();
	
	// Install ambulant logger
	initialize_logger();
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];

    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}

- (BOOL) isValid: (NSURL*) url {
	return YES;
}

- (BOOL) application:(UIApplication* ) application handleOpenURL: (NSURL*) url {
	BOOL validated = NO;
	if (YES) { //([isValid: url]) {
		validated = YES;
		viewController.URLEntryField.text = [[[NSMutableString alloc] initWithString: @"http://"]
							stringByAppendingString: [[url resourceSpecifier] substringFromIndex:2]];
		if (viewController.myMainloop) {
			viewController.myMainloop->stop();
			delete viewController.myMainloop;
			viewController.myMainloop = NULL;
		}
	}
	return validated;
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers,
	 and store enough application state information to restore your application to its current state
	 in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate:
	 when the user quits.
     */
	//XXXX TBD: store state
	if (viewController && viewController.myMainloop) {
		viewController.myMainloop->pause();
	}
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state:
	 here you can undo many of the changes made on entering the background.
	 */
	//XXXX TBD: restore state
	if (viewController && viewController.myMainloop) {
		viewController.myMainloop->play();
	} else {
		if (viewController) {
			[self.viewController handleURLEntered];
		}
	}

}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. 
	 If the application was previously in the background, optionally refresh the user interface.
     */
//XXXX TBD: restore state
	if (viewController && viewController.myMainloop) {
		//	viewController.myMainloop->restart(true);
		viewController.myMainloop->play();
	} else {
		if (viewController) {
			[self.viewController handleURLEntered];
		}
	}

}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
	if (viewController && viewController.myMainloop) {
		viewController.myMainloop->stop();
	}
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
	NSLog(@"Memory warning received");
}


- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}


@end
