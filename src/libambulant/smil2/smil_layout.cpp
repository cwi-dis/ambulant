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

// #define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

// Factory function
common::layout_manager *
common::create_smil2_layout_manager(common::factories *factory,lib::document *doc)
{
	return new smil_layout_manager(factory, doc);
}

smil_layout_manager::smil_layout_manager(common::factories *factory,lib::document *doc)
:	m_schema(common::schema::get_instance()),
	m_surface_factory(common::create_smil_surface_factory()),
	m_layout_tree(NULL),
	m_uses_bgimages(false)
{
	// First find the correct layout section to use.
	m_layout_section = get_document_layout(doc);

	// Then scan the DOM tree and create our own tree of region_node objects
	build_layout_tree(m_layout_section, doc);

	// Next we create the region_nodes for body nodes that need one (subregion
	// positioning, etc)
	build_body_regions(doc);

	// Next, create the surfaces that correspond to this tree
	build_surfaces(factory->get_window_factory());

	// Now we make sure there is at least one root-layout. This allows us
	// to use this as the default region. XXXX Should be auto-show eventually.
	if (m_rootsurfaces.empty()) {
		AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: no rootLayouts, creating one");
		create_top_surface(factory->get_window_factory(), NULL, NULL);
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
	delete m_surface_factory;
}

lib::node *
smil_layout_manager::get_document_layout(lib::document *doc)
{
	// We first have to find the layout section in the tree
	lib::node *doc_root = doc->get_root();
	lib::node *head = doc_root->get_first_child("head");
	if (!head) {
		lib::logger::get_logger()->trace("smil_layout_manager: no <head> section");
		return NULL;
	}
	lib::node *layout_root = head->get_first_child("layout");
	if (layout_root) {
		// Check that the type is supported
		const char *layout_type = layout_root->get_attribute("type");
		if (layout_type && strcmp(layout_type, "text/smil-basic-layout") != 0 ) {
			lib::logger::get_logger()->trace("smil_layout_manager: <layout type=\"%s\"> not supported", layout_type);
			lib::logger::get_logger()->warn(gettext("Problem with layout in SMIL document"));
			return NULL;
		}
		AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: returning node 0x%x", layout_root);
		return layout_root;
	}
	// Otherwise check for a switch
	lib::node *layout_switch = head->get_first_child("switch");
	AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: examining <switch>");
	if (layout_switch) {
		layout_root = layout_switch->down();
		if (layout_root == NULL) return NULL;
		while (layout_root) {
			AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: examining node 0x%x", layout_root);
			// Check that it is indeed a <layout> node
			if (m_schema->get_layout_type((layout_root)->get_local_name()) != common::l_layout) {
				lib::logger::get_logger()->trace("smil_layout_manager: <switch> in <head> should contain only <layout>s");
				lib::logger::get_logger()->warn(gettext("Problem with layout in SMIL document"));
				continue;
			}
			// Check that the type is supported
			const char *layout_type = layout_root->get_attribute("type");
			if (layout_type && strcmp(layout_type, "text/smil-basic-layout") != 0 ) {
				lib::logger::get_logger()->trace("smil_layout_manager: <layout type=\"%s\"> not supported, skipping", layout_type);
				continue;
			}
			test_attrs *tester = new test_attrs(layout_root);
			if (tester->selected()) {
				delete tester;
				AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: returning node 0x%x", layout_root);
				return layout_root;
			}
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: skipping layout section due to test attributes");
			delete tester;
			layout_root = layout_root->next();
		}
	}

	return NULL;
}

void
smil_layout_manager::build_layout_tree(lib::node *layout_root, lib::document *doc)
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
			AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::get_document_layout: examining %s %s",
				n->get_local_name().c_str(), (pid?pid:"no-id"));
			// Find node type
			common::layout_type tp = m_schema->get_layout_type(n->get_local_name());
			// First we handle regPoint nodes, which we only store
			if (tp == common::l_regpoint) {
				if (pid) {
					std::string id = pid;
					m_id2regpoint[id] = n;
				} else {
					lib::logger::get_logger()->trace("smil_layout_manager: regPoint node without id attribute");
				}
				// We should push something on the stack, because it is popped later
				stack.push(NULL);
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
			doc->register_for_avt_changes(n, this);

			if (stack.empty()) {
				AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::get_document_layout: 0x%x is m_layout_tree", (void*)rn);
				if(m_layout_tree != NULL) {
					lib::logger::get_logger()->error("smil_layout_manager: multiple layout roots!");
				}
				m_layout_tree = rn;
			} else {
				region_node *parent = stack.top();
				parent->append_child(rn);
				AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::get_document_layout: 0x%x is child of 0x%x", (void*)rn, (void*)parent);
			}
			// Enter the region ID into the id-mapping
			if(pid) {
				std::string id = pid;
				AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: mapping id %s", pid);
				m_id2region[id] = rn;
			}

			// And the same for the regionName multimap
			const char *pname = n->get_attribute("regionName");
			if(pname) {
				AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: mapping regionName %s", pname);
				m_name2region[pname].push_back(rn);
			}

			// See if the node uses background images
            if (n->get_attribute("backgroundImage")) {
                m_uses_bgimages = true;   
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
		lib::logger::get_logger()->error(gettext("Document has no <body> section, nothing to play"));
		return;
	}
	lib::node::const_iterator it;
	lib::node::const_iterator end = body->end();
	for(it = body->begin(); it != end; it++) {
		std::pair<bool, const lib::node*> pair = *it;
		if (!pair.first) continue;
		const lib::node *n = pair.second;
		if (n->is_data_node()) continue;
		if(!test_attrs(n).selected()) continue; // XXXJACK: does this depend on dynamic_cc? I think so...
		if(!n->get_attribute("region") && !region_node::needs_region_node(n) &&
			n->get_local_name() != "area") continue;
		AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::build_body_regions: region for 0x%x %s", (void*)n, n->get_local_name().c_str());
		region_node *rn = new region_node(n, di_parent);
		region_node *parent = get_region_node_for(n, false);
		if (parent) rn->fix_from_region_node(parent);
		rn->reset();
		doc->register_for_avt_changes(n, this);
		rn->set_showbackground(false);
		rn->set_as_subregion(true);

		if (!parent) {
			AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: subregion positioning on default region, node=0x%x, rn=0x%x", (void*)n, (void*)rn);
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

	AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::build_surfaces called");
	assert(wf);
	// First we check for a root-layout node. This will be used as the parent
	// of toplevel region nodes. If there is no root-layout but there are
	// toplevel region nodes we will create it later.
	common::surface_template *root_surface = NULL;
	region_node *first_root_layout = m_layout_tree ? m_layout_tree->get_first_child("root-layout") : NULL;
	if (first_root_layout) {
		common::bgrenderer *bgrenderer = wf->new_background_renderer(first_root_layout);
		AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::build_surfaces: create root_layout");
		root_surface = create_top_surface(wf, first_root_layout, bgrenderer);
		assert(root_surface);
		first_root_layout->set_surface_template(root_surface);
	}

	// Loop over all the layout elements, create the regions and root_layouts,
	// and keep a stack to tie everything together.
	if (m_layout_tree) {
		for(it = m_layout_tree->begin(); it != end; it++) {
			std::pair<bool, region_node*> pair = *it;
			region_node *rn = pair.second;
			const lib::node *n = rn->dom_node();
			AM_DBG lib::logger::get_logger()->debug("smil_layout_manager: examining %s node 0x%x", n->get_local_name().c_str(), rn);
			common::layout_type tag = m_schema->get_layout_type(n->get_local_name());
			if(tag == common::l_rootlayout) {
				continue;
			}
			if (tag == common::l_layout)
				continue;
			if (tag == common::l_none ) {
				// XXXX Will need to handle switch here too
				// Assume subregion positioning.
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: skipping <%s> in layout", n->get_local_name().c_str());
				continue;
			}
			if (tag == common::l_media) {
				tag = common::l_region;
			}
			if (pair.first) {
				// On the way down we create the regions and remember
				// them
				common::bgrenderer *bgrenderer = wf->new_background_renderer(rn);
				common::surface_template *surf;
				const char *pid = n->get_attribute("id");
				std::string ident = "<unnamed>";
				if(pid) {
					ident = pid;
				}
				// Test that rootlayouts are correctly nested.
				if (!stack.empty()) {
					if (tag != common::l_region) {
						lib::logger::get_logger()->trace("%s: topLayout element inside other element", n->get_sig().c_str());
						lib::logger::get_logger()->error(gettext("Problem with layout in SMIL document"));
						tag = common::l_region;
					}
				}
				// Create the region or the root-layout
				if (tag == common::l_toplayout) {
					AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::build_surfaces: create topLayout");
					surf = create_top_surface(wf, rn, bgrenderer);
				} else if (tag == common::l_region && !stack.empty()) {
					common::surface_template *parent = stack.top();
					surf = parent->new_subsurface(rn, bgrenderer);
				} else if (tag == common::l_region && stack.empty()) {
					// Create root-layout if it doesn't exist yet
					if (root_surface == NULL) {
						AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::build_surfaces: create default root-layout");
						root_surface = create_top_surface(wf, NULL, NULL);
					}
					surf = root_surface->new_subsurface(rn, bgrenderer);
				} else {
					assert(0);
				}
				// Store in the region_node
				assert(surf); // XXXX Good idea?
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
		common::bgrenderer *bgrenderer = wf->new_background_renderer(rn);
		if (root_surface == NULL) {
			AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::build_surfaces: create default root-layout for subregion positioning");
			root_surface = create_top_surface(wf, NULL, NULL);
		}
		common::surface_template *surf = root_surface->new_subsurface(rn, bgrenderer);
		assert(surf); // XXXX Good idea?
		rn->set_surface_template(surf);
		AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::build_surfaces: subregion 0x%x surface 0x%x", (void*)rn, (void*)surf);
	}
}

common::surface_template *
smil_layout_manager::create_top_surface(common::window_factory *wf, const region_node *rn, common::bgrenderer *bgrenderer)
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
			AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::get_surface(): per-node override");
			return (*m_node2region.find(n)).second;
		}
	}
	const char *prname = NULL;
	if (n->get_local_name() == "a" && n->down()) {
		// XXX This is not 100% correct: it does not take subregion positioning into account
		prname = n->down()->get_attribute("region");
	} else if (n->get_local_name() == "area" && n->up()) {
		// XXX This is not 100% correct: it does not take subregion positioning into account
		prname = n->up()->get_attribute("region");
	} else {
		prname = n->get_attribute("region");
	}
	const char *nid = n->get_attribute("id");
	if (prname == NULL) {
		AM_DBG lib::logger::get_logger()->debug(
			"smil_layout_manager::get_surface(): no region attribute on %s",
			(nid?nid:""));
		return NULL;
	}
	std::string rname = prname;

	std::map<std::string, std::list<region_node*> >::iterator rit = m_name2region.find(rname);
	std::map<std::string, region_node*>::size_type namecount = (rit == m_name2region.end())?0:(*rit).second.size();

	if (namecount > 1)
		lib::logger::get_logger()->trace("smil_layout_manager::get_surface(): Using first region %s only", prname);
	if (namecount > 0) {
		AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::get_surface(): matched %s by regionName", prname);
		return (*m_name2region.find(rname)).second.front();
	}
	std::multimap<std::string, region_node*>::size_type idcount = m_id2region.count(rname);
	if (idcount > 0) {
		AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::get_surface(): matched %s by id", prname);
		return m_id2region[rname];
	}
	AM_DBG lib::logger::get_logger()->debug("smil_layout_manager::get_surface(): no match for %s", prname);
	return NULL;
}

common::surface *
smil_layout_manager::get_surface(const lib::node *n) {
	region_node *rn = get_region_node_for(n, true);
	if (rn == NULL) {
		AM_DBG lib::logger::get_logger()->debug("get_surface: returning default region for 0x%x", (void*)n);
		return get_default_rendering_surface(n);
	}
	common::surface_template *stemp = rn->get_surface_template();
	if (stemp == NULL) {
		lib::logger::get_logger()->debug("Internal error: get_surface: region found, but no surface, node=0x%x, rn=0x%x", (void*)n, (void*)rn);
		lib::logger::get_logger()->error(gettext("Programmer error encountered, attempting to continue"));
		return get_default_rendering_surface(n);
	}
	common::surface *surf = stemp->activate();
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

void
smil_layout_manager::avt_value_changed_for(const lib::node *n) {
	AM_DBG lib::logger::get_logger()->debug("smil_playout_manager::avt_values_changed_for(%s)", n->get_sig().c_str());
	region_node *rn = get_region_node_for(n, true);
	assert(rn);
	rn->reset();
	// XXXJACK: descend children as well???
	common::animation_notification *an = rn->get_animation_notification();
	assert(an);
	if (an) an->animated();
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
get_regiondim_attr(const lib::node *rn, const char *attrname)
{
	const char *attrvalue = rn->get_attribute(attrname);
	common::region_dim rd;
	if (attrvalue == NULL || *attrvalue == '\0') {
		// pass: region_dim are initialized as auto
	} else {
		int ivalue;
		char *endptr;
		ivalue = (int)strtol(attrvalue, &endptr, 10);
		if (*endptr == '\0' || strcmp(endptr, "px") == 0) {
			rd = ivalue;
		} else if (*endptr == '%') {
			double fvalue;
			fvalue = ivalue / 100.0;
			rd = fvalue;
		} else {
			lib::logger::get_logger()->trace("%s: cannot parse %s=\"%s\"", rn->get_sig().c_str(), attrname, attrvalue);
			lib::logger::get_logger()->warn(gettext("Syntax error in SMIL document"));
		}
	}
	return rd;
}

common::alignment *
smil_layout_manager::get_alignment(const lib::node *n)
{
	const char *regPoint;
	const char *regAlign;
	// Keep the names and nodes used to get regPoint and regAlign data, so we can give meaningful error messages
	const char *rpname = "regPoint";
	const char *raname = "regAlign";
	const lib::node *rpnode = n;
	const lib::node *ranode = n;
	
	// Highest priority for rp/ra values are regPoint and regAlign on the media item itself
	regPoint = n->get_attribute("regPoint");
	regAlign = n->get_attribute("regAlign");

	// Second priority: mediaAlign on media item
	const char *mediaAlign = n->get_attribute("mediaAlign");
	if (mediaAlign && regPoint == NULL) {
		regPoint = mediaAlign;
		rpname = "mediaAlign";
	}
	if (mediaAlign && regAlign == NULL) {
		regAlign = mediaAlign;
		raname = "mediaAlign";
	}

	if (regPoint == NULL || regAlign == NULL) {
		assert(mediaAlign == NULL);
		const region_node *rrn = get_region_node_for(n, false);
		if (rrn != NULL) {
			const lib::node *rn = rrn->dom_node();
			// Third priority: regPoint and regAlign attributes on the region node
			if (regPoint == NULL) {
				regPoint = rn->get_attribute("regPoint");
				rpnode = rn;
			}
			if (regAlign == NULL) {
				regAlign = rn->get_attribute("regAlign");
				ranode = rn;
			}
			// Fourth priority: mediaAlign on the region node
			mediaAlign = rn->get_attribute("mediaAlign");
			if (mediaAlign && regPoint == NULL) {
				regPoint = mediaAlign;
				rpname = "mediaAlign";
				rpnode = rn;
			}
			if (mediaAlign && regAlign == NULL) {
				regAlign = mediaAlign;
				raname = "mediaAlign";
				ranode = rn;
			}			
		}
	}
	// If at this point we have neither regPoint nor regAlign we don't have any alignment and we can return
	if (regPoint == NULL && regAlign == NULL) return NULL;

	common::regpoint_spec image_fixpoint = common::regpoint_spec(0, 0);
	common::regpoint_spec surface_fixpoint = common::regpoint_spec(0, 0);
	lib::node *regpoint_node = NULL;

	// Now we try to decode the regPoint. Note that this means a predefined regPoint name cannot be overridden
	// with a regPoint element with that same name.
	if (!decode_regpoint(surface_fixpoint, regPoint) && regPoint != NULL) {
		// Non-standard regpoint. Look it up.
		std::map<std::string, lib::node*>::iterator it = m_id2regpoint.find(regPoint);
		if (it == m_id2regpoint.end()) {
			lib::logger::get_logger()->trace("%s: unknown %s value: %s", rpnode->get_sig().c_str(), rpname, regPoint);
			lib::logger::get_logger()->warn(gettext("Syntax error in SMIL document"));
		} else {
			regpoint_node = (*it).second;
			// XXXJACK we only look at top/left here, but we should look at bottom and right too...
			surface_fixpoint.left = get_regiondim_attr(regpoint_node, "left");
			surface_fixpoint.top = get_regiondim_attr(regpoint_node, "top");
			// Fifth priority: pick up regAlign from regPoint
			if (regAlign == NULL) {
				regAlign = regpoint_node->get_attribute("regAlign");
				ranode = regpoint_node;
			}
		}
	}
	if (!decode_regpoint(image_fixpoint, regAlign)) {
		lib::logger::get_logger()->trace("%s: unknown %s value: %s", ranode->get_sig().c_str(), raname, regAlign);
		lib::logger::get_logger()->warn(gettext("Syntax error in SMIL document"));
	}
	return new common::smil_alignment(image_fixpoint, surface_fixpoint);
}

common::surface *
smil_layout_manager::get_default_rendering_surface(const lib::node *n) {
	const char *nid = n->get_attribute("id");
	lib::logger::get_logger()->trace("Returning default rendering surface for node %s", (nid?nid:""));
	return m_rootsurfaces[0]->activate();
}

class bgimage_loader : public lib::ref_counted_obj, public common::playable_notification {
  public:
	bgimage_loader(const lib::node *layout_root, common::factories *factories);
	~bgimage_loader();

	void run(smil_layout_manager *layout_mgr);

	/// playable_notification interface:
	void started(cookie_type n, double t = 0) {};
	void stopped(cookie_type n, double t = 0);
	void clicked(cookie_type n, double t = 0) {};
	void pointed(cookie_type n, double t = 0) {};
	void transitioned(cookie_type n, double t = 0) {};
	void marker_seen(cookie_type n, const char *name, double t = 0) {};
	void playable_stalled(const common::playable *p, const char *reason) {};
	void playable_unstalled(const common::playable *p) {};
	void playable_started(const common::playable *p, const lib::node *n, const char *comment) {};
	void playable_resource(const common::playable *p, const char *resource, long amount) {};
  private:
	const lib::node *m_layout_root;
	common::factories *m_factories;
	lib::timer_control *m_timer;
	lib::event_processor *m_event_processor;
	std::vector<lib::node*> m_nodes;
	std::map<int, common::playable*> m_playables;
	std::vector<common::playable*> m_old_playables;
	std::set<common::gui_window*> m_gui_windows;

	lib::critical_section_cv m_lock;
};

void
smil_layout_manager::load_bgimages(common::factories *factories)
{
//	if (!m_uses_bgimages) return;
//	abort();
    return; // disable bgimages, see bug #853
	if (!m_layout_section || !m_uses_bgimages) return;
	bgimage_loader *loader = new bgimage_loader(m_layout_section, factories);
	loader->run(this);
	loader->release();
}

bgimage_loader::bgimage_loader(const lib::node *layout_root, common::factories *factories)
:	m_layout_root(layout_root),
	m_factories(factories),
	m_timer(new lib::timer_control_impl(lib::realtime_timer_factory(), 1.0, false, true)),
	m_event_processor(NULL)
{
	m_event_processor = event_processor_factory(m_timer);
}

bgimage_loader::~bgimage_loader()
{
	m_lock.enter();
	std::set<common::gui_window*>::iterator iw;
	AM_DBG lib::logger::get_logger()->debug("bgimage_loader::~bgimage_loader(): sync redraw");
	for (iw=m_gui_windows.begin(); iw != m_gui_windows.end(); iw++)
		(*iw)->redraw_now();

	AM_DBG lib::logger::get_logger()->debug("bgimage_loader::~bgimage_loader(): delete playables");
	std::vector<common::playable*>::iterator ip;
	for (ip=m_old_playables.begin(); ip != m_old_playables.end(); ip++) {
		(*ip)->stop();
		(*ip)->release();
	}

	AM_DBG lib::logger::get_logger()->debug("bgimage_loader::~bgimage_loader(): delete nodes");
	std::vector<lib::node*>::iterator in;
	for (in=m_nodes.begin(); in != m_nodes.end(); in++)
		delete *in;

	AM_DBG lib::logger::get_logger()->debug("bgimage_loader::~bgimage_loader(): delete event processor");
	delete m_event_processor;
	delete m_timer;
	m_lock.leave();
}

void
bgimage_loader::run(smil_layout_manager *layout_mgr)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("load_bgimages: loading background images");
	// XXX Create an event_processor

	// Loop over the layout section, starting the renderers as we find regions that
	// want a background image.
	lib::node::const_iterator it;
	lib::node::const_iterator end = m_layout_root->end();
	const lib::node_context *context = m_layout_root->get_context();
	for(it = m_layout_root->begin(); it != end; it++) {
		if (!(*it).first) continue;
		const lib::node *rn = (*it).second;
		if (rn->get_attribute("backgroundImage") == NULL) continue;

		common::surface *surf = layout_mgr->get_surface(rn);
		if (!surf) {
			lib::logger::get_logger()->debug("bgimage_loader: no surface");
			continue;
		}
		const char *bgimage = surf->get_info()->get_bgimage();
		if (!bgimage) continue;
		const char *bgrepeat = rn->get_attribute("backgroundRepeat");
		AM_DBG lib::logger::get_logger()->debug("load_bgimages: load bgimage %s", bgimage);
		const char *attrs[9], **attrp = attrs;
		*attrp++ = "src";
		*attrp++ = bgimage;
		*attrp++ = "backgroundImage";
		*attrp++ = bgimage;
		*attrp++ = "erase";
		*attrp++ = "never";
		if (bgrepeat) {
			*attrp++ = "backgroundRepeat";
			*attrp++ = bgrepeat;
		}
		*attrp++ = NULL;
		lib::node *n = m_factories->get_node_factory()->new_node("img", attrs, context);

		// Create the renderer
		if (n) {
			cookie_type p_index = (cookie_type)m_playables.size();
			common::playable *p = m_factories->get_playable_factory()->new_playable(this, p_index, n, m_event_processor);
			if (p) {
				// Now tell the renderer where to render to, and remember the
				// toplevel gui_window so we can synchronise the redraws before
				// zapping the whole bgimage_loader and other objects.
				common::renderer *r = p->get_renderer();
				if (r) {
					r->set_surface(surf);
					common::gui_window *w = surf->get_gui_window();
					if (w && !m_gui_windows.count(w))
						m_gui_windows.insert(w);
				} else {
					lib::logger::get_logger()->debug("bgimage_loader: no renderer");
				}
				// Remember renderer and node.
				m_nodes.push_back(n);
				m_playables[p_index] = p;
				// And start it
				m_lock.leave();
				p->start(0);
				m_lock.enter();
			} else {
				delete n;
			}
		}
	}
	if (m_playables.size()) {
		// All the renderers are started. Wait for everything to finish.
		AM_DBG lib::logger::get_logger()->debug("bgimage_loader::run: waiting for %d renderers", m_playables.size());
		m_lock.wait();
	}
	m_lock.leave();
}

void
bgimage_loader::stopped(cookie_type n, double t)
{
	AM_DBG lib::logger::get_logger()->debug("bgimage_loader::stopped() called");
	m_lock.enter();
	if (m_playables.count(n) > 0) {
		common::playable *p = m_playables[n];
		m_old_playables.push_back(p);
		m_playables.erase(n);
		if (m_playables.size() == 0)	{
			AM_DBG lib::logger::get_logger()->debug("bgimage_loader::stopped: signalling condition");
			m_lock.signal();
		} else {
			AM_DBG lib::logger::get_logger()->debug("bgimage_loader::stopped: %d more renderers", m_playables.size());
		}
	}
	m_lock.leave();
}

