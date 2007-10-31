import ambulant
import Foundation

class MyStateComponentFactory(ambulant.state_component_factory):
    def __init__(self, domdocument):
        self.domdocument = domdocument
        
    def new_state_component(self, uri):
        print 'new_state_component, uri=', uri
        if uri == "http://www.w3.org/TR/1999/REC-xpath-19991116":
            return MyStateComponent(self.domdocument)
        return None

class MicroXpath:
    def __init__(self, domdoc, domnode):
        self.domdoc = domdoc
        self.domnode = domnode
        
    def __getitem__(self, name):
        nodelist = self.domnode.getElementsByTagName_(name)
        if not nodelist:
            raise KeyError("No XML nodes named '%s'" % name)
        node = nodelist.item_(0)
        if not node:
            raise KeyError("No XML node named '%s'" % name)
        valuenode = node.firstChild()
        if not valuenode:
            return ''
        value = valuenode.nodeValue()
        try:
            value = int(value)
        except ValueError:
            pass
        return value
        
    def __setitem__(self, name, value):
        value = unicode(value)
        nodelist = self.domnode.getElementsByTagName_(name)
        node = nodelist.item_(0)
        if not nodelist or not node:
            # It does not exist yet. Create it.
            node = self.domdoc.createElement_(name)
            valuenode = self.domdoc.createTextNode_(value)
            node.appendChild_(valuenode)
            self.domnode.appendChild_(node)
            return
        valuenode = node.firstChild()
        if not valuenode:
            valuenode = self.domdoc.createTextNode_(value)
            node.appendChild_(valuenode)
            return
        valuenode.setNodeValue_(value)        
        
class MyStateComponent(ambulant.state_component):
    def __init__(self, domdocument):
        print 'MyStateComponent()'
        self.globscope = {}
        self.localscope = None
        self.domdocument = domdocument
        print 'DOMDocument is', self.domdocument
        print 'DOMDocument has', dir(self.domdocument)
        self.statenode = None
        # What do we want to export to scope???
        
    def register_state_test_methods(self, stm):
        print 'register_state_test_methods, stm=', stm
        # Export things to the scripts
        for name in dir(stm):
            if name[:5] == 'smil_':
                self.globscope[name] = getattr(stm, name)
        
    def declare_state(self, state):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'declare_state, node=', state
        src = state.get_attribute_1("src")
        if not src:
            print "webkitpluginstate: only state with src attribute allowed"
            return
        if src[0] != "#":
            print "webkitpluginstate: only #id allowed for src attribute on state"
            return
        print "state id is", src[1:]
        self.statenode = self.domdocument.getElementById_(src[1:])
        print "state node is", self.statenode
        self.localscope = MicroXpath(self.domdocument, self.statenode)
        
    def bool_expression(self, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'bool_expression, expr=', expr
        strexpr = self.string_expression(expr)
        if not strexpr:
            return False
        try:
            number = int(strexpr)
        except:
            pass
        else:
            return not not number
        return True
        
    def set_value(self, var, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'set_value', (var, expr)
        value = self.string_expression(expr)
        nodelist = self.statenode.getElementsByTagName_(var)
        node = nodelist.item_(0)
        if not node:
            print 'set_value: no such node:', var
            return
        valuenode = node.firstChild()
        if not valuenode:
            print 'set_value: not yet imp: node has no data yet:', node
            return
        valuenode.setNodeValue_(value)
        
    def new_value(self, ref, where, name, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'set_value, statement=', (ref, where, name, expr)
##        exec stmt in self.scope, self.globscope
        
    def del_value(self, ref):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'del_value, ref=', ref
        
    def send(self, submission):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'send, submission=', submission
        
    def string_expression(self, expr):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'string_expression, expr=', expr
        rv = eval(expr, self.globscope, self.localscope)
        print 'string_expression returning', rv
        rv = str(rv)
        print 'string_expression returning casted', rv
        return rv
        