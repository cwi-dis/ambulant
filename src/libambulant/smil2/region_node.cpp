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
#include "ambulant/common/schema.h"
#include "ambulant/common/region_eval.h"
#include "ambulant/common/preferences.h"
#include "ambulant/smil2/region_node.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

// Attribute names that cause needs_region_node() to return true
// for a given body node
// XXXX Not checked with SMIL2 standard yet!
static char *subregionattrs[] = {
	"left", "width", "right", "top", "height", "bottom",
	"backgroundColor", "background-color",
	"transparent",
	"fit",
	NULL
};


// Helper function: get region_dim value from an attribute
static common::region_dim
get_regiondim_attr(const lib::node *rn, char *attrname)
{
	const char *attrvalue = rn->get_attribute(attrname);
	common::region_dim rd;
	if (attrvalue == NULL || *attrvalue == '\0'
	    || strcasecmp(attrvalue, "auto") == 0) {
		// pass: region_dim are initialized as "auto"
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
			lib::logger::get_logger()->error("%s: cannot parse %s=\"%s\"", rn->get_sig().c_str(), attrname, attrvalue);
		}
	}
	return rd;
}


bool 
region_node::needs_region_node(const lib::node *n) {
	char **attrnamep = subregionattrs;
	while (*attrnamep) {
		if (n->get_attribute(*attrnamep))
			return true;
		attrnamep++;
	}
	return false;
}

// static
int region_node::node_counter = 0;

region_node::region_node(const lib::node *n, dimension_inheritance di)
:	m_node(n),
	m_dim_inherit(di),
	m_fit(common::fit_default),
	m_zindex(0),
	m_bgcolor(lib::to_color(0,0,0)),
	m_transparent(true),
	m_showbackground(true),
	m_inherit_bgcolor(false),
	m_surface_template(NULL),
	m_is_subregion(false),
	m_parent(NULL),
	m_child(NULL),
	m_next(NULL) {node_counter++;}

region_node::~region_node() 
{
	node_counter--;
	lib::node_navigator<region_node>::delete_tree(this); 	
}

bool
region_node::fix_from_dom_node()
{
	bool changed = false;
	
	// For every node in the layout section we fill in the dimensions
	AM_DBG lib::logger::get_logger()->debug("region_node::reset: adjusting %s %s", m_node->get_local_name().c_str(), m_node->get_attribute("id"));
	common::region_dim_spec rdspec;
	rdspec.left = get_regiondim_attr(m_node, "left");
	rdspec.width = get_regiondim_attr(m_node, "width");
	rdspec.right = get_regiondim_attr(m_node, "right");
	rdspec.top = get_regiondim_attr(m_node, "top");
	rdspec.height = get_regiondim_attr(m_node, "height");
	rdspec.bottom = get_regiondim_attr(m_node, "bottom");
#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_OPERATORS_IN_NAMESPACE)
	AM_DBG {
		//lib::logger::ostream os = lib::logger::get_logger()->trace_stream();
		// XXXX Why the &^%$#%& can't we use os << rdspec << lib::endl ??!??
		//os << "region_node::reset: result=(" 
		//	<< rdspec.left << ", " << rdspec.width << ", " << rdspec.right << ", "
		//	<< rdspec.top << ", " << rdspec.height << ", " << rdspec.bottom << ")" << lib::endl;
	}
#endif
	if (rdspec != m_rds) {
		changed = true;
		m_rds = rdspec;
	}
	m_display_rds = m_rds;
	
	// Next we set background color
	const char *bgcolor_attr = m_node->get_attribute("backgroundColor");
	if (bgcolor_attr == NULL) bgcolor_attr = m_node->get_attribute("background-color");
	if (bgcolor_attr == NULL) bgcolor_attr = "transparent";
	lib::color_t bgcolor = lib::to_color(0, 0, 0);
	bool transparent = true, inherit = false;
	if (strcmp(bgcolor_attr, "transparent") == 0) transparent = true;
	else if (strcmp(bgcolor_attr, "inherit") == 0) inherit = true;
	else if (!lib::is_color(bgcolor_attr)) lib::logger::get_logger()->error("%s: Invalid color: %s", m_node->get_sig().c_str(), bgcolor_attr);
	else {
		bgcolor = lib::to_color(bgcolor_attr);
		transparent = false;
	}
	AM_DBG lib::logger::get_logger()->debug("region_node::reset: Background color 0x%x %d %d", (int)bgcolor, (int)transparent, (int)inherit);
	if (bgcolor != m_bgcolor || transparent != m_transparent || inherit != m_inherit_bgcolor) {
		changed = true;
		set_bgcolor(bgcolor, transparent, inherit);
	}
	
	// showBackground
	const char *sbg_attr = m_node->get_attribute("showBackground");
	bool sbg = true;
	if (sbg_attr) {
		if (strcmp(sbg_attr, "whenActive") == 0) sbg = false;
		else if (strcmp(sbg_attr, "always") == 0) sbg = true;
		else lib::logger::get_logger()->error("%s: Invalid showBackground value: %s", m_node->get_sig().c_str(), sbg_attr);
	}
	if (sbg != m_showbackground) {
		changed = true;
		set_showbackground(sbg);
	}
	
	// And fit
	const char *fit_attr = m_node->get_attribute("fit");
	common::fit_t fit = common::fit_default;
	if (fit_attr) {
		if (strcmp(fit_attr, "fill") == 0) fit = common::fit_fill;
		else if (strcmp(fit_attr, "hidden") == 0) fit = common::fit_hidden;
		else if (strcmp(fit_attr, "meet") == 0) fit = common::fit_meet;
		else if (strcmp(fit_attr, "scroll") == 0) fit = common::fit_scroll;
		else if (strcmp(fit_attr, "slice") == 0) fit = common::fit_slice;
		else lib::logger::get_logger()->error("%s: Invalid fit value: %s", m_node->get_sig().c_str(), fit_attr);
	}
	if (fit != m_fit) {
		changed = true;
		set_fit(fit);
	}
	
	// And finally z-index.
	// XXXX Note that the implementation of z-index isn't 100% correct SMIL 2.0:
	// we interpret missing z-index as zero, but the standard says "auto" which is
	// slightly different.
	const char *z_attr = m_node->get_attribute("z-index");
	common::zindex_t z = 0;
	if (z_attr) z = strtol(z_attr, NULL, 10);
	AM_DBG lib::logger::get_logger()->debug("region_node::reset: z-index=%d", z);
	if (z != m_zindex) {
		changed = true;
	}
       	set_zindex(z);
	return changed;
}

