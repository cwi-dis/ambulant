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
#include "ambulant/common/region_node.h"
#include "ambulant/common/region.h"
#include "ambulant/common/preferences.h"
#include "ambulant/smil2/smil_layout.h"
#include <stack>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

// Helper function: get region_dim value from an attribute
static lib::region_dim
get_regiondim_attr(const lib::region_node *rn, char *attrname)
{
	const char *attrvalue = rn->get_attribute(attrname);
	lib::region_dim rd;
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
			lib::logger::get_logger()->error("smil_layout_manager: cannot parse %s=\"%s\"", attrname, attrvalue);
		}
	}
	return rd;
}

lib::smil_layout_manager::smil_layout_manager(window_factory *wf,lib::document *doc)
:   m_schema(lib::schema::get_instance())
{
	fix_document_layout(doc);
	const lib::node *layout_root = doc->get_layout();

	if (layout_root) {
		build_layout_tree(wf, layout_root);
	} else {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: no layout section");
	}
	// Finally we make sure there is at least one root-layout. This allows us
	// to use this as the default region. XXXX Should be auto-show eventually.
	if (m_rootlayouts.empty()) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: no rootLayouts, creating one");
		create_top_region(wf, NULL);
	}
}

lib::smil_layout_manager::~smil_layout_manager()
{
	m_schema = NULL;
	std::vector<passive_root_layout*>::iterator i;
	for (i=m_rootlayouts.begin(); i != m_rootlayouts.end(); i++)
		delete (*i);
}

