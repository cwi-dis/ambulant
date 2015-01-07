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

#include <iostream>
#import <Foundation/Foundation.h>
#include "AmbulantAppDelegate.h" // for document_stopped
#include "mainloop.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/node.h"
#ifdef WITH_CG
#include "ambulant/gui/cg/cg_gui.h"
#else
#include "ambulant/gui/cocoa/cocoa_gui.h"
#endif
#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_factory.h"
#endif
#ifdef NONE_PLAYER
#include "ambulant/gui/none/none_gui.h"
#endif
#include "ambulant/net/datasource.h"
#include "ambulant/net/posix_datasource.h"
//#define WITH_STDIO_DATASOURCE
#ifdef WITH_STDIO_DATASOURCE
#include "ambulant/net/stdio_datasource.h"
#endif
#ifdef WITH_FFMPEG
#include "ambulant/net/ffmpeg_factory.h"
#endif
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/common/plugin_engine.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

mainloop::mainloop(const char *urlstr, void *view, ambulant::common::embedder *app)
:   common::gui_player(),
	m_view(view),
	m_gui_screen(NULL),
	m_nsurl(NULL),
	m_current_item(NULL),
	m_last_node_started(NULL),
	m_no_stopped_callbacks(false)
{

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	set_embedder(app);
	AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop(0x%x): created", (void*)this);
    // Set systemComponent values that are relevant
    smil2::test_attrs::set_default_tests_attrs();
    smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("Standalone"), true);
 	init_factories();
	
	init_plugins();

    // Order the factories according to the preferences
    iOSpreferences *prefs = iOSpreferences::get_preferences();
	prefs->load_preferences();
    get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererOpen"));
    if (!prefs->m_prefer_ffmpeg)
        get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererAVFoundation"))    ;   
	
	// check trivial preconditions
	// assert (urlstr != NULL && view != NULL && app != NULL); 
	if (urlstr == NULL || view == NULL || app == NULL) {
		[pool release];
		return;
	}
	ambulant::net::url url = ambulant::net::url::from_url(urlstr);
	m_doc = create_document(url);
	if (!m_doc) {
		lib::logger::get_logger()->error(gettext("%s: Cannot build DOM tree"), urlstr);
		return;
	}
	lib::logger::get_logger()->debug(" creating smil2 player, %s", prefs->repr().c_str());	
	m_player = common::create_smil2_player(m_doc, this, m_embedder);

	if (m_player) {
		m_player->set_feedback(this);
		m_player->initialize();

		const std::string& id = url.get_ref();
		if (id != "") {
			const ambulant::lib::node *node = m_doc->get_node(id);
			if (!node)
				lib::logger::get_logger()->warn(gettext("%s: node ID not found"), id.c_str());
			goto_node(node);
		}
		Playlist* history = prefs->m_history;
		if (history == NULL) {
			history = new Playlist(NULL);
		}
		if (history != NULL) {
			m_nsurl = [NSURL URLWithString:[NSString stringWithUTF8String:urlstr]];
			if (m_nsurl == NULL) {
				m_nsurl = [NSURL fileURLWithPath:[NSString stringWithUTF8String:urlstr]];
			}
			[m_nsurl retain];
			NSString* title = get_meta_content("title");
			if ([title compare:@""] == NSOrderedSame) {
				title = [m_nsurl lastPathComponent];
				NSLog(@"title=%@", title);
				if (title == nil || [title compare:@""] == NSOrderedSame) {
					title = [NSString stringWithUTF8String: urlstr];
				}
				[title retain];
			}
			NSData* image_data = NULL; // contains data for CGImage
			NSString* poster = get_meta_content("poster");
			if ([poster compare:@""] != NSOrderedSame) {
				net::url poster_url = net::url::from_url([poster cStringUsingEncoding: NSUTF8StringEncoding]);
				const char* abs_poster_charp = NULL;
				if ( ! poster_url.is_absolute()) {
					// convert to asolute URL
					net::url base_url = m_doc->get_src_url();
					net::url abs_poster_url = poster_url.join_to_base(base_url);
					std::string abs_poster_string = abs_poster_url.get_protocol();
					if (abs_poster_string != "file") {
						abs_poster_string += "://"+abs_poster_url.get_host();
					} else abs_poster_string += "://";
					abs_poster_string += abs_poster_url.get_path();
					abs_poster_charp = strdup(abs_poster_string.c_str());
				} else {
					abs_poster_charp = poster_url.get_url().c_str();
				}
				CFStringRef posterCFString = CFStringCreateWithCString(NULL, abs_poster_charp , kCFStringEncodingASCII);
				free((void*) abs_poster_charp);
				CFStringRef posterCFStringX = CFURLCreateStringByAddingPercentEscapes(NULL, posterCFString, NULL, NULL, kCFStringEncodingUTF8);
				CFRelease(posterCFString);
				CFURLRef posterCFURL = CFURLCreateWithString(NULL, posterCFStringX, NULL);
				CFRelease(posterCFStringX);
				if (posterCFURL != NULL) {
					CGImageSourceRef poster_src = CGImageSourceCreateWithURL(posterCFURL, NULL);
					CFRelease(posterCFURL);
					if (poster_src != NULL) {
						CGImageRef cg_image = CGImageSourceCreateImageAtIndex(poster_src, 0, NULL);
						if (cg_image != NULL)  {
							UIImage *img = [UIImage imageWithCGImage:cg_image];
							image_data = UIImagePNGRepresentation(img);
							[image_data retain];
							CFRelease(cg_image);
						}
						CFRelease(poster_src);
					}				
				}
			}
			[poster release];
            NSString *author = get_meta_content("author");
			NSString* description = get_meta_content("description");
			if ([description compare: @""] == NSOrderedSame) {
				description = [m_nsurl absoluteString];
			}
		
			NSString* dur = get_meta_content("duration");
//			if ([dur compare: @""] == NSOrderedSame) {
//				dur = [[NSString stringWithUTF8String:"indefinite"] retain];
//			}
			NSUInteger position_offset = 0;
			PlaylistItem* new_item = [[PlaylistItem alloc] 
                initWithTitle:title 
                url:m_nsurl 
                image_data:image_data
                author: author
                description:description 
                duration:dur 
                last_node_repr:NULL 
                position_offset:position_offset];
			PlaylistItem* last_item = history->get_last_item();
			if (last_item == NULL || ! [new_item equalsPlaylistItem: last_item]) {
				history->insert_item_at_index(new_item, 0);
				m_current_item = new_item;
			} else if (last_item != NULL) {
				// new item not stored
				[new_item release];
                m_current_item = last_item;
			}
			prefs->m_normal_exit = false;
			prefs->m_history = history;
		}
		prefs->save_preferences();
	}
	[pool release];
}

