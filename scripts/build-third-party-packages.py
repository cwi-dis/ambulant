import sys
import os
import subprocess
import urllib
import urlparse
import posixpath
import platform
from optparse import OptionParser

NOCHECK=False
NORUN=False
TRYMIRROR=True
#
# Where the mirrored copies of the Ambulant 3rd party packages for this release live.
# Before cutting a release, update this directory name, and run
#   python build-third-party-packages.py -m
# in the directory on the server.
MIRRORBASE="http://www.ambulantplayer.org/thirdpartymirror/2.3/"
LIVE_MIRRORDATE="2012.02.29"
SDL_MIRRORDATE="20120306"

#
# Path names for Windows programs and such
#
WINDOWS_PROGRAMFILES=os.getenv("ProgramFiles", "c:\Program Files")
WINDOWS_PROGRAMFILES32=os.getenv("ProgramFiles(x86)", "")
WINDOWS_UNTAR_PATH="%s\\7-Zip\\7z.exe" % WINDOWS_PROGRAMFILES
if not os.path.exists(WINDOWS_UNTAR_PATH) and os.path.exists(WINDOWS_PROGRAMFILES32):
    WINDOWS_UNTAR_PATH="%s\\7-Zip\\7z.exe" % WINDOWS_PROGRAMFILES32
WINDOWS_UNTAR='"%s" x -y' % WINDOWS_UNTAR_PATH
WINDOWS_UNZIP_PATH=WINDOWS_UNTAR_PATH
WINDOWS_UNZIP=WINDOWS_UNTAR
WINDOWS_DXSDK_PATH="%s\\Microsoft DirectX SDK (February 2010)" % WINDOWS_PROGRAMFILES
if not os.path.exists(WINDOWS_DXSDK_PATH) and os.path.exists(WINDOWS_PROGRAMFILES32):
    WINDOWS_DXSDK_PATH="%s\\Microsoft DirectX SDK (February 2010)" % WINDOWS_PROGRAMFILES32
WINDOWS_DXSDK='"%s"' % WINDOWS_DXSDK_PATH

XULRUNNER_URL="http://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/8.0/sdk/"
XULRUNNER_VERSION="xulrunner-8.0"

#
# FFMPEG GIT version-id for Windows, from ffmpeg.zeranoe.com
FFMPEG_WIN_GIT_ID="13f0cd6"
FFMPEG_WIN_GIT_DATE="20120927"

#
# urlretrieve silently ignores 404 errors. We want them, so we can download
# our shadow copies.
class MyURLOpener(urllib.FancyURLopener):
    def http_error_default(self, url, fp, errcode, errmsg, headers):
        """Default error handling -- raise an exception."""
        raise IOError, ('HTTP Error', errcode)

_urlopener=None

def myurlretrieve(url, filename=None, reporthook=None, data=None):
    global _urlopener
    if not _urlopener:
        _urlopener = MyURLOpener()
    return _urlopener.retrieve(url, filename, reporthook, data)

class CommonTPP:
    def __init__(self, name):
        self.name = name
        self.output = sys.stdout
        
    def _command(self, cmd, force=False):
        if not cmd:
            return True
        print >>self.output, "+ run:", cmd
        if NORUN and not force:
            print >>self.output, "+ dry run, skip execution"
            return True
        self.output.flush()
        sts = subprocess.call(cmd, shell=True, stdout=self.output, stderr=subprocess.STDOUT)
        print >>self.output, "+ run status:", sts
        return sts == 0

class TPP(CommonTPP):
    
    DEFAULT_BUILD_COMMAND="cd %s && ./configure && make && make install"
    DEFAULT_EXTRACT_COMMAND="tar xf %s"
    
    def __init__(self, name, url=None, url2=None, downloadedfile=None, extractcmd=None, checkcmd=None, buildcmd=None):
        CommonTPP.__init__(self, name)
        
        self.url = url
        self.url2 = url2
        if url and not downloadedfile:
            _, _, path, _, _, _ = urlparse.urlparse(url)
            downloadedfile = posixpath.basename(path)
        self.downloadedfile = downloadedfile
        self.checkcmd = checkcmd
        if not extractcmd:
            extractcmd = self.DEFAULT_EXTRACT_COMMAND % self.downloadedfile
        self.extractcmd = extractcmd
        self.buildcmd = buildcmd
        if not buildcmd:
            buildcmd = self.DEFAULT_BUILD_COMMAND
        self.buildcmd = buildcmd
        self.output = None
        
    def begin(self):
        self.output = open("log.%s.txt" % self.name, "w")
        
    def end(self):
        self.output.close()
        self.output = None
        
    def _command(self, cmd, force=False):
        if not cmd:
            return True
        print >>self.output, "+ run:", cmd
        if NORUN and not force:
            print >>self.output, "+ dry run, skip execution"
            return True
        self.output.flush()
        sts = subprocess.call(cmd, shell=True, stdout=self.output, stderr=subprocess.STDOUT)
        print >>self.output, "+ run status:", sts
        return sts == 0
        
    def check(self):
        if not self.checkcmd:
            print >>self.output, "+ skip availability check"
            return False
        if NOCHECK:
            print >>self.output, "+ dry run, pretend failure:", self.checkcmd
            return False
        return self._command(self.checkcmd, force=True)
        
    def download(self, trymirror=None):
        if trymirror is None:
            trymirror = TRYMIRROR

        if trymirror and self.url2:
            # Try the mirror
            print >>self.output, "+ mirror download:", MIRRORBASE+self.url2
            try:
                myurlretrieve(MIRRORBASE+self.url2, self.downloadedfile)
            except IOError, arg:
                print >>self.output, "+ download status: error:", arg
            else:
                print >>self.output, "+ download status: success"
                return True

        print >>self.output, "+ download:", self.url
        try:
            myurlretrieve(self.url, self.downloadedfile)
        except IOError, arg:
            print >>self.output, "+ download status: error:", arg
        else:
            print >>self.output, "+ download status: success"
            return True
            
    def extract(self):
        return self._command(self.extractcmd)
        
    def build(self):
        return self._command(self.buildcmd)
        
    def run(self):
        self.begin()
        ok = True
        if self.check():
            print >>self.output, "+ already installed"
            self.end()
            return True
        if self.url:
            ok = self.download()
            if ok:
                ok = self.extract()
        if ok:
            ok = self.build()
        if ok:
            print >>self.output, "+ installed"
        else:
            print >>self.output, "+ not installed"
        self.end()
        return ok
        
    def mirror(self):
        if not self.url2:
            return True
        self.downloadedfile = self.url2
        return self.download(trymirror=False)

class DebianTPP(CommonTPP):
    PACKAGE_INSTALL_CMD = "apt-get -y install %s"
    
    def run(self):
        return self._command(self.PACKAGE_INSTALL_CMD % self.name)
        
    def mirror(self):
        return True
    
class WinTPP(TPP):

    DEFAULT_BUILD_COMMAND=None
    DEFAULT_EXTRACT_COMMAND=WINDOWS_UNZIP + " %s"
    
