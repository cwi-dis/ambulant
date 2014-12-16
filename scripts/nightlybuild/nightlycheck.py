import sys
import os
import urllib2
import time


USERS=["Jack.Jansen@cwi.nl", "Kees.Blom@cwi.nl"]

LOG_URL="http://ambulantplayer.org/nightlybuilds/logs/default-%(platform)s-%(ydate)s.txt"

URLS={
    'mac' : [
        "http://ambulantplayer.org/nightlybuilds/default/mac-intel-desktop-cg/Ambulant-2.5.%(ydate)s-default-mac.dmg",
        "http://ambulantplayer.org/nightlybuilds/default/mac-intel-firefoxplugin/npambulant-2.5.%(ydate)s-default-mac.dmg",
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

    'linux-i686' : [
        "http://ambulantplayer.org/nightlybuilds/default/src/ambulant-2.5.%(ydate)s-default.tar.gz",
        "http://ambulantplayer.org/nightlybuilds/default/linux-i686-firefoxplugin/npambulant-2.5.%(ydate)s-default-linux-i686.xpi",
    ],
    'linux-x86_64' : [
        "http://ambulantplayer.org/nightlybuilds/default/src/ambulant-2.5.%(ydate)s-default.tar.gz",
        "http://ambulantplayer.org/nightlybuilds/default/linux-i686-firefoxplugin/npambulant-2.5.%(ydate)s-default-linux-i686.xpi",
        "http://ambulantplayer.org/nightlybuilds/default/linux-x86_64-firefoxplugin/npambulant-2.5.%(ydate)s-default-linux-x86_64.xpi",
    ],
    
    'Ubuntu-14.04-source' : [
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty.dsc",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty.tar.gz",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty_amd64.build",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty_source.build",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/source/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty_source.changes",
    ],
    'Ubuntu-14.04-amd64' : [
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/ambulant-common_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/ambulant-gtk_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/ambulant-plugins-dev_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/ambulant-plugins_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/ambulant-python_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/ambulant-sdl_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/libambulant_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/libambulant-dev_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty.dsc",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-amd64/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty_amd64.changes",
    ],
    'Ubuntu-14.04-i386' : [
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/ambulant-common_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/ambulant-gtk_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/ambulant-plugins-dev_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/ambulant-plugins_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/ambulant-python_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/ambulant-sdl_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/libambulant_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/libambulant-dev_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty.dsc",
        "http://ambulantplayer.org/nightlybuilds/default/deb/dists/14.04/ambulant/binary-i386/debian-%(ydate)s/ambulant_2.5.%(ydate)s%%7Etrusty_i386.changes",
    ],
    
    'Ubuntu-14.04-amd64-PPA' : [
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-common_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-gtk_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-plugins-dev_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-plugins_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-python_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-sdl_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/libambulant_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/libambulant-dev_2.5.%(ydate)s%%7Etrusty_amd64.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant_2.5.%(ydate)s%%7Etrusty.dsc",
    ],
    'Ubuntu-14.04-i386-PPA' : [
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-common_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-gtk_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-plugins-dev_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-plugins_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-python_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant-sdl_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/libambulant_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/libambulant-dev_2.5.%(ydate)s%%7Etrusty_i386.deb",
        "http://ppa.launchpad.net/ambulant/ambulant-nightly/ubuntu/pool/main/a/ambulant/ambulant_2.5.%(ydate)s%%7Etrusty.dsc",
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
            "ydate" : time.strftime("%Y%m%d"),
            "platform" : name
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
            thislogfile = LOG_URL % vars
            lastgoodbuild = self.lastGoodBuild.get(name, "unknown")
            lastgoodlogfile = LOG_URL % dict(ydate=lastgoodbuild, platform=name)
            rv = "Platform %s did not build correctly.\n" % name
            rv += "Log: %s\n" % thislogfile
            rv += "Last correct build: %s, log: %s\n" % (lastgoodbuild, lastgoodlogfile)
            rv += "Missing files:\n"
            for url in urlsNotOK:
                rv += "\t%s\n" % url
            if urlsOK:
                rv += "NOTE: some files did build correctly:\n"
                for url in urlsOK:
                    rv += "\t%s\n" % url
            rv += "\n"
        else:
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
        correct = ""
        rv = ""
        for platform, urls in URLS.items():
            platformMessage = self.checkPlatform(platform, urls)
            if platformMessage:
                rv += platformMessage
            else:
                correct += " " + platform
        self.save()
        if correct:
            rv = "Plaforms built correctly: " + correct + "\n\n" + rv
        return rv
    
def main():
    checker = BuildChecker()
    rv = checker.check()
    print rv
    if rv:
        sys.exit(1)
        
if __name__ == '__main__':
    main()
    
