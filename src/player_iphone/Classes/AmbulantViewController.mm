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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif


void
document_embedder::show_file(const ambulant::net::url& href)
{
//	CFStringRef cfhref = CFStringCreateWithCString(NULL, href.get_url().c_str(), kCFStringEncodingUTF8);
//	CFURLRef url = CFURLCreateWithString(NULL, cfhref, NULL);
//	OSErr status;
	
//	if ((status=LSOpenCFURLRef(url, NULL)) != 0) {
//		ambulant::lib::logger::get_logger()->trace("Opening URL <%s>: LSOpenCFURLRef error %d", href.get_url().c_str());
//		ambulant::lib::logger::get_logger()->error(gettext("Cannot open: %s"), href.get_url().c_str());
//	}
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
#ifdef WITH_OVERLAY_WINDOW
	if (newdoc.get_protocol() == "ambulant_aux") {
		std::string aux_url = newdoc.get_url();
		aux_url = aux_url.substr(13);
		ambulant::net::url auxdoc = ambulant::net::url::from_url(aux_url);
		aux_open(auxdoc);
	}
#endif
	
	if (old) {
		AM_DBG NSLog(@"performSelectorOnMainThread: close: on 0x%x", (void*)m_mydocument);
		[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *str_url = [NSString stringWithUTF8String: newdoc.get_url().c_str()];
	AmbulantAppDelegate *delegate = [[UIApplication sharedApplication] delegate];
	[delegate performSelectorOnMainThread: @selector(openWebLink:)
							   withObject: str_url	waitUntilDone: NO];
	
	[pool release];
}

@implementation AmbulantViewController

@synthesize interactionView, modeBar, originalPlayerViewFrame, originalInteractionViewFrame,
			playerView, myMainloop, linkURL, playURL,
			keyboardIsShown, currentOrientation, autoCenter, autoResize,
			nativeRenderer, play_active, historyViewController;

/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/


/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/
- (void) doPlayURL:(NSString*) ns_node_repr {
	AM_DBG NSLog(@"AmbulantViewController viewDidLoad(0x%x): ns_node_repr=%@", self, ns_node_repr);
	if (myMainloop != NULL) {
		myMainloop->stop();
		delete myMainloop;
	}		
	myMainloop = new mainloop([[self playURL] UTF8String], playerView, embedder);	
	if (myMainloop) {
		if (ns_node_repr != NULL) {
			std::string node_repr = [ns_node_repr UTF8String];
			myMainloop->goto_node_repr(node_repr);
		}
		myMainloop->play();
	}
	if ( ! interactionView.hidden) {
		interactionView.hidden = true;
		interactionView.opaque = false;
		modeBar.hidden = true;
		modeBar.opaque = false;
	}
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
	AM_DBG NSLog(@"AmbulantViewController viewDidLoad(0x%x)", self);
    [super viewDidLoad];
	// prepare to react after keyboard show/hide
	[[NSNotificationCenter defaultCenter]
	 addObserver:self
	 selector:@selector(keyboardWillShow:)
	 name:UIKeyboardWillShowNotification
	 object: nil];
	[[NSNotificationCenter defaultCenter]
	 addObserver:self
	 selector:@selector(keyboardWillHide:)
	 name:UIKeyboardWillHideNotification
	 object: nil];

	// prepare to react when device is rotated
	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	[[NSNotificationCenter defaultCenter]
	 addObserver:self
	 selector:@selector(orientationChanged:)
	 name:UIDeviceOrientationDidChangeNotification
	 object: nil];
	ambulant::iOSpreferences::get_preferences()->load_preferences();
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	autoCenter = prefs->m_auto_center;
	autoResize = prefs->m_auto_resize;
	nativeRenderer = ! prefs->m_prefer_ffmpeg;
	
	// prepare to react on "tap" gesture (select object in playerView with 1 finger tap)
	UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc]
											  initWithTarget:self action:@selector(handleTapGesture:)];
	[self.playerView addGestureRecognizer:tapGesture];
    [tapGesture release];
	
	// prepare to react on "pinch" gesture (zoom playerView with 2 fingers)
	UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc]
											  initWithTarget:self action:@selector(handlePinchGesture:)];
	[self.playerView addGestureRecognizer:pinchGesture];
    [pinchGesture release];
	// prepare to react on "pan" gesture (move playerView with one finger)
    UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc]
										  initWithTarget:self action:@selector(handlePanGesture:)];
    [self.playerView addGestureRecognizer:panGesture];
    [panGesture release];
	
