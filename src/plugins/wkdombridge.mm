#include "wkdombridge.h"
#include "ambulant/lib/node_navigator.h"
#include "ambulant/lib/string_util.h"

using namespace ambulant;

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

lib::node_factory *
create_wkdom_node_factory(void *webframe)
{
	WebFrame *wf = (WebFrame *)webframe;
	if (!wf) {
		lib::logger::get_logger()->trace("wkdombridge: no WebFrame");
		return NULL;
	}
	DOMDocument *doc = [wf DOMDocument];
	if (!doc) {
		lib::logger::get_logger()->trace("wkdombridge: no DOMDocument in WebFrame");
		return NULL;
	}
	DOMImplementation *imp = [doc implementation];
	if (!imp) {
		lib::logger::get_logger()->trace("wkdombridge: DOMDocument has no DOMImplementation?");
		return NULL;
	}
	return new wkdom_node_factory(imp);
}

wkdom_node_factory::wkdom_node_factory(DOMImplementation *domimp)
{
	assert(domimp);
	wkdom_node::s_implementation = domimp;
}

lib::node *
wkdom_node_factory::new_node(const char *local_name, const char **attrs , const lib::node_context *ctx )
{
	DOMDocument *doc = wkdom_node::ctx2doc(ctx);
	assert(doc);
	NSString *tag = [NSString stringWithCString: local_name];
	DOMElement *w = [doc createElement: tag];
	while (*attrs) {
		const char *attrname = attrs[0];
		const char *attrvalue = attrs[1];
		attrs += 2;
		assert(attrvalue);
		NSString *nsattrname = [NSString stringWithCString: attrname];
		NSString *nsattrvalue = [NSString stringWithCString: attrvalue];
		[w setAttribute: nsattrname : nsattrvalue];
		AM_DBG NSLog(@"wkdom_node: <%@>.set_attribute(%@, %@)", [w tagName], nsattrname, nsattrvalue);
	}
	return wkdom_node::id2node(w, ctx);
}


lib::node *
wkdom_node_factory::new_node(const lib::xml_string& local_name, const char **attrs, const lib::node_context *ctx)
{
	DOMDocument *doc = wkdom_node::ctx2doc(ctx);
	assert(doc);
	NSString *tag = [NSString stringWithCString: local_name.c_str()];
	DOMElement *w = [doc createElement: tag];
	while (*attrs) {
		const char *attrname = attrs[0];
		const char *attrvalue = attrs[1];
		attrs += 2;
		assert(attrvalue);
		NSString *nsattrname = [NSString stringWithCString: attrname];
		NSString *nsattrvalue = [NSString stringWithCString: attrvalue];
		[w setAttribute: nsattrname : nsattrvalue];
		AM_DBG NSLog(@"wkdom_node: <%@>.set_attribute(%@, %@)", [w tagName], nsattrname, nsattrvalue);
	}
	return wkdom_node::id2node(w, ctx);
}

lib::node *
wkdom_node_factory::new_node(const lib::q_name_pair& qn, const lib::q_attributes_list& qattrs, const lib::node_context *ctx)
{
	DOMDocument *doc = wkdom_node::ctx2doc(ctx);
	assert(doc);
	NSString *uri = [NSString stringWithCString: qn.first.c_str()];
	NSString *tag = [NSString stringWithCString: qn.second.c_str()];
	DOMElement *w = [doc createElementNS: uri : tag];
	lib::q_attributes_list::const_iterator i;
	for (i=qattrs.begin(); i != qattrs.end(); i++) {
		const lib::xml_string& attruri = (*i).first.first;
		const lib::xml_string& attrname = (*i).first.second;
		const lib::xml_string& attrvalue = (*i).second;
		NSString *nsattruri = [NSString stringWithCString: attruri.c_str()];
		NSString *nsattrname = [NSString stringWithCString: attrname.c_str()];
		NSString *nsattrvalue = [NSString stringWithCString: attrvalue.c_str()];
		if ([nsattruri length] == 0) {
			[w setAttribute: nsattrname : nsattrvalue];
			AM_DBG NSLog(@"wkdom_node: <%@>.set_attribute(%@, %@)", [w tagName], nsattrname, nsattrvalue);
		} else {
			[w setAttributeNS: nsattruri : nsattrname : nsattrvalue];
			AM_DBG NSLog(@"wkdom_node: <%@>.set_attribute(%@:%@, %@)", [w tagName], nsattruri, nsattrname, nsattrvalue);
		}
	}
	return wkdom_node::id2node(w, ctx);
}

