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

/////////////////////////////
// region_node
//
// A representation of a region as a node.
//
// A region node may be used to build a pure layout tree
// or participate in a tree of plain nodes.
//
/////////////////////////////

#ifndef AMBULANT_SMIL2_REGION_NODE_H
#define AMBULANT_SMIL2_REGION_NODE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/node_navigator.h"
#include "ambulant/lib/node_iterator.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/colors.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace smil2 {

enum dimension_inheritance { di_none, di_parent, di_rootlayout, di_stored };

class region_node : public common::animation_destination {
  public:
	typedef lib::node_navigator<region_node> nnhelper;
	typedef lib::node_navigator<const region_node> const_nnhelper;

	///////////////////////////////
	// tree iterators
	typedef lib::tree_iterator<region_node> iterator;
	typedef lib::const_tree_iterator<region_node> const_iterator;

	// static method which tests whether a body node needs
	// a region counterpart (because it uses subregion positioning
	// or some such)
	static bool needs_region_node(const lib::node *n);

	// constructs a region node with local name and attrs
	region_node(const lib::node *n, dimension_inheritance di);
	virtual ~region_node();

	// Initialize body subregion nodes from their real region node.
	void fix_from_region_node(const region_node *parent);

	// Initialize data structures from DOM node attributes.
	bool fix_from_dom_node();

	// Tie together region and surface_template trees
	void set_surface_template(common::surface_template *surf) { m_surface_template = surf; }
	common::surface_template *get_surface_template() { return m_surface_template; }
	common::animation_notification *get_animation_notification() { return m_surface_template; };
	common::animation_destination *get_animation_destination() { return this; };

	// query for this region's rectangle
	// the rectangle is evaluaded on the fly
	// the evaluation takes into account relative coordinates
	lib::rect get_rect(const lib::rect *default_rect = NULL) const;

	// gets the underlying region_dim_spec for modification
	common::region_dim_spec& rds() {return m_rds;}

	// Set and get where this node inherits dimension information from
	void set_dimension_inheritance(dimension_inheritance di) { m_dim_inherit = di; }
	dimension_inheritance get_dimension_inheritance() const { return m_dim_inherit; }

	// region_info implementation
	std::string get_name() const;
	common::fit_t get_fit() const { return m_fit; }
	lib::color_t get_bgcolor() const;
	double get_bgopacity() const;
	bool get_showbackground() const;
	bool get_transparent() const { return m_transparent; }
	common::zindex_t get_zindex() const { return m_display_zindex; }
	bool is_subregion() const { return m_is_subregion; }
	double get_soundlevel() const { return m_display_soundlevel; }
	common::sound_alignment get_soundalign() const { return m_display_soundalign; }
	const char *get_bgimage() const;
	common::tiling get_tiling() const;
	lib::rect get_crop_rect(const lib::size& srcsize) const;
	double get_mediaopacity() const;
	double get_mediabgopacity() const;
	bool is_chromakey_specified() const;
	double get_chromakeyopacity() const;
	lib::color_t get_chromakey() const;
	lib::color_t get_chromakeytolerance() const;

	// And corresponding setting interface
	void reset() {(void)fix_from_dom_node(); };
	void set_fit(common::fit_t f) { m_fit = f; }
	void set_bgcolor(lib::color_t c, bool transparent, bool inherit);
	void set_bgcolor(lib::color_t c) { set_bgcolor(c, false, false); };
	void set_showbackground(bool showbackground) { m_showbackground = showbackground; }
	void set_zindex(common::zindex_t z) { m_zindex = z; m_display_zindex = z; }
	void set_soundlevel(double l) { m_soundlevel = l; m_display_soundlevel = l; }
	void set_soundalign(common::sound_alignment sa) { m_soundalign = sa; m_display_soundalign = sa; }
	void set_panzoom(const common::region_dim_spec& rds_) { m_panzoom = rds_; m_display_panzoom = rds_; }
	void set_bgopacity(double l) { m_bgopacity = l; m_display_bgopacity = l; }
	void set_mediaopacity(double l) { m_mediaopacity = l; m_display_mediaopacity = l; }
	void set_mediabgopacity(double l) { m_mediabgopacity = l; m_display_mediabgopacity = l; }
	void set_chromakeyopacity(double l) { m_chromakeyopacity = l; m_display_chromakeyopacity = l; }
	void set_chromakey(lib::color_t c) { m_chromakey = c; m_display_chromakey = c; };
	void set_chromakeytolerance(lib::color_t ct) { m_chromakeytolerance = ct; m_display_chromakeytolerance = ct;};
	void set_as_subregion(bool b) { m_is_subregion = b; }

