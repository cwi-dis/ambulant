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
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/common/schema.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/preferences.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/smil2/region_node.h"
#include "ambulant/smil2/smil_layout.h"
#include "ambulant/smil2/test_attrs.h"
#include <stack>

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

// Factory function
common::layout_manager *
common::create_smil2_layout_manager(common::window_factory *wf,lib::document *doc)
{
	return new smil_layout_manager(wf, doc);
}

smil_layout_manager::smil_layout_manager(common::window_factory *wf,lib::document *doc)
:   m_schema(common::schema::get_instance()),
	m_surface_factory(common::create_smil_surface_factory()),
	m_layout_tree(NULL)
{
	// First find the correct layout section to use.
	lib::node *layout_section = get_document_layout(doc);

	// Then scan the DOM tree and create our own tree of region_node objects
	build_layout_tree(layout_section);
	
	// Next we create the region_nodes for body nodes that need one (subregion
	// positioning, etc)
	build_body_regions(doc);

	// Next, create the surfaces that correspond to this tree
	build_surfaces(wf);
	
	// Now we make sure there is at least one root-layout. This allows us
	// to use this as the default region. XXXX Should be auto-show eventually.
	if (m_rootsurfaces.empty()) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: no rootLayouts, creating one");
		create_top_surface(wf, NULL, NULL);
	}
}

smil_layout_manager::~smil_layout_manager()
{
	m_schema = NULL;
	std::vector<common::surface_template*>::iterator i;
	for (i=m_rootsurfaces.begin(); i != m_rootsurfaces.end(); i++)
		delete (*i);
	// XXX Delete m_layout_tree tree
	delete m_layout_tree;
}

lib::node *
smil_layout_manager::get_document_layout(lib::document *doc)
{
	// We first have to find the layout section in the tree
	lib::node *doc_root = doc->get_root();
	lib::node *head = doc_root->get_first_child("head");
	if (!head) {
		lib::logger::get_logger()->warn("smil_layout_manager: no <head> section");
		return NULL;
	}
	lib::node *layout_root = head->get_first_child("layout");
	if (layout_root) {
		// Check that the type is supported
		const char *layout_type = layout_root->get_attribute("type");
		if (layout_type && strcmp(layout_type, "text/smil-basic-layout") != 0 ) {
			lib::logger::get_logger()->error("smil_layout_manager: <layout type=\"%s\"> not supported", layout_type);
			return NULL;
		}
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: returning node 0x%x", layout_root);
		return layout_root;
	}
	// Otherwise check for a switch
	lib::node *layout_switch = head->get_first_child("switch");
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: examining <switch>");
	if (layout_switch) {
		layout_root = layout_switch->down();
		while (layout_root) {
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: examining node 0x%x", layout_root);
			// Check that it is indeed a <layout> node
			if (m_schema->get_layout_type((layout_root)->get_qname()) != common::l_layout) {
				lib::logger::get_logger()->error("smil_layout_manager: <switch> in <head> should contain only <layout>s");
				continue;
			}
			// Check that the type is supported
			const char *layout_type = layout_root->get_attribute("type");
			if (layout_type && strcmp(layout_type, "text/smil-basic-layout") != 0 ) {
				lib::logger::get_logger()->warn("smil_layout_manager: <layout type=\"%s\"> not supported", layout_type);
				continue;
			}
			test_attrs *tester = new test_attrs(layout_root);
			if (tester->selected()) {
				delete tester;
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: returning node 0x%x", layout_root);
				return layout_root;
			}
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: skipping layout section due to test attributes");
			delete tester;
			layout_root = layout_root->next();
		}
	}
	lib::logger::get_logger()->trace("smil_layout_manager: no supported <layout> section");
	return NULL;
}

