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

#include "ambulant/lib/nfa.h"
#include "ambulant/lib/logger.h"
#include <vector>
#include <map>

#define NFA_VERBOSE 0

using namespace ambulant;

// static
int lib::nfa_node::nfa_nodes_counter = 0;

// Returns a clone of this nfa_node.
// The clone maybe a new instance or an already created instance.
// If a clone already exists in clones returns that instance else creates a new.
lib::nfa_node *lib::nfa_node::clone(std::map<nfa_node*, nfa_node*>& clones) {
	std::map<nfa_node*, nfa_node*>::iterator it = clones.find(this);
	if(it != clones.end()) return (*it).second;
	nfa_node* c = new nfa_node(edge);
	c->anchor = anchor;
	clones[this] = c;
	c->next1 = next1?next1->clone(clones):0;
	c->next2 = next2?next2->clone(clones):0;
	return c;
}

// Returns the set of nodes associated with this expr.
std::set<lib::nfa_node*>&
lib::nfa_expr::get_expr_nodes(std::set<nfa_node*>& nodes) const {
	nodes.clear();
	if(!start) return nodes;
	std::stack<nfa_node*> stack;
	stack.push(start);
	while(!stack.empty()) {
		nfa_node *t = stack.top();
		stack.pop();
		if(nodes.find(t) == nodes.end()) {
			if(t->next1) stack.push(t->next1);
			if(t->next2) stack.push(t->next2);
			nodes.insert(t);
		}
	}
	return nodes;
}
// Frees the set of nodes associated with this expr.
void lib::nfa_expr::free() {
	if(!start) return;
	std::set<nfa_node*> nodes;
	get_expr_nodes(nodes);
	for(std::set<nfa_node*>::iterator i = nodes.begin(); i!=nodes.end(); i++)
		delete (*i);
	start = accept = 0;
}

// Creates and returns a clone of the expr.
// This indirecly creates a clone of the graph associated with this expr
lib::nfa_expr* lib::nfa_expr::clone() const {
	nfa_expr *eptr = new nfa_expr();
	if(!empty()) {
		std::map<nfa_node*, nfa_node*> clones;
		eptr->accept = accept->clone(clones);
		eptr->start = start->clone(clones);
	}
	return eptr;
}

// Cats to this and consumes the provided expression.
// The provided expression becomes null.
const lib::nfa_expr& lib::nfa_expr::cat_expr_abs(nfa_expr *eptr) {
	if(eptr->empty()) return *this;
	assert(this != eptr);
	if(this->empty()) {
		start = eptr->start;
		accept = eptr->accept;
	} else {
		accept->set_transition(eptr->start->edge, eptr->start->next1, eptr->start->next2);
		accept->anchor |= eptr->start->anchor;
		accept = eptr->accept;
		delete eptr->start;
	}
	eptr->clear();
	return *this;
}

// Cats to this the provided expression
const lib::nfa_expr& lib::nfa_expr::cat_expr(const nfa_expr& expr) {
	if(expr.empty()) return *this;
	nfa_expr *eptr = expr.clone();
	cat_expr_abs(eptr);
	assert(eptr->empty());
	delete eptr;
	return *this;
}

// Ors to this and consumes the provided expression.
// The provided expression becomes null.
const lib::nfa_expr& lib::nfa_expr::or_expr_abs(nfa_expr *eptr) {
	if(eptr->empty()) return *this;
	assert(this != eptr);
	if(empty()) {
		start = eptr->start;
		accept = eptr->accept;
	} else {
		nfa_node *nfinal = new nfa_node(ACCEPT);
		nfa_node *nstart = new nfa_node(EPSILON, start, eptr->start);
		accept->set_transition(EPSILON, nfinal);
		eptr->accept->set_transition(EPSILON, nfinal);
		start = nstart;
		accept = nfinal;
	}
	eptr->clear();
	return *this;
}

// Ors to this the provided expression.
const lib::nfa_expr& lib::nfa_expr::or_expr(const nfa_expr& expr) {
	if(expr.empty()) return *this;
	nfa_expr *eptr = expr.clone();
	or_expr_abs(eptr);
	assert(eptr->empty());
	delete eptr;
	return *this;
}

const lib::nfa_expr& lib::nfa_expr::star() {
	if(!start) return *this;
	nfa_node *nfinal = new nfa_node(ACCEPT);
	nfa_node *nstart = new nfa_node(EPSILON, start, nfinal);
	accept->set_transition(EPSILON, nfinal, start);
	start = nstart;
	accept = nfinal;
	return *this;
}

const lib::nfa_expr& lib::nfa_expr::cat_expr(const char *psz)  {
	nfa_expr expr;
	if(!psz || !psz[0]) {
		expr.accept = new nfa_node(ACCEPT);
		expr.start = new nfa_node(EPSILON, accept);
	} else {
		expr.start = expr.accept = new nfa_node(ACCEPT);
		size_type n = strlen(psz);
		for(size_type i=0;i<n;i++) {
			nfa_node *next = new nfa_node(ACCEPT);
			expr.accept->set_transition((uchar_t)psz[i], next);
			expr.accept = next;
		}
	}
	cat_expr_abs(&expr);
	assert(expr.empty());
	return *this;
}

