// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#import "PresentationViewController.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

@implementation PresentationViewController

- (NSArray*) get_playlist {
	NSArray* playlist;
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();

	if (isHistory) {
		playlist = prefs->m_history->get_playlist();		
	} else {
		playlist = prefs->m_favorites->get_playlist();
	}
	return playlist;
}

- (BOOL)
isHistory {
	return isHistory;
}

- (void) awakeFromNib
{
	AM_DBG NSLog(@"PresentationViewController awakeFromNib(%p)", self);
	if (presentationsArray == NULL) {
		presentationsArray = [ [ NSMutableArray alloc ] init ];
	}
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	prefs->load_preferences();
	BOOL history = [self.title isEqualToString:@"History"];
	isHistory = history;
	currentIndex = -1; //XXXX should ths be saved in properties ??
	if (isHistory) {
		[delegate setHistoryViewController: self];
	}
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void) viewDidLoad
{
    [super viewDidLoad];
	AM_DBG NSLog(@"PresentationViewController viewDidLoad(%p)", self);
//	self.tableView.rowHeight = 60;
	if (presentationsArray == NULL) {
		presentationsArray = [[NSMutableArray alloc] init];
	}
	[self updatePlaylist];
}

- (void) viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
	[self updatePlaylist];
}

- (IBAction) done:(id)sender
{
	AM_DBG NSLog(@"PresentationViewController done(%p)", self);
	[delegate auxViewControllerDidFinish:self];
}

// Overriden to allow orientations other than the default portrait orientation.
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return [delegate canShowRotatedUIViews];
}


- (void) didReceiveMemoryWarning
{
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void) viewDidUnload
{
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	if (presentationsArray != NULL) {
		[presentationsArray dealloc];
		presentationsArray = NULL;
	}
}

// Customize the number of rows in the table view.
- (NSInteger) tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	AM_DBG NSLog(@"tableView:numberOfRowsInSection(%p) section=%d", self, section);
    return [presentationsArray count];
}

// Customize the appearance of table view cells.
- (UITableViewCell *) tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    static NSString *CellIdentifier = @"PresentationTableCell"; // this name must match Identity field in nib. 
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
		[ [ NSBundle mainBundle ] loadNibNamed: @"PresentationTableCell" owner: self options: NULL ];
		cell = nibLoadedCell;
    }
    
	// Configure the cell.
	try {
		PlaylistItem* aPresentation = [presentationsArray objectAtIndex: indexPath.row ];
		UIImageView* posterView = (UIImageView*) [cell viewWithTag:5]; // tags are assigned in the nib
		posterView.contentMode = UIViewContentModeScaleAspectFit;
		NSData *poster_data = [aPresentation poster_data];
		if (poster_data) {
			posterView.image = [UIImage imageWithData: poster_data];
		} else {
			posterView.image = [UIImage imageNamed: @"DefaultPoster.png"];
		}
		[posterView setNeedsDisplay];
		
		// Set the title
		UILabel* label = (UILabel*) [cell viewWithTag: 1];
		label.text = aPresentation.title;
		
		// Set the duration
		label = (UILabel*) [cell viewWithTag: 2];
		label.text = aPresentation.duration;
		
		// Set the progress
		UIButton *button = (UIButton*)[cell viewWithTag: 6];
		button.hidden = aPresentation.position_node == nil || [aPresentation.position_node isEqualToString:@""];
		
		// Set the author
		label = (UILabel*) [cell viewWithTag: 4];
		label.text = aPresentation.author;
		
		// Set the description
		label = (UILabel*) [cell viewWithTag: 3];
		label.text = aPresentation.description;
	}
	catch (NSException* exception) {
		AM_DBG NSLog(@"cellForRowAtIndexPath:indexPath=%@ exception name=%@ reason=%@", indexPath, [exception name], [exception reason]);
	}
	return cell;
}

// Support row selection in the table view.
- (void) tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if (editingStyle != UITableViewCellEditingStyleNone) {
		return;
	}
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	NSArray* playlist = isHistory ? prefs->m_history->get_playlist() : prefs->m_favorites->get_playlist();
	NSUInteger playlistIndex = indexPath.row;
	currentIndex = playlistIndex;
	PlaylistItem* selectedItem = [playlist objectAtIndex: playlistIndex];
	[delegate playPresentation:selectedItem fromPresentationViewController: self];
}

// Support conditional editing of the table view.
- (BOOL) tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}

// Show editing style button
- (UITableViewCellEditingStyle) tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return editingStyle;
}

// Support editing the table view (deletion only, adding is automatic).
- (void) tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyleArg 
    forRowAtIndexPath:(NSIndexPath *)indexPath
{    
	NSUInteger playlistIndex = indexPath.row;
    if (editingStyleArg == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source.
		ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		ambulant::Playlist* playlist = isHistory ? prefs->m_history : prefs->m_favorites;
		
		playlist->remove_playlist_item_at_index(playlistIndex);
		[presentationsArray removeObjectAtIndex: indexPath.row];
		[tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath]
            withRowAnimation:UITableViewRowAnimationMiddle];
		prefs->save_preferences();
// correct currentIndex for this deletion s.t. selectNextPresentation will select the same presentation as before
		if (playlistIndex <= currentIndex) {
			currentIndex--;
		}
		if (currentIndex < -1) {
			currentIndex = -1;
		}
 	}   
    else if (editingStyleArg == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
		[self insertCurrentItemAtIndexPath: indexPath];
    } 
}