void
mainloop::init_factories()
{
	init_playable_factory();
	init_window_factory();
	init_datasource_factory();
	init_parser_factory();
	init_node_factory();
	init_state_component_factory();
}

void
mainloop::init_playable_factory()
{
	common::global_playable_factory *pf = common::get_global_playable_factory();
	set_playable_factory(pf);
#ifndef NONE_PLAYER
#ifdef WITH_CG
	pf->add_factory(gui::cg::create_cg_dsvideo_playable_factory(this, NULL));
	pf->add_factory(gui::cg::create_cg_fill_playable_factory(this, NULL));
//	pf->add_factory(gui::cg::create_cg_html_playable_factory(this, NULL));
	pf->add_factory(gui::cg::create_cg_image_playable_factory(this, NULL));
//	pf->add_factory(gui::cg::create_cg_ink_playable_factory(this, NULL));
	pf->add_factory(gui::cg::create_cg_smiltext_playable_factory(this, NULL));
	pf->add_factory(gui::cg::create_cg_text_playable_factory(this, NULL));
//XXXX #define WITH_AVFOUNDATION
#ifdef	WITH_AVFOUNDATION
	pf->add_factory(gui::cg::create_cg_avfoundation_video_playable_factory(this, NULL));
#endif//WITH_AVFOUNDATION
#else//WITH_CG
	pf->add_factory(gui::cocoa::create_cocoa_audio_playable_factory(this, NULL));
	pf->add_factory(gui::cocoa::create_cocoa_dsvideo_playable_factory(this, NULL));
	pf->add_factory(gui::cocoa::create_cocoa_fill_playable_factory(this, NULL));
	pf->add_factory(gui::cocoa::create_cocoa_html_playable_factory(this, NULL));
	pf->add_factory(gui::cocoa::create_cocoa_image_playable_factory(this, NULL));
	pf->add_factory(gui::cocoa::create_cocoa_ink_playable_factory(this, NULL));
	pf->add_factory(gui::cocoa::create_cocoa_smiltext_playable_factory(this, NULL));
	pf->add_factory(gui::cocoa::create_cocoa_text_playable_factory(this, NULL));
#ifndef __LP64__
	pf->add_factory(gui::cocoa::create_cocoa_video_playable_factory(this, NULL));
#endif
#endif
#ifdef WITH_SDL
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add factory for SDL");
	pf->add_factory(gui::sdl::create_sdl_playable_factory(this));      
#endif // WITH_SDL
#endif // NONE_PLAYER
}

