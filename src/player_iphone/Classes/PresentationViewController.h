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

@property(nonatomic, retain) IBOutlet UITableViewCell* nibLoadedCell;
@end
