import sys
import os
import urllib2
import time


USERS=["Jack.Jansen@cwi.nl", "Kees.Blom@cwi.nl"]

URLS={
    'mac' : [
        "http://ambulantplayer.org/nightlybuilds/default/mac-intel-desktop-cg/Ambulant-2.5.%(ydate)s-default-mac.dmg",
        "http://ambulantplayer.org/nightlybuilds/default/mac-intel-firefoxplugin/npambulant-2.5.%(ydate)s-default-mac.dmg",
    ],
    'linux' : [
        "http://ambulantplayer.org/nightlybuilds/default/src/ambulant-2.5.%(ydate)s-default.tar.gz",
        "http://ambulantplayer.org/nightlybuilds/default/linux-i686-firefoxplugin/npambulant-2.5.%(ydate)s-default-linux-i686.xpi",
        "http://ambulantplayer.org/nightlybuilds/default/linux-x86_64-firefoxplugin/npambulant-2.5.%(ydate)s-default-linux-x86_64.xpi",
    ],
    'win32' : [
        "http://ambulantplayer.org/nightlybuilds/default/win32-intel-desktop/Ambulant-2.5.%(ydate)s-win32.exe",
        "http://ambulantplayer.org/nightlybuilds/default/win32-intel-firefoxplugin/npambulant-2.5.%(ydate)s-win32.xpi",
        "http://ambulantplayer.org/nightlybuilds/default/win32-intel-ieplugin/ieambulant-2.5.%(ydate)s-win32.cab",
        "http://ambulantplayer.org/nightlybuilds/default/win32-intel-ieplugin/ieambulant-2.5.%(ydate)s-win32.html",
    ],
    'iphone' : [
        "http://ambulantplayer.org/nightlybuilds/default/iphone/iAmbulant-2.5.%(ydate)s-default.ipa",
    ],
    'ubuntu 13.10 (source)' : [
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy.dsc",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy.tar.gz",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy_amd64.build",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy_source.build",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy_source.changes",
    ],
    'ubuntu 13.10 (amd64)' : [
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/ambulant-common_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/ambulant-gtk_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/ambulant-plugins-dev_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/ambulant-plugins_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/ambulant-python_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/ambulant-sdl_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/libambulant_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/libambulant-dev_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy.dsc",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-amd64/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy_amd64.changes",
    ],
    'ubuntu 13.10 (i386)' : [
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/ambulant-common_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/ambulant-gtk_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/ambulant-plugins-dev_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/ambulant-plugins_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/ambulant-python_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/ambulant-sdl_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/libambulant_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/libambulant-dev_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy.dsc",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/13.10/ambulant/binary-i386/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Esaucy_i386.changes",
    ],
    'ubuntu 13.10 (amd64 PPA)' : [
         "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-common_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-gtk_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-plugins-dev_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-plugins_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-python_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-sdl_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/libambulant_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/libambulant-dev_2.5.%(ydate)s%%7Esaucy_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant_2.5.%(ydate)s%%7Esaucy.dsc",
    ],
    'ubuntu 13.10 (i386 PPA)' : [
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-common_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-gtk_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-plugins-dev_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-plugins_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-python_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-sdl_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/libambulant_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/libambulant-dev_2.5.%(ydate)s%%7Esaucy_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant_2.5.%(ydate)s%%7Esaucy.dsc",
    ],
}

class BuildChecker:
    def __init__(self):
        self.lastGoodBuild = {}
        
    def checkURL(self, url):
        try:
            handle = urllib2.urlopen(url)
        except urllib2.URLError:
            return False
        return True
    
    def checkPlatform(self, name, urls):
        vars = {
            "ydate" : time.strftime("%Y%m%d")
            }
        urlsOK = []
        urlsNotOK = []
        for url in urls:
            url = url % vars
            if self.checkURL(url):
                urlsOK.append(url)
            else:
                urlsNotOK.append(url)
        rv = ""
        if urlsNotOK:
            rv = "Platform %s did not build correctly.\n" % name
            rv += "(Last correct build: %s)\n" % self.lastGoodBuild.get(name, "unknown")
            rv += "Missing files:\n"
            for url in urlsNotOK:
                rv += "\t%s\n" % url
            if urlsOK:
                rv += "NOTE: some files did build correctly:\n"
                for url in urlsOK:
                    rv += "\t%s\n" % url
            rv += "\n"
        self.lastGoodBuild[name] = vars["ydate"]
        return rv
    
    def load(self):
        filename = os.path.join(os.path.dirname(__file__), 'LastGoodBuild.txt')
        try:
            fp = open(filename)
            self.lastGoodBuild = eval(fp.read())
        except:
            pass
                
    def save(self):
        if not self.lastGoodBuild:
            return
        filename = os.path.join(os.path.dirname(__file__), 'LastGoodBuild.txt')
        fp = open(filename, 'w')
        fp.write(repr(self.lastGoodBuild) + '\n')
        
    def check(self):
        self.load()
        rv = ""
        for platform, urls in URLS.items():
            rv += self.checkPlatform(platform, urls)
        self.save()
        return rv
    
def main():
    checker = BuildChecker()
    rv = checker.check()
    print rv
    if rv:
        sys.exit(1)
        
if __name__ == '__main__':
    main()
    
