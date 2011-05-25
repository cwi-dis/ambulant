/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

#import "Presentation.h"

@implementation Presentation
@synthesize title, duration, description, poster_data;

-(id)initWithTitle:(NSString*) newTitle
	   poster_data:(NSData*) newPoster_data
		  duration:(NSString*) newDuration
	   description:(NSString*) newDescription
{
	self = [super init];
	if (self != nil) {
		self.title = newTitle;
		self.poster_data = newPoster_data;
		self.duration = newDuration;
		self.description = newDescription;
	}
	return self;
}

-(void) dealloc {
	self.title = nil;
	self.poster_data = nil;
	self.duration = nil;
	self.description = nil;
	[super dealloc];
}
@end
