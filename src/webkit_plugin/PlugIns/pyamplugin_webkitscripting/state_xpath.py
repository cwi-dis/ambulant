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
        self.rv = func(*args, **kwargs)

def callWait(func, *args, **kwargs):
    """call a function on the main thread (sync)"""
    pool = Foundation.NSAutoreleasePool.alloc().init()
    obj = MainThreadCaller.alloc().initWithArgs_((func, args, kwargs))
    return obj.callWait_(None)

class MyStateComponent(ambulant.state_component):
    def __init__(self, domdocument):
        print 'MyStateComponent()'
        self.globscope = {}
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
        return callWait(self._set_value, var, expr)
    
    def _set_value(self, var, expr):
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
        #pool = Foundation.NSAutoreleasePool.alloc().init()
        print 'string_expression, expr=', expr
        rv = self.domdocument.evaluate_____(expr, self.statenode, None, 0, None)
        import pdb ; pdb.set_trace()
        print 'string_expression returned', rv
        if rv.resultType() <= 3: # any, number, string, boolean
            return rv.stringValue()
        if rv.resultType() <= 5: # node iterators
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
        
