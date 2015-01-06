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

#ifndef AMBULANT_LIB_NFA_H
#define AMBULANT_LIB_NFA_H

#include "ambulant/config/config.h"

#include <string>
#include <ctype.h>
#include <set>
#include <map>
#include <stack>
#include <string.h>
#include <cassert>

namespace ambulant {

namespace lib {

//////////////////////////
//
// A simple not optimized NFA based regular expression matcher.
//
// May be used to match short strings created by
// the scanner against hand made (coded) regex.
//
// The intented usage is:
// a) shring the string to be matched into tokens using the scanner
// b) use the matcher against the short string of the tokens.
// This way for example a decimal becomes the literals: d | d.d
// instead of [0-9]+ | [0-9]+.[0-9]+ resulting to an improvement
// nore than an order of magnitude (~20 times).

const int EPSILON = -1;
const int ACCEPT  = -9;
const int REPEAT_LIMIT  = 1024;

// An nfa node represents a node in a
// nonderministic finite automaton (NFA)
// and is associated with 2 transitions
// that occur on symbol 'edge'.
//
// NFA nodes could be linearized in a
// continous memory buffer for efficiency.
//
// nfa node flavors:
// edge: {int, nfa_node*, 0, 0}
// accept: {-9, 0, 0, 0}
// epsilon: {-1, nfa_node*, nfa_node*, 0}

struct nfa_node {
	typedef unsigned char uchar_t;
	typedef std::string::size_type size_type;

	nfa_node(int e, nfa_node *n1 = 0, nfa_node *n2 = 0)
	:	edge(e), next1(n1), next2(n2), anchor(0) {
		++nfa_nodes_counter;
	}
	~nfa_node() { --nfa_nodes_counter;}

	void set_transition(int e, nfa_node *n1 = 0, nfa_node *n2 = 0)
		{ edge = e; next1 = n1; next2 = n2;}

	bool is_epsilon_trans() const { return edge == EPSILON;}
	bool is_important_trans() const { return edge != EPSILON;}
	bool is_accept_node() { return edge == ACCEPT;}
	int get_anchor() const { return anchor;}
	char get_edge_repr() const { return (edge == EPSILON)?'!':((edge == ACCEPT)?'$':char(edge));}

	// Used for expr construction
	nfa_node *clone(std::map<nfa_node*, nfa_node*>& clones);

	int edge;
	nfa_node *next1;
	nfa_node *next2;
	int anchor;

	// verifier
	static int nfa_nodes_counter;
};

// forward declaration;
class nfa_matcher;

// An nfa_expr is a directed graph of nfa nodes that represent a NFA.
// An nfa_expr object keeps a reference to the start and the accept NFA nodes
// and is the owner of the NFA nodes.
class nfa_expr {
  public:
	typedef unsigned char uchar_t;
	typedef std::string::size_type size_type;

	nfa_expr() : accept(0), start(0) {}

	explicit nfa_expr(int edge) : accept(0), start(0) {
		accept = new nfa_node(ACCEPT);
		start = new nfa_node(edge, accept);
	}

	explicit nfa_expr(char edge, bool opt = false) : accept(0), start(0) {
		accept = new nfa_node(ACCEPT);
		start = new nfa_node(uchar_t(edge), accept);
		if(opt) optional();
	}

	nfa_expr(const char *s)
	:	accept(0), start(0) {
		cat_expr(s);
	}

	// Assignment constructor.
	nfa_expr(const nfa_expr& other)
	:	accept(0), start(0) {
		std::map<nfa_node*, nfa_node*> clones;
		accept = other.accept->clone(clones);
		start = other.start->clone(clones);
	}

	// Copy constructor
	const nfa_expr& operator=(const nfa_expr& expr) {
		if(this == &expr) return *this;
		free();
		nfa_expr *eptr = expr.clone();
		start = eptr->start;
		accept = eptr->accept;
		eptr->clear();
		delete eptr;
		return *this;
	}

	////////////////
	// Constructors that simplify some common cases

	nfa_expr(const char *s1, const char *s2) : accept(0), start(0) {
		cat_expr(s1);
		or_expr(s2);
	}

	nfa_expr(const char *s1, const char *s2, const char *s3) : accept(0), start(0) {
		cat_expr(s1);
		or_expr(s2);
		or_expr(s3);
	}

	nfa_expr(const char *s1, const char *s2, const char *s3, const char *s4) : accept(0), start(0) {
		cat_expr(s1);
		or_expr(s2);
		or_expr(s3);
		or_expr(s4);
	}

	nfa_expr(char c1, char c2) : accept(0), start(0) {
		cat_expr(c1);
		or_expr(c2);
	}

	nfa_expr(char c1, char c2, char c3) : accept(0), start(0) {
		cat_expr(c1);
		or_expr(c2);
		or_expr(c3);
	}

	nfa_expr(char c1, char c2, char c3, char c4) : accept(0), start(0) {
		cat_expr(c1);
		or_expr(c2);
		or_expr(c3);
		or_expr(c4);
	}

	// The destructor deletes all the nodes of the expr
	~nfa_expr() { free(); }

	// Free the NFA nodes of this expr
	void free();

	// nullify this without deleting nodes
	void clear() { start = accept = 0;}

	// deep copy of this
	nfa_expr *clone() const;

	// Returns true when the urderlying NFA is empty
	bool empty() const {return start == 0;}

	// Returns the number of nodes of this expr
	int size() const {
		std::set<nfa_node*> nodes;
		return int(get_expr_nodes(nodes).size());
	}

	// Returns the memory consumed by this expr in bytes
	int memsize() const {
		return int(size()*sizeof(nfa_node)) + int(sizeof(nfa_expr));
	}

	void mark_expr(int index);

	//////////////////////////
	// Match

