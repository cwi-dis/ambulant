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

#include "ambulant/common/preferences.h"

@interface PlaylistItem : NSObject {
	NSString* ns_title; // <meta name="title" content=.. /> 
	NSURL* ns_url;
	NSData* ns_image_data; // contains CGImage
	NSString* ns_description;
	NSString* ns_dur;
	NSString* ns_last_node_repr;
	NSUInteger position;
}
@property(nonatomic,retain) NSString* ns_title;
@property(nonatomic,retain) NSURL* ns_url;
@property(nonatomic,retain) NSData* ns_image_data;
@property(nonatomic,retain) NSString* ns_description;
@property(nonatomic,retain) NSString* ns_dur;
@property(nonatomic,assign) NSString* ns_last_node_repr;
@property(nonatomic,assign) NSUInteger position;

// initialize all fields
- (PlaylistItem*) initWithTitle: (NSString*) atitle
	url: (NSURL*) ansurl
	image_data: (NSData*) ans_image_data
	description: (NSString*) ans_description
	duration: (NSString*) ans_dur
	last_node_repr: (NSString*) alast_node_repr
	position: (NSUInteger) aposition;
	
// compare with another PlaylistItem
- (bool) equalsPlaylistItem: (PlaylistItem*) playlistitem;

// Next 2 methods are provided for use by NSKeyedArchiver
-(void) encodeWithCoder: (NSCoder*) encoder;
-(id) initWithCoder: (NSCoder*) decoder;

@end

namespace ambulant {
	
#define AM_IOS_PLAYLISTVERSION @"0.3" // Needs to be updated when Playlist format changes 
	
	
class Playlist {
public:
	Playlist(NSArray* ansarray);
	~Playlist();
	
	// get the current version of PlaylistItem 
	NSString* get_version();
	// get an immutable copy of the playlist
	NSArray* get_playlist();
	// remove the PlaylistItem at the given index
	void remove_playlist_item_at_index(NSUInteger idx);
	// get and manipulate the last item
	PlaylistItem* get_last_item();
	void insert_item_at_index(PlaylistItem* item, NSUInteger index);
	void remove_last_item();
	void replace_last_item(PlaylistItem* new_last_item);
	
private:
	NSString* am_ios_version;
	NSMutableArray* am_ios_playlist; // PlaylistItem* objects
};



class iOSpreferences : public common::preferences {

protected:
	iOSpreferences();

public:
	~iOSpreferences();
	static void install_singleton();
	
	static iOSpreferences* get_preferences();
	
	static void set_preferences_singleton(iOSpreferences *prefs);
	
	static void delete_preferences_singleton();
	
	bool load_preferences();
	bool save_preferences();
	const std::string repr();
	
	bool m_loaded;
	
	/// iOs player auto center
	bool m_auto_center;
	/// iOs player auto resize
	bool m_auto_resize;
	/// crash protector
	bool m_normal_exit;
	/// HUD auto hide
	bool m_hud_auto_hide;
	/// HUD short tap
	bool m_hud_short_tap;
	Playlist* m_favorites;
	Playlist* m_history;
	
private:
	static iOSpreferences* s_preferences; // singleton

}; // class iOSpreferences

} // namespace ambulant