def appendPath(varname, value):
    """Append a value to a colon-separated environment variable, if
    it isn't in there already.
    """
    oldvalue = os.getenv(varname)
    if not oldvalue:
        os.putenv(varname, value)
        return
    
    oldlist = oldvalue.split(':')
    if not value in oldlist:
        oldlist.append(value)
    os.putenv(varname, ':'.join(oldlist))

# If the environment variable AMBULANT_3PP has been set that is where we should
# build the third party packages
override_3pp = os.getenv("AMBULANT_3PP")
if override_3pp:
    if not os.path.exists(override_3pp):
        print '+ creating directory', override_3pp
        os.makedirs(override_3pp)
    os.chdir(override_3pp)
    print '+ building in', os.getcwd()
    
# Locate ambulant base directory
dir=os.getcwd()
while dir != '/':
    dir = os.path.dirname(dir)
    if os.path.exists(os.path.join(dir, 'configure.in')):
        break
if dir == '/':
    print 'ERROR: cannot find Ambulant toplevel directory'
    sys.exit(1)
print '+ Ambulant toplevel directory:', dir
AMBULANT_DIR=dir
COMMON_INSTALLDIR=os.path.join(os.getcwd(), "installed")
os.environ["AMBULANT_DIR"]=AMBULANT_DIR
XCODE_SDK_BASE=""
if platform.system() == "Darwin":
	# The set_environment.sh script finds the values we need and puts them in the environment of the subshell.
	# These then are available to 'set', but not to 'echo'. Therefore we use the former, and extract the desired value from its output.
	XCODE_SDK_BASE=os.popen('bash -c "(. $AMBULANT_DIR/scripts/set_environment.sh; set | grep -v BASH_EXECUTION_STRING | grep  XCODE_SDK_BASE)"').read().partition("=")[2][:-1]
# print "XCODE_SDK_BASE=", XCODE_SDK_BASE
#
# Common flags for MacOSX 10.4
#
MAC104_COMMON_CFLAGS="-arch i386 -arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk"
MAC104_COMMON_CONFIGURE="./configure --prefix='%s' CFLAGS='%s' CC=gcc-4.0 CXX=g++-4.0   " % (COMMON_INSTALLDIR, MAC104_COMMON_CFLAGS)

#
# Common flags for MacOSX 10.6
#
MAC106_COMMON_CFLAGS="-arch i386 -arch x86_64"
MAC106_COMMON_CONFIGURE="./configure --prefix='%s' CFLAGS='%s'  " % (COMMON_INSTALLDIR, MAC106_COMMON_CFLAGS)

#
# Common flags for iphone OS 4.3. Older (< 4.2) iPhone releases will not work, essential frameworks missing (ImageIO)
#

# Initial iPhone support for iPhoneOS 4.3, unstable, unfinished. Both for device and simulator; they use distinct development environments.
# for now (device): --prefix=installed/arm; export IPHONEOS_DEPLOYMENT_TARGET=4.3; export MACOSX_DEPLOYMENT_TARGET=10.6; PATH=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin:$PATH
# or (simulator): --prefix=installed/i386; export IPHONEOS_DEPLOYMENT_TARGET=4.3; export MACOSX_DEPLOYMENT_TARGET=10.6; PATH=/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin:$PATH
# other mods needed a couple of lines down
# 
# ffmpeg: http://lists.mplayerhq.hu/pipermail/ffmpeg-devel/2009-October/076618.html
# SDL: based on SDL-1.3.0-4429/Xcode-iOS/SDL/SDLiPhoneOS.xcodeproj
# live: use config.iphone42-Device/Simulator as in http://cache.gmane.org//gmane/comp/multimedia/live555/devel/5394-001.bin
##XXX IPHONE_DEVICE_COMMON_CONFIGURE="./configure --prefix='%s' --host=arm-apple-darwin10 CC=arm-apple-darwin10-gcc-4.2.1  CXX=arm-apple-darwin10-g++-4.2.1 LD=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/ld CPP=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/cpp CFLAGS=-isysroot\ /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS%s.sdk" % (COMMON_INSTALLDIR,  os.getenv("IPHONEOS_DEPLOYMENT_TARGET"))
#
# Starting from Xcode 4.0, all SDKs are stored relative to the Xcode Application.
# The script set_environment.sh gets the proper values for DEVELOPER_DIR, ARCHS, ARCH_ARGS, SDK_PATH etc.
IPHONE_DEVICE_COMMON_CFLAGS="$ARCH_ARGS -isysroot $SDK_PATH"
IPHONE_DEVICE_COMMON_CONFIGURE="./configure --host=arm-apple-darwin10 --prefix='%s' --disable-shared CFLAGS=\"%s\" CC=llvm-gcc-4.2 CXX=llvm-g++-4.2    " % (COMMON_INSTALLDIR, IPHONE_DEVICE_COMMON_CFLAGS)
##XXX IPHONE_SIMULATOR_COMMON_CONFIGURE="./configure --prefix='%s' --host=arm-apple-darwin10 CC=arm-apple-darwin10-gcc-4.2.1  CXX=arm-apple-darwin10-g++-4.2.1 LD=/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/ld CPP=/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin/cpp CFLAGS=-isysroot\ /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator%s.sdk" % (COMMON_INSTALLDIR,  os.getenv("IPHONEOS_DEPLOYMENT_TARGET"))
IPHONE_SIMULATOR_COMMON_CFLAGS="$ARCH_ARGS -isysroot $SDK_PATH" 
IPHONE_SIMULATOR_COMMON_CONFIGURE="CFLAGS=\"%s\" && ./configure --prefix='%s' CFLAGS=\"$CFLAGS\" CXXFLAGS=\"$CFLAGS\"  LDFLAGS=\"$CFLAGS\" " % (IPHONE_SIMULATOR_COMMON_CFLAGS, COMMON_INSTALLDIR)

#
# Common flags for Linux
#
LINUX_COMMON_CONFIGURE="./configure --prefix='%s'" % COMMON_INSTALLDIR

#
# Common Win32 flags
#
WIN32_COMMON_CONFIG="Release"
#WIN32_COMMON_CONFIG="Debug"
WIN32_VCVERSION="unknown"
WIN32_VSVERSION="unknown"

if os.path.sep == '/':
    #
    # Assume we are running on some Unix variant. Adapt PATH and PKG_CONFIG_PATH
    #
    appendPath("PATH", os.path.join(COMMON_INSTALLDIR, "bin"))
    appendPath("PKG_CONFIG_PATH", os.path.join(COMMON_INSTALLDIR, "lib/pkgconfig"))
