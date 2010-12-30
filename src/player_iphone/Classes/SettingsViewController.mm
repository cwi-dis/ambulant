//
//  SettingsViewController.m
//
//  Created by Kees Blom on 7/31/10.
//  Copyright CWI 2010. All rights reserved.
//

#import "SettingsViewController.h"
#import "AmbulantViewController.h"
#include "ambulant/common/preferences.h"

@implementation SettingsViewController

//@synthesize delegate, autoCenterSwitch, autoResizeSwitch, nativeRendererSwitch;

- (void)viewDidLoad {
    [super viewDidLoad];
	// initialize to values taken from AmbulantView
	autoCenterSwitch.on = delegate.autoCenter;
	autoResizeSwitch.on = delegate.autoResize;
	nativeRendererSwitch.on = delegate.nativeRenderer;	
}

- (BOOL) autoCenter {
	return autoCenterSwitch.on;
}

- (BOOL) autoResize {
	return autoResizeSwitch.on;
}

- (BOOL) nativeRenderer {
	return nativeRendererSwitch.on;
}

- (IBAction) done:(id)sender {
	[delegate settingsHaveChanged:self];
	[delegate auxViewControllerDidFinish:self];	
}

- (IBAction) playWelcome {
    // XXXJACK send to app delegate
	[delegate playPresentation:@"Welcome" fromPresentationViewController: NULL];
}

- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation {
	return YES;
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}


- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	// Return YES for supported orientations
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/


- (void)dealloc {
    [super dealloc];
}


@end
