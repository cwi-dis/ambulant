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

    def generate(self):
        OutHeader1("Callbacks Module " + self.name)
        Output("#include \"Python.h\"")
        Output()

        if self.includestuff:
            Output()
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
        
    def add(self, g, dupcheck=0):
        BackGeneratorGroup.add(self, g, dupcheck)
        g.setClass(self.name)
        
    def generate(self):
        OutHeader2("Class %s"%self.name)
        
        self.outputClassDeclaration()
        
        self.outputConstructor()
        self.outputDestructor()
        
        BackGeneratorGroup.generate(self)
        
    def outputClassDeclaration(self):
        Output("class %s : public %s {", self.name, self.itselftype)
        self.generateConDesDeclaration()        
        Output("public:")
        IndentLevel()
        self.generateDeclaration()
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
        
    def outputConstructorInitializers():
        pass
        
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
    def __init__(self, returntype, name, *argumentList, **conditionlist):
        self.returntype = returntype
        self.name = name
        self.setreturnvar()
        self.virtual = 'virtual '
        self.argumentList = []
        self.parseArgumentList(argumentList)
        self.classname = ''
        
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
        Output("%s%s;", self.virtual, self.getDeclaration())
        
    def getDeclaration(self, qualified=False):
        if qualified:
            name = "%s::%s" % (self.classname, self.name)
        else:
            name = self.name
        if self.returntype:
            namedecl = self.returntype.getDeclaration(name)
        else:
            namedecl = "void %s" % self.name
        argdecllist = []
        for arg in self.argumentList:
            argdecllist.append(arg.getDeclaration())
        argdecl = ', '.join(argdecllist)
        return "%s(%s)" % (namedecl, argdecl)
        
    def generate(self):
        Output("%s", self.getDeclaration(qualified=True))
        OutLbrace()
        if self.rv:
            self.rv.declare()
        Output("/* insert code here */")
        if self.rv:
            Output("return _rv;")
        OutRbrace()
        Output()
        
def _test():
    m = BackModule("spam")
    o = BackObjectDefinition("eggs", "eggsbase")
    m.addobject(o)
    o.add(BackMethod("ham"))
    o.add(BackMethod("cheese"))
    m.generate()
    
if __name__ == '__main__':
    _test()
    
