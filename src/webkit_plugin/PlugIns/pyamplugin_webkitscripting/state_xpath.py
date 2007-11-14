import ambulant
import Foundation
import traceback

DEBUG=True

NS_XFORMS="http://www.w3.org/2002/xforms"
NS_XPATH="http://www.w3.org/TR/1999/REC-xpath-19991116"

class MyStateComponentFactory(ambulant.state_component_factory):
    def __init__(self, domdocument):
        self.domdocument = domdocument
        
    def new_state_component(self, uri):
        print 'new_state_component, uri=', uri
        if uri == NS_XPATH:
            return MyStateComponent(self.domdocument)
        return None
        
class MyFormFacesStateComponentFactory(ambulant.state_component_factory):
    def __init__(self, domdocument, scriptobject):
        self.domdocument = domdocument
        self.scriptobject = scriptobject
        
    def new_state_component(self, uri):
        print 'new_state_component, uri=', uri
        if uri == NS_XPATH:
            return MyFormFacesStateComponent(self.domdocument, self.scriptobject)
        return None

class MainThreadCaller(Foundation.NSObject):
    
    def initWithArgs_(self, args):
        self = self.init()
        self.args = args
        self.rv = None
        return self

    def callWait_(self, sender):
        self.performSelectorOnMainThread_withObject_waitUntilDone_(
            self.call_, self.args, True)
        return self.rv
    
    def call_(self, (func, args, kwargs)):
        try:
            self.rv = func(*args, **kwargs)
        except:
            print 'webkitpluginstate: HELP! Exception!'
            traceback.print_exc()
 
def callWait(func, *args, **kwargs):
    """call a function on the main thread (sync)"""
    pool = Foundation.NSAutoreleasePool.alloc().init()
    obj = MainThreadCaller.alloc().initWithArgs_((func, args, kwargs))
    return obj.callWait_(None)

