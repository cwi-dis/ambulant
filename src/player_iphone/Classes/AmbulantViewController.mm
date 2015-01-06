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

#import "AmbulantViewController.h"
#import "AmbulantAppDelegate.h"
#import "SettingsViewController.h"
#import <QuartzCore/QuartzCore.h>
#include "ambulant/lib/logger.h"

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
		AM_DBG NSLog(@"performSelectorOnMainThread: close: on 0x%@", (void*)m_mydocument);
		[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *str_url = [NSString stringWithUTF8String: newdoc.get_url().c_str()];
	id appDelegate = [[UIApplication sharedApplication] delegate];
	[appDelegate performSelectorOnMainThread: @selector(openWebLink:) withObject: str_url	waitUntilDone: NO];
	
	[pool release];
}

@implementation AmbulantViewController

#pragma mark -
#pragma mark Lifecycle

- (void) awakeFromNib
{
    AM_DBG NSLog(@"AmbulantViewController viewDidLoad(0x%@)", self);
    is_visible = NO;
    currentURL = nil;

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
    assert(scalerView);
	
	SEL shortTapAction = @selector(selectPointGesture:);
	SEL longTapAction = @selector(showHUDGesture:);
	if (delegate.shortTapForHUD) {
		shortTapAction = @selector(showHUDGesture:);
		longTapAction = @selector(selectPointGesture:);
	}
	// prepare to react on "double tap" gesture (select object in playerView with 1 finger tap)
	UITapGestureRecognizer *doubleTapGesture = [[UITapGestureRecognizer alloc]
		initWithTarget:self
		action:@selector(handleDoubleTapGesture:)];
	doubleTapGesture.numberOfTapsRequired = 2;
    doubleTapGesture.delegate = delegate;
	[scalerView.superview addGestureRecognizer:doubleTapGesture];
    [doubleTapGesture release];

	// prepare to react on "tap" gesture (select object in playerView with 1 finger tap)
	UITapGestureRecognizer *singleTapGesture = [[UITapGestureRecognizer alloc]
		initWithTarget:self
		action:shortTapAction];
    singleTapGesture.delegate = delegate;
	[scalerView.superview addGestureRecognizer:singleTapGesture];	
    [singleTapGesture release];
	// do not also errnoneously recognize a single tap when a double tap is recognized
	[singleTapGesture requireGestureRecognizerToFail:doubleTapGesture];	
	
	// prepare to react on "longPress" gesture (hold finger in one spot, longer than 0.4 sec.)
    UILongPressGestureRecognizer *longPressGesture = [[UILongPressGestureRecognizer alloc]
		initWithTarget:self
		action:longTapAction];
    longPressGesture.delegate = delegate;
    [scalerView.superview addGestureRecognizer:longPressGesture];
    [longPressGesture release];

	// prepare to react on "pinch" gesture (zoom playerView with 2 fingers)
	UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc]
		initWithTarget:self
		action:@selector(handlePinchGesture:)];
    pinchGesture.delegate = delegate;
	[scalerView.superview addGestureRecognizer:pinchGesture];
    [pinchGesture release];

	// prepare to react on "pan" gesture (move playerView with one finger)
    UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc]
		initWithTarget:self
		action:@selector(handlePanGesture:)];
    panGesture.delegate = delegate;
    [scalerView.superview addGestureRecognizer:panGesture];
    [panGesture release];
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
// - install gesture recognizers
- (void) viewDidLoad {
	AM_DBG NSLog(@"AmbulantViewController viewDidLoad(0x%@)", self);
    [super viewDidLoad];
    [self initGestures];
}

- (void)viewWillAppear:(BOOL)animated
{
	AM_DBG NSLog(@"AmbulantViewController viewWillAppear(0x%@)", self);
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	AM_DBG NSLog(@"AmbulantViewController viewWillDisappear(0x%@)", self);
    [super viewWillDisappear:animated];
    is_visible = NO;
}

