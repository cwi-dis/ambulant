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

/* For Auto rotation in iOS 6. 
  From iOS release notes (https://developer.apple.com/library/ios/#releasenotes/General/RN-iOSSDK-6_0/_index.html):
 Autorotation is changing in iOS 6. In iOS 6, the shouldAutorotateToInterfaceOrientation: method of UIViewController is deprecated. In its place, you should use the supportedInterfaceOrientationsForWindow: and shouldAutorotate methods.
 More responsibility is moving to the app and the app delegate. Now, iOS containers (such as UINavigationController) do not consult their children to determine whether they should autorotate. By default, an app and a view controller’s supported interface orientations are set to UIInterfaceOrientationMaskAll for the iPad idiom and UIInterfaceOrientationMaskAllButUpsideDown for the iPhone idiom.
 A view controller’s supported interface orientations can change over time—even an app’s supported interface orientations can change over time. The system asks the top-most full-screen view controller (typically the root view controller) for its supported interface orientations whenever the device rotates or whenever a view controller is presented with the full-screen modal presentation style. Moreover, the supported orientations are retrieved only if this view controller returns YES from its shouldAutorotate method. The system intersects the view controller’s supported orientations with the app’s supported orientations (as determined by the Info.plist file or the app delegate’s application:supportedInterfaceOrientationsForWindow: method) to determine whether to rotate.
 The system determines whether an orientation is supported by intersecting the value returned by the app’s supportedInterfaceOrientationsForWindow: method with the value returned by the supportedInterfaceOrientations method of the top-most full-screen controller.
 The setStatusBarOrientation:animated: method is not deprecated outright. It now works only if the supportedInterfaceOrientations method of the top-most full-screen view controller returns 0. This makes the caller responsible for ensuring that the status bar orientation is consistent.

    More relevant information on: https://devforums.apple.com/message/744384, especially the contribution by 
    the Apple employee 'behrens' and on http://donniedeboer.blogspot.nl/2012/09/ios-6-auto-rotation.html
    (window.rootViewController must be set).
 */

@interface AmbulantTabBarController : UITabBarController

@end
