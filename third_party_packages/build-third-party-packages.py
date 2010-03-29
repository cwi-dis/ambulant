import sys
import os
import subprocess
import urllib
import urlparse
import posixpath

NOCHECK=False
NORUN=False

class TPP:
	def __init__(self, name, url, downloadedfile=None, extractcmd=None, checkcmd=None, buildcmd=None):
		self.name = name

		self.url = url
		if not downloadedfile:
			_, _, path, _, _, _ = urlparse.urlparse(url)
			downloadedfile = posixpath.basename(path)
		self.downloadedfile = downloadedfile
		self.checkcmd = checkcmd
		if not extractcmd:
			extractcmd = "tar xf %s" % self.downloadedfile
		self.extractcmd = extractcmd
		self.buildcmd = buildcmd
		if not buildcmd:
			buildcmd = "cd %s && ./configure && make && make install"
		self.buildcmd = buildcmd
		self.output = None
		
	def begin(self):
		self.output = open("log.%s.txt" % self.name, "w")
		
	def end(self):
		self.output.close()
		self.output = None
		
	def _command(self, cmd, force=False):
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
			return True
		if NOCHECK:
			print >>self.output, "+ dry run, pretend failure:", self.checkcmd
			return False
		return self._command(self.checkcmd, force=True)
		
	def download(self):
		print >>self.output, "+ download:", self.url
		try:
			urllib.urlretrieve(self.url, self.downloadedfile)
		except IOError, arg:
			print >>self.output, "+ download status: error:", arg
			return False
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
	
#AMBULANT_DIR="%s/src/ambulant" % os.getenv("HOME")
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

MAC104_COMMON_CFLAGS="-arch i386 -arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk"
MAC104_COMMON_CONFIGURE="./configure --prefix='%s' CFLAGS='%s' CC=gcc-4.0 CXX=g++-4.0	" % (COMMON_INSTALLDIR, MAC104_COMMON_CFLAGS)

MAC106_COMMON_CFLAGS="-arch i386 -arch x86_64"
MAC106_COMMON_CONFIGURE="./configure --prefix='%s' CFLAGS='%s'	" % (COMMON_INSTALLDIR, MAC106_COMMON_CFLAGS)

LINUX_COMMON_CONFIGURE="./configure --prefix='%s'" % COMMON_INSTALLDIR

appendPath("PATH", os.path.join(COMMON_INSTALLDIR, "bin"))
appendPath("PKG_CONFIG_PATH", os.path.join(COMMON_INSTALLDIR, "lib/pkgconfig"))

