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
        
class CxxGeneratorGroupMixin:
    """Mixin class for ObjectDefinition and Module that handles duplicate
    names."""
    
    def resolveduplicates(self):
        names = {}
        dupnames = {}
        # First collect duplicate names in dupnames, and
        # also initialize the counter to 1
        for g in self.generators:
            name = g.name
            if name in names:
                dupnames[name] = 1
                print 'DBG: duplicate %s.%s' % (self.name, name)
            names[name] = True
        for g in self.generators:
            name = g.name
            if name in dupnames:
                count = dupnames[name]
                dupnames[name] = count + 1
                name = '%s_%d' % (name, count)
                g.name = name
        for g in self.generators:
            if isinstance(g, CxxGeneratorGroupMixin):
                g.resolveduplicates()
        
                
class CxxMixin(CxxGeneratorGroupMixin):
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

    def outputCheckNewArg(self):
        """This implementation assumes we are generating a two-way bridge, and
        the Convert method has been declared a friend of the C++ encapsulation
        class. And it assumes RTTI."""
        DedentLevel()
        Output("#ifdef BGEN_BACK_SUPPORT_%s", self.name)
        IndentLevel()
        Output("%s *encaps_itself = dynamic_cast<%s *>(itself);",
            self.name, self.name)
        Output("if (encaps_itself && encaps_itself->py_%s)", self.name)
        OutLbrace()
        Output("Py_INCREF(encaps_itself->py_%s);", self.name)
        Output("return encaps_itself->py_%s;", self.name)
        OutRbrace()
        DedentLevel()
        Output("#endif")
        IndentLevel()

    def outputTypeObjectInitializerCompat(self):
        pass
        
class CxxModule(CxxGeneratorGroupMixin, Module):
    pass
    
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
        self.silent = 0
        #self.debug = 1
        
    def initnamespaces(self):
        self.namespaces = []
        self.in_class_defn = 0
        self.last_scope_name = None
        self.last_scope_was_class = False

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
            mode = mode + "+ConstMode"
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
        self.head_pat = r"^\s*(AMBULANTAPI|static|virtual|extern|pyapi)\s+"
        self.tail_pat = r"[;={}]"
        self.type_pat = r"(?P<storage>AMBULANTAPI|static|virtual|extern|pyapi)" + \
                        r"\s+" + \
                        r"(?P<type>[a-zA-Z0-9_*:& \t]*[ *&])"
        self.name_pat = r"(?P<name>[a-zA-Z0-9_]+)\s*"
        self.args_pat = r"\((?P<args>[^\(;\)]*)\)"
        self.const_pat = r"\s*(?P<const>const)?"
        self.whole_pat = self.type_pat + self.name_pat + self.args_pat + self.const_pat
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
        self.last_scope_name = name
        self.last_scope_was_class = False
        if self.debug:
            self.report("      %d: namespace %s" % (len(self.namespaces), name))
        
    def doclass(self, match):
        name = match.group("name")
        self.last_scope_name = name
        self.last_scope_was_class = True
        if self.debug:
            self.report("      %d: namespace %s" % (len(self.namespaces), name))
            
    def dobeginscope(self, count):
        for i in range(count):
            if self.last_scope_name:
                self.namespaces.append(self.last_scope_name)
                self.last_scope_name = None
                if self.last_scope_was_class:
                    self.in_class_defn += 1
            else:
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
                match = self.klass.match(line)
                if match:
                    if self.debug:
                        self.report("\tmatches class.")
                    self.doclass(match)
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

    def getmodifiers(self, match):
        modifiers = []

        if match.group('storage') == 'static':
            modifiers.append('static')
        elif match.group('storage') == 'virtual':
            modifiers.append('virtual')
            
        if match.group('const') == 'const':
            modifiers.append('const')

        return modifiers
        
    def checkduplicate(self, name):
        """By default we do not check for duplicates in C++ code"""
        self.alreadydone.append(name)
        return False

    def destination(self, type, name, arglist, modifiers=[]):
        if self.in_class_defn:
            classname = self.namespaces[-1]
            classname = self.pythonizename(classname)
            # First check that we don't skip this class altogether
            if classname in self.blacklisttypes:
                return None, None
            
            # Next, treat static methods as functions.
            if "static" in modifiers:
                return "Function", "functions"
                
            # Finally treat const methods differently
            if "const" in modifiers:
                return "ConstMethod", "methods_%s" % classname
            return "Method", "methods_%s" % classname
        return "Function", "functions"

    def generatemodifiers(self, classname, name, modifiers):
        if classname == 'Function' and self.namespaces:
            callname = '::'.join(self.namespaces + [name])
            self.specfile.write('    callname="%s",\n' % callname)
        if modifiers:
            self.specfile.write('    modifiers=%s,\n' % repr(modifiers))
            
            