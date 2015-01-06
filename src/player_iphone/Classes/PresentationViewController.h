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
#import "PlaylistItem.h"
#import "PresentationViewController.h"
#import "iOSpreferences.h"
#import "PlaylistItem.h"
#import "PlaylistAppDelegate.h"

@protocol PresentationViewControllerDelegate;

/// The PresentationViewController is used to manage 2 different instances: History and Favorites.

@interface PresentationViewController : UITableViewController < UITableViewDataSource, UITableViewDelegate > {
	id <PlaylistViewControllerDelegate> delegate;
	NSMutableArray* presentationsArray;
	IBOutlet UITableViewCell* nibLoadedCell; // XXXJACK thinks this isn't neeeded: it isn't initializaed anywhere...
	NSInteger currentIndex;
	BOOL isHistory;
	UITableViewCellEditingStyle editingStyle;
}
- (void) awakeFromNib;
- (void) viewDidLoad;

// user actions
- (IBAction) toggleEditMode;
- (IBAction) done: (id) sender;
// aux.
- (void) insertCurrentItemAtIndexPath: (NSIndexPath*) indexPath;
- (void) selectNextPresentation;
- (NSArray*) get_playlist;
- (void) updatePlaylist;
- (BOOL) isHistory;
@end