	// animation_destination interface
	common::region_dim get_region_dim(const std::string& which, bool fromdom = false) const;
	lib::color_t get_region_color(const std::string& which, bool fromdom = false) const;
	common::zindex_t get_region_zindex(bool fromdom = false) const;
	double get_region_soundlevel(bool fromdom = false) const;
	common::sound_alignment get_region_soundalign(bool fromdom = false) const;
	const common::region_dim_spec& get_region_panzoom(bool fromdom = false) const;
	double get_region_opacity(const std::string& which, bool fromdom = false) const;

	void set_region_dim(const std::string& which, const common::region_dim& rd);
	void set_region_color(const std::string& which, lib::color_t clr);
	void set_region_zindex(common::zindex_t z);
	void set_region_soundlevel(double level);
	void set_region_soundalign(common::sound_alignment sa);
	void set_region_panzoom(const common::region_dim_spec& rds);
	void set_region_opacity(const std::string& which, double level);

	// sets explicitly the dimensions of this region
	template <class L, class W, class R, class T, class H, class B>
	void set_dims(L l, W w, R r, T t, H h, B b) {
		m_rds.left = l; m_rds.width = w;  m_rds.right = r;
		m_rds.top = t; m_rds.height = h;  m_rds.bottom = b;
	}

	///////////////////////////////
	// iterators

	iterator begin() { return iterator(this);}
	const_iterator begin() const { return const_iterator(this);}

	iterator end() { return iterator(0);}
	const_iterator end() const { return const_iterator(0);}

	// Std xml tree interface
	const region_node *down() const { return m_child;}
	const region_node *up() const { return m_parent;}
	const region_node *next() const { return m_next;}

	region_node *down()  { return m_child;}
	region_node *up()  { return m_parent;}
	region_node *next()  { return m_next;}

	void down(region_node *n)  { m_child = n;}
	void up(region_node *n)  { m_parent = n;}
	void next(region_node *n)  { m_next = n;}

	const region_node* previous() const {return const_nnhelper::previous(this);}
	region_node* previous() { return nnhelper::previous(this);}

	region_node* last_child() { return nnhelper::last_child(this);}
	const region_node* last_child() const { return const_nnhelper::last_child(this);}

	region_node* get_root() {return nnhelper::get_root(this);}
	const region_node* get_root() const {return const_nnhelper::get_root(this);}

	bool is_descendent_of(region_node *tn) const {return const_nnhelper::is_descendent(this, tn);}

	region_node *append_child(region_node *child) {return nnhelper::append_child(this, child);}
	void get_children(std::list<region_node*>& l) { nnhelper::get_children(this, l); }
	region_node *get_first_child(const char *name);
	const region_node *get_first_child(const char *name) const;

	const lib::node *dom_node() const { return m_node; }

	static int get_node_counter() { return node_counter;}

  private:

	const lib::node *m_node;
	common::region_dim_spec m_rds;
	dimension_inheritance m_dim_inherit;
	lib::rect m_stored_dim_inherit;
	common::fit_t m_fit;
	common::zindex_t m_zindex;
	bool m_transparent;
	lib::color_t m_bgcolor;
	double m_soundlevel;
	common::sound_alignment m_soundalign;
	common::region_dim_spec m_panzoom;
	double m_mediaopacity;
	double m_mediabgopacity;
	bool m_chromakey_specified;
	double m_chromakeyopacity;
	lib::color_t m_chromakey;
	lib::color_t m_chromakeytolerance;
	const char *m_bgimage;
	common::tiling m_tiling;
	bool m_inherit_bgrepeat;
	double m_bgopacity;
	bool m_showbackground;
	bool m_inherit_bgcolor;
	common::surface_template *m_surface_template;
	bool m_is_subregion;

	// display attributes for the animation_destination interface
	common::region_dim_spec m_display_rds;
	common::zindex_t m_display_zindex;
	lib::color_t m_display_bgcolor;
	lib::color_t m_display_color;
	double m_display_soundlevel;
	common::sound_alignment m_display_soundalign;
	common::region_dim_spec m_display_panzoom;
	double m_display_bgopacity;
	double m_display_mediaopacity;
	double m_display_mediabgopacity;
	double m_display_chromakeyopacity;
	lib::color_t m_display_chromakey;
	lib::color_t m_display_chromakeytolerance;

	// verifier
	static int node_counter;

	// XML tree glue
	region_node *m_parent;
	region_node *m_child;
	region_node *m_next;
};
} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_REGION_NODE_H
