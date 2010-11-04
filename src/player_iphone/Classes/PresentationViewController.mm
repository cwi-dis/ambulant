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

@implementation PresentationViewController

@synthesize delegate,naviationController,nibLoadedCell;

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
	naviationController = [[UINavigationController alloc]initWithRootViewController: self];
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
			aPresentation.duration = [item ns_dur];
			aPresentation.description = [item ns_description];
			[ presentationsArray addObject:aPresentation ];
			[ aPresentation release ];								  
		}];
	[self presentModalViewController:naviationController animated:YES];
}

- (IBAction) done:(id)sender
{
	[self.delegate presentationViewControllerDidFinish:self];
//	[self.delegate done:self];
}


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

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
	UILabel* label = (UILabel*) [ cell viewWithTag: 1];
//	label.poster = aPresentation.poster;
	label.text = aPresentation.title;
	label = (UILabel*) [ cell viewWithTag: 2];
	label.text = aPresentation.duration;
	//	[ duration release ];
	label = (UILabel*) [ cell viewWithTag: 3];
	label.text = aPresentation.description;
	return cell;
}

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
	NSLog(@"tabBar:0x%x didSelectItem:0x%x idx=%d", tabBar, item, idx);
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