lib::node *
wkdom_node_factory::new_node(const lib::node* other)
{
	const wkdom_node *o = dynamic_cast<const wkdom_node *>(other);
	assert(o);
	DOMNode *w = [o->m_self cloneNode: true];
	return wkdom_node::id2node(w, o->get_context());
}

lib::node *
wkdom_node_factory::new_data_node(const char *data, size_t size, const lib::node_context *ctx)
{
	DOMDocument *doc = wkdom_node::ctx2doc(ctx);
	assert(doc);
	NSString *str = [[NSString alloc] initWithBytes: data length: size encoding:NSUTF8StringEncoding];
	DOMText *w = [doc createTextNode: str];
	return wkdom_node::id2node(w, ctx);
}

wkdom_node::wkdom_node(DOMElement *w, const lib::node_context *ctx)
:	m_self(w),
	m_context(ctx)
{
	assert(m_self);
	[m_self retain];
}

std::map<const DOMNode *, wkdom_node *> wkdom_node::s_id2node;
wkdom_node *
wkdom_node::id2node(const DOMNode *w, const lib::node_context *ctx)
{
	if (w == NULL) return NULL;
	std::map<const DOMNode *, wkdom_node *>::iterator i = s_id2node.find(w);
	if (i == s_id2node.end()) {
		wkdom_node *wrapper = new wkdom_node(w, ctx);
		assert(wrapper);
		s_id2node[w] = wrapper;
		AM_DBG lib::logger::get_logger()->debug("id2node: NEW 0x%x -> 0x%x %d %s", w, wrapper, [w nodeType], [[w nodeName] UTF8String]);
		return wrapper;
	}
	AM_DBG lib::logger::get_logger()->debug("id2node:     0x%x -> 0x%x", w, (*i).second);
	return (*i).second;
}

std::map<const lib::node_context*, DOMDocument *> wkdom_node::s_ctx2doc;
DOMImplementation *wkdom_node::s_implementation;
DOMDocument *
wkdom_node::ctx2doc(const lib::node_context *ctx)
{
	assert(ctx);
	std::map<const lib::node_context*, DOMDocument *>::iterator i = s_ctx2doc.find(ctx);
	if (i == s_ctx2doc.end()) {
		if (!s_implementation) {
			lib::logger::get_logger()->error("wkdom_node: no DOMImplementation, cannot create new nodes");
			return NULL;
		}
		DOMDocument *doc = [s_implementation createDocument: @"" : @"" : NULL];
		[doc retain];
		s_ctx2doc[ctx] = doc;
		return doc;
	}
	return (*i).second;
}

// Two static helper routines, to give error messages
static void
readonly()
{
	lib::logger::get_logger()->fatal("programmer error: attempt to write to readonly DOM node");
}

static void
unimplemented()
{
	lib::logger::get_logger()->fatal("programmer error: attempt to use unimplemented feature of DOM node");
}

///////////////////////////////////////////////
// wkdom_node implementation

//////////////////////
// Node destructor

wkdom_node::~wkdom_node() {
	[m_self release];
}

///////////////////////////////
// basic navigation

const wkdom_node *
wkdom_node::down() const
{
	return id2node([m_self firstChild], m_context);
}

const wkdom_node *
wkdom_node::up() const
{
	return id2node([m_self parentNode], m_context);
}

const wkdom_node *
wkdom_node::next() const
{
	return id2node([m_self nextSibling], m_context);
}

wkdom_node *
wkdom_node::down()
{
	return id2node([m_self firstChild], m_context);
}

