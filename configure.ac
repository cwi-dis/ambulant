AC_INIT([ambulant],[2.6])
AC_CONFIG_SRCDIR(src/libambulant/get_version.cpp)
AC_CONFIG_AUX_DIR([libltdl/config])
AC_CONFIG_MACRO_DIR([libltdl/m4])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])
# Maybe better use AUTOMAKE_OPTIONS = subdir-objects
# at the top of any relevant Makefile.am
AM_INIT_AUTOMAKE([subdir-objects])
# Is this better? m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AM_SILENT_RULES([yes])
AM_PATH_PYTHON
AMBULANT_VERSION=2.6
#
# Platform specific setup
#
#############################
AC_CANONICAL_HOST
# Check for which host we are on and setup a few things
# specifically based on the host
on_bsd=false
on_linux=false
on_macosx=false

case $host_os in
  darwin*)
        # Do something specific for mac
		on_macosx=true
        ;;
  linux*)
		on_linux=true
        # Do something specific for linux
        ;;
   *BSD*)
        # Do something specific for BSD
		on_bsd=true
        ;;
    *)
        #Default Case
        AC_MSG_ERROR([Your platform is not currently supported])
        ;;
esac

dnl =================================================================
GTK_REQUIRED=1.2.8

dnl ================================================================

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MKDIR_P
AC_HEADER_STDC
AC_HEADER_ASSERT
AC_PROG_LIBTOOL
LT_AC_PROG_SED
am__api_version="1.9"
#
# Allow either included or standard ltdl
#
LT_CONFIG_LTDL_DIR([libltdl])
LT_INIT(dlopen)
LTDL_INIT(convenience)
# ??? AC_LIBTOOL_DLOPEN
# ??? AC_LIB_LTDL
# ??? AC_SUBST(INCLTDL)
# ??? AC_SUBST(LIBLTDL)

AC_CONFIG_FILES([Makefile po/Makefile.in
	src/Makefile 
	src/npambulant/Makefile
	src/libambulant/Makefile
	src/libambulant/pc-uninstalled/Makefile
	src/player_gtk/Makefile
	src/player_sdl/Makefile
	src/player_macosx/Makefile
	src/plugins/Makefile
	src/pyambulant/Makefile
	include/Makefile
	include/ambulant/Makefile
	include/ambulant/config/Makefile
	include/ambulant/config/compiler/Makefile
	include/ambulant/config/platform/Makefile
	include/ambulant/config/stdlib/Makefile
	include/ambulant/common/Makefile
	include/ambulant/lib/Makefile
	include/ambulant/lib/unix/Makefile
	include/ambulant/lib/win32/Makefile
	include/ambulant/smil2/Makefile
	include/ambulant/net/Makefile
	include/ambulant/gui/Makefile
	include/ambulant/gui/d2/Makefile
	include/ambulant/gui/none/Makefile
	include/ambulant/gui/cg/Makefile
	include/ambulant/gui/gtk/Makefile
	include/ambulant/gui/SDL/Makefile
	include/ambulant/gui/gstreamer/Makefile
	third_party_packages/Makefile
	Documentation/Makefile
	Documentation/API/Makefile
	Extras/Makefile])
	

AC_CONFIG_COMMANDS([libtool_patch],[chmod 755 $srcdir/scripts/patch_libtool.sh;$srcdir/scripts/patch_libtool.sh])

# Export configure args to the makefiles (for make distcheck)
AC_SUBST(ac_configure_args)

#
# First check whether we have third_party_packages in an alternate location (normally during
# "make distcheck").
AC_MSG_CHECKING(for --with-third-party-packages)
third_party_packages=`pwd`/third_party_packages
AC_ARG_WITH(third-party-packages,
	AS_HELP_STRING([--with-third-party-packages],[Directory where 3rd party packages are installed]),
	[
		case $withval in
		yes|no)
			AC_MSG_ERROR("--with-third-party-packages requires a pathname")
			;;
		*)
			third_party_packages=`cd $withval; pwd`
			;;
		esac
	],[
		third_party_packages=`cd third_party_packages; pwd`
		ac_configure_args="$ac_configure_args --with-third_party_packages=$third_party_packages"
	]
)
AC_MSG_RESULT($third_party_packages)

# Variables needed.
export PATH=$third_party_packages/installed/bin:$PATH
export PKG_CONFIG_PATH=$third_party_packages/installed/lib/pkgconfig:$PKG_CONFIG_PATH

#
# Do we have pkg-config?
#
#AC_MSG_CHECKING(for pkg-config)
#PKG_CHECK_MODULES(pkg-config,[pkg-config >= 0.18], pkg_config=true, pkg_config=false)
#AM_CONDITIONAL(WITH_PKG_CONFIG, test $pkg_config = true)
#AC_MSG_RESULT($pkg_config)
AC_CHECK_PROG(PKG_CONFIG,pkg-config,`which pkg-config`)

#
# This variable records whether we have support for
# any windowing system (so we can give a warning at
# the end if not)
#
with_frontend=false

#
# Do we want MacOSX universal (ppc/intel) binary support?
#
AC_MSG_CHECKING(for --with-universalsdk)
AC_ARG_WITH(universalsdk,
    AS_HELP_STRING([--with-universalsdk],[Build MacOSX 10.4 universal (ppc/intel) binary]),
	[
		need_sysroot=no
		case $withval in
		yes)
			withval=/Developer/SDKs/MacOSX10.4u.sdk
			case `uname -r` in
			8*) 
				need_sysroot=yes
				;;
			esac
			;;
		*)	need_sysroot=yes
			;;
		esac
		case $withval in
		no)
			with_universalsdk=false
			need_sysroot=no
			;;
		*)
			with_universalsdk=true
			if test $need_sysroot = yes ; then
				sysroot="-isysroot ${withval}"
			else
				sysroot=""
			fi
			CFLAGS="-arch ppc -arch i386 ${sysroot} ${CFLAGS}"
			CXXFLAGS="-arch ppc -arch i386 ${sysroot} ${CXXFLAGS}"
			OBJCFLAGS="-arch ppc -arch i386 ${sysroot} ${OBJCFLAGS}"
			#LDFLAGS="-arch ppc -arch i386 ${sysroot} ${LDFLAGS}"
			# Need to pass these to recursive configure calls to (esp. libltdl)
			export CFLAGS
			ac_configure_args="$ac_configure_args --disable-dependency-tracking"
			;;
		esac
	],
	[with_universalsdk=false])
