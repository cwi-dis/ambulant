from distutils.core import setup, Extension
import sys
import os

#
# Get compile and link arguments from environment variables (set by the makefile)
# and some per-platform magic.
#
INCDIRS=['../../include']
DEFS=[ 
#	('WITH_PYTHON_PLUGIN', '1'),
#	('WITH_REMOTE_SYNC', '1'),
	]
EXTRA_LINK_ARGS=[]
LIBRARIES=[]
LIBDIRS=[]

if 0:
    # This turns out to be a bad idea: setup() already picks up the
    # CFLAGS and LDFLAGS environment variables.
    cflags=os.getenv("CFLAGS")
    if cflags:
        cflags=cflags.split()
        for cflag in cflags:
            if cflag[:2] == "-I":
                INCDIRS.append(cflag[2:])
            elif cflag[:2] == "-D":
                if "=" in cflag:
                    name, value = cflag[2:].split("=")
                    DEFS.append((name, value))
                else:
                    DEFS.append((cflag[2:], '1'))

    ldflags=os.getenv("LDFLAGS")
    if sys.platform == "win32":
        if ldflags:
            ldflags=ldflags.split()
            for flag in ldflags:
                if flag[:2] == '-L':
                    LIBDIRS.append(flag[2:])
                else:
                    LIBRARIES.append(flag)
    else:				
        if ldflags:
            EXTRA_LINK_ARGS=ldflags.split()

#if sys.platform != 'win32':
#    DEFS.append(('ENABLE_NLS', '1'))
    
if sys.platform == 'win32':
    libambulant = os.getenv("LIBAMBULANT")
    if libambulant:
        LIBRARIES.append(libambulant)
    elif "--debug" in sys.argv:
        LIBRARIES.append('libambulant_shwin32_D')
    else:
        LIBRARIES.append('libambulant_shwin32')
else:
    LIBRARIES.append('ambulant')
        
setup(name='ambulant',
      version='2.6',
      ext_modules=[
        Extension('ambulant',
            ['ambulantmodule.cpp', 'ambulantinterface.cpp', 'ambulantutilities.cpp'],
            libraries=LIBRARIES,
            extra_link_args=EXTRA_LINK_ARGS,
            library_dirs=LIBDIRS,
            include_dirs=INCDIRS,
            define_macros=DEFS,
            export_symbols=['initambulant', 'factoriesObj_New', 'gui_playerObj_New']
        )
      ]
)
