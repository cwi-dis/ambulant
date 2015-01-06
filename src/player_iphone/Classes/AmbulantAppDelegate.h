/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#import <UIKit/UIKit.h>
#import "PresentationViewController.h"
#import "SettingsViewController.h"

@class AmbulantViewController;

@interface AmbulantAppDelegate : NSObject <UIApplicationDelegate, UIGestureRecognizerDelegate, PlaylistViewControllerDelegate> {
    IBOutlet UIWindow *window;
	IBOutlet UITabBarController* tabBarController;
    IBOutlet AmbulantViewController *viewController;
    PresentationViewController *history;
    PresentationViewController *currentPVC;    // Either history or favorites

	BOOL autoCenter;
	BOOL autoResize;
	BOOL nativeRenderer;
	BOOL autoHideHUD;
	BOOL shortTapForHUD;
}

@property(readonly) BOOL autoCenter;
@property(readonly) BOOL autoResize;
@property(readonly) BOOL nativeRenderer;
@property(readonly) BOOL autoHideHUD;
@property(readonly) BOOL shortTapForHUD;

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
- (BOOL) canSelectNextPresentation;
- (void) selectNextPresentation;
- (void) playURL: (NSString*) whatString;
- (void) playPresentation: (PlaylistItem*) item fromPresentationViewController: (PresentationViewController*) controller;
- (BOOL) canShowRotatedUIViews;

- (void) applicationDidReceiveMemoryWarning:(UIApplication *)application;
- (void) dealloc;
@end
