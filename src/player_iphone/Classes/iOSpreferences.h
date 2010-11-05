//
//  iOSpreferences.h
//  player_iphone
//
//  Created by Kees Blom on 10/5/10.
//  Copyright 2010 Stg.CWI. All rights reserved.
//

#include "ambulant/common/preferences.h"

@interface PlaylistItem : NSObject {
	NSString* ns_title;
	NSURL* ns_url;
	id cg_image; //CGImageRef
	NSString* ns_description;
	NSString* ns_dur;
	NSUInteger position;
}
@property(nonatomic,retain) NSString* ns_title;
@property(nonatomic,retain) NSURL* ns_url;
//@property(nonatomic,retain) CGImageRef cg_image;
@property(nonatomic,retain) NSString* ns_description;
@property(nonatomic,retain) NSString* ns_dur;
@property(nonatomic,assign) NSUInteger position;

// initialize all fields
- (PlaylistItem*) initWithTitle: (NSString*) atitle
				   url: (NSURL*) ansurl
				 image: (id) acg_image // CGImageRef
		   description: (NSString*) ans_description
			  duration: (NSString*) ans_dur
			  position: (NSUInteger) aposition;
// compare with another PlaylistItem
- (bool) equalsPlaylistItem: (PlaylistItem*) playlistitem;

// Next 2 methods are provided for use by NSKeyedArchiver
-(void) encodeWithCoder: (NSCoder*) encoder;
-(id) initWithCoder: (NSCoder*) decoder;

@end

namespace ambulant {
	
#define AM_IOS_PLAYLISTVERSION @"0.1" // Needs to be updated when Playlist format changes 
	
	
class Playlist {
public:
	Playlist(NSArray* ansarray);
	~Playlist();
	void add_item(PlaylistItem* item);
	void delete_item(PlaylistItem* item);
	PlaylistItem* get_last_item();
	NSString* get_version();
	NSArray* get_playlist();
	
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
	
	bool m_loaded;
	
	/// iOs player auto center
	bool m_auto_center;
	/// iOs player auto resize
	bool m_auto_resize;
	
	NSString* m_last_used;
	
	Playlist* m_history;
	
private:
	static iOSpreferences* s_preferences; // singleton

}; // class iOSpreferences

} // namespace ambulant