AC_MSG_RESULT($with_universalsdk)

#
# Do we want MacOSX fat (32/64 bit) binary support?
#
AC_MSG_CHECKING(for --with-macfat)
AC_ARG_WITH(macfat,
    AS_HELP_STRING([--with-macfat],[Build MacOSX 10.6 universal (32/64 bit intel) binary]),
	[
		if $with_universalsdk; then
			AC_MSG_WARN([--with-universalsdk and --with-macfat are mutually exclusive])
		fi
		case `uname -r` in
		10*)
			;;
		*)
			AC_MSG_WARN([--with-macfat only supported on MacOSX 10.6])
			;;
		esac
		case $withval in
		no)
			with_macfat=false
			;;
		*)
			with_macfat=true
			CFLAGS="-arch x86_64 -arch i386 ${sysroot} ${CFLAGS}"
			CXXFLAGS="-arch x86_64 -arch i386 ${sysroot} ${CXXFLAGS}"
			OBJCFLAGS="-arch x86_64 -arch i386 ${sysroot} ${OBJCFLAGS}"
			#LDFLAGS="-arch x86_64 -arch i386 ${sysroot} ${LDFLAGS}"
			# Need to pass these to recursive configure calls to (esp. libltdl)
			export CFLAGS
			ac_configure_args="$ac_configure_args --disable-dependency-tracking"
			;;
		esac
	],
	[
		with_macfat=false
	])

AC_MSG_RESULT($with_macfat)

#
#
# Do we want plugin support?
#
AC_MSG_CHECKING(for --with-ltdl-plugins)
AC_ARG_WITH([ltdl-plugins],
    AS_HELP_STRING([--without-ltdl-plugins],[Disable plugin support (based on libltdl)]),
    [
		case "${withval}" in
		yes)    ;;
		no)     unset LIBLTDL
				UNSET INCLTDL
				;;
		*)      AC_MSG_ERROR("No value allowed for --with-ltdl-plugins")
				;;
		esac
    ])
if test "x$LIBLTDL" = x;
then
    with_ltdl=false
else
    with_ltdl=true
    AC_DEFINE(WITH_LTDL_PLUGINS, 1, [Define to use libltdl-based plugins])
fi
AM_CONDITIONAL(WITH_LTDL_PLUGINS, test $with_ltdl = true)
AC_MSG_RESULT($with_ltdl)


# Do we want dummy plugins ?
#

AC_MSG_CHECKING(for --with-dummy-plugin)
AC_ARG_WITH([dummy-plugin],
    AS_HELP_STRING([--with-dummy-plugin],[Enable the dummy plugin (default: disabled)]),
	[
		case "${withval}" in
		yes)  	with_dummy_plugin=true
				;;
		no)		with_dummy_plugin=false
				;;
		*)		AC_MSG_ERROR("--with-dummy-plugin takes no argument")
				;;
		esac],
	[with_dummy_plugin=false])
AM_CONDITIONAL(WITH_DUMMY_PLUGIN, test x$with_dummy_plugin = xtrue)
AC_MSG_RESULT($with_dummy_plugin)

#
# Do we want GCD event processor support?
#
AC_MSG_CHECKING(for --with-gcd-event-processor)
AC_ARG_WITH([gcd-event-processor],
    AS_HELP_STRING([--with-gcd-event-processor],[Enable gcd event processor]),
	[
		case "${withval}" in    
		yes) 
			with_gcd_event_processor=yes	
			;;
		no)
			with_gcd_event_processor=no

			;;
		*)
			with_gcd_event_processor=yes	
    			GCD_LIBS=${withval}
			;;
		esac
	],[
		# On MacOSX we don't need any special libraries.
		GCD_LIBS=
    		AC_TRY_LINK([#include "dispatch/dispatch.h"], [(void)dispatch_group_create();], with_gcd_event_processor=yes, with_gcd_event_processor=no)
		if test "x$with_gcd_event_processor" = xno; then
			oldLIBS=$LIBS
			GCD_LIBS=-ldispatch
			LIBS="$LIBS $GCD_LIBS"
			AC_TRY_LINK([#include "dispatch/dispatch.h"], [(void)dispatch_group_create();], with_gcd_event_processor=yes, with_gcd_event_processor=no)
			LIBS=$oldLIBS
		fi
	])
    
if test "x$with_gcd_event_processor" = xyes;
then
    AC_DEFINE(WITH_GCD_EVENT_PROCESSOR, 1, [Define to enable gcd event processor support])
fi
AC_SUBST(GCD_LIBS)
AM_CONDITIONAL(WITH_GCD_EVENT_PROCESSOR, test x$with_gcd_event_processor = xyes)
AC_MSG_RESULT($with_gcd_event_processor)

#
# Do we want external clock sync support?
#
AC_MSG_CHECKING(for --with-external-clock-sync)
AC_ARG_WITH([external-clock-sync],
    AS_HELP_STRING([--with-external-clock-sync],[Enable external clock synchronization]),
    [
    	with_external_clock_sync=yes
    ],[
    	with_external_clock_sync=no
    ])
    
if test "x$with_external_clock_sync" = xyes;
then
    AC_DEFINE(WITH_REMOTE_SYNC, 1, [Define to enable external clock syncing])
fi
AC_MSG_RESULT($with_external_clock_sync)

#
# Do we want overlay window support support?
#
test_overlay=false
AC_MSG_CHECKING(for --with-overlay-window)
AC_ARG_WITH([overlay-window],
    AS_HELP_STRING([--with-overlay-window],[Enable a secondary document overlaying the main document]),
    [with_overlay_window=${withval}],
    [with_overlay_window=yes])
if test "x$with_overlay_window" = xyes;
then
    AC_DEFINE(WITH_OVERLAY_WINDOW, 1, [Define to enable secondary document overlaying the main document])
   test_overlay=true
fi
AC_MSG_RESULT($with_overlay_window)

dnl
dnl Check for expat
dnl
test_expat=false
expat_pkgconfig=false
AC_MSG_CHECKING(for expat XML handling library)
AC_ARG_WITH(expat,
	AS_HELP_STRING([--with-expat], [Turn on/off use of expat XML library]),
	[
		case "${withval}" in
		yes)
			test_expat=true
			expat_pkgconfig=true
			;;
		no)
			;;
		*)
		EXPAT_PREFIX=`cd ${withval} ; pwd`
			EXPAT_LIBS="-L$EXPAT_PREFIX/lib -lexpat"
			EXPAT_CFLAGS="-I$EXPAT_PREFIX/include"
			test_expat=true
			;;
		esac], 
	[expat_pkgconfig=true])

