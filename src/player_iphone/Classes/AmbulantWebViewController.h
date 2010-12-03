//
//  AmbulantWebViewController.h
//  player_iphone
//
//  Created by Kees Blom on 8/7/10.
//  Copyright 2010 CWI. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PlaylistAppDelegate.h"


@interface AmbulantWebViewController : UIViewController {
	id <PlaylistViewControllerDelegate> delegate;
	IBOutlet UIWebView* webView;
	NSString* urlField;
}
- (IBAction) handleBackTapped;
- (IBAction) handleDoneTapped;

@property (nonatomic, retain) UIWebView* webView;
@property (nonatomic, retain) NSString* urlField;
@property (nonatomic, retain) id <PlaylistViewControllerDelegate> delegate;

@end