void
mainloop::init_window_factory()
{
#ifdef NONE_PLAYER
	// Replace the real window factory with a none_window_factory instance.
	set_window_factory(gui::none::create_none_window_factory());
#else
#ifdef WITH_CG
	set_window_factory(gui::cg::create_cg_window_factory(m_view));
#else
	set_window_factory(gui::cocoa::create_cocoa_window_factory(m_view));
#endif
#endif // NONE_PLAYER
}

void
mainloop::init_datasource_factory()
{
	net::datasource_factory *df = new net::datasource_factory();
	set_datasource_factory(df);
#ifndef NONE_PLAYER
#ifdef WITH_FFMPEG
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_video_datasource_factory");
	df->add_video_factory(net::get_ffmpeg_video_datasource_factory());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_datasource_factory");
	df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_decoder_finder");
	df->add_audio_decoder_finder(net::get_ffmpeg_audio_decoder_finder());
#ifdef WITH_RESAMPLE_DATASOURCE
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
#endif
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_raw_datasource_factory");
	df->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif // WITH_FFMPEG
#endif // NONE_PLAYER
#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add stdio_datasource_factory");
	df->add_raw_factory(net::create_stdio_datasource_factory());
#endif
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add posix_datasource_factory");
	df->add_raw_factory(net::create_posix_datasource_factory());
}

void
mainloop::init_parser_factory()
{
	set_parser_factory(lib::global_parser_factory::get_parser_factory());	
}

mainloop::~mainloop()
{
	lib::logger::get_logger()->debug("mainloop::~mainloop: %s", m_url.get_url().c_str());
	ambulant::iOSpreferences* prefs = ambulant::iOSpreferences::get_preferences();

	// update the last_item pointer in the history
    PlaylistItem* last_item = iOSpreferences::get_preferences()->m_history->get_last_item();
    if (last_item) {
        if (m_last_node_started) {
            lib::xml_string last_node_repr = m_last_node_started->get_xpath();
            NSString* position_node = [NSString stringWithUTF8String: last_node_repr.c_str()];
            last_item.position_node = position_node;
        } else {
            last_item.position_node = NULL;
        }
    }
	// We need to delete gui_player::m_player before deleting m_doc, because the
	// timenode graph in the player has referrences to the node graph in m_doc.
    if (m_player) {
        m_player->terminate();
        m_player->release();
        m_player = NULL;
    }
	delete m_doc;
	m_doc = NULL;
	delete m_gui_screen;
//	delete m_window_factory;
	if (m_nsurl != NULL) {
//XX	[m_nsurl release]; //XX crashes when called from Safari
		m_nsurl =  NULL;
	}
	m_current_item = NULL;
	
	if (prefs != NULL) {
		prefs->m_normal_exit = true;
		prefs->save_preferences();
		ambulant::iOSpreferences::delete_preferences_singleton();
	}
	
}

void
mainloop::restart(bool reparse)
{
	m_no_stopped_callbacks = true;
	
	bool playing = is_play_active();
	bool pausing = is_pause_active();

	stop();
	
    if (m_player) {
        m_player->terminate();
        m_player->release();
        m_player = NULL;
    }
	if (reparse) {
		m_doc = create_document(m_url);
		if(!m_doc) {
			lib::logger::get_logger()->show(gettext("Failed to parse document %s"), m_url.get_url().c_str());
			return;
		}
	}
	AM_DBG lib::logger::get_logger()->debug("Creating player instance for: %s", m_url.get_url().c_str());
	// XXXX
	m_player = common::create_smil2_player(m_doc, this, m_embedder);

	m_player->set_feedback(this);
	m_player->initialize();

	if (playing || pausing) play();
	if (pausing) pause();
	
	m_no_stopped_callbacks = false;
}