if $expat_pkgconfig; then
	PKG_CHECK_MODULES(EXPAT, [expat >= 2.0.0] , test_expat=true, test_expat=false)
	if $test_expat; then
		:
	else
		dnl No pkgconfig, but maybe available in a standard place?
		oldLIBS=$LIBS
		LIBS="$LIBS -lexpat"
		AC_TRY_LINK([#include <expat.h>], [(void)XML_ParserCreate("");],test_expat=true,test_expat=false)
		LIBS=$oldLIBS
		if $test_expat; then
			EXPAT_LIBS=-lexpat
			EXPAT_CFLAGS=
		else
			expat_warning="Expat not found through pkg-config. Check third_party_packages/readme.txt for instructions."
		fi
	fi
fi
if $test_expat; then
	AC_DEFINE(WITH_EXPAT, 1, [Define to enable expat XML parser])
fi
AC_SUBST(EXPAT_LIBS)
AC_SUBST(EXPAT_CFLAGS)
AM_CONDITIONAL(WITH_EXPAT, test x$test_expat = xtrue)
AC_MSG_RESULT($test_expat)

#
# Find xerces XML handling library
#
AC_MSG_CHECKING(for xerces XML handling library)

AC_ARG_WITH(xerces,
	AS_HELP_STRING([--with-xerces], [Turn on/off use of Xerces XML library]),
	[
		case "${withval}" in    
		yes) 
			test_xerces=true
			;;
		no)
			test_xerces=false
			;;
		*)
			AC_MSG_ERROR("No value allowed for --with-xerces")
			;;
		esac],
	[test_xerces=true])

if $test_xerces; then
	PKG_CHECK_MODULES(XERCES, xerces-c >= 3.0.0, test_xerces=true, test_xerces=false)
fi
if $test_xerces; then
	AC_DEFINE(WITH_XERCES, 1, [Define to enable xerces XML parser])
fi

AC_SUBST(XERCES_LIBS)
AC_SUBST(XERCES_CFLAGS)
AM_CONDITIONAL(WITH_XERCES, test x$test_xerces = xtrue)
AC_MSG_RESULT($test_xerces)

#
# Do we want xerces in a plugin only?
#
AC_MSG_CHECKING(for --with-xerces-plugin)
xerces_plugin=false

AC_ARG_WITH([xerces-plugin],
	AS_HELP_STRING([--with-xerces-plugin],[Put the xerces parser in a plugin]),
    [
		case "${withval}" in    
		yes)
			xerces_plugin=true
			;;
		no)
			xerces_plugin=false
			;;
		*)
			AC_MSG_ERROR("No value allowed for --with-xerces-plugin")
			;;
		esac])
     
if test x$xerces_plugin = xtrue;
then
	if test x$xerces = xfalse;
	then
		AC_MSG_ERROR("--with-xerces-plugin requires --with-xerces")
	fi
	# Disable builtin xerces support
	AM_CONDITIONAL(WITH_XERCES, test false)
else
	if test x$xerces = xtrue;
	then
	    AC_DEFINE(WITH_XERCES_BUILTIN, 1, [Define to include xerces parser in core])
	fi
fi
AM_CONDITIONAL(WITH_XERCES_PLUGIN, test x$xerces_plugin = xtrue)
AM_CONDITIONAL(WITH_XERCES_BUILTIN, test x$test_xerces = xtrue -a x$xerces_plugin = xfalse)
AC_MSG_RESULT($xerces_plugin)

#
# Test that we have at least one XML parser available
#
if test "x$test_expat" = "xfalse" -a "x$test_xerces" = "xfalse"; then
	AC_MSG_WARN([No XML parser configured - Use at least one of --with-expat or --with-xerces])
	xml_parser_warning="No XML parser configured - Use at least one of --with-expat or --with-xerces"
fi

#
#
AC_MSG_CHECKING(for --with-cg)
AC_ARG_WITH(cg,
     AS_HELP_STRING([--with-cg], [Turn on CoreGraphics support]),
     [
		case "${withval}" in
		yes)
			cg=true
			;;
		no)
			cg=false
			;;
		*)
			AC_MSG_ERROR(bad value ${withval} for --with-cg)
			;;
		esac
		with_frontend=true],
     [
		if test -d /System/Library/Frameworks/AppKit.framework; then
			cg=true
			with_frontend=true
		else
			cg=false
		fi
     ])
if test x$cg = xtrue; then
    AC_DEFINE(WITH_CG, 1, [Define to enable CoreGraphics rendering])
fi
AM_CONDITIONAL(WITH_CG, test x$cg = xtrue)
AC_MSG_RESULT($cg)