lib::basic_rect<int>
region_node::get_rect() const {
	const region_node *inherit_region = NULL;
	const region_node *parent_node = up();
	switch(m_dim_inherit) {
	  case di_parent:
		if (parent_node)
			inherit_region = parent_node;
		break;
	  case di_rootlayout:
		{
			const region_node *root_node = get_root();
			const region_node *rootlayout_node = root_node->get_first_child("root-layout");
			if (rootlayout_node)
				inherit_region = rootlayout_node;
		}
		break;
	  case di_none:
		break;
	}
	lib::basic_rect<int> rc;
	if(inherit_region == NULL) {
		rc = lib::basic_rect<int, int>(lib::basic_size<int>(common::default_layout_width, common::default_layout_height)); 
	} else {
		rc = inherit_region->get_rect();
	}
	common::region_evaluator re(rc.w, rc.h);
	AM_DBG lib::logger::get_logger()->debug("HEIGHT=%s", repr(m_display_rds.height).c_str());
	re.set(m_display_rds);
	return re.get_rect();
}
 
lib::screen_rect<int>
region_node::get_screen_rect() const {
	return lib::screen_rect<int>(get_rect());
}

std::string
region_node::get_name() const {
	const char *pid = m_node->get_attribute("id");
	if (pid) return pid;
	return "";
}

lib::color_t
region_node::get_bgcolor() const
{
	if(m_inherit_bgcolor) {
		const region_node *parent_node = up();
		if (parent_node)
			return parent_node->get_bgcolor();
	}
	return m_display_bgcolor;
	//return m_bgcolor;
}

bool
region_node::get_transparent() const
{
	return m_transparent;
}

bool
region_node::get_showbackground() const
{
	return m_showbackground;
}

void
region_node::set_bgcolor(lib::color_t c, bool transparent, bool inherit) { 
	m_display_bgcolor = m_bgcolor = c;
	m_transparent = transparent;
	m_inherit_bgcolor = inherit;
}

// I don't like it that we need this one...
region_node * 
region_node::get_first_child(const char *name) {
	region_node *e = down();
	if(!e) return 0;
	if(e->m_node->get_local_name() == name) return e;
	e = e->next();
	while(e != 0) {
		if(e->m_node->get_local_name() == name) 
			return e;
		e = e->next();
	}
	return 0;
}

const region_node * 
region_node::get_first_child(const char *name) const {
	const region_node *e = down();
	if(!e) return 0;
	if(e->m_node->get_local_name() == name) return e;
	e = e->next();
	while(e != 0) {
		if(e->m_node->get_local_name() == name) 
			return e;
		e = e->next();
	}
	return 0;
}

/////////////////////////
// animation_destination interface
// XXX: The implementations below are almost dummy

common::region_dim region_node::get_region_dim(const std::string& which, bool fromdom) const {
	const common::region_dim_spec& rds = fromdom?m_rds:m_display_rds;
	if(which == "left") return rds.left;
	else if(which == "width") return rds.width;
	else if(which == "right") return rds.right;
	else if(which == "top") return rds.top;
	else if(which == "height") return rds.height;
	else if(which == "bottom") return rds.bottom;
	assert(false);
	return common::region_dim();
}

lib::color_t region_node::get_region_color(const std::string& which, bool fromdom) const {
	if(which == "backgroundColor") {
		return fromdom?m_bgcolor:m_display_bgcolor;
	}
	return 0;
}

common::zindex_t region_node::get_region_zindex(bool fromdom) const {
	return fromdom?m_zindex:m_display_zindex;
}

// Sets the display value of a region dimension
void region_node::set_region_dim(const std::string& which, const common::region_dim& rd) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_dim(\"%s\", \"%s\") to %s", m_node->get_attribute("id"), which.c_str(), repr(rd).c_str());
	common::region_dim_spec& rds = m_display_rds;
	if(which == "left") rds.left = rd;
	else if(which == "width") rds.width = rd;
	else if(which == "right") rds.right = rd;
	else if(which == "top") rds.top = rd;
	else if(which == "height") rds.height = rd;
	else if(which == "bottom") rds.bottom = rd;
}

// Sets the display value of the backgroundColor or color
void region_node::set_region_color(const std::string& which, lib::color_t clr) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_color(\"%s\", \"%s\")", m_node->get_attribute("id"), which.c_str());
	if(which == "backgroundColor") m_display_bgcolor = clr;
	//else if(which == "color") set_fgcolor(clr);
}

// Sets the display value of the z-index
void region_node::set_region_zindex(common::zindex_t z) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_zindex()");
	m_display_zindex = z;
}

