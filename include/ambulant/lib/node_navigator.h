
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
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
	static const N* previous(const N *n);
	static const N* last_child(const N *n);
	static void get_children(const N *n, std::list<const N*>& l);
	static N* append_child(N *n, N* child);
	static N* detach(N *n);
	static void delete_tree(N *n);
	static N* get_root(N *n);
};

template <class N>
inline const N* node_navigator<N>::previous(const N *n) {
	assert(n != 0);
	const N *e = n->up();
	if(!e) return 0;

	e = e->down();
	if(e == n) return 0;

	const N *p = 0;
	while(e->next()) {
		p = e;
		e = e->next();
		if(e == n) break;
	}
	return p;
}

template <class N>
inline const N* node_navigator<N>::last_child(const N *n) {
	assert(n != 0);
	const N *e = n->down();
	if(!e) return 0;
	const N *last = e;
	while((e = e->next())) last = e;
	return last;
}

template <class N>
inline void node_navigator<N>::get_children(const N *n, std::list<const N*>& l) {
	assert(n != 0);
	const N *e = n->down();
	if(!e) return;
	l.push_back(e);
	while((e = e->next())) l.push_back(e);
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
	while((e = e->next())) {
		if(e == n) {
			// set previous to next
			prev->next(n->next());

			// detach
			n->next(0);
			n->up(0);
			return n;
		}
		prev = e;
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
			N *tmp = e;
			e = e->next();
			delete tmp;
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
