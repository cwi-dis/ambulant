from bgenOutput import *
from bgenVariable import *

class BackGeneratorGroup:

    def __init__(self):
        self.generators = []

    def add(self, g, dupcheck=0):
        self.generators.append(g)

    def generateDeclaration(self):
        for g in self.generators:
            g.generateDeclaration()
            
    def generate(self):
        for g in self.generators:
            g.generate()

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
        OutHeader1("Declaration of C++ to Python callback module " + self.name)
        Output("#include \"Python.h\"")
        Output()

        if self.includestuff:
            Output("%s", self.includestuff)
    
        BackGeneratorGroup.generateDeclaration(self)
        
    def generate(self):
        OutHeader1("Callbacks Module " + self.name)
        if self.includestuff:
            Output("%s", self.includestuff)

        BackGeneratorGroup.generate(self)

        if self.finalstuff:
            Output()
            Output("%s", self.finalstuff)

        OutHeader1("End callbacks module " + self.name)

class BackObjectDefinition(BackGeneratorGroup):

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
        self.virtual = "virtual "   # Or "", for non-virtual destructor
        if hasattr(self, "assertions"):
            self.assertions()
        self.baseconstructors = None
        
    def add(self, g, dupcheck=0):
        BackGeneratorGroup.add(self, g, dupcheck)
        g.setClass(self.name)
        
    def generate(self):
        OutHeader2("Class %s"%self.name)
                
        self.outputConstructor()
        self.outputDestructor()
        
        BackGeneratorGroup.generate(self)
        
    def generateDeclaration(self):
        Output("class %s : public %s {", self.name, self.itselftype)
        self.generateConDesDeclaration()        
        Output("public:")
        IndentLevel()
        BackGeneratorGroup.generateDeclaration(self)
        DedentLevel()
        Output("  private:")
        IndentLevel()
        self.outputMembers()
        DedentLevel()
        Output("};")
        Output()
        
    def generateConDesDeclaration(self):
        Output("public:")
        IndentLevel()
        Output("%s(PyObject *itself);", self.name)
        Output("%s~%s();", self.virtual, self.name)
        Output()
        DedentLevel()
        
    def outputMembers(self):
        Output("PyObject *py_%s;", self.name)
        
    def outputConstructor(self):
        Output("%s::%s(PyObject *itself)", self.name, self.name)
        self.outputConstructorInitializers()
        OutLbrace()
        self.outputCheckConstructorArg()
        Output("py_%s = itself;", self.name)
        Output("Py_XINCREF(itself);")
        OutRbrace()
        Output()
        
    def outputConstructorInitializers(self):
        if self.baseconstructors:
            Output(":\t%s", self.baseconstructors)
        
    def outputCheckConstructorArg(self):
        Output("if (itself == NULL) itself = Py_None;")
        Output()
        
    def outputDestructor(self):
        Output("%s::~%s()", self.name, self.name)
        OutLbrace()
        Output("Py_XDECREF(py_%s);", self.name)
        OutRbrace()
        Output()
        
class BackMethodGenerator:
    const = ""
    
    def __init__(self, returntype, name, *argumentList, **conditionlist):
        self.returntype = returntype
        self.name = name
        self.setreturnvar()
        self.virtual = 'virtual '
        self.argumentList = []
        self.parseArgumentList(argumentList)
        self.classname = ''
        self.condition = conditionlist.get('condition')
        
    def setClass(self, name):
        self.classname = name
        
    def setreturnvar(self):
        if self.returntype:
            self.rv = self.makereturnvar()
        else:
            self.rv = None

    def makereturnvar(self):
        return Variable(self.returntype, "_rv", OutMode)

    def parseArgumentList(self, argumentList):
        iarg = 0
        for type, name, mode in argumentList:
            iarg = iarg + 1
            if name is None: name = "_arg%d" % iarg
            arg = Variable(type, name, mode)
            self.argumentList.append(arg)

    def generateDeclaration(self):
        if self.condition:
            DedentLevel()
            Output(self.condition)
            IndentLevel()
        Output("%s%s;", self.virtual, self.getDeclaration())
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
            namedecl = self.returntype.getDeclaration(name)
        else:
            namedecl = "void %s" % name
        argdecllist = []
        for arg in self.argumentList:
            argdecllist.append(arg.getDeclaration())
        argdecl = ', '.join(argdecllist)
        return "%s(%s)%s" % (namedecl, argdecl, self.const)
        
    def generate(self):
        if self.condition:
            Output(self.condition)

        self.functionheader()
        self.functionbody()
        self.functiontrailer()

        if self.condition:
            Output("#endif")

    def functionheader(self):
        Output()
        Output("%s", self.getDeclaration(qualified=True))
        OutLbrace()

    def functionbody(self):
        Output("/* XXX To be provided */")
        self.declarations()
        self.precheck()
        self.callit()
        self.checkit()
        self.returnargs()
        self.returnvalue()

    def functiontrailer(self):
        OutRbrace()
        
    def declarations(self):
        if self.rv:
            self.rv.declare()
        for arg in self.argumentList:
            if arg.mode in (InMode, InOutMode):
                initializer = 'Py_BuildValue("%s", %s)' % (arg.mkvalueFormat(), arg.mkvalueArgs())
                Output("PyObject *py_%s = %s;", arg.name, initializer)
        
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
        Output('PyObject *py_rv = PyObject_CallMethod(py_%s, "%s", "%s"%s);',
            self.classname, self.name, argsformat, argsnames)
        
    def checkit(self):
        Output("if (PyErr_Occurred()) abort();");
        
    def returnargs(self):
        for arg in self.argumentList:
            self.converttoc(arg)
        
    def returnvalue(self):
        if self.rv:
            self.converttoc(self.rv)
            Output("return %s;", self.rv.name)
        else:
            Output("Py_DECREF(py_rv);")
            
    def converttoc(self, arg):
        if arg.mode in (OutMode, InOutMode, ReturnMode):
            Output('if (!PyArg_Parse(py_%s, "%s", %s)',
                self.rv.name,
                self.rv.getargsFormat(),
                self.rv.getargsArgs())
            IndentLevel()
            Output("abort();")
            DedentLevel()
        Output('Py_DECREF(py_%s);', arg.name)

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

class ConstBackMethodGenerator(BackMethodGenerator):
    const = " const"
    
def _test():
    m = BackModule("spam")
    o = BackObjectDefinition("eggs", "eggsbase")
    m.addobject(o)
    o.add(BackMethod("ham"))
    o.add(BackMethod("cheese"))
    m.generate()
    
if __name__ == '__main__':
    _test()
    