AC_MSG_CHECKING(for --with-uikit)
AC_ARG_WITH(uikit,
     AS_HELP_STRING([--with-uikit], [Turn on UIKit support]),
     [
		case "${withval}" in
		yes)
			uikit=true
			;;
		no)
			uikit=false
			;;
		*)
			AC_MSG_ERROR(bad value ${withval} for --with-uikit)
			;;
		esac],[uikit=false] )
if test x$uikit = xtrue; then
    if test ! x$cg = xtrue; then
        AC_MSG_ERROR(--with-uikit requires --with-cg)
    fi
    AC_DEFINE(WITH_UIKIT, 1, [Define to enable UIKit main program])
    with_frontend=true
fi
AM_CONDITIONAL(WITH_UIKIT, test x$uikit = xtrue)
AC_MSG_RESULT($uikit)


# Calling _AM_DEPENDENCIES here is a workaround for a bug
# in automake: ObjC support is incomplete. And manually
# doing AC_SUBST(OBJCDEPMODE) also fails.
# Some details are in
# http://mail.gnu.org/archive/html/automake/2003-04/msg00105.html
#
_AM_DEPENDENCIES([OBJC])
OBJCFLAGS="-ObjC++ ${OBJCFLAGS}"
AC_SUBST(OBJCFLAGS)

OBJC=$CC
AC_SUBST(OBJC)

dnl Tests for SDL
dnl --with-sdl2 and WITH_SDL2 (cpp-define en AM_CONDITIONAL) - autotested, means: "use sdl2"
dnl --with-sdl1 and WITH_SDL1 (cpp-define en AM_CONDITIONAL) - autotested, only if WITH_SDL2 is false, means: "use sdl 1.x (audio only)"
dnl WITH_SDL - True if either sdl1 or sdl2 is used.
dnl WITH_SDL_AUDIO - True if WITH_SDL is set, use sdl audio renderer
dnl WITH_SDL_IMAGE - True if WITH_SDL2 is set, Linux is used and SDL2_IMAGE is available
dnl WITH_SDL_VIDEO - True if WITH_SDL2 is set, Linux is used and SDL2_IMAGE is available
dnl WITH_SDL_PANGO - True if WITH_SDL2 is set, Linux is used and SDL_Pango is available
dnl WITH_SDL_TTF - True if WITH_SDL2 is set, WITH_SDL_PANGO is not set, Linux is used and SDL2_ttf is available

dnl ======================================================================
# Prefer sdl2
sdl_version="<none>"
test_sdl=false
test_sdl1=false
test_sdl2=false
test_sdl2_frontend=false
test_sdl_audio=false
test_sdl_video=false
test_sdl_image=false
test_sdl_pango=false
test_sdl_ttf=false
with_sdl_frontend=false
SDL_CFLAGS=
SDL_LIBS=

if test "x$on_linux" = "xtrue" -o "x$on_macosx" = "xtrue"; then
   test_sdl2=true # use SDL2 by default on linux and macosx, if possible
   if test "x$on_linux" = "xtrue" ; then
      test_sdl2_frontend=true # use SDL2 Player  by default on linux, if possible
   fi
fi

AC_MSG_CHECKING(for sdl2 - Simple Direct Media layer - Audio/Video support)
AC_ARG_WITH(sdl2,
	AS_HELP_STRING([--with-sdl2], [Turn on/off SDL2 audio/video support]),
	[
		if test "x$withval" = "xno"; then
		   test_sdl2=false
		fi
	] )

AC_ARG_WITH(sdl1,
	AS_HELP_STRING([--with-sdl1], [Turn on/off SDL1.x audio support]),
	[
		if test "x$withval" = "xyes"; then
		   test_sdl1=true
		fi
	] )

if test "x$test_sdl2" = "xtrue" ; then
	if sdl2-config --version > /dev/null; then
		SDL_CFLAGS="`sdl2-config --cflags`"
		SDL_LIBS=`sdl2-config --libs`
		sdl_version=`sdl2-config --version`
	else
		test_sdl1=true
		test_sdl2=false
	fi
else	
	test_sdl1=true
	test_sdl2=false
fi
if test "x$test_sdl1" = "xtrue"; then
	if sdl-config --version > /dev/null; then
		SDL_CFLAGS="`sdl-config --cflags`"
		SDL_LIBS=`sdl-config --libs`
		sdl_version=`sdl-config --version`
	else
		# for 1.3 we need to use pkg-config
		PKG_CHECK_MODULES(SDL, [sdl >= 1.2.0], test_sdl1=true, test_sdl1=false)
	fi   	
fi

if test "x$test_sdl2" = "xtrue" -o  "x$test_sdl1" = "xtrue"; then
   	test_sdl_audio=true
fi

AC_ARG_WITH(sdl2-frontend,
        AS_HELP_STRING([--with-sdl2-frontend], [Turn on/off SDL2 player]),
        [
                if test "x$withval" = "xyes"; then
			if test "x$test_sdl2" = "xfalse"; then
			      	AC_MSG_ERROR("--with-sdl2-frontend needs --with-sdl2");
			fi   
               		test_sdl2_frontend=true
                fi
        ] )

if test "x$test_sdl2" = "xtrue" -a "x$test_sdl2_frontend" = "xtrue"; then
   	# want these as well
   	test_sdl_image=true
   	test_sdl_video=true
   	test_sdl_pango=true
	with_sdl_frontend=true    # have AmbulantPlayer_sdl too
	with_frontend=true
fi

if test "x$test_sdl_image" = "xtrue"; then
   	# need SDL2_image for images (.png,.gif,.jpg) and video
	PKG_CHECK_MODULES(SDL2_image, [SDL2_image >= 2.0.0], test_sdl_image=true, test_sdl_image=false)
fi
if test "x$test_sdl_image" = "xtrue"; then
	SDL_CFLAGS="$SDL_CFLAGS  -DWITH_SDL_IMAGE `pkg-config --cflags SDL2_image`  `pkg-config --cflags libpng`"
	SDL_LIBS="$SDL_LIBS `pkg-config --libs SDL2_image`"
else
	# If we don't have image support we can't do video either
	test_sdl_video=false
