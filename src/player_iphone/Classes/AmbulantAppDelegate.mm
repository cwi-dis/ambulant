//
//  AmbulantAppDelegate.mm
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright CWI 2010. All rights reserved.
//

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#define DBG_ADD(_x)
#define DBG_ADD_NSSTRING(_x)
#else //AM_DBG is set
#define GLOB_DBG if(1)
#ifdef	GLOB_DBG
// Debugging without debugger from traces in memory stored in a global array of char*.
// Next code is used to inspect program sections only used when it is not started from debugger, but e.g. by Safari
// When the programs has been launched, then XCode can attach to it and display the global string array 'DBG'
char* DBG[80]; int DBGi = 0;
#define DBG_ADD(_x)  { DBG[DBGi++] = strdup(_x); }
#define DBG_ADD_NSSTRING(_x) { const char*s = [_x cStringUsingEncoding: NSUTF8StringEncoding]; DBG_ADD(s); }
#endif//GLOB_DBG
#endif//AM_DBG
#import "AmbulantAppDelegate.h"
#import "AmbulantViewController.h"
#import "AmbulantWebViewController.h"
#import <CoreFoundation/CoreFoundation.h>

#include "ambulant/lib/logger.h"
#include "iOSpreferences.h"
#include <fstream>

#ifndef NDEBUG
#define WITH_CONSOLE_LOGGING
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
			syslog(LOG_INFO, "%s", m_curstring.c_str());
			m_curstring = "";
		}
		return 0;
	}
	void flush() {}
};
#endif // WITH_CONSOLE_LOGGING
#endif // NDEBUG


static void
show_message(int level, const char *format)
{
	static NSString* old_message = NULL;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *message = [[NSString stringWithUTF8String: format] retain];
	AmbulantAppDelegate *delegate = [[UIApplication sharedApplication] delegate];
	
	// do not repeat the same error succesively
	if (old_message == NULL || ! [old_message isEqualToString:message]) {
		[delegate performSelectorOnMainThread: @selector(showAlert:)
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
	ambulant::lib::logger::get_logger()->trace("starting:%s", "player_iphone");
#ifdef WITH_CONSOLE_LOGGING
	ambulant::lib::logger::get_logger()->set_ostream(new nslog_ostream);
#endif
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
//JNK @synthesize webViewController;


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
//	AM_DBG [DBG addObject: [NSString stringWithString: @"didFinishLaunchingWithOptions"]]; 
	AM_DBG DBG_ADD("didFinishLaunchingWithOptions");
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
	[viewController play];
}

- (void)
//application:(UIApplication *)application 
showPresentationViews:(void *)id
{
	[viewController pause];
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
document_stopped: (NSObject*) obj
{
	[viewController pause]; // to activate the 'Play" button
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

//XX need to use: application:openURL:sourceApplication:annotation: {}

- (BOOL)
application:(UIApplication* ) application handleOpenURL: (NSURL*) url {
	AM_DBG NSLog(@"AmbulantAppDelegate application handleOpenURL");
	AM_DBG NSLog(@"AmbulantAppDelegate handleOpenURL: %@", [url absoluteURL]);
	AM_DBG DBG_ADD("handleOpenURL");
//	const char*s = [[url absoluteString] cStringUsingEncoding: NSUTF8StringEncoding];
	AM_DBG DBG_ADD_NSSTRING([url absoluteString]);
	
	BOOL validated = NO;
	if ([self isValid:url] && viewController != NULL) { 
		validated = YES;
#if 0
		viewController.playURL = (NSMutableString*) [[[NSString alloc] initWithString: @"http://"]
									stringByAppendingString: [[url resourceSpecifier] substringFromIndex:2]];
#else
        NSString *urlstr = [url absoluteString];
        if ([urlstr hasPrefix: @"ambulant://"]) {
            // Kees' format: replace http by ambulant
            urlstr = [NSMutableString stringWithFormat: @"http:%@", [urlstr substringFromIndex: 9]]; // Length of "ambulant:"
        }
        if ([urlstr hasPrefix: @"ambulant:"]) {
            // Jack's format: prepend ambulant:
            urlstr = [urlstr substringFromIndex: 9]; // Length of "ambulant:"
        }
        // XXXJACK: Why is this a NSMutableString???? Also, this hard cast looks very dangerous.....
        viewController.playURL = [NSMutableString stringWithString:urlstr];
#endif
		char* s2;
		if (viewController == NULL)
			s2 = (char*) "viewController == NULL";
		else if (viewController.playURL == NULL)
			s2 =  (char*) "viewController.playURL == NULL";
		else s2 =  (char*) [viewController.playURL cStringUsingEncoding: NSUTF8StringEncoding];
		AM_DBG DBG_ADD(s2);
		if (viewController.myMainloop) {
//XXXX		viewController.myMainloop->stop();
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
	// save current state
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	prefs->save_preferences();
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
	// restore state
	// ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	NSString* ns_node_repr = NULL;// [prefs->m_history->get_last_item m_last_node];
	if (viewController != NULL && viewController.myMainloop != NULL) {
		viewController.myMainloop->play();
	} else {
		if (viewController != NULL) {
			[self.viewController doPlayURL: ns_node_repr];
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
			[self.viewController doPlayURL:NULL]; //[prefs->m_history.last_item() m_ns_noder_repr];
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
