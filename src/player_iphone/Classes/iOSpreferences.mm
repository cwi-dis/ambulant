// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/net/url.h"
#include "iOSpreferences.h"
#import <ImageIO/ImageIO.h>

using namespace ambulant;

iOSpreferences*  iOSpreferences::s_preferences = 0;

iOSpreferences::iOSpreferences()
:	common::preferences(),
    m_loaded(false),
    m_auto_center(true),
    m_auto_resize(true),
    m_normal_exit(false),
    m_favorites(NULL),
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
	if (s_preferences != NULL) {
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
	if (s_preferences == NULL) {
		set_preferences_singleton(new iOSpreferences);
	}
	// XXX Workaround
	get_preferences()->load_preferences();
}

ambulant::iOSpreferences*
iOSpreferences::get_preferences() {
	if (s_preferences == NULL) {
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
		[NSNumber numberWithBool: true], @"autoCenter",
		[NSNumber numberWithBool: true], @"autoResize",
		[NSNumber numberWithBool: true], @"HUDautoHide",
		[NSNumber numberWithBool: true], @"HUDshortTap",
		[NSNumber numberWithBool: true], @"normalExit",
		@"", @"plugin_dir",
		[NSNumber numberWithBool: false], @"dynamic_content_control",
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
	m_plugin_path = [[prefs stringForKey: @"plugin_dir"] UTF8String];
	m_prefer_ffmpeg = [prefs boolForKey: @"prefer_ffmpeg"];
	m_prefer_rtsp_tcp = [prefs boolForKey: @"prefer_rtsp_tcp"];
	m_strict_url_parsing = [prefs boolForKey: @"strict_url_parsing"];
	m_tabbed_links = [prefs boolForKey: @"tabbed_links"];
	m_dynamic_content_control = [prefs boolForKey: @"dynamic_content_control"];
	m_fullscreen = [prefs boolForKey: @"fullScreen"];
	m_auto_center = [prefs boolForKey: @"autoCenter"];
	m_auto_resize = [prefs boolForKey: @"autoResize"];
	m_hud_auto_hide = [prefs boolForKey: @"HUDautoHide"];
	m_hud_short_tap = [prefs boolForKey: @"HUDshortTap"];
	m_normal_exit = [prefs boolForKey: @"normalExit"];

	// Check version of preferences, to see whether we should
    // decode history/favorites
    NSArray* history = NULL;
    NSArray* favorites = NULL;
	NSString* version = [prefs stringForKey:@"version"];
	version = [prefs stringForKey:@"version"];
	if ([version isEqualToString: AM_IOS_PLAYLISTVERSION]) {
        // Decode history
        NSData* history_archive = [prefs objectForKey:@"history"];
        if ([history_archive length] != 1) {
            history = [NSKeyedUnarchiver unarchiveObjectWithData:history_archive];
        }
        // Decode Favorites
        NSData* favorites_archive = [prefs objectForKey:@"favorites"];
        if ([favorites_archive length] != 1) {
            favorites = [NSKeyedUnarchiver unarchiveObjectWithData:favorites_archive];
	}
    } else {
		history = NULL;
		favorites = NULL;
	}
	m_history = new ambulant::Playlist(history);
	m_favorites = new ambulant::Playlist(favorites);
	
	save_preferences();
	[pool release];
	return true;
}

bool
iOSpreferences::save_preferences()
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
	[prefs setObject: [NSString stringWithUTF8String: m_plugin_path.c_str()] forKey: @"plugin_dir"];
	[prefs setBool: m_prefer_ffmpeg forKey: @"prefer_ffmpeg"];
	[prefs setBool: m_prefer_rtsp_tcp forKey: @"prefer_rtsp_tcp"];
	[prefs setBool: m_strict_url_parsing forKey: @"strict_url_parsing"];
	[prefs setBool: m_tabbed_links forKey: @"tabbed_links"];
	[prefs setBool: m_fullscreen forKey: @"fullScreen"];
	[prefs setBool: m_auto_center forKey: @"autoCenter"];
	[prefs setBool: m_auto_resize forKey: @"autoResize"];
	[prefs setBool: m_hud_auto_hide forKey: @"HUDautoHide"];
	[prefs setBool: m_hud_short_tap forKey: @"HUDshortTap"];
	[prefs setBool: m_normal_exit forKey: @"normalExit"];
	if (m_favorites != NULL) {
		NSArray* favorites = m_favorites->get_playlist();
		NSData* data = [NSKeyedArchiver archivedDataWithRootObject:favorites];
		[prefs setObject: data forKey:@"favorites"];
		[prefs setObject: [NSString stringWithString: m_favorites->get_version()] forKey:@"version"];
	}
	if (m_history != NULL) {
		NSArray* history = m_history->get_playlist();
		NSData* data = [NSKeyedArchiver archivedDataWithRootObject:history];
		[prefs setObject: data forKey:@"history"];
		[prefs setObject: [NSString stringWithString: m_history->get_version()] forKey:@"version"];
	}
	[prefs setBool: m_dynamic_content_control forKey: @"dynamic_content_control"];
	[prefs synchronize];
	ambulant::net::url::set_strict_url_parsing(m_strict_url_parsing);

	[pool release];
	return true;
}

const std::string
iOSpreferences::repr()
{
	std::string rv = "iOSpreferences:";
	rv += " auto_center="; rv += m_auto_center ? "true" : "false";
	rv += ", auto_resize="; rv += m_auto_resize ? "true" : "false";
	rv += ", normal_exit="; rv += m_normal_exit ? "true" : "false";
	rv += ", hud_auto_hide="; rv += m_hud_auto_hide ? "true" : "false";
	rv += ", hud_short_tap="; rv += m_hud_short_tap ? "true" : "false";
	rv += "; ";
	rv += common::preferences::repr();
	return rv;
}


ambulant::Playlist::Playlist(NSArray* ansarray)
{
	am_ios_version = AM_IOS_PLAYLISTVERSION;
	am_ios_playlist = [[NSMutableArray arrayWithArray: ansarray] retain];
}

ambulant::Playlist::~Playlist()
{
	[am_ios_version release];
	[am_ios_playlist release];
}

void
ambulant::Playlist::insert_item_at_index (PlaylistItem* item, NSUInteger index)
{
	if (item == NULL) {
		return;
	}
	[item retain];
	if (index >= [am_ios_playlist count]) {
		[am_ios_playlist addObject: (NSObject*) item];
	} else {
		[am_ios_playlist insertObject: item atIndex: index];
	}

}

void
ambulant::Playlist::remove_last_item ()
{
	if ([am_ios_playlist count] > 0) {
		[am_ios_playlist removeLastObject];
	}
}

void
ambulant::Playlist::replace_last_item (PlaylistItem* new_item)
{
	[am_ios_playlist replaceObjectAtIndex:[am_ios_playlist count] - 1 withObject: new_item];
}

PlaylistItem*
ambulant::Playlist::get_last_item()
{
	if (am_ios_playlist == NULL || [am_ios_playlist count] == 0) {
		return NULL;
	} else {
		return (PlaylistItem*) [am_ios_playlist objectAtIndex: 0];
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

void
ambulant::Playlist::remove_playlist_item_at_index(NSUInteger idx)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (idx < [am_ios_playlist count]) {
		[am_ios_playlist removeObjectAtIndex: idx];
	}
	[pool release];
}