fi
if test "x$test_sdl_pango" = "xtrue"; then
	# need SDL_Pango for smiltext layout
	PKG_CHECK_MODULES(SDL_Pango, [SDL_Pango >= 0.1.3], test_sdl_pango=true, test_sdl_pango=false)
	if test "x$test_sdl_pango" = "xtrue" ; then
	   	SDL_CFLAGS="$SDL_CFLAGS -DWITH_SDL_PANGO `pkg-config --cflags SDL_Pango`"
	   	SDL_LIBS="$SDL_LIBS `pkg-config --libs SDL_Pango`"
	else    # Alternatively SDL2_ttf can be used instead of SDL_Pango
		test_sdl_ttf=true
	fi
fi
if test "x$test_sdl_ttf" = "xtrue" ; then
	PKG_CHECK_MODULES(SDL2_ttf, [SDL2_ttf >= 2.0.0], test_sdl_ttf=true, test_sdl_ttf=false)
fi
if test "x$test_sdl_ttf" = "xtrue" ; then
	SDL_CFLAGS="$SDL_CFLAGS -DWITH_SDL_TTF `pkg-config --cflags SDL2_ttf`"
	SDL_LIBS="$SDL_LIBS `pkg-config --libs SDL2_ttf`"
fi

AM_CONDITIONAL(WITH_SDL, test x$test_sdl_audio = xtrue)
AM_CONDITIONAL(WITH_SDL1, test x$test_sdl1 = xtrue)
AM_CONDITIONAL(WITH_SDL2, test x$test_sdl2 = xtrue)
AM_CONDITIONAL(WITH_SDL_AUDIO, test x$test_sdl_audio = xtrue)
AM_CONDITIONAL(WITH_SDL_VIDEO, test x$test_sdl_video = xtrue)
AM_CONDITIONAL(WITH_SDL_IMAGE, test x$test_sdl_image = xtrue)
AM_CONDITIONAL(WITH_SDL_PANGO, test x$test_sdl_pango = xtrue)
AM_CONDITIONAL(WITH_SDL_TTF, test x$test_sdl_ttf = xtrue)
AM_CONDITIONAL(WITH_SDL_FRONTEND, test x$with_sdl_frontend = xtrue)
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)

AC_MSG_RESULT($test_sdl_audio)
AC_MSG_RESULT($test_sdl_video)
AC_MSG_RESULT($test_sdl_image)
AC_MSG_RESULT($test_sdl_pango)
AC_MSG_RESULT($test_sdl_ttf)
dnl Testing for GTK
dnl ====================================================================

probe_for_gtk=yes
test_gtk=false
AC_MSG_CHECKING(for gtk - the GIMP Toolkit)
AC_ARG_WITH(gtk,
     AS_HELP_STRING([--with-gtk@<:@=gtk2|gtk3@:>@], [Enable/disable gtk2 or gtk3 support]),
     [
     	case ${withval} in
     	no)
     		test_gtk=false
     		probe_for_gtk=no
     		;;
     	gtk3)
     		PKG_CHECK_MODULES(GTK,[gtk+-3.0 >= 3.10.0 gthread-2.0 >= 2.2], test_gtk=gtk3, test_gtk=false)
			if test $test_gtk = false; then
				AC_MSG_WARN([gtk+-3.0 or gthread-2.0 not found, or too old])
			fi
			probe_for_gtk=no
     		;;
     	gtk2)
			PKG_CHECK_MODULES(GTK,[gtk+-2.0 >= 2.6.0 gthread-2.0 >= 2.2], test_gtk=gtk2, test_gtk=false)
			if test $test_gtk = false; then
				AC_MSG_WARN([gtk+-2.0 or gthread-2.0 not found, or too old])
			fi
			probe_for_gtk=no
     		;;
     	yes)
     		probe_for_gtk=yes
     		;;
     	*)
     		AC_MSG_WARN([Unknown value for --with-gtk: ${withval}])
     		probe_for_gtk=no
     		test_gtk=false
     		;;
		esac
     ],[
     	probe_for_gtk=yes
     ])
if test x$probe_for_gtk = xyes ; then
	# GTK2 is preferred over GTK3, for now
	PKG_CHECK_MODULES(GTK,[gtk+-2.0 >= 2.6.0 gthread-2.0 >= 2.2], [
		test_gtk=gtk2
	],[
		PKG_CHECK_MODULES(GTK,[gtk+-3.0 >= 3.10.0 gthread-2.0 >= 2.2], test_gtk=gtk3, test_gtk=false)
	])
fi
if test $test_gtk != false; then
	with_frontend=true
fi
AM_CONDITIONAL(WITH_GTK, test x$test_gtk != xfalse)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_MSG_RESULT($test_gtk)

dnl Testing for gstreamer
dnl ====================================================================

test_gstreamer=false
AC_MSG_CHECKING(for gstreamer multimedia framework)
AC_ARG_WITH(gstreamer,
     AS_HELP_STRING([--with-gstreamer], [Enable/disable gstreamer support]),
     [
     	if test "x$withval" = "xyes"; then
			test_gstreamer=true	
      	fi
     ])
if test "x$test_gstreamer" = "xtrue"; then
	dnl	PKG_CHECK_MODULES(GTK,[gtk+-2.0 >= 2.6.0], test_gstreamer=true, test_gstreamer=false)
	if test "x$test_gtk" = "xfalse"; then
	   AC_MSG_ERROR(--with-gstreamer requires --with-gtk);
	fi
	PKG_CHECK_MODULES(GSTREAMER, gstreamer-0.10 >= 0.10.0, test_gstreamer=true, test_gstreamer=false)
	AC_SUBST(GSTREAMER_LIBS)
	AC_SUBST(GSTREAMER_CFLAGS)

 fi

AM_CONDITIONAL(WITH_GSTREAMER, test x$test_gstreamer = xtrue)
AC_MSG_RESULT($test_gstreamer)