else:
    #
    # Assume we are running on Windows. Check that vcvars32.bat has been run.
    #
    vsdir = os.getenv("DevEnvDir")
    if not vsdir:
        print "** This script needs the Visual Studio environment vars to be able to run"
        print '** Run "call ....Microsoft Visual Studio X.Y\\VC\\bin\\vcvars32.bat" from your VC9 dir first.'
        sys.exit(1)
    if '10.0' in vsdir:
        WIN32_VCVERSION="vc10"
        WIN32_VSVERSION="vs2010"
    elif '9.0' in vsdir:
        WIN32_VCVERSION="vc9"
        WIN32_VSVERSION="vs2008"
    else:
        print "** Unknown version of Visual Studio:", vsdir
        sysexit(1)
    #
    # There is a Visual Studio bug that temporary object files with pathnames > approx 200
    # characters are lost. This can happen with Xerces, which uses deep pathnames.
    # The number below is a bit of a guess, it may be off by one or two.
    if len(os.getcwd()) > 110:
        print "** The current directory (%s) has too long a pathname."
        print "** This will make the Xerces build hit a Visual Studio bug and fail"
        sys.exit(1)
    #
    # Check that the DirectShow baseclasses have been installled and built.
    #
    sdkdir = os.getenv("WindowsSdkDir")
    if not sdkdir:
        print "** WindowsSdkDir environment variable not set? Did you run vcvars32.bat?"
        sys.exit(1)
    baseclasses_dir = os.path.join(sdkdir, 'Samples\\multimedia\\directshow\\baseclasses')
    if not os.path.exists(baseclasses_dir):
        print '** Ambulant needs the DirectShow baseclasses,'
        print '** which are part of the MS Windows API installer.'
        print '** Expected them in', baseclasses_dir
        print '** Please install and build them (as Administrator, probably)'
        sys.exit(1)
    lib_r = os.path.join(baseclasses_dir, 'Release\\strmbase.lib')
    lib_d = os.path.join(baseclasses_dir, 'Debug\\strmbasd.lib')
    ok = True
    if not os.path.exists(lib_r):
        print '** Missing:', lib_r
        ok = False
    if not os.path.exists(lib_d):
        print '** Missing:', lib_d
        ok = False
    if not ok:
        sln = os.path.join(baseclasses_dir, 'baseclasses.sln')
        print '** DirectShow Baseclasses not built.'
        print '** Please open %s (as Administrator) and build Debug and Release targets'% sln
        sys.exit(1)
    
    
third_party_packages={
    'debian' : [
        DebianTPP("automake"),
        DebianTPP("autoconf"),
        DebianTPP("autotools-dev"),
        DebianTPP("gettext"),
        DebianTPP("libgtk2.0-dev"),
        DebianTPP("libgdk-pixbuf2.0-dev"),
        DebianTPP("libxml2-dev"),
        DebianTPP("libqt3-mt-dev"),
        DebianTPP("liblivemedia-dev"),
        DebianTPP("libltdl-dev"),
        DebianTPP("libsdl1.2-dev"),
        DebianTPP("libavformat-dev"),
        DebianTPP("libavcodec-dev"),
        DebianTPP("libswscale-dev"),
        DebianTPP("libxerces-c-dev"),
        DebianTPP("libexpat1-dev"),
        DebianTPP("python-dev"),
        DebianTPP("python-gtk2-dev"),
        DebianTPP("python-gobject-dev"),
    ],
 
    'mac10.6' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            url2="expat-2.0.1.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
                "cd expat-2.0.1 && "
                "patch --forward < %s/third_party_packages/expat.patch ; "
                "echo $PATH ; "
                "autoconf && "
                "%s && "
                "make ${MAKEFLAGS} && "
                "make install" % (AMBULANT_DIR, MAC106_COMMON_CONFIGURE)
            ),
        TPP("xerces-c",
            url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.tar.gz",
            url2="xerces-c-3.1.1.tar.gz",
            checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
            buildcmd=
                "cd xerces-c-3.1.1 && "
                "%s CXXFLAGS='%s' --disable-dependency-tracking && "
                "make ${MAKEFLAGS} && "
                "make install" % (MAC106_COMMON_CONFIGURE, MAC106_COMMON_CFLAGS)
            ),
        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            url2="faad2-2.7.tar.gz",
            checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make ${MAKEFLAGS} && "
                "make install" % MAC106_COMMON_CONFIGURE
            ),

        TPP("ffmpeg",
            url="http://ffmpeg.org/releases/ffmpeg-1.0.tar.gz",
            url2="ffmpeg-1.0.tar.gz",
            checkcmd="pkg-config --atleast-version=54.29.100 libavformat",
            buildcmd=
            	"rm -rf ffmpeg-1.0-universal && "
                "mkdir ffmpeg-1.0-universal && "
                "cd ffmpeg-1.0-universal && "
                "sh %s/scripts/ffmpeg-osx-fatbuild.sh %s/ffmpeg-1.0 all" % 
                    (AMBULANT_DIR, os.getcwd())
            ),
        TPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
#			url="http://www.ambulantplayer.org/thirdpartymirror/2.3/SDL-1.3-20110522.tar.gz",
            url2="SDL-1.3-%s.tar.gz"%SDL_MIRRORDATE,
            checkcmd="pkg-config --atleast-version=1.3.0 sdl",
            buildcmd=
                "cd SDL-1.3.0-* && "
                "./configure --prefix='%s' --disable-dependency-tracking "
                    "CFLAGS='%s -framework ForceFeedback' "
                    "LDFLAGS='%s -framework ForceFeedback' &&"
                "make ${MAKEFLAGS} && "
                "make install" % (COMMON_INSTALLDIR, MAC106_COMMON_CFLAGS, MAC106_COMMON_CFLAGS)
            ),
