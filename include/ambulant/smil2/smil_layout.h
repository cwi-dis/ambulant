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

#ifndef AMBULANT_SMIL2_REGION_BUILDER_H
#define AMBULANT_SMIL2_REGION_BUILDER_H

#include "ambulant/config/config.h"
#include "ambulant/common/layout.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"

namespace ambulant {
namespace common {
class schema;
}}

namespace ambulant {
namespace lib {
class document;
}}

namespace ambulant {

namespace smil2 {

class region_node;

class smil_layout_manager :
	public common::layout_manager,
	public lib::avt_change_notification
{
  public:
	smil_layout_manager(common::factories *factory, lib::document *doc);
	~smil_layout_manager();

	void load_bgimages(common::factories *factories);

	common::surface *get_surface(const lib::node *node);
	common::alignment *get_alignment(const lib::node *node);
	common::animation_notification *get_animation_notification(const lib::node *node);
	common::animation_destination *get_animation_destination(const lib::node *node);
	common::surface_template *get_region(const lib::node *n);
	void avt_value_changed_for(const lib::node *n);

  private:
	lib::node *get_document_layout(lib::document *doc);
	void build_layout_tree(lib::node *layout_root, lib::document *doc);

	region_node *get_region_node_for(const lib::node *n, bool nodeoverride);
	common::surface *get_default_rendering_surface(const lib::node *n);

	void build_surfaces(common::window_factory *wf);
	void build_body_regions(lib::document *doc);
	common::surface_template *create_top_surface(common::window_factory *wf,
		const region_node *rn, common::bgrenderer *bgrenderer);

	const common::schema *m_schema;
	common::surface_factory *m_surface_factory;

	lib::node *m_layout_section;
	region_node *m_layout_tree;
	std::vector<region_node *>m_default_region_subregions;

	std::vector<common::surface_template*> m_rootsurfaces;
	std::map<std::string, region_node*> m_id2region;
	std::map<std::string, lib::node*> m_id2regpoint;
	std::map<std::string, std::list<region_node*> > m_name2region;
	std::map<const lib::node*, region_node*> m_node2region;
	bool m_uses_bgimages;
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_REGION_BUILDER_H
