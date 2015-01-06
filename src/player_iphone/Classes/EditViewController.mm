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

#import "EditViewController.h"
#import "AmbulantViewController.h"
#import "iOSpreferences.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

@implementation EditViewController

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)
viewDidLoad
{
    [super viewDidLoad];
	[textField becomeFirstResponder];
	AM_DBG NSLog(@"EditViewController viewDidLoad(0x%@)", self);
}

// Allow orientations other than the default portrait orientation.
- (BOOL)
shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return [delegate canShowRotatedUIViews];
}

- (IBAction)
done:(id)sender
{
	AM_DBG NSLog(@"EditViewController done(0x%@)", self);
    NSString *theUrl = textField.text;
	// Remove leading/trailing whitespace (copy/paste can be a bit
	// finicky on the iPhone)
	theUrl = [theUrl stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
	// Remove an optional "ambulant:" pseudo-protocol
    if ([theUrl hasPrefix: @"ambulant:"])
        theUrl = [theUrl substringFromIndex: 9];
	
	if ( ![theUrl isEqualToString:@""]) [delegate playURL:theUrl];
	[self cancel: sender];
}

- (IBAction)
cancel:(id)sender
{
	AM_DBG NSLog(@"EditViewController done(0x%@)", self);
	[textField resignFirstResponder];
	[delegate auxViewControllerDidFinish:self];
}

// From UITextFieldDelegate
- (BOOL)
textFieldShouldReturn:(UITextField*) sender {
	AM_DBG NSLog(@"EditViewController viewDidLoad(0x%@): %@", self, textField.text);
	[textField resignFirstResponder];
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
