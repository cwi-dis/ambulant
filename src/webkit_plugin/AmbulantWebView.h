//
//  AmbulantWebView.h
//  AmbulantWebKitPlugin
//
//  Created by Jack Jansen on 20-12-05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "../player_macosx/mainloop.h"

void set_statusline(void *view, const char *msg);

@interface AmbulantWebView : AmbulantView {
	NSDictionary *m_arguments;
	mainloop *m_mainloop;
	id container;
}

- (void)setArguments:(NSDictionary *)arguments;

- (void)startPlayer;
- (void)stopPlayer;
- (void)restartPlayer;
- (void)pausePlayer;
- (void)resumePlayer;
- (bool)ignoreResize;
- (id)container;
@end
