# C++ support for bgen.
# Preliminary, 14-jun-05, Jack.Jansen@cwi.nl

from bgen import *
from scantools import Scanner
import re

class CxxMethodGenerator(FunctionGenerator):
    """C++ Method generator.
    
    Almost identical to a FunctionGenerator (not a MethodGenerator: that class
    expects that the underlying C object needs to be passed as an argument!)
    but knows that the method can be called through the object pointer.
    """
    
    def __init__(self, returntype, name, *argumentlist, **conditionlist):
        FunctionGenerator.__init__(self, returntype, name, *argumentlist, **conditionlist)
        self.callname = "_self->ob_itself->" + self.name

class CxxMixin:
    """Mixin for ObjectDefinition.
    
    The C++ compiler is more picky on use preceding declaration so
    we need a workaround for that.
    """
    
    def outputCheck(self):
        Output("extern PyTypeObject %s;", self.typename)
        Output()
        Output("inline bool %s_Check(PyObject *x)", self.prefix)
        OutLbrace()
        Output("return ((x)->ob_type == &%s || PyObject_TypeCheck((x), &%s));",
               self.typename, self.typename)
        OutRbrace()
        Output()

class CxxScanner(Scanner):
    """Pretty dumb C++ scanner.
    
    It is basically the standard Scanner, but it caters for namespace and
    class declarations. It also has rudimental understanding of default
    arguments and inline code (it tries to skip it).
    
    Methods must be declared virtual in C++ to be seen,
    functions extern. Change self.head_pat and type_pat to fix this.
    Unqualified class name is used as the Python class name, and methods are
    for class xyzzy are stored in methods_xyzzy by the generated script.
    """
    def __init__(self, input=None, output=None, defsoutput=None):
        Scanner.__init__(self, input, output, defsoutput)
        self.initnamespaces()
        #self.debug = 1
        #self.silent = 0
        
    def initnamespaces(self):
        self.namespaces = []
        self.in_class_defn = 0

    def pythonizename(self, name):        
        if '<' in name or '>' in name:
            self.error("Use of templates in typename or functionname not supported: %s", name)
        name = re.sub("\*", " ptr", name)
        name = re.sub("&", " ref", name)
        name = re.sub("::", " ", name)
        name = name.strip()
        name = re.sub("[ \t]+", "_", name)
        return name
        
    def modifyarg(self, type, name, mode):
        if type[:6] == "const_":
            type = type[6:]
            # Note that const ref and const ptr parameters stay InMode
            if type[-4:] == '_ref':
                type = type[:-4]
                mode = mode + "+RefMode"
        elif type[-4:] == '_ref':
            type = type[:-4]
            mode = "OutMode+RefMode"
        elif type in self.inherentpointertypes:
            mode = "OutMode"
        if type[-4:] == "_far":
            type = type[:-4]
        return type, name, mode

    def initpatterns(self):
        self.head_pat = r"^\s*(virtual|extern)\s+"
        self.tail_pat = r"[;={}]"
        self.type_pat = r"(virtual|extern)" + \
                        r"\s+" + \
                        r"(?P<type>[a-zA-Z0-9_*:& \t]*[a-zA-Z0-9_*&])" + \
                        r"\s+"
        self.name_pat = r"(?P<name>[a-zA-Z0-9_]+)\s*"
        self.args_pat = r"\((?P<args>([^\(;\)]+|\([^\(;\)]*\))*)\)"
        self.whole_pat = self.type_pat + self.name_pat + self.args_pat
        self.sym_pat = r"^[ \t]*(?P<name>[a-zA-Z0-9_]+)[ \t]*=" + \
                       r"[ \t]*(?P<defn>[-0-9_a-zA-Z'\"\(][^\t\n,;}]*),?"
        self.asplit_pat = r"^(?P<type>[^=]*[^a-zA-Z0-9_])(?P<name>[a-zA-Z0-9_]+)(?P<array>\[\])?(?P<initializer>\s*=[a-zA-Z0-9_ ]+)?$"
        self.comment1_pat = r"(?P<rest>.*)//.*"
        # note that the next pattern only removes comments that are wholly within one line
        self.comment2_pat = r"(?P<rest1>.*)/\*.*\*/(?P<rest2>.*)"
        self.namespace_pat = r"^\s*namespace\s+(?P<name>[a-zA-Z0-9_:]+)\s+{"
        self.klass_pat = r"^\s*class\s+(?P<name>[a-zA-Z0-9_:]+)\s+[{:]"

    def donamespace(self, match):
        if self.in_class_defn:
            self.report("Cannot do namespace inside class")
        name = match.group("name")
        self.namespaces.append(name)
        if self.debug:
            self.report("      %d: namespace %s" % (len(self.namespaces), name))
        
    def doclass(self, match):
        name = match.group("name")
        self.namespaces.append(name)
        self.in_class_defn += 1
        if self.debug:
            self.report("      %d: namespace %s" % (len(self.namespaces), name))
            
    def dobeginscope(self, count):
        for i in range(count):
            self.namespaces.append("<scope>")
            if self.in_class_defn:
                self.in_class_defn += 1
        
    def doendscope(self, count):
        for i in range(count):
            if self.in_class_defn:
                self.in_class_defn -= 1
            if self.debug:
                self.report("      %d: leaving %s" % (len(self.namespaces), self.namespaces[-1]))
            del self.namespaces[-1]
            count -= 1

    def scan(self):
        if not self.scanfile:
            self.error("No input file has been specified")
            return
        inputname = self.scanfile.name
        self.report("scanfile = %r", inputname)
        if not self.specfile:
            self.report("(No interface specifications will be written)")
        else:
            self.report("specfile = %r", self.specfile.name)
            self.specfile.write("# Generated from %r\n\n" % (inputname,))
        if not self.defsfile:
            self.report("(No symbol definitions will be written)")
        else:
            self.report("defsfile = %r", (self.defsfile.name,))
            self.defsfile.write("# Generated from %r\n\n" % (os.path.split(inputname)[1],))
            self.writeinitialdefs()
        self.alreadydone = []
        try:
            while 1:
                try: line = self.getline()
                except EOFError: break
                if self.debug:
                    self.report("LINE: %r" % (line,))
                match = self.comment1.match(line)
                if match:
                    line = match.group('rest')
                    if self.debug:
                        self.report("\tafter comment1: %r" % (line,))
                match = self.comment2.match(line)
                while match:
                    line = match.group('rest1')+match.group('rest2')
                    if self.debug:
                        self.report("\tafter comment2: %r" % (line,))
                    match = self.comment2.match(line)
                if self.defsfile:
                    match = self.sym.match(line)
                    if match:
                        if self.debug:
                            self.report("\tmatches sym.")
                        self.dosymdef(match)
                        continue
                match = self.head.match(line)
                if match:
                    if self.debug:
                        self.report("\tmatches head.")
                    line = self.dofuncspec()
                    # XXX Need to check for { and }
                    beginscopecount = line.count('{')
                    endscopecount = line.count('}')
                    if beginscopecount > endscopecount:
                        self.dobeginscope(beginscopecount-endscopecount)
                    elif beginscopecount < endscopecount:
                        self.doendscope(endscopecount-beginscopecount)
                    continue
                match = self.namespace.match(line)
                if match:
                    if self.debug:
                        self.report("\tmatches namespace.")
                    self.donamespace(match)
                    continue
                match = self.klass.match(line)
                if match:
                    if self.debug:
                        self.report("\tmatches class.")
                    self.doclass(match)
                    continue
                beginscopecount = line.count('{')
                endscopecount = line.count('}')
                if beginscopecount > endscopecount:
                    self.dobeginscope(beginscopecount-endscopecount)
                elif beginscopecount < endscopecount:
                    self.doendscope(endscopecount-beginscopecount)
                continue
        except EOFError:
            self.error("Uncaught EOF error")
        self.reportusedtypes()

    def destination(self, type, name, arglist):
        if self.in_class_defn:
            classname = self.namespaces[-1]
            classname = self.pythonizename(classname)
            if classname in self.blacklisttypes:
                return None, None
            return "CxxMethodGenerator", "methods_%s" % classname
        return "FunctionGenerator", "functions"
