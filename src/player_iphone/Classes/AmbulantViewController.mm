//
//  AmbulantViewController.mm
//  Ambulant
//
//  Created by Kees Blom on 7/12/10.
//  Copyright CWI 2010. All rights reserved.
//

#import "AmbulantViewController.h"
#import "AmbulantAppDelegate.h"
#import "PlaylistViewController.h"

#define AM_DBG
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
	document_embedder::open(href, true, NULL);
}

void
document_embedder::close(ambulant::common::player *p)
{
	[m_mydocument performSelectorOnMainThread: @selector(close:) withObject: nil waitUntilDone: NO];
}

void
document_embedder::open(ambulant::net::url newdoc, bool start, ambulant::common::player *old)
{
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

@synthesize interactionView, originalPlayerViewFrame, originalInteractionViewFrame,
			playerView, myMainloop, URLEntryField, linkURL, playURL,
			keyboardIsShown, currentOrientation, autoCenter, autoResize,
			nativeRenderer, play_active;

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
- (void) doPlayURL {
	if (myMainloop != NULL) {
		myMainloop->stop();
		delete myMainloop;
	}		
	myMainloop = new mainloop([[self playURL] UTF8String], playerView, embedder);	
	if (myMainloop) {
		myMainloop->play();
		self.URLEntryField.text = [self playURL];
	}
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
	AM_DBG NSLog(@"AmbulantViewController viewDidLoad:self=0x%x", self);
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
	
	// prepare to react when text is entered
	self.URLEntryField.delegate = self;
	
	embedder = new document_embedder(self);
	AM_DBG NSLog(@"View=%@ playUrl=%@", [self playerView], [self playURL]);
	if (self.playURL != nil) {
		[self handleURLEntered];
	} else {
		NSString *welcomePath = prefs->m_last_used; // default: Welcome.smil
		if ([welcomePath isEqualToString:@"Welcome.smil"]) {
			NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
			welcomePath = [thisBundle pathForResource:@"Welcome" ofType:@"smil"];
		}
//		NSString *welcomePath = [thisBundle pathForResource:@"test" ofType:@"smil"];
//		NSString *welcomePath = [thisBundle pathForResource:@"iPhoneAVPlayerTest" ofType:@"smil"];
//		NSString *welcomePath = @"http://ambulantPlayer.org/Demos/Birthday/HappyBirthday.smil";
		AM_DBG NSLog (@ "%@", welcomePath);
		if (welcomePath) {
			void* theview = [self playerView];
			AM_DBG NSLog(@"view %@ responds %d", (NSObject *)theview, [(NSObject *)theview respondsToSelector: @selector(isAmbulantWindowInUse)]);
			playURL = [[NSMutableString alloc] initWithString: welcomePath];
			[self doPlayURL ];
		}
	} 	
//X	[URLEntryField addTarget:self action:@selector(textFieldDidChange:) forControlEvents:UIControlEventEditingChanged];
}

- (IBAction) handlePlayTapped {
	if (myMainloop) {
		myMainloop->play();
	} else {
		[self doPlayURL];
	}
}

- (IBAction) handlePauseTapped {
	if (myMainloop) {
		myMainloop->pause();
	}
}

- (IBAction) handleStopTapped {
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
	CGPoint location = [(UITapGestureRecognizer *)sender locationInView:self.playerView];
	[self.playerView tappedAtPoint:location];
}

//  XXXX cleanup needed: move the this code into genuine member function of AmbulantPlayer
- (IBAction) handlePinchGesture:(UIGestureRecognizer *)sender { // zoom
	CGFloat factor = [(UIPinchGestureRecognizer *)sender scale];
	[self.playerView zoomWithScale:factor inState: [sender state]];
}

//  XXXX cleanup needed: move the this code into genuine member function of AmbulantPlayer
- (IBAction) handlePanGesture:(UIPanGestureRecognizer *)sender {
	CGPoint translate = [sender translationInView: playerView.superview];
	[self.playerView  translateWithPoint: (CGPoint) translate inState: [sender state]];
}

// dismiss the keyboard when the <Return> is tapped
- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	[textField resignFirstResponder]; // dismiss keyboard
	return NO;
}

-(IBAction) textFieldTextDidChange {	
// This method will be called whenever an object sends UITextFieldTextDidChangeNotification
	AM_DBG NSLog(@"textFieldTextDidChange: text=%@",URLEntryField.text);
	[playURL setString: URLEntryField.text];
} 

- (void)textFieldDidEndEditing:(UITextField *)textField
// A delegate method called by the URL text field when the editing is complete. 
// We save the current value of the field in our settings.
{
	AM_DBG NSLog(@"textFieldDidEndEditing: %@",textField.text);
}	
- (IBAction) handleURLEntered {
	/*
	if (URLEntryField.text.length == 0 || [URLEntryField.text isEqual: playURL]) {
		return;
	}
	if (playURL != NULL) {
		[playURL release];
	}
	playURL = [[NSString alloc] initWithString: URLEntryField.text];
	 */
	[self doPlayURL];
}

