//
//  PresentationViewController.m
//  PresentationView
//
//  Created by Kees Blom on 10/31/10.
//  Copyright Stg.CWI 2010. All rights reserved.
//

#import "PresentationViewController.h"
#import "iOSpreferences.h"
#import "Presentation.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

@implementation PresentationViewController

@synthesize delegate,nibLoadedCell;

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
	NSArray* history = prefs->m_history->get_playlist();
	// populate the table view with objects in the history, last one at top
	[history enumerateObjectsWithOptions: NSEnumerationReverse 
							  usingBlock:
		^(id obj, NSUInteger idx, BOOL *stop)
		{
			PlaylistItem* item = (PlaylistItem*) obj;
			Presentation* aPresentation = [ [ Presentation alloc ] init ];
			aPresentation.title = [item ns_title];
			aPresentation.poster = [item cg_image];
			aPresentation.duration = [item ns_dur];
			aPresentation.description = [item ns_description];
			[ presentationsArray addObject:aPresentation ];							  
		}];
//	[pool release];
}

- (IBAction) done:(id)sender
{
	AM_DBG NSLog(@"PresentationViewController done(0x%x)", self);
	[self.delegate playlistViewControllerDidFinish:self];
//	[self.delegate done:self];
}

// Overriden to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return YES;
}


- (void)didReceiveMemoryWarning
{
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload
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
	NSArray* history = ambulant::iOSpreferences::get_preferences()->m_history->get_playlist();
	PlaylistItem* selectedItem = [history objectAtIndex: [history count] - 1 - indexPath.row];
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
		ambulant::Playlist* history = ambulant::iOSpreferences::get_preferences()->m_history;
		
		history->remove_playlist_item_at_index([history->get_playlist() count] - 1 - indexPath.row);
		[presentationsArray removeObjectAtIndex: indexPath.row];
		[tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
 	}   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }   
}
#ifdef TBD: adapt following methods 
#endif//TBD
   
// Support for TabBarItem selection
- (void)
tabBar:(UITabBar *)tabBar didSelectItem:(UITabBarItem *)item
{
	NSEnumerator* itemsEnum = [[tabBar items] objectEnumerator];
	UITabBarItem* tbi = NULL;
	NSInteger idx = 0;
	while ((tbi = [itemsEnum nextObject]) != NULL) {
		if (tbi == item) {
			break;
		}
		idx++;
	}
	if (tbi == NULL) {
		idx = -999;
	}
	AM_DBG NSLog(@"tabBar:0x%x didSelectItem:0x%x idx=%d", tabBar, item, idx);
	switch (idx) {
		case 0:
			[self.delegate done:self];
			break;
		default:
			break;
	}
	return;
}

- (void)dealloc
{
    [super dealloc];
}

@end
