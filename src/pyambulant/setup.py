from distutils.core import setup, Extension
import sys
import os

# Distinguish 32/64 bit installation
##import struct
##if struct.calcsize('P') == 8:
##    bits='64'
##else:
##    bits=''

# Set these variable identical to your configure options.
##if sys.platform == 'linux2':
##    # Not really the correct test, but okay for the time being
##    WITH_GTK=True
##    WITH_QT=False
##else:
##    WITH_GTK=False
##    WITH_QT=False
##if sys.platform == 'win32':
##    WITH_FFMPEG=False
##    WITH_SDL=False
##else:
##    WITH_FFMPEG=True
##    WITH_SDL=True

#
# Get compile and link arguments from environment variables (set by the makefile)
# and some per-platform magic.
#
INCDIRS=['../../include']
DEFS=[ ('WITH_PYTHON_PLUGIN', '1') ]
EXTRA_LINK_ARGS=[]
LIBRARIES=[]
LIBDIRS=[]

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
	if sys.platform == 'darwin':
		EXTRA_LINK_ARGS += ['-framework', 'QuickTime', '-framework', 'CoreFoundation', '-framework', 'Cocoa']

if sys.platform != 'win32':
    DEFS.append(('ENABLE_NLS', '1'))
    
##if WITH_QT:
##    DEFS.append(('WITH_QT', '1'))
##    LIBRARIES.append('ambulant_qt')
##if WITH_GTK:
##    DEFS.append(('WITH_GTK', '1'))
##    LIBRARIES.append('ambulant_gtk')
##    LIBRARIES.append('gtk-x11-2.0')
##    LIBRARIES.append('gdk-x11-2.0')
##    LIBRARIES.append('atk-1.0')
##    LIBRARIES.append('gdk_pixbuf-2.0')
##    LIBRARIES.append('pangocairo-1.0')
##    INCDIRS.append('/usr/include/pygtk-2.0')
##    INCDIRS.append('/usr/include/gtk-2.0')
##    INCDIRS.append('/usr/include/gdk-pixbuf-2.0')
##    INCDIRS.append('/usr/lib/gtk-2.0/include')
##    INCDIRS.append('/usr/include/atk-1.0')
##    INCDIRS.append('/usr/include/cairo')
##    INCDIRS.append('/usr/include/pango-1.0')
##    INCDIRS.append('/usr/include/glib-2.0')
##    INCDIRS.append('/usr/lib'+bits+'/glib-2.0/include')
##    INCDIRS.append('/usr/include/freetype2')
##if WITH_FFMPEG:
##    DEFS.append(('WITH_FFMPEG', '1'))
##    LIBRARIES.append('ambulant_ffmpeg')
##if WITH_SDL:
##    DEFS.append(('WITH_SDL', '1'))
##    LIBRARIES.append('ambulant_sdl')

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
    
##if sys.platform == 'darwin':
##    LIBRARIES += ['intl']
##    INCDIRS += ['/usr/local/include']
    
setup(name='ambulant',
      version='2.4',
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
