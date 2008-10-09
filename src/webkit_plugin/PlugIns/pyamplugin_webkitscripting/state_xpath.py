import ambulant
import Foundation
import traceback
import proxy
import bjshare
import os

NS_XFORMS="http://www.w3.org/2002/xforms"
NS_XPATH="http://www.w3.org/TR/1999/REC-xpath-19991116"

REX_TEMPLATE="""
<rex xmlns='http://www.w3.org/ns/rex#'>
    <event target='%(target)s' name='%(event)s' %(attrs)s>%(elements)s</event>
</rex>
"""

SHARE_URL="bjshare:///1234567890JACKTESTSHARE"

class MyStateComponentFactory(ambulant.state_component_factory):
    def __init__(self, domdocument):
        self.domdocument = domdocument
        
    def new_state_component(self, uri):
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'new_state_component, uri=', uri
        if uri == NS_XPATH:
            real_component = MyStateComponent(self.domdocument)
            class MTProxyStateComponent(proxy.MTHookProxy):
                pass
            proxy_component = MTProxyStateComponent('proxy_component', real_component)
            # test: proxy_component = real_component
            return proxy_component
        return None
        
class MyFormFacesStateComponentFactory(ambulant.state_component_factory):
    def __init__(self, domdocument, scriptobject):
        self.domdocument = domdocument
        self.scriptobject = scriptobject
        
    def new_state_component(self, uri):
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'new_state_component, uri=', uri
        if uri == NS_XPATH:
            real_component = MyFormFacesStateComponent(self.domdocument, self.scriptobject)
            class MTProxyStateComponent(proxy.MTHookProxy):
                pass
            proxy_component = MTProxyStateComponent('proxy_component', real_component)
            # test: proxy_component = real_component
            return proxy_component
        return None

class MyDomTreeModifier(object):
    def __init__(self, domdocument):
        self.domdocument = domdocument
         
    def replaceTextChild(self, node, value):
        valuenode = node.firstChild()
        if valuenode:
            valuenode.setNodeValue_(value)
        else:
            valuenode = self.domdocument.createTextNode_(value)
            node.appendChild_(valuenode)
            
    def createTextChild(self, parentnode, where, name, value):
        if where and where != 'child':
            print 'webkitplugin: newvalue: only child supported'
        newnode = self.domdocument.createElement_(name)
        if value:
            valuenode = self.domdocument.createTextNode_(value)
            newnode.appendChild_(valuenode)
        parentnode.appendChild_(newnode)
        
    def removeNode(self, node):
        parent = node.parentElement()
        parentElement.removeChild_(node)
        
class MyDomTreeRexModifier(object):
    def __init__(self, domdocument):
        self.domdocument = domdocument
        self.sync = bjshare.start_sync(SHARE_URL)
                
    def replaceTextChild(self, node, value):
        self.sync.transaction()
        valuenode = node.firstChild()
        if valuenode:
            valuenode.setNodeValue_(value)
        else:
            valuenode = self.domdocument.createTextNode_(value)
            node.appendChild_(valuenode)
        node_path = self.get_xpath(node)
        update = self.gen_rex(node_path, 'DOMCharacterDataModified', {'newValue':value}, None)
        self.sync.commit(update)
            
    def createTextChild(self, parentnode, where, name, value):
        self.sync.transaction()
        if where and where != 'child':
            print 'webkitplugin: newvalue: only child supported'
        newnode = self.domdocument.createElement_(name)
        if value:
            valuenode = self.domdocument.createTextNode_(value)
            newnode.appendChild_(valuenode)
        parentnode.appendChild_(newnode)
        node_path = self.get_xpath(parentnode)
        # XXX Need to do escapes.
        fragment = '<%s xmlns=''>%s</%s>' % (name, value, name)
        update = self.gen_rex(node_path, 'DOMNodeInserted', {}, fragment)
        self.sync.commit(update)
        
    def removeNode(self, node):
        self.sync.transaction()
        parent = node.parentElement()
        node_path = self.get_xpath(node)
        parentElement.removeChild_(node)
        update = self.gen_rex(node_path, 'DOMNodeRemoved', {}, None)
        self.sync.commit(update)
        
    def get_xpath(self, node):
        rest = ''
        while 1:
            element = node.nodeName()
            element_name = node.nodeName()
            parent = node.parentNode()
            if parent:
                sibling = node.previousSibling()
                count = 0
                while sibling:
                    if sibling.nodeName() == element_name:
                        count += 1
                    sibling = sibling.previousSibling()
                if count:
                    element_name += '[%d]' % count
                # Hack: the topmost parent is the #document, and we don't want that one.
                rest = '/' + element_name + rest
            node = parent
            if not node: break
        return rest
        
    def gen_rex(self, target, event, attrdict, elements):
        if not elements:
            elements = ''
        attrs = ' '.join(["%s='%s'" % (k, v) for k, v in attrdict.items()])
        rv = REX_TEMPLATE % locals()
        return rv
        
