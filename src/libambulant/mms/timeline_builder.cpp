/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/lib/logger.h"
#include "ambulant/common/region.h"
#include "ambulant/net/datasource.h"
#include "ambulant/mms/timeline_builder.h"
#include <stdlib.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef WIN32
#ifdef IGNORE
#undef IGNORE
#endif
#endif

using namespace ambulant;
using namespace mms;

typedef enum {
	PAR,
	SEQ,
	LEAF,
	IGNORE,
	IGNORE_RECURSIVE
} node_type;


// Helper function to get the type of a node
node_type
get_node_type(const lib::node& n)
{
	lib::q_name_pair qname = n.get_qname();
	
	if (qname.first != "") {
		lib::logger::get_logger()->warn("timeline_builder.get_node_type: ignoring XML namespace");
	}
	if (qname.second == "par")
		return PAR;
	if (qname.second == "seq" || qname.second == "body")
		return SEQ;
	if (qname.second == "img" || qname.second == "text" || qname.second == "ref" ||
		qname.second == "audio" || qname.second == "video" )
		return LEAF;
	if (qname.second == "smil")
		return IGNORE;
	if (qname.second == "head")
		return IGNORE_RECURSIVE;
	lib::logger::get_logger()->error("timeline_builder.get_node_type: unknown nodename" /* , qname.second */);
	return IGNORE_RECURSIVE;
}

mms_layout_manager::mms_layout_manager(common::window_factory *wf, const lib::document *doc)
{
	AM_DBG lib::logger::get_logger()->trace("mms_layout_manager()->0x%x", (void *)this);
	const lib::screen_rect<int> image_rect = lib::screen_rect<int>(lib::point(0, 0), lib::size(176, 144));
	const lib::screen_rect<int> text_rect = lib::screen_rect<int>(lib::point(0, 144), lib::size(176, 72));
	common::passive_root_layout *root_layout = new common::passive_root_layout("root_layout", lib::size(176, 216), wf);
	
	m_audio_rgn = new common::passive_region("Audio");
	m_text_rgn = root_layout->subregion("Text", text_rect);
	m_image_rgn = root_layout->subregion("Image", image_rect);
}

mms_layout_manager::~mms_layout_manager()
{
	AM_DBG lib::logger::get_logger()->trace("~mms_layout_manager(0x%x)", (void *)this);
	delete m_image_rgn;
	m_image_rgn = NULL;
	delete m_text_rgn;
	m_text_rgn = NULL;
	delete m_audio_rgn;
	m_audio_rgn = NULL;
}

common::surface *
mms_layout_manager::get_surface(const lib::node *node)
{
	common::surface *rgn;
	lib::xml_string tag = node->get_qname().second;
	if (tag == "img" || tag == "video") rgn = m_image_rgn->activate(node);
	else if ( tag == "text") rgn = m_text_rgn->activate(node);
	else if ( tag == "audio") rgn = m_audio_rgn->activate(node);
	else {
		lib::logger::get_logger()->error("timeline_builder.build_leaf: unknown node type %s", tag.c_str());
		return NULL;
	}
	return rgn;
}


timeline_builder::timeline_builder(common::window_factory *wf, lib::node& root)
:	m_root(root),
	m_timeline(new passive_timeline(&root))
{
}

timeline_builder::~timeline_builder()
{
	if (m_timeline)
		delete m_timeline;
	m_timeline = NULL;
}

passive_timeline *
timeline_builder::build()
{
	passive_timeline *rv;
	
	build_node(m_root);
	m_timeline->build();
	rv = m_timeline;
	m_timeline = NULL;
	return rv;
}

void
timeline_builder::build_node(const lib::node& n)
{
	node_type tp = get_node_type(n);
	
#ifndef AMBULANT_NO_IOSTREAMS
	AM_DBG std::cout << "build_node(type=" << tp << ")" << std::endl;
#endif

	if (tp == LEAF) {
		build_leaf(n);
	} else if (tp == SEQ) {
		build_seq(n);
	} else if (tp == PAR) {
		build_par(n);
	} else if (tp == IGNORE) {
		std::list<const lib::node*> children;
		std::list<const lib::node*>::iterator ch;
		
		n.get_children(children);
		for (ch = children.begin(); ch != children.end(); ch++)
			build_node((const lib::node&)**ch);
		
	} else if (tp == IGNORE_RECURSIVE) {
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "build_node: skipping node" << std::endl;
#endif
	} else {
		lib::logger::get_logger()->error("timeline_builder.build_node: unknown nodetype %d", (int)tp);
	}
}

