import ambulant

class MyScriptComponentFactory(ambulant.script_component_factory):
    def __init__(self):
        pass
        
    def new_script_component(self, uri):
        print 'new_script_component, uri=', uri
        if uri == "http://www.ambulantplayer.org/components/pyscript":
            return MyScriptComponent()
        return None
        
class MyScriptComponent(ambulant.script_component):
    def __init__(self):
        print 'MyScriptComponent()'
        self.globscope = {}
        self.scope = {}
        # What dowe want to export to scope???
        
    def register_state_test_methods(self, stm):
        print 'register_state_test_methods, stm=', stm
        # Export things to the scripts
        for name in dir(stm):
            if name[:5] == 'smil_':
                self.globscope[name] = getattr(stm, name)
        
    def declare_state(self, state):
        print 'declare_state, node=', state
        statements = state.get_trimmed_data() + "\n"
        child = state.down_1()
        while child:
            statements += child.get_trimmed_data() + "\n"
            child = child.next_1()
        exec statements in self.scope, self.globscope
        #import pdb
        #pdb.set_trace()
        
    def bool_expression(self, expr):
        print 'bool_expression, expr=', expr
        rv = eval(expr, self.scope, self.globscope)
        print 'bool_expression returning', rv
        rv = not not rv
        print 'bool_expression returning casted', rv
        return rv
        
    def set_value(self, var, expr):
        stmt = "%s = %s" % (var, expr)
        print 'set_value, statement=', stmt
        exec stmt in self.scope, self.globscope
        
    def send(self, submission):
        print 'send, submission=', submission
        
    def string_expression(self, expr):
        print 'string_expression, expr=', expr
        rv = eval(expr, self.scope, self.globscope)
        print 'string_expression returning', rv
        rv = str(rv)
        print 'string_expression returning casted', rv
        return rv
        