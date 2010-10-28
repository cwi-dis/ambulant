//
//  PlaylistViewController.m
//  FlipableClock
//
//  Created by Kees Blom on 7/31/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "PlaylistViewController.h"
#import "AmbulantViewController.h"
#include "ambulant/common/preferences.h"

@implementation PlaylistViewController

@synthesize delegate, autoCenterSwitch, autoResizeSwitch, nativeRendererSwitch;

- (void)viewDidLoad {
    [super viewDidLoad];
	// initialize to values taken from AmbulantView
	AmbulantViewController* AmVC = (AmbulantViewController*)delegate;
	autoCenterSwitch.on = AmVC.autoCenter;
	autoResizeSwitch.on = AmVC.autoResize;
	nativeRendererSwitch.on = AmVC.nativeRenderer;	
}

- (BOOL) autoCenter {
	return autoCenterSwitch.on;
}

- (BOOL) autoResize {
	return autoResizeSwitch.on;
}

- (BOOL) nativeRenderer {
	return nativeRendererSwitch.on;
}

- (IBAction) done:(id)sender {
	[self.delegate playlistViewControllerDidFinish:self];	
}

- (IBAction) playWelcome {
	[self.delegate playIt:self selected:@"Welcome"];
}

- (IBAction) playNYC {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/smilText/NYC-sT.smil"];
}

- (IBAction) playVideoTests {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/VideoTests/VideoTests.smil"];	
//	[self.delegate playIt:self selected:@"http://homepages.cwi.nl/~kees/ambulant/VideoTests2+3.smil"];	
}

- (IBAction) playPanZoom {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/PanZoom/Fruits-4s.smil"];	
}

- (IBAction) playBirthday {
	[self.delegate playIt:self selected:@"http://homepages.cwi.nl/~kees/ambulant/ios/Demos/Birthday/HappyBirthday.smil"];	
}

- (IBAction) playBirthdayRTSP {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/Birthday/HappyBirthday-rtsp.smil"];	
}

- (IBAction) playEuros {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/Euros/Euros.smil"];	
}

- (IBAction) playEurosRTSP {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/Euros/Euros-rtsp.smil"];	
}

- (IBAction)playFlashlight {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/Flashlight/Flashlight-US.smil"];	
}

- (IBAction)playFlashlightRTSP {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/Flashlight/Flashlight-US-rtsp.smil"];	
}

- (IBAction)playNews {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/News/DanesV2-Desktop.smil"];	
}

- (IBAction)playNewsRTSP {
	[self.delegate playIt:self selected:@"http://ambulantPlayer.org/Demos/News/DanesV2-Desktop-rtsp.smil"];	
}

- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation {
	return YES;
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}


- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	// Return YES for supported orientations
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/


- (void)dealloc {
    [super dealloc];
}


@end