wkdom_node *
wkdom_node::up()
{
	return id2node([m_self parentNode], m_context);
}

wkdom_node *
wkdom_node::next()
{
	return id2node([m_self nextSibling], m_context);
}

//////////////////////
// set down/up/next

void
wkdom_node::down(wkdom_node *n)
{
	DOMNode *firstchild = [m_self firstChild];
	if (firstchild) {
		[m_self replaceChild: firstchild : n->m_self];
	} else {
		[m_self appendChild: n->m_self];
	}
}

void
wkdom_node::up(wkdom_node *n)
{
	assert(0);
}

void
wkdom_node::next(wkdom_node *n)
{
	DOMNode *parent = [m_self parentNode];
	assert(parent);
	DOMNode *next = [m_self nextSibling];
	if (next) {
		[parent insertBefore: n->m_self : next];
	} else {
		[parent appendChild: n->m_self];
	}
}

///////////////////////////////
// deduced navigation

const wkdom_node*
wkdom_node::previous() const {
	return id2node([m_self previousSibling], m_context);
}

const wkdom_node*
wkdom_node::get_last_child() const {
	return id2node([m_self lastChild], m_context);
}

void wkdom_node::get_children(std::list<const lib::node*>& l) const {
	lib::node_navigator<const lib::node>::get_children(this, l);
}

///////////////////////////////
// search operations

wkdom_node*
wkdom_node::get_first_child(const char *name) {
	wkdom_node *e = down();
	if(!e) return 0;
	if(e->get_local_name() == name) return e;
	e = e->next();
	while(e != 0) {
		if(e->get_local_name() == name)
			return e;
		e = e->next();
	}
	return 0;
}

const wkdom_node*
wkdom_node::get_first_child(const char *name) const {
	const wkdom_node *e = down();
	if(!e) return 0;
	if(e->get_local_name() == name) return e;
	e = e->next();
	while(e != 0) {
		if(e->get_local_name() == name)
			return e;
		e = e->next();
	}
	return 0;
}

wkdom_node*
wkdom_node::locate_node(const char *path) {
	if(!path || !path[0]) {
		return this;
	}
	lib::tokens_vector v(path, "/");
	wkdom_node *n = 0;
	lib::tokens_vector::iterator it = v.begin();
	if(path[0] == '/') { // or v.at(0) is empty
		// absolute
		it++; // skip empty
		n = get_root();
		if(it == v.end())
			return n;
		if(n->get_local_name() != (*it))
			return 0;
		it++; // skip root
	} else n = this;
	for(; it != v.end() && n != 0;it++) {
		n = n->get_first_child((*it).c_str());
	}
	return n;
}

wkdom_node*
wkdom_node::get_root() {
	return lib::node_navigator<wkdom_node>::get_root(this);
}

inline std::string int2string(int number)
{
	std::stringstream ss;	//create a stringstream
	ss << number;		//add number to the stream
	return ss.str();	//return a string with the contents of the stream
}

inline std::string get_path_desc_comp(const wkdom_node *n) {
	std::string my_id = n->get_local_name();
	std::string sbuf;
	sbuf += my_id;
	const wkdom_node* parent = n->up();

	if (parent != NULL) {
		std::list<const lib::node*> children;
		parent->get_children(children);
		std::list<const lib::node*>::iterator it = children.begin();

		int count = 0;
		while (it != children.end()) {
			if ((*it)->get_local_name() == my_id) {
				count++;
				if ((*it) == n) {
					if (count > 1) {
						sbuf += "["+ int2string(count) + "]";
					}
					break;
				}
			}
			it++;
		}
	}
	return sbuf;
}

std::string wkdom_node::get_xpath() const {
	std::string sbuf;
	std::list<const wkdom_node*> path;
	lib::node_navigator<const wkdom_node>::get_path(this, path);
	std::list<const wkdom_node*>::reverse_iterator it = path.rbegin();
	sbuf += get_path_desc_comp(this);
	it++;
	for(;it != path.rend();it++) {
		std::string ln = (*it)->get_local_name();
		sbuf.insert(0, "/");
		sbuf.insert(0, get_path_desc_comp(*it));
	}
	return sbuf;
}