third_party_packages={
	'mac10.6' : [
		TPP("expat", 
			url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
			checkcmd="pkg-config --atleast-version=2.0.0 expat",
			buildcmd=
				"cd expat-2.0.1 && "
				"patch < %s/third_party_packages/expat.patch && "
				"autoconf && "
				"%s && "
				"make $(MAKEFLAGS) && "
				"make install" % (AMBULANT_DIR, MAC106_COMMON_CONFIGURE)
			),
		TPP("xerces-c",
			url="http://apache.mirror.transip.nl/xerces/c/3/sources/xerces-c-3.1.0.tar.gz",
			checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
			buildcmd=
				"cd xerces-c-3.1.0 && "
				"%s CXXFLAGS='%s' --disable-dependency-tracking && "
				"make $(MAKEFLAGS) && "
				"make install" % (MAC106_COMMON_CONFIGURE, MAC106_COMMON_CFLAGS)
			),
		TPP("faad2",
			url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
			checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
			buildcmd=
				"cd faad2-2.7 && "
				"%s --disable-dependency-tracking && "
				"make $(MAKEFLAGS) && "
				"make install" % MAC106_COMMON_CONFIGURE
			),
##		TPP("ffmpeg",
##			url="http://ffmpeg.org/releases/ffmpeg-0.5.tar.bz2",
##			checkcmd="pkg-config --atleast-version=52.20.0 libavformat",
##			buildcmd=
##				"mkdir ffmpeg-0.5-universal && "
##				"cd ffmpeg-0.5-universal && "
##				"%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-0.5 all" % 
##					(AMBULANT_DIR, os.getcwd())
##			),
		TPP("ffmpeg",
##			url="http://homepages.cwi.nl/~jack/ambulant/ffmpeg-export-2010-01-22.tgz",
			url="http://sourceforge.net/projects/ambulant/files/ffmpeg%20for%20Ambulant/ffmpeg-export-2010-01-22.tar.gz/download",
			checkcmd="pkg-config --atleast-version=52.20.0 libavformat",
			buildcmd=
				"mkdir ffmpeg-export-universal && "
				"cd ffmpeg-export-universal && "
				"%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-export-2010-01-22 all" % 
					(AMBULANT_DIR, os.getcwd())
			),
		TPP("SDL",
			url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
			checkcmd="pkg-config --atleast-version=1.3.0 sdl",
			buildcmd=
				"cd SDL-1.3.0-* && "
				"./configure --prefix='%s' --disable-dependency-tracking "
					"CFLAGS='%s -framework ForceFeedback' "
					"LDFLAGS='%s -framework ForceFeedback' &&"
				"make $(MAKEFLAGS) && "
				"make install" % (COMMON_INSTALLDIR, MAC106_COMMON_CFLAGS, MAC106_COMMON_CFLAGS)
			),
		TPP("live",
			url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
			checkcmd="test -f ./live/liveMedia/libliveMedia.a",
			buildcmd=
				"cd live && "
				"tar xf %s/third_party_packages/live-osx-fatbuild-patches.tar && "
				"./genMakefiles macosx3264 && "
				"make $(MAKEFLAGS) " % AMBULANT_DIR
			),
		TPP("gettext",
			url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.17.tar.gz",
			checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
			buildcmd=
				"cd gettext-0.17 && "
				"%s --disable-csharp && "
				"make $(MAKEFLAGS) && "
				"make install" % MAC106_COMMON_CONFIGURE
			),
		TPP("libxml2",
			url="ftp://xmlsoft.org/libxml2/libxml2-2.7.5.tar.gz",
			checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
			buildcmd=
				"cd libxml2-2.7.5 && "
				"%s --disable-dependency-tracking && "
				"make $(MAKEFLAGS) && "
				"make install" % MAC106_COMMON_CONFIGURE
			)
		],
	'mac10.4' : [
		TPP("expat", 
			url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
			checkcmd="pkg-config --atleast-version=2.0.0 expat",
			buildcmd=
				"cd expat-2.0.1 && "
				"patch < %s/third_party_packages/expat.patch && "
				"autoconf && "
				"%s && "
				"make $(MAKEFLAGS) && "
				"make install" % (AMBULANT_DIR, MAC104_COMMON_CONFIGURE)
			),
		TPP("xerces-c",
			url="http://apache.mirror.transip.nl/xerces/c/3/sources/xerces-c-3.1.0.tar.gz",
			checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
			buildcmd=
				"cd xerces-c-3.1.0 && "
				"%s CXXFLAGS='%s' --disable-dependency-tracking --without-curl && "
				"make $(MAKEFLAGS) && "
				"make install" % (MAC104_COMMON_CONFIGURE, MAC104_COMMON_CFLAGS)
			),
		TPP("faad2",
			url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
			checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
			buildcmd=
				"cd faad2-2.7 && "
				"%s --disable-dependency-tracking && "
				"make $(MAKEFLAGS) && "
				"make install" % MAC104_COMMON_CONFIGURE
			),
		TPP("ffmpeg",
			url="http://ffmpeg.org/releases/ffmpeg-0.5.tar.bz2",
			checkcmd="pkg-config --atleast-version=52.20.0 libavformat",
			buildcmd=
				"mkdir ffmpeg-0.5-universal && "
				"cd ffmpeg-0.5-universal && "
				"%s/third_party_packages/ffmpeg-osx-fatbuild.sh %s/ffmpeg-0.5 all" % 
					(AMBULANT_DIR, os.getcwd())
			),
##		TPP("SDL",
##			url="http://www.libsdl.org/release/SDL-1.2.13.tar.gz",
##			checkcmd="sdl-config",
##			buildcmd=
##				"cd SDL-1.2.13 && "
##				"./configure --prefix='%s' "
##					"--disable-dependency-tracking "
##					"--disable-video-x11 "
##					"CC=gcc-4.0 CXX=g++-4.0 "
##					"CFLAGS='%s' "
##					"LDFLAGS='%s' &&"
##				"make $(MAKEFLAGS) && "
##				"make install" % (COMMON_INSTALLDIR, MAC104_COMMON_CFLAGS, MAC104_COMMON_CFLAGS)
##			),
		TPP("SDL",
			url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
			checkcmd="pkg-config --atleast-version=1.3.0 sdl",
			buildcmd=
				"cd SDL-1.3.0-* && "
				"./configure --prefix='%s' "
					"--disable-dependency-tracking "
					"CC=gcc-4.0 CXX=g++-4.0 "
					"CFLAGS='%s' "
					"LDFLAGS='%s -framework ForceFeedback' &&"
				"make $(MAKEFLAGS) && "
				"make install" % (COMMON_INSTALLDIR, MAC104_COMMON_CFLAGS, MAC104_COMMON_CFLAGS)
			),
		TPP("live",
			url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
			checkcmd="test -f ./live/liveMedia/libliveMedia.a",
			buildcmd=
				"cd live && "
				"tar xf %s/third_party_packages/live-osx-fatbuild-patches.tar && "
				"./genMakefiles macosxfat && "
				"make $(MAKEFLAGS) " % AMBULANT_DIR
			),
		TPP("gettext",
			url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.17.tar.gz",
			checkcmd="test -f %s/lib/libintl.a" % COMMON_INSTALLDIR,
			buildcmd=
				"cd gettext-0.17 && "
				"%s --disable-csharp && "
				"make $(MAKEFLAGS) && "
				"make install" % MAC104_COMMON_CONFIGURE
			),
		TPP("libxml2",
			url="ftp://xmlsoft.org/libxml2/libxml2-2.7.5.tar.gz",
			checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
			buildcmd=
				"cd libxml2-2.7.5 && "
				"%s --disable-dependency-tracking && "
				"make $(MAKEFLAGS) && "
				"make install" % MAC104_COMMON_CONFIGURE
			)
		],
	'linux' : [
		TPP("libtool", 
			url="http://ftp.gnu.org/gnu/libtool/libtool-2.2.6a.tar.gz",
			checkcmd="test -f %s/lib/libltdl.a" % COMMON_INSTALLDIR,
			buildcmd=
				"cd libtool-2.2.6 && "
		                "%s --enable-ltdl-install &&"
				"make $(MAKEFLAGS) && "
				"make install" % LINUX_COMMON_CONFIGURE
			),
		TPP("expat", 
			url="http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?use_mirror=autoselect",
			checkcmd="pkg-config --atleast-version=2.0.0 expat",
			buildcmd=
				"set -x;cd expat-2.0.1 && "
				"if [ ! -e expat.pc.in ] ; then patch < %s/third_party_packages/expat.patch; fi && "
				"autoconf && "
				"%s && "
				"make $(MAKEFLAGS) && "
				"make install" % (AMBULANT_DIR, LINUX_COMMON_CONFIGURE)
			),
		TPP("xerces-c",
			url="http://apache.mirror.transip.nl/xerces/c/3/sources/xerces-c-3.1.0.tar.gz",
			checkcmd="pkg-config --atleast-version=3.0.0 xerces-c",
			buildcmd=
				"cd xerces-c-3.1.0 && "
				"%s && "
				"make $(MAKEFLAGS) && "
				"make install" % (LINUX_COMMON_CONFIGURE)
			),
		TPP("faad2",
			url="http://downloads.sourceforge.net/project/faac/faad2-src/faad2-2.7/faad2-2.7.tar.gz?use_mirror=autoselect",
			checkcmd="test -f %s/lib/libfaad.a" % COMMON_INSTALLDIR,
			buildcmd=
				"cd faad2-2.7 && "
				"%s && "
				"make $(MAKEFLAGS) && "
				"make install" % LINUX_COMMON_CONFIGURE
			),
## xulrunner-sdk is only needed for building npambulant firefox plugin
		TPP("xulrunner-sdk",
			url="http://releases.mozilla.org/pub/mozilla.org/xulrunner/releases/1.9.2/sdk/xulrunner-1.9.2.en-US.linux-i686.sdk.tar.bz2",
			checkcmd="test -d xulrunner-sdk",
			buildcmd="test -d xulrunner-sdk"
			),
##		TPP("ffmpeg",
##			url="http://ffmpeg.org/releases/ffmpeg-0.5.tar.bz2",
##			checkcmd="pkg-config --atleast-version=52.20.0 libavformat",
##			buildcmd=
##				"cd ffmpeg-0.5 && "
##				"%s --enable-gpl --enable-libfaad --enable-swscale --enable-shared --extra-cflags=-I%s/include --extra-ldflags=-L%s/lib&&"
##				"make $(MAKEFLAGS) && "
##				"make install " % 
##					(LINUX_COMMON_CONFIGURE, COMMON_INSTALLDIR, COMMON_INSTALLDIR)
##			),
		TPP("ffmpeg",
##			url="http://homepages.cwi.nl/~jack/ambulant/ffmpeg-export-2010-01-22.tgz",
			url="http://sourceforge.net/projects/ambulant/files/ffmpeg%20for%20Ambulant/ffmpeg-export-2010-01-22.tar.gz/download",
			checkcmd="pkg-config --atleast-version=52.20.0 libavformat",
			buildcmd=
				"cd ffmpeg-export-2010-01-22 && "
				"%s --enable-gpl --enable-libfaad --enable-shared --extra-cflags=-I%s/include --extra-ldflags=-L%s/lib&&"
				"make install " % 
					(LINUX_COMMON_CONFIGURE, COMMON_INSTALLDIR, COMMON_INSTALLDIR)
			),
		TPP("SDL",
			url="http://www.libsdl.org/tmp/SDL-1.3.tar.gz",
			checkcmd="pkg-config --atleast-version=1.3.0 sdl",
			buildcmd=
				"cd SDL-1.3.0-* && "
				"%s &&"
				"make $(MAKEFLAGS) && "
				"make install" % (LINUX_COMMON_CONFIGURE)
			),
		TPP("live",
			url="http://www.live555.com/liveMedia/public/live555-latest.tar.gz",
			checkcmd="test -f ./live/liveMedia/libliveMedia.a",
			buildcmd=
				"cd live && "
		                "( grep fPIC config.linux >/dev/null || patch -i %s/third_party_packages/live.patch config.linux ) &&"
				"./genMakefiles linux && "
				"make $(MAKEFLAGS) " % (AMBULANT_DIR)
			),
		TPP("gettext",
			url="http://ftp.gnu.org/pub/gnu/gettext/gettext-0.17.tar.gz",
			checkcmd="test -d %s/lib/gettext -o -d /usr/lib/gettext" % COMMON_INSTALLDIR,
			buildcmd=
				"cd gettext-0.17 && "
				"%s --disable-csharp && "
				"make $(MAKEFLAGS) && "
				"make install" % LINUX_COMMON_CONFIGURE
			),
		TPP("libxml2",
			url="ftp://xmlsoft.org/libxml2/libxml2-2.7.5.tar.gz",
			checkcmd="pkg-config --atleast-version=2.6.9 libxml-2.0",
			buildcmd=
				"cd libxml2-2.7.5 && "
				"%s && "
				"make $(MAKEFLAGS) && "
				"make install" % LINUX_COMMON_CONFIGURE
			)
		],
	
}

def main():
	if len(sys.argv) != 2 or sys.argv[1] not in third_party_packages:
		print "Usage: %s platform" % sys.argv[0]
		print "Platform is one of:", ' '.join(third_party_packages.keys())
		return 2
	allok = True
	for pkg in third_party_packages[sys.argv[1]]:
		print "+ processing:", pkg.name
		ok = pkg.run()
		if ok:
			print "+ ok:", pkg.name
		else:
			print "+ failed:", pkg.name
			allok = False
	if not allok:
		return 1
	return 0
	
if __name__ == '__main__':
	sts = main()
	sys.exit(sts)
