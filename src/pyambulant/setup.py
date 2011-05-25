from distutils.core import setup, Extension
import sys
import os

# Distinguish 32/64 bit installation
import struct
if struct.calcsize('P') == 8:
    bits='64'
else:
    bits=''

# Set these variable identical to your configure options.
if sys.platform == 'linux2':
    # Not really the correct test, but okay for the time being
    WITH_GTK=False
    WITH_QT=True
else:
    WITH_GTK=False
    WITH_QT=False
if sys.platform == 'win32':
    WITH_FFMPEG=False
    WITH_SDL=False
else:
    WITH_FFMPEG=True
    WITH_SDL=True

DEFS =  [
    ('WITH_PYTHON_PLUGIN', '1'),
    ('WITH_SEAMLESS_PLAYBACK', '1'),
    ]
if sys.platform != 'win32':
    DEFS.append(('ENABLE_NLS', '1'))
    
EXTRA_LINK_ARGS=[]
ldflags=os.getenv("LDFLAGS")
if ldflags:
    EXTRA_LINK_ARGS=ldflags.split()
if sys.platform == 'darwin':
    EXTRA_LINK_ARGS += ['-framework', 'QuickTime', '-framework', 'CoreFoundation', '-framework', 'Cocoa']

LIBRARIES=[]
LIBDIRS=[]
INCDIRS=['../../include']
if WITH_QT:
    DEFS.append(('WITH_QT', '1'))
    LIBRARIES.append('ambulant_qt')
if WITH_GTK:
    DEFS.append(('WITH_GTK', '1'))
    LIBRARIES.append('ambulant_gtk')
    LIBRARIES.append('gtk-x11-2.0')
    LIBRARIES.append('gdk-x11-2.0')
    LIBRARIES.append('atk-1.0')
    LIBRARIES.append('gdk_pixbuf-2.0')
    LIBRARIES.append('pangocairo-1.0')
    INCDIRS.append('/usr/include/pygtk-2.0')
    INCDIRS.append('/usr/include/gtk-2.0')
    INCDIRS.append('/usr/include/gdk-pixbuf-2.0')
    INCDIRS.append('/usr/lib/gtk-2.0/include')
    INCDIRS.append('/usr/include/atk-1.0')
    INCDIRS.append('/usr/include/cairo')
    INCDIRS.append('/usr/include/pango-1.0')
    INCDIRS.append('/usr/include/glib-2.0')
    INCDIRS.append('/usr/lib'+bits+'/glib-2.0/include')
    INCDIRS.append('/usr/include/freetype2')
if WITH_FFMPEG:
    DEFS.append(('WITH_FFMPEG', '1'))
    LIBRARIES.append('ambulant_ffmpeg')
if WITH_SDL:
    DEFS.append(('WITH_SDL', '1'))
    LIBRARIES.append('ambulant_sdl')

if sys.platform == 'win32':
    if "--debug" in sys.argv: # THIS IS GROSS!!
        LIBRARIES.append('libambulant_shwin32_D')
    else:
        LIBRARIES.append('libambulant_shwin32')
    LIBRARIES.append('libexpat')
    LIBDIRS.append('../../lib/win32')
else:
    LIBRARIES.append('ambulant')
    LIBRARIES.append('expat')
    LIBDIRS.append('../../third_party_packages/expat-unix/lib')
    
if sys.platform == 'darwin':
    LIBRARIES += ['intl']
    INCDIRS += ['/usr/local/include']
    
setup(name='ambulant',
      version='2.2',
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
