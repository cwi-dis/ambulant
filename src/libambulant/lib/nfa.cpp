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

#include "ambulant/lib/nfa.h"
#include <vector>
#include <map>

#include "ambulant/lib/logger.h"

#define NFA_VERBOSE 1

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

void lib::nfa_expr::mark_expr(int anchor) {
	std::set<nfa_node*> nodes;
	std::stack<nfa_node*> ststack;
	nodes.insert(start);
	if(start->is_epsilon_trans()) ststack.push(start);
	if(!ststack.empty()) closure(nodes, ststack);
	for(std::set<lib::nfa_node*>::const_iterator it = nodes.begin(); it!=nodes.end(); it++) {
		if((*it)->is_important_trans()) (*it)->anchor = anchor;
	}
}

// Adds to the set of nodes all epsilon-reachable-nodes from the nodes on the stack.
// The stack contains the unchecked nodes
// On exit the stack is empty  
// param: nodes [in, out]
// param stack [in]
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

// param: nodes [in, out] : on entry nodes contains the previous nodes and on exit the nodes due to transitions on edge 
// param: stack [out] : contains the epsilon-unchecked nodes
// param: edge [in] : the transition symbol
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

bool lib::nfa_expr::match(const std::string& str) {
	std::set<nfa_node*> nodes;
	std::stack<nfa_node*> stack;
	// Start scanning from this expr start.
	nodes.insert(start);
	if(start->is_epsilon_trans()) stack.push(start);
	if(!stack.empty()) closure(nodes, stack);
	assert(stack.empty());
#if !defined(AMBULANT_NO_IOSTREAMS) && NFA_VERBOSE
	std::cout << " expecting: " << nodes << std::endl;;
#endif
	// Transitions driven by string
	for(size_type i=0;i<str.length();i++) {
		move(nodes, stack, uchar_t(str[i]));
		if(!stack.empty()) closure(nodes, stack);
		assert(stack.empty());
#if !defined(AMBULANT_NO_IOSTREAMS) && NFA_VERBOSE
		std::cout << str.substr(0, i+1) << " expecting: " << nodes << std::endl;
#endif
		if(nodes.empty()) {
#if !defined(AMBULANT_NO_IOSTREAMS) && NFA_VERBOSE
			// point out mismatch		
			std::cout << "F>> " << str.substr(0, i) << '^' << str[i] << '^' << std::endl;
#endif
			break;
		}
	}
	return nodes.find(accept) != nodes.end();
}

#ifndef AMBULANT_NO_IOSTREAMS
std::ostream& operator<<(std::ostream& os, const std::set<lib::nfa_node*>& nodes) {
	int count = 0;
	std::set<char> next_chars;
	int accept_counter = 0;
	int epsilon_counter = 0;
	for(std::set<lib::nfa_node*>::const_iterator it = nodes.begin(); it!=nodes.end(); it++) {
		if((*it)->is_important_trans()) {
			if((*it)->is_accept_node()) 
				accept_counter++;
			else
				next_chars.insert((*it)->get_edge_repr());
		} else {
			next_chars.insert((*it)->get_edge_repr());
		}
	}
	for(std::set<char>::iterator it2 = next_chars.begin(); it2!=next_chars.end(); it2++)
		os << (*it2);
	while((accept_counter--)>0) os << '$';
	return os;
}
#endif






