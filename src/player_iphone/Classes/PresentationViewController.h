//
//  PresentationViewController.h
//  PresentationView
//
//  Created by Kees Blom on 10/31/10.
//  Copyright Stg.CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PresentationViewController.h"

@protocol PresentationViewControllerDelegate;

@interface PresentationViewController : UIViewController {
	id <PresentationViewControllerDelegate> delegate;
	NSMutableArray* presentationsArray;
	UITableViewCell* nibLoadedCell;
	UINavigationController* naviationController;
}
- (IBAction) done: (id) sender;

@property(nonatomic, assign) id <PresentationViewControllerDelegate> delegate;
@property(nonatomic, retain) UINavigationController* naviationController;
@property(nonatomic, retain) IBOutlet UITableViewCell* nibLoadedCell;
@end

@protocol PresentationViewControllerDelegate
- (void) presentationViewControllerDidFinish: (PresentationViewController *)controller;
- (void) playPresentation: (PresentationViewController *)controller selected: (NSString*) what;
- (IBAction) done: (id) sender;
@end
