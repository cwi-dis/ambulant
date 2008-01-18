// This file is a workaround for automake's inability to handle ObjC++:
// We pretend we have an ObjC source file, but add a compiler option to
// compile it as ObjC++.
#include "cg_image.mm"
