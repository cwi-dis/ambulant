from bgenOutput import *
from bgenVariable import *

class BackGeneratorGroup:

    def __init__(self):
        self.generators = []

    def add(self, g, dupcheck=0):
        self.generators.append(g)

    def checkgenerate(self):
        for g in self.generators:
            if g.checkgenerate():
                return True
        return False
        
    def generateDeclaration(self):
        for g in self.generators:
            g.generateDeclaration()
            
    def generate(self):
        for g in self.generators:
            g.generate()
            
    def generateAttributeExistenceTest(self):
        for g in self.generators:
            g.generateAttributeExistenceTest()

class BackModule(BackGeneratorGroup):
    def __init__(self, name,
             includestuff = None,
             finalstuff = None):
        BackGeneratorGroup.__init__(self)
        self.name = name
        self.includestuff = includestuff
        self.finalstuff = finalstuff

    def addobject(self, od):
        self.add(od)

    def generateDeclaration(self):
        if not self.checkgenerate():
            return
        OutHeader1("Declaration of C++ to Python callback module " + self.name)
        Output("#include \"Python.h\"")
        Output()

        if self.includestuff:
            Output("%s", self.includestuff)
            
        self.generateBridgeDeclarations()
    
        BackGeneratorGroup.generateDeclaration(self)
        
    def generateBridgeDeclarations(self):
        OutHeader1("Glue classes to maintain object identity")
        Output("class cpppybridge {")
        Output("  public:")
        Output("\tvirtual ~cpppybridge() {};")
        Output("};")
        Output()
        Output("#if 1")
        Output("extern PyTypeObject pycppbridge_Type;")
        Output()
        Output("extern cpppybridge *pycppbridge_getwrapper(PyObject *o);")
        Output("extern void pycppbridge_setwrapper(PyObject *o, cpppybridge *w);")
        Output()
        Output("inline bool pycppbridge_Check(PyObject *x)")
        OutLbrace()
        Output("return PyObject_TypeCheck(x, &pycppbridge_Type);")
        OutRbrace()
        Output()
        Output("#else")
        Output("inline bool pycppbridge_Check(PyObject *x) { return false; };")
        Output("inline cpppybridge *pycppbridge_getwrapper(PyObject *o) { return NULL; };")
        Output("inline void pycppbridge_setwrapper(PyObject *o, cpppybridge *w) {};")
        Output("#endif")
        Output()
        
        
    def generate(self):
        if not self.checkgenerate():
            return
        OutHeader1("Callbacks Module " + self.name)
        if self.includestuff:
            Output("%s", self.includestuff)

        BackGeneratorGroup.generate(self)

        if self.finalstuff:
            Output()
            Output("%s", self.finalstuff)

        OutHeader1("End callbacks module " + self.name)