void
smil_layout_manager::build_layout_tree(lib::node *layout_root)
{
	std::stack<region_node *> stack;
	// Now we iterate over all the elements, set their dimensions
	// from the attributes and determine their inheritance
	lib::node::iterator it;
	lib::node::iterator end = layout_root->end();
	int level = -1;
	for(it = layout_root->begin(); it != end; it++) {
		std::pair<bool, lib::node*> pair = *it;
		if (pair.first) {
			level++;
			//if (level == 0) continue; // Skip layout section itself
			lib::node *n = pair.second;
			const char *pid = n->get_attribute("id");
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_document_layout: examining %s %s", 
				n->get_qname().second.c_str(), (pid?pid:"no-id"));
			// Find node type
			common::layout_type tp = m_schema->get_layout_type(n->get_qname());
			// First we handle regPoint nodes, which we only store
			if (tp == common::l_regpoint) {
				if (pid) {
					std::string id = pid;
					m_id2regpoint[id] = n;
				} else {
					lib::logger::get_logger()->warn("smil_layout_manager: regPoint node without id attribute");
				}
				continue;
			}
			dimension_inheritance di;
			if (tp == common::l_rootlayout || tp == common::l_toplayout) {
				di = di_none;
			} else if (level == 1) {
				// Toplevel region node
				di = di_rootlayout;
			} else {
				// lower-level region node
				di = di_parent;
			}

			// Put it in the tree
			region_node *rn = new region_node(n, di);
			rn->reset();
			if (stack.empty()) {
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_document_layout: 0x%x is m_layout_tree", (void*)rn);
				if(m_layout_tree != NULL) {
					lib::logger::get_logger()->error("smil_layout_manager::get_document_layout: multiple layout roots!");
				}
				m_layout_tree = rn;
			} else {
				region_node *parent = stack.top();
				parent->append_child(rn);
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_document_layout: 0x%x is child of 0x%x", (void*)rn, (void*)parent);
			}
			// Enter the region ID into the id-mapping
			if(pid) {
				std::string id = pid;
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: mapping id %s", pid);
				m_id2region[id] = rn;
			}
			
			// And the same for the regionName multimap
			const char *pname = n->get_attribute("regionName");
			if(pname) {
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: mapping regionName %s", pname);
				m_name2region[pname].push_back(rn);
			}
			
			// And finally into the node->region mapping (for animate)
			m_node2region[n] = rn;

			stack.push(rn);
		} else {
			level--;
			stack.pop();
		}
	}
}


void
smil_layout_manager::build_body_regions(lib::document *doc) {

	// Finally we loop over the body nodes, and determine which ones need a region_node
	// because they use subregion positioning, override background color, etc.
	lib::node *doc_root = doc->get_root();
	lib::node *body = doc_root->get_first_child("body");
	if (!body) {
		lib::logger::get_logger()->error("smil_layout_manager: no <body> section");
		return;
	}
	lib::node::const_iterator it;
	lib::node::const_iterator end = body->end();
	for(it = body->begin(); it != end; it++) {
		std::pair<bool, const lib::node*> pair = *it;
		if (!pair.first) continue;
		const lib::node *n = pair.second;
#ifdef OLD_SUBREGIONS		
		if (!region_node::needs_region_node(n)) continue;
#else
		if(!test_attrs(n).selected()) continue;
		if(!n->get_attribute("region") && !region_node::needs_region_node(n) &&
			n->get_local_name() != "area") continue;
#endif
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_body_regions: region for 0x%x %s", (void*)n, n->get_local_name().c_str());
		region_node *rn = new region_node(n, di_parent);
		rn->reset();
		rn->set_showbackground(false);
		rn->set_as_subregion(true);

		region_node *parent = get_region_node_for(n, false);
		if (!parent) {
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: subregion positioning on default region, node=0x%x, rn=0x%x", (void*)n, (void*)rn);
			m_default_region_subregions.push_back(rn);
		} else {
			parent->append_child(rn);
		}
		m_node2region[n] = rn;
	}
}
	
