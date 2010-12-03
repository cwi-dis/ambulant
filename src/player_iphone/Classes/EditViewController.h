//
//  EditViewController.h
//  EditView
//
//  Created by Kees Blom on 01/12/10.
//  Copyright Stg.CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AmbulantAppDelegate.h"
#import "PlaylistAppDelegate.h"

@protocol PlaylistControllerDelegate;

@interface EditViewController : UIViewController  <UITextFieldDelegate> {
	id <PlaylistViewControllerDelegate> delegate;
	IBOutlet UITextField* textField;
}
- (IBAction) cancel: (id) sender;
- (IBAction) done: (id) sender;

@property(nonatomic, assign) id <PlaylistViewControllerDelegate> delegate;
@property(nonatomic, retain) IBOutlet UITextField* textField;
@end
