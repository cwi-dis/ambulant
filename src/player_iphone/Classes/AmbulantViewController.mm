//
//  AmbulantViewController.mm
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright CWI 2010. All rights reserved.
//

#import "AmbulantViewController.h"
#import "AmbulantAppDelegate.h"
#import "SettingsViewController.h"
#import <QuartzCore/QuartzCore.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#pragma mark -
#pragma mark document_embedder

void
document_embedder::show_file(const ambulant::net::url& href)
{
	AM_DBG ambulant::lib::logger::get_logger()->trace("document_embedder::show_file(0x%x) href=%s", this, href.get_url().c_str());
	document_embedder::open(href, true, NULL);
}

void
document_embedder::close(ambulant::common::player *p)
{
	AM_DBG ambulant::lib::logger::get_logger()->trace("document_embedder::close(0x%x) player=%0x%x", this, p);
	[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
}

void
document_embedder::open(ambulant::net::url newdoc, bool start, ambulant::common::player *old)
{
	AM_DBG ambulant::lib::logger::get_logger()->trace("document_embedder::open(0x%x) new_doc=%s start=%d old_player=%0x%x", this,  newdoc.get_url().c_str(), start, old);

	if (old) {
		AM_DBG NSLog(@"performSelectorOnMainThread: close: on 0x%x", (void*)m_mydocument);
		[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *str_url = [NSString stringWithUTF8String: newdoc.get_url().c_str()];
	id appDelegate = [[UIApplication sharedApplication] delegate];
	[appDelegate performSelectorOnMainThread: @selector(openWebLink:)
							   withObject: str_url	waitUntilDone: NO];
	
	[pool release];
}

@implementation AmbulantViewController

#pragma mark -
#pragma mark Lifecycle

- (void) awakeFromNib
{
    AM_DBG NSLog(@"AmbulantViewController viewDidLoad(0x%x)", self);

	// prepare to react when device is rotated
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter]
        addObserver:self
        selector:@selector(orientationChanged:)
        name:UIDeviceOrientationDidChangeNotification
        object: nil];
	embedder = new document_embedder(self);
}

- (void) initGestures
{
    assert(playerView);
	// prepare to react on "double tap" gesture (select object in playerView with 1 finger tap)
	UITapGestureRecognizer *doubleTapGesture = [[UITapGestureRecognizer alloc]
								initWithTarget:self action:@selector(handleDoubleTapGesture:)];
	doubleTapGesture.numberOfTapsRequired = 2;
	[playerView addGestureRecognizer:doubleTapGesture];
    [doubleTapGesture release];

	// prepare to react on "tap" gesture (select object in playerView with 1 finger tap)
	UITapGestureRecognizer *singleTapGesture = [[UITapGestureRecognizer alloc]
								initWithTarget:self action:@selector(handleSingleTapGesture:)];
	[playerView addGestureRecognizer:singleTapGesture];	
    [singleTapGesture release];
	// do not also errnoneously recognize a single tap when a double tap is recognized
	[singleTapGesture requireGestureRecognizerToFail:doubleTapGesture];	
	
	// prepare to react on "longPress" gesture (hold finger in one spot, longer than 0.4 sec.)
    UILongPressGestureRecognizer *longPressGesture = [[UILongPressGestureRecognizer alloc]
								initWithTarget:self action:@selector(handleLongPressGesture:)];
    [playerView addGestureRecognizer:longPressGesture];
    [longPressGesture release];

	// prepare to react on "pinch" gesture (zoom playerView with 2 fingers)
	UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc]
								initWithTarget:self action:@selector(handlePinchGesture:)];
	[playerView addGestureRecognizer:pinchGesture];
    [pinchGesture release];

	// prepare to react on "pan" gesture (move playerView with one finger)
    UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc]
								initWithTarget:self action:@selector(handlePanGesture:)];
    [playerView addGestureRecognizer:panGesture];
    [panGesture release];
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
// - install gesture recognizers
- (void) viewDidLoad {
	AM_DBG NSLog(@"AmbulantViewController viewDidLoad(0x%x)", self);
    [super viewDidLoad];
    [self initGestures];
}

- (void)viewWillAppear:(BOOL)animated
{
	/*AM_DBG*/ NSLog(@"AmbulantViewController viewWillAppear(0x%x)", self);
}

- (void)viewDidAppear:(BOOL)animated
{
	/*AM_DBG*/ NSLog(@"AmbulantViewController viewDidAppear(0x%x)", self);
    [self play];
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	AM_DBG NSLog(@"AmbulantViewController viewDidUnLoad:self=0x%x", self);
}

- (void) willTerminate
{
	if (myMainloop)
		delete myMainloop;
    myMainloop = NULL;
	[playerView release];
    playerView = nil;
	if (currentURL)
		[currentURL release];
    currentURL = nil;
}