- (void)viewDidAppear:(BOOL)animated
{
	AM_DBG NSLog(@"AmbulantViewController viewDidAppear(0x%@)", self);
    [super viewDidAppear:animated];
    [self play];
    is_visible = YES;
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	AM_DBG NSLog(@"AmbulantViewController viewDidUnLoad:self=0x%@", self);
    [super viewDidUnload];
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
	AM_DBG NSLog(@"AmbulantViewController dealloc:self=0x%@", self);
    [super dealloc];
	if (myMainloop)
		delete myMainloop;
	[playerView release];
	if (currentURL)
		[currentURL release];
}

#pragma mark -
#pragma mark Player control
- (bool) hasNetwork {
	static SCNetworkReachabilityRef reachabilityRef;
	if (reachabilityRef == NULL) {
		reachabilityRef = SCNetworkReachabilityCreateWithName(NULL, "www.ambulantplayer.org");
	}
	if (reachabilityRef == NULL)
		return false;
	SCNetworkReachabilityFlags flags;
	SCNetworkReachabilityGetFlags(reachabilityRef, &flags);
	return flags & kSCNetworkReachabilityFlagsReachable;
}

// create a new instance of the smil player
- (void) doPlayURL: (NSString*) theUrl fromNode: (NSString*) ns_node_repr
{    
    if (theUrl) {
        if (currentURL) [currentURL release];
        currentURL = [theUrl retain];
    }
    
	AM_DBG ambulant::lib::logger::get_logger()->trace("AmbulantViewController doPlayURL(0x%x): url=%s ns_node_repr=%s", self, currentURL? [ currentURL UTF8String]: "NULL", ns_node_repr? [ns_node_repr UTF8String] : "NULL");
	if (myMainloop != NULL) {
		myMainloop->no_stopped_callbacks();
		myMainloop->stop();
		delete myMainloop;
        
	}
    if (!currentURL) {
		return;
	}
    if (!playerView) {
        [self view]; // This loads the view
    }
    assert(self.view);
    assert(currentURL);
    assert(playerView);
    assert(embedder);
	if (self.view == NULL || currentURL == NULL || playerView == NULL || embedder == NULL) {
		return;
	}
	if ( ! [currentURL hasPrefix:@"file://"] && ! [self hasNetwork]) {
		[delegate showAlert: @"No Internet Connection"];
		return;
	}
	myMainloop = new mainloop([currentURL UTF8String], playerView, embedder);	
	if (myMainloop) {
		if (ns_node_repr != NULL) {
			std::string node_repr = [ns_node_repr UTF8String];
			myMainloop->goto_node_repr(node_repr);
		}
		[self showInteractionView: NO];
		[self showFinishedView: NO];
        if (is_visible) {
            [self play];
        } else {
            // play will be called in viewDidAppear
        }
	} else {
		return;
	}
}

- (bool) canPlay
{
    return myMainloop != NULL;
}

- (void) pause {
	AM_DBG NSLog(@"AmbulantViewController pause(0x%@)", self);
	if (myMainloop) {
		myMainloop->pause();
		UIImage* playImage = [UIImage imageNamed: @"Play_iPhone.png"];
	   [playPauseButton setImage:playImage forState:UIControlStateNormal];
	}
}

- (void) stopped {
    UIImage* playImage = [UIImage imageNamed: @"Play_iPhone.png"];
   [playPauseButton setImage:playImage forState:UIControlStateNormal];
   [self showFinishedView: YES];
}

- (void) play {
	AM_DBG NSLog(@"AmbulantViewController play(0x%@)", self);
	if (myMainloop) {
		myMainloop->play();
		UIImage* pauseImage = [UIImage imageNamed: @"Pause_iPhone.png"];
		[playPauseButton setImage:pauseImage forState:UIControlStateNormal];
	}
}

- (PlaylistItem*) currentItem {
	AM_DBG NSLog(@"AmbulantViewController currentItem(0x%@)", self);
	if (myMainloop) {
		return myMainloop->get_current_item();
	}
	return NULL;
}

#pragma mark -
#pragma mark View control

- (BOOL) isSupportedOrientation: (UIDeviceOrientation) orientation {
	AM_DBG NSLog(@"AmbulantViewController isSupportedOrientation(0x%@) orientation=%d", self, orientation);
	return 
		orientation == UIDeviceOrientationPortrait
        ||	orientation == UIDeviceOrientationPortraitUpsideDown
        ||	orientation == UIDeviceOrientationLandscapeLeft
        ||	orientation == UIDeviceOrientationLandscapeRight;
}

