from distutils.core import setup, Extension
DEFS =  [
    ('USE_SMIL21', '1'),
    ('WITH_EXTERNAL_DOM', '1'),
    ('ENABLE_NLS', '1'),
    ]

setup(name='ambulant',
      version='0.1',
      ext_modules=[
        Extension('ambulant',
            ['ambulantmodule.cpp', 'ambulantinterface.cpp', 'ambulantutilities.cpp'],
            libraries=['ambulant', 'expat', 'intl'],
            extra_link_args=['-framework', 'CoreFoundation'],
            library_dirs=['../../third_party_packages/expat-unix/lib'],
            include_dirs=['../../include'],
            define_macros=DEFS
        )
      ]
)
