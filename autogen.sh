#!/bin/sh
# Run this to generate all the initial makefiles, etc.

#name of package
PKG_NAME="ambulant"
srcdir=${srcdir:-.}

# default version requirements ...
REQUIRED_AUTOCONF_VERSION=${REQUIRED_AUTOCONF_VERSION:-2.68}
REQUIRED_AUTOMAKE_VERSION=${REQUIRED_AUTOMAKE_VERSION:-1.11}
REQUIRED_LIBTOOL_VERSION=${REQUIRED_LIBTOOL_VERSION:-2.2.6}
REQUIRED_PKG_CONFIG_VERSION=${REQUIRED_PKG_CONFIG_VERSION:-0.21}
REQUIRED_GETTEXT_VERSION=${REQUIRED_GETTEXT_VERSION:-0.18.1}

# a list of required m4 macros.  Package can set an initial value
REQUIRED_M4MACROS=${REQUIRED_M4MACROS:-}
FORBIDDEN_M4MACROS=${FORBIDDEN_M4MACROS:-}

if test -d /usr/local/share/aclocal; then
    ACLOCAL_FLAGS="-I /usr/local/share/aclocal $ACLOCAL_FLAG"
fi


# some terminal codes ...
boldface="`tput bold 2>/dev/null`"
normal="`tput sgr0 2>/dev/null`"
printbold() {
    echo -n "$boldface"
    echo "$@"
    echo -n "$normal"
}    
printerr() {
    echo "$@" >&2
}

# Usage:
#     compare_versions MIN_VERSION ACTUAL_VERSION
# returns true if ACTUAL_VERSION >= MIN_VERSION
compare_versions() {
    local min_version actual_version status save_IFS cur min
    min_version=$1
    actual_version=$2
    status=0
    IFS="${IFS=         }"; save_IFS="$IFS"; IFS="."
    set $actual_version
    for min in $min_version; do
        cur=`echo $1 | sed 's/[^0-9].*$//'`; shift # remove letter suffixes
        if [ -z "$min" ]; then break; fi
        if [ -z "$cur" ]; then status=1; break; fi
        if [ $cur -gt $min ]; then break; fi
        if [ $cur -lt $min ]; then status=1; break; fi
    done
    IFS="$save_IFS"
    return $status
}

# Usage:
#     version_check PACKAGE VARIABLE CHECKPROGS MIN_VERSION SOURCE
# checks to see if the package is available
version_check() {
    local package variable checkprogs min_version source status checkprog actual_version
    package=$1
    variable=$2
    checkprogs=$3
    min_version=$4
    source=$5
    status=1

    checkprog=`eval echo "\\$$variable"`
    if [ -n "$checkprog" ]; then
	printbold "using $checkprog for $package"
	return 0
    fi

    printbold "checking for $package >= $min_version..."
    for checkprog in $checkprogs; do
	echo -n "  testing $checkprog... "
	if $checkprog --version < /dev/null > /dev/null 2>&1; then
	    actual_version=`$checkprog --version | head -n 1 | \
                               sed 's/^.*[ 	]\([0-9.]*[a-z]*\).*$/\1/'`
	    if compare_versions $min_version $actual_version; then
		echo "found."
		# set variable
		eval "$variable=$checkprog"
		status=0
		break
	    else
		echo "too old (found version $actual_version)"
	    fi
	else
	    echo "not found."
	fi
    done
    if [ "$status" != 0 ]; then
	printerr "***Error***: You must have $package >= $min_version installed"
	printerr "  to build $PKG_NAME.  Download the appropriate package for"
	printerr "  from your distribution or get the source tarball at"
        printerr "    $source"
	printerr
    fi
    return $status
}

# Usage:
#     require_m4macro command filename.m4
# test if filename.m4 exists relative to command's search path
# if so, adds filename.m4 to the list of required macros
require_m4macro() {
#   set -x
    M4DIR=`which $1 | sed -e "s@bin/$1@share/aclocal@"`
    M4FILE="$M4DIR/$2"
    if [ ! -e $M4FILE ]  
	then printerr "***Error***:  $M4FILE not found"
	exit 1
    fi
    NEW_ACLOCAL_FLAG="-I$M4DIR"
    # look if $NEW_ACLOCAL_FLAG is already present
    aclocal_flag_found=false
    for aclocal_flag in $ACLOCAL_FLAGS; do
	if [ $aclocal_flag = $NEW_ACLOCAL_FLAG ]; then
		aclocal_flag_found=true
		break
	fi
    done
    if $aclocal_flag_found; then :
    else ACLOCAL_FLAGS="$ACLOCAL_FLAGS $NEW_ACLOCAL_FLAG"
    fi 
 #  set +x
    case "$REQUIRED_M4MACROS" in
	$2\ * | *\ $2\ * | *\ $2) ;;
	*) REQUIRED_M4MACROS="$REQUIRED_M4MACROS $2" ;;
    esac
}

