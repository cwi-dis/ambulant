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
#import "PlaylistAppDelegate.h"

@interface SettingsViewController : UIViewController {
	id <PlaylistViewControllerDelegate> delegate;
	IBOutlet UISwitch* autoCenterSwitch;	
	IBOutlet UISwitch* autoResizeSwitch;	
	IBOutlet UISwitch* nativeRendererSwitch;
	IBOutlet UISegmentedControl* HUDhide;
	IBOutlet UISegmentedControl* HUDtap;
}

- (IBAction) done: (id) sender;
- (BOOL) autoCenter;
- (BOOL) autoResize;
- (BOOL) nativeRenderer;
- (IBAction) playWelcome;
- (IBAction) handleHideChanged;
- (IBAction) handleTapChanged;

@end