#         TPP("live",
#             url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
#             url2="live555-%s.tar.gz"%LIVE_MIRRORDATE,
#             checkcmd="test -f ./live/liveMedia/libliveMedia.a",
#             buildcmd=
#                 "cd live && "
#                 "tar xf %s/third_party_packages/live-patches.tar && "
#                 "./genMakefiles macosx3264 && "
#                 "make ${MAKEFLAGS} " % AMBULANT_DIR
#             ),
        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.18.1.1.tar.gz",
            url2="gettext-0.18.1.1.tar.gz",
            checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.18.1.1 && "
            	"patch -p1 --forward < %s/third_party_packages/gettext-0.18.1.1.patch && "
                "%s --disable-csharp && "
                "make ${MAKEFLAGS} && "
                "make install" % (AMBULANT_DIR, MAC106_COMMON_CONFIGURE)
            ),
        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.7.tar.gz",
            url2="libxml2-2.7.7.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.7.7 && "
                "%s --disable-dependency-tracking && "
                "make ${MAKEFLAGS} && "
                "make install" % MAC106_COMMON_CONFIGURE
            ),
        TPP("xulrunner-sdk", # no libraries used, 32 bit version also works w. 64 bit build
            url="%s%s.en-US.mac-i386.sdk.tar.bz2" % (XULRUNNER_URL, XULRUNNER_VERSION),
            url2="%s.en-US.mac-i386.sdk.tar.bz2" % XULRUNNER_VERSION,
            checkcmd="test -d xulrunner-sdk",
            buildcmd="test -d xulrunner-sdk"
            ),
        TPP("libltdl", # Workaround/hack for missing libltdl on 10.8
            checkcmd="test -f ../libltdl/.libs/libltdlc.a",
            buildcmd=
                "rm -rf ../libltdl &&"
                "mkdir ../libltdl &&"
                "cd ../libltdl &&"
                "../../libltdl/configure CFLAGS='%s' --disable-dependency-tracking &&"
                "make" % MAC106_COMMON_CFLAGS
            ),
        ],

    'mac10.4' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
                "cd expat-2.0.1 && "
                "patch --forward < %s/third_party_packages/expat.patch && "
                "autoconf && "
                "%s && "
                "make ${MAKEFLAGS} && "
                "make install" % (AMBULANT_DIR, MAC104_COMMON_CONFIGURE)
            ),
        TPP("xerces-c",
            url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.tar.gz",
            checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
            buildcmd=
                "cd xerces-c-3.1.1 && "
                "%s CXXFLAGS='%s' --disable-dependency-tracking --without-curl && "
                "make ${MAKEFLAGS} && "
                "make install" % (MAC104_COMMON_CONFIGURE, MAC104_COMMON_CFLAGS)
            ),
        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make ${MAKEFLAGS} && "
                "make install" % MAC104_COMMON_CONFIGURE
            ),

        TPP("ffmpeg",
            url="http://sourceforge.net/projects/ambulant/files/ffmpeg%20for%20Ambulant/ffmpeg-export-2010-01-22.tar.gz/download",
            checkcmd="pkg-config --atleast-version=52.47.0 libavformat",
            buildcmd=
                "mkdir ffmpeg-export-universal && "
                "cd ffmpeg-export-universal && "
                "sh %s/scripts/ffmpeg-osx-fatbuild.sh %s/ffmpeg-export-2010-01-22 all" % 
                    (AMBULANT_DIR, os.getcwd())
            ),

        TPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
            checkcmd="pkg-config --atleast-version=1.3.0 sdl",
            buildcmd=
                "cd SDL-1.3.0-* && "
                "./configure --prefix='%s' "
                    "--disable-dependency-tracking  --enable-video-x11=no"
                    "CC=gcc-4.0 CXX=g++-4.0 "
                    "CFLAGS='%s' "
                    "LDFLAGS='%s -framework ForceFeedback' &&"
                "make ${MAKEFLAGS} && "
                "make install" % (COMMON_INSTALLDIR, MAC104_COMMON_CFLAGS, MAC104_COMMON_CFLAGS)
            ),
#         TPP("live",
#             url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
#             url2="live555-%s.tar.gz"%LIVE_MIRRORDATE,
#             checkcmd="test -f ./live/liveMedia/libliveMedia.a",
#             buildcmd=
#                 "cd live && "
#                 "tar xf %s/third_party_packages/live-patches.tar && "
#                 "./genMakefiles macosxfat && "
#                 "make ${MAKEFLAGS} " % AMBULANT_DIR
#             ),
        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.18.1.1.tar.gz",
            checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.18.1.1 && "
                "%s --disable-csharp && "
                "make ${MAKEFLAGS} && "
                "make install" % MAC104_COMMON_CONFIGURE
            ),
        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.7.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.7.7 && "
                "%s --disable-dependency-tracking && "
                "make ${MAKEFLAGS} && "
                "make install" % MAC104_COMMON_CONFIGURE
            )
        ],


    'iOS-Device' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            url2="expat-2.0.1.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
            	" . $AMBULANT_DIR/scripts/set_environment.sh iPhoneOS $IPHONEOS_DEPLOYMENT_TARGET; "
                "set && cd expat-2.0.1 && "
                "patch --forward < $AMBULANT_DIR/third_party_packages/expat.patch && "
                "autoconf && "
                "%s && "
                "make clean;make ${MAKEFLAGS} && "
                "make install" % IPHONE_DEVICE_COMMON_CONFIGURE
            ),

        TPP("libtool", 
            url="http://ftp.gnu.org/gnu/libtool/libtool-2.2.6a.tar.gz",
            url2="libtool-2.2.6a.tar.gz",
            checkcmd="test -f %s/lib/libltdl.a" % COMMON_INSTALLDIR,
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneOS $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd libtool-2.2.6 && "
                        "%s --enable-ltdl-install --disable-dependency-tracking &&"
                "make ${MAKEFLAGS} && "
                "make install"  % IPHONE_DEVICE_COMMON_CONFIGURE
            ),

        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            url2="faad2-2.7.tar.gz",
            checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneOS $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make clean;make ${MAKEFLAGS} && "
                "make install" % IPHONE_DEVICE_COMMON_CONFIGURE
            ),
# create fat libraries for armv6/7/7s: foreach arch do configure ...;make; rename lib...a to lib..$arch, then use lipo to combine them 
        TPP("ffmpeg",
            url="http://ffmpeg.org/releases/ffmpeg-1.0.tar.gz",
            url2="ffmpeg-1.0.tar.gz",
            checkcmd="pkg-config --atleast-version=54.29.100 libavformat",
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneOS $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd ffmpeg-1.0 && "
                "set -x; for arch in $ARCHS ; do"
				"    export CPU_TYPE=unknown; "
                "    case $arch in "
				"	    armv6)  CPU_TYPE=arm1176jzf-s ;;"
				"		armv7)  CPU_TYPE=cortex-a8 ;;"
				"		armv7s) CPU_TYPE=cortex-a8 ;;"
				"	 esac; "
				"    echo 'arch='$arch 'CPU_TYPE='$CPU_TYPE >/tmp/out; "
                "    ./configure --enable-cross-compile --arch=$arch --target-os=darwin "
			    "        --cc=$PLATFORM_PATH/Developer/usr/bin/gcc "
                "        --sysroot=$SDK_PATH "
				"        --cpu=$CPU_TYPE "
                "        --as='gas-preprocessor.pl $PLATFORM_PATH/Developer/usr/bin/gcc' "
                "        --extra-cflags='-arch $arch -I../installed/include' "
				"        --extra-ldflags='-arch $arch -L../installed/lib -L$SDK_PATH/usr/lib/system' "
                "        --prefix=../installed/ --enable-gpl  --disable-mmx --disable-asm "
				"        --disable-ffmpeg --disable-ffserver --disable-ffplay --disable-ffprobe --disable-neon --disable-doc;"
                "     make clean;make ${MAKEFLAGS}; "
				"	  for i in `ls */*.a`; do cp $i `dirname $i`/`basename $i .a`-$arch; done;echo $arch done ;"
                "done &&"
                "for i in `ls */*.a`; do rm $i; (cd `dirname $i`; lipo `basename $i .a`-arm* -create -output `basename $i`); done; "
 				"make install"
            ),

        TPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
            url2="SDL-1.3-%s.tar.gz"%SDL_MIRRORDATE,
            checkcmd="test -f %s/lib/libSDL.a" % COMMON_INSTALLDIR,
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneOS $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd SDL-1.3.0-*  && "
                "./configure --without-video --disable-dependency-tracking --disable-video-cocoa --disable-video-x11 --disable-video-opengl --disable-haptic --disable-diskaudio  --host=`uname -m`-darwin &&"                
	           "(cd src/video/uikit; patch -p1 -N -r - < $AMBULANT_DIR/third_party_packages/SDL-uikitviewcontroller.patch) && "
                "cd Xcode-iOS/SDL  && "
                "xcodebuild -target libSDL $ARCH_ARGS -sdk iphoneos$IPHONEOS_DEPLOYMENT_TARGET -configuration Release &&"
                "mkdir -p ../../../installed/include/SDL && "
                "cp ../../include/* ./build/Release-iphoneos/usr/local/include/* ../../../installed/include/SDL &&"
                "mkdir -p ../../../installed/include/lib && cp ./build/Release-iphoneos/libSDL.a ../../../installed/lib"
            ),

