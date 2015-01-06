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

#ifndef AMBULANT_LIB_NODE_ITERATOR_H
#define AMBULANT_LIB_NODE_ITERATOR_H

#include "ambulant/config/config.h"

// pair
#include <utility>

namespace ambulant {

namespace lib {

/// Node tree iterator.
///
/// Traverses a constant XML tree.
///
/// Root designates the container to be traversed and remains const.
/// Each node is visited twice: once in the 'down' writing_mode and once in the 'up-next'.
///
/// the first element is: (true, root)
/// the last element is: (false, root)
/// incrementing last element results to the universal 'end' element (0, *).
///
/// (*it).first is true or false depending on the writing_mode
/// (*it).second is a pointer to the node.
template <class Node>
class const_tree_iterator  {

  //////////////
  public:

	/// The type of the values returned by this iterator.
	typedef std::pair<bool, const Node*> const_deref_type;

	const_tree_iterator()
	:	m_root(0), m_cur(0), m_move(&const_tree_iterator::down) {}

	/// Constructs a const_tree_iterator for the subtree rooted at the given node.
	const_tree_iterator(const Node *p)
	:	m_root(p), m_cur(p), m_move(&const_tree_iterator::down) {}

	const_tree_iterator& operator++() { if(m_cur)(this->*m_move)(); return *this;}	///< Operator

	const_tree_iterator operator++(int) { const_tree_iterator temp(*this); ++*this; return temp;}	///< Operator

	const_deref_type operator*() { return const_deref_type((m_move == &const_tree_iterator::down), m_cur);}	///< Operator

	// an instance of this object acts like a pointer to a const_deref_type
	// it->m_cur is the node, it->m_move is true if 'down'
	const_deref_type* operator->() {return (&**this); }	///< Operator

	/// True if iterator is exhausted.
	bool is_end() { return m_cur == 0;}

	/// Returns the current node.
	const Node *get_cur() const { return m_cur;}

	/// Returns the root of this iterator.
	const Node *get_root() const { return m_root;}

	/// Returns true if the two iterators move in the same direction
	bool same_move(const const_tree_iterator& o) const
		{ return m_move == o.m_move;}

///////////////
  protected:
	void down();	///< Internal helper method: interator should move to first child.
	void next();	///< Internal helper method: interator should move to next sibling.
	void up();	///< Internal helper method: interator should move up the tree.

	const Node *m_root;	///< Container traversed by this iterator.

	const Node *m_cur;	///< Current node.
	void (const_tree_iterator::*m_move)();	///< Method to get at next node.
};

////////////////////////
// tree_iterator

/// Node tree iterator.
///
/// Traverses an XML tree.
///
/// Root designates the container to be traversed and remains const.
/// Each node is visited twice: once in the 'down' direction and once in the 'up-next'.
///
/// the first element is: (true, root)
/// the last element is: (false, root)
/// incrementing last element results to the universal 'end' element (0, *).
///
/// (*it).first is true or false depending on the writing_mode
/// (*it).second is a pointer to the node.
template <class Node>
class tree_iterator : public const_tree_iterator<Node> {
  public:

	/// The type of the values returned by this iterator.
	typedef std::pair<bool, Node*> deref_type;

	tree_iterator() : const_tree_iterator<Node>(){}
	/// Consrtruct tree_iterator for subtree rooted at the given node.
	tree_iterator(Node *p) : const_tree_iterator<Node>(p){}
	virtual ~tree_iterator() {}

	// pre-increment
	tree_iterator& operator++() { if(this->m_cur)(this->*const_tree_iterator<Node>::m_move)(); return *this;}

	// post-increment
	tree_iterator operator++(int)
		{ tree_iterator temp(*this); ++*this; return temp;}

	// dereferencing this returns a pair of deref_type
	deref_type operator*()
		{ return deref_type( (this->m_move == &tree_iterator::down), const_cast<Node*>(this->m_cur));}

	// an instance of this object acts like a pointer to a deref_type
	// it->m_cur is the node, it->m_move is true if 'down'
	deref_type* operator->() {return (&**this); }
};

////////////////////////////
// const_tree_iterator inline implementation

template<class Node>
inline void const_tree_iterator<Node>::down() {
	const Node *p = m_cur->down();
	if(!p) m_move = &const_tree_iterator::next;
	else m_cur = p;
}

template<class Node>
inline void const_tree_iterator<Node>::next() {
	if(m_cur == m_root) { m_cur = 0; return;}
		const Node* it = m_cur->next();
	if(it) {
		m_move = &const_tree_iterator::down;
		m_cur = it;
	} else {
		m_move = &const_tree_iterator::up;
		(this->*m_move)();
	}
}

template<class Node>
inline void const_tree_iterator<Node>::up() {
	if(m_cur == m_root) { m_cur = 0; return;}
	const Node* it = m_cur->up();
	assert(it);
	if(it) {
		m_move = &const_tree_iterator::next;
		m_cur = it;
	}
}

} // namespace lib

} // namespace ambulant

/// Returns true if two iterators are in exactly the same state on the same tree.
template <class Node>
bool operator==(const ambulant::lib::const_tree_iterator<Node>& lhs,
	const ambulant::lib::const_tree_iterator<Node>& rhs) {
		if((lhs.get_cur() == 0 && rhs.get_cur() != 0) ||
			(lhs.get_cur() != 0 && rhs.get_cur() == 0)) return false;
		if(lhs.get_cur() == 0 && rhs.get_cur() == 0) return true;
		return lhs.get_root() == rhs.get_root() &&
			lhs.get_cur() == rhs.get_cur() &&
			lhs.same_move(rhs);
	}

template <class Node>
bool operator!=(const ambulant::lib::const_tree_iterator<Node>& lhs,
	const ambulant::lib::const_tree_iterator<Node>& rhs)
	{ return !(lhs == rhs);}

/// Returns true if two iterators are in exactly the same state on the same tree.
template <class Node>
bool operator==(const ambulant::lib::tree_iterator<Node>& lhs,
	const ambulant::lib::tree_iterator<Node>& rhs) {
		if((lhs.get_cur() == 0 && rhs.get_cur() != 0) ||
			(lhs.get_cur() != 0 && rhs.get_cur() == 0)) return false;
		if(lhs.get_cur() == 0 && rhs.get_cur() == 0) return true;
		return lhs.get_root() == rhs.get_root() &&
			lhs.get_cur() == rhs.get_cur() &&
			lhs.same_move(rhs);
	}

template <class Node>
bool operator!=(const ambulant::lib::tree_iterator<Node>& lhs,
	const ambulant::lib::tree_iterator<Node>& rhs)
	{ return !(lhs == rhs);}


#endif // AMBULANT_LIB_TREE_NODE_ITERATOR_H