dnl Testing for ffmpeg
dnl ======================================================================
##
# Find ffmpeg audio/video codec library
#
AC_MSG_CHECKING(for ffmpeg audio/video codec library)
test_ffmpeg=false
AC_ARG_WITH(ffmpeg,
	AS_HELP_STRING([--with-ffmpeg], [Turn on/off use of Ffmpeg audio/video codec library]),
	[
	case "${withval}" in    
	yes)
		test_ffmpeg=true
		;;
	no)
		test_ffmpeg=false
		;;
	*)
		AC_MSG_ERROR("Cannot supply value to --with-ffmpeg")
		;;
	esac],
	[test_ffmpeg=true])

if $test_ffmpeg; then
	PKG_CHECK_MODULES(LIBAVUTIL,[libavutil >= 49.7.0], , test_ffmpeg=false)
	PKG_CHECK_MODULES(LIBAVCODEC,[libavcodec >= 51.57.2], , test_ffmpeg=false)
	PKG_CHECK_MODULES(LIBAVFORMAT,[libavformat >= 52.14.0], , test_ffmpeg=false)
	PKG_CHECK_MODULES(LIBSWRESAMPLE, [libswresample >= 0.17.102], , test_ffmpeg=false)
	PKG_CHECK_EXISTS([libswscale >= 0.7.1], test_ffmpeg_swscale=true ,test_ffmpeg_swscale=false)
	if $test_ffmpeg_swscale; then
		PKG_CHECK_MODULES(LIBSWSCALE, [libswscale >= 0.7.1], test_ffmpeg_swscale=true, test_ffmpeg_swscale=false)
	fi
	if $test_ffmpeg_swscale; then
		:
	else
		LIBSWSCALE_CFLAGS=""
		LIBSWSCALE_LIBS=""
	fi
	FFMPEG_CFLAGS="${LIBAVFORMAT_CFLAGS} ${LIBAVCODEC_CFLAGS} ${LIBAVUTIL_CFLAGS} ${LIBSWRESAMPLE_CFLAGS} ${LIBSWSCALE_CFLAGS}"
	FFMPEG_LIBS="${LIBAVFORMAT_LIBS} ${LIBAVCODEC_LIBS} ${LIBAVUTIL_LIBS} ${LIBSWRESAMPLE_LIBS} ${LIBSWSCALE_LIBS}"
fi
AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(FFMPEG_LIBS)
AM_CONDITIONAL(WITH_FFMPEG, test x$test_ffmpeg = xtrue)
AM_CONDITIONAL(WITH_FFMPEG_SWSCALE, test x$test_ffmpeg_swscale = xtrue)
AC_SUBST(WITH_FFMPEG)
AC_MSG_RESULT($test_ffmpeg)

dnl Doxygen documentation?
AC_MSG_CHECKING(for --with-doxygen)
AC_ARG_WITH(doxygen,
	AS_HELP_STRING([--with-doxygen], [Build doxygen API documentation]),
	[
		case "${withval}" in
		yes)
			DOXYGEN=`which doxygen`
			with_doxygen=true
			;;
		no)
			DOXYGEN=:
			with_doxygen=false
			;;
		*)
			DOXYGEN=`which ${withval}`
			with_doxygen=true
			;;
		esac],
	[
		if doxygen --version >/dev/null; then
			with_doxygen=true
			DOXYGEN=`which doxygen`
		else
			with_doxygen=false
			DOXYGEN=:
		fi
	])
AM_CONDITIONAL(WITH_DOXYGEN, test x$with_doxygen = xtrue)
AC_SUBST(DOXYGEN)
AC_MSG_RESULT($with_doxygen - $DOXYGEN)

dnl Testing for Python
dnl ======================================================================
AC_MSG_CHECKING(whether to build Python interface)
AC_ARG_WITH(python,
    AS_HELP_STRING([--with-python], [Build Python interface to Ambulant]),
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
AC_MSG_RESULT($with_python - $PYTHON)
if test x$with_python = xtrue; then
	pythonprefix=`$PYTHON -c 'import sys; print sys.prefix'`
	if test ! -w $pythonprefix; then
		AC_MSG_WARN([Python installation not writeable, install may fail])
		python_warning="
You appear to have no permission to write into your Python installation.
This may mean 'make install' will fail, use 'make install -k' and manually
install the ambulant python module."
	fi
fi

# Do we want python plugins ?
#

AC_MSG_CHECKING(for --with-python-plugin)
AC_ARG_WITH([python-plugin],
    AS_HELP_STRING([--with-python-plugin],[Enable the python plugin (default: disabled)]),
    [
		case "${withval}" in
		yes)  	with_python_plugin=true
				;;
		no)		with_python_plugin=false
				;;
		*)		AC_MSG_ERROR("--with-python-plugin takes no argument")
				;;
		esac],
	[with_python_plugin=false])
	
if test x$with_python_plugin = xtrue;
then
	if test x$with_python = xfalse;
	then
		AC_MSG_ERROR("--with-python-plugin requires --with-python")
		with_python_plugin=false
	elif ${PYTHON}-config --prefix > /dev/null 2>&1; then
		# If we have python-config for the selected python we use that to obtain
		# the parameters we need
		PYTHON_INCLUDES=`python-config --includes`
		PYTHON_LIBS=`python-config --ldflags`
	else
		# Otherwise we use the distutils commands		
		PYTHON_INCLUDES=-I`$PYTHON -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc()"`
		# Trick to determine whether we're using a Mac framework build, in which
		# case we need to specify the library differently.
		case x$PYTHON_INCLUDES in
		x-I/Library*)
		   PYTHON_LIBS="-framework Python" 
		   ;;
		x-I/System/Library*)
		   PYTHON_LIBS="-F/System/Library/Frameworks -framework Python"
		   ;;
		*)
		   pylibdir=`$PYTHON -c 'from distutils.sysconfig import *; print parse_makefile(get_makefile_filename()).get("LIBPL")'`
		   pyversion=`$PYTHON -c 'import distutils.sysconfig ; print distutils.sysconfig.get_python_version()'`
		   PYTHON_LIBS="-L${libdir} -L${pylibdir} -lpython${pyversion}"
		   ;;
		esac
	fi
