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

/// The PresentationViewController is used to manage 2 different instances: History and Favorites.
/// The implementation for the latter also alows for adding the item that is currently being played.

// The current implementation has a hack, that an empty first item is always present in the TableView,
// which is obscured by the NavigationBar
#define FIRST_ITEM 2

@interface PresentationViewController : UITableViewController < UITableViewDataSource, UITableViewDelegate > {
	id <PlaylistViewControllerDelegate> delegate;
	NSMutableArray* presentationsArray;
	UITableViewCell* nibLoadedCell;
	Presentation* selectedPresentation;
	BOOL isFavorites;
	UITableViewCellEditingStyle editingStyle;
	BOOL wantStyleInsert;
	Presentation* newPresentation;
}
- (IBAction) toggleEditMode;
- (IBAction) done: (id) sender;
- (void) insertCurrentItemAtIndexPath: (NSIndexPath*) indexPath;
- (Presentation*) getPresentationFromPlaylistItem: (PlaylistItem *) item;

@property(nonatomic, assign) id <PlaylistViewControllerDelegate> delegate;
@property(nonatomic, retain) IBOutlet UITableViewCell* nibLoadedCell;
@property(nonatomic, retain) NSMutableArray* presentationsArray;
@end
