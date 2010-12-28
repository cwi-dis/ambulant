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

class document_embedder : public ambulant::common::embedder {
public:
	document_embedder(id mydocument)
	:   m_mydocument(mydocument) {}
	
	// common:: embedder interface
	void show_file(const ambulant::net::url& href);
	void close(ambulant::common::player *p);
	void open(ambulant::net::url newdoc, bool start, ambulant::common::player *old=NULL);
#ifdef WITH_OVERLAY_WINDOW
	bool aux_open(const ambulant::net::url& href);
#endif
private:
	id m_mydocument;
};

@interface AmbulantViewController : UIViewController 
				<UITextFieldDelegate, PlaylistViewControllerDelegate> {
	document_embedder *embedder;
	mainloop *myMainloop;
	IBOutlet id view; // main view, nib-connected to AmbulantView (in cg_gui)
	IBOutlet AmbulantAppDelegate* delegate;
	IBOutlet AmbulantView* playerView;
	IBOutlet UIView* interactionView;
	IBOutlet UIButton* playPauseButton;
    IBOutlet UITabBar* modeBar; // XXXJACK is this used?
	PresentationViewController* currentPresentationViewController;
    PresentationViewController* historyViewController;
	NSString* currentURL;      // The document that is currently playing (or will play shortly)
	UIDeviceOrientation currentOrientation; // Current orientation of playback window
	BOOL autoCenter;
	BOOL autoResize;
	BOOL nativeRenderer;
}
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation) interfaceOrientation;
- (BOOL) isSupportedOrientation: (UIDeviceOrientation) orientation;
- (void) handleDoubleTapGesture:(UITapGestureRecognizer*) sender;
- (void) handleTapGesture:(UITapGestureRecognizer*) sender;
- (void) handleLongPressGesture:(UILongPressGestureRecognizer *)sender;
- (void)adjustAnchorPointForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer;
- (void) handlePanGesture:(UIPanGestureRecognizer*) sender;
- (void) handlePinchGesture:(UIGestureRecognizer*) sender;
- (IBAction) handleRestartTapped;
- (IBAction) handlePlayOrPauseTapped;
- (IBAction) playNextItem;
- (IBAction) addFavorites:(id)sender;
- (void) pause;
- (void) play;
- (PlaylistItem*) currentItem;
- (void) doPlayURL: (NSString*) theUrl fromNode: (NSString*) ns_node_repr;
- (void) showInteractionView: (BOOL) on;
- (void) autoHideInteractionView;
- (void) willTerminate;

//@property(nonatomic,retain) IBOutlet AmbulantAppDelegate* delegate;
//@property(nonatomic,retain) PresentationViewController* historyViewController;
//@property(nonatomic,retain)	IBOutlet AmbulantView* playerView;
//@property(nonatomic,retain) IBOutlet UITabBar* modeBar;
//@property(nonatomic) mainloop* myMainloop;
//@property(nonatomic,retain)	IBOutlet UIView* interactionView;
//@property(nonatomic,retain) IBOutlet UIButton* playPauseButton;
@property(readonly) BOOL autoCenter;
@property(readonly) BOOL autoResize;
@property(readonly) BOOL nativeRenderer;
//@property(nonatomic) BOOL play_active;
//@property(nonatomic) UIDeviceOrientation currentOrientation;

@end