- (IBAction) showPlaylist:(id)sender {    
	
	if (myMainloop != NULL) {
		play_active = myMainloop->is_play_active();
		myMainloop->pause();
	}
	PlaylistViewController *controller = [[PlaylistViewController alloc]
										  initWithNibName:@"PlaylistViewController" bundle:nil];
	controller.title = @"Playlist";
	controller.delegate = self;
	
	controller.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
	[self presentModalViewController:controller animated:YES];
	
	[controller release];
}

- (void)
playlistViewControllerDidFinish: (PlaylistViewController *)controller {
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
	[self dismissModalViewControllerAnimated:YES];
	
	if (myMainloop != NULL) {
		if (play_active) {
			myMainloop->play();
		}
		UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
		[playerView adaptDisplayAfterRotation: orientation];
	}
}

- (IBAction) showHistory:(id)sender {    
	
	if (myMainloop != NULL) {
		play_active = myMainloop->is_play_active();
		myMainloop->pause();
	}
	PresentationViewController *controller = [[PresentationViewController alloc]
										  initWithNibName:@"PresentationTableViewController" bundle:nil];
	controller.title = @"Presentations";
	controller.delegate = self;
	
	controller.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
	[self presentModalViewController:controller animated:YES];
	
	[controller release];
}

- (void) done: (id) sender {
	[self playlistViewControllerDidFinish: (PresentationViewController*) sender];
}

- (void) presentationViewControllerDidFinish: (PresentationViewController *)controller {
//	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();
	[self dismissModalViewControllerAnimated:YES];
	
	if (myMainloop != NULL) {
		if (play_active) {
			myMainloop->play();
		}
		UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
		[playerView adaptDisplayAfterRotation: orientation];
	}
}


- (void) playPresentation: (NSString*) whatString {
	AM_DBG NSLog(@"Selected: %@",whatString);
	self.handleStopTapped;
	if ( ! [whatString hasPrefix:@"http://"]) {
		NSString* homedir = NSHomeDirectory();
		homedir = [homedir stringByAppendingString:@"/player_iphone.app/"];
		whatString = [homedir stringByAppendingString:whatString];//[thisBundle pathForResource:whatString ofType:@"smil"];
		if ( ! [whatString hasSuffix:@".smil"]) {
			whatString = [whatString stringByAppendingString:@".smil"];
		}
	}
	if (whatString != NULL) {
		if (playURL) {
			[playURL release];
		}
		playURL = [[NSMutableString alloc] initWithString: whatString];
		[self doPlayURL];
	}
	[self done: self];
}


- (void) playPresentation: selected: (NSString*) whatString {
	AM_DBG NSLog(@"Selected: %@",whatString);
	self.handleStopTapped;
	if ( ! [whatString hasPrefix:@"http://"]) {
		NSString* homedir = NSHomeDirectory();
		homedir = [homedir stringByAppendingString:@"/player_iphone.app/"];
		whatString = [homedir stringByAppendingString:whatString];//[thisBundle pathForResource:whatString ofType:@"smil"];
		if ( ! [whatString hasSuffix:@".smil"]) {
			whatString = [whatString stringByAppendingString:@".smil"];
		}
	}
	if (whatString != NULL) {
		if (playURL) {
			[playURL release];
		}
		playURL = [[NSMutableString alloc] initWithString: whatString];
		[self doPlayURL];
	}
	[self done: self];
}

- (void)keyboardWillShow:(NSNotification *)notification {
    
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
	return 
		orientation == UIDeviceOrientationPortrait
	||	orientation == UIDeviceOrientationPortraitUpsideDown
	||	orientation == UIDeviceOrientationLandscapeLeft
	||	orientation == UIDeviceOrientationLandscapeRight;
}

/* */
// Override to allow orientations other than the default portrait orientation.
- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation {
	return [self isSupportedOrientation:(UIDeviceOrientation) interfaceOrientation];
}
// react on device rotation
- (void) orientationChanged:(NSNotification *)notification {
	AM_DBG NSLog(@"orientationChanged");
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	if (orientation == currentOrientation || ! [self isSupportedOrientation: orientation]) {
		return;
	}
	currentOrientation = orientation;
	if (keyboardIsShown) {
		[[self URLEntryField] resignFirstResponder]; // dismiss keyboard
	}
	[playerView adaptDisplayAfterRotation: orientation];
}

- (void) close: (NSString*) id {
	AM_DBG NSLog(@"AmbulantViewController close: unimplemented");
	[self handleStopTapped];
}

- (void) pause {
	if (myMainloop) {
		myMainloop->pause();
	}
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
