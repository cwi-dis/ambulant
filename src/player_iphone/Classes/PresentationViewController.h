//
//  PresentationViewController.h
//  PresentationView
//
//  Created by Kees Blom on 10/31/10.
//  Copyright Stg.CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Presentation.h"
#import "PresentationViewController.h"
#import "PlaylistAppDelegate.h"

@protocol PresentationViewControllerDelegate;

@interface PresentationViewController : UIViewController {
	id <PlaylistViewControllerDelegate> delegate;
	NSMutableArray* presentationsArray;
	UITableViewCell* nibLoadedCell;
	Presentation* selectedPresentation;
}
- (IBAction) done: (id) sender;

@property(nonatomic, assign) id <PlaylistViewControllerDelegate> delegate;
@property(nonatomic, retain) IBOutlet UITableViewCell* nibLoadedCell;
@end