#         TPP("live",
#             url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
#             url2="live555-%s.tar.gz"%LIVE_MIRRORDATE,
#             checkcmd="test -f ./live/liveMedia/libliveMedia.a",
#             buildcmd=
#                 "set -x;cd live && "
#                 ". $AMBULANT_DIR/scripts/set_environment.sh iPhoneOS %s; "
#                 "tar xf $AMBULANT_DIR/third_party_packages/live-patches.tar && "
#                 "export C=c; for arch in $ARCHS ; do export TARGET_CPU_ARCH=$arch; ./genMakefiles iOS && "
#                 "make clean;make ${MAKEFLAGS}; for i in `ls */*.a`; do cp $i `basename $i .a`-$arch; done; done &&" 
#                 "for i in `ls */*.a`; do rm $i; (cd `dirname $i`; lipo `basename $i .a`-arm* -create -output `basename $i`); done; "
#                 "" % os.getenv("IPHONEOS_DEPLOYMENT_TARGET")
#             ),

##      TPP("gettext",
##          url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.18.1.1.tar.gz",
##          checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
##          buildcmd=
##           	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneOS $IPHONEOS_DEPLOYMENT_TARGET; "
##              "cd gettext-0.18.1.1 && "
##              "%s --disable-csharp && "
##              "make clean;make ${MAKEFLAGS} && "
##              "make install" % IPHONE_DEVICE_COMMON_CONFIGURE
##          ),

        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.7.tar.gz",
            url2="libxml2-2.7.7.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneOS $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd libxml2-2.7.7 && "
                "%s --disable-dependency-tracking --without-python && "
                "make ${MAKEFLAGS} && "
                "make install" % IPHONE_DEVICE_COMMON_CONFIGURE
            )
        ],

    'iOS-Simulator' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            url2="expat-2.0.1.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneSimulator $IPHONEOS_DEPLOYMENT_TARGET && "
                "cd expat-2.0.1 && "
                "patch --forward < $AMBULANT_DIR/third_party_packages/expat.patch && "
                "autoconf && "
                "%s && "
                "make clean;make ${MAKEFLAGS} && "
                "make install" % IPHONE_SIMULATOR_COMMON_CONFIGURE
            ),

        TPP("libtool", 
            url="http://ftp.gnu.org/gnu/libtool/libtool-2.2.6a.tar.gz",
            url2="libtool-2.2.6a.tar.gz",
            checkcmd="test -f %s/lib/libltdl.a" % COMMON_INSTALLDIR,
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneSimulator $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd libtool-2.2.6 && "
                "%s --enable-ltdl-install &&"
                "make ${MAKEFLAGS} && "
                "make install" % IPHONE_SIMULATOR_COMMON_CONFIGURE
            ),

        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            url2="faad2-2.7.tar.gz",
            checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneSimulator $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make clean;make ${MAKEFLAGS} && "
                "make install" % IPHONE_SIMULATOR_COMMON_CONFIGURE
            ),

        TPP("ffmpeg",
            url="http://ffmpeg.org/releases/ffmpeg-1.0.tar.gz",
            url2="ffmpeg-1.0.tar.gz",
            checkcmd="pkg-config --atleast-version=54.29.100 libavformat",
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneSimulator $IPHONEOS_DEPLOYMENT_TARGET; "
            	"cd ffmpeg-1.0 && "
                "./configure --enable-cross-compile --arch=i386 --target-os=darwin --sysroot=$SDK_PATH --cc=$PLATFORM_PATH/Developer/usr/bin/gcc "
                "--as='gas-preprocessor.pl $PLATFORM_PATH/Developer/usr/bin/gcc' --enable-cross-compile "
                "--extra-cflags='$ARCH_ARGS -I../installed/include' "
                "--extra-ldflags='$ARCH_ARGS -L../installed/lib -L$SDK_PATH/usr/lib/system ' "
                "--prefix=%s --enable-gpl --disable-mmx --disable-asm --disable-ffprobe;"
                "make clean;make ${MAKEFLAGS}; make install" % COMMON_INSTALLDIR
            ),

        TPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
            url2="SDL-1.3-%s.tar.gz"%SDL_MIRRORDATE,
            checkcmd="test -f %s/lib/libSDL.a" % COMMON_INSTALLDIR,
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneSimulator $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd SDL-1.3.0-*  && "
                "(cd src/video/uikit; patch -p1 -N -r - < $AMBULANT_DIR/third_party_packages/SDL-uikitviewcontroller.patch) && "
                "./configure --prefix=%s --without-video --disable-dependency-tracking --disable-video-cocoa --disable-video-x11 --disable-video-opengl --disable-haptic --disable-diskaudio --host=`uname -m`-darwin &&"                
                "cd Xcode-iOS/SDL  && "
                "xcodebuild -target libSDL -sdk iphonesimulator$IPHONEOS_DEPLOYMENT_TARGET -configuration Debug ARCHS=$ARCHS &&"
                "mkdir -p ../../../installed/include/SDL && cp ../../include/* ../../../installed/include/SDL &&"
                "cp ./build/Debug-iphonesimulator/usr/local/include/* ../../../installed/include/SDL &&"
                "mkdir -p ../../../installed/include/lib && cp ./build/Debug-iphonesimulator/libSDL.a ../../../installed/lib" % COMMON_INSTALLDIR
		 ),

#         TPP("live",
#             url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
#             url2="live555-%s.tar.gz"%LIVE_MIRRORDATE,
#             checkcmd="test -f ./live/liveMedia/libliveMedia.a",
#             buildcmd=
#            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneSimulator $IPHONEOS_DEPLOYMENT_TARGET; "
#                 "cd live && "
#                 "tar xf %s/third_party_packages/live-patches.tar && "
#                 "./genMakefiles iOS-Simulator && "
#                 "make clean;make ${MAKEFLAGS} " % AMBULANT_DIR
#             ),