class MyStateComponent(ambulant.state_component, MyDomTreeRexModifier):
    def __init__(self, domdocument):
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'MyStateComponent()'
        self.globscope = {}
        self.domdocument = domdocument
        MyDomTreeRexModifier.__init__(self, domdocument)
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'DOMDocument is', self.domdocument
        self.statecontainer_id = None
        self.statenode = None
        self.nsresolver = None
        # What do we want to export to scope???
        
    def register_state_test_methods(self, stm):
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'register_state_test_methods, stm=', stm
        # Export things to the scripts
        for name in dir(stm):
            if name[:5] == 'smil_':
                self.globscope[name] = getattr(stm, name)
        
    def get_state_container(self, node_id):
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "state id is", src[1:]
        statecontainer = self.domdocument.getElementById_(src[1:])
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "state container node is", statecontainer
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): self.dump_dom_node(statecontainer)
        return statecontainer
        
    def _recalculate(self, node):
        pass
        
    def declare_state(self, state):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'declare_state, node=', state
        src = state.get_attribute_1("src")
        if not src:
            print "webkitplugin: only state with src attribute allowed"
            return
        if src[0] != "#":
            print "webkitplugin: only #id allowed for src attribute on state"
            return
        self.statecontainer_id = src[1:]
        statecontainer = self.get_state_container(self.statecontainer_id)
        ch = statecontainer.firstChild()
        while ch and ch.nodeType() != 1:
            if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'webkitpluginstate: Skip', ch
            ch = ch.nextSibling()
        if not ch:
            print "webkitplugin: state container",src,"is empty"
            return
        self.statenode = ch
        ch = ch.nextSibling()
        while ch:
            if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'webkitpluginstate: examine', ch
            if ch.nodeType() == 1:
                print "webkitplugin: state container", src, "has more than one child"
                self.statenode = None
                return
            ch = ch.nextSibling()
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "state node is", self.statenode
        
    def bool_expression(self, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'bool_expression, expr=', expr
        strexpr = self.string_expression(expr)
        if not strexpr:
            if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'bool_expression: empty string -> False'
            return False
        try:
            number = int(strexpr)
        except:
            pass
        else:
            if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'bool_expression: number ->', not not number
            return not not number
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'bool_expression: nonempty string -> True'
        return True
    
    def _find_node(self, ref):
        if not self.nsresolver:
            self.nsresolver = self.domdocument.createNSResolver_(self.statenode)
        rv = self.domdocument.evaluate_____(ref, self.statenode, self.nsresolver, 0, None)
        if rv == None or rv.resultType() != 4:
            print 'webkitplugin: ref="%s": did not return a nodeset' % ref
            return None
        node = rv.iterateNext()
        if not node:
            print 'webkitplugin: ref="%s": empty nodeset' % ref
            return None
        if rv.iterateNext():
            print 'webkitplugin: ref="%s": more than one item in nodeset' % ref
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print '_find_node("%s")->%s' % (ref, node)
        return node
        
    def set_value(self, var, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'set_value', (var, expr)
        # XXXX There is no reason value has to be string. Could be subtree, etc.
        value = self.string_expression(expr)
        node = self._find_node(var)
        if not node:
            return
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'setvalue: NODE', node
        self.replaceTextChild(node, value)
        self._recalculate(node)
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'set_value: node is now', self.string_expression(var)
        
    def new_value(self, ref, where, name, expr):
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'newvalue', (ref, where, name, expr)
        parentnode = self._find_node(ref)
        if not parentnode:
            return
        if expr:
            value = self.string_expression(expr)
        else:
            value = None
        self.createTextChild(parentnode, where, name, value)
        self._recalculate(parentnode)
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'newvalue: all done'
        x = self.string_expression(ref+'/'+name)
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'newvalue:', ref+'/'+name, 'is now', x
        
    def del_value(self, ref):
        node = self._find_node(ref)
        if not node: return
        self.removeNode(node)
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'XXX delete', node
        self._recalculate(parentElement)
        
    def send(self, submission):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'webkitplugin: send: not implemented. submission=', submission
        
    def string_expression(self, expr):
        #pool = Foundation.NSAutoreleasePool.alloc().init()
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'string_expression, expr=', expr
        if not self.nsresolver:
            self.nsresolver = self.domdocument.createNSResolver_(self.statenode)
        rv = self.domdocument.evaluate_____(expr, self.statenode, self.nsresolver, 0, None)
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'string_expression returned', rv
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'string_expression resultType', rv.resultType()
##       import pdb ; pdb.set_trace()
        if rv.resultType() == 1:
            return str(rv.numberValue())
        if rv.resultType() == 2:
            return rv.stringValue()
        if rv.resultType() == 3:
            return str(rv.booleanValue())
        if rv.resultType() == 4: # node iterators
            node = rv.iterateNext()
            if not node:
                print 'webkitplugin: string_expression: does not match a node:', expr
                return ''
            if rv.iterateNext():
                print 'webkitplugin: string_expression: matches multiple nodes:', expr
            valuenode = node.firstChild()
            if not valuenode:
                return ''
            value = valuenode.nodeValue()
            return value
        print 'webkitplugin: string_expression: XPath returned unknown type, resultType=', rv.resultType()
        
    def dump_dom_node(self, domnode, indent=0):
        if not domnode: return
        print ' '*indent, domnode
        ch = domnode.firstChild()
        self.dump_dom_node(ch, indent+2)
        if indent > 0:
            self.dump_dom_node(domnode.nextSibling(), indent)
        
class MyFormFacesStateComponent(MyStateComponent):
    def __init__(self, domdocument, scriptengine):
        self.scriptengine = scriptengine
        MyStateComponent.__init__(self, domdocument)
         
    def get_state_container(self, node_id):
        # First: a sanity check that this is indeed FormFaces
        xform = self.scriptengine.evaluateWebScript_("xform")
        if xform == None:
            print 'webkitplugin: no "xform" variable, is this really FormFaces?'
            return

        # Second: communicate state instance to javascript glue
        tmp = self.scriptengine.evaluateWebScript_("smil_state_glue_initialize")
        if tmp:
            self.scriptengine.evaluateWebScript_("smil_state_glue_initialize('%s')" % node_id)
            
        stateinstance = self.scriptengine.evaluateWebScript_("xform.getObjectById('%s', XFormInstance).document" % node_id )
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'stateinstance=', stateinstance
        return stateinstance

    def _recalculate(self, node):
        self.scriptengine.evaluateWebScript_("XmlEvent.dispatch(xform.getObjectById('%s', XFormInstance).model.htmlNode, 'xforms-rebuild')" % self.statecontainer_id)
        self.scriptengine.evaluateWebScript_("XmlEvent.dispatch(xform.getObjectById('%s', XFormInstance).model.htmlNode, 'xforms-recalculate')" % self.statecontainer_id)
        self.scriptengine.evaluateWebScript_("XmlEvent.dispatch(xform.getObjectById('%s', XFormInstance).model.htmlNode, 'xforms-revalidate')" % self.statecontainer_id)
        self.scriptengine.evaluateWebScript_("XmlEvent.dispatch(xform.getObjectById('%s', XFormInstance).model.htmlNode, 'xforms-refresh')" % self.statecontainer_id)
        