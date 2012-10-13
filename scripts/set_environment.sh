#/bin/sh
# exports environment variables ARCHS, ARCH_ARGS, PLATFORM_PATH, SDK_PATH and XCODE_SDK_BASE for use in  MacOSX/iOS build scripts
# usage: set_environment <Platform> <version> e.g.: . set_environment iPphoneOS 6.0
# known bugs: no error checking (e.g. requested SDK or information in SDK not available)
# set -x

# set default _PLATFORM, _VERSION
_PLATFORM=iPhoneOS
_VERSION=5.1

# get _PLATFORM, _VERSION from arguments, if any
if [ $# -gt 0 ] ; then _PLATFORM=$1; shift; fi
if [ $# -gt 0 ] ; then _VERSION=$1; shift; fi

# get PLATFORM_PATH and SDK_PATH for the selected _PLATFORM, _VERSION
_LC_PLATFORM=`echo $_PLATFORM | tr '[:upper:]' '[:lower:]'`
PLATFORM_PATH=`xcodebuild -version -sdk "$_LC_PLATFORM"$_VERSION PlatformPath`
SDK_PATH=`xcodebuild -version -sdk "$_LC_PLATFORM"$_VERSION Path`

# get the supported processor architectures for this  _PLATFORM, _VERSION in ARCHS
ARCHS=""
case $_PLATFORM in 
MacOSX)		case $_VERSION in 
		10.5) ARCHS="ppc i386 x86_64"	;;
		10.*) ARCHS="i386 x86_64"	;;
		esac
		;;
iPhoneOS)  	case $_VERSION in 
			4.*) ARCHS="armv6 armv7" ;;
			5.*) ARCHS="armv6 armv7" ;;
			6.*) ARCHS="armv7 armv7s" ;;
		esac
		;;
iPhoneSimulator)
		ARCHS="i386"
		;;
esac

# echo "Found archs: "  `lipo -info $SDK_PATH/usr/lib/crt1.o|awk -F ": " '{ print $3 }'`
# echo "Using archs: " $ARCHS

# construct compiler/linker '-arch' flags in ARCH_ARGS 
ARCH_ARGS=""
for _arch in $ARCHS 
do 
	if [ "$ARCH_ARGS" = "" ]
	then ARCH_ARGS="-arch $_arch"
	else ARCH_ARGS="$ARCH_ARGS -arch $_arch"
	fi
done

# Starting from Xcode 4.0, all Developer extras are not in '/Developer/...' anymore,
#  but in '/Applications/Xcode/Contents/Developer/...'. We use XCODE_SDK_BASE to reflect this.
XCODE_SDK_BASE=""
if [ `xcodebuild -version|grep Xcode|awk '{print $2; exit}' | cut -c 1` -ge 4 ] ; 
then
	XCODE_SDK_BASE="/Applications/Xcode.app/Contents"
fi
# According to WWDC 2012 Building from the Command Line with Xcode (https://developer.apple.com/videos/wwdc/2012/?id=404),
# before calling xcode build tools, the following environment variable should have the proper value
DEVELOPER_DIR=$XCODE_SDK_BASE/Developer

# export results
export ARCHS ARCH_ARGS DEVELOPER_DIR PLATFORM_PATH SDK_PATH XCODE_SDK_BASE

# clear temporary environment variables
unset _PLATFORM _LC_PLATFORM _VERSION _XCODE_MAJOR_VERSION _arch 
# set +x