///////////////////////////////
// iterators
// inline

///////////////////////
// build tree functions

wkdom_node*
wkdom_node::append_child(wkdom_node* child) {
	return id2node([m_self appendChild: child->m_self], m_context);
}

wkdom_node*
wkdom_node::append_child(const char *name) {
	// Create a new node with the given tag and append
	DOMDocument *doc = [m_self ownerDocument];
	assert(doc);
	NSString *tag = [NSString stringWithCString: name];
	DOMElement *w = [doc createElement: tag];
	append_child(id2node(w, m_context));
}

wkdom_node*
wkdom_node::detach() {
	return lib::node_navigator<wkdom_node>::detach(this);
}

void wkdom_node::append_data(const char *data, size_t len) {
	// Append to the data of this node
	readonly();
}

void wkdom_node::append_data(const char *c_str) {
	NSString *data = [m_self nodeValue];
	NSString *newdata = [NSString stringWithCString: c_str];
	data = [data stringByAppendingString: newdata];
	[m_self setNodeValue: data];

}

void wkdom_node::append_data(const lib::xml_string& str) {
	// Append to the data of this node
	append_data(str.c_str());
}

void wkdom_node::set_attribute(const char *name, const char *value) {
	NSString *nsattrname = [NSString stringWithCString: name];
	NSString *nsattrvalue = [NSString stringWithCString: value];
	assert([m_self nodeType] == DOM_ELEMENT_NODE);
	[(DOMElement *)m_self setAttribute: nsattrname : nsattrvalue];
	AM_DBG NSLog(@"wkdom_node: <%@>.set_attribute(%@, %@)", [(DOMElement *)m_self tagName], nsattrname, nsattrvalue);
}

void wkdom_node::set_attribute(const char *name, const lib::xml_string& value) {
	NSString *nsattrname = [NSString stringWithCString: name];
	NSString *nsattrvalue = [NSString stringWithCString: value.c_str()];
	assert([m_self nodeType] == DOM_ELEMENT_NODE);
	[(DOMElement *)m_self setAttribute: nsattrname : nsattrvalue];
	AM_DBG NSLog(@"wkdom_node: <%@>.set_attribute(%@, %@)", [(DOMElement *)m_self tagName], nsattrname, nsattrvalue);
}

// Note: attrs are as per expat parser
// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
void wkdom_node::set_attributes(const char **attrs) {
	if(attrs == 0) return;
	for(int i=0;attrs[i];i+=2)
		set_attribute(attrs[i], attrs[i+1]);
}

void
wkdom_node::set_prefix_mapping(const std::string& prefix, const std::string& uri)
{
}

// create a deep copy of this
wkdom_node*
wkdom_node::clone() const {
	return id2node([m_self cloneNode: true], m_context);
}

////////////////////////
// data queries
/// Return the namespace part of the tag for this node.
const lib::xml_string&
wkdom_node::get_namespace() const
{
	unimplemented();
	return "";
}

/// Return the local part of the tag for this node.
const lib::xml_string&
wkdom_node::get_local_name() const
{
	static lib::xml_string rv; // XXXX bad static!
	AM_DBG lib::logger::get_logger()->debug("get_local_name: 0x%x: 0x%x", this, m_self);
	if([m_self nodeType] == DOM_ELEMENT_NODE) {
		assert(m_self);
		NSString *nstag = [(DOMElement *)m_self tagName];
		assert(nstag);
		rv = lib::xml_string([nstag cStringUsingEncoding: NSUTF8StringEncoding]);
	} else {
		rv = lib::xml_string("");
	}
	return rv;
}

/// Return namespace and local part of the tag for this node.
const lib::q_name_pair&
wkdom_node::get_qname() const
{
	return lib::q_name_pair(get_namespace(),get_local_name());
}

/// Return the unique numeric ID for this node.
int
wkdom_node::get_numid() const
{
	return (int)((size_t)m_self & 0x7fffffff);
}