// Adapt to device orientation
- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation {  // iOS < 6
	AM_DBG NSLog(@"AmbulantViewController shouldAutorotateToInterfaceOrientation(0x%@): interfaceOrientation=%d", self, interfaceOrientation);
	return [self isSupportedOrientation:(UIDeviceOrientation) interfaceOrientation];
}

- (BOOL)shouldAutorotate { // iOS >= 6
    return YES;
}

- (NSUInteger)supportedInterfaceOrientations { // iOS >= 6
#ifndef __IPHONE_6_0
    return UIInterfaceOrientationPortrait;
#else
    return UIInterfaceOrientationMaskAll;
#endif
}

// react on device rotation
- (void) orientationChanged:(NSNotification *)notification {
//	AM_DBG NSLog(@"AmbulantViewController orientationChanged(0x%x):notification=%d", self, notification);
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	if (orientation == currentOrientation || ! [self isSupportedOrientation: orientation]) {
		return;
	}
	currentOrientation = orientation;
	if (scalerView != NULL) {
		[scalerView adaptDisplayAfterRotation];
	}
}

// display the Control Panel (as a HUD) at the bottom of the player view 
- (void) showInteractionView: (BOOL) want_show {
	if (want_show && interactionView.hidden) {
		nextPresentationButton.enabled = [delegate canSelectNextPresentation];
		interactionView.hidden = false;
		interactionView.opaque = true;
        assert(self.view);
        assert(interactionView);
        [self.view bringSubviewToFront:interactionView];
		if (delegate.autoHideHUD) {
			[NSObject cancelPreviousPerformRequestsWithTarget: self selector:@selector(autoHideInteractionView) object:nil];
			[self performSelector:@selector(autoHideInteractionView) withObject:nil afterDelay:(NSTimeInterval)5.0];
		}
	} else {
		interactionView.hidden = true;
		interactionView.opaque = false;
        [NSObject cancelPreviousPerformRequestsWithTarget: self selector:@selector(autoHideInteractionView) object:nil];
	}
}

