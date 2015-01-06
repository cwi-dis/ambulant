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

#include "ambulant/common/preferences.h"
#include "PlaylistItem.h"

namespace ambulant {
	
#define AM_IOS_PLAYLISTVERSION @"0.5" // Needs to be updated when Playlist format changes 
	
	
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
	
	bool m_auto_center;     // Center the player on the screen
	bool m_auto_resize;     // Resize the player to fit the screen
	bool m_normal_exit;     // True if previous exit was normal, used for crash prevention
	bool m_hud_auto_hide;   // Does the HUD controls disappear automatically?
	bool m_hud_short_tap;   // Flip meaning of short and long tap
	Playlist* m_favorites;
	Playlist* m_history;
	
  private:
	static iOSpreferences* s_preferences; // singleton

}; // class iOSpreferences

} // namespace ambulant