- (void)dealloc {
	AM_DBG NSLog(@"AmbulantViewController dealloc:self=0x%x", self);
    [super dealloc];
	if (myMainloop)
		delete myMainloop;
	[playerView release];
	if (currentURL)
		[currentURL release];
}

#pragma mark -
#pragma mark Player control

// create a new instance of the smil player
- (void) doPlayURL: (NSString*) theUrl fromNode: (NSString*) ns_node_repr {
    if (theUrl) {
        if (currentURL) [currentURL release];
        currentURL = [theUrl retain];
    }
	AM_DBG ambulant::lib::logger::get_logger()->trace("AmbulantViewController doPlayURL(0x%x): url=%s ns_node_repr=%s", self, currentURL? [ currentURL UTF8String]: "NULL", ns_node_repr? [ns_node_repr UTF8String] : "NULL");
	if (myMainloop != NULL) {
		myMainloop->stop();
		delete myMainloop;
        
	}
    if (!currentURL) return;
    if (!playerView) {
        (void)self.view; // This loads the view
    }
    assert(currentURL);
    assert(playerView);
    assert(embedder);	
	myMainloop = new mainloop([currentURL UTF8String], playerView, embedder);	
	if (myMainloop) {
		if (ns_node_repr != NULL) {
			std::string node_repr = [ns_node_repr UTF8String];
			myMainloop->goto_node_repr(node_repr);
		}
		[self showInteractionView: NO];
//		[self play]; // This will be done in viewDidAppear
	}
}

- (bool) canPlay
{
    return myMainloop != NULL;
}

- (void) pause {
	AM_DBG NSLog(@"AmbulantViewController pause(0x%x)", self);
	if (myMainloop) {
		myMainloop->pause();
		UIImage* playImage = [UIImage imageNamed: @"Play_iPhone.png"];
	   [playPauseButton setImage:playImage forState:UIControlStateNormal];
	}
}

- (void) play {
	AM_DBG NSLog(@"AmbulantViewController play(0x%x)", self);
	if (myMainloop) {
		myMainloop->play();
		UIImage* pauseImage = [UIImage imageNamed: @"Pause_iPhone.png"];
		[playPauseButton setImage:pauseImage forState:UIControlStateNormal];
	}
}

- (PlaylistItem*) currentItem {
	AM_DBG NSLog(@"AmbulantViewController currentItem(0x%x)", self);
	if (myMainloop) {
		return myMainloop->get_current_item();
	}
	return NULL;
}

#pragma mark -
#pragma mark View control

- (BOOL) isSupportedOrientation: (UIDeviceOrientation) orientation {
	AM_DBG NSLog(@"AmbulantViewController isSupportedOrientation(0x%x) orientation=%d", self, orientation);
	return 
		orientation == UIDeviceOrientationPortrait
	||	orientation == UIDeviceOrientationPortraitUpsideDown
	||	orientation == UIDeviceOrientationLandscapeLeft
	||	orientation == UIDeviceOrientationLandscapeRight;
}

/* */
// Override to allow orientations other than the default portrait orientation.
- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation {
	AM_DBG NSLog(@"AmbulantViewController shouldAutorotateToInterfaceOrientation(0x%x): interfaceOrientation=%d", self, interfaceOrientation);
	return [self isSupportedOrientation:(UIDeviceOrientation) interfaceOrientation];
}

// react on device rotation
- (void) orientationChanged:(NSNotification *)notification {
//	AM_DBG NSLog(@"AmbulantViewController orientationChanged(0x%x):notification=%d", self, notification);
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	if (orientation == currentOrientation || ! [self isSupportedOrientation: orientation]) {
		return;
	}
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	currentOrientation = orientation;
	if (playerView != NULL) {
		[playerView adaptDisplayAfterRotation: orientation withAutoCenter: prefs->m_auto_center withAutoResize: prefs->m_auto_resize];
	}
}

// display the Control Panel (as a HUD) at the bottom of the player view 
- (void) showInteractionView: (BOOL) want_show {
	if (want_show && interactionView.hidden) {
		interactionView.hidden = false;
		interactionView.opaque = true;
		ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
		if (prefs->m_hud_auto_hide) {
			[NSObject cancelPreviousPerformRequestsWithTarget: self selector:@selector(autoHideInteractionView) object:nil];
			[self performSelector:@selector(autoHideInteractionView) withObject:nil afterDelay:(NSTimeInterval)5.0];
		}
	} else {
		interactionView.hidden = true;
		interactionView.opaque = false;
        [NSObject cancelPreviousPerformRequestsWithTarget: self selector:@selector(autoHideInteractionView) object:nil];
	}
}

- (void) autoHideInteractionView
{
    interactionView.hidden = true;
    interactionView.opaque = false;
}

#pragma mark -
#pragma mark Gesture interaction

