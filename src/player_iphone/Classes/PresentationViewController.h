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
#define FIRST_ITEM 1

@interface PresentationViewController : UITableViewController < UITableViewDataSource, UITableViewDelegate > {
	id <PlaylistViewControllerDelegate> delegate;
	NSMutableArray* presentationsArray;
	UITableViewCell* nibLoadedCell;
	Presentation* selectedPresentation;
	NSInteger currentIndex;
	BOOL isFavorites;
	UITableViewCellEditingStyle editingStyle;
	Presentation* newPresentation;
}
- (void) awakeFromNib;
- (void) viewDidLoad;

// user actions
- (IBAction) toggleEditMode;
- (IBAction) done: (id) sender;
// aux.
- (void) insertCurrentItemAtIndexPath: (NSIndexPath*) indexPath;
- (Presentation*) getPresentationFromPlaylistItem: (PlaylistItem *) item;
- (void) selectNextPresentation;
- (NSArray*) get_playlist;
- (void) updatePlaylist;
- (BOOL) isFavorites;
@end