fi

AC_SUBST(PYTHON_INCLUDES)
AC_SUBST(PYTHON_LIBS)
AM_CONDITIONAL(WITH_PYTHON_PLUGIN, test x$with_python_plugin = xtrue)
if test x$with_python_plugin = xtrue ; then
    AC_DEFINE(WITH_PYTHON_PLUGIN, 1, [Define to enable support for plugins in Python])
fi
AC_MSG_RESULT($with_python_plugin)

# Do we want the npambulant browser (firefox) plugin ?

AC_MSG_CHECKING(for --with-npambulant)
NPAMBULANT_INCLUDES=
AC_ARG_WITH([npambulant],
    AS_HELP_STRING([--with-npambulant],[Enable building the Ambulant plugin for npapi compatible browsers, like firefox (default: disabled)]),
	[
		case "${withval}" in
		yes)
			with_npambulant=true
			# Determine where the include files should come from, as it hasn't been passed
			# as an option.
			# Options tested, in order:
			# - From a copy of the xulrunner distribution in third_party_packages. This
			#   is used when building binary standalone plugins.
			# - From a firefox-dev debian package (or anything else) installed
			#   system-wide.
			# - From a xulrunner distribution installed system-wide.
			if test -f $third_party_packages/xulrunner-sdk/include/npapi.h; then
				NPAMBULANT_INCLUDES=-I$third_party_packages/xulrunner-sdk/include
			else
				PKG_CHECK_MODULES(mozilla_plugin, [mozilla-plugin >= 8.0],[
					NPAMBULANT_INCLUDES=${mozilla_plugin_INCLUDES}
				],[
					AC_MSG_ERROR("npapi include directory not specified and cannot find it")
					with_npambulant=false
				])
			fi
			;;
		no)
			with_npambulant=false
			;;
		*)
			NPAMBULANT_INCLUDES=-I${withval}
			with_npambulant=true
			;;
		esac
		if  test x$with_npambulant = xtrue -a $test_gtk = "gtk3"; then
		   AC_MSG_ERROR("--with-npambulant is not yet available for gtk3. Please use --with-gtk=gtk2.")
		   with_npambulant=false
		fi
	],
	[with_npambulant=false])

AC_SUBST(NPAMBULANT_INCLUDES)
AM_CONDITIONAL(WITH_NPAMBULANT, test x$with_npambulant = xtrue)
AC_MSG_RESULT($with_npambulant)

dnl
dnl Check for libxml2
dnl
test_libxml2=false
libxml2_config=false
AC_MSG_CHECKING(for libxml2 XML handling library)
AC_ARG_WITH(libxml2,
	AS_HELP_STRING([--with-libxml2], [Turn on/off use of libxml2 XML library]),
	[
		case "${withval}" in
		yes)
			test_libxml2=true
			libxml2_config=true
			;;
		no)
			;;
		*)
			LIBXML2_PREFIX=`cd ${withval} ; pwd`
			LIBXML2_LIBS="-L$LIBXML2_PREFIX/lib -llibxml2"
			LIBXML2_CFLAGS="-I$LIBXML2_PREFIX/include"
			test_libxml2=true
			;;
		esac], 
	[libxml2_config=true])

if test $libxml2_config = true && xml2-config --version > /dev/null; then
	LIBXML2_CFLAGS=`xml2-config --cflags`
	LIBXML2_LIBS=`xml2-config --libs`
	test_libxml2=true
else
	if test $libxml2_config = false; then
	   	libxml2_warning="libxml2 not configured. State plugin will not be built."
	else	libxml2_warning="command xml2-config not found. State plugin cannot be built."
	fi
	LIBXML2_CFLAGS=
	LIBXML2_LIBS=
fi

if $test_libxml2; then
	AC_DEFINE(WITH_LIBXML2, 1, [Define to enable libxml2 XML parser])
fi
AC_SUBST(LIBXML2_LIBS)
AC_SUBST(LIBXML2_CFLAGS)
AM_CONDITIONAL(WITH_LIBXML2, test x$test_libxml2 = xtrue)
AC_MSG_RESULT($test_libxml2)

#
# Nowadays, we always build with linking to the shared Ambulant library.
#
AC_MSG_CHECKING([whether we should link against core ambulant shared lib (alternative is static)])
with_ambulant_shared_lib=true
AM_CONDITIONAL(WITH_AMBULANT_SHARED_LIB, $with_ambulant_shared_lib)
AC_MSG_RESULT($with_ambulant_shared_lib)

AC_MSG_CHECKING(for --with-webkit-dom)
AC_ARG_WITH([webkit-dom],
	AS_HELP_STRING([--with-webkit-dom],[Enable the WebKit DOM plugin (default: disabled)]),
	[
		case "${withval}" in
		yes)	
			with_webkit_dom=true
			;;
		no)		
			with-webkit_dom=false
			;;
		*)		
			AC_MSG_ERROR("--with-webkit-dom takes no argument")
			;;
		esac],
	[with_webkit_dom=false])
