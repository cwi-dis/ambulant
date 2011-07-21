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

#import "PlaylistItem.h"

@implementation PlaylistItem
@synthesize title, url, description, duration, position_node, position_offset;
@synthesize poster_data;

- (PlaylistItem*) initWithTitle: (NSString*) atitle
	url: (NSURL*) ans_url
	image_data: (NSData*) ans_image_data
	description: (NSString*) ans_description
	duration: (NSString*) ans_dur
	last_node_repr: (NSString*) alast_node_repr
	position_offset: (NSUInteger) aposition
{
	title = atitle;
	url = [ans_url retain];
	poster_data = ans_image_data;
	description = ans_description;
	duration = ans_dur;
	if (alast_node_repr == NULL) {
		position_node = [NSString stringWithString: @""];
	} else {
		position_node = alast_node_repr;			
	}
	position_offset = aposition;
	return self;
}

- (bool) equalsPlaylistItem: (PlaylistItem*) playlistitem
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	BOOL rv = [[self url] isEqual: [playlistitem url]];
	[pool release];
	return rv; 
}
	
- (void) encodeWithCoder: (NSCoder*) encoder	
{
	[encoder encodeObject:title forKey:@"Ns_title"];
	[encoder encodeObject:url forKey:@"Ns_url"];
	if (poster_data != NULL) {
		[encoder encodeObject:poster_data forKey:@"Ns_image_data"];
	}
	[encoder encodeObject:description forKey:@"Ns_description"];
	[encoder encodeObject:duration forKey:@"Ns_dur"];
	[self.position_node retain];
	[encoder encodeObject:position_node forKey:@"Ns_lastnode"];
//TBD [encoder encodeObject:position_offset forKey:@"Position"];
}

- (id) initWithCoder: (NSCoder*) decoder
{
	self.title = [decoder decodeObjectForKey:@"Ns_title"];
	self.url = [decoder decodeObjectForKey:@"Ns_url"];
	self.poster_data = [decoder decodeObjectForKey:@"Ns_image_data"];
	self.description = [decoder decodeObjectForKey:@"Ns_description"];
	self.duration = [decoder decodeObjectForKey:@"Ns_dur"];
	self.position_node = [decoder decodeObjectForKey:@"Ns_lastnode"];
	[self.position_node retain];
//TBD self.position_offset = [decoder decodeObjectForKey:@"Position"];
	return self;
}

- (void) dealloc {
	[title release];
	[url release];
	[poster_data release];
	[description release];
	[duration release];

	[super dealloc];
}

@end