common::gui_screen *
mainloop::get_gui_screen()
{

#ifdef WITH_CG
	if (!m_gui_screen) m_gui_screen = new gui::cg::cg_gui_screen(m_view);
#else
	if (!m_gui_screen) m_gui_screen = new gui::cocoa::cocoa_gui_screen(m_view);
#endif
	return m_gui_screen;
}

void
mainloop::node_focussed(const lib::node *n)
{
	if (n == NULL) {
		AM_DBG lib::logger::get_logger()->debug("node_focussed(0)");
//X		set_statusline(m_view, "");
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("node_focussed(%s)", n->get_sig().c_str());
	const char *alt = n->get_attribute("alt");
	if (alt) {
		AM_DBG lib::logger::get_logger()->debug("node_focussed: alt=%s", alt);
//X		set_statusline(m_view, alt);
		return;
	}
	const char *href = n->get_attribute("href");
	if (href) {
		AM_DBG lib::logger::get_logger()->debug("node_focussed: href=%s", href);
		std::string msg = "Go to ";
		msg += href;
//X		set_statusline(m_view, msg.c_str());
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("node_focussed: nothing to show");
//X	set_statusline(m_view, "???");
}

void
mainloop::print_nodes()
{
#ifdef KEES_LASTNODE_CODE
	for (std::list<const lib::node*>::iterator node_iter = m_nodes.begin(); 
		 node_iter != m_nodes.end(); node_iter++) {
		AM_DBG lib::logger::get_logger()->debug("node_active(%s)", (*node_iter)->get_sig().c_str());
	}
#endif
}

void
mainloop::node_started(const lib::node *n)
{
#ifdef KEES_LASTNODE_CODE

	AM_DBG lib::logger::get_logger()->debug("node_started(%s)", n->get_sig().c_str());
	m_nodes.push_back(n);
//X	print_nodes();
#else
    m_last_node_started = n;
#endif
}

void
mainloop::node_stopped(const lib::node *n)
{
#ifdef KEES_LASTNODE_CODE
	AM_DBG lib::logger::get_logger()->debug("node_stopped(%s)", n->get_sig().c_str());
	std::list<const lib::node*>::reverse_iterator rnode_iter = m_nodes.rbegin(); 
	for ( ; rnode_iter != m_nodes.rend(); rnode_iter++) {
		AM_DBG lib::logger::get_logger()->debug("node_iter(%s)", (*rnode_iter)->get_sig().c_str());
		if ((*rnode_iter) == n) {
			break;
		}
	}
	if (rnode_iter != m_nodes.rend()) {
		// get corresponding forward iterator for erase
		std::list<const lib::node*>::iterator node_victim = rnode_iter.base();
		node_victim--;		
		AM_DBG lib::logger::get_logger()->debug("node_victim(%s)", (*node_victim)->get_sig().c_str());		
//X		m_nodes.erase(node_victim);
	}
//X	print_nodes();
#endif
}

void
mainloop::goto_node_repr(const std::string node_repr)
{
	if (m_player != NULL) {
		const lib::node* n = m_doc->locate_node(node_repr.c_str());
		if (n != NULL) {
			m_player->goto_node(n);
		}
	}
}

PlaylistItem*
mainloop::get_current_item()
{
	return m_current_item;
}

NSString*
mainloop::get_meta_content(const char* name)
{
	const char* s = "";
	char buf[255];
	bool found = false;
	int i = 1;
	lib::node* n = m_doc->locate_node("/smil/head/meta");
	while (n != NULL) {
		if (strcmp(n->get_attribute("name"), name) == 0) {
//X			printf("found %s\n", name);
			found = true;
			break;
		}
		sprintf(buf, "/smil/head/meta[%d]", ++i);
		n = m_doc->locate_node(buf);
	}
	if (n != NULL && found) {
		s = n->get_attribute("content");
	}
	if (s == NULL) {
		s = ""; // <meta name="..."/> found, but no content
	}
	NSString* rv = [[NSString stringWithUTF8String:s] retain];
	return rv;
}

void
mainloop::document_stopped()
{
#ifndef KEES_LASTNODE_CODE
    m_last_node_started = NULL;
#endif
	AM_DBG NSLog(@"document_stopped");
	if (m_no_stopped_callbacks) {
		return;
	}
	AmbulantAppDelegate* am_delegate = (AmbulantAppDelegate*)[[UIApplication sharedApplication] delegate];
	[am_delegate performSelectorOnMainThread: @selector(document_stopped:) withObject: NULL waitUntilDone:NO];
}

