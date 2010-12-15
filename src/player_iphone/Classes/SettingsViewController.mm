//
//  SettingsViewController.m
//
//  Created by Kees Blom on 7/31/10.
//  Copyright CWI 2010. All rights reserved.
//

#import "SettingsViewController.h"
#import "AmbulantViewController.h"
#include "ambulant/common/preferences.h"

@implementation SettingsViewController

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
	[self.delegate settingsHaveChanged:self];
	[self.delegate playlistViewControllerDidFinish:self];	
}

- (IBAction) playWelcome {
	[self.delegate playPresentation:@"Welcome" fromPresentationViewController: NULL];
}

- (IBAction) playNYC {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/smilText/NYC-sT.smil" fromPresentationViewController: NULL];
}

- (IBAction) playVideoTests {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/VideoTests/VideoTests.smil" fromPresentationViewController: NULL];	
//	[self.delegate playPresentation:@"http://homepages.cwi.nl/~kees/ambulant/VideoTests2+3.smil"];	
}

- (IBAction) playPanZoom {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/PanZoom/Fruits-4s.smil" fromPresentationViewController: NULL];	
}

- (IBAction) playBirthday {
	[self.delegate playPresentation:@"http://homepages.cwi.nl/~kees/ambulant/ios/Demos/Birthday/HappyBirthday.smil" fromPresentationViewController: NULL];	
}

- (IBAction) playBirthdayRTSP {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/Birthday/HappyBirthday-rtsp.smil" fromPresentationViewController: NULL];	
}

- (IBAction) playEuros {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/Euros/Euros.smil" fromPresentationViewController: NULL];	
}

- (IBAction) playEurosRTSP {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/Euros/Euros-rtsp.smil" fromPresentationViewController: NULL];	
}

- (IBAction)playFlashlight {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/Flashlight/Flashlight-US.smil" fromPresentationViewController: NULL];	
}

- (IBAction)playFlashlightRTSP {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/Flashlight/Flashlight-US-rtsp.smil" fromPresentationViewController: NULL];	
}

- (IBAction)playNews {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/News/DanesV2-Desktop.smil" fromPresentationViewController: NULL];	
}

- (IBAction)playNewsRTSP {
	[self.delegate playPresentation:@"http://ambulantPlayer.org/Demos/News/DanesV2-Desktop-rtsp.smil" fromPresentationViewController: NULL];	
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
