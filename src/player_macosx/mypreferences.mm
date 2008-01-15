// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#include "mypreferences.h"
#import <Cocoa/Cocoa.h>
#include "ambulant/net/url.h"

void
mypreferences::install_singleton()
{
	set_preferences_singleton(new mypreferences);
	// XXX Workaround
	get_preferences()->load_preferences();
}

bool
mypreferences::load_preferences()
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
		@"", @"plugin_dir",
		[NSNumber numberWithBool: false], @"dynamic_content_control",
		nil];
	[prefs registerDefaults: defaultDefaults];
	m_parser_id = [[prefs stringForKey: @"parser_id"] cString];
	m_validation_scheme = [[prefs stringForKey: @"validation_scheme"] cString];
	m_do_namespaces = [prefs boolForKey: @"do_namespaces"];
	m_do_schema = [prefs boolForKey: @"do_schema"];
	m_validation_schema_full_checking = [prefs boolForKey: @"validation_schema_full_checking"];
	m_log_level = [prefs integerForKey: @"log_level"];
	m_use_plugins = [prefs boolForKey: @"use_plugins"];
	m_plugin_dir = [[prefs stringForKey: @"plugin_dir"] cString];
	m_prefer_ffmpeg = [prefs boolForKey: @"prefer_ffmpeg"];
	m_prefer_rtsp_tcp = [prefs boolForKey: @"prefer_rtsp_tcp"];
	m_strict_url_parsing = [prefs boolForKey: @"strict_url_parsing"];
	m_tabbed_links = [prefs boolForKey: @"tabbed_links"];
	m_dynamic_content_control = [prefs boolForKey: @"dynamic_content_control"];
	m_fullscreen = [prefs boolForKey: @"fullScreen"];
	save_preferences();
	return true;
}

bool
mypreferences::save_preferences()
{
	NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
	[prefs setObject: [NSString stringWithCString: m_parser_id.c_str()] forKey: @"parser_id"];
	[prefs setObject: [NSString stringWithCString: m_validation_scheme.c_str()] forKey: @"validation_scheme"];
	[prefs setBool: m_do_namespaces forKey: @"do_namespaces"];
	[prefs setBool: m_do_schema forKey: @"do_schema"];
	[prefs setBool: m_validation_schema_full_checking forKey: @"validation_schema_full_checking"];
	[prefs setInteger: m_log_level forKey: @"log_level"];
	[prefs setBool: m_use_plugins forKey: @"use_plugins"];
	[prefs setObject: [NSString stringWithCString: m_plugin_dir.c_str()] forKey: @"plugin_dir"];
	[prefs setBool: m_prefer_ffmpeg forKey: @"prefer_ffmpeg"];
	[prefs setBool: m_prefer_rtsp_tcp forKey: @"prefer_rtsp_tcp"];
	[prefs setBool: m_strict_url_parsing forKey: @"strict_url_parsing"];
	[prefs setBool: m_tabbed_links forKey: @"tabbed_links"];
	[prefs setBool: m_fullscreen forKey: @"fullScreen"];
	[prefs setBool: m_dynamic_content_control forKey: @"dynamic_content_control"];
	ambulant::net::url::set_strict_url_parsing(m_strict_url_parsing);
	return true;
}
