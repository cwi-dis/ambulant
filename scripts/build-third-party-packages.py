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
MIRRORBASE="http://www.ambulantplayer.org/thirdpartymirror/2.6/"
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
    
    def __init__(self, name, url=None, url2=None, downloadedfile=None, extractcmd=None, extract2cmd=None, checkcmd=None, buildcmd=None):
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
        self.extract2cmd = extract2cmd
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
        ok = self._command(self.extractcmd)
        if ok and self.extract2cmd:
            ok = self._command(self.extract2cmd)
        return ok
        
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

    def __init__(self, name, ppa=None, releases=None):
        CommonTPP.__init__(self, name)
        self.output = open("log.debian.txt", "a")
        self.ppa = ppa
        if ppa:
            assert ppa.startswith("ppa:")
        self.releases = releases
    
    def run(self):
        if not self._check_ppa():
            return False
        return self._command(self.PACKAGE_INSTALL_CMD % self.name)
        
    def mirror(self):
        return True
    
    def _check_ppa(self):
        if not self.ppa:
            return True
        cur_release = os.popen("lsb_release -cs", "r").read().strip()
        ppaname = self.ppa[len("ppa:"):]
        ppaname = ppaname.replace('/', '-')
        if not os.path.exists('/etc/apt/sources.list.d/%s-%s.list' % (ppaname, cur_release)):
            print >>self.output, '* Missing PPA %s' % self.ppa
            return False
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
dir = os.getenv('AMBULANT_DIR')
if dir:
    if not os.path.exists(os.path.join(dir, 'configure.ac')):
        print 'ERROR: AMBULANT_DIR=%s, but it does not look like an Ambulant toplevel directory' % dir
        sys.exit(1)
else:
    dir = os.getcwd()
    while dir != '/':
        dir = os.path.dirname(dir)
        if os.path.exists(os.path.join(dir, 'configure.ac')):
            break
    if dir == '/':
        print 'ERROR: cannot find Ambulant toplevel directory'
        sys.exit(1)
    print '+ Ambulant toplevel directory:', dir
    os.environ["AMBULANT_DIR"]=dir
AMBULANT_DIR=dir
COMMON_INSTALLDIR=os.path.join(os.getcwd(), "installed")

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
# Determine iOS and iOS Simulator SDKs and architectures.
#
# The standard way to set these (which Xcode uses too) is to use the environment variables
# IPHONEOS_DEPLOYMENT_TARGET and SDKROOT.
# If these are missing we try to infer them.

IOS_VERSION=os.environ.get('IPHONEOS_DEPLOYMENT_TARGET', '')
IOS_SDK=os.environ.get('SDKROOT', None)
IOSSIM_SDK=IOS_SDK

if IOS_SDK and not IOS_VERSION:
    IOS_VERSION = IOS_SDK[-7:-4]
    print '*Warning: Assuming IOS_VERSION=%s, from IOS_SDK=%s' % (IOS_VERSION, IOS_SDK)
if IOS_VERSION and not IOS_SDK:
    IOS_SDK= "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS%s.sdk" % IOS_VERSION
    IOSSIM_SDK= "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator%s.sdk" % IOS_VERSION
    
    
IOS_VERSION_TO_PARAMETERS = {
    '' : {
        'arch' : '',
        'simarch' : '',
        },
    '5.1' : {
        'arch' : '-arch armv7',
        'simarch' : '-arch i386',
        },
    '6.0' : {
        'arch' : '-arch armv7 -arch armv7s',
        'simarch' : '-arch i386',
        },
    '7.0' : {
        'arch' : '-arch armv7 -arch armv7s',
        'simarch' : '-arch i386',
        },
    '7.1' : {
        'arch' : '-arch armv7 -arch armv7s',
        'simarch' : '-arch i386',
        },
    '8.0' : {
        'arch' : '-arch armv7 -arch armv7s',
        'simarch' : '-arch i386',
        },
    '8.1' : {
        'arch' : '-arch armv7 -arch armv7s',
        'simarch' : '-arch i386',
        },
}

if not IOS_VERSION in IOS_VERSION_TO_PARAMETERS:
    print '** Warning: IOS_VERSION "%s" not known, guessing arch and simarch (which will probably fail)' % IOS_VERSION
    IOS_VERSION=''