/*	swipe doesn't work well with pan
	// prepare to react on "swipe" gesture (move finger in one direction continuously)
    UISwipeGestureRecognizer *swipeGesture = [[UISwipeGestureRecognizer alloc]
													  initWithTarget:self action:@selector(handleSwipeOrLongPressGesture:)];
	swipeGesture.direction = (UISwipeGestureRecognizerDirection)
							(UISwipeGestureRecognizerDirectionRight | UISwipeGestureRecognizerDirectionLeft
							  | UISwipeGestureRecognizerDirectionUp | UISwipeGestureRecognizerDirectionDown);
    [self.playerView addGestureRecognizer:swipeGesture];
    [swipeGesture release];
*/	
	// prepare to react on "longPress" gesture (hold finger in one spot, longer than 0.4 sec.)
    UILongPressGestureRecognizer *longPressGesture = [[UILongPressGestureRecognizer alloc]
													  initWithTarget:self action:@selector(handleSwipeOrLongPressGesture:)];
	//	longPressGesture.direction = (UIlongPressGestureRecognizerDirectionRight | UIlongPressGestureRecognizerDirectionLeft
	//							  | UIlongPressGestureRecognizerDirectionUp | UIlongPressGestureRecognizerDirectionDown);
    [self.playerView addGestureRecognizer:longPressGesture];
    [longPressGesture release];
	
	
	embedder = new document_embedder(self);
	
//	if ( ! prefs->m_normal_exit) {
		// was crashed
//		[self initialize_after_crashing];
//		return;
//	}
	if (self.playURL != nil) {
		// launched by Safari
		AM_DBG NSLog(@"View=%@ playUrl=%@", [self playerView], [self playURL]);
		[self handleURLEntered];
	} else {
		// launched by user
		NSString *startPath = NULL;
		NSString *startNodeRepr = NULL;
		if (prefs->m_normal_exit) {
			// restart were we left
			PlaylistItem* last_item = prefs->m_history != NULL ? prefs->m_history->get_last_item() : NULL;
			startPath = last_item != NULL ? [[last_item ns_url] absoluteString] : NULL;
			startNodeRepr = last_item != NULL ? [last_item ns_last_node_repr] : NULL;		
		}
		if (startPath == NULL) {
			NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
			startPath = [thisBundle pathForResource:@"Welcome" ofType:@"smil"];
		}
//		NSString *startPath = [thisBundle pathForResource:@"test" ofType:@"smil"];
//		NSString *startPath = [thisBundle pathForResource:@"iPhoneAVPlayerTest" ofType:@"smil"];
//		NSString *startPath = @"http://ambulantPlayer.org/Demos/Birthday/HappyBirthday.smil";
		AM_DBG NSLog (@"startPath=%@, startNodeRepr%@", startPath, startNodeRepr);
		if (startPath != NULL) {
			prefs->m_normal_exit = false;
			prefs->save_preferences();
			void* theview = [self playerView];
			AM_DBG NSLog(@"view %@ responds %d", (NSObject *)theview, [(NSObject *)theview respondsToSelector: @selector(isAmbulantWindowInUse)]);
			playURL = [[NSMutableString alloc] initWithString: startPath];
			[self doPlayURL: startNodeRepr];
		}
	} 	
}

- (IBAction) handlePlayTapped {
	AM_DBG NSLog(@"AmbulantViewController handlePlayTapped(0x%x)", self);
	if (myMainloop) {
		myMainloop->play();
	} else {
		[self doPlayURL:NULL];
	}
}

- (IBAction) handlePauseTapped {
	AM_DBG NSLog(@"AmbulantViewController handlePauseTapped(0x%x)", self);
	if (myMainloop) {
		myMainloop->pause();
	}
}

- (IBAction) handleStopTapped {
	AM_DBG NSLog(@"AmbulantViewController handleStopTapped(0x%x)", self);
	if (myMainloop == NULL) {
		return;
	}
	myMainloop->stop();
	if (playerView == NULL)
		//XXXX for some reason the playerView is reset to 0 when play starts
		playerView = (id) myMainloop->get_view();
	delete myMainloop;
//	[playerView release];
	myMainloop = NULL;
}


/*	Code from Apple's developer documentation "Gesture Recognizers"*/
- (IBAction) handleTapGesture:(UIGestureRecognizer *)sender { // select
	AM_DBG NSLog(@"AmbulantViewController handleTapGesture(0x%x): sender=0x%x", self, sender);
	CGPoint location = [(UITapGestureRecognizer *)sender locationInView:self.playerView];
	[self.playerView tappedAtPoint:location];
}

