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

@class AmbulantViewController;

@interface AmbulantAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	IBOutlet UITabBarController* tabBarController;
    AmbulantViewController *viewController;
//JNK    AmbulantWebViewController *webViewController;
}
- (void) openWebLink: (NSString*) url;

- (void) document_stopped: (NSObject*) obj;
- (PresentationViewController*) getPresentationView: (void*) id withIndex: (NSUInteger) index; 
- (void) showAlert: (NSString*) msgtype;
- (void) showAmbulantPlayer: (void*) id;
- (void) showPresentationView: (void*) id withIndex: (NSUInteger) index; 
- (void) showPresentationViews: (void*) id;

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UITabBarController* tabBarController;
@property (nonatomic, retain) IBOutlet AmbulantViewController *viewController;
//JNK @property (nonatomic, retain) IBOutlet AmbulantWebViewController* webViewController;
@end
#ifdef GLOB_DBG
extern char* DBG[];
extern int DBGi;
#define DBG_ADD(_x)  { DBG[DBGi++] = strdup(_x); }
#define DBG_ADD_NSSTRING(_x) { const char*s = [_x cStringUsingEncoding: NSUTF8StringEncoding]; DBG_ADD(s); }
#endif//GLOB_DBG