IPHONE_DEVICE_COMMON_CFLAGS="%s -isysroot %s" % (IOS_VERSION_TO_PARAMETERS[IOS_VERSION]['arch'], IOS_SDK)
IPHONE_DEVICE_COMMON_CONFIGURE=("./configure " +
    " --host=arm-apple-darwin11 " +
    " --prefix='%s'" % COMMON_INSTALLDIR +
    " --disable-shared " +
    " CFLAGS=\"%s\" " % IPHONE_DEVICE_COMMON_CFLAGS +
    " LDFLAGS=\"%s\" " % IPHONE_DEVICE_COMMON_CFLAGS +
    " CC='xcrun -sdk iphoneos cc -isysroot %s' " % IOS_SDK +
    " CPP='xcrun -sdk iphoneos cc -E -arch armv7 -isysroot %s' " % IOS_SDK +
    " CXX='xcrun -sdk iphoneos cc -isysroot %s' " % IOS_SDK +
    " CXXCPP='xcrun -sdk iphoneos cc -E -arch armv7 -isysroot %s' " % IOS_SDK
    )

IPHONE_SIMULATOR_COMMON_CFLAGS="%s -isysroot %s" % (IOS_VERSION_TO_PARAMETERS[IOS_VERSION]['simarch'], IOSSIM_SDK)
IPHONE_SIMULATOR_COMMON_CONFIGURE=("./configure " +
    " --prefix='%s'" % COMMON_INSTALLDIR +
    " --disable-shared " +
    " CFLAGS=\"%s\" " % IPHONE_SIMULATOR_COMMON_CFLAGS +
    " LDFLAGS=\"%s\" " % IPHONE_SIMULATOR_COMMON_CFLAGS +
    " CC='xcrun -sdk iphonesimulator cc -isysroot %s' " % IOSSIM_SDK +
    " CPP='xcrun -sdk iphonesimulator cc -E -arch i386 -isysroot %s' " % IOSSIM_SDK +
    " CXX='xcrun -sdk iphonesimulator cc -isysroot %s' " % IOSSIM_SDK +
    " CXXCPP='xcrun -sdk iphonesimulator cc -E -arch i386 -isysroot %s' " % IOSSIM_SDK
    )
#IPHONE_SIMULATOR_COMMON_CONFIGURE="CFLAGS=\"%s\" && ./configure --prefix='%s'  CC=llvm-gcc CXX=llvm-g++ CFLAGS=\"$CFLAGS\" CXXFLAGS=\"$CFLAGS\"  LDFLAGS=\"$CFLAGS\" " % (IPHONE_SIMULATOR_COMMON_CFLAGS, COMMON_INSTALLDIR)

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
        DebianTPP("libtool"),
        DebianTPP("gettext"),
        DebianTPP("autotools-dev"),
        DebianTPP("gettext"),
        DebianTPP("libgtk2.0-dev"),
        DebianTPP("libgdk-pixbuf2.0-dev"),
        DebianTPP("libxml2-dev"),
        DebianTPP("libltdl-dev"),
        DebianTPP("libsdl1.2-dev"),
        DebianTPP("libxerces-c-dev"),
        DebianTPP("libexpat1-dev"),
        DebianTPP("python-dev"),
        DebianTPP("python-gtk2-dev"),
        DebianTPP("python-gobject-dev"),
        DebianTPP("libdispatch-dev"),
        
        # The following come from ppa:zoogie/sdl2-snapshots (as of 06-Oct-2013).
        # Unfortunately they don't work on 13.10...
        #DebianTPP("libsdl2-dev", ppa="ppa:zoogie/sdl2-snapshots"),
        #DebianTPP("libsdl2-image-dev", ppa="ppa:zoogie/sdl2-snapshots"),
        #DebianTPP("libsdl2-ttf-dev", ppa="ppa:zoogie/sdl2-snapshots"),
        
        # The following are from ppa:samrog131/ppa (as of 12-Oct-2013)
        # The -opti- has been added in Dec-2014, as samrog moved to ffmpeg 2.5
        DebianTPP("libavutil-ffmpeg-opti-dev", ppa="ppa:samrog131/ppa"),
        DebianTPP("libswscale-ffmpeg-opti-dev", ppa="ppa:samrog131/ppa"),
        DebianTPP("libswresample-ffmpeg-opti-dev", ppa="ppa:samrog131/ppa"),
        DebianTPP("libavcodec-ffmpeg-opti-dev", ppa="ppa:samrog131/ppa"),
        DebianTPP("libavformat-ffmpeg-opti-dev", ppa="ppa:samrog131/ppa"),

    ],
 
    'macosx' : [
        TPP("libltdl", # Workaround/hack for missing libltdl on 10.8 and later
            checkcmd="test -f ../libltdl/.libs/libltdlc.a",
            buildcmd=
                "mkdir -p ../libltdl &&"
                "cd ../libltdl &&"
                "%s/libltdl/%s --enable-ltdl-install --disable-dependency-tracking &&"
                "make &&"
                "make install" % (AMBULANT_DIR, MAC106_COMMON_CONFIGURE)
            ),

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
            url="http://ffmpeg.org/releases/ffmpeg-2.0.2.tar.gz",
            url2="ffmpeg-2.0.2.tar.gz",
            checkcmd="pkg-config --atleast-version=55.12.0 libavformat",
            buildcmd=
                "rm -rf ffmpeg-2.0.2-universal && "
                "mkdir ffmpeg-2.0.2-universal && "
                "cd ffmpeg-2.0.2-universal && "
                "sh %s/scripts/ffmpeg-osx-fatbuild.sh %s/ffmpeg-2.0.2 all" % 
                    (AMBULANT_DIR, os.getcwd())
            ),
                