class MyStateComponent(ambulant.state_component):
    def __init__(self, domdocument):
        if DEBUG: print 'MyStateComponent()'
        self.globscope = {}
        self.domdocument = domdocument
        if DEBUG: print 'DOMDocument is', self.domdocument
        self.statenode = None
        self.nsresolver = None
        # What do we want to export to scope???
        
    def register_state_test_methods(self, stm):
        if DEBUG: print 'register_state_test_methods, stm=', stm
        # Export things to the scripts
        for name in dir(stm):
            if name[:5] == 'smil_':
                self.globscope[name] = getattr(stm, name)
        
    def get_state_container(self, node_id):
        if DEBUG: print "state id is", src[1:]
        statecontainer = self.domdocument.getElementById_(src[1:])
        if DEBUG: print "state container node is", statecontainer
        if DEBUG: self._dump(statecontainer)
        return statecontainer
        
    def _recalculate(self, node):
        pass
        
    def declare_state(self, state):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'declare_state, node=', state
        src = state.get_attribute_1("src")
        if not src:
            print "webkitpluginstate: only state with src attribute allowed"
            return
        if src[0] != "#":
            print "webkitpluginstate: only #id allowed for src attribute on state"
            return
        statecontainer = self.get_state_container(src[1:])
        ch = statecontainer.firstChild()
        while ch and ch.nodeType() != 1:
            if DEBUG: print 'webkitpluginstate: Skip', ch
            ch = ch.nextSibling()
        if not ch:
            print "webkitpluginstate: state container",src,"is empty"
            return
        self.statenode = ch
        ch = ch.nextSibling()
        while ch:
            print 'webkitpluginstate: examine', ch
            if ch.nodeType() == 1:
                print "webkitpluginstate: state container", src, "has more than one child"
                self.statenode = None
                return
            ch = ch.nextSibling()
        if DEBUG: print "state node is", self.statenode
        
    def bool_expression(self, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'bool_expression, expr=', expr
        strexpr = self.string_expression(expr)
        if not strexpr:
            if DEBUG: print 'bool_expression: empty string -> False'
            return False
        try:
            number = int(strexpr)
        except:
            pass
        else:
            if DEBUG: print 'bool_expression: number ->', not not number
            return not not number
        if DEBUG: print 'bool_expression: nonempty string -> True'
        return True
    
    def _find_node(self, ref):
        if not self.nsresolver:
            self.nsresolver = self.domdocument.createNSResolver_(self.statenode)
        rv = self.domdocument.evaluate_____(ref, self.statenode, self.nsresolver, 0, None)
        if rv == None or rv.resultType() != 4:
            print 'webkitpluginstate: ref="%s": did not return a nodeset' % ref
            return None
        node = rv.iterateNext()
        if not node:
            print 'webkitpluginstate: ref="%s": empty nodeset' % ref
            return None
        if rv.iterateNext():
            print 'webkitpluginstate: ref="%s": more than one item in nodeset' % ref
        print '_find_node("%s")->%s' % (ref, node)
        return node
        
    def set_value(self, var, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        return callWait(self._set_value, var, expr)
    
    def _set_value(self, var, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'set_value', (var, expr)
        value = self._string_expression(expr)
        node = self._find_node(var)
        if not node:
            return
        print 'setvalue: NODE', node
##        print 'DIR()', dir(node)
##        print 'DIR(document)', dir(self.domdocument)
        valuenode = node.firstChild()
        if not valuenode:
            valuenode = self.domdocument.createTextNode_(value)
            node.appendChild_(valuenode)
        valuenode.setNodeValue_(value)
        self._recalculate(node)
        print '_set_value: node is now', self._string_expression(var)
        
    def new_value(self, ref, where, name, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        return callWait(self._new_value, ref, where, name, expr)
    
    def _new_value(self, ref, where, name, expr):
        if DEBUG: print 'newvalue', (ref, where, name, expr)
        parentnode = self._find_node(ref)
        if not parentnode:
            return
        if where and where != 'child':
            print 'XXX newvalue: only child supported'
        newnode = self.domdocument.createElement_(name)
        if expr:
            value = self._string_expression(expr)
            if value:
                valuenode = self.domdocument.createTextNode_(value)
                newnode.appendChild_(valuenode)
        parentnode.appendChild_(newnode)
        self._recalculate(parentnode)
        print 'newvalue: all done'
        x = self._string_expression(ref+'/'+name)
        print 'newvalue:', ref+'/'+name, 'is now', x
        
    def del_value(self, ref):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        return callWait(self._del_value, ref)
        
    def _del_value(self, ref):
        node = self._find_node(ref)
        if not node: return
        parent = node.parentElement()
        parentElement.removeChild_(node)
        print 'XXX delete', node
        self._recalculate(parentElement)
        
    def send(self, submission):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'NOT IMPLEMENTED: send, submission=', submission
        
    def string_expression(self, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        return callWait(self._string_expression, expr)

    def _string_expression(self, expr):
        #pool = Foundation.NSAutoreleasePool.alloc().init()
        if DEBUG: print 'string_expression, expr=', expr
        if not self.nsresolver:
            self.nsresolver = self.domdocument.createNSResolver_(self.statenode)
        rv = self.domdocument.evaluate_____(expr, self.statenode, self.nsresolver, 0, None)
        if DEBUG: print 'string_expression returned', rv
        if DEBUG: print 'string_expression resultType', rv.resultType()
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
                print 'string_expression: does not match a node:', expr
                return ''
            if rv.iterateNext():
                print 'string_expression: matches multiple nodes:', expr
            valuenode = node.firstChild()
            if not valuenode:
                return ''
            value = valuenode.nodeValue()
            return value
        print 'string_expression: XPath returned unknown type, resultType=', rv.resultType()
        
    def _dump(self, domnode, indent=0):
        if not domnode: return
        print ' '*indent, domnode
        ch = domnode.firstChild()
        self._dump(ch, indent+2)
        if indent > 0:
            self._dump(domnode.nextSibling(), indent)
        
class MyFormFacesStateComponent(MyStateComponent):
    def __init__(self, domdocument, scriptengine):
        MyStateComponent.__init__(self, domdocument)
        self.scriptengine = scriptengine
        
    def get_state_container(self, node_id):
        stateinstance = self.scriptengine.evaluateWebScript_("document.getElementById('jacksmodel').getInstanceDocument('jacksinstance')" )
        print 'stateinstance=', stateinstance
        return stateinstance
##        xform = self.scriptengine.evaluateWebScript_("xform")
##        if xform == None:
##            print 'webkitplugin: no "xform" variable, is this really FormFaces?'
##            return
##        print "xform=", xform
##        print "dir(xform)=", dir(xform)

    def _recalculate(self, node):
        ##node.revalidate()
        self.scriptengine.evaluateWebScript_("document.getElementById('jacksmodel').rebuild()")
        self.scriptengine.evaluateWebScript_("document.getElementById('jacksmodel').recalculate()")
        self.scriptengine.evaluateWebScript_("document.getElementById('jacksmodel').revalidate()")
        self.scriptengine.evaluateWebScript_("document.getElementById('jacksmodel').refresh()")
        ##import pdb ; pdb.set_trace()
        