forbid_m4macro() {
    case "$FORBIDDEN_M4MACROS" in
	$1\ * | *\ $1\ * | *\ $1) ;;
	*) FORBIDDEN_M4MACROS="$FORBIDDEN_M4MACROS $1" ;;
    esac
}

# Usage:
#     check_m4macros
# Checks that all the requested macro files are in the aclocal macro path
# Uses REQUIRED_M4MACROS and ACLOCAL variables.
check_m4macros() {
    local macrodirs status macro dir macrofound

    # construct list of macro directories
    macrodirs="`$ACLOCAL --print-ac-dir`"
    macrodirs="$macrodirs"
    set - $ACLOCAL_FLAGS
    while [ $# -gt 0 ]; do
	if [ "$1" = "-I" ]; then
	    macrodirs="$macrodirs $2"
	    shift
	fi
	shift
    done

    status=0
    if [ -n "$REQUIRED_M4MACROS" ]; then
	printbold "Checking for required M4 macros..."
	# check that each macro file is in one of the macro dirs
	for macro in $REQUIRED_M4MACROS; do
	    macrofound=false
	    for dir in $macrodirs; do
		if [ -f "$dir/$macro" ]; then
		    macrofound=true
		    break
		fi
	    done
	    if $macrofound; then
		:
	    else
		printerr "  $macro not found"
		status=1
	    fi
	done
    fi
    status=0
    if [ -n "$FORBIDDEN_M4MACROS" ]; then
	printbold "Checking for forbidden M4 macros..."
	# check that each macro file is in one of the macro dirs
	for macro in $FORBIDDEN_M4MACROS; do
	    macrofound=false
	    for dir in $macrodirs; do
		if [ -f "$dir/$macro" ]; then
		    macrofound=true
		    break
		fi
	    done
	    if $macrofound; then
		printerr "  $macro found (should be cleared from macros dir)"
		status=1
	    fi
	done
    fi
    if [ "$status" != 0 ]; then
	printerr "***Error***: some autoconf macros required to build $PKG_NAME"
	printerr "  were not found in your aclocal path, or some forbidden"
	printerr "  macros were found.  Perhaps you need to adjust your"
	printerr "  ACLOCAL_PATH?"
	printerr
    fi
    return $status
}

# try to catch the case where the macros2/ directory hasn't been cleared out.
forbid_m4macro gnome-cxx-check.m4

want_libtool=false
want_gettext=false
want_glib_gettext=false
want_intltool=false
want_pkg_config=false
want_gtk_doc=false

#configure_files="`find $srcdir -name configure.ac -print -or -name configure.ac -print`"
configure_files=configure.ac

for configure_in in $configure_files; do
    if grep "^A[CM]_PROG_LIBTOOL" $configure_in >/dev/null; then
	want_libtool=true
    fi
    if grep "^AM_GNU_GETTEXT" $configure_in >/dev/null; then
	want_gettext=true
    fi
    if grep "^AC_PROG_INTLTOOL" $configure_in >/dev/null; then
	want_intltool=true
    fi
    if grep "^PKG_CHECK_MODULES" $configure_in >/dev/null; then
	want_pkg_config=true
    fi
done

DIE=0

#tell Mandrake autoconf wrapper we want autoconf 2.5x, not 2.13
WANT_AUTOCONF_2_5=1
export WANT_AUTOCONF_2_5
version_check autoconf AUTOCONF 'autoconf2.50 autoconf autoconf-2.53' $REQUIRED_AUTOCONF_VERSION \
    "http://ftp.gnu.org/pub/gnu/autoconf/autoconf-$REQUIRED_AUTOCONF_VERSION.tar.gz" || DIE=1
AUTOHEADER=`echo $AUTOCONF | sed s/autoconf/autoheader/`

case $REQUIRED_AUTOMAKE_VERSION in
    1.4*) automake_progs="automake-1.4" ;;
    1.5*) automake_progs="automake-1.10 automake-1.9 automake-1.8 automake-1.7 automake-1.6 automake-1.5" ;;
    1.6*) automake_progs="automake-1.10 automake-1.9 automake-1.8 automake-1.7 automake-1.6" ;;
    1.7*) automake_progs="automake-1.10 automake-1.9 automake-1.8 automake-1.7" ;;
    1.8*) automake_progs="automake-1.10 automake-1.9 automake-1.8" ;;
    1.9*) automake_progs="automake-1.10 automake-1.9" ;;
    1.10*) automake_progs="automake-1.10 automake" ;;
    1.11*) automake_progs="automake-1.13 automake-1.11 automake" ;;
    1.13*) automake_progs="automake-1.13 automake" ;;
