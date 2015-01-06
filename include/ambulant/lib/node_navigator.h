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

#ifndef AMBULANT_LIB_NODE_NAVIGATOR_H
#define AMBULANT_LIB_NODE_NAVIGATOR_H

#include "ambulant/config/config.h"

// return list of nodes
#include <list>
#include <algorithm>
#include <cassert>

namespace ambulant {

namespace lib {

/// Navigate a DOM tree.
///
/// A utility template class for tree nodes, providing more compex
/// navigation implemented using only the up/down/next methods of the
/// underlying node class.
/// The actual type of the node is passed in as a template argument.
///
/// This template requires from tree nodes to implement the following interface:
/// class N {
///  public:
/// 	virtual ~N() {}
///
///	const N *down() const = 0;
///	const N *up() const = 0;
///	const N *next() const = 0;
///
///	N *down() = 0;
///	N *up() = 0;
///	N *next() = 0;
///
///	void down(N *n) = 0;
///	void up(N *n) = 0;
///	void next(N *n) = 0;
///};
///
template <class N>
class node_navigator {
  public:

	/// Return previous sibling.
	static N* previous(N *n);

	/// Return last child.
	static N* last_child(N *n);

	/// Return a list of all children.
	static void get_children(N *n, std::list<N*>& l);

	/// Append a new child at the end of the existing children.
	static N* append_child(N *n, N* child);

	/// Detach a node from its tree.
	static N* detach(N *n);

	/// Delete a subtree.
	static void delete_tree(N *n);

	/// Return the root of a tree.
	static N* get_root(N *n);

	/// Return the depth of this node in its tree.
	static int get_depth(N *n);

	/// Return the path from the root to this node.
	static void get_path(N *n, std::list<N*>& path);

	/// Return true if n is a descendant of a.
	static bool is_descendent(N *n, N *a);

	/// Return true if n is an ancestor of a.
	static bool is_ancestor(N *n, N *d);

	/// Return the nearest common ancestor of two nodes.
	static  N* get_common_ancestor(N *n1, N *n2);
};

template <class N>
inline N* node_navigator<N>::previous(N *n) {
	assert(n != 0);
	N *e = n->up();
	if(!e) return 0;

	e = e->down();
	if(e == n) return 0;

	N *p = 0;
	while(e->next()) {
		p = e;
		e = e->next();
		if(e == n) break;
	}
	return p;
}

template <class N>
inline N* node_navigator<N>::last_child(N *n) {
	assert(n != 0);
	N *e = n->down();
	if(!e) return 0;
	N *last = e;
	e = e->next();
	while(e) {
		last = e;
		e = e->next();
	}
	return last;
}

template <class N>
inline void node_navigator<N>::get_children(N *n, std::list<N*>& l) {
	assert(n != 0);
	N *e = n->down();
	if(!e) return;
	l.push_back(e);
	e = e->next();
	while(e) {
		l.push_back(e);
		e = e->next();
	}
}

template <class N>
inline N* node_navigator<N>::append_child(N *n, N* child) {
	assert(n != 0);
	assert(child != 0);

	// assert that does not has an owner
	assert(child->up() == 0);

	// assert that is a node or a sub-tree root node
	assert(child->next() == 0);

	if(n->down() == 0)
		n->down(child);
	else {
		N *e = n->down();
		while(e->next()) e = e->next();
		e->next(child);
	}
	child->up(n);
	return child;
}

template <class N>
inline N* node_navigator<N>::detach(N *n) {
	assert(n != 0);
	N *parent = n->up();

	// this is the root
	if(!parent) return n;

	// first child
	N *e = parent->down();
	if(e == n)  {
		// set parent first-child to next
		parent->down(n->next());

		// detach
		n->next((N*)0);
		n->up((N*)0);
		return n;
	}

	N *prev = e;
	e = e->next();
	while(e) {
		if(e == n) {
			// set previous to next
			prev->next(n->next());

			// detach
			n->next((N*)0);
			n->up((N*)0);
			return n;
		}
		prev = e;
		e = e->next();
	}
	assert(0);
	return 0;
}

template <class N>
inline void node_navigator<N>::delete_tree(N *n) {
	assert(n != 0);
	// if its not the root, remove it from the tree
	if(n->up() != 0) detach(n);
	assert(n->up() == 0);

	N *e = n->down();
	if(e) {
		e->up((N*)0);
		N *tmp = e;
		e = e->next();
		delete tmp;
		while(e) {
			e->up((N*)0);
			N *tmp2 = e;
			e = e->next();
			delete tmp2;
		}
	}
}

template <class N>
inline N* node_navigator<N>::get_root(N *n) {
	assert(n != 0);
	while(n->up()) n = n->up();
	return n;
}

// Returns the depth of n. Root has depth zero.
template <class N>
inline int node_navigator<N>::get_depth(N *n) {
	assert(n != 0);
	int depth = 0;
	while(n->up()) {
		n = n->up();
		depth++;
	}
	return depth;
}

// Creates the path of node as a std::list<N*>
template <class N>
inline void node_navigator<N>::get_path(N *n, std::list<N*>& path) {
	assert(n != 0);
	path.clear();
	while(n->up()) {
		path.push_front(n);
		n = n->up();
	}
	path.push_front(n);
}

// Returns true when 'n' is descendent of 'a' or when 'n' is 'a'.
template <class N>
inline bool node_navigator<N>::is_descendent(N *n, N *a) {
	assert(n && a);
	if(n == a) return true;
	while(n->up()) {
		n = n->up();
		if(n == a)
			return true;
	}
	return false;
}

// Returns true when 'n' is ancestor of 'd' or when 'n' is 'd'.
template <class N>
inline bool node_navigator<N>::is_ancestor(N *n, N *d) {
	return is_descendent(d, n);
}

// Returns the common ancestor of n1 and n2.
// The common ancestor is defined to be the first node
// that is common to the paths of n1 and n2 and has children.
template <class N>
inline N* node_navigator<N>::get_common_ancestor(N *n1, N *n2) {
	assert(n1 && n2);
	std::list<N*> path1; get_path(n1, path1);
	std::list<N*> path2; get_path(n2, path2);
	typedef typename std::list<N*>::reverse_iterator reverse_iterator;
	for(reverse_iterator it1=path1.rbegin();it1!=path1.rend();it1++) {
		reverse_iterator it2 = std::find(path2.rbegin(), path2.rend(), (*it1));
		if(it2 != path2.rend() && (*it2)->down())
			return (*it2);
	}
	// we can reach this point only for a single element tree.
	assert(n1==n2 && !n1->down() && !n1->up());
	return n1;
}


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_NODE_NAVIGATOR_H
