# Experimental plugin for AmbulantPlayer (www.ambulantplayer.org)
# It implements a wrapper for ginga-ncl (http://www.gingancl.org.br/)
# s.t. NCL media presentations can be used inside SMIL presentations
#
# Ginga-NCL and Ambulant need to be installed, with their header files.
#
AC_INIT(ambulant-recorder_plugin, 2.3, Ambulant-feedback@ambulantplayer.org)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

AC_LANG_CPLUSPLUS
AC_PROG_CXX([g++])
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

for incdir in /usr/include/ /usr/local/include/ ; do
if test -d ${incdir}; then
CPPFLAGS="-I${incdir} $CPPFLAGS"
fi
done
CPPFLAGS="-I../../include $CPPFLAGS"

AC_MSG_RESULT($test_gtk)
#
# Find ambulant SMIL player library
#
AC_MSG_CHECKING(for ambulant SMIL rendering library)
test_ambulant=true
PKG_CHECK_MODULES(AMBULANT, libambulant >= 2.3, test_ambulant=true, test_ambulant=false)
if $test_ambulant; then
	AC_DEFINE(WITH_AMBULANT, 1, [Define to enable 'ambulant' SMIL rendering library])
else 
	AC_MSG_ERROR([ambulant >= 2.3 not found])
fi
#
# Find ambulant SDL 2 audio/video rendering library
#
AC_MSG_CHECKING(for ambulant libSDL 2.0 Audio/Video rendering library)
test_sdl=true
PKG_CHECK_MODULES(SDL, sdl2 >= 2.0.0, test_sdl=true, test_sdl=false)
if $test_sdl; then
	AC_DEFINE(WITH_SDL, 1, [Define to enable 'SDL' Audio/Video rendering library])
else 
	AC_MSG_ERROR([SDL >= 2.0.0 not found])
fi

AC_SUBST(SDL_LIBS)
AC_SUBST(SDL_CFLAGS)
AM_CONDITIONAL(WITH_SDL, test x$test_sdl = xtrue)
AC_MSG_RESULT($test_sdl)

AC_CONFIG_FILES(Makefile src/Makefile tests/Makefile)
AC_OUTPUT

AC_MSG_RESULT([
Packageqs found: 
  Ambulant		  $test_ambulant

Now type 'make [[install]]' to build [[and install]] AC_PACKAGE_STRING
])