esac
version_check automake AUTOMAKE "$automake_progs" $REQUIRED_AUTOMAKE_VERSION \
    "http://ftp.gnu.org/pub/gnu/automake/automake-$REQUIRED_AUTOMAKE_VERSION.tar.gz" || DIE=1
ACLOCAL=`echo $AUTOMAKE | sed s/automake/aclocal/`

if $want_libtool; then
    version_check libtool LIBTOOLIZE 'libtoolize glibtoolize' $REQUIRED_LIBTOOL_VERSION \
        "http://ftp.gnu.org/pub/gnu/libtool/libtool-$REQUIRED_LIBTOOL_VERSION.tar.gz" || DIE=1
    require_m4macro $LIBTOOLIZE libtool.m4
	require_m4macro $LIBTOOLIZE ltdl.m4
fi

if $want_gettext; then
    version_check gettext GETTEXTIZE gettextize $REQUIRED_GETTEXT_VERSION \
        "http://ftp.gnu.org/pub/gnu/gettext/gettext-$REQUIRED_GETTEXT_VERSION.tar.gz" || DIE=1
    require_m4macro $GETTEXTIZE gettext.m4
fi

if $want_pkg_config; then
    version_check pkg-config PKG_CONFIG pkg-config $REQUIRED_PKG_CONFIG_VERSION \
        "'http://www.freedesktop.org/software/pkgconfig/releases/pkgconfig-$REQUIRED_PKG_CONFIG_VERSION.tar.gz" || DIE=1
    require_m4macro $PKG_CONFIG pkg.m4
fi


check_m4macros || DIE=1

if [ "$DIE" -eq 1 ]; then
  exit 1
fi

##if test -z "$*"; then
##  printerr "**Warning**: I am going to run \`configure' with no arguments."
##  printerr "If you wish to pass any to it, please specify them on the"
##  printerr \`$0\'" command line."
##  printerr
##fi

topdir=`pwd`
for configure_in in $configure_files; do 
    dirname=`dirname $configure_in`
    basename=`basename $configure_in`
    if test -f $dirname/NO-AUTO-GEN; then
	echo skipping $dirname -- flagged as no auto-gen
    else
	printbold "Processing $configure_in"

	if grep "^A[CM]_PROG_LIBTOOL" $basename >/dev/null; then
	    printbold "Running $LIBTOOLIZE..."
	    $LIBTOOLIZE --force || exit 1
	fi

	aclocalinclude="$ACLOCAL_FLAGS"
	printbold "Running $ACLOCAL..."
	$ACLOCAL $aclocalinclude || exit 1

	if grep "GNOME_AUTOGEN_OBSOLETE" aclocal.m4 >/dev/null; then
	    printerr "*** obsolete gnome macros were used in $configure_in"
	fi

	if grep "^AC_PROG_INTLTOOL" $basename >/dev/null; then
	    printbold "Running $INTLTOOLIZE..."
	    $INTLTOOLIZE --force --automake || exit 1
	fi
	if grep "^GTK_DOC_CHECK" $basename >/dev/null; then
	    printbold "Running $GTKDOCIZE..."
	    $GTKDOCIZE || exit 1
	fi
	if grep "^A[CM]_CONFIG_HEADER" $basename >/dev/null; then
	    printbold "Running $AUTOHEADER..."
	    $AUTOHEADER || exit 1
	fi

	# Fix for an error I don't understand, based on info from
	# <http://ramblingfoo.blogspot.com/2007/07/required-file-configrpath-not-found.html>
	
	touch libltdl/config/config.rpath
	
	printbold "Running $AUTOMAKE..."
	$AUTOMAKE --gnu --add-missing  || exit 1

	printbold "Running $AUTOCONF..."
	$AUTOCONF || exit 1

	cd $topdir
    fi
done

conf_flags="--enable-maintainer-mode --enable-compile-warnings"

NOCONFIGURE=true

if test x$NOCONFIGURE = x; then
    printbold Running $srcdir/configure $conf_flags "$@" ...
    $srcdir/configure $conf_flags "$@" \
	&& echo Now type \`make\' to compile $PKG_NAME || exit 1
else
    echo Skipping configure process.
fi