- (IBAction) toggleEditMode
{
	switch (editingStyle) {
    case UITableViewCellEditingStyleNone:
        editingStyle = UITableViewCellEditingStyleDelete;
        break;
    default:
        editingStyle = UITableViewCellEditingStyleNone;
        break;
	}
	[[self tableView] setEditing: editingStyle != UITableViewCellEditingStyleNone animated: YES];
}

// Support re-arranging table items (not for History)
- (BOOL) tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be moveable.
	return not isHistory;
}

// Implement re-arranging table items (not for History)
- (void) tableView:(UITableView *)tableView moveRowAtIndexPath: (NSIndexPath*) fromIndexPath toIndexPath: (NSIndexPath*) toIndexPath
{
	NSUInteger fromPlaylistIndex = fromIndexPath.row, toPlaylistIndex = toIndexPath.row;
	AM_DBG NSLog(@"moveRowAtIndexPath: %d toIndexPath: %d", fromPlaylistIndex, toPlaylistIndex);
	if (fromPlaylistIndex == toPlaylistIndex) {
		return;
	}
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	ambulant::Playlist* playlist = isHistory ? prefs->m_history : prefs->m_favorites;
	PlaylistItem* selectedItem = [playlist->get_playlist() objectAtIndex: fromPlaylistIndex];
	playlist->insert_item_at_index(selectedItem, toPlaylistIndex);
	if (toPlaylistIndex > fromPlaylistIndex) {
		playlist->remove_playlist_item_at_index(fromPlaylistIndex);
	} else {
		playlist->remove_playlist_item_at_index(fromPlaylistIndex + 1);
	}
}

- (void) insertCurrentItemAtIndexPath: (NSIndexPath*) indexPath
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (not isHistory) {
		NSInteger playlistIndex = indexPath != NULL ? indexPath.row : -1;
		ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		ambulant::Playlist* playlist = prefs->m_favorites;
		PlaylistItem* new_item = prefs->m_history->get_last_item();
		// Check if we have 'new_item' already in the playlist; if so ignore
		AM_DBG NSLog(@"new_item.url=%p: %@", new_item.url, new_item.url != NULL ? [new_item.url absoluteString]:@"<nil>");
		BOOL found = NO;
		BOOL* found_ref = &found;
		NSArray* items = playlist->get_playlist();
		[items enumerateObjectsWithOptions: nil
            usingBlock:
            ^(id obj, NSUInteger idx, BOOL *stop)
            {
                PlaylistItem* item = (PlaylistItem*) obj;
                AM_DBG NSLog(@"item.url=%p: %@", item.url, item.url != NULL ? [item.url absoluteString]:@"<nil>");
                if ([new_item.url isEqual: (id) item.url]) {
                    *found_ref = YES;
                }
            }
        ];
		if (found) {
			return;
		}
		playlist->insert_item_at_index(new_item, playlistIndex);
		if (playlistIndex < 0 || [presentationsArray count] == 0) {
			[presentationsArray addObject: new_item];
			[self.tableView reloadData];
		} else {
			[presentationsArray insertObject: new_item atIndex: indexPath.row ];
			NSIndexPath* updatedPath = [ NSIndexPath indexPathForRow:indexPath.row inSection: 0 ];
			AM_DBG NSLog(@"updatedPath.row=%d",updatedPath.row);
			NSMutableArray* updatedPaths = [ [NSMutableArray alloc] init ];
			[updatedPaths addObject: updatedPath];
			[self.tableView insertRowsAtIndexPaths: updatedPaths withRowAnimation: UITableViewRowAnimationMiddle]; //UITableViewRowAnimationMiddle ];
			[updatedPaths release];
		}
	}
	[pool release];
}

- (void) updatePlaylist {

	NSArray* playlist = [self get_playlist];
	
	if ([playlist count] > [presentationsArray count]) {
		currentIndex++; //assume insert at 0 occurred
	}
	[presentationsArray removeAllObjects];
	// populate the table view with objects in 'playlist'
	[playlist enumerateObjectsWithOptions: nil
        usingBlock:
        ^(id obj, NSUInteger idx, BOOL *stop)
        {
            PlaylistItem* item = (PlaylistItem*) obj;
            [presentationsArray addObject: item];
        }
    ];
	[[self tableView] reloadData];
}

- (void) selectNextPresentation
{
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	NSArray* playlist = isHistory ? prefs->m_history->get_playlist() : prefs->m_favorites->get_playlist();
	NSUInteger playlistIndex = ++currentIndex;
	if (currentIndex >= [playlist count]) {
		playlistIndex = currentIndex = 0;
	}
	PlaylistItem* selectedItem = [playlist objectAtIndex: playlistIndex];
	[delegate playPresentation:selectedItem fromPresentationViewController: self];
}
	
- (void) viewWillDisappear:(BOOL)animated
{
	if (editingStyle != UITableViewCellEditingStyleNone) {
		[self toggleEditMode];
	}
}

- (void) dealloc
{
   [super dealloc];
}

@end
