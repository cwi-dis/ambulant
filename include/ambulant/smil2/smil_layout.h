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

#ifndef AMBULANT_SMIL2_REGION_BUILDER_H
#define AMBULANT_SMIL2_REGION_BUILDER_H

#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace smil2 {

class region_node;

class smil_layout_manager : public common::layout_manager {
  public:
	smil_layout_manager(common::window_factory *wf, lib::document *doc);
	~smil_layout_manager();
	
	common::surface *get_surface(const lib::node *node);
  private:
	void get_document_layout(lib::document *doc);
	
	region_node *get_region_node_for(const lib::node *n, bool nodeoverride);
	common::surface *get_default_rendering_surface(const lib::node *n);
	
	void build_surfaces(common::window_factory *wf);
	void build_body_regions(lib::document *doc);
	common::surface_template *create_top_surface(common::window_factory *wf, 
		const region_node *rn, common::renderer *bgrenderer);
	
	common::alignment *get_alignment(const lib::node *node);
	
	const common::schema *m_schema;
	common::surface_factory *m_surface_factory;
	
	region_node *m_layout_tree;
	
	std::vector<common::surface_template*> m_rootsurfaces;
	std::map<std::string, region_node*> m_id2region;
	std::map<std::string, lib::node*> m_id2regpoint;
	std::multimap<std::string, region_node*> m_name2region;
	std::map<const lib::node*, region_node*> m_node2region;
};

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_REGION_BUILDER_H
