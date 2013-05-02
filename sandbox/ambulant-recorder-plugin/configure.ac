# Experimental plugin for AmbulantPlayer (www.ambulantplayer.org)
#
# See: README for details
#
AC_INIT(ambulant-recorder_plugin, 2.5, Ambulant-feedback@ambulantplayer.org)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

AC_LANG_CPLUSPLUS
AC_PROG_CXX([g++])
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

#
# Find ambulant SMIL player library
#
AC_MSG_CHECKING(for ambulant SMIL rendering library)
test_ambulant=true
PKG_CHECK_MODULES(AMBULANT,libambulant >= 2.3, test_ambulant=true,test_ambulant=false)
if $test_ambulant; then
	AC_DEFINE(WITH_AMBULANT, 1, [Define to enable 'ambulant' SMIL rendering library])
else 
	AC_MSG_ERROR([ambulant >= 2.3 not found])
fi
AC_SUBST(AMBULANT_LIBS)
AC_SUBST(AMBULANT_CFLAGS)
AM_CONDITIONAL(WITH_AMBULANT, test x$test_ambulant = xtrue)
AC_MSG_RESULT($test_ambulant)

#
# Find ambulant SDL 2 audio/video rendering library
#
AC_MSG_CHECKING(for libSDL 2.0 Audio/Video rendering library)
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

AC_ARG_WITH(python,
    AS_HELP_STRING([--with-python], [Use Python plugin instead of C-version]),
    [
    	case "${withval}" in
        yes)
            PYTHON=`which python` 
            with_python=true
            ;;
        no)
            PYTHON=
            with_python=false
            ;;
        *)
            PYTHON=`which ${withval}`
            with_python=true
            ;;
    	esac],
    [with_python=false])
AM_CONDITIONAL(WITH_PYTHON, test x$with_python = xtrue)
AC_SUBST(PYTHON)

AC_CONFIG_FILES(Makefile src/Makefile tests/Makefile)
AC_OUTPUT

AC_MSG_RESULT([
Packageqs found: 
  Ambulant		  $test_ambulant
  SDL 2:		  $test_sdl

Now type 'make [[install]]' to build [[and install]] AC_PACKAGE_STRING
])