	// Matches as much as possible from the input.
	// Positions the iterator at the end of the parsed input and returns the length parsed.
	// Returns -1 on failure to parse anything
	std::ptrdiff_t
	parse(std::string::const_iterator& it, const std::string::const_iterator& end) const;

	// Matches the argument string against the regex
	// that this nfa_expr represents.
	bool matches(const std::string& str) const {
		std::string::const_iterator b = str.begin(), e = str.end();
		return (size_t)parse(b, e) == str.length();
	}

	// Creates a matcher for the argument string
	nfa_matcher* create_matcher(const std::string& str) const;

	//////////////////////////
	// Regex operations

	// expr expr
	const nfa_expr& cat_expr(const nfa_expr& expr);

	// expr | expr
	const nfa_expr& or_expr(const nfa_expr& expr);

	// expr?
	const nfa_expr& optional() {
		nfa_expr e(EPSILON);
		or_expr_abs(&e);
		assert(e.empty());
		return *this;
	}

	// expr*
	const nfa_expr& star();

	// expr+
	const nfa_expr& plus() {
		nfa_expr *eptr = this->clone();
		eptr->star();
		cat_expr_abs(eptr);
		assert(eptr->empty());
		delete eptr;
		return *this;
	}

	// expr{n}
	const nfa_expr& power(int n);

	// expr{n, m}
	const nfa_expr& repeat(int n, int m);

	// sets this expression to the char class
	// consisting of the literals in the string
	const lib::nfa_expr&
	set_to_char_class(const std::string& s);

	//////////////////
	// Regex operations shortcuts

	const nfa_expr& cat_expr(char ch)	{
		nfa_expr e(ch);
		cat_expr_abs(&e);
		assert(e.empty());
		return *this;
	}

	const nfa_expr& cat_expr(const char *psz);


	const nfa_expr& or_expr(char ch) {
		nfa_expr e(ch);
		or_expr_abs(&e);
		assert(e.empty());
		return *this;
	}

	const nfa_expr& or_expr(const char *psz) {
		nfa_expr e(psz);
		or_expr_abs(&e);
		assert(e.empty());
		return *this;
	}

	// Alt cat_expr
	const nfa_expr& operator+=(char ch) { return cat_expr(ch);}
	const nfa_expr& operator+=(const char *psz) { return cat_expr(psz);}
	const nfa_expr& operator+=(const nfa_expr& e) { return cat_expr(e);}

	// Alt or_expr
	const nfa_expr& operator*=(char ch) { return or_expr(ch);}
	const nfa_expr& operator*=(const char *psz) { return or_expr(psz);}
	const nfa_expr& operator*=(const nfa_expr& e) { return or_expr(e);}

	// Returns the nodes of this expression
	std::set<nfa_node*>& get_expr_nodes(std::set<nfa_node*>& nodes) const;

	// expr expr
	// This consumes expr which becomes null
	const nfa_expr& cat_expr_abs(nfa_expr *eptr);

	// expr | expr
	// This consumes expr which becomes null
	const nfa_expr& or_expr_abs(nfa_expr *eptr);

	friend class nfa_matcher;

  private:

	// invariants checks
	void verify1() const;

	// match algorithm execution
	static void closure(std::set<nfa_node*>& nodes, std::stack<nfa_node*>& stack);
	static void  move(std::set<nfa_node*>& nodes, std::stack<nfa_node*>& stack, int edge);
	bool accepts_state(const std::set<nfa_node*>& nodes) const {
		return !nodes.empty() && nodes.find(accept) != nodes.end();
	}

	nfa_node *accept;
	nfa_node *start;
};


class nfa_matcher {
  public:
	typedef unsigned char uchar_t;
	typedef std::string::size_type size_type;
	enum {max_group = 32};

	nfa_matcher(const std::string& str, const nfa_expr* expr)
	:	m_str(str), m_expr(expr) {
		memset(m_group_begin, 0, max_group*sizeof(int));
		memset(m_group_end, 0, max_group*sizeof(int));
		m_matches = match(m_str);
	}

	bool seen_group(int i) const {
		return i<max_group && m_group_end[i] > 0;
	}

	std::string get_group(int i) const {
		if(i>=max_group || m_group_end[i] == 0) return "";
		int n = m_group_end[i] - m_group_begin[i];
		return m_str.substr(m_group_begin[i], n);
	}

	int length() const {
		return m_group_end[0] - m_group_begin[0];
	}

	void set_groups_begin(const std::set<int>& groups, int pos) {
		std::set<int>::const_iterator it;
		for(it=groups.begin();it!=groups.end();it++)
			m_group_begin[*it] = pos;
	}

	void set_groups_end(const std::set<int>& groups, int pos) {
		std::set<int>::const_iterator it;
		for(it=groups.begin();it!=groups.end();it++)
			m_group_end[*it] = pos;
	}

	void set_match_end(int pos) {
		m_group_end[0] = pos;
	}

	bool matches() const { return m_matches; }

	void dump_groups(std::ostream& os);

  private:
	// NFA search algorithm helpers
	bool match(const std::string& str);
	void  move(std::set<nfa_node*>& nodes, std::stack<nfa_node*>& stack,
		int edge, std::set<int>& groups);
	static void get_groups(int anchor, std::set<int>& groups);

	std::string m_str;
	const nfa_expr* m_expr;
	bool m_matches;
	int m_group_begin[max_group];
	int m_group_end[max_group];
};

} // namespace lib

} // namespace ambulant

#include <ostream>

inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::nfa_node& n) {
	return os << n.get_edge_repr();
}
std::ostream& operator<<(std::ostream& os, const std::set<ambulant::lib::nfa_node*>& nodes);

#endif // AMBULANT_LIB_NFA_H
