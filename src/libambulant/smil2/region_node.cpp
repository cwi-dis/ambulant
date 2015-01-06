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
static const char *subregionattrs[] = {
	"left", "width", "right", "top", "height", "bottom",
	"backgroundColor", "background-color",
	"transparent",
	"fit",
	"soundLevel",
	"soundAlign",
	"panZoom",
	"backgroundOpacity",
	"mediaOpacity",
	"mediaBackgroundOpacity",
	"chromaKeyOpacity",
	"chromaKey",
	"chromaKeyTolerance",
	NULL
};


// Helper function: get region_dim value from an attribute
static common::region_dim
get_regiondim_attr(const lib::node *rn, const char *attrname)
{
	const char *attrvalue = rn->get_attribute(attrname);
	common::region_dim rd;
	if (attrvalue == NULL || *attrvalue == '\0'
		|| strcmp(attrvalue, "auto") == 0)
	{
		// pass: region_dim are initialized as "auto"
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


bool
region_node::needs_region_node(const lib::node *n) {
	const char **attrnamep = subregionattrs;
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
	m_soundlevel(1.0),
	m_soundalign(common::sa_default),
	m_mediaopacity(1.0),
	m_mediabgopacity(1.0),
	m_chromakey_specified(false),
	m_chromakeyopacity(0.0),
	m_chromakey(lib::to_color(0,0,0)),
	m_chromakeytolerance(lib::to_color(0,0,0)),
	m_bgimage(NULL),
	m_tiling(common::tiling_default),
	m_bgopacity(1.0),
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

void
region_node::fix_from_region_node(const region_node *parent)
{
//XXX Wrong ! This will not work for animation.
	set_soundlevel(parent->get_soundlevel());
	set_soundalign(parent->get_soundalign());
	set_mediaopacity(parent->get_mediaopacity());
	set_mediabgopacity(parent->get_mediabgopacity());
	set_chromakey(parent->get_chromakey());
	set_chromakeyopacity(parent->get_chromakeyopacity());
	set_chromakeytolerance(parent->get_chromakeytolerance());
}

bool
region_node::fix_from_dom_node()
{
	bool changed = false;
	char *lastp;
	{
		// For every node in the layout section we fill in the dimensions
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: adjusting %s %s", m_node->get_local_name().c_str(), m_node->get_attribute("id"));
		common::region_dim_spec rdspec;
		rdspec.left = get_regiondim_attr(m_node, "left");
		rdspec.width = get_regiondim_attr(m_node, "width");
		rdspec.right = get_regiondim_attr(m_node, "right");
		rdspec.top = get_regiondim_attr(m_node, "top");
		rdspec.height = get_regiondim_attr(m_node, "height");
		rdspec.bottom = get_regiondim_attr(m_node, "bottom");
		AM_DBG {
			//lib::logger::ostream os = lib::logger::get_logger()->trace_stream();
			// XXXX Why the &^%$#%& can't we use os << rdspec << lib::endl ??!??
			//os << "region_node::reset: result=("
			//	<< rdspec.left << ", " << rdspec.width << ", " << rdspec.right << ", "
			//	<< rdspec.top << ", " << rdspec.height << ", " << rdspec.bottom << ")" << lib::endl;
		}
		if (rdspec != m_rds) {
			changed = true;
			m_rds = rdspec;
		}
		m_display_rds = m_rds;
	}

	{
		// Next we set background color
		const char *bgcolor_attr = m_node->get_attribute("backgroundColor");
		if (bgcolor_attr == NULL) bgcolor_attr = m_node->get_attribute("background-color");
		if (bgcolor_attr == NULL) bgcolor_attr = "transparent";
		lib::color_t bgcolor = lib::to_color(0, 0, 0);
		bool transparent = true, inherit = false;
		if (strcmp(bgcolor_attr, "transparent") == 0) transparent = true;
		else if (strcmp(bgcolor_attr, "inherit") == 0) inherit = true;
		else if (!lib::is_color(bgcolor_attr)) {
			lib::logger::get_logger()->trace("%s: Invalid color: %s", m_node->get_sig().c_str(), bgcolor_attr);
			lib::logger::get_logger()->warn(gettext("Ignoring minor errors in document"));
		} else {
			bgcolor = lib::to_color(bgcolor_attr);
			transparent = false;
		}
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: Background color 0x%x %d %d %d", (int)bgcolor, (int)transparent, (int)inherit);
		if (bgcolor != m_bgcolor || transparent != m_transparent || inherit != m_inherit_bgcolor) {
			changed = true;
		}
		set_bgcolor(bgcolor, transparent, inherit);
	}
	{
		// showBackground
		const char *sbg_attr = m_node->get_attribute("showBackground");
		bool sbg = true;
		if (sbg_attr) {
			if (strcmp(sbg_attr, "whenActive") == 0) sbg = false;
			else if (strcmp(sbg_attr, "always") == 0) sbg = true;
			else {
				lib::logger::get_logger()->error(gettext("%s: Invalid showBackground value: %s"), m_node->get_sig().c_str(), sbg_attr);
				lib::logger::get_logger()->warn(gettext("Ignoring minor errors in document"));
			}
		}
		if (sbg != m_showbackground) {
			changed = true;
		}
		set_showbackground(sbg);
	}

	{
		// And fit
		const char *fit_attr = m_node->get_attribute("fit");
		common::fit_t fit = common::fit_default;
		if (fit_attr) {
			if (strcmp(fit_attr, "fill") == 0) fit = common::fit_fill;
			else if (strcmp(fit_attr, "hidden") == 0) fit = common::fit_hidden;
			else if (strcmp(fit_attr, "meet") == 0) fit = common::fit_meet;
			else if (strcmp(fit_attr, "meetBest") == 0) fit = common::fit_meetbest;
			else if (strcmp(fit_attr, "scroll") == 0) fit = common::fit_scroll;
			else if (strcmp(fit_attr, "slice") == 0) fit = common::fit_slice;
			else {
				lib::logger::get_logger()->trace("%s: Invalid fit value: %s", m_node->get_sig().c_str(), fit_attr);
				lib::logger::get_logger()->warn(gettext("Ignoring minor errors in document"));
			}
		}
		if (fit != m_fit) {
			changed = true;
		}
		set_fit(fit);
	}

	{
		// And z-index.
		// XXXX Note that the implementation of z-index isn't 100% correct SMIL 2.0:
		// we interpret missing z-index as zero, but the standard says "auto" which is
		// slightly different.
		const char *z_attr = m_node->get_attribute("z-index");
		common::zindex_t z = 0;
		if (z_attr) z = (common::zindex_t)strtol(z_attr, NULL, 10);
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: z-index=%d", z);
		if (z != m_zindex) {
			changed = true;
		}
		set_zindex(z);
	}

	{
		// soundLevel.
		const char *soundlevel_attr = m_node->get_attribute("soundLevel");
		double sl = m_soundlevel;
		if (soundlevel_attr) {
			sl = strtod(soundlevel_attr, &lastp);
			if (*lastp == '%') {
				sl *= 0.01;
                lastp++;
			} else if (strcmp(lastp,"dB") == 0) {
				sl = pow(10.0, sl/20);
                lastp+=2;
			} else {
				// default: percentage implied
				sl *= 0.01;
			}
            if (*lastp) lib::logger::get_logger()->trace("%s: invalid soundLevel value: %s", m_node->get_sig().c_str(), soundlevel_attr);
		}
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: soundLevel=%g", sl);
		if (sl != m_soundlevel) {
			changed = true;
		}
		set_soundlevel(sl);
	}

	{
		// soundAlign
		const char *soundalign_attr = m_node->get_attribute("soundAlign");
		common::sound_alignment sa = m_soundalign;

		if (soundalign_attr == NULL) {
			/*do nothing*/;
		} else if (strcmp(soundalign_attr, "both") == 0) {
			sa = common::sa_both;
		} else if (strcmp(soundalign_attr, "left") == 0) {
			sa = common::sa_left;
		} else if (strcmp(soundalign_attr, "right") == 0) {
			sa = common::sa_right;
		} else {
			lib::logger::get_logger()->trace("%s: Invalid soundAlign value: %s", m_node->get_sig().c_str(), soundalign_attr);
			lib::logger::get_logger()->warn(gettext("Ignoring minor errors in document"));
		}
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: soundAlign=%d", (int)sa);
		if (sa != m_soundalign) {
			changed = true;
		}
		set_soundalign(sa);
	}

	{
		// panZoom
		const char *panzoom_attr = m_node->get_attribute("panZoom");
		common::region_dim_spec rds_;
		AM_DBG lib::logger::get_logger()->debug("panZoom is now '%s'", panzoom_attr);
		if (panzoom_attr) rds_ = common::region_dim_spec(panzoom_attr, "panZoomRect");
		if (rds_ != m_panzoom) {
			changed = true;
		}
		set_panzoom(rds_);
	}

	{
		// backgroundOpacity.
		const char *bgopacity_attr = m_node->get_attribute("backgroundOpacity");
		double bo = m_bgopacity;
		if (bgopacity_attr) {
			bo = strtod(bgopacity_attr, &lastp);
			if (*lastp == '%') {
                bo *= 0.01;
                lastp++;
            }
            if (*lastp) lib::logger::get_logger()->trace("%s: invalid backgroundOpacity value: %s", m_node->get_sig().c_str(), bgopacity_attr);
		}
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: backgroundOpacity=%g", bo);
		if (bo != m_bgopacity) {
			changed = true;
		}
		set_bgopacity(bo);
	}

	{
		// mediaOpacity.
		const char *mediaopacity_attr = m_node->get_attribute("mediaOpacity");
		double fo = m_mediaopacity;
		if (mediaopacity_attr) {
			fo = strtod(mediaopacity_attr, &lastp);
			if (*lastp == '%') {
                fo *= 0.01;
                lastp++;
            }
            if (*lastp) lib::logger::get_logger()->trace("%s: invalid mediaOpacity value: %s", m_node->get_sig().c_str(), mediaopacity_attr);
		}
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: mediaOpacity=%g", fo);
		if (fo != m_mediaopacity) {
			changed = true;
		}
		set_mediaopacity(fo);
	}

	{
		// mediaBackgroundOpacity.
		const char *mediabgopacity_attr = m_node->get_attribute("mediaBackgroundOpacity");
		double mbo = m_mediabgopacity;
		if (mediabgopacity_attr) {
			mbo = strtod(mediabgopacity_attr, &lastp);
			if (*lastp == '%') {
                mbo *= 0.01;
                lastp++;
            }
            if (*lastp) lib::logger::get_logger()->trace("%s: invalid mediaBackgroundOpacity value: %s", m_node->get_sig().c_str(), mediabgopacity_attr);
		}
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: mediaBackgroundOpacity=%g", mbo);
		if (mbo != m_mediabgopacity) {
			changed = true;
		}
		set_mediabgopacity(mbo);
	}

	{
		// chromaKeyOpacity.
		const char *chromakeyopacity_attr = m_node->get_attribute("chromaKeyOpacity");
		double fo = m_chromakeyopacity;
		if (chromakeyopacity_attr) {
			fo = strtod(chromakeyopacity_attr, &lastp);
			if (*lastp == '%') {
                fo *= 0.01;
                lastp++;
            }
            if (*lastp) lib::logger::get_logger()->trace("%s: invalid chromaKeyOpacity value: %s", m_node->get_sig().c_str(), chromakeyopacity_attr);
		}
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: chromaKeyOpacity=%g", fo);
		if (fo != m_chromakeyopacity) {
			changed = true;
		}
		set_chromakeyopacity(fo);
	}
	{
		// chromaKey
		const char *chromakey_attr = m_node->get_attribute("chromaKey");
		lib::color_t chromakey = lib::to_color(0, 0, 1); //XXXX default chromakey
		if (chromakey_attr) {
			if (!lib::is_color(chromakey_attr)) {
				lib::logger::get_logger()->trace("%s: Invalid chromaKey color: %s", m_node->get_sig().c_str(), chromakey_attr);
				lib::logger::get_logger()->warn(gettext("Ignoring minor errors in document"));
			} else {
				chromakey = lib::to_color(chromakey_attr);
				m_chromakey_specified=true;
			}
			AM_DBG lib::logger::get_logger()->debug("region_node::reset: chromaKey color 0x%x", (int)chromakey);
		}
		if (m_chromakey_specified && chromakey != m_chromakey) {
			changed = true;
		}
		set_chromakey(chromakey);
	}
	{
		// chromaKeyTolerance
		const char *chromakeytolerance_attr = m_node->get_attribute("chromaKeyTolerance");
		lib::color_t chromakeytolerance = m_chromakeytolerance;
		if (chromakeytolerance_attr) {
			if (!lib::is_color(chromakeytolerance_attr)) {
				lib::logger::get_logger()->trace("%s: Invalid chromaKeytolerance color: %s", m_node->get_sig().c_str(), chromakeytolerance_attr);
				lib::logger::get_logger()->warn(gettext("Ignoring minor errors in document"));
			} else {
				chromakeytolerance = lib::to_color(chromakeytolerance_attr);
			}
		}
		AM_DBG lib::logger::get_logger()->debug("region_node::reset: chromaKeyTolerance=%0x%x", (int) chromakeytolerance);
		if (chromakeytolerance != m_chromakeytolerance) {
			changed = true;
		}
		set_chromakeytolerance(chromakeytolerance);
	}
	// backgroundImage

	// Note: we simply share a reference to the char* in the DOM tree,
	// so we should not free the memory when the region_node gets cleaned up
	m_bgimage = m_node->get_attribute("backgroundImage");
	// Don't need to set changed

	{
		// backgroundRepeat
		const char *bgrepeat_attr = m_node->get_attribute("backgroundRepeat");

		if (bgrepeat_attr == NULL) {
			m_tiling = common::tiling_default;
		} else if (strcmp(bgrepeat_attr, "repeat") == 0) {
			m_tiling = common::tiling_both;
		} else if (strcmp(bgrepeat_attr, "repeatX") == 0) {
			m_tiling = common::tiling_horizontal;
		} else if (strcmp(bgrepeat_attr, "repeatY") == 0) {
			m_tiling = common::tiling_vertical;
		} else if (strcmp(bgrepeat_attr, "noRepeat") == 0) {
			m_tiling = common::tiling_none;
		} else if (strcmp(bgrepeat_attr, "inherit") == 0) {
			m_tiling = common::tiling_inherit;
		} else {
			lib::logger::get_logger()->trace("%s: Invalid backgroundRepeat value: %s", m_node->get_sig().c_str(), bgrepeat_attr);
			lib::logger::get_logger()->warn(gettext("Ignoring minor errors in document"));
		}
		// Don't need to set changed
	}

	return changed;
}

lib::rect
region_node::get_rect(const lib::rect *default_rect) const {
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
	case di_stored:
		break;
	}
	lib::rect rc;
	if (m_dim_inherit == di_stored) {
		rc = m_stored_dim_inherit;
	} else if(inherit_region == NULL) {
		if (default_rect) {
			rc = *default_rect;
			const_cast<region_node*>(this)->m_stored_dim_inherit = rc;
			const_cast<region_node*>(this)->m_dim_inherit = di_stored;
		} else {
			rc = lib::rect(lib::size(common::default_layout_width, common::default_layout_height));
		}
	} else {
		rc = inherit_region->get_rect();
	}
	common::region_evaluator re(rc.w, rc.h);
	AM_DBG lib::logger::get_logger()->debug("HEIGHT=%s", repr(m_display_rds.height).c_str());
	re.set(m_display_rds);
	return re.get_rect();
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

double
region_node::get_bgopacity() const
{
	return m_display_bgopacity;
}

double
region_node::get_mediaopacity() const
{
	return m_display_mediaopacity;
}

double
region_node::get_mediabgopacity() const
{
	return m_display_mediabgopacity;
}

bool
region_node::is_chromakey_specified() const
{
	return m_chromakey_specified;
}

double
region_node::get_chromakeyopacity() const
{
	return m_display_chromakeyopacity;
}

lib::color_t
region_node::get_chromakey() const
{
	return m_display_chromakey;
}

lib::color_t
region_node::get_chromakeytolerance() const
{
	return m_display_chromakeytolerance;
}

bool
region_node::get_showbackground() const
{
	return m_showbackground;
}

common::tiling
region_node::get_tiling() const
{
	if(m_tiling == common::tiling_inherit) {
		const region_node *inherit_node = NULL;
		if (m_dim_inherit == di_parent) {
			inherit_node = up();
		} else {
			const region_node *root_node = get_root();
			inherit_node = root_node->get_first_child("root-layout");
		}
		if (inherit_node)
			return inherit_node->get_tiling();
	}
	return m_tiling;
}

lib::rect
region_node::get_crop_rect(const lib::size& srcsize) const
{
	common::region_dim_spec rdspec(m_display_panzoom);
	lib::rect croprect(lib::point(0,0), srcsize);
	rdspec.convert(croprect);
	if (rdspec.left.defined()) croprect.x = rdspec.left.get_as_int();
	if (rdspec.top.defined()) croprect.y = rdspec.top.get_as_int();
	if (rdspec.width.defined()) croprect.w = rdspec.width.get_as_int();
	if (rdspec.height.defined()) croprect.h = rdspec.height.get_as_int();
	return croprect;
}

const char *
region_node::get_bgimage() const
{
	const char *bgimage = m_bgimage;
	if (bgimage && strcmp(bgimage, "inherit") == 0) {
		const region_node *inherit_node;
		if (m_dim_inherit == di_parent) {
			inherit_node = up();
		} else {
			const region_node *root_node = get_root();
			inherit_node = root_node->get_first_child("root-layout");
		}
		if (inherit_node)
			bgimage = inherit_node->get_bgimage();
		else
			bgimage = NULL;
	}
	if (bgimage && strcmp(bgimage, "none") == 0)
		bgimage = NULL;
	return bgimage;
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
	const common::region_dim_spec& myrds = fromdom?m_rds:m_display_rds;
	if(which == "left") return myrds.left;
	else if(which == "width") return myrds.width;
	else if(which == "right") return myrds.right;
	else if(which == "top") return myrds.top;
	else if(which == "height") return myrds.height;
	else if(which == "bottom") return myrds.bottom;
	assert(false);
	return common::region_dim();
}

lib::color_t region_node::get_region_color(const std::string& which, bool fromdom) const {
	if(which == "backgroundColor") {
		return fromdom?m_bgcolor:m_display_bgcolor;
	}
	if (which == "chromaKey")
		return fromdom?m_chromakey:m_display_chromakey;
	return 0;
}

common::zindex_t region_node::get_region_zindex(bool fromdom) const {
	return fromdom?m_zindex:m_display_zindex;
}

double region_node::get_region_soundlevel(bool fromdom) const {
	return fromdom?m_soundlevel:m_display_soundlevel;
}

common::sound_alignment region_node::get_region_soundalign(bool fromdom) const {
	return fromdom?m_soundalign:m_display_soundalign;
}

const common::region_dim_spec& region_node::get_region_panzoom(bool fromdom) const {
	return fromdom?m_panzoom:m_display_panzoom;
}

double region_node::get_region_opacity(const std::string& which, bool fromdom) const {
	if (which == "backgroundOpacity")
		return fromdom?m_bgopacity:m_display_bgopacity;
	if (which == "chromakeyOpacity")
		return fromdom?m_chromakeyopacity:m_display_chromakeyopacity;
	if (which == "chromaKeyTolerance")
		return fromdom?m_chromakeytolerance:m_display_chromakeytolerance;
	if (which == "mediaOpacity")
		return fromdom?m_mediaopacity:m_display_mediaopacity;
	if (which == "mediaBackgroundOpacity")
		return fromdom?m_mediabgopacity:m_display_mediabgopacity;
	assert(0);
	return 1.0;
}

// Sets the display value of a region dimension
void region_node::set_region_dim(const std::string& which, const common::region_dim& rd) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_dim(\"%s\", \"%s\") to %s", m_node->get_attribute("id"), which.c_str(), repr(rd).c_str());
	common::region_dim_spec& myrds = m_display_rds;
	if(which == "left") myrds.left = rd;
	else if(which == "width") myrds.width = rd;
	else if(which == "right") myrds.right = rd;
	else if(which == "top") myrds.top = rd;
	else if(which == "height") myrds.height = rd;
	else if(which == "bottom") myrds.bottom = rd;
}

// Sets the display value of the backgroundColor or color
void region_node::set_region_color(const std::string& which, lib::color_t clr) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_color(\"%s\", \"%s\")", m_node->get_attribute("id"), which.c_str());
	if(which == "backgroundColor") m_display_bgcolor = clr;
	else if (which == "chromaKey")
		m_display_chromakey = clr;
	else if (which == "chromaKeyTolerance")
		m_display_chromakeytolerance = clr;
	//else if(which == "color") set_fgcolor(clr);
}

// Sets the display value of the z-index
void region_node::set_region_zindex(common::zindex_t z) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_zindex()");
	m_display_zindex = z;
}

// Sets the display value of the sound level
void region_node::set_region_soundlevel(double level) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_soundlevel()");
	m_display_soundlevel = level;
}

void region_node::set_region_soundalign(common::sound_alignment sa) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_soundalign()");
	m_display_soundalign = sa;
}

void region_node::set_region_panzoom(const common::region_dim_spec& rds_) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_panzoom()");
	m_display_panzoom = rds_;
}

void region_node::set_region_opacity(const std::string& which, double level) {
	AM_DBG lib::logger::get_logger()->debug("region_node::set_region_bgopacity()");
	if (which == "backgroundOpacity")
		m_display_bgopacity = level;
	else if (which == "mediaOpacity")
		m_display_mediaopacity = level;
	else if (which == "mediaBackgroundOpacity")
		m_display_mediabgopacity = level;
	else if (which == "chromaKeyOpacity")
		m_display_chromakeyopacity = level;
	else
		assert(0);
}