#         TPP("SDL",
#             url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
#             url2="SDL-1.3-%s.tar.gz"%SDL_MIRRORDATE,
#             checkcmd="pkg-config --atleast-version=1.3.0 sdl",
#             buildcmd=
#                 "cd SDL-1.3.0-* && "
#                 "./configure --prefix='%s' --disable-dependency-tracking "
#                 "CFLAGS='%s -framework ForceFeedback' "
#                 "LDFLAGS='%s -framework ForceFeedback' &&"
#                 "make ${MAKEFLAGS} && "
#                 "make install" % (COMMON_INSTALLDIR, MAC106_COMMON_CFLAGS, MAC106_COMMON_CFLAGS)
#             ),

        TPP("SDL",
            url="https://www.libsdl.org/release/SDL2-2.0.3.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 sdl2",
            buildcmd=
               "cd SDL2-2.* && "
               "./configure --prefix='%s' CFLAGS='%s' LDFLAGS='%s' --disable-dependency-tracking &&"
               "make ${MAKEFLAGS} && "
               "make install &&"
               "cd .." % (COMMON_INSTALLDIR, MAC106_COMMON_CFLAGS, MAC106_COMMON_CFLAGS)
            ),
          
        TPP("SDL_image",
# mercurial version needed for compatibilty with SDL2
#           url="http://www.libsdl.org/projects/SDL_image/release/SDL_image-1.2.13.tar.gz",
#           url2="SDL-1.2.13-%s.tar.gz"%SDL_MIRRORDATE,
            checkcmd="pkg-config --atleast-version=1.2.13 SDL2_image",
            buildcmd=
                "if [ ! -e SDL_image ] ; then  hg clone http://hg.libsdl.org/SDL_image ; fi && "
                "cd SDL_image && sh autogen.sh && "
                "mkdir -p build && cd build && "
                "SDL_CONFIG=`pwd`/../../installed/bin/sdl2-config .%s --disable-dependency-tracking --disable-webp &&"
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % MAC106_COMMON_CONFIGURE
            ),

