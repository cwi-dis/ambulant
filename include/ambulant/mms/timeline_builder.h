/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_MMS_TIMELINE_BUILDER_H
#define AMBULANT_MMS_TIMELINE_BUILDER_H

#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/common/layout.h"
#include "ambulant/mms/timelines.h"

namespace ambulant {

namespace mms {

class mms_layout_manager : public common::layout_manager {
  public:
	mms_layout_manager(common::window_factory *wf, const lib::document *doc);
	~mms_layout_manager();
	
	common::surface *get_surface(const lib::node *node);
	common::animation_notification *get_animation_notification(const lib::node *node) { return NULL; };
	common::animation_destination *get_animation_destination(const lib::node *node) { return NULL; };
	common::alignment *get_alignment(const lib::node *node) { return NULL; };
  private:
  	common::surface_template *m_audio_rgn, *m_text_rgn, *m_image_rgn;
};

class timeline_builder {
  public:

	timeline_builder(common::window_factory *wf, lib::node& root);
	~timeline_builder();
	
	passive_timeline *build();
	
  private:
  	void build_node(const lib::node& n);
  	void build_leaf(const lib::node& n);
  	void build_seq(const lib::node& n);
  	void build_par(const lib::node& n);
  	
  	lib::node& m_root;
  	passive_timeline *m_timeline;
};


} // namespace mms
 
} // namespace ambulant

#endif // AMBULANT_MMS_TIMELINE_BUILDER_H

