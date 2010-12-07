//
//  EditViewController.m
//  EditView
//
//  Created by Kees Blom on 01/12/10.
//  Copyright Stg.CWI 2010. All rights reserved.
//

#import "EditViewController.h"
#import "AmbulantViewController.h"
#import "iOSpreferences.h"

#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

@implementation EditViewController

@synthesize delegate, textField;

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
	AM_DBG NSLog(@"EditViewController viewDidLoad(0x%x)", self);
	NSString* url = [self.delegate playURL];
	if (url != NULL) {
		self.textField.text = [NSString stringWithString: url];
	}
//	[pool release];
}

// Allow orientations other than the default portrait orientation.
- (BOOL)
shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return YES;
}

- (IBAction)
done:(id)sender
{
	AM_DBG NSLog(@"EditViewController done(0x%x)", self);
	
	[self.delegate playPresentation:self.textField.text];
	[self cancel: sender];
	
}

- (IBAction)
cancel:(id)sender
{
	AM_DBG NSLog(@"EditViewController done(0x%x)", self);
	
	[self.delegate playlistViewControllerDidFinish:self];
}

// From UITextFieldDelegate
- (BOOL)
textFieldShouldReturn:(UITextField*) textField {
	AM_DBG NSLog(@"EditViewController viewDidLoad(0x%x): %@", self, self.textField.text);
	[self.textField resignFirstResponder];
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


- (void)
dealloc
{
	[textField release];
    [super dealloc];
}

@end
