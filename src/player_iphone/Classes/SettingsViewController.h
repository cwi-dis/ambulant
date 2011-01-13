//
//  SettingsViewController.h
//  Settings
//
//  Created by Kees Blom on 7/31/10.
//  Copyright CWI 2010. All rights reserved.
//

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

//@property(nonatomic,assign) id <PlaylistViewControllerDelegate> delegate;
//@property(nonatomic,retain) IBOutlet UISwitch* autoCenterSwitch;
//@property(nonatomic,retain) IBOutlet UISwitch* autoResizeSwitch;
//@property(nonatomic,retain) IBOutlet UISwitch* nativeRendererSwitch;

@end

