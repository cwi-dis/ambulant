//
//  PresentationViewController.m
//  PresentationView
//
//  Created by Kees Blom on 10/31/10.
//  Copyright Stg.CWI 2010. All rights reserved.
//

#import "PresentationViewController.h"

#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

@implementation PresentationViewController

@synthesize delegate, nibLoadedCell;

/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/

- (Presentation*)
getPresentationFromPlaylistItem: (PlaylistItem*) item {
	Presentation* aPresentation = [ [ Presentation alloc ] init ];
	aPresentation.title = [item ns_title];
	aPresentation.poster = [item cg_image];
	aPresentation.duration = [item ns_dur];
	aPresentation.description = [item ns_description];
	[ presentationsArray addObject:aPresentation ];	
	return aPresentation;
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)
viewDidLoad
{
    [super viewDidLoad];
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AM_DBG NSLog(@"PresentationViewController viewDidLoad(0x%x)", self);
	presentationsArray = [ [ NSMutableArray alloc ] init ];
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	prefs->load_preferences();
	NSArray* playlist;
	BOOL favorites = [self.title isEqualToString:@"Favorites"];
	isFavorites = favorites;
	if (favorites) {
		playlist = prefs->m_favorites->get_playlist();
		
	} else {
		playlist = prefs->m_history->get_playlist();
	}
	// populate the table view with objects in the list
	[playlist enumerateObjectsWithOptions: nil usingBlock:
		^(id obj, NSUInteger idx, BOOL *stop)
		{
			PlaylistItem* item = (PlaylistItem*) obj;
            [ presentationsArray addObject:[self getPresentationFromPlaylistItem: item]];
		}];
//	[pool release];
}
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
	// update table view if a presentation was added
	if (newPresentation != nil) {
		NSIndexPath* updatedPath = [ NSIndexPath indexPathForRow: [ presentationsArray indexOfObject: newPresentation] inSection: 0 ];
		NSArray* updatedPaths = [ NSArray arrayWithObject:updatedPath ];
		[ self.tableView reloadRowsAtIndexPaths: updatedPaths withRowAnimation: UITableViewRowAnimationMiddle ];
		newPresentation = nil;
	}
}

- (IBAction)
done:(id)sender
{
	AM_DBG NSLog(@"PresentationViewController done(0x%x)", self);
	[self.delegate playlistViewControllerDidFinish:self];
//	[self.delegate done:self];
}

// Overriden to allow orientations other than the default portrait orientation.
- (BOOL)
shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return YES;
}


- (void)
didReceiveMemoryWarning
{
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)
viewDidUnload
{
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}

// Customize the number of rows in the table view.
- (NSInteger)
tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	AM_DBG NSLog(@"tableView:0x%x numberOfRowsInSection(0x%x) section=%d", self, section);
    return [ presentationsArray count ];
}

// Customize the appearance of table view cells.
- (UITableViewCell *)
tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
//      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
		[ [ NSBundle mainBundle ] loadNibNamed: @"PresentationTableCell" owner: self options: NULL ];
		cell = nibLoadedCell;
    }
    
	// Configure the cell.
	Presentation* aPresentation = [ presentationsArray objectAtIndex: indexPath.row ];
	UIImageView* posterView = (UIImageView*) [ cell viewWithTag:0]; // tags are assigned in the nib
	posterView.image = [UIImage imageWithCGImage:(CGImageRef) aPresentation.poster];
	[posterView setNeedsDisplay];
	UILabel* label = (UILabel*) [ cell viewWithTag: 1];
	label.text = aPresentation.title;
	label = (UILabel*) [ cell viewWithTag: 2];
	label.text = aPresentation.duration;
//	[ duration release ];
	label = (UILabel*) [ cell viewWithTag: 3];
	label.text = aPresentation.description;
	return cell;
}

// Support row selection in the table view.
- (void)
tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	NSArray* playlist = isFavorites ? prefs->m_favorites->get_playlist() : prefs->m_history->get_playlist();
	PlaylistItem* selectedItem = [playlist objectAtIndex: indexPath.row];
	[self.delegate playPresentation:[[selectedItem ns_url] absoluteString]];
}

// Support conditional editing of the table view.
- (BOOL)
tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}

// Support editing the table view (deletion only, adding is automatic).
- (void)
tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source.
		ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		ambulant::Playlist* playlist = isFavorites ? prefs->m_favorites : prefs->m_history;
		
		playlist->remove_playlist_item_at_index(indexPath.row);
		[presentationsArray removeObjectAtIndex: indexPath.row];
		[tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
		prefs->save_preferences();
 	}   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }   
}

- (IBAction)
addCurrentItem;
{
	if (isFavorites) {
		ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		ambulant::Playlist* playlist = prefs->m_favorites;
		PlaylistItem* new_item = prefs->m_history->get_last_item();
		playlist->add_item(new_item);
		[ presentationsArray insertObject:new_item atIndex: 0];
		newPresentation = [self getPresentationFromPlaylistItem: new_item];
	}
}


- (void)dealloc
{
    [super dealloc];
}

@end