##      TPP("gettext",
##            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.18.1.1.tar.gz",
##          checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
##          buildcmd=
##            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneSimulator $IPHONEOS_DEPLOYMENT_TARGET; "
##              "cd gettext-0.18.1.1 && "
##              "%s --disable-csharp && "
##              "make clean;make ${MAKEFLAGS} && "
##              "make install"% IPHONE_SIMULATOR_COMMON_CONFIGURE
##          ),

        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.7.tar.gz",
            url2="libxml2-2.7.7.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
            	". $AMBULANT_DIR/scripts/set_environment.sh iPhoneSimulator $IPHONEOS_DEPLOYMENT_TARGET; "
                "cd libxml2-2.7.7 && "
                "%s --disable-dependency-tracking --without-python && "
                "make ${MAKEFLAGS} && "
                "make install" % IPHONE_SIMULATOR_COMMON_CONFIGURE
            )
        ],

    'linux' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            url2="expat-2.0.1.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
                "set -x;cd expat-2.0.1 && "
                "if [ ! -e expat.pc.in ] ; then patch --forward < %s/third_party_packages/expat.patch; fi && "
                "autoconf && "
                "%s && "
                "make ${MAKEFLAGS} && "
                "make install && "
                "install -D -m 755 expat.pc %s/lib/pkgconfig/expat.pc" % (AMBULANT_DIR, LINUX_COMMON_CONFIGURE, COMMON_INSTALLDIR)
            ),

        TPP("xerces-c",
            url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.tar.gz",
            url2="xerces-c-3.1.1.tar.gz",
            checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
            buildcmd=
                "cd xerces-c-3.1.1 && "
                "%s && "
                "make ${MAKEFLAGS} && "
                "make install" % (LINUX_COMMON_CONFIGURE)
            ),

        TPP("faad2",
            url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
            url2="faad2-2.7.tar.gz",
            checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd faad2-2.7 && "
                "%s && "
                "make ${MAKEFLAGS} && "
                "make install" % LINUX_COMMON_CONFIGURE
            ),

        TPP("xulrunner-sdk",
            url="%s%s.en-US.linux-i686.sdk.tar.bz2" % (XULRUNNER_URL, XULRUNNER_VERSION),
            url2="%s.en-US.linux-i686.sdk.tar.bz2" % XULRUNNER_VERSION,
            checkcmd="test -d xulrunner-sdk",
            buildcmd="test -d xulrunner-sdk"
            ),

        TPP("ffmpeg",
            url="http://ffmpeg.org/releases/ffmpeg-1.0.tar.gz",
            url2="ffmpeg-1.0.tar.gz",
            checkcmd="pkg-config --atleast-version=53.24.2 libavformat",
            buildcmd=
                "cd ffmpeg-1.0&& "
                "%s --enable-gpl --enable-shared --disable-bzlib --extra-cflags=-I%s/include --extra-ldflags=-L%s/lib&&"
                "make install " % 
                    (LINUX_COMMON_CONFIGURE, COMMON_INSTALLDIR, COMMON_INSTALLDIR)
            ),

        TPP("SDL",
#           url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
#           url2="SDL-1.3-%s.tar.gz"%SDL_MIRRORDATE,
            # patch takes care of SDL bug #1513 http://bugzilla.libsdl.org/buglist.cgi?quicksearch=SDL_SetWindowSize
            checkcmd="pkg-config --atleast-version=2.0.0 sdl2",
            buildcmd=
               "if [ ! -e SDL ] ; then hg clone http://hg.libsdl.org/SDL; "
               "cd SDL && "
               "patch -p1 < %s/third_party_packages/SDL-bug-1513.patch; " 
               "else cd SDL; fi; mkdir -p build; cd build &&"
# Note: SDL 2.0 wants a different build directory, therefore one '.' is preprended
                ".%s --disable-video-x11-xinput&&"
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % (AMBULANT_DIR, LINUX_COMMON_CONFIGURE)
            ),

        TPP("SDL_image",
# mercurial version needed for compatibilty with SDL2
#           url="http://www.libsdl.org/projects/SDL_image/release/SDL_image-1.2.13.tar.gz",
#           url2="SDL-1.2.13-%s.tar.gz"%SDL_MIRRORDATE,
            checkcmd="pkg-config --atleast-version=1.2.13 SDL2_image",
            buildcmd=
                "if [ ! -e SDL_image ] ; then  hg clone http://hg.libsdl.org/SDL_image ; fi && "
                "cd SDL_image && mkdir -p build && cd build && "
                ".%s &&"
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % LINUX_COMMON_CONFIGURE
            ),

        TPP("SDL_Pango", # SDL interface for Pango glyph rendering system
            url="http://sourceforge.net/projects/sdlpango/files/latest/download",
            url2="SDL_Pango-0.1.2.tar.gz",
# patches needed for compatibilty with distributed versions and one for SDL2
            checkcmd="pkg-config --atleast-version=0.1.3 SDL_Pango",
            buildcmd=
                "unset PKG_CONFIG_LIBDIR &&"
                "cd SDL_Pango-0.1.2 && "
                "patch -p1 < %s/third_party_packages/SDL_Pango-0.1.2-API-Changes.patch && " 
                "patch -p1 < %s/third_party_packages/SDL_Pango-0.1.2-SDL2-Changes.patch && echo 'AC_DEFUN([AM_PATH_SDL])' > acinclude.m4 && autoreconf && libtoolize && " 
                "which sdl2-config >/dev/null && %s --with-sdl2 && "
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % (AMBULANT_DIR, AMBULANT_DIR, LINUX_COMMON_CONFIGURE)
            ),

