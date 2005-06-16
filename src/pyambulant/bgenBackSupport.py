from bgenOutput import *

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

    def __init__(self, name, itselftype):
        """ObjectDefinition constructor.  May be extended, but do not override.

        - name: the name of the C++ class we will implement
        - prefix: the prefix used for the object's functions and data, e.g. 'SndCh'.
        - itselftype: the C++ base class we need to implement

        XXX For official Python data types, rules for the 'Py' prefix are a problem.
        """

        BackGeneratorGroup.__init__(self)
        self.name = name
        self.itselftype = itselftype
        #self.objecttype = name + 'Object'
        #self.typename = name + '_Type'
        #self.argref = ""    # set to "*" if arg to <type>_New should be pointer
        #self.static = "static " # set to "" to make <type>_New and <type>_Convert public
        #self.modulename = None
        if hasattr(self, "assertions"):
            self.assertions()
		
		
	def generate(self):
		OutHeader2("Class %s", self.name)
		
		self.outputClassDeclaration()
		
		BackGeneratorGroup.generate(self)
		
	def outputClassDeclaration(self):
		Output("class %s : public %s {", self.name, self.itselfname)
		Output("  public:")
		IndentLevel()
		self.generateDeclaration()
		DedentLevel()
		Output("  private:")
		IndentLevel()
		self.outputMembers()
		DedentLevel()
		Output("};")
		Output()
		
	def outputMembers(self):
		Output("PyObject *py_%s;", self.name)
		
class BackMethod:
	def __init__(self, name):
		self.name = name
		self.virtual = 'virtual '
		self.returntype = 'void'
		self.argdeclaration = ''
		
	def generateDeclaration(self):
		Output("%s%s %s(%s);", self.virtual, self.returntype, self.name, self.argdeclaration)
		
	def generate(self):
		Output("%s", self.returntype)
		Output("%s (%s)", self.name, self.argdeclaration)
		OutLbrace()
		Output("/* insert code here */")
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
	