void
lib::smil_layout_manager::fix_document_layout(lib::document *doc)
{
	// If we have a layout section already we're done
	if (doc->get_layout()) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: using existing layout");
		return;
	}
	// Otherwise we first have to find the layout section in the tree
	lib::node *doc_root = doc->get_root();
	lib::node *head = doc_root->get_first_child("head");
	if (!head) {
		lib::logger::get_logger()->warn("smil_layout_manager: no <head> section");
		return;
	}
	lib::node *layout_root = head->get_first_child("layout");
	if (!layout_root) {
		lib::logger::get_logger()->trace("smil_layout_manager: no <layout> section");
		return;
	}
	// Now we iterate over all the elements, set their dimensions
	// from the attributes and determine their inheritance
	lib::node::iterator it;
	lib::node::const_iterator end = layout_root->end();
	int level = -1;
	for(it = layout_root->begin(); it != end; it++) {
		std::pair<bool, lib::node*> pair = *it;
		if (pair.first) {
			level++;
			if (level == 0) continue; // Skip layout section itself
			assert(pair.second->is_region_node());
			lib::region_node *rn = static_cast<lib::region_node *>(pair.second);
			// For every node in the layout section we fill in the dimensions
			AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: adjusting %s %s", rn->get_local_name().c_str(), rn->get_attribute("id"));
			region_dim_spec& rds = rn->rds();
			rds.left = get_regiondim_attr(rn, "left");
			rds.width = get_regiondim_attr(rn, "width");
			rds.right = get_regiondim_attr(rn, "right");
			rds.top = get_regiondim_attr(rn, "top");
			rds.height = get_regiondim_attr(rn, "height");
			rds.bottom = get_regiondim_attr(rn, "bottom");
#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_OPERATORS_IN_NAMESPACE)
			AM_DBG {
				lib::logger::ostream os = lib::logger::get_logger()->trace_stream();
				// XXXX Why the &^%$#%& can't we use os << rds << lib::endl ??!??
				os << "smil_layout_manager: result=(" 
					<< rds.left << ", " << rds.width << ", " << rds.right << ", "
					<< rds.top << ", " << rds.height << ", " << rds.bottom << ")" << lib::endl;
			}
#endif
			
			// Next, we set the inheritance
			layout_type tp = m_schema->get_layout_type(rn->get_qname());
			if (tp == l_rootlayout || tp == l_toplayout) {
				rn->set_dimension_inheritance(di_none);
			} else if (level == 1) {
				// Toplevel region node
				rn->set_dimension_inheritance(di_rootlayout);
			} else {
				// lower-level region node
				rn->set_dimension_inheritance(di_parent);
			}
			// Next we set background color
			const char *bgcolor_attr = rn->get_attribute("backgroundColor");
			if (bgcolor_attr == NULL) bgcolor_attr = rn->get_attribute("background-color");
			if (bgcolor_attr == NULL) bgcolor_attr = "transparent";
			color_t bgcolor = to_color(0, 0, 0);
			bool transparent = true, inherit = false;
			if (strcmp(bgcolor_attr, "transparent") == 0) transparent = true;
			else if (strcmp(bgcolor_attr, "inherit") == 0) inherit = true;
			else if (!is_color(bgcolor_attr)) lib::logger::get_logger()->error("Invalid color: %s", bgcolor_attr);
			else {
				bgcolor = to_color(bgcolor_attr);
				transparent = false;
			}
			AM_DBG lib::logger::get_logger()->trace("Background color 0x%x %d %d", (int)bgcolor, (int)transparent, (int)inherit);
			rn->set_bgcolor(bgcolor, transparent, inherit);
			// showBackground
			const char *sbg_attr = rn->get_attribute("showBackground");
			if (sbg_attr) {
				if (strcmp(sbg_attr, "whenActive") == 0) rn->set_showbackground(false);
				else if (strcmp(sbg_attr, "always") == 0) rn->set_showbackground(true);
				else lib::logger::get_logger()->error("Invalid showBackground value: %s", sbg_attr);
			}
			// And fit
			const char *fit_attr = rn->get_attribute("fit");
			fit_t fit = fit_hidden;
			if (fit_attr) {
				if (strcmp(fit_attr, "fill") == 0) fit = fit_fill;
				else if (strcmp(fit_attr, "hidden") == 0) fit = fit_hidden;
				else if (strcmp(fit_attr, "meet") == 0) fit = fit_meet;
				else if (strcmp(fit_attr, "scroll") == 0) fit = fit_scroll;
				else if (strcmp(fit_attr, "slice") == 0) fit = fit_slice;
				else lib::logger::get_logger()->error("Invalid fit value: %s", fit_attr);
			}
			rn->set_fit(fit);
			// And finally z-index.
			// XXXX Note that the implementation of z-index isn't 100% correct SMIL 2.0:
			// we interpret missing z-index as zero, but the standard says "auto" which is
			// slightly different.
			const char *z_attr = rn->get_attribute("z-index");
			zindex_t z = 0;
			if (z_attr) z = strtol(z_attr, NULL, 10);
			AM_DBG lib::logger::get_logger()->trace("z-index=%d", z);
			rn->set_zindex(z);
		} else {
			level--;
		}
	}

	// XXXX Undecided on what to do for subregion positioning: maybe best to
	// simply create new subregions with "impossible" ids.
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: setting layout");
	doc->set_layout(layout_root);
}

