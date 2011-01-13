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
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	if (prefs->m_hud_auto_hide) {
		HUDhide.selectedSegmentIndex = 0;
	} else {
		HUDhide.selectedSegmentIndex = 1;
	}
	if (prefs->m_hud_short_tap) {
		HUDtap.selectedSegmentIndex = 1;
	} else {
		HUDtap.selectedSegmentIndex = 0;
	}	
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

- (IBAction) handleHideChanged {
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	switch (HUDhide.selectedSegmentIndex) {
		default:
		case 0:
			prefs->m_hud_auto_hide = true;
			break;
		case 1:
			prefs->m_hud_auto_hide = false;			
			break;
	}
}

- (IBAction) handleTapChanged {
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	switch (HUDtap.selectedSegmentIndex) {
		default:
		case 0:
			prefs->m_hud_short_tap = false;
			break;
		case 1:
			prefs->m_hud_short_tap = true;			
			break;
	}
}

- (IBAction) playWelcome {
	[delegate playWelcome: NULL];
	[delegate auxViewControllerDidFinish:self];	
}

- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation {
	return [delegate canShowRotatedUIViews];
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