- (IBAction) handlePinchGesture:(UIGestureRecognizer *)sender { // zoom
	AM_DBG NSLog(@"AmbulantViewController handlePinchGesture(0x%x): sender=0x%x", self, sender);
	CGFloat factor = [(UIPinchGestureRecognizer *)sender scale];
	[self.playerView zoomWithScale:factor inState: [sender state]];
}

- (IBAction) handlePanGesture:(UIPanGestureRecognizer *)sender {
	AM_DBG NSLog(@"AmbulantViewController handlePanGesture(0x%x): sender=0x%x", self, sender);
	CGPoint translate = [sender translationInView: playerView.superview];
	[self.playerView  translateWithPoint: (CGPoint) translate inState: [sender state]];
}

- (IBAction) handleSwipeOrLongPressGesture:(UIGestureRecognizer *)sender {
	//	CGPoint translate = [sender translationInView: playerView.superview];
	//	[self.playerView  translateWithPoint: (CGPoint) translate inState: [sender state]];
	static bool	wasPressed = false;
	AM_DBG NSLog(@"AmbulantViewController handleSwipeOrLongPressGesture(0x%x): sender=0x%x, wasPressed=%d", self, sender, wasPressed);
	if (wasPressed && interactionView != NULL) {
		interactionView.hidden = ! interactionView.hidden;
		interactionView.opaque = ! interactionView.opaque;
		modeBar.hidden = ! modeBar.hidden;
		modeBar.opaque = ! modeBar.opaque;
		wasPressed = false;
	} else if (interactionView != NULL) {
		wasPressed = ! wasPressed;
	}
}

// dismiss the keyboard when the <Return> is tapped
- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	AM_DBG NSLog(@"textFieldShouldReturn: text=%@", textField.text);
	[textField resignFirstResponder]; // dismiss keyboard
	return NO;
}
- (IBAction) handleURLEntered {
	AM_DBG NSLog(@"AmbulantViewController handleURLEntered(0x%x)", self);
	[self doPlayURL:NULL];
}

- (IBAction) showSettings:(id)sender { //JNK
	AM_DBG NSLog(@"AmbulantViewController showSettings(0x%x)", self);
	
	if (myMainloop != NULL) {
		play_active = myMainloop->is_play_active();
		myMainloop->pause();
	}
	SettingsViewController *controller = [[SettingsViewController alloc]
										  initWithNibName:@"SettingsViewController" bundle:nil];
	controller.title = @"Settings";
	controller.delegate = self;
	
	controller.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
	[self presentModalViewController:controller animated:YES];
	
	[controller release];
}

- (IBAction) addFavorites:(id)sender {
	AM_DBG NSLog(@"AmbulantViewController addFavorites(0x%x)", sender);
	AmbulantAppDelegate* app_delegate = (AmbulantAppDelegate*)([UIApplication sharedApplication].delegate);
	PresentationViewController* favoritesVC = [ app_delegate getPresentationView: self withIndex: 1];	
#ifdef	FIRST_ITEM
	[favoritesVC insertCurrentItemAtIndexPath: [ NSIndexPath indexPathForRow:FIRST_ITEM inSection: 0 ]];
#else //FIRST_ITEM
	[favoritesVC insertCurrentItemAtIndexPath: [ NSIndexPath indexPathForRow:0 inSection: 0 ]];
#endif//FIRST_ITEM
	[app_delegate showPresentationView: self withIndex: 1];
}


- (void)
settingsHaveChanged:(SettingsViewController *)controller {
	AM_DBG NSLog(@"AmbulantViewController showSettings(0x%x)", self);
	// check we have the settings view
	if (controller.view.tag != 40) {
		return;
	}
	// get the values entered by the user
	autoCenter = [controller autoCenter];
	autoResize = [controller autoResize];
	nativeRenderer = [controller nativeRenderer];
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	prefs->m_auto_center = autoCenter;
	prefs->m_auto_resize = autoResize;
	prefs->m_prefer_ffmpeg = ! nativeRenderer;
	if (myMainloop) {
		if (nativeRenderer) {
			myMainloop->get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererAVFoundation"))    ;   
		} else {
			myMainloop->get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererOpen"));
		}
	}
	prefs->save_preferences();
//	[self dismissModalViewControllerAnimated:YES];
}