void
lib::smil_layout_manager::build_layout_tree(window_factory *wf, const lib::node *layout_root) {
	std::stack<lib::passive_region*> stack;
	lib::node::const_iterator it;
	lib::node::const_iterator end = layout_root->end();
	
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_layout_tree called");
	// First we check for a root-layout node. This will be used as the parent
	// of toplevel region nodes. If there is no root-layout but there are
	// toplevel region nodes we will create it later.
	lib::passive_root_layout *root_layout = NULL;
	const lib::node *rlnode = layout_root->get_first_child("root-layout");
	if (rlnode) {
		assert(rlnode->is_region_node());
		const lib::region_node *rrlnode = static_cast<const lib::region_node *>(rlnode);
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_layout_tree: create root_layout");
		root_layout = create_top_region(wf, rrlnode);
	}
		
	// Loop over all the layout elements, create the regions and root_layouts,
	// and keep a stack to tie everything together.
	for(it = layout_root->begin(); it != end; it++) {
		std::pair<bool, const lib::node*> pair = *it;
		const lib::node *n = pair.second;
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: examining %s node", n->get_qname().second.c_str());
		lib::layout_type tag = m_schema->get_layout_type(n->get_qname());
		if(tag == l_none || tag == l_rootlayout) {
			// XXXX Will need to handle switch here
			continue;
		}
		if(pair.first) {
			// On the way down we create the regions and remember
			// them
			assert(n->is_region_node());
			const lib::region_node *rn = static_cast<const lib::region_node *>(n);
			lib::passive_region *rgn;
			const char *pid = n->get_attribute("id");
			std::string ident = "<unnamed>";
			if(pid) {
				ident = pid;
			}
			// Test that rootlayouts are correctly nested.
			if (!stack.empty()) {
				if (tag != l_region) {
					lib::logger::get_logger()->error("topLayout element inside other element: %s", ident.c_str());
					tag = l_region;
				}
			}
			// Create the region or the root-layout
			if (tag == l_toplayout) {	
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_layout_tree: create topLayout");
				lib::passive_root_layout *rootrgn = create_top_region(wf, rn);
				rgn = rootrgn;
			} else if (tag == l_region && !stack.empty()) {
				lib::passive_region *parent = stack.top();
				rgn = parent->subregion(rn);
			} else if (tag == l_region && stack.empty()) {
				// Create root-layout if it doesn't exist yet
				if (root_layout == NULL) {
					AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::build_layout_tree: create default root-layout");
					root_layout = create_top_region(wf, NULL);
				}
				rgn = root_layout->subregion(rn);
			} else {
				assert(0);
			}
			
			// Enter the region ID into the id-mapping
			if(pid) {
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: mapping id %s", pid);
				m_id2region[ident] = rgn;
			}
			
			// And the same for the regionName multimap
			const char *pname = n->get_attribute("regionName");
			if(pname) {
				AM_DBG lib::logger::get_logger()->trace("smil_layout_manager: mapping regionName %s", pname);
				std::string name;
				name = pname;
				m_name2region.insert(std::pair<std::string, passive_region*>(name, rgn));
			}
			// Finally push on to the stack for reference by child nodes
			stack.push(rgn);
		} else {
			// On the way back up we only need to pop the stack
			stack.pop();
		}
	}
}

lib::passive_root_layout *
lib::smil_layout_manager::create_top_region(window_factory *wf, const lib::region_node *rn)
{
	lib::size size = lib::size(lib::default_layout_width, lib::default_layout_height);
	if (rn) {
		screen_rect<int> bounds = rn->get_screen_rect();
		size = lib::size(bounds.right(), bounds.bottom());
	}
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::create_top_region: size=(%d, %d)", size.w, size.h);
	lib::passive_root_layout *rootrgn;
	rootrgn = new passive_root_layout(rn, size, wf);
	m_rootlayouts.push_back(rootrgn);
	return rootrgn;
}

lib::abstract_rendering_surface *
lib::smil_layout_manager::get_rendering_surface(const node *n) {
	// XXXX This code is blissfully unaware of subregion positioning right now.
	const char *prname = n->get_attribute("region");
	const char *nid = n->get_attribute("id");
	if (prname == NULL) {
		AM_DBG lib::logger::get_logger()->trace(
			"smil_layout_manager::get_rendering_surface(): no region attribute on %s",
			(nid?nid:""));
		return get_default_rendering_surface(n);
	}
	std::string rname = prname;
	std::map<std::string, passive_region*>::size_type namecount = m_name2region.count(rname);
	if (namecount > 1)
		lib::logger::get_logger()->warn("smil_layout_manager::get_rendering_surface(): Using first region %s only", prname);
	if (namecount > 0) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_rendering_surface(): matched %s by regionName", prname);
		return (*m_name2region.find(rname)).second->activate(n);
	}
	std::multimap<std::string, passive_region*>::size_type idcount = m_id2region.count(rname);
	if (idcount > 0) {
		AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_rendering_surface(): matched %s by id", prname);
		return m_id2region[rname]->activate(n);
	}
	AM_DBG lib::logger::get_logger()->trace("smil_layout_manager::get_rendering_surface(): no match for %s", prname);
	return get_default_rendering_surface(n);
}

lib::abstract_rendering_surface *
lib::smil_layout_manager::get_default_rendering_surface(const node *n) {
	const char *nid = n->get_attribute("id");
	lib::logger::get_logger()->warn("Returning default rendering surface for node %s", (nid?nid:""));
	return m_rootlayouts[0]->activate(n);
}