void
timeline_builder::build_leaf(const lib::node& n)
{
	// For now we create a region per node
	timeline_node *transitions = m_timeline->add_node(&n);
	timeline_node_transition *tmp;
	
	// This code is valid for leafs without explicit or implicit
	// timing.
	tmp = transitions->add_transition();
	tmp->add_lhs(START_PREROLL, &n);
	tmp->add_rhs(START_PREROLL_RENDERER, &n);
	
	tmp = transitions->add_transition();
	tmp->add_lhs(START_PLAY, &n);
	tmp->add_rhs(START_PLAY_RENDERER, &n);

	tmp = transitions->add_transition();
	tmp->add_lhs(DONE_PLAY_RENDERER, &n);
	tmp->add_rhs(DONE_PLAY, &n);

	tmp = transitions->add_transition();
	tmp->add_lhs(STOP_PLAY, &n);
	tmp->add_rhs(STOP_PLAY_RENDERER, &n);
}

void
timeline_builder::build_seq(const lib::node& n)
{
	// This code is valid for a <seq> with at least one child,
	// no explicit timing and an untimed parent (if any).
	std::list<const lib::node*> children;
	n.get_children(children);
	if (children.size() < 1) {
		lib::logger::get_logger()->error("timeline_builder.build_par: empty seq not allowed in MMS");
		return;
	}
	
	timeline_node *transitions = m_timeline->add_node(&n);
	timeline_node_transition *preroll = transitions->add_transition();
	timeline_node_transition *play = transitions->add_transition();
	timeline_node_transition *next = play;
	
	preroll->add_lhs(START_PREROLL, &n);
	play->add_lhs(START_PLAY, &n);

	std::list<const lib::node*>::iterator ch;
	for (ch = children.begin(); ch != children.end(); ch++) {

		preroll->add_rhs(START_PREROLL, *ch);
		play->add_rhs(START_PLAY, *ch);

		build_node(**ch);

		next = transitions->add_transition();
		next->add_lhs(DONE_PLAY, *ch);
		preroll = play;
		play = next;
	}
	next->add_rhs(DONE_PLAY, &n);
}

void
timeline_builder::build_par(const lib::node& n)
{
	// This code is valid for a <par> with explicit timing with at least one
	// child and a parent without explicit timing.
	std::list<const lib::node*> children;
	n.get_children(children);
	if (children.size() < 1) {
		lib::logger::get_logger()->error("timeline_builder.build_par: empty par not allowed in MMS");
		return;
	}
	
	int duration = 5000;
	const char *duration_str = n.get_attribute("dur");
	if (duration_str == NULL) {
		lib::logger::get_logger()->error("timeline_builder.build_par: par must have explicit delay in MMS");
	} else {
		char *dur_follow_str;
		duration = strtol(duration_str, &dur_follow_str, 10);
		if (strcmp(dur_follow_str, "ms") != 0) {
			lib::logger::get_logger()->error("timeline_builder.build_par: dur attribute must be of form \"1000ms\"");
			duration = 5000;
		}
	}
	timeline_delay *delay = m_timeline->add_delay(duration);

	timeline_node *transitions = m_timeline->add_node(&n);
	timeline_node_transition *preroll = transitions->add_transition();
	timeline_node_transition *play = transitions->add_transition();
	timeline_node_transition *pre_stop = transitions->add_transition();
	timeline_node_transition *stop = transitions->add_transition();
	
	preroll->add_lhs(START_PREROLL, &n);
	play->add_lhs(START_PLAY, &n);
	play->add_rhs(DELAY, delay);
	stop->add_lhs(DELAY, delay);

	std::list<const lib::node*>::iterator ch;
	for (ch = children.begin(); ch != children.end(); ch++) {

		preroll->add_rhs(START_PREROLL, *ch);
		play->add_rhs(START_PLAY, *ch);

		build_node(**ch);

		pre_stop->add_lhs(DONE_PLAY, *ch);
		stop->add_rhs(STOP_PLAY, *ch);
	}
	stop->add_rhs(DONE_PLAY, &n);
}
