//
//  cg_preferences.mm
//  player_iphone
//
//  Created by Kees Blom on 10/5/10.
//  Copyright 2010 Stg.CWI. All rights reserved.
//
#include "ambulant/net/url.h"

#include "cg_preferences.h"

using namespace ambulant::gui::cg;

cg_preferences*  ambulant::gui::cg::cg_preferences::s_preferences = 0;


ambulant::gui::cg::cg_preferences::cg_preferences()
	:	ambulant::common::preferences(),
		m_auto_center(false),
		m_auto_resize(false)
		{}
	
void
cg_preferences::install_singleton()
{
	set_preferences_singleton(new cg_preferences);
	// XXX Workaround
	get_preferences()->load_preferences();
}

cg_preferences*
ambulant::gui::cg::cg_preferences::get_preferences() {
	if (s_preferences == 0) {
		s_preferences =  new cg_preferences;
	}
	return s_preferences;
}

bool
cg_preferences::load_preferences()
{
	NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
	NSDictionary *defaultDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
									 @"any", @"parser_id",
									 @"auto", @"validation_scheme",
									 [NSNumber numberWithBool: true], @"do_namespaces",
									 [NSNumber numberWithBool: false], @"do_schema",
									 [NSNumber numberWithBool: false], @"validation_schema_full_checking",
									 [NSNumber numberWithInt: 2], @"log_level",
									 [NSNumber numberWithBool: true], @"use_plugins",
									 [NSNumber numberWithBool: false], @"prefer_ffmpeg",
									 [NSNumber numberWithBool: false], @"prefer_rtsp_tcp",
									 [NSNumber numberWithBool: false], @"strict_url_parsing",
									 [NSNumber numberWithBool: false], @"tabbed_links",
									 [NSNumber numberWithBool: false], @"fullScreen",
									 [NSNumber numberWithBool: false], @"autoCenter",
									 [NSNumber numberWithBool: false], @"autoResize",
									 @"", @"plugin_dir",
									 [NSNumber numberWithBool: false], @"dynamic_content_control",
									 @"Welcome.smil", @"last_used",
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
	save_preferences();
	return true;
}

bool cg_preferences::save_preferences()
{
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
	[prefs setBool: m_auto_resize forKey: @"autoResize"];
	[prefs setObject: m_last_used forKey: @"last_used"];
	
	[prefs setBool: m_dynamic_content_control forKey: @"dynamic_content_control"];
	ambulant::net::url::set_strict_url_parsing(m_strict_url_parsing);
	return true;
}

