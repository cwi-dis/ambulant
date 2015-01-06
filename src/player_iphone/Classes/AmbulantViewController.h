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
#import <MessageUI/MessageUI.h>
#import <MessageUI/MFMailComposeViewController.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import "AmbulantAppDelegate.h"
#import "SettingsViewController.h"
#import "PlaylistItem.h"
#import "PresentationViewController.h"
#import "ambulant/common/embedder.h"
#import "ambulant/net/url.h"
#import "ambulant/gui/cg/cg_gui.h"
#import "iOSpreferences.h"
#import "mainloop.h"

//
// Implements inter-document actions in SMIL, such as opening new presentations.
//
class document_embedder : public ambulant::common::embedder {
  public:
	document_embedder(id mydocument)
	:   m_mydocument(mydocument) {}
	
	// common:: embedder interface
	void show_file(const ambulant::net::url& href);
	void close(ambulant::common::player *p);
	void open(ambulant::net::url newdoc, bool start, ambulant::common::player *old=NULL);
  private:
	id m_mydocument;
};

@interface AmbulantContainerView : UIView {
}
@end

enum ZoomState {
    zoomUnknown,    // Only before first setting, will load from preferences
    zoomFillScreen,
    zoomNaturalSize,
//  zoomRegion,
    zoomUser
};

@interface AmbulantScalerView : UIView {
    ZoomState zoomState;  // What sort of zooming we currently use
	bool anchorTopLeft;	// Only for zoomFillScreen and zoomNaturalSize: anchorpoint is not center
    CGPoint translation_origin; // During translation: point of origin of subwindow
    CGAffineTransform zoom_transform; // During zoom: original scale factor
}
- (void) adaptDisplayAfterRotation;
- (void) zoomWithScale: (float) scale inState: (UIGestureRecognizerState) state;
- (void) autoZoomAtPoint: (CGPoint) point;
- (void) translateWithPoint: (CGPoint) point inState: (UIGestureRecognizerState) state;
- (void) recomputeZoom;
@end

@interface AmbulantViewController : UIViewController 
		<UITextFieldDelegate, 
		MFMailComposeViewControllerDelegate,
		UIActionSheetDelegate>
{
	document_embedder *embedder;    // Our class to handle inter-SMIL-document commands.
	mainloop *myMainloop;   // Controller object for the SMIL player
//	IBOutlet AmbulantContainerView* view; // our main view, contains scalerView and interactionView
    IBOutlet AmbulantScalerView* scalerView; // The zoom/pan view, contains playerView
	IBOutlet AmbulantView* playerView;
	IBOutlet UIView* interactionView;
	IBOutlet UIView* finishedView;
    IBOutlet UIImageView* finishedViewImage;
	IBOutlet AmbulantAppDelegate* delegate; // Our higher-level controller
	IBOutlet UIButton* playPauseButton;
	IBOutlet UIButton* nextPresentationButton;
	NSString* currentURL;      // The document that is currently playing (or will play shortly)
	UIDeviceOrientation currentOrientation; // Current orientation of playback window
    BOOL is_visible;    // Set by viewDidAppear, cleared by viewWillDisappear
}

// Lifecycle
- (void) awakeFromNib;
- (void) initGestures;
- (void) viewDidLoad;
- (void) viewWillAppear:(BOOL)animated;
- (void) viewDidAppear:(BOOL)animated;
- (void) viewWillDisappear:(BOOL)animated;
- (void) viewDidUnload;
- (void) willTerminate;

// Player control
- (bool) hasNetwork;
- (void) doPlayURL: (NSString*) theUrl fromNode: (NSString*) ns_node_repr;
- (bool) canPlay;
- (PlaylistItem*) currentItem;
- (void) pause;
- (void) play;
- (void) stopped;

// View control
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation) interfaceOrientation; - (BOOL)shouldAutorotate; // iOS < 6
//- (BOOL)shouldAutorotate; // iOS >= 6
//- (NSUInteger)supportedInterfaceOrientations; // iOS >= 6
- (BOOL) isSupportedOrientation: (UIDeviceOrientation) orientation;
- (void) orientationChanged:(NSNotification *)notification;
- (void) showInteractionView: (BOOL) on;
- (void) showFinishedView: (BOOL) on;
- (void) autoHideInteractionView;

// User interaction through gestures
- (IBAction) handleDoubleTapGesture:(UITapGestureRecognizer*) sender;
- (IBAction) showHUDGesture:(UITapGestureRecognizer*) sender;
- (IBAction) selectPointGesture:(UILongPressGestureRecognizer *)sender;
- (IBAction) handlePanGesture:(UIPanGestureRecognizer*) sender;
- (IBAction) handlePinchGesture:(UIGestureRecognizer*) sender;
- (IBAction) adjustAnchorPointForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer;

// User interaction through buttons
- (IBAction) doRestartTapped: (id)sender;
- (IBAction) doPlayOrPauseTapped: (id)sender;
- (IBAction) doNextItem: (id)sender;
- (IBAction) doAddFavorite:(id)sender;
- (IBAction) doPlaylists:(id)sender;

// Delegates and such
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex;
- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error ;


// Notifications from other views, etc.
- (void) settingsHaveChanged;
- (void) didReceiveMemoryWarning;

@end