#         TPP("SDL_Pango", # SDL interface for Pango glyph rendering system
#             url="http://sourceforge.net/projects/sdlpango/files/latest/download",
#             url2="SDL_Pango-0.1.2.tar.gz",
# patches needed for compatibilty with distributed versions and one for SDL2
#             checkcmd="pkg-config --atleast-version=0.1.3 SDL_Pango",
#             buildcmd=
#                 "unset PKG_CONFIG_LIBDIR &&"
#                 "cd SDL_Pango-0.1.2 && "
#                 "patch -p1 < %s/third_party_packages/SDL_Pango-0.1.2-API-Changes.patch && "
#                 "patch -p1 < %s/third_party_packages/SDL_Pango-0.1.2-SDL2-Changes.patch && echo 'AC_DEFUN([AM_PATH_SDL])' > acinclude.m4 && autoreconf -i && libtoolize && "
#                 "which sdl2-config >/dev/null && %s --with-sdl2 && "
#                 "make ${MAKEFLAGS} && "
#                 "make install &&"
#                 "cd .." % (AMBULANT_DIR, AMBULANT_DIR, MAC106_COMMON_CONFIGURE)
#             ),

        TPP("FreeType2", # SDL2 interface for FreeType2 glyph rendering system needed for SDL2_ttf
            url="http://download.savannah.gnu.org/releases/freetype/freetype-2.5.3.tar.gz",
            url2="freetype-2.5.3.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0 freetype2",
            buildcmd=
                "unset PKG_CONFIG_LIBDIR &&"
                "cd freetype-2.* && "
                "%s --disable-dependency-tracking && "
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % MAC106_COMMON_CONFIGURE
            ),

        TPP("SDL2_ttf", # SDL2 interface for FreeType2 glyph rendering system
            url="https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.12.tar.gz",
#           url2="SDL2_ttf-2.0.12.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.12 SDL2_ttf",
            buildcmd=
                "unset PKG_CONFIG_LIBDIR &&"
                "cd SDL2_ttf-2.0.12 && "
                "%s --disable-dependency-tracking && "
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % MAC106_COMMON_CONFIGURE
            ),

        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.18.2.tar.gz",
            url2="gettext-0.18.2.tar.gz",
            checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.18.2 && "
                "patch -p1 --forward < %s/third_party_packages/gettext-xcode.patch && "
                "%s --disable-csharp && "
                "make ${MAKEFLAGS} && "
                "make install" % ( AMBULANT_DIR, MAC106_COMMON_CONFIGURE)
            ),
            
        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.9.2.tar.gz",
            url2="libxml2-2.9.2.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.9.2 && "
                "%s --disable-dependency-tracking --without-python && "
                "make ${MAKEFLAGS} && "
                "make install" % MAC106_COMMON_CONFIGURE
            ),
            
        TPP("xulrunner-sdk", # no libraries used, 32 bit version also works w. 64 bit build
            url="%s%s.en-US.mac-i386.sdk.tar.bz2" % (XULRUNNER_URL, XULRUNNER_VERSION),
            url2="%s.en-US.mac-i386.sdk.tar.bz2" % XULRUNNER_VERSION,
            checkcmd="test -d xulrunner-sdk",
            buildcmd="test -d xulrunner-sdk"
            ),
        ],

    'iphoneos' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            url2="expat-2.0.1.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
                "cd expat-2.0.1 && "
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
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make clean;make ${MAKEFLAGS} && "
                "make install" % IPHONE_DEVICE_COMMON_CONFIGURE
            ),
            
        # NOTE: The disable-asm should go, it is a serious performance issue....
        TPP("ffmpeg",
            url="http://ffmpeg.org/releases/ffmpeg-2.0.2.tar.gz",
            url2="ffmpeg-2.0.2.tar.gz",
            checkcmd="pkg-config --atleast-version=55.12.0 libavformat",
            buildcmd=
                "cd ffmpeg-2.0.2 && "
                "./configure "
                "    --enable-cross-compile "
                "    --arch=%(arch)s "
                "    --target-os=darwin "
                "    --disable-asm "
                "    --cc='xcrun -sdk iphoneos cc -isysroot %(sdk)s'"
                "    --extra-cflags='-arch %(arch)s -I%(installed)s/include' "
				"    --extra-ldflags='-arch %(arch)s -L%(installed)s/lib' "
                "    --prefix=%(installed)s "
                "    --enable-gpl "
                "    --enable-pic "
				"    --disable-programs "
				"    --disable-doc "
				"&&"
                "make ${MAKEFLAGS} &&"
 				"make install" % 
 			
                    dict(
                        arch="armv7",
                        sdk=IOS_SDK,
                        installed=COMMON_INSTALLDIR,
                    )
            ),

        TPP("SDL",
            url="http://www.libsdl.org/release/SDL2-2.0.3.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 sdl2",
            buildcmd=
               "cd SDL2-2.* && "
               "%s --disable-dependency-tracking &&"
               "(cd include ; cp SDL_config_iphoneos.h SDL_config.h) &&"
               "make ${MAKEFLAGS} && "
               "make install &&"
               "cd .." % (IPHONE_DEVICE_COMMON_CONFIGURE)
            ),
