//
//  AmbulantWebViewController.h
//  player_iphone
//
//  Created by Kees Blom on 8/7/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface AmbulantWebViewController : UIViewController {
	IBOutlet UIWebView* webView;
	NSString* urlField;
}
- (IBAction) handleBackTapped;

@property (nonatomic, retain) UIWebView* webView;
@property (nonatomic, retain) NSString* urlField;

@end