/*	Code derived from Apple's developer documentation "Gesture Recognizers"*/

- (IBAction) handleLongPressGesture:(UILongPressGestureRecognizer *)sender {
	AM_DBG NSLog(@"AmbulantViewController handleLongPressGesture(0x%x): sender=0x%x", self, sender);
	CGPoint location = [sender locationInView:playerView];
	if ( ! [playerView tappedAtPoint:location]) {
//		[delegate showPresentationViews:self];
	}
};

- (IBAction) handleSingleTapGesture:(UITapGestureRecognizer *)sender { // select
	AM_DBG NSLog(@"AmbulantViewController handleSingleTapGesture(0x%x): sender=0x%x", self, sender);
	[self showInteractionView: YES];
}

- (IBAction) handleDoubleTapGesture:(UITapGestureRecognizer *)sender { // select
	AM_DBG NSLog(@"AmbulantViewController handleDoubleTapGesture(0x%x): sender=0x%x", self, sender);
	CGPoint location = [sender locationInView:playerView];
	[playerView autoZoomAtPoint:location];
}

- (void) adjustAnchorPointForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer {
    if (gestureRecognizer.state == UIGestureRecognizerStateBegan) {
        UIView *piece = gestureRecognizer.view;
        CGPoint locationInView = [gestureRecognizer locationInView:piece];
        CGPoint locationInSuperview = [gestureRecognizer locationInView:piece.superview];
        
        piece.layer.anchorPoint = CGPointMake(locationInView.x / piece.bounds.size.width, locationInView.y / piece.bounds.size.height);
        piece.center = locationInSuperview;
    } else if (gestureRecognizer.state == UIGestureRecognizerStateEnded) {
        UIView *piece = gestureRecognizer.view;
        CGPoint centerInView = CGPointMake(piece.bounds.size.width/2, piece.bounds.size.height/2);
        CGPoint centerInSuperview = [piece convertPoint: centerInView toView: piece.superview];
        piece.layer.anchorPoint = CGPointMake(0.5, 0.5);
        piece.center = centerInSuperview;
    }
}

- (IBAction) handlePinchGesture:(UIPinchGestureRecognizer *)sender { // zoom
	AM_DBG NSLog(@"AmbulantViewController handlePinchGesture(0x%x): sender=0x%x", self, sender);
    [self adjustAnchorPointForGestureRecognizer: sender];
	CGFloat factor = [(UIPinchGestureRecognizer *)sender scale];
	[playerView zoomWithScale:factor inState: [sender state]];
}

- (IBAction) handlePanGesture:(UIPanGestureRecognizer *)sender {
	AM_DBG NSLog(@"AmbulantViewController handlePanGesture(0x%x): sender=0x%x", self, sender);
	CGPoint translate = [sender translationInView: playerView.superview];
	[playerView  translateWithPoint: (CGPoint) translate inState: [sender state]];
}

#pragma mark -
#pragma mark Button interaction

- (IBAction) doPlayOrPauseTapped: (id)sender
{
	AM_DBG NSLog(@"AmbulantViewController handlePlayOrPauseTapped(0x%x)", self);
	if (myMainloop) {
		if (myMainloop->is_play_active()) {
			[self pause];
		} else {
			[self play];
		}
	} else {
		[self doPlayURL: nil fromNode: nil];
	}
}

- (IBAction) doRestartTapped: (id)sender
{
	AM_DBG NSLog(@"AmbulantViewController handleRestartTapped(0x%x)", self);
	if (myMainloop != NULL) {
		myMainloop->restart(false);
	} else {
		[self doPlayURL: nil fromNode: nil];
	}
} 

- (IBAction) doNextItem: (id)sender
{
	AM_DBG NSLog(@"AmbulantViewController playNextItem(0x%x)", self);
    [delegate selectNextPresentation];
}

- (IBAction) doAddFavorite: (id)sender
{
	AM_DBG NSLog(@"AmbulantViewController addFavorites(0x%x)", sender);
    assert(delegate);
	PresentationViewController* favoritesVC = [ delegate getPresentationViewWithIndex: 1];	
	[favoritesVC insertCurrentItemAtIndexPath: [ NSIndexPath indexPathForRow:0 inSection: 0 ]];
}

- (IBAction) doPlaylists: (id)sender
{
    assert(delegate);
    [self pause];
    [delegate showPresentationViews: self];
}

#pragma mark -
#pragma mark Notifications, etc.

- (void)settingsHaveChanged
{
	if (myMainloop) {
		if (delegate.nativeRenderer) {
			myMainloop->get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererAVFoundation"));   
		} else {
			myMainloop->get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererOpen"));
		}
	}
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
 	AM_DBG NSLog(@"AmbulantViewController didReceiveMemoryWarning:self=0x%x", self);
   [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

@end
