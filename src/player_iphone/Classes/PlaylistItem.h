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

#import <Foundation/Foundation.h>

@interface PlaylistItem : NSObject {
	NSString* title; // <meta name="title" content=.. /> 
	NSURL* url;
	NSData* poster_data; // contains CGImage
    NSString* author;
	NSString* description;
	NSString* duration;
	NSString* position_node;
	NSUInteger position_offset;
}
@property(nonatomic,retain) NSString* title;
@property(nonatomic,retain) NSURL* url;
@property(nonatomic,retain) NSData* poster_data;
@property(nonatomic,retain) NSString* author;
@property(nonatomic,retain) NSString* description;
@property(nonatomic,retain) NSString* duration;
@property(nonatomic,retain) NSString* position_node;
@property(nonatomic,assign) NSUInteger position_offset;

// initialize all fields
- (PlaylistItem*) initWithTitle: (NSString*) atitle
	url: (NSURL*) ansurl
	image_data: (NSData*) ans_image_data
    author: (NSString*) ans_author
	description: (NSString*) ans_description
	duration: (NSString*) ans_dur
	last_node_repr: (NSString*) alast_node_repr
	position_offset: (NSUInteger) aposition;
	
// compare with another PlaylistItem
- (bool) equalsPlaylistItem: (PlaylistItem*) playlistitem;

// Next 2 methods are provided for use by NSKeyedArchiver
-(void) encodeWithCoder: (NSCoder*) encoder;
-(id) initWithCoder: (NSCoder*) decoder;

@end
