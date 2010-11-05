//
//  iOSpreferences.mm
//  player_iphone
//
//  Created by Kees Blom on 10/5/10.
//  Copyright 2010 Stg.CWI. All rights reserved.
//
#include "ambulant/net/url.h"

#include "iOSpreferences.h"

using namespace ambulant;

iOSpreferences*  iOSpreferences::s_preferences = 0;

iOSpreferences::iOSpreferences()
	:	common::preferences(),
		m_loaded(false),
		m_auto_center(false),
		m_auto_resize(false),
		m_history(NULL)
		{}

iOSpreferences::~iOSpreferences()
{
	if (m_history != NULL) {
		delete m_history;
		m_history = NULL;
	}
}

void
iOSpreferences::set_preferences_singleton(iOSpreferences *prefs) {
	if (s_preferences != 0) {
//		ambulant::lib::logger::get_logger()->debug("Programmer error: preferences singleton already set");
		return;
	}
	s_preferences = prefs;
}

void
iOSpreferences::delete_preferences_singleton()
{
	if (s_preferences != NULL) {
		delete s_preferences;
		s_preferences = NULL;
	}
}

void
iOSpreferences::install_singleton()
{
	set_preferences_singleton(new iOSpreferences);
	// XXX Workaround
	get_preferences()->load_preferences();
}

ambulant::iOSpreferences*
iOSpreferences::get_preferences() {
	if (s_preferences == 0) {
		s_preferences =  new iOSpreferences();
	}
	return s_preferences;
}

bool
iOSpreferences::load_preferences()
{
	if (m_loaded) {
		return m_loaded;
	}
	m_loaded = true;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
	NSDictionary *defaultDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
									 @"any", @"parser_id",
									 @"auto", @"validation_scheme",
									 [NSNumber numberWithBool: true], @"do_namespaces",
									 [NSNumber numberWithBool: false], @"do_schema",
									 [NSNumber numberWithBool: false], @"validation_schema_full_checking",
									 [NSNumber numberWithInt: 2], @"log_level",
									 [NSNumber numberWithBool: true], @"use_plugins",
									 [NSNumber numberWithBool: true], @"prefer_ffmpeg",
									 [NSNumber numberWithBool: false], @"prefer_rtsp_tcp",
									 [NSNumber numberWithBool: false], @"strict_url_parsing",
									 [NSNumber numberWithBool: false], @"tabbed_links",
									 [NSNumber numberWithBool: false], @"fullScreen",
									 [NSNumber numberWithBool: false], @"autoCenter",
									 [NSNumber numberWithBool: false], @"autoResize",
									 @"", @"plugin_dir",
									 [NSNumber numberWithBool: false], @"dynamic_content_control",
									 @"Welcome.smil", @"last_used",
									 AM_IOS_PLAYLISTVERSION, @"version",
									 [NSData dataWithBytes:"X" length:1], @"history",
									 nil];
	[prefs registerDefaults: defaultDefaults];
	m_parser_id = [[prefs stringForKey: @"parser_id"] UTF8String];
	m_validation_scheme = [[prefs stringForKey: @"validation_scheme"] UTF8String];
	m_do_namespaces = [prefs boolForKey: @"do_namespaces"];
	m_do_schema = [prefs boolForKey: @"do_schema"];
	m_validation_schema_full_checking = [prefs boolForKey: @"validation_schema_full_checking"];
	m_log_level = (int)[prefs integerForKey: @"log_level"];
	m_use_plugins = [prefs boolForKey: @"use_plugins"];
	m_plugin_dir = [[prefs stringForKey: @"plugin_dir"] UTF8String];
	m_prefer_ffmpeg = [prefs boolForKey: @"prefer_ffmpeg"];
	m_prefer_rtsp_tcp = [prefs boolForKey: @"prefer_rtsp_tcp"];
	m_strict_url_parsing = [prefs boolForKey: @"strict_url_parsing"];
	m_tabbed_links = [prefs boolForKey: @"tabbed_links"];
	m_dynamic_content_control = [prefs boolForKey: @"dynamic_content_control"];
	m_fullscreen = [prefs boolForKey: @"fullScreen"];
	m_auto_center = [prefs boolForKey: @"autoCenter"];
	m_auto_resize = [prefs boolForKey: @"autoResize"];
	m_last_used = [prefs stringForKey:@"last_used"];
	// history is archived
	NSData* history_archive = [prefs objectForKey:@"history"];
	NSArray* history = NULL;
	if ([history_archive length] != 1) {
		history = [NSKeyedUnarchiver unarchiveObjectWithData:history_archive];
	}
	NSString* version = [prefs stringForKey:@"version"];
	if ( ! [version isEqualToString: AM_IOS_PLAYLISTVERSION]) {
		if (history != NULL) {
			[history release];
			history = NULL;
		}
	}
	m_history = new ambulant::Playlist(history);
	
	save_preferences();
	[pool release];
	return true;
}

