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

#include "ambulant/lib/string_util.h"

#include "ambulant/lib/logger.h"

using namespace ambulant;


//////////////////
// tokens_vector

lib::tokens_vector::tokens_vector(const char* entry, const char* delims) {
	std::string s = (!entry || !entry[0])?"":entry;
	typedef std::string::size_type size_type;
	size_type offset = 0;
	while(offset != std::string::npos) {
		size_type i = s.find_first_of(delims, offset);
		if(i != std::string::npos) {
			push_back(std::string(s.c_str() + offset, i-offset));
			offset = i+1;
		} else {
			push_back(std::string(s.c_str() + offset));
			offset = std::string::npos;
		}
	}	
}			

std::string lib::tokens_vector::join(size_type i, char sep) {
	std::string s;
	size_type n = size();
	if(i<n) s +=  (*this)[i++]; // this->at(i) seems missing from gcc 2.95
	for(;i<n;i++) {
		s += sep;
		s += (*this)[i];
	}
	return s;
}


//////////////////
// nfa_expr

// static
int lib::nfa_node::nfa_nodes_counter = 0;

lib::nfa_node *lib::nfa_node::clone(std::set<nfa_node*>& clones) {
	std::set<nfa_node*>::iterator it = clones.find((nfa_node*)this);
	if(it != clones.end()) return myclone;
	myclone = new nfa_node(edge);
	myclone->anchor = anchor;
	clones.insert((nfa_node*)this);
	myclone->next1 = next1?next1->clone(clones):0; 
	myclone->next2 = next2?next2->clone(clones):0; 
	return myclone;
}

// Free the NFA nodes reachable by this
void lib::nfa_expr::free() {
	if(!start) return;
	std::set<nfa_node*> nodes;
	get_expr_nodes(nodes);
	for(std::set<nfa_node*>::iterator i = nodes.begin(); i!=nodes.end(); i++)
		delete (*i);
	start = accept = 0;
}

lib::nfa_expr lib::nfa_expr::clone() const {
	nfa_expr expr;
	if(!empty()) {
		std::set<nfa_node*> cloned;
		expr.accept = accept->clone(cloned);
		expr.start = start->clone(cloned);
	}
	return expr;
}

// This consumes e which becomes null
const lib::nfa_expr& lib::nfa_expr::cat_expr(nfa_expr& e) {
	if(e.empty()) return *this;
	assert(this != &e);
	if(this->empty()) {
		start = e.start;
		accept = e.accept;
	} else {
		accept->set_transition(e.start->edge, e.start->next1, e.start->next2);
		accept->anchor |= e.start->anchor;
		accept = e.accept;
		delete e.start;
	}
	e.clear();
	return *this;
}

const lib::nfa_expr& lib::nfa_expr::cat_expr_clone(const nfa_expr& expr) {
	if(expr.empty()) return *this;
	nfa_expr e = expr.clone();
	cat_expr(e);
	assert(e.empty());
	return *this;
}

// This consumes e which becomes null
const lib::nfa_expr& lib::nfa_expr::or_expr(nfa_expr& e) {
	if(e.empty()) return *this;
	assert(this != &e);
	if(empty()) {
		start = e.start;
		accept = e.accept;
	} else {
		nfa_node *nfinal = new nfa_node(ACCEPT);
		nfa_node *nstart = new nfa_node(EPSILON, start, e.start);
		accept->set_transition(EPSILON, nfinal);
		e.accept->set_transition(EPSILON, nfinal);
		start = nstart;
		accept = nfinal;
	}
	e.clear();
	return *this;
}

const lib::nfa_expr& lib::nfa_expr::or_expr_clone(const nfa_expr& expr) {
	if(expr.empty()) return *this;
	nfa_expr e = expr.clone();
	or_expr(e);
	assert(e.empty());
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
	cat_expr(expr);
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
		nfa_expr org(clone());
		for(int i=1;i<n-1;i++) cat_expr(org.clone());
		cat_expr(org);
		assert(org.empty());
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
		nfa_expr e(clone());
		power(n);
		e.optional();
		e.power(m-n);
		cat_expr(e);
		assert(e.empty());
	}
	return *this;
}

std::set<lib::nfa_node*>& 
lib::nfa_expr::get_expr_nodes(std::set<nfa_node*>& nodes) const {
	nodes.clear();
	if(!start) return nodes;
	std::stack<nfa_node*> sstack;
	sstack.push(start);
	while(!sstack.empty()) {
		nfa_node *t = sstack.top();sstack.pop();
		if(nodes.find(t) == nodes.end()) {
			if(t->next1) sstack.push(t->next1);
			if(t->next2) sstack.push(t->next2);
			nodes.insert(t);
		}
	}
	return nodes;
}

