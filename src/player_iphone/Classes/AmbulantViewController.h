//
//  AmbulantViewController.h
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "PlaylistViewController.h"
#import "ambulant/common/embedder.h"
#import "ambulant/net/url.h"
#import "ambulant/gui/cg/cg_gui.h"
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
	IBOutlet id view;
	IBOutlet AmbulantView* playerView;
	IBOutlet UIView* interactionView;
	IBOutlet UITextField* URLEntryField;
	CGRect originalPlayerViewFrame, originalInteractionViewFrame;
	NSString* playURL;
	NSString* linkURL;
	BOOL keyboardIsShown;
	UIDeviceOrientation currentOrientation;
	BOOL autoCenter;
	BOOL autoResize;
	BOOL play_active;
}
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation) interfaceOrientation;
- (BOOL) isSupportedOrientation: (UIDeviceOrientation) orientation;
- (IBAction) handlePlayTapped;
- (IBAction) handlePauseTapped;
- (IBAction) handleStopTapped;
- (IBAction) handleURLEntered;
- (IBAction) handlePanGesture:(UIPanGestureRecognizer *)sender;
- (IBAction) handlePinchGesture:(UIGestureRecognizer *)sender;
- (IBAction) showPlaylist:(id)sender;
- (IBAction) close:(id) str;
- (void) pause;

@property(nonatomic,retain) IBOutlet UITextField* URLEntryField;
@property(nonatomic,retain)	IBOutlet AmbulantView* playerView;
@property(nonatomic) mainloop* myMainloop;
@property(nonatomic,retain) NSString* linkURL, *playURL;
@property(nonatomic,retain)	IBOutlet UIView* interactionView;
@property(nonatomic) CGRect originalPlayerViewFrame, originalInteractionViewFrame;
@property(nonatomic) BOOL keyboardIsShown, autoCenter, autoResize, play_active;
@property(nonatomic) UIDeviceOrientation currentOrientation;

@end