bool iOSpreferences::save_preferences()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
	[prefs setObject: [NSString stringWithUTF8String: m_parser_id.c_str()] forKey: @"parser_id"];
	[prefs setObject: [NSString stringWithUTF8String: m_validation_scheme.c_str()] forKey: @"validation_scheme"];
	[prefs setBool: m_do_namespaces forKey: @"do_namespaces"];
	[prefs setBool: m_do_schema forKey: @"do_schema"];
	[prefs setBool: m_validation_schema_full_checking forKey: @"validation_schema_full_checking"];
	[prefs setInteger: m_log_level forKey: @"log_level"];
	[prefs setBool: m_use_plugins forKey: @"use_plugins"];
	[prefs setObject: [NSString stringWithUTF8String: m_plugin_dir.c_str()] forKey: @"plugin_dir"];
	[prefs setBool: m_prefer_ffmpeg forKey: @"prefer_ffmpeg"];
	[prefs setBool: m_prefer_rtsp_tcp forKey: @"prefer_rtsp_tcp"];
	[prefs setBool: m_strict_url_parsing forKey: @"strict_url_parsing"];
	[prefs setBool: m_tabbed_links forKey: @"tabbed_links"];
	[prefs setBool: m_fullscreen forKey: @"fullScreen"];
	[prefs setBool: m_auto_center forKey: @"autoCenter"];
	[prefs setBool: m_auto_resize forKey: @"autoResize"];
	[prefs setObject: m_last_used forKey: @"last_used"];
	NSArray* history = m_history->get_playlist();
	NSData* data = [NSKeyedArchiver archivedDataWithRootObject:history];
	[prefs setObject: data forKey:@"history"];
	[prefs setObject: [NSString stringWithString: m_history->get_version()] forKey:@"version"];
	
	[prefs setBool: m_dynamic_content_control forKey: @"dynamic_content_control"];
	[prefs synchronize];
	ambulant::net::url::set_strict_url_parsing(m_strict_url_parsing);
	[pool release];
	return true;
}
@implementation PlaylistItem
@synthesize ns_title, ns_url, ns_description, ns_dur, position;
- (PlaylistItem*) initWithTitle: (NSString*) atitle
				   url: (NSURL*) ans_url
				 image: (id) acg_image
		   description: (NSString*) ans_description
			  duration: (NSString*) ans_dur
			  position: (NSUInteger) aposition
{
	ns_title = atitle;
	ns_url = ans_url;
	cg_image = acg_image;
	ns_description = ans_description;
	ns_dur = ans_dur;
	position = aposition;
	return self;
}

- (bool) equalsPlaylistItem: (PlaylistItem*) playlistitem
{
	if ( ! [[self ns_title] compare: [playlistitem ns_title]] == NSOrderedSame) {
		return false;
	}
	return true;
}
	
- (void) encodeWithCoder: (NSCoder*) encoder	
{
	[encoder encodeObject:ns_title forKey:@"Ns_title"];
	[encoder encodeObject:ns_url forKey:@"Ns_url"];
//	[encoder encodeObject:cg_image forKey:@"Cg_image"];
	[encoder encodeObject:ns_description forKey:@"Ns_description"];
	[encoder encodeObject:ns_dur forKey:@"Ns_dur"];
//	[encoder encodeObject:position forKey:@"Position"];
}
	
-(id) initWithCoder: (NSCoder*) decoder
{
	self.ns_title = [decoder decodeObjectForKey:@"Ns_title"];
	self.ns_url = [decoder decodeObjectForKey:@"Ns_url"];
//	self.cg_image = [decoder decodeObjectForKey:@"Cg_image"];
	self.ns_description = [decoder decodeObjectForKey:@"Ns_description"];
	self.ns_dur = [decoder decodeObjectForKey:@"Ns_dur"];
//	self.position = [decoder decodeObjectForKey:@"Position"];
	return self;
}

@end

ambulant::Playlist::Playlist(NSArray* ansarray)
{
	am_ios_version = [NSString stringWithString: AM_IOS_PLAYLISTVERSION];
	am_ios_playlist = [[NSMutableArray arrayWithArray: ansarray] retain];
}

ambulant::Playlist::~Playlist()
{
//	[am_ios_playlist enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) { [(NSObject*) obj release]; }];
	[am_ios_version release];
	[am_ios_playlist release];
}

void
ambulant::Playlist::add_item(PlaylistItem* item)
{
	assert(am_ios_playlist);
	[am_ios_playlist addObject:(NSObject*) item];
}

void
ambulant::Playlist::delete_item(PlaylistItem* item)
{
}

ambulant::PlaylistItem*
ambulant::Playlist::get_last_item()
{
	if (am_ios_playlist == NULL || [am_ios_playlist count] == 0) {
		return NULL;
	} else {
		return (ambulant::PlaylistItem*) [am_ios_playlist objectAtIndex:[am_ios_playlist count] - 1];
	}

}

NSString*
ambulant::Playlist::get_version()
{
	return am_ios_version;
}

NSArray*
ambulant::Playlist::get_playlist()
{
	return [NSArray arrayWithArray: am_ios_playlist];
}