// invariants checks
void lib::nfa_expr::verify1() const {
	std::set<nfa_node*> nodes;
	get_expr_nodes(nodes);
	int count = 0;
	for(std::set<nfa_node*>::iterator i = nodes.begin(); i!=nodes.end(); i++)
		if((*i)->next1 == 0 && (*i)->next2 == 0) count++;
#ifndef AMBULANT_NO_IOSTREAMS
	if(count != 1) std::cout << "invariant 1 violation" << std::endl;
#endif
}

// Alt cat
lib::nfa_expr operator+(const lib::nfa_expr& e1, const lib::nfa_expr& e2) {
	lib::nfa_expr expr(e1.clone());
	expr.cat_expr(e2.clone());
	return expr;
}

// Alt or
lib::nfa_expr operator*(const lib::nfa_expr& e1, const lib::nfa_expr& e2) {
	lib::nfa_expr expr(e1.clone());
	expr.or_expr(e2.clone());
	return expr;
}

// Called after move
// The stack contains the unchecked nodes
// On exit the stack is empty  
void lib::nfa_expr::closure(std::stack<nfa_node*>& ststack, std::set<nfa_node*>& nodes) {
	while(!ststack.empty()) {
		nfa_node *t = ststack.top();ststack.pop();
		if(t->is_epsilon_trans()) {
			if(t->next1 && nodes.find(t->next1)==nodes.end()) {
				// transition: t --EPSILON--> next1
				nodes.insert(t->next1);
				if(t->next1->is_epsilon_trans()) ststack.push(t->next1);
			}
			if(t->next2 && nodes.find(t->next2)==nodes.end()) {
				// transition: t --EPSILON--> next2
				nodes.insert(t->next2);
				if(t->next2->is_epsilon_trans()) ststack.push(t->next2);
			}
		}
	}
}

// Move nodes 
void lib::nfa_expr::move(std::set<nfa_node*>& nodes, int edge, std::stack<nfa_node*>& ststack) {
	while(!ststack.empty()) ststack.pop();
	std::set<nfa_node*> movestates;
	for(std::set<nfa_node*>::const_iterator i = nodes.begin(); i!=nodes.end(); i++) {
		if((*i)->edge == edge) {
			movestates.insert((*i)->next1);
			if((*i)->next1->is_epsilon_trans()) ststack.push((*i)->next1);
		}
	}
	nodes = movestates;
}

bool lib::nfa_expr::match(const std::string& str) {
	const nfa_expr& expr = *this;
	std::set<nfa_node*> nodes;
	std::stack<nfa_node*> ststack;
	
	// Move to start
	nodes.insert(expr.start);
	if(expr.start->is_epsilon_trans()) ststack.push(expr.start);
	closure(ststack, nodes);

	// Move driven by string
	for(size_type i=0;i<str.length();i++) {
		move(nodes, (uchar_t)str.at(i), ststack);
		if(!ststack.empty()) closure(ststack, nodes);
	}
	return nodes.find(expr.accept) != nodes.end();
}

bool lib::nfa_expr::match(const std::string& str, bool anchors) {
	const nfa_expr& expr = *this;
	std::set<nfa_node*> nodes;
	std::stack<nfa_node*> ststack;
	
	// Move to start
	nodes.insert(expr.start);
	if(expr.start->is_epsilon_trans()) ststack.push(expr.start);
	closure(ststack, nodes);

	std::vector<size_type> ixv;
	
	// Move driven by string
	for(size_type i=0;i<str.length();i++) {
		move(nodes, (uchar_t)str.at(i), ststack);
		if(!ststack.empty()) closure(ststack, nodes);
#ifndef AMBULANT_NO_IOSTREAMS		
		std::cout << int(i) << ": ";
		for(std::set<nfa_node*>::const_iterator it = nodes.begin(); it!=nodes.end(); it++) {
			if((*it)->anchor & GROUP_BEGIN) std::cout << "begin, ";
			if((*it)->anchor & GROUP_END) std::cout << "end, ";
		}
		std::cout << std::endl;
	}
#endif
	return nodes.find(expr.accept) != nodes.end();
}


//static 
lib::nfa_expr lib::nfa_expr::create_char_class_expr(const std::string& s) {
	lib::nfa_expr e;
	for(std::string::const_iterator it=s.begin();it!=s.end();it++)
		e.or_expr(*it);
	return e;
}