class BackObjectDefinition(BackGeneratorGroup):
    baseclass = None   # Can be overridden by subclasses

    def __init__(self, name, dummy, itselftype):
        """ObjectDefinition constructor.  May be extended, but do not override.

        - name: the name of the C++ class we will implement
        - prefix: the prefix used for the object's functions and data, e.g. 'SndCh'.
        - itselftype: the C++ base class we need to implement

        XXX For official Python data types, rules for the 'Py' prefix are a problem.
        """

        BackGeneratorGroup.__init__(self)
        self.name = name
        # Gross hack: assume itselftype is a pointer type. Bad bad.
        assert itselftype[-1] == '*'
        itselftype = itselftype[:-1]
        
        self.itselftype = itselftype
        #self.objecttype = name + 'Object'
        #self.typename = name + '_Type'
        #self.argref = ""    # set to "*" if arg to <type>_New should be pointer
        #self.static = "static " # set to "" to make <type>_New and <type>_Convert public
        #self.modulename = None
        self.virtual_destructor = "virtual "   # Or "", for non-virtual destructor
        if hasattr(self, "assertions"):
            self.assertions()
        if self.baseclass:
            self.baseconstructors = ["::%s(itself)" % self.baseclass]
        else:
            self.baseconstructors = []
        self.othermethods = []
        
    def add(self, g, dupcheck=0):
        BackGeneratorGroup.add(self, g, dupcheck)
        g.setClass(self.name)
        
    def generate(self):
        if not self.checkgenerate():
            return
        OutHeader2("Class %s"%self.name)
                
        self.outputConstructor()
        self.outputDestructor()
        
        BackGeneratorGroup.generate(self)
        
    def generateDeclaration(self):
        if not self.checkgenerate():
            return
        if self.baseclass:
            baseclass = "public %s, " % self.baseclass
        else:
            baseclass = "public cpppybridge, "
        Output("class %s : %spublic %s {", self.name, baseclass, self.itselftype)
        self.generateConDesDeclaration()        

        BackGeneratorGroup.generateDeclaration(self)
        self.outputOtherMethods()
        DedentLevel()
        Output("  private:")
        IndentLevel()
        self.outputMembers()
        self.outputReturnVars()
        self.outputFriends()
        DedentLevel()
        Output("};")
        Output("#define BGEN_BACK_SUPPORT_%s", self.name)
        Output("inline %s *Py_WrapAs_%s(PyObject *o)", self.name, self.name)
        OutLbrace()
        Output("%s *rv = dynamic_cast<%s*>(pycppbridge_getwrapper(o));", self.name, self.name)
        Output("if (rv) return rv;")
        Output("rv = new %s(o);", self.name)
        Output("pycppbridge_setwrapper(o, rv);")
        Output("return rv;")
        OutRbrace()
        Output()
        
    def generateConDesDeclaration(self):
        # XXX Constructor needs to be private, with WrapAs a friend function
        Output("public:")
        IndentLevel()
        Output("%s(PyObject *itself);", self.name)
        Output("%s~%s();", self.virtual_destructor, self.name)
        Output()
        
    def outputOtherMethods(self):
        for om in self.othermethods:
            Output(om)
            
    def outputMembers(self):
        Output("PyObject *py_%s;", self.name)
        
    def outputReturnVars(self):
        rvdecls = []
        for g in self.generators:
            rv = g.checkreturnvar()
            if rv:
                rvdecls.append(rv)
        for d in rvdecls:
            Output("%s;", d)
            
    def outputFriends(self):
        Output()
        Output("friend PyObject *%sObj_New(%s *itself);",
            self.name, self.itselftype)
        
    def outputConstructor(self):
        Output("%s::%s(PyObject *itself)", self.name, self.name)
        self.outputConstructorInitializers()
        OutLbrace()
        self.beginGIL()
        self.outputCheckConstructorArg()
        Output("py_%s = itself;", self.name)
        Output("Py_XINCREF(itself);")
        self.endGIL()
        OutRbrace()
        Output()
        
    def outputConstructorInitializers(self):
        if self.baseconstructors:
            # import pdb ; pdb.set_trace()
            con = ", ".join(self.baseconstructors)
            Output(":\t%s", con)
        
    def outputCheckConstructorArg(self):
        Output("if (itself)")
        OutLbrace()
        self.generateAttributeExistenceTest()
        OutRbrace()
        Output("if (itself == NULL) itself = Py_None;")
        Output()
        
    def outputDestructor(self):
        Output("%s::~%s()", self.name, self.name)
        OutLbrace()
        self.beginGIL()
        Output("PyObject *itself = py_%s;", self.name)
        Output("py_%s = NULL;", self.name)
        Output("if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)")
        OutLbrace()
        Output("pycppbridge_setwrapper(itself, NULL);")
        OutRbrace()
        Output("Py_XDECREF(itself);")
        self.endGIL()
        OutRbrace()
        Output()
        
    def beginGIL(self):
        #OutLbrace()
        Output("PyGILState_STATE _GILState = PyGILState_Ensure();")
        
    def endGIL(self):
        Output("PyGILState_Release(_GILState);")
        #OutRbrace()
        
