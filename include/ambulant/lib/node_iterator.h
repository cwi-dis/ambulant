/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
#ifndef AMBULANT_LIB_NODE_ITERATOR_H
#define AMBULANT_LIB_NODE_ITERATOR_H

// pair
#include <utility>

namespace ambulant {

namespace lib {

////////////////////////
// Tree Input Iterator

// Traverses an XML tree

// root designates the container to be traversed and remains const
// each node is visited twice: once in the 'down' direction and once in the 'up-next'

// the first element is: (root, down) 
// the last element is: (root, next_or_up) 
// incrementing last element results to the universal 'end' element (0, *)

// (*it).first is true or false depending on the direction
// (*it).second is a pointer to the node

template <class Node>
class const_tree_iterator  {

  //////////////
  public:

	typedef std::pair<bool, const Node*> const_deref_type;

	const_tree_iterator()
	:	m_root(0), m_cur(0), m_move(&const_tree_iterator::down) {}
		
	const_tree_iterator(const Node *p)
	:	m_root(p), m_cur(p), m_move(&const_tree_iterator::down) {}

	friend bool operator==(const const_tree_iterator& lhs, const const_tree_iterator& rhs) {
		if((lhs.m_cur == 0 && rhs.m_cur != 0) || 
			(lhs.m_cur != 0 && rhs.m_cur == 0)) return false;
		if(lhs.m_cur == 0 && rhs.m_cur == 0) return true;
		return lhs.m_root == rhs.m_root && lhs.m_cur == rhs.m_cur && lhs.m_move == rhs.m_move;
	}

	friend bool operator!=(const const_tree_iterator& lhs, const const_tree_iterator& rhs)
		{ return !(lhs == rhs);}

	// pre-increment
	const_tree_iterator& operator++() { if(m_cur)(this->*m_move)(); return *this;}
	
	// post-increment
	const_tree_iterator operator++(int) 
		{ const_tree_iterator temp(*this); ++*this; return temp;}

	// dereferencing this returns a pair of const_deref_type
	const_deref_type operator*() { return const_deref_type((m_move == &const_tree_iterator::down), m_cur);}

	// an instance of this object acts like a pointer to a const_deref_type
	// it->m_cur is the node, it->m_move is true if 'down'
    const_deref_type* operator->() {return (&**this); }

	bool is_end() { return m_cur == 0;}

  ///////////////
  protected:
	void down();
	void next();
	void up();

	// container traversed by this iterator
	const Node *m_root;
	
	// iterator state
	const Node *m_cur;
	void (const_tree_iterator::*m_move)();
};

////////////////////////
// tree_iterator

template <class Node>
class tree_iterator : public const_tree_iterator<Node> {
  public:
	typedef std::pair<bool, Node*> deref_type;

	tree_iterator() : const_tree_iterator<Node>(){}
	tree_iterator(Node *p) : const_tree_iterator<Node>(p){}

	friend bool operator==(const tree_iterator& lhs, const tree_iterator& rhs) {
		if((lhs.m_cur == 0 && rhs.m_cur != 0) || 
			(lhs.m_cur != 0 && rhs.m_cur == 0)) return false;
		if(lhs.m_cur == 0 && rhs.m_cur == 0) return true;
		return lhs.m_root == rhs.m_root && lhs.m_cur == rhs.m_cur && lhs.m_move == rhs.m_move;
	}

	friend bool operator!=(const tree_iterator& lhs, const tree_iterator& rhs)
		{ return !(lhs == rhs);}

	// pre-increment
	tree_iterator& operator++() { if(m_cur)(this->*m_move)(); return *this;}
	
	// post-increment
	tree_iterator operator++(int) 
		{ tree_iterator temp(*this); ++*this; return temp;}

	// dereferencing this returns a pair of deref_type
	deref_type operator*() 
		{ return deref_type( (m_move == &tree_iterator::down), const_cast<Node*>(m_cur));}

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
	if(it) {
		m_move = &const_tree_iterator::next;
		m_cur = it;
	}
}

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_TREE_NODE_ITERATOR_H
