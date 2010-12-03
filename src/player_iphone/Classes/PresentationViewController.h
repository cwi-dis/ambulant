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
#import "iOSpreferences.h"
#import "Presentation.h"
#import "PlaylistAppDelegate.h"

@protocol PresentationViewControllerDelegate;

@interface PresentationViewController : UITableViewController < UITableViewDataSource, UITableViewDelegate > {
	id <PlaylistViewControllerDelegate> delegate;
	NSMutableArray* presentationsArray;
	UITableViewCell* nibLoadedCell;
	Presentation* selectedPresentation;
	BOOL isFavorites;
	Presentation* newPresentation;
}
- (IBAction) done: (id) sender;
- (IBAction) addCurrentItem;
- (Presentation*) getPresentationFromPlaylistItem: (PlaylistItem *) item;

@property(nonatomic, assign) id <PlaylistViewControllerDelegate> delegate;
@property(nonatomic, retain) IBOutlet UITableViewCell* nibLoadedCell;
@end
