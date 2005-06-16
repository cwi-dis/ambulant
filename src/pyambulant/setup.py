from distutils.core import setup, Extension


setup(name='ambulant',
      version='0.1',
      ext_modules=[
        Extension('ambulant',
            ['ambulantmodule.cpp'],
            libraries=['ambulant', 'expat'],
            extra_link_args=['-framework', 'CoreFoundation'],
            library_dirs=['../../third_party_packages/expat-unix/lib']
        )
      ]
)