#         TPP("live",
#             url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
#             url2="live555-%s.tar.gz"%LIVE_MIRRORDATE,
#             checkcmd="test -f ./live/liveMedia/libliveMedia.a",
#             buildcmd=
#                 "cd live && "
#                 "tar xf %s/third_party_packages/live-patches.tar && "
#                 "./genMakefiles linux && "
#                 "make ${MAKEFLAGS} " % (AMBULANT_DIR)
#             ),

        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.18.1.1.tar.gz",
            url2="gettext-0.18.1.1.tar.gz",
            checkcmd="test -d %s/lib/gettext -o -d /usr/lib/gettext" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.18.1.1 && "
                "%s --disable-csharp && "
                "make ${MAKEFLAGS} && "
                "make install" % LINUX_COMMON_CONFIGURE
            ),

        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.7.tar.gz",
            url2="libxml2-2.7.7.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.7.7 && "
                "%s && "
                "make ${MAKEFLAGS} && "
                "make install" % LINUX_COMMON_CONFIGURE
            )
        ],



    'win32' : [
        WinTPP("expat-jpeg-lpng-mp3lib-zlib",
            url="https://sourceforge.net/projects/ambulant/files/Third%20Party%20Packages%2Cwin32_wm5/tpp-win-20081123/tpp-win-20081123.zip/download",
            checkcmd="if not exist expat exit 1",
            buildcmd=
                'move "INTO third_party_packages"\\expat expat && ' +
                'move "INTO third_party_packages"\\jpeg jpeg && ' +
                'move "INTO third_party_packages"\\lpng128 lpng128 && ' +
                'move "INTO third_party_packages"\\mp3lib mp3lib && ' +
                'move "INTO third_party_packages"\\zlib zlib',
            # Real building is done by FINAL
            ),
            
        WinTPP("xerces-c",
            url="http://apache.proserve.nl/xerces/c/3/sources/xerces-c-3.1.1.zip",
            url2="xerces-c-3.1.1.zip",
            checkcmd="if not exist xerces-c-3.1.1\\Build\\Win32\\%s\\%s\\xerces-c_3.lib exit 1" % (WIN32_VCVERSION, WIN32_COMMON_CONFIG),
            buildcmd=
                "cd xerces-c-3.1.1\\projects\\Win32\\%s\\xerces-all && "
                "devenv xerces-all.sln /build Debug /project XercesLib && "
                "devenv xerces-all.sln /build Release /project XercesLib" % (WIN32_VCVERSION)
            ),
            
        WinTPP("xulunner-sdk",
            #url="http://releases.mozilla.org/pub/mozilla.org/xulrunner/releases/1.9.2.17/sdk/xulrunner-1.9.2.17.en-US.win32.sdk.zip",
	    #url2="xulrunner-1.9.2.17.en-US.win32.sdk.zip",
	    url="%s%s.en-US.win32.sdk.zip" % (XULRUNNER_URL, XULRUNNER_VERSION), 
            url2="%s.en-US.win32.sdk.zip" % XULRUNNER_VERSION,
            checkcmd="if not exist xulrunner-sdk\\include\\npapi.h exit 1",
            # No build needed
            ),

        WinTPP("ffmpeg-bin",
            url="http://ffmpeg.zeranoe.com/builds/win32/shared/ffmpeg-%s-git-%s-win32-shared.7z" % (FFMPEG_WIN_GIT_DATE, FFMPEG_WIN_GIT_ID),
            checkcmd="if not exist ffmpeg-%s-git-%s-win32-shared\\bin\\avformat-53.dll exit 1" % (FFMPEG_WIN_GIT_DATE, FFMPEG_WIN_GIT_ID),
            # No build needed
            ),

        WinTPP("ffmpeg-dev",
            url="http://ffmpeg.zeranoe.com/builds/win32/dev/ffmpeg-%s-git-%s-win32-dev.7z" % (FFMPEG_WIN_GIT_DATE, FFMPEG_WIN_GIT_ID),
            checkcmd="if not exist ffmpeg-%s-git-%s-win32-dev\\lib\\avformat.lib exit 1" % (FFMPEG_WIN_GIT_DATE, FFMPEG_WIN_GIT_ID),
            # No build needed
            ),

        #  The WINDOWS_DXSDK paths (DirectX SDK) need to be added for the SDL build to work.
        # Note: older zipfiles for 1.2.14 (sigh) had the VisualC directory zipped. Add a line
        #                 "%s VisualC.zip && "
        # and WINDOWS_UNZIP to the arglist if that turns out to happen again in future.
        WinTPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-1.3.0-6050.zip",
            url2="SDL-1.3.0-6050.zip",
            checkcmd="if not exist SDL-1.3.0-6050\\VisualC\\SDL\\Win32\\%s\\SDL.dll exit 1" % WIN32_COMMON_CONFIG,
            buildcmd=
                "cd SDL-1.3.0-6050 && "
                "cd VisualC && "
                "set INCLUDE=%s\\Include;%%INCLUDE%% && "
                "set LIB=%s\\Lib\\x86;%%LIB%% && "
                "devenv SDL_%s.sln /UseEnv /build %s" % (WINDOWS_DXSDK_PATH, WINDOWS_DXSDK_PATH, WIN32_VSVERSION, WIN32_COMMON_CONFIG)
            ),
        # NOTE: the double quotes are needed because of weird cmd.exe unquoting
#         WinTPP("live",
#             url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
#             url2="live555-%s.tar.gz"%LIVE_MIRRORDATE,
#             extractcmd='cmd /c "%s live555-latest.tar.gz && %s live555-latest.tar"' % (WINDOWS_UNTAR, WINDOWS_UNTAR),
#             checkcmd="if not exist live\\liveMedia\\COPYING exit 1",
#             # Build is done by FINAL
#             ),
            
        # NOTE: the double quotes are needed because of weird cmd.exe unquoting
##        WinTPP("libxml2",
##            url="ftp://xmlsoft.org/libxml2/libxml2-2.7.7.tar.gz",
##            url2="libxml2-2.7.7.tar.gz",
##            extractcmd='cmd /c "%s libxml2-2.7.7.tar.gz && %s libxml2-2.7.7.tar"' % (WINDOWS_UNTAR, WINDOWS_UNTAR),
##            checkcmd="if not exist libxml2-2.7.7\\xml2-config.in exit 1",
##            # Build is done by FINAL
##            ),
        WinTPP("libxml2",
            url="http://ambulantplayer.org/our/mirror/has/essential/fixes",
            url2="libxml2-2.7.7-modforVC10.zip",
            checkcmd="if not exist libxml2-2.7.7\\xml2-config.in exit 1",
            # Build is done by FINAL
            ),
        WinTPP("libdispatch-vs2010",
            url="http://ambulantplayer.org/only/our/mirror/is/available/as/zip",
            url2="libdispatch-jack-hg284.zip",
            checkcmd="if defined VS100COMNTOOLS if not exist libdispatch-jack-hg284\\libdispatch\\bin\\Win32\\StaticRelease\\libdispatch.lib exit 1",
            buildcmd="cd libdispatch-jack-hg284 && " +
                "devenv libdispatch.sln /build StaticRelease /project libdispatch && " +
                "devenv libdispatch.sln /build StaticDebug /project libdispatch "
        ),
        WinTPP("libdispatch-vs2008",
            url="http://ambulantplayer.org/only/our/mirror/is/available/as/zip",
            url2="libdispatch-jack-hg284.zip",
            checkcmd="if defined VS90COMNTOOLS if not exist libdispatch-jack-hg284\\VS2008\\StaticRelease\\libdispatch.lib exit 1",
            buildcmd="cd libdispatch-jack-hg284\\VS2008 && " +
                "devenv libdispatch.sln /build StaticRelease /project libdispatch && " +
                "devenv libdispatch.sln /build StaticDebug /project libdispatch "
        ),
            
        WinTPP("FINAL",
            # The FINAL step builds some packages and copies everything to
            # where Ambulant expects it (bin\\win32 and lib\\win32)
            buildcmd=
                "call ..\\scripts\\upgrade3pp2VC10.bat && " +
                ("cd ..\\projects\\%s && " % WIN32_VCVERSION) +
                "devenv third_party_packages.sln /Upgrade && " +
                "devenv third_party_packages.sln /build Debug && " +
                ("devenv third_party_packages.sln /build %s" % WIN32_COMMON_CONFIG)
            ),
        ],
    
}
third_party_packages['mac10.7'] = third_party_packages['mac10.6']

def checkenv_win32(target):
    ok = True
    if not os.path.exists(WINDOWS_UNZIP_PATH):
        print "** Expected unzip at \"%s\", not found." % WINDOWS_UNZIP_PATH
        ok = False
    if not os.path.exists(WINDOWS_UNTAR_PATH):
        print "** Expected 7-zip (for tar extraction) at \"%s\", not found." % WINDOWS_UNTAR_PATH
        ok = False
    if not os.path.exists(WINDOWS_DXSDK_PATH):
        print "** Expected DirectX SDK at \"%s\", not found." % WINDOWS_DXSDK_PATH
        ok = False
    if not ok:
        print "** Please install, and/or edit build_third_party_packages.py to fix"
        return False
    return True

