#ifndef AMBULANT_PLUGIN_WKDOMBRIDGE_H
#define AMBULANT_PLUGIN_WKDOMBRIDGE_H

#include "ambulant/lib/logger.h"
#include "ambulant/lib/node.h"
#include "wkdomplugin.h"
#include <WebKit/WebKit.h>

using namespace ambulant;

class wkdom_node_factory : public lib::node_factory {
  public:
	wkdom_node_factory(DOMImplementation *domimp);

	lib::node *new_node(const char *local_name, const char **attrs = 0, const lib::node_context *ctx = 0);

	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	lib::node *new_node(const lib::xml_string& local_name, const char **attrs = 0, const lib::node_context *ctx = 0);

	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	lib::node *new_node(const lib::q_name_pair& qn, const lib::q_attributes_list& qattrs, const lib::node_context *ctx = 0);

	// shallow copy from other.
	lib::node *new_node(const lib::node* other);

	lib::node *new_data_node(const char *data, size_t size, const lib::node_context *ctx);

};

class wkdom_node : public lib::node_interface {
	friend class wkdom_node_factory;
  private:

  private:
	wkdom_node(DOMElement *w, const lib::node_context *ctx);
	DOMNode *m_self;
	const lib::node_context *m_context;
	static std::map<const DOMNode *, wkdom_node *> s_id2node; // XXXX Temp static
	static std::map<const lib::node_context *, DOMDocument *> s_ctx2doc; // XXXX Temp static
	static DOMImplementation *s_implementation;
	static wkdom_node *id2node(const DOMNode *w, const lib::node_context *);
	static DOMDocument *ctx2doc(const lib::node_context *ctx);

  public:

	///////////////////////////////
	// tree iterators
	typedef lib::tree_iterator<wkdom_node> iterator;
	typedef lib::const_tree_iterator<wkdom_node> const_iterator;

	/// Destruct this node and its contents.
	/// If this node is part of a tree, detach it first
	/// and then delete the node and its contents.
	virtual ~wkdom_node();

	/// Return first child of this node.
	const wkdom_node *down() const;

	/// Return parent of this node.
	const wkdom_node *up() const;

	/// Return next sibling of this node.
	const wkdom_node *next() const;

	/// Return first child of this node.
	wkdom_node *down();

	/// Return parent of this node.
	wkdom_node *up();

	/// Return next sibling of this node.
	wkdom_node *next();

	/// Set first child of this node.
	void down(wkdom_node *n);

	/// Set first child of this node, after dynamic typecheck
	void down(lib::node_interface *n);

	/// Set parent of this node.
	void up(wkdom_node *n);

	/// Set parent of this node, after dynamic typecheck
	void up(lib::node_interface *n);

	/// Set next sibling of this node.
	void next(wkdom_node *n);

	/// Set next sibling of this node, after dynamic typecheck
	void next(lib::node_interface *n);

	/// Returns the previous sibling node
	/// or null when this is the first child.
	const wkdom_node* previous() const;

	/// Returns the last child
	/// or null when this has not any children.
	const wkdom_node* get_last_child() const;

	/// Appends the children of this node (if any) to the provided list.
	void get_children(std::list<const lib::node*>& l) const;

	///////////////////////////////
	// search operations
	// this section should be extented to allow for XPath selectors

	/// Find a node given a path of the form tag/tag/tag.
	wkdom_node* locate_node(const char *path);

	/// Find the first direct child with the given tag.
	wkdom_node *get_first_child(const char *name);

	/// Find the first direct child with the given tag.
	const wkdom_node *get_first_child(const char *name) const;

	/// Find the root of the tree to which this node belongs.
	wkdom_node* get_root();

	/// Get an attribute from this node or its nearest ancestor that has the attribute.
	const char *get_container_attribute(const char *name) const;
	///////////////////////////////
	// iterators

	/// Return iterator for this node and its subtree.
	iterator begin() { return iterator(this);}

	/// Return iterator for this node and its subtree.
	const_iterator begin() const { return const_iterator(this);}

	iterator end() { return iterator(0);}
	const_iterator end() const { return const_iterator(0);}

	///////////////////////
	// build tree functions

	/// Append a child node to this node.
	wkdom_node* append_child(wkdom_node* child);

	/// Append a child node to this node, after dynamic typecheck
	lib::node_interface* append_child(lib::node_interface* child);

	/// Append a new child node with the given name to this node.
	wkdom_node* append_child(const char *name);

	/// Detach this node and its subtree from its parent tree.
	wkdom_node* detach();

	/// Create a deep copy of this node and its subtree.
	wkdom_node* clone() const;

	/// Append data to the data of this node.
	void append_data(const char *data, size_t len);

	/// Append c_str to the data of this node.
	void append_data(const char *c_str);

	/// Append str to the data of this node.
	void append_data(const lib::xml_string& str);

	/// Add an attribute/value pair.
	void set_attribute(const char *name, const char *value);

	/// Add an attribute/value pair.
	void set_attribute(const char *name, const lib::xml_string& value);

	/// Set a number of attribute/value pairs.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	void set_attributes(const char **attrs);

	/// Override prefix mapping for this node and descendents
	void set_prefix_mapping(const std::string& prefix, const std::string& uri);
	/////////////////////
	// data queries

	/// Return the namespace part of the tag for this node.
	const lib::xml_string& get_namespace() const;

	/// Return the local part of the tag for this node.
	const lib::xml_string& get_local_name() const;

	/// Return namespace and local part of the tag for this node.
	const lib::q_name_pair& get_qname() const;

	/// Return the unique numeric ID for this node.
	int get_numid() const;

	/// Return the data for this node.
	const lib::xml_string& get_data() const;

	///
	bool is_data_node() const;

	/// Return the trimmed data for this node.
	lib::xml_string get_trimmed_data() const;

	/// Return the value for the given attribute.
	const char *get_attribute(const char *name) const;

	/// Return the value for the given attribute.
	const char *get_attribute(const std::string& name) const;

	/// Remove the first occurrence of the given attribute.
	void del_attribute(const char *name);

	/// Return the value for the given attribute, interpreted as a URL.
	/// Relative URLs are resolved against the document base URL, if possible.
	net::url get_url(const char *attrname) const;

	/// Return the number of nodes of the xml (sub-)tree starting at this node.
	unsigned int size() const;

	/// Returns XPath locator for this node.
	std::string get_xpath() const;

	/// Return a friendly string describing this node.
	/// The string will be of a form similar to \<tag id="...">
	std::string get_sig() const;

	/////////////////////
	// string repr

	/// Return the
	lib::xml_string xmlrepr() const;

	/////////////////////
	// node context

	/// Return the node_context for this node.
	const lib::node_context* get_context() const;

	/// Set the node_context for this node.
	void set_context(lib::node_context *c);

	/// Return the next unique ID.
	static int get_node_counter();
};
#endif // AMBULANT_PLUGIN_WKDOMBRIDGE_H