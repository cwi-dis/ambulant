from distutils.core import setup, Extension
import sys
import os

DEFS =  [
    ('WITH_EXTERNAL_DOM', '1'),
    ('ENABLE_NLS', '1'),
    ]
    
EXTRA_LINK_ARGS=[]
ldflags=os.getenv("LDFLAGS")
if ldflags:
    EXTRA_LINK_ARGS=ldflags.split()
if sys.platform == 'darwin':
    EXTRA_LINK_ARGS += ['-framework', 'CoreFoundation', '-framework', 'Cocoa']

LIBRARIES=['ambulant', 'expat']
if True:
    DEFS.append(('WITH_QT', '1'))
    LIBRARIES.append('ambulant_qt')
if True:
    DEFS.append(('WITH_FFMPEG', '1'))
    LIBRARIES.append('ambulant_ffmpeg')
if True:
    DEFS.append(('WITH_SDL', '1'))
    LIBRARIES.append('ambulant_sdl')

LIBDIRS=['../../third_party_packages/expat-unix/lib']
if sys.platform == 'darwin':
    LIBRARIES += ['intl']
    
setup(name='ambulant',
      version='0.1',
      ext_modules=[
        Extension('ambulant',
            ['ambulantmodule.cpp', 'ambulantinterface.cpp', 'ambulantutilities.cpp'],
            libraries=LIBRARIES,
            extra_link_args=EXTRA_LINK_ARGS,
            library_dirs=LIBDIRS,
            include_dirs=['../../include'],
            define_macros=DEFS
        )
      ]
)