def checkenv_debian(target):
    if os.geteuid() != 0:
        print '** WARNING: you should probably run this as superuser (with sudo)'
    return True
    
def checkenv_unix(target):
    rv = True
    if os.system("make -v >/dev/null") != 0:
        print "** make not in $PATH"
        rv = False
    if os.system("tar --help >/dev/null") != 0:
        print "** tar not in $PATH"
        rv = False
    if os.system("autoconf --version >/dev/null") != 0:
        print "** autoconf not in $PATH"
        rv = False
    if os.system("patch -v >/dev/null") != 0:
        print "** patch not in $PATH"
        rv = False
    if os.system("pkg-config --version >/dev/null") != 0:
        print '** pkg-config not in $PATH'
        rv = False
    return rv
    
def get_mac_build_platform():
	un = os.uname()
	if un[0] != 'Darwin': return None
	major, minor, micro = un[2].split('.')
	osx_minor = int(major)-4
	return "mac10.%d" % osx_minor

def checkenv_mac(target):
    rv = True
    if not checkenv_unix(target):
        rv = False
    if os.system("xcodebuild -version >/dev/null") != 0:
        print "** xcodebuild not in $PATH"
        rv = False
    # Make sure we have MACOSX_DEPLOYMENT_TARGET set, if needed
    build_platform = get_mac_build_platform()
    if target != build_platform and not os.environ.has_key('MACOSX_DEPLOYMENT_TARGET'):
        print '** MACOSX_DEPLOYMENT_TARGET must be set for %s development on %s' % (target, build_platform)
        rv = False
    if target != build_platform and not os.environ.has_key('SDKROOT'):
        print '** SDKROOT must be set for %s development on %s' % (target, build_platform)
        rv = False
    # We need gas-preprocessor, for ffmpeg
    if os.system("gas-preprocessor.pl 2>&1 | grep Unrecognized >/dev/null") != 0:
        print '** Need gas-preprocessor.pl on $PATH. See https://github.com/yuvi/gas-preprocessor'
        rv = False
    return rv

def checkenv_iphone(target):
    rv = True
    if not checkenv_unix(target):
        rv = False
    if os.system("xcodebuild -version >/dev/null") != 0:
        print "** xcodebuild not in $PATH"
        rv = False
    # Make sure we have IPHONEOS_DEPLOYMENT_TARGET set
    if not os.environ.has_key('IPHONEOS_DEPLOYMENT_TARGET'):
        print '** IPHONEOS_DEPLOYMENT_TARGET must be set for %s development' % target
        rv = False
    # Check that we have the right compilers, etc in PATH
    if target == 'iOS-Simulator':
        wanted = '%s/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin' % XCODE_SDK_BASE
        notwanted = '%s/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin' % XCODE_SDK_BASE
    elif target == 'iOS-Device':
        wanted = '%s/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin' % XCODE_SDK_BASE
        notwanted = '%s/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin' % XCODE_SDK_BASE

    else:
        assert 0
    if not wanted in os.environ['PATH']:
        print 'os.environ[PATH]=%s' % os.environ['PATH']
        print '** %s should be in $PATH for %s development' % (wanted, target)
        rv = False
    if notwanted in os.environ['PATH']:
        newpath = os.environ['PATH'].replace(notwanted, wanted)
        os.putenv('PATH', newpath)
        print '+ WARNING: removed %s from $PATH for %s development' % (notwanted, target)
    if not os.environ.has_key('PKG_CONFIG_LIBDIR'):
        print '** PKG_CONFIG_LIBDIR must be set for cross-development'
        rv = False
    # We need gas-preprocessor, for ffmpeg
    if os.system("gas-preprocessor.pl 2>&1 | grep Unrecognized >/dev/null") != 0:
        print '** Need gas-preprocessor.pl on $PATH. See https://github.com/yuvi/gas-preprocessor'
        rv = False
    return rv
        
environment_checkers = {
    'mac10.7' : checkenv_mac,
    'mac10.6' : checkenv_mac,
    'mac10.4' : checkenv_mac,
    'iOS-Simulator' : checkenv_iphone,
    'iOS-Device' : checkenv_iphone,
    'linux': checkenv_unix,
    'win32': checkenv_win32,
    'debian': checkenv_debian,
}

def main():
    global TRYMIRROR
    global NOCHECK
    global NORUN

    parser = OptionParser(usage="Usage: %prog [options] platform")
    parser.add_option("-M", "--nomirror", dest="nomirror", action="store_true",
        help="Ignore mirrored packages, download packages from original location")
    parser.add_option("-f", "--force", dest="nocheck", action="store_true",
        help="Force rebuild of all packages")
    parser.add_option("-x", "--crossbuild", dest="crossbuild", action="store_true",
        help="Build packages here, ignoring whether they are installed system-wide already")
    parser.add_option("-n", "--dry_run", dest="norun", action="store_true",
        help="Don't run commands, only print them")
    parser.add_option("-m", "--loadmirror", dest="mirror", action="store_true",
        help="Mirror all third party packages in the current directory")
    options, args = parser.parse_args()
        
    if options.mirror:
        if args:
            print '-m and platform argument are mutually exclusive'
            return 2
        good = 0
        bad = 0
        all = []
        for pkglist in third_party_packages.values():
            all += pkglist
        for pkg in all:
            if pkg.mirror():
                good += 1
            else:
                bad += 1
        print '+ mirrored: %d packages' % good
        print '+ failed: %d packages' % bad
        sys.exit(bad)
    
    if len(args) != 1 or args[0] not in third_party_packages:
        parser.print_help()
        print "\nPlatform is one of:", ' '.join(third_party_packages.keys())
        return 2

        
    if options.nomirror:
        TRYMIRROR=False
    NOCHECK=options.nocheck
    NORUN=options.norun
    if options.crossbuild:
        if not os.environ.has_key('PKG_CONFIG_LIBDIR'):
            print '** PKG_CONFIG_LIBDIR must be set for cross-development'
            return 1

    ok = environment_checkers[args[0]](args[0])
    if not ok:
        return 1
    allok = True
    final_package = None
    for pkg in third_party_packages[args[0]]:
        if pkg.name == "FINAL":
            # Do this package last
            final_package=pkg
            continue
        print "+ processing:", pkg.name
        ok = pkg.run()
        if ok:
            print "+ ok:", pkg.name
        else:
            print "+ failed:", pkg.name
            allok = False
    if allok and final_package:
        print "+ processing FINAL package"
        ok = pkg.run()
        if ok:
            print "+ ok: FINAL package"
        else:
            print "+ failed: FINAL package"
            allok = False
    elif final_package:
        print "+ skipped: FINAL package, due to earlier errors"
    if not allok:
        return 1
    return 0
    
if __name__ == '__main__':
    sts = main()
    sys.exit(sts)