- (void) showFinishedView: (BOOL) want_show {
	if (want_show) {
		finishedView.hidden = false;
		finishedView.opaque = true;
        NSData *poster_data = nil;
        PlaylistItem *item = myMainloop->get_current_item();
        if (item) poster_data = [item poster_data];
		if (poster_data) {
			finishedViewImage.image = [UIImage imageWithData: poster_data];
		} else {
			finishedViewImage.image = [UIImage imageNamed: @"DefaultPoster.png"];
		}
		[self.view bringSubviewToFront: finishedView];
	} else {
		finishedView.hidden = true;
		finishedView.opaque = false;
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

- (void) selectPointGesture:(UILongPressGestureRecognizer *)sender {
	AM_DBG NSLog(@"AmbulantViewController selectPointGesture(0x%@): sender=0x%@", self, sender);
	CGPoint location = [sender locationInView:playerView];
	if ( ! [playerView tappedAtPoint:location]) {
//		[delegate showPresentationViews:self];
	}
};

- (void) showHUDGesture:(UITapGestureRecognizer *)sender { // select
	AM_DBG NSLog(@"AmbulantViewController showHUDGesture(0x%@): sender=0x%@", self, sender);
	[self showInteractionView: YES];
}

- (void) handleDoubleTapGesture:(UITapGestureRecognizer *)sender { // select
	AM_DBG NSLog(@"AmbulantViewController handleDoubleTapGesture(0x%@): sender=0x%@", self, sender);
	CGPoint location = [sender locationInView:scalerView];
	[scalerView autoZoomAtPoint:location];
}

- (void) adjustAnchorPointForGestureRecognizer:(UIGestureRecognizer *)gestureRecognizer {
    // XXXJACK: should move to zoomView?
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
	AM_DBG NSLog(@"AmbulantViewController handlePinchGesture(0x%@): sender=0x%@", self, sender);
    [self adjustAnchorPointForGestureRecognizer: sender];
	CGFloat factor = [(UIPinchGestureRecognizer *)sender scale];
	[scalerView zoomWithScale:factor inState: [sender state]];
}

- (IBAction) handlePanGesture:(UIPanGestureRecognizer *)sender {
	AM_DBG NSLog(@"AmbulantViewController handlePanGesture(0x%@): sender=0x%@", self, sender);
	CGPoint translate = [sender translationInView: playerView.superview];
	[scalerView  translateWithPoint: (CGPoint) translate inState: [sender state]];
}

#pragma mark -
#pragma mark Button interaction

- (IBAction) doPlayOrPauseTapped: (id)sender
{
	AM_DBG NSLog(@"AmbulantViewController handlePlayOrPauseTapped(0x%@)", self);
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
	AM_DBG NSLog(@"AmbulantViewController handleRestartTapped(0x%@)", self);
	[self showFinishedView: NO];
	if (myMainloop != NULL) {
		myMainloop->restart(false);
		if (!myMainloop->is_play_active()) {
			[self play];
		}
	} else {
		[self doPlayURL: nil fromNode: nil];
	}
} 

- (IBAction) doNextItem: (id)sender
{
	AM_DBG NSLog(@"AmbulantViewController playNextItem(0x%@)", self);
    [delegate selectNextPresentation];
}

- (IBAction) doAddFavorite: (id)sender
{
	AM_DBG NSLog(@"AmbulantViewController addFavorites(0x%@)", sender);
    NSString *email_or_nil = @"EMail";
    if ([currentURL hasPrefix: @"file:"]) email_or_nil = nil;
    UIActionSheet *sheet = [[UIActionSheet alloc] 
        initWithTitle:@"Share" 
        delegate:self 
        cancelButtonTitle:@"Cancel" 
        destructiveButtonTitle:nil 
        otherButtonTitles:@"Add to Favorites", email_or_nil, nil];
    [sheet showInView: interactionView];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    NSLog(@"Clicked button %d", buttonIndex);
    NSString *action = [actionSheet buttonTitleAtIndex: buttonIndex];
    if ([action isEqualToString: @"Add to Favorites"]) {
        assert(delegate);
        PresentationViewController* favoritesVC = [ delegate getPresentationViewWithIndex: 1];	
        [favoritesVC insertCurrentItemAtIndexPath: [ NSIndexPath indexPathForRow:0 inSection: 0 ]];
    } else if ([action isEqualToString: @"EMail"]) {
        [self pause];
        MFMailComposeViewController *mc = [[MFMailComposeViewController alloc] init];
        mc.mailComposeDelegate = self;
        [mc setSubject: @"A presentation for you"];
        NSString *body = [NSString stringWithFormat: 
            @"<p>Here is a presentation I want to share with you. You may need to install Ambulant.</p>\n"
            "<p><a href=\"ambulant:%@\">%@</a></p>",
            currentURL, currentURL];
            
        [mc setMessageBody: body isHTML: YES];
        [self presentModalViewController:mc animated:YES];
    }
}

- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error 
{
    if (result == MFMailComposeResultFailed) {
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle: @"Mail"
            message: @"Failed to send Mail message"
            delegate:nil	
            cancelButtonTitle:@"OK"
            otherButtonTitles:nil];
        [alert show];
        [alert release];

    }
    [self dismissModalViewControllerAnimated:YES];
    [self play];
    [controller release];
}

- (IBAction) doPlaylists: (id)sender
{
    assert(delegate);
    [self pause];
	[self showFinishedView: NO];
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
	[scalerView recomputeZoom];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
 	AM_DBG NSLog(@"AmbulantViewController didReceiveMemoryWarning:self=0x%@", self);
   [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

@end

#pragma mark -

@implementation AmbulantContainerView
- (void) layoutSubviews {
    [super layoutSubviews];
}

@end

#pragma mark -

@implementation AmbulantScalerView
- (void) adaptDisplayAfterRotation
{
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	if (orientation == UIDeviceOrientationLandscapeLeft || orientation == UIDeviceOrientationLandscapeRight) {
		[[UIApplication sharedApplication] setStatusBarHidden: YES withAnimation: UIStatusBarAnimationNone];
	} else if (orientation == UIDeviceOrientationPortrait || orientation == UIDeviceOrientationPortraitUpsideDown) {
		[[UIApplication sharedApplication] setStatusBarHidden: NO withAnimation: UIStatusBarAnimationNone];
	} else {
		return;
	}

	// redisplay AmbulantView using the new settings
    [self setNeedsLayout];
	[self setNeedsDisplay];
}

- (void) recomputeZoom {
	zoomState = zoomUnknown;
	[self layoutSubviews];
}

- (void) layoutSubviews {
    UIView *playerView = [[self subviews] objectAtIndex: 0];
    assert(playerView);
    if (!playerView) return;

    // Initialize the zoomState, if not done already
    if (zoomState == zoomUnknown) {
        // adapt the ambulant window needed (bounds) in the current View
        ambulant::iOSpreferences *prefs = ambulant::iOSpreferences::get_preferences();
         anchorTopLeft = !(bool) prefs->m_auto_center;
        if (prefs->m_auto_resize) {
            zoomState = zoomFillScreen;
        } else {
            zoomState = zoomNaturalSize;
        }
    }
	
    if (zoomState == zoomFillScreen || zoomState == zoomNaturalSize) {
		// If we were showing the  presentation with a well-known aspect ratio (not
		// user panned or zoomed) we recreate that from scratch
        AM_DBG NSLog(@"LayoutSubviews fillScreen");
        
        // We start by making ourselves the same size as the child, and centering the child.
        self.bounds = playerView.bounds;
        playerView.bounds = self.bounds;
        CGSize pSize = self.bounds.size;
        CGFloat ourWidth, ourHeight;
        ourWidth = pSize.width;
        ourHeight = pSize.height;
        playerView.center = CGPointMake(ourWidth/2, ourHeight/2);
        
        // Now we need to determine our scale factor and (x,y) offset
        pSize = [self superview].bounds.size;
        CGFloat availableWidth, availableHeight;
		// It seems our width/height has been adapted when we get here during an orientation change.
		availableWidth = pSize.width;
		availableHeight = pSize.height;
		// find the smallest scale factor for both x- and y-directions
		float scale_x = availableWidth / ourWidth;
		float scale_y = availableHeight / ourHeight;
		float scale = scale_x < scale_y ? scale_x : scale_y;
		if (zoomState == zoomNaturalSize)
			scale = 1;
        float scale_cur = fabs(self.transform.a+self.transform.c); // We know x/y scale are same, and one of a/c is zero.
        self.transform = CGAffineTransformScale(self.transform, scale/scale_cur, scale/scale_cur);

        if (anchorTopLeft) {
			CGRect curFrame = self.frame;
			curFrame.origin = CGPointMake(0, 0);
			self.frame = curFrame;
		} else {
			self.center = CGPointMake(availableWidth/2, availableHeight/2);
		}
		
    } else {
		// if we were showing a user-determined pan/zoom we keep the current
		// aspect ratio and maintain the center.
		// XXX to be done
	}
}

- (void) zoomWithScale: (float) scale  inState: (UIGestureRecognizerState) state {
	if (state == UIGestureRecognizerStateBegan) {
		zoom_transform = self.transform;
	}
    self.transform = CGAffineTransformScale(zoom_transform, scale, scale);
	if (state == UIGestureRecognizerStateEnded) {
	}
    zoomState = zoomUser;
}

- (void) translateWithPoint: (CGPoint) point inState: (UIGestureRecognizerState) state {
    UIView *playerView = [[self subviews] objectAtIndex: 0];
    assert(playerView);
    if (!playerView) return;
	if (state == UIGestureRecognizerStateBegan) {
        translation_origin = self.center;
    }
    
    CGPoint newCenter = translation_origin;
	newCenter.x += point.x*self.transform.a;
	newCenter.y += point.y*self.transform.d;
    self.center = newCenter;
	
	if (state == UIGestureRecognizerStateEnded) {
	}
    zoomState = zoomUser;
}

- (void) autoZoomAtPoint: (CGPoint) point
{
    // Advance to "next" zoomstate, currently only fill-screen and natural-size.
    // Eventually we will add zoom-to-region here.
    zoomState = (ZoomState)(zoomState + 1);
    if (zoomState >= zoomUser) zoomState = zoomFillScreen;
    [self setNeedsLayout];
}

@end