const lib::nfa_expr& lib::nfa_expr::power(int n) {
	if(!start) return *this;
	assert(n>=0);
	if(n==0) {
		free();
		accept = new nfa_node(ACCEPT);
		start = new nfa_node(EPSILON, accept);
	} else if(n>1) {
		n = std::min(n, REPEAT_LIMIT);
		nfa_expr *org_eptr = clone();
		for(int i=1;i<n-1;i++) {
			nfa_expr *eptr = org_eptr->clone();
			cat_expr_abs(eptr);
			assert(eptr->empty());
			delete eptr;
		}
		cat_expr_abs(org_eptr);
		assert(org_eptr->empty());
		delete org_eptr;
	}
	return *this;
}

const lib::nfa_expr& lib::nfa_expr::repeat(int n, int m) {
	if(!start) return *this;
	assert(m>=n);
	if(n==m) power(n);
	else if(n==0) { // {0,m}
		optional();
		if(m>1) power(m);
	} else { // {n,m}
		nfa_expr *eptr = clone();
		power(n);
		eptr->optional();
		eptr->power(m-n);
		cat_expr_abs(eptr);
		assert(eptr->empty());
		delete eptr;
	}
	return *this;
}

const lib::nfa_expr& lib::nfa_expr::set_to_char_class(const std::string& s) {
	if(!empty()) free();
	for(std::string::const_iterator it=s.begin();it!=s.end();it++)
		or_expr(*it);
	return *this;
}

// invariants checks
void lib::nfa_expr::verify1() const {
	std::set<nfa_node*> nodes;
	get_expr_nodes(nodes);
	int count = 0;
	for(std::set<nfa_node*>::iterator i = nodes.begin(); i!=nodes.end(); i++)
		if((*i)->next1 == 0 && (*i)->next2 == 0) count++;
	assert(empty() || count == 1);
}

void lib::nfa_expr::mark_expr(int index) {
	// mark start
	std::set<nfa_node*> nodes;
	std::stack<nfa_node*> stack;
	nodes.insert(start);
	if(start->is_epsilon_trans()) stack.push(start);
	if(!stack.empty()) closure(nodes, stack);
	int anchor = (1 << index);
	for(std::set<lib::nfa_node*>::const_iterator it = nodes.begin(); it!=nodes.end(); it++) {
		if((*it)->is_important_trans()) (*it)->anchor = anchor + 1;
	}
	// mark end
	accept->anchor = anchor;
}

// Adds to the set of nodes all epsilon-reachable-nodes from the nodes on the stack.
// The stack contains the unchecked nodes
// On exit the stack is empty
// param: nodes [in, out]
// param stack [in]
// static
void lib::nfa_expr::closure(std::set<nfa_node*>& nodes, std::stack<nfa_node*>& stack) {
	while(!stack.empty()) {
		nfa_node *t = stack.top();
		stack.pop();
		if(t->is_epsilon_trans()) {
			if(t->next1 && nodes.find(t->next1)==nodes.end()) {
				// transition: t --EPSILON--> next1
				nodes.insert(t->next1);
				if(t->next1->is_epsilon_trans()) stack.push(t->next1);
			}
			if(t->next2 && nodes.find(t->next2)==nodes.end()) {
				// transition: t --EPSILON--> next2
				nodes.insert(t->next2);
				if(t->next2->is_epsilon_trans()) stack.push(t->next2);
			}
		}
	}
}

// param: nodes [in, out] : on entry nodes contains the previous state nodes and
//		on exit the nodes due to the transitions on edge
// param: stack [out] : contains the epsilon-unchecked nodes
// param: edge [in] : the transition symbol
// static
void lib::nfa_expr::move(std::set<nfa_node*>& nodes, std::stack<nfa_node*>& stack, int edge) {
	assert(stack.empty());
	std::set<nfa_node*> from = nodes;
	nodes.clear();
	for(std::set<nfa_node*>::iterator it=from.begin(); it!=from.end(); it++) {
		if((*it)->edge == edge) {
			nodes.insert((*it)->next1);
			if((*it)->next1->is_epsilon_trans())
				stack.push((*it)->next1);
		}
	}
}

