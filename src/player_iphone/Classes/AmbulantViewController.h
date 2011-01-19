//
//  AmbulantViewController.h
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright CWI 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AmbulantAppDelegate.h"
#import "SettingsViewController.h"
#import "Presentation.h"
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

@interface AmbulantViewController : UIViewController 
				<UITextFieldDelegate> {
	document_embedder *embedder;    // Our class to handle inter-SMIL-document commands.
	mainloop *myMainloop;   // Controller object for the SMIL player
	IBOutlet AmbulantContainerView* view; // our main view, contains playerView and interactionView
	IBOutlet AmbulantView* playerView;
	IBOutlet UIView* interactionView;
	IBOutlet AmbulantAppDelegate* delegate; // Our higher-level controller
	IBOutlet UIButton* playPauseButton;
	NSString* currentURL;      // The document that is currently playing (or will play shortly)
	UIDeviceOrientation currentOrientation; // Current orientation of playback window
}

// Lifecycle
- (void) awakeFromNib;
- (void) initGestures;
- (void) viewDidLoad;
- (void) viewWillAppear:(BOOL)animated;
- (void) viewDidAppear:(BOOL)animated;
- (void)viewDidUnload;
- (void) willTerminate;

// Player control
- (void) doPlayURL: (NSString*) theUrl fromNode: (NSString*) ns_node_repr;
- (bool) canPlay;
- (PlaylistItem*) currentItem;
- (void) pause;
- (void) play;

// View control
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation) interfaceOrientation;
- (BOOL) isSupportedOrientation: (UIDeviceOrientation) orientation;
- (void) orientationChanged:(NSNotification *)notification;
- (void) showInteractionView: (BOOL) on;
- (void) autoHideInteractionView;

// User interaction through gestures
- (void) handleDoubleTapGesture:(UITapGestureRecognizer*) sender;
- (void) handleSingleTapGesture:(UITapGestureRecognizer*) sender;
- (void) handleLongPressGesture:(UILongPressGestureRecognizer *)sender;
- (void) handlePanGesture:(UIPanGestureRecognizer*) sender;
- (void) handlePinchGesture:(UIGestureRecognizer*) sender;
- (void) adjustAnchorPointForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer;

// User interaction through buttons
- (IBAction) doRestartTapped: (id)sender;
- (IBAction) doPlayOrPauseTapped: (id)sender;
- (IBAction) doNextItem: (id)sender;
- (IBAction) doAddFavorite:(id)sender;
- (IBAction) doPlaylists:(id)sender;

// Notifications from other views, etc.
- (void) settingsHaveChanged;
- (void) didReceiveMemoryWarning;

@end