#             buildcmd=
#                 "cd SDL-1.3.0-*  && "
#                 "./configure --without-video --disable-dependency-tracking --disable-video-cocoa --disable-video-x11 --disable-video-opengl --disable-haptic --disable-diskaudio  --host=`uname -m`-darwin && "                
# 	           "(cd src/video/uikit; patch -p1 -N -r - < $AMBULANT_DIR/third_party_packages/SDL-uikitviewcontroller.patch) && "
#                 "cd Xcode-iOS/SDL  && "
#                 "xcodebuild -target libSDL -configuration Release && "
#                 "mkdir -p ../../../installed/include/SDL && "
#                 "cp ../../include/* ./build/Release-iphoneos/usr/local/include/* ../../../installed/include/SDL &&"
#                 "mkdir -p ../../../installed/include/lib && cp ./build/Release-iphoneos/libSDL.a ../../../installed/lib"
#             ),

        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.9.2.tar.gz",
            url2="libxml2-2.9.2.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.9.2 && "
                "%s --disable-dependency-tracking --without-python && "
                "make ${MAKEFLAGS} && "
                "make install" % IPHONE_DEVICE_COMMON_CONFIGURE
            ),
        ],

    'iphonesimulator' : [
        TPP("expat", 
            url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
            url2="expat-2.0.1.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 expat",
            buildcmd=
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
                "cd faad2-2.7 && "
                "%s --disable-dependency-tracking && "
                "make clean;make ${MAKEFLAGS} && "
                "make install" % IPHONE_SIMULATOR_COMMON_CONFIGURE
            ),

        TPP("ffmpeg",
            url="http://ffmpeg.org/releases/ffmpeg-2.0.2.tar.gz",
            url2="ffmpeg-2.0.2.tar.gz",
            checkcmd="pkg-config --atleast-version=55.12.0 libavformat",
            buildcmd=
                "cd ffmpeg-2.0.2 && "
                "./configure "
                "    --enable-cross-compile "
                "    --arch=%(arch)s "
                "    --target-os=darwin "
                "    --cc='xcrun -sdk iphonesimulator cc -sysroot %(sdk)s' "
                "    --extra-cflags='-arch %(arch)s -I%(installed)s/include' "
				"    --extra-ldflags='-arch %(arch)s -L%(installed)s/lib' "
                "    --prefix=%(installed)s "
                "    --enable-gpl "
                "    --enable-pic "
				"    --disable-programs "
				"    --disable-doc "
				"&&"
                "make ${MAKEFLAGS} &&"
 				"make install" % 
 			
                    dict(
                        arch="i386",
                        sdk=IOSSIM_SDK,
                        installed=COMMON_INSTALLDIR,
                    )
            ),

        TPP("SDL",
            url="http://www.libsdl.org/release/SDL2-2.0.3.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 sdl2",
            buildcmd=
               "cd SDL2-2.* && "
               "%s --disable-video-opengl --disable-audio --disable-joystick --disable-video-opengles --disable-dependency-tracking &&"
               "(cd include ; cp SDL_config_iphoneos.h SDL_config.h) &&"
               "make ${MAKEFLAGS} && "
               "make install &&"
               "cd .." % (IPHONE_SIMULATOR_COMMON_CONFIGURE)
            ),

        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.9.2.tar.gz",
            url2="libxml2-2.9.2.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.9.2 && "
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
            url="http://ffmpeg.org/releases/ffmpeg-2.0.2.tar.gz",
            url2="ffmpeg-2.0.2.tar.gz",
            checkcmd="pkg-config --atleast-version=55.12.0 libavformat",
            buildcmd=
                "cd ffmpeg-2.0.2&& "
                "%s --enable-gpl --enable-shared --disable-bzlib --extra-cflags=-I%s/include --extra-ldflags=-L%s/lib&&"
                "make install " % 
                    (LINUX_COMMON_CONFIGURE, COMMON_INSTALLDIR, COMMON_INSTALLDIR)
            ),

        TPP("SDL",
            url="http://www.libsdl.org/tmp/SDL-2.0.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.0 sdl2",
            buildcmd=
               "cd SDL-2.0.*-* && "
                "%s --disable-video-x11-xinput &&"
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % (LINUX_COMMON_CONFIGURE)
            ),

        TPP("SDL_image",
# mercurial version needed for compatibilty with SDL2
#           url="http://www.libsdl.org/projects/SDL_image/release/SDL_image-1.2.13.tar.gz",
#           url2="SDL-1.2.13-%s.tar.gz"%SDL_MIRRORDATE,
            checkcmd="pkg-config --atleast-version=1.2.13 SDL2_image",
            buildcmd=
                "if [ ! -e SDL_image ] ; then  hg clone http://hg.libsdl.org/SDL_image ; fi && "
                "cd SDL_image && sh autogen.sh && "
                "mkdir -p build && cd build && "
                "SDL_CONFIG=`pwd`/../../installed/bin/sdl2-config .%s &&"
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
                "patch -p1 < %s/third_party_packages/SDL_Pango-0.1.2-SDL2-Changes.patch && echo 'AC_DEFUN([AM_PATH_SDL])' > acinclude.m4 && autoreconf -i && libtoolize && " 
                "which sdl2-config >/dev/null && %s --with-sdl2 && "
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % (AMBULANT_DIR, AMBULANT_DIR, LINUX_COMMON_CONFIGURE)
            ),

        TPP("SDL2_ttf", # SDL2 interface for FreeType2 glyph rendering system
            url="https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.12.tar.gz",
            url2="SDL2_ttf-2.0.12.tar.gz",
            checkcmd="pkg-config --atleast-version=2.0.12 SDL2_ttf",
            buildcmd=
                "unset PKG_CONFIG_LIBDIR &&"
                "cd SDL2_ttf-2.0.12 && "
                "%s && "
                "make ${MAKEFLAGS} && "
                "make install &&"
                "cd .." % LINUX_COMMON_CONFIGURE
            ),

        TPP("gettext",
            url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.18.2.tar.gz",
            url2="gettext-0.18.2.tar.gz",
            checkcmd="test -d %s/lib/gettext -o -d /usr/lib/gettext" % COMMON_INSTALLDIR,
            buildcmd=
                "cd gettext-0.18.2 && "
                "%s --disable-csharp && "
                "make ${MAKEFLAGS} && "
                "make install" % LINUX_COMMON_CONFIGURE
            ),

        TPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.9.2.tar.gz",
            url2="libxml2-2.9.2.tar.gz",
            checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
            buildcmd=
                "cd libxml2-2.9.2 && "
                "%s --without-python && "
                "make ${MAKEFLAGS} && "
                "make install" % LINUX_COMMON_CONFIGURE
            ),
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
            url="http://ffmpeg.zeranoe.com/builds/win32/shared/ffmpeg-2.0.2-win32-shared.7z" ,
            checkcmd="if not exist ffmpeg-2.0.2-win32-shared\\bin\\avformat-55.dll exit 1" ,
            # No build needed
            ),

        WinTPP("ffmpeg-dev",
            url="http://ffmpeg.zeranoe.com/builds/win32/dev/ffmpeg-2.0.2-win32-dev.7z",
            checkcmd="if not exist ffmpeg-2.0.2-win32-dev\\lib\\avformat.lib exit 1",
            # No build needed
            ),

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

        WinTPP("libxml2",
            url="ftp://xmlsoft.org/libxml2/libxml2-2.9.1.tar.gz",
            url2="libxml2-2.9.1.tar.gz",
            extractcmd=WINDOWS_UNZIP + " libxml2-2.9.1.tar.gz",
            extract2cmd=WINDOWS_UNZIP + " libxml2-2.9.1.tar",
            checkcmd="if not exist libxml2-2.9.1\\xml2-config.in exit 1",
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

third_party_packages['macosx10.8'] = third_party_packages['macosx10.7'] = third_party_packages['macosx10.6'] = third_party_packages['macosx']

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
	return "macosx10.%d" % osx_minor

def checkenv_mac(target):
    rv = True
    if not checkenv_unix(target):
        rv = False
        
    if os.system("xcodebuild -version >/dev/null") != 0:
        print "** xcodebuild not in $PATH"
        rv = False
        
    # Make sure we have MACOSX_DEPLOYMENT_TARGET set, if needed
    build_platform = get_mac_build_platform()
    if target == 'macosx':
        target = build_platform
    if target != build_platform and not os.environ.has_key('MACOSX_DEPLOYMENT_TARGET'):
        print '** MACOSX_DEPLOYMENT_TARGET must be set for %s development on %s' % (target, build_platform)
        rv = False
    if target != build_platform and not os.environ.has_key('SDKROOT'):
        print 'Warning: SDKROOT not set for %s development on %s' % (target, build_platform)
        #rv = False
        
    # We need gas-preprocessor, for ffmpeg
    if os.system("gas-preprocessor.pl 2>&1 | grep Unrecognized >/dev/null") != 0:
        print '** Need gas-preprocessor.pl on $PATH. See https://github.com/yuvi/gas-preprocessor'
        rv = False
    return rv

def checkenv_iphone(target):
    rv = True
    wanted = notwanted = ''
    if not checkenv_unix(target):
        rv = False
        
    if os.system("xcodebuild -version >/dev/null") != 0:
        print "** xcodebuild not in $PATH"
        rv = False
        
    # Make sure we have IPHONEOS_DEPLOYMENT_TARGET set
    if not os.environ.has_key('IPHONEOS_DEPLOYMENT_TARGET') and IOS_VERSION:
        os.environ['IPHONEOS_DEPLOYMENT_TARGET'] = IOS_VERSION
        print '+ IPHONEOS_DEPLOYMENT_TARGET set to %s for %s development' % (IOS_VERSION, target)

    # Check that we are not in an xcode-initiated build for the other platform.
    # This is a hack, but I don't see a way around it...
    if target == 'iphoneos':
        if os.environ.get('PLATFORM_NAME') == 'iphonesimulator':
            print 'ERROR: asking for iphoneos build but $PLATFORM_NAME=iphonesimulator'
            sys.exit(1)
    elif target == 'iphonesimulator':
        if os.environ.get('PLATFORM_NAME') == 'iphoneos':
            print 'ERROR: asking for iphonesimulator build but $PLATFORM_NAME=iphoneos'
            sys.exit(0)
    else:
        assert 0
   
    # Check that we have the right compilers, etc in PATH
    if target == 'iphoneos':
        wanted = 'iPhoneOS.platform/Developer/usr/bin'
        notwanted = 'iPhoneSimulator.platform/Developer/usr/bin'
    elif target == 'iphonesimulator':
        wanted = 'iPhoneSimulator.platform/Developer/usr/bin'
        notwanted = 'iPhoneOS.platform/Developer/usr/bin'
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
 
    # Check that the SDK (either passed in SDKROOT or inferred at the top of this file)
    # actually exists
    if target == 'iphoneos':
        if not os.path.exists(IOS_SDK):
            print '** Selected iOS SDK does not exist: %s' % IOS_SDK
            rv = False
    if target == 'iphonesimulator':
        if not os.path.exists(IOSSIM_SDK):
            print '** Selected iOS Simulator SDK does not exist: %s' % IOSSIM_SDK
            rv = False
    return rv
        
environment_checkers = {
    'macosx' : checkenv_mac,
    'macosx10.8' : checkenv_mac,
    'macosx10.7' : checkenv_mac,
    'macosx10.6' : checkenv_mac,
    'macosx10.4' : checkenv_mac,
    'iphonesimulator' : checkenv_iphone,
    'iphoneos' : checkenv_iphone,
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
    
    if len(args) == 1 and args[0] == 'autoXcode':
        # We are run from the XCode third_party_packages project.
        # Inspect the environment to decide what needs to be built.
        args[0] = os.getenv('PLATFORM_NAME')
        if not args[0]:
            sdkroot = os.getenv("SDKROOT")
            if not sdkroot:
                print '** ERROR: platform autoXcode requires $PLATFORM_NAME or $SDKROOT to be set'
                sys.exit(1)
            plistfile = os.path.join(sdkroot, "SDKSettings.plist")
            plistfp = os.popen("plutil -convert xml1 -o - %s" % plistfile)
            import plistlib
            try:
                plist = plistlib.readPlist(plistfp)
            except IOError:
                print '** ERROR: $SDKROOT has no readable SDKSettings.plist'
                raise
            args[0] = plist['DefaultProperties']['PLATFORM_NAME']
            
    if len(args) != 1 or args[0] not in third_party_packages:
        parser.print_help()
        print "\nPlatform is one of:", ' '.join(third_party_packages.keys())
        print "On Mac, a special platform 'autoXcode' will build what xcode needs"
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
            print "* failed:", pkg.name
            allok = False
    if allok and final_package:
        print "+ processing FINAL package"
        ok = pkg.run()
        if ok:
            print "+ ok: FINAL package"
        else:
            print "* failed: FINAL package"
            allok = False
    elif final_package:
        print "+ skipped: FINAL package, due to earlier errors"
    if not allok:
        return 1
    return 0
    
if __name__ == '__main__':
    sts = main()
    sys.exit(sts)
