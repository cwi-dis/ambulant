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
/////////////////////////////
// node_navigator
//
// A utility template class for tree nodes.
// The actual type of the node is passed in as a template argument.
//
// This template requires from tree nodes to implement the following interface:
// class N {
//  public:
// 	virtual ~N() {}
//	
//	const N *down() const = 0;
//	const N *up() const = 0;
//	const N *next() const = 0;
//
//	N *down() = 0;
//	N *up() = 0;
//	N *next() = 0;
//
//	void down(N *n) = 0;
//	void up(N *n) = 0;
//	void next(N *n) = 0;
//};
//

#ifndef AMBULANT_LIB_NODE_NAVIGATOR_H
#define AMBULANT_LIB_NODE_NAVIGATOR_H

// return list of nodes
#include <list>

// assert
#include <cassert>

namespace ambulant {

namespace lib {

template <class N>
class node_navigator {
  public:
	static N* previous(N *n);
	static N* last_child(N *n);
	static void get_children(N *n, std::list<N*>& l);
	static N* append_child(N *n, N* child);
	static N* detach(N *n);
	static void delete_tree(N *n);
	static N* get_root(N *n);
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
		n->next(0);
		n->up(0);
		return n;
	}

	N *prev = e;
	e = e = e->next();
	while(e) {
		if(e == n) {
			// set previous to next
			prev->next(n->next());

			// detach
			n->next(0);
			n->up(0);
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
		e->up(0);
		N *tmp = e;
		e = e->next();
		delete tmp;
		while(e) {
			e->up(0);
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


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_NODE_NAVIGATOR_H
