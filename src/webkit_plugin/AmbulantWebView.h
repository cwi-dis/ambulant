/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
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