void
smil_layout_manager::build_surfaces(common::window_factory *wf) {
	std::stack<common::surface_template*> stack;
	region_node::iterator it;
	region_node::const_iterator end = m_layout_tree->end();
	
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_surfaces called");
	// First we check for a root-layout node. This will be used as the parent
	// of toplevel region nodes. If there is no root-layout but there are
	// toplevel region nodes we will create it later.
	common::surface_template *root_surface = NULL;
	region_node *first_root_layout = m_layout_tree ? m_layout_tree->get_first_child("root-layout") : NULL;
	if (first_root_layout) {
		common::renderer *bgrenderer = wf->new_background_renderer(first_root_layout);
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_surfaces: create root_layout");
		root_surface = create_top_surface(wf, first_root_layout, bgrenderer);
	}
		
	// Loop over all the layout elements, create the regions and root_layouts,
	// and keep a stack to tie everything together.
	if (m_layout_tree) {
		for(it = m_layout_tree->begin(); it != end; it++) {
			std::pair<bool, region_node*> pair = *it;
			region_node *rn = pair.second;
			const lib::node *n = rn->dom_node();
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: examining %s node 0x%x", n->get_qname().second.c_str(), rn);
			common::layout_type tag = m_schema->get_layout_type(n->get_qname());
			if(tag == common::l_rootlayout) {
				continue;
			}
			if (tag == common::l_layout)
				continue;
			if (tag == common::l_none ) {
				// XXXX Will need to handle switch here too
				// Assume subregion positioning.
				AM_DBG lib::logger::get_logger()->warn("smil_layout_manager: skipping <%s> in layout", n->get_qname().second.c_str());
				continue;
			}
			if (tag == common::l_media) {
				tag = common::l_region;
			}
			if (pair.first) {
				// On the way down we create the regions and remember
				// them
				common::renderer *bgrenderer = wf->new_background_renderer(rn);
				common::surface_template *surf;
				const char *pid = n->get_attribute("id");
				std::string ident = "<unnamed>";
				if(pid) {
					ident = pid;
				}
				// Test that rootlayouts are correctly nested.
				if (!stack.empty()) {
					if (tag != common::l_region) {
						lib::logger::get_logger()->error("topLayout element inside other element: %s", ident.c_str());
						tag = common::l_region;
					}
				}
				// Create the region or the root-layout
				if (tag == common::l_toplayout) {	
					AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_surfaces: create topLayout");
					surf = create_top_surface(wf, rn, bgrenderer);
				} else if (tag == common::l_region && !stack.empty()) {
					common::surface_template *parent = stack.top();
					surf = parent->new_subsurface(rn, bgrenderer);
				} else if (tag == common::l_region && stack.empty()) {
					// Create root-layout if it doesn't exist yet
					if (root_surface == NULL) {
						AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_surfaces: create default root-layout");
						root_surface = create_top_surface(wf, NULL, NULL);
					}
					surf = root_surface->new_subsurface(rn, bgrenderer);
				} else {
					assert(0);
				}
				// Store in the region_node
				rn->set_surface_template(surf);
				
				// Finally push on to the stack for reference by child nodes
				stack.push(surf);
			} else {
				// On the way back up we only need to pop the stack
				stack.pop();
			}
		}
	}
	// Finally we create the regions for body regions that use subregion
	// positioning on the default region
	std::vector<region_node *>::iterator bit;
	std::vector<region_node *>::const_iterator bend = m_default_region_subregions.end();
	for (bit = m_default_region_subregions.begin(); bit != bend; bit++) {
		region_node *rn = *bit;
		common::renderer *bgrenderer = wf->new_background_renderer(rn);
		if (root_surface == NULL) {
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_surfaces: create default root-layout for subregion positioning");
			root_surface = create_top_surface(wf, NULL, NULL);
		}
		common::surface_template *surf = root_surface->new_subsurface(rn, bgrenderer);
		rn->set_surface_template(surf);
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_surfaces: subregion 0x%x surface 0x%x", (void*)rn, (void*)surf);
	}
}

common::surface_template *
smil_layout_manager::create_top_surface(common::window_factory *wf, const region_node *rn, common::renderer *bgrenderer)
{
	common::surface_template *rootrgn;
	rootrgn = m_surface_factory->new_topsurface(rn, bgrenderer, wf);
	m_rootsurfaces.push_back(rootrgn);
	return rootrgn;
}

region_node *
smil_layout_manager::get_region_node_for(const lib::node *n, bool nodeoverride)
{
	if (nodeoverride) {
		std::map<const lib::node*, region_node*>::size_type count = m_node2region.count(n);
		if (count) {
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_surface(): per-node override");
			return (*m_node2region.find(n)).second;
		}
	}
	const char *prname = NULL;
	if(n->get_local_name() == "area" && n->up()) {
		prname = n->up()->get_attribute("region");
	} else {
		prname = n->get_attribute("region");
	}
	const char *nid = n->get_attribute("id");
	if (prname == NULL) {
		AM_DBG lib::logger::get_logger()->trace(
			"smil_layout_manager::get_surface(): no region attribute on %s",
			(nid?nid:""));
		return NULL;
	}
	std::string rname = prname;

	std::map<std::string, std::list<region_node*> >::iterator rit = m_name2region.find(rname);
	std::map<std::string, region_node*>::size_type namecount = (rit == m_name2region.end())?0:(*rit).second.size();

	if (namecount > 1)
		lib::logger::get_logger()->warn("smil_layout_manager::get_surface(): Using first region %s only", prname);
	if (namecount > 0) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_surface(): matched %s by regionName", prname);
		return (*m_name2region.find(rname)).second.front();
	}
	std::multimap<std::string, region_node*>::size_type idcount = m_id2region.count(rname);
	if (idcount > 0) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_surface(): matched %s by id", prname);
		return m_id2region[rname];
	}
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_surface(): no match for %s", prname);
	return NULL;
}

common::surface *
smil_layout_manager::get_surface(const lib::node *n) {
	region_node *rn = get_region_node_for(n, true);
	if (rn == NULL) {
		lib::logger::get_logger()->trace("get_surface: returning default region for 0x%x", (void*)n);
		return get_default_rendering_surface(n);
	}
	common::surface_template *stemp = rn->get_surface_template();
	if (stemp == NULL) {
		lib::logger::get_logger()->error("get_surface: region found, but no surface, node=0x%x, rn=0x%x", (void*)n, (void*)rn);
		return get_default_rendering_surface(n);
	}
	common::surface *surf = stemp->activate();
	surf->set_alignment(get_alignment(n));
	return surf;
}

common::surface_template *
smil_layout_manager::get_region(const lib::node *n) {
	region_node *rn = get_region_node_for(n, true);
	return rn?rn->get_surface_template():NULL;
}

common::animation_notification *
smil_layout_manager::get_animation_notification(const lib::node *n) {
	region_node *rn = get_region_node_for(n, true);
	return rn?rn->get_animation_notification():NULL;
}

common::animation_destination *
smil_layout_manager::get_animation_destination(const lib::node *n) {
	region_node *rn = get_region_node_for(n, true);
	return rn?rn->get_animation_destination():NULL;
}

// Helper function: decode pre-defined repoint names
static bool
decode_regpoint(common::regpoint_spec &pt, const char *name)
{
	if(!name || !name[0] || strcmp(name, "topLeft") == 0) pt = common::regpoint_spec(0.0, 0.0);
	else if (strcmp(name, "topMid") == 0) pt = common::regpoint_spec(0.5, 0.0);
	else if (strcmp(name, "topRight") == 0) pt = common::regpoint_spec(1.0, 0.0);
	else if (strcmp(name, "midLeft") == 0) pt = common::regpoint_spec(0.0, 0.5);
	else if (strcmp(name, "center") == 0) pt = common::regpoint_spec(0.5, 0.5);
	else if (strcmp(name, "midRight") == 0) pt = common::regpoint_spec(1.0, 0.5);
	else if (strcmp(name, "bottomLeft") == 0) pt = common::regpoint_spec(0.0, 1.0);
	else if (strcmp(name, "bottomMid") == 0) pt = common::regpoint_spec(0.5, 1.0);
	else if (strcmp(name, "bottomRight") == 0) pt = common::regpoint_spec(1.0, 1.0);
	else
		return false;
	return true;
}

// Helper function: get region_dim value from an attribute
static common::region_dim
get_regiondim_attr(const lib::node *rn, char *attrname)
{
	const char *attrvalue = rn->get_attribute(attrname);
	common::region_dim rd;
	if (attrvalue == NULL || *attrvalue == '\0') {
		// pass: region_dim are initialized as auto
	} else {
		int ivalue;
		char *endptr;
		ivalue = strtol(attrvalue, &endptr, 10);
		if (*endptr == '\0' || strcmp(endptr, "px") == 0) {
			rd = ivalue;
		} else if (*endptr == '%') {
			double fvalue;
			fvalue = ivalue / 100.0;
			rd = fvalue;
		} else {
			lib::logger::get_logger()->error("region_node: cannot parse %s=\"%s\"", attrname, attrvalue);
		}
	}
	return rd;
}

common::alignment *
smil_layout_manager::get_alignment(const lib::node *n)
{
	const char *regPoint = n->get_attribute("regPoint");
	const char *regAlign = n->get_attribute("regAlign");
	if (regPoint == NULL && regAlign == NULL) return NULL;
	
	common::regpoint_spec image_fixpoint = common::regpoint_spec(0, 0);
	common::regpoint_spec surface_fixpoint = common::regpoint_spec(0, 0);
	lib::node *regpoint_node = NULL;

	if (!decode_regpoint(surface_fixpoint, regPoint) && regPoint != NULL) {
		// Non-standard regpoint. Look it up.
		std::map<std::string, lib::node*>::iterator it = m_id2regpoint.find(regPoint);
		if (it == m_id2regpoint.end()) {
			lib::logger::get_logger()->error("smil_alignment: unknown regPoint: %s", regPoint);
		} else {
			regpoint_node = (*it).second;
			// XXX Just for now:-)
			surface_fixpoint.left = get_regiondim_attr(regpoint_node, "left");
			surface_fixpoint.top = get_regiondim_attr(regpoint_node, "top");
		}
	}
	if (!decode_regpoint(image_fixpoint, regAlign)) {
		// See if we can get one from the regPoint node, if there is one
		bool found = false;
		if (regpoint_node) {
			const char *regPointAlign = regpoint_node->get_attribute("regAlign");
			if (decode_regpoint(image_fixpoint, regPointAlign))
				found = true;
		}
		if (!found && regAlign != NULL)
			lib::logger::get_logger()->error("smil_alignment: unknown regAlign value: %s", regAlign);
	}
	return new common::smil_alignment(image_fixpoint, surface_fixpoint);
}

common::surface *
smil_layout_manager::get_default_rendering_surface(const lib::node *n) {
	const char *nid = n->get_attribute("id");
	lib::logger::get_logger()->warn("Returning default rendering surface for node %s", (nid?nid:""));
	return m_rootsurfaces[0]->activate();
}


