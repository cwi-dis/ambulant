//
//  AmbulantAppDelegate.h
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
//JNK #import "AmbulantWebViewController.h"
#import "PresentationViewController.h"
#import "SettingsViewController.h"

@class AmbulantViewController;

@interface AmbulantAppDelegate : NSObject <UIApplicationDelegate, PlaylistViewControllerDelegate> {
    IBOutlet UIWindow *window;
	IBOutlet UITabBarController* tabBarController;
    IBOutlet AmbulantViewController *viewController;
    PresentationViewController *history;
    PresentationViewController *currentPVC;    // Either history or favorites

	BOOL autoCenter;
	BOOL autoResize;
	BOOL nativeRenderer;
}

@property(readonly) BOOL autoCenter;
@property(readonly) BOOL autoResize;
@property(readonly) BOOL nativeRenderer;

- (BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions;
- (void) applicationWillResignActive:(UIApplication *)application;
- (BOOL) isValid: (NSURL*) url;
- (BOOL) application:(UIApplication* ) application handleOpenURL: (NSURL*) url;
- (void) applicationWillEnterForeground:(UIApplication *)application;
- (void) applicationDidBecomeActive:(UIApplication *)application;
- (void) applicationDidEnterBackground:(UIApplication *)application;
- (void) applicationWillTerminate:(UIApplication *)application;

- (void) showAmbulantPlayer: (id)sender;
- (void) showPresentationViews: (id)sender;
- (void) showPresentationViewWithIndex: (NSUInteger) index; 
- (void) openWebLink: (NSString*) url;
- (void) playWelcome: (id)sender;

- (void) showAlert: (NSString*) msgtype;
- (PresentationViewController*) getPresentationViewWithIndex: (NSUInteger) index; 
- (void) document_stopped: (id) sender;
- (void) settingsHaveChanged:(SettingsViewController *)controller;
- (void) auxViewControllerDidFinish: (UIViewController *)controller;
- (void) setHistoryViewController:(PresentationViewController *)controller;
- (void) selectNextPresentation;
- (void) playPresentation: (NSString*) whatString fromPresentationViewController: (PresentationViewController*) controller;

- (void) applicationDidReceiveMemoryWarning:(UIApplication *)application;
- (void) dealloc;
@end
#ifdef GLOB_DBG
extern char* DBG[];
extern int DBGi;
#define DBG_ADD(_x)  { DBG[DBGi++] = strdup(_x); }
#define DBG_ADD_NSSTRING(_x) { const char*s = [_x cStringUsingEncoding: NSUTF8StringEncoding]; DBG_ADD(s); }
#endif//GLOB_DBG
