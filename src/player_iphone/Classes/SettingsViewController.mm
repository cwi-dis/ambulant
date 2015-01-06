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

#import "SettingsViewController.h"
#import "AmbulantViewController.h"
#include "ambulant/common/preferences.h"

@implementation SettingsViewController

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

- (void)dealloc {
    [super dealloc];
}
@end