// Matches as much as possible from the input iterator
// Positions the iterator at the end of the parsed input and returns the length parsed.
// Returns -1 on failure to parse anything
std::ptrdiff_t
lib::nfa_expr::parse(std::string::const_iterator& it, const std::string::const_iterator& end_it) const {
	// The data struct used by this algorithm
	std::set<nfa_node*> nodes;
	std::stack<nfa_node*> stack;

	// Create startup state nodes
	nodes.insert(start);
	if(start->is_epsilon_trans()) stack.push(start);
	if(!stack.empty()) nfa_expr::closure(nodes, stack);
	assert(stack.empty());

	std::ptrdiff_t parsed_length = 0;
	std::ptrdiff_t delta = -1;
	if(accepts_state(nodes)) {
		delta = parsed_length;
	}

	// Execute the NFA for the provided input stream
	std::string::const_iterator it_test = it;
	while(it_test != end_it && !nodes.empty()) {
		nfa_expr::move(nodes, stack, uchar_t(*it_test));
		if(!stack.empty()) nfa_expr::closure(nodes, stack);
		assert(stack.empty());
		it_test++;parsed_length++;
		if(accepts_state(nodes)) {
			it = it_test;
			delta = parsed_length;
		}
	}
	return delta;
}

// Creates a matcher for the argument string
lib::nfa_matcher*
lib::nfa_expr::create_matcher(const std::string& str) const {
	return new nfa_matcher(str, this);
}


/////////////////////////////////////////
// nfa_matcher

// param: nodes [in, out] : on entry nodes contains the previous state nodes and
//		on exit the nodes due to the transitions on edge
// param: stack [out] : contains the epsilon-unchecked nodes
// param: edge [in] : the transition symbol
void lib::nfa_matcher::move(std::set<nfa_node*>& nodes, std::stack<nfa_node*>& stack, int edge,
	std::set<int>& groups) {
	assert(stack.empty());
	std::set<nfa_node*> from = nodes;
	nodes.clear();
	for(std::set<nfa_node*>::iterator it=from.begin(); it!=from.end(); it++) {
		if((*it)->edge == edge) {
			if((*it)->anchor != 0) {
				get_groups((*it)->anchor, groups);
			}
			nodes.insert((*it)->next1);
			if((*it)->next1->is_epsilon_trans()) {
				stack.push((*it)->next1);
			}
		}
	}
}

bool lib::nfa_matcher::match(const std::string& str) {
	// The containers used by this algorithm
	std::set<nfa_node*> nodes;
	std::stack<nfa_node*> stack;

	// Create startup state nodes
	nodes.insert(m_expr->start);
	if(m_expr->start->is_epsilon_trans()) stack.push(m_expr->start);
	if(!stack.empty()) nfa_expr::closure(nodes, stack);
	assert(stack.empty());

#if NFA_VERBOSE
	// Report expected start symbols
	std::cout << "seen:none expecting:" << nodes << std::endl;
#endif

	// Execute the NFA for the provided string
	for(size_type i=0;i<str.length();i++) {

		// New state nodes after seen str[i]
		std::set<int> begin_groups;
		move(nodes, stack, uchar_t(str[i]), begin_groups);
		if(!stack.empty()) nfa_expr::closure(nodes, stack);
		assert(stack.empty());
		if(!begin_groups.empty())
			set_groups_begin(begin_groups, int(i));

		std::set<int> end_groups;
		for(std::set<nfa_node*>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
			if( (*it)->anchor && ((*it)->anchor & 1) == 0) get_groups((*it)->anchor, end_groups);
		}
		if(!end_groups.empty())
			set_groups_end(end_groups, int(i)+1);
		if(!nodes.empty())
			set_match_end(int(i)+1);

#if NFA_VERBOSE
		// Report seen and expected symbols
		std::cout << "seen:" << str.substr(0, i+1) << " expecting:" << nodes << std::endl;
#endif

		if(nodes.empty()) {
			// Missmatch; report offending position end break out
#if NFA_VERBOSE
			std::cout << "F>> " << str.substr(0, i) << '^' << str[i] << '^' << std::endl;
#endif
			break;
		}
	}

#if NFA_VERBOSE
	// Report groups seen
	dump_groups(std::cout);
#endif
	return !nodes.empty() && nodes.find(m_expr->accept) != nodes.end();
}


// static
void lib::nfa_matcher::get_groups(int anchor, std::set<int>& groups) {
	for(int i=1;i<(int)sizeof(int);i++) {
		anchor >>= 1;
		if(anchor & 1) groups.insert(i);
	}
}

void lib::nfa_matcher::dump_groups(std::ostream& os) {
	for(int i=0;i<max_group;i++) {
		if(seen_group(i)) {
			std::cout << "group " << i << " : " << get_group(i) << std::endl;
		}
	}
}

///////////////////////

std::ostream& operator<<(std::ostream& os, const std::set<lib::nfa_node*>& nodes) {
//	int count = 0;
	std::set<char> next_chars;
	int accept_counter = 0;
//	int epsilon_counter = 0;
	for(std::set<lib::nfa_node*>::const_iterator it = nodes.begin(); it!=nodes.end(); it++) {
		if((*it)->is_important_trans()) {
			if((*it)->is_accept_node())
				accept_counter++;
			else
				next_chars.insert((*it)->get_edge_repr());
		}
	}
	for(std::set<char>::iterator it2 = next_chars.begin(); it2!=next_chars.end(); it2++)
		os << (*it2);
	while((accept_counter--)>0) os << '$';
	return os;
}