AM_CONDITIONAL(WITH_WEBKIT_DOM, test x$with_webkit_dom = xtrue)
AC_MSG_RESULT($with_webkit_dom)
#
# Test for pthreads. Modifed from http://www.gnu.org/software/ac-archive/htmldoc/acx_pthread.html
#
AC_DEFUN([ACX_PTHREAD], [
#AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_PUSH([C])
acx_pthread_ok=no

# We used to check for pthread.h first, but this fails if pthread.h
# requires special compiler flags (e.g. on True64 or Sequent).
# It gets checked for in the link test anyway.

# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works using
# them:
if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
        AC_TRY_LINK_FUNC(pthread_join, acx_pthread_ok=yes)
        AC_MSG_RESULT($acx_pthread_ok)
        if test x"$acx_pthread_ok" = xno; then
                PTHREAD_LIBS=""
                PTHREAD_CFLAGS=""
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
fi

# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).

# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all.

acx_pthread_flags="pthreads none -Kthread -kthread lthread pthread -pthread -pthreads -mthreads --thread-safe -mt"

# Note: Jack has moved pthread forward, otherwise debian builds failed.

# The ordering *is* (sometimes) important.  Some notes on the
# individual items follow:

# pthreads: AIX (must check this before -lpthread)
# none: in case threads are in libc; should be tried before -Kthread and
#       other compiler flags to prevent continual compiler warnings
# -Kthread: Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread: FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread: LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread: Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads: Solaris/gcc
# -mthreads: Mingw32/gcc, Lynx/gcc
# -mt: Sun Workshop C (may only link SunOS threads [-lthread], but it
#      doesn't hurt to check since this sometimes defines pthreads too;
#      also defines -D_REENTRANT)
# pthread: Linux, etcetera
# --thread-safe: KAI C++

case "${host_cpu}-${host_os}" in
        *solaris*)

        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthread or
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:

        acx_pthread_flags="-pthread -pthreads pthread -mt $acx_pthread_flags"
        ;;
esac

if test x"$acx_pthread_ok" = xno; then
for flag in $acx_pthread_flags; do

        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;

                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;

                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>]], [[pthread_t th; pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ]])],[acx_pthread_ok=yes],[])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        AC_MSG_RESULT($acx_pthread_ok)
        if test "x$acx_pthread_ok" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
done
fi

# Various other checks:
if test "x$acx_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Detect AIX lossage: threads are created detached by default
        # and the JOINABLE attribute has a nonstandard name (UNDETACHED).
        AC_MSG_CHECKING([for joinable pthread attribute])
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>]], [[int attr=PTHREAD_CREATE_JOINABLE;]])],[ok=PTHREAD_CREATE_JOINABLE],[ok=unknown])
        if test x"$ok" = xunknown; then
                AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>]], [[int attr=PTHREAD_CREATE_UNDETACHED;]])],[ok=PTHREAD_CREATE_UNDETACHED],[ok=unknown])
        fi
        if test x"$ok" != xPTHREAD_CREATE_JOINABLE; then
                AC_DEFINE(PTHREAD_CREATE_JOINABLE, $ok,
                          [Define to the necessary symbol if this constant
                           uses a non-standard name on your system.])
        fi
        AC_MSG_RESULT(${ok})
        if test x"$ok" = xunknown; then
                AC_MSG_WARN([we do not know how to create joinable pthreads])
        fi

        AC_MSG_CHECKING([if more special flags are required for pthreads])
        flag=no
        case "${host_cpu}-${host_os}" in
                *-aix* | *-freebsd*)     flag="-D_THREAD_SAFE";;
                *solaris* | *-osf* | *-hpux*) flag="-D_REENTRANT";;
        esac
        AC_MSG_RESULT(${flag})
        if test "x$flag" != xno; then
                PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
        fi

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        # More AIX lossage: must compile with cc_r
        AC_CHECK_PROG(PTHREAD_CC, cc_r, cc_r, ${CC})
else
        PTHREAD_CC="$CC"
fi

AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_CC)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_pthread_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.]),[$1])
        :
else
        acx_pthread_ok=no
        $2
fi
AC_LANG_POP([])
])dnl ACX_PTHREAD

ACX_PTHREAD
CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
AC_CONFIG_FILES([
libambulant.pc
ambulant.spec
src/libambulant/pc-uninstalled/libambulant.pc
])
AC_OUTPUT

dnl ============================================================================

echo "Attempting to fix libtool, assuming 2.2.6"
sh $srcdir/scripts/fixlibtool226.sh

echo "
ambulant-$AMBULANT_VERSION will be built with following parameters:

    targeted prefix:                ${prefix}
    third party packages:           ${third_party_packages}
    compiler:                       ${CC}

    Enable overlay window:          ${test_overlay}
    Build with Python support:      ${with_python}
    Build with Python plugin:       ${with_python_plugin}

    Use expat XML library:          ${test_expat}
    Use xerces XML library:         ${test_xerces}
    Enable Xerces as plugin:        ${xerces_plugin}
    Use libxml2 XML library:        ${test_libxml2}
    Build with ffmpeg:              ${test_ffmpeg}
    Build with SDL version:         ${sdl_version}
    Build with SDL audio:           ${test_sdl_audio}
    Build with SDL2 video:          ${test_sdl_video}
    Build with SDL2_image:          ${test_sdl_image}
    Build with SDL_Pango:           ${test_sdl_pango}
    Build with SDL2_ttf:            ${test_sdl_ttf}
    Build SDL frontend:             ${with_sdl_frontend}
    Build with gstreamer:           ${test_gstreamer}

    Build with GTK:                 ${test_gtk}

    Build with WebKit DOM plugin:   ${with_webkit_dom}
    Build CG frontend:              ${cg}
    Build UIKit frontend:           ${uikit}
    Build OSX 10.4 universal:       ${with_universalsdk}
    Build OSX 10.6 universal:       ${with_macfat}

    Build with npambulant plugin:   ${with_npambulant}
    Use shared libs internally:     ${with_ambulant_shared_lib}
    Build with GCD:                 ${with_gcd_event_processor}
    Build with external clock sync: ${with_external_clock_sync}
     "

if test "x$with_frontend" = "xfalse"; then
    AC_MSG_WARN([No GUI support, so no player will be built])
fi
AC_MSG_RESULT([$ffmpeg_warning]);
AC_MSG_RESULT([$expat_warning]);
AC_MSG_RESULT([$libxml2_warning]);
AC_MSG_RESULT([$python_warning]);
AC_MSG_RESULT([$xml_parser_warning]);
if test "x$test_ffmpeg_swscale" = "xtrue"; then
	AC_MSG_WARN([Built with ffmpeg libswscale, therefore under GPL license])
fi