class BackMethodGenerator:
    
    def __init__(self, returntype, name, *argumentList, **conditionlist):
        self.returntype = returntype
        self.name = name
        self.argumentList = []
        self.parseArgumentList(argumentList)
        self.classname = ''
        self.condition = conditionlist.get('condition')
        self.modifiers = conditionlist.get('modifiers', [])
        if 'const' in self.modifiers:
            self.const = ' const'
        else:
            self.const = ''
        self.virtual = 'virtual' in self.modifiers
        self.rvname = "_rv"
        self.rv = None
        self.return_keepref = False
        
    def setClass(self, name):
        self.classname = name
        
    def checkgenerate(self):
        return self.virtual
        
    def checkreturnvar(self):
        """Check whether the return value is a reference.
        
        If it is we need to declare it in the object in stead of
        in the method."""
        if self.returntype:
            # Hacking our way ahead. We peek in the typeName,
            # and to make matters worse we change it to get rid
            # of the reference and the const.
            returntypedecl = self.returntype.getArgDeclarations("")
            if len(returntypedecl) != 1:
                return None
            returntypedecl = returntypedecl[0]
            if "&" in returntypedecl:
                # It appears to be a reference. Check for "const "
                if returntypedecl[:6] == "const ":
                    returntypedecl = returntypedecl[6:]
                while "&" in returntypedecl:
                    returntypedecl = returntypedecl[:-1]
                    
                returnvarname = self.name + "_rvkeepref"
                returntypedecl += " " + returnvarname
                # Hacking gets worse: we create the return
                # variable in the "usual" way hoping things will "just work"
                self.rvname = self.name
                #self.rv = self.makereturnvar()
                self.return_keepref = True
                
                return returntypedecl
        return None
            
    def declarereturnvar(self):
        if self.rv:
            return False
        if not self.returntype:
            return False
        self.rv = self.makereturnvar()
        if self.return_keepref:
            decl = self.rv.type.getArgDeclarations(self.rvname)
            assert len(decl) == 1
            decl = decl[0]
            assert "&" in decl
            if decl[:6] == "const ":
                decl = decl[6:]
            while "&" in decl:
                decl = decl[:-1]
            Output("%s %s;", decl, self.rvname)
            for decl in self.rv.type.getAuxDeclarations(self.rvname):
                Output("%s;", decl)
        else:
            self.rv.declare()
        return True

    def makereturnvar(self):
        return Variable(self.returntype, self.rvname, OutMode)

    def parseArgumentList(self, argumentList):
        iarg = 0
        for type, name, mode in argumentList:
            iarg = iarg + 1
            if name is None: name = "_arg%d" % iarg
            arg = Variable(type, name, mode)
            self.argumentList.append(arg)

    def generateDeclaration(self):
        if not self.checkgenerate():
            return
        if self.condition:
            DedentLevel()
            Output(self.condition)
            IndentLevel()
        Output("%s;", self.getDeclaration())
        if self.condition:
            DedentLevel()
            Output("#endif")
            IndentLevel()
        
    def getDeclaration(self, qualified=False):
        if qualified:
            name = "%s::%s" % (self.classname, self.name)
        else:
            name = self.name
        if self.returntype:
            namedecl = self.returntype.getArgDeclarations(name)
            if len(namedecl) != 1:
                raise RuntimeError, "Illegal return type"
            namedecl = namedecl[0]
        else:
            namedecl = "void %s" % name
        argdecllist = []
        for arg in self.argumentList:
            argdecllist = argdecllist + arg.getArgDeclarations(fullmodes=True)
        argdecl = ', '.join(argdecllist)
        return "%s(%s)%s" % (namedecl, argdecl, self.const)
        
    def generate(self):
        if not self.checkgenerate():
            return
        Output()
        if self.condition:
            Output(self.condition)

        self.functionheader()
        self.functionbody()
        self.functiontrailer()

        if self.condition:
            Output("#endif")

    def functionheader(self):
        Output("%s", self.getDeclaration(qualified=True))
        OutLbrace()
        auxdecllist = []
        for arg in self.argumentList:
            auxdecllist = auxdecllist + arg.getAuxDeclarations()
        if auxdecllist:
            for decl in auxdecllist:
                Output("%s;", decl)
            Output()

    def functionbody(self):
        self.beginGIL()
        self.declarations()
        self.precheck()
        self.callit()
        self.checkit()
        self.returnargs()
        self.endGIL()
        self.returnvalue()

    def functiontrailer(self):
        OutRbrace()
        
    def beginGIL(self):
        #OutLbrace()
        Output("PyGILState_STATE _GILState = PyGILState_Ensure();")
        
    def endGIL(self):
        Output("PyGILState_Release(_GILState);")
        #OutRbrace()
        
    def returnGIL(self):
        Output("PyGILState_Release(_GILState);")
        
    def declarations(self):
        anydone = self.declarereturnvar()
        for arg in self.argumentList:
            if arg.mode in (InMode, InOutMode):
                arg.mkvaluePreCheck()
                initializer = 'Py_BuildValue("%s", %s)' % (arg.mkvalueFormat(), arg.mkvalueArgs())
                Output("PyObject *py_%s = %s;", arg.name, initializer)
                anydone = True
        if anydone:
            Output()
        
    def precheck(self):
        # Could acquire Python lock here
        pass
        
    def callit(self):
        argsformat = self.getargsformat()
        argsnames = self.getargsnames()
        if argsnames:
            argsnames = ', ' + ', '.join(argsnames)
        else:
            argsnames = ''
        Output('PyObject *py_rv = PyObject_CallMethod(py_%s, "%s", "(%s)"%s);',
            self.classname, self.name, argsformat, argsnames)
        
        
    def checkit(self):
        Output("if (PyErr_Occurred())")
        OutLbrace()
        #self.returnGIL()
        Output("PySys_WriteStderr(\"Python exception during %s::%s() callback:\\n\");", self.classname, self.name)
        Output("PyErr_Print();")
        OutRbrace()
        Output()
        
    def returnargs(self):
        pyvars = ['py_rv']
        if self.rv:
            self.rv.getargsPreCheck()
            fmt = self.rv.getargsFormat()
            args = self.rv.getargsArgs()
        else:
            fmt = ""
            args = ""
        nargs = 0
        for arg in self.argumentList:
            if arg.mode in (OutMode, InOutMode):
                arg.getargsPreCheck()
                fmt = fmt + arg.getargsFormat()
                thisargs = arg.getargsArgs()
                if thisargs:
                    nargs += 1
                    if args:
                        args = args + ", " + thisargs
                    else:
                        args = thisargs
            if arg.mode in (InMode, InOutMode):
                pyvars.append('py_%s' % arg.name)
        if fmt:
            if nargs > 1:
                fmt = "(" + fmt + ")"
            Output('if (py_rv && !PyArg_Parse(py_rv, "%s", %s))',
                fmt, args)
            OutLbrace()
            #self.returnGIL()
            Output("PySys_WriteStderr(\"Python exception during %s::%s() return:\\n\");", self.classname, self.name)
            Output("PyErr_Print();")
            OutRbrace()
            Output()
        if self.rv:
            self.rv.getargsCheck()
        for arg in self.argumentList:
            if arg.mode in (OutMode, InOutMode):
                arg.getargsCheck()
        for pyvar in pyvars:
            Output("Py_XDECREF(%s);", pyvar)
        Output()
 
    def returnvalue(self):
        if self.rv:
            #self.converttoc(self.rv)
            if self.return_keepref:
                fixconst = ""
                if self.const == ' const':
                    fixconst = 'const_cast<%s *>(this)->' % self.classname
                Output("%s%s_rvkeepref = %s;", fixconst, self.rv.name, self.rv.name)
                Output("return %s_rvkeepref;", self.rv.name)
            else:
                Output("return %s;", self.rv.name)
            
    def getargsformat(self):
        format = ""
        for arg in self.argumentList:
            if arg.mode in (InMode, InOutMode):
                format = format + "O"
        return format

    def getargsnames(self):
        argnames = []
        for arg in self.argumentList:
            if arg.mode in (InMode, InOutMode):
                argnames.append("py_%s" % arg.name)
        return argnames
        
    def generateAttributeExistenceTest(self):
        Output('if (!PyObject_HasAttrString(itself, "%s")) PyErr_Warn(PyExc_Warning, "%s: missing attribute: %s");',
            self.name, self.classname, self.name)

def _test():
    m = BackModule("spam")
    o = BackObjectDefinition("eggs", "eggsbase")
    m.addobject(o)
    o.add(BackMethod("ham"))
    o.add(BackMethod("cheese"))
    m.generate()
    
if __name__ == '__main__':
    _test()
    