- (void)
playlistViewControllerDidFinish: (UIViewController *)controller {
	
	AM_DBG NSLog(@"playlistViewControllerDidFinish: controller=0x%x", controller);
	[self settingsHaveChanged: controller];	
//	[self dismissModalViewControllerAnimated:YES];
	[(AmbulantAppDelegate*)([UIApplication sharedApplication].delegate) showAmbulantPlayer: (id) self];
	if (myMainloop != NULL) {
		if (play_active) {
			myMainloop->play();
		}
		UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
		[playerView adaptDisplayAfterRotation: orientation];
	}
	if ( ! interactionView.hidden) {
		interactionView.hidden = true;
		interactionView.opaque = false;
		modeBar.hidden = true;
		modeBar.opaque = false;
	}
	playerView.alpha = 1.0;
}

- (IBAction) showHistory:(id)sender {    
	AM_DBG NSLog(@"AmbulantViewController (0x%x)", self);
	
	if (myMainloop != NULL) {
		play_active = myMainloop->is_play_active();
		myMainloop->pause();
	}
	/*
	PresentationViewController *controller = [[PresentationViewController alloc]
										  initWithNibName:@"PresentationTableViewController" bundle:nil];
	controller.title = @"Presentations";
	controller.delegate = self;
	
	controller.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
	[self presentModalViewController:controller animated:YES];
	
	[controller release];
	 */
	[(AmbulantAppDelegate*)([UIApplication sharedApplication].delegate) showPresentationViews: self];
}

- (void)
done: (id) sender {
	AM_DBG NSLog(@"AmbulantViewController done(0x%x): sender=0x%x", self, sender);
	[self playlistViewControllerDidFinish: (UIViewController*) sender];
}

- (void)
presentationViewControllerDidFinish: (PresentationViewController *)controller {
	AM_DBG NSLog(@"AmbulantViewController presentationViewControllerDidFinish(0x%x): controller=0x%x", self, controller);
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	[(AmbulantAppDelegate*)([UIApplication sharedApplication].delegate) showAmbulantPlayer: (id) self];
	if (controller != NULL && controller.editingStyle != UITableViewCellEditingStyleNone) {
		[controller toggleEditMode];
	}
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	[playerView adaptDisplayAfterRotation: orientation];
	if ( ! interactionView.hidden) {
		interactionView.hidden = true;
		interactionView.opaque = false;
		modeBar.hidden = true;
		modeBar.opaque = false;
	}
	if (myMainloop != NULL) {
		UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
		[playerView adaptDisplayAfterRotation: orientation];
		
		if (play_active) {
			myMainloop->play();
		}
	}
	if ( ! interactionView.hidden) {
		interactionView.hidden = true;
		interactionView.opaque = false;
		modeBar.hidden = true;
		modeBar.opaque = false;
	}
	playerView.alpha = 1.0;
	prefs->save_preferences(); // save possible edits
}

- (void)
setHistoryViewController:(PresentationViewController *)controller
{
	AM_DBG NSLog(@"AmbulantViewController setHistoryViewController(0x%x) controller=0x%x", self, controller);
	if ( ! controller.isFavorites) {
		historyViewController = controller;
	}
}
- (void)
playPresentation: (NSString*) whatString {
	AM_DBG NSLog(@"AmbulantViewController (0x%x)", self);
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AM_DBG NSLog(@"Selected: %@",whatString);
	self.handleStopTapped;
	if ( ! ([whatString hasPrefix:@"file://"] || [whatString hasPrefix:@"http://"])) {
		// assume local file, check for aboslute path
		if ( ! [whatString hasPrefix:@"/"]) {
			NSString* homedir = NSHomeDirectory();
			homedir = [homedir stringByAppendingString:@"/player_iphone.app/"];
			whatString = [homedir stringByAppendingString:whatString];//[thisBundle pathForResource:whatString ofType:@"smil"];
		}
		whatString = [@"file://" stringByAppendingString:whatString];
	}
	if (whatString != NULL) {
		if ( ! [whatString hasSuffix:@".smil"]) {
			whatString = [whatString stringByAppendingString:@".smil"];
		}
		if (playURL) {
			[playURL release];
		}
		playURL = [[NSMutableString alloc] initWithString: whatString];
		[self doPlayURL:NULL];
		[[self historyViewController] updatePlaylist];
	}
	[self done: self];
	[pool release];
}

- (void)keyboardWillShow:(NSNotification *)notification {
	AM_DBG NSLog(@"AmbulantViewController keyboardWillShow(0x%x) notification=", self, notification);    
    /*
     Reduce the size of the playerView so that it's not obscured by the keyboard.
     Animate the resize so that it's in sync with the appearance of the keyboard.
     */
	if (keyboardIsShown) {
		return;
	}
	keyboardIsShown = true;
    NSDictionary *userInfo = [notification userInfo];
    
    // Get the height of the keyboard when it's displayed.
    NSValue* aValue = [userInfo objectForKey:UIKeyboardFrameEndUserInfoKey];
	
    // Get the top of the keyboard as the y coordinate of its origin in self's view's coordinate system.
	// The bottom of the text view's frame should align with the top of the keyboard's final position.
    CGRect keyboardRect = [aValue CGRectValue];
    keyboardRect = [self.view convertRect:keyboardRect fromView:nil];
	
	AM_DBG NSLog(@"keyboardRect=(%f,%f,%f,%f",
		  keyboardRect.origin.x,keyboardRect.origin.y,
		  keyboardRect.size.width,keyboardRect.size.height);
    CGFloat keyboardHeight = keyboardRect.size.height;
	originalInteractionViewFrame = interactionView.frame;
    CGRect newInteractionViewFrame = interactionView.frame;
	AM_DBG NSLog(@"newInteractionViewFrame=(%f,%f,%f,%f",
		  newInteractionViewFrame.origin.x,newInteractionViewFrame.origin.y,
		  newInteractionViewFrame.size.width,newInteractionViewFrame.size.height);
	newInteractionViewFrame.origin.y -= keyboardHeight;
	originalPlayerViewFrame = playerView.frame;
    CGRect newPlayerViewFrame = originalPlayerViewFrame;
	newPlayerViewFrame.origin.y -= keyboardHeight;
    // Get the duration of the animation.
    NSValue *animationDurationValue = [userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey];
    NSTimeInterval animationDuration;
    [animationDurationValue getValue:&animationDuration];
    
    // Animate the resize of the playerView's frame and the repositioning of the
	// interactionView in sync with the keyboard's appearance.
	[UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:animationDuration];
    
    interactionView.frame = newInteractionViewFrame;
	playerView.frame = newPlayerViewFrame;
	
    [UIView commitAnimations];
}

- (void)keyboardWillHide:(NSNotification *)notification {
	AM_DBG NSLog(@"AmbulantViewController keyboardWillHide(0x%x): notification=%d", self, notification);    
	if ( ! keyboardIsShown) {
		return;
	}	
	keyboardIsShown = false;

	[self handlePauseTapped];
	
    NSDictionary* userInfo = [notification userInfo];    
    /*
     Restore the size of the playerView and the position of the InteractionView.
     Animate the resize so that it's in sync with the disappearance of the keyboard.
     */
    NSValue *animationDurationValue = [userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey];
    NSTimeInterval animationDuration;
    [animationDurationValue getValue:&animationDuration];
    
    [UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:animationDuration];
	interactionView.frame = originalInteractionViewFrame;
	playerView.frame = originalPlayerViewFrame;
    [UIView commitAnimations];
	[self handlePlayTapped];
}	

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
	//AM_DBG NSLog(@"AmbulantViewController orientationChanged(0x%x):notification=%d", self, notification);
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	if (orientation == currentOrientation || ! [self isSupportedOrientation: orientation]) {
		return;
	}
	currentOrientation = orientation;
	if (view != NULL) {
		[playerView adaptDisplayAfterRotation: orientation];
	}
}

- (void) close: (NSString*) id {
	AM_DBG NSLog(@"AmbulantViewController close: id=%@", id);
	[self handleStopTapped];
}

- (void) pause {
	AM_DBG NSLog(@"AmbulantViewController pause(0x%x)", self);
	if (myMainloop) {
		myMainloop->pause();
	}
}

- (PlaylistItem*) currentItem {
	AM_DBG NSLog(@"AmbulantViewController currentItem(0x%x)", self);
	if (myMainloop) {
		return myMainloop->get_current_item();
	}
	return NULL;
}
- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
 	AM_DBG NSLog(@"AmbulantViewController didReceiveMemoryWarning:self=0x%x", self);
   [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	AM_DBG NSLog(@"AmbulantViewController viewDidUnLoad:self=0x%x", self);
}

- (void) initialize_after_crashing {
	AM_DBG NSLog(@"AmbulantViewController initialize_after_crashing:self=0x%x", self);
	// We are in an unknown state. Make 'History' view visible
//	[self handleSwipeOrLongPressGesture: (UIGestureRecognizer*) self];
//	sleep(1);
//	[self handleSwipeOrLongPressGesture:(UIGestureRecognizer*) self];
	[self showHistory:(id) self];
}

- (void)dealloc {
	AM_DBG NSLog(@"AmbulantViewController dealloc:self=0x%x", self);
    [super dealloc];
	if (myMainloop)
		delete myMainloop;
	[playerView release];
	if (playURL)
		[playURL release];
}

@end
