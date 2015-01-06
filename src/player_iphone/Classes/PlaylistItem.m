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

#import "PlaylistItem.h"

@implementation PlaylistItem
@synthesize title;
@synthesize url;
@synthesize poster_data;
@synthesize author;
@synthesize description;
@synthesize duration;
@synthesize position_node;
@synthesize position_offset;

- (PlaylistItem*) initWithTitle: (NSString*) atitle
	url: (NSURL*) ans_url
	image_data: (NSData*) ans_image_data
    author: (NSString*) ans_author
	description: (NSString*) ans_description
	duration: (NSString*) ans_dur
	last_node_repr: (NSString*) alast_node_repr
	position_offset: (NSUInteger) aposition
{
	title = atitle;
	url = [ans_url retain];
	poster_data = ans_image_data;
	description = ans_description;
    author = ans_author;
	duration = ans_dur;
	if (alast_node_repr == NULL) {
		position_node = @"";
	} else {
		position_node = alast_node_repr;			
	}
	position_offset = aposition;
	return self;
}

- (bool) equalsPlaylistItem: (PlaylistItem*) playlistitem
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    // Note by Jack: this means two references to different positions in one document
    // compare as equal.
	BOOL rv = [[self url] isEqual: [playlistitem url]];
	[pool release];
	return rv; 
}
	
- (void) encodeWithCoder: (NSCoder*) encoder	
{
	[encoder encodeObject:title forKey:@"title"];
	[encoder encodeObject:url forKey:@"url"];
	if (poster_data != NULL) {
		[encoder encodeObject:poster_data forKey:@"poster_data"];
	}
    [encoder encodeObject:author forKey:@"author"];
	[encoder encodeObject:description forKey:@"description"];
	[encoder encodeObject:duration forKey:@"duration"];
	[encoder encodeObject:position_node forKey:@"position_node"];
    [encoder encodeInt:position_offset forKey:@"position_offset"];
}

- (id) initWithCoder: (NSCoder*) decoder
{
    // Note: the trick of using the accessor means we don't have to do the retain
	self.title = [decoder decodeObjectForKey:@"title"];
	self.url = [decoder decodeObjectForKey:@"url"];
	self.poster_data = [decoder decodeObjectForKey:@"poster_data"];
    self.author = [decoder decodeObjectForKey:@"author"];
	self.description = [decoder decodeObjectForKey:@"description"];
	self.duration = [decoder decodeObjectForKey:@"duration"];
	self.position_node = [decoder decodeObjectForKey:@"position_node"];
    self.position_offset = [decoder decodeIntForKey:@"position_offset"];
	return self;
}

- (void) dealloc {
	[title release];
	[url release];
	[poster_data release];
    [author release];
	[description release];
	[duration release];

	[super dealloc];
}

@end