/// Return the data for this node.
const lib::xml_string&
wkdom_node::get_data() const
{
	static lib::xml_string rv; // XXXX bad static!
	NSString *nstag = [m_self nodeValue];
	rv = lib::xml_string([nstag cStringUsingEncoding: NSUTF8StringEncoding]);
	return rv;
}
bool
wkdom_node::is_data_node() const
{
	return [m_self nodeType] == DOM_TEXT_NODE;
}

lib::xml_string
wkdom_node::get_trimmed_data() const {
	return lib::trim(get_data());
}

const char *
wkdom_node::get_attribute(const char *name) const {
	AM_DBG lib::logger::get_logger()->debug("get_attribute: 0x%x: 0x%x", this, m_self);
	if ([m_self nodeType] != DOM_ELEMENT_NODE)
		return NULL;
	NSString *attrname = [NSString stringWithCString: name];
	if (![(DOMElement *)m_self hasAttribute: attrname]) {
		AM_DBG NSLog(@"wkdom_node: <%@>.get_attribute(%@) -> NULL", [(DOMElement *)m_self tagName], attrname);
		return NULL;
	}
	NSString *attrvalue = [(DOMElement *)m_self getAttribute: attrname];
	if (attrvalue == NULL) return NULL;
	AM_DBG NSLog(@"wkdom_node: <%@>.get_attribute(%@) -> %@", [(DOMElement *)m_self tagName], attrname, attrvalue);
	return [attrvalue cStringUsingEncoding: NSUTF8StringEncoding];
}

void
wkdom_node::del_attribute(const char *name)
{
	NSString *attrname = [NSString stringWithCString: name];
	assert([m_self nodeType] == DOM_ELEMENT_NODE);
	[(DOMElement *)m_self removeAttribute: attrname];
}

const char *
wkdom_node::get_attribute(const std::string& name) const {
	return get_attribute(name.c_str());
}

// returns the resolved url of an attribute
net::url
wkdom_node::get_url(const char *attrname) const {
	const char *rurl = get_attribute(attrname);
	if(!rurl) return net::url();
	net::url url = net::url::from_url(rurl);
	return m_context ? m_context->resolve_url(url) : url;
	return url;
}

const char *
wkdom_node::get_container_attribute(const char *name) const {
	if(!name || !name[0]) return 0;
	const wkdom_node *n = this;
	const char *p = 0;
	while(n->up()) {
		n = n->up();
		p = n->get_attribute(name);
		if(p) break;
	}
	return p;

}

/////////////////////
// string repr

lib::xml_string
wkdom_node::xmlrepr() const {
	// Return a representation of the node, without < and >
	unimplemented();
	return "";
}

std::string wkdom_node::get_sig() const {
	std::string s = "<";
	s += get_local_name();
	const char *pid = get_attribute("id");
	if (pid) {
		s += " id=\"";
		s += pid;
		s += "\"";
	}
	s += ">";
	return s;
}

unsigned int
wkdom_node::size() const {
	const_iterator it;
	const_iterator e = end();
	unsigned int count = 0;
	for(it = begin(); it != e; it++)
		if((*it).first) count++;
	return count;
}

/////////////////////
// node context
// these must be implemented

/// Return the node_context for this node.
const lib::node_context*
wkdom_node::get_context() const
{
	return m_context;
}

/// Set the node_context for this node.
void wkdom_node::set_context(lib::node_context *c)
{
	unimplemented();
}

/////////////////////
// dynamic_cast helper methods
// only needed for read-write DOM access, otherwise not

void
wkdom_node::down(lib::node_interface *n)
{
	down(dynamic_cast<wkdom_node*>(n));
}

void
wkdom_node::up(lib::node_interface *n)
{
	up(dynamic_cast<wkdom_node*>(n));
}

void
wkdom_node::next(lib::node_interface *n)
{
	next(dynamic_cast<wkdom_node*>(n));
}

lib::node_interface*
wkdom_node::append_child(lib::node_interface* child)
{
	return append_child(dynamic_cast<wkdom_node*>(child));
}
