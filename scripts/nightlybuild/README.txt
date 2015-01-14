This directory contains the scripts and auxiliary files to do nightly builds of Ambulant.
It is only directly useful for the Ambulant folks at CWI, because some of the required
components (such as certificates for signing installers) are obviously not available
outside the group. But if you want to fork Ambulant you'll just have to change
references to use your own certificates:-)

As of this writing, nightly builds run on VMs, which are all hosted
on maunakea.

debian and linux:
	Various Parallels VMs on maunakea, user nightlybuilds.
	We do 32 and 64 bit builds for the most recent Ubuntu release and the
	most recent Ubuntu LTS release. Machine names have the form
	Ubuntu-1404-64bit. Ubuntu username is "nightlybuilds" with the well-known
	password. (NOTE: there may still be 2 machines that use user "jack").
	
	Should run on any Ubuntu machine that has the right PGP key (with
	the right passphrase).
	
	To prepare a new machine (for a new release, for example) do the following.
	Please update these instructions (and any mentioned script) if things need
	updating.
	
	- Install virgin Ubuntu
	- Create user nightlybuilds, well-know password, autologin, no locking
	- Use "sudo scripts/precheck-ubuntu.sh debinstall" to install needed packages
	- Set hostname to maunakea-ubuntu1404-64 (in /etc/hostname AND /etc/hosts)
	- Add alias for IP address to maunakea ~nightlybuilds/.ssh/config
	- Copy .ssh from maunakea to the new VM
	- cd src; hg clone ssh://hg@ambulantplayer.org/hg/ambulant
	- cd src; hg clone ssh://hg@ambulantplayer.org/hgpriv/ambulant-private
	- Get PGP key installed (see ambulant-private/certificates/pgp)
	- add ppa-repositories (and apt-get update)
	  - If you get key errors use "sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys ..."
	    to get the missing keys
	- build-third-party-packages debian
	- test a normal ambulant build
	- Test nightly build (both linux and debian)
	- Get mail to work. You have to edit /etc/postfix/main.cf and possibly
	  /etc/mailname. Compare with an ubuntu that is known to work.
	- Create correct crontab

mac and iphone:
	Another Parallels VM running OSX 10.7 currently, on maunakea. User nightlybuilds.
	To prepare a new VM (for example a new OSX release):
	- Create Parallels VM from recovery partition. Sign in to appstore/icloud
	  using Jack's account (jackjansen@mac.com).
	- Install convenience tools such as TextWrangler and its commandline tools
	- Install XCode, open, accept license, download updates
	- install command line tools
	- install Mercurial, copy or create ~/.hgrc
	- copy .ssh from another nightlybuilds machine
	- clone ssh://hg@ambulantplayer.org/hg/ambulant into src/ambulant
	- clone ssh://hg@ambulantplayer.org/hgpriv/ambulant-private into src/ambulant-private
	- create directory ~/packages and from there run (or inspect) scripts/precheck-mac
	- Install certificates from ambulant-private/certificates/xcode
	- Create nightlybuilds keychain, fix password, copy certificates across, set keychain optiosn to never lock.
	- run ambulant autogen.sh
	- try building third-party-packages
	- try building ambulant
	- try running nightlybuild scripts (mac and iphone)
	- Get mail working, fix /etc/postfix/main.cf (see maunakea or another mnightlybuild mac)
	- try nightlyrun scripts, which do funky things with ssh localhost
	- add crontab
	
win32:
	"Win7-VS2010-Nightly" Parallels VM on maunakea. This has the nightlyrun.win32.bat
	script sitting in "My Documents" of user Jack. Then, in "Computer Management",
	System Tools->Task Scheduler->Task Scheduler Library, there's a task "Nightly Build"
	which has the action to run the script, with a trigger of Jack logging in (which
	happens automatically on boot). It also has the "run with highest privileges" flag set.
	There may be other things in this VM that are important that I forgot:-)

nightlyclean:
	Maunakea itself, user nightlybuilds. Runs end-of-day just before midnight.
	
nightlycheck:
	Maunakea itself, user nightlybuilds. Runs after all other builds.
	
nightlytestsuite:
	XXX Don't think this is used at the moment.
	
nightlychangelog:
	XXX Absolutely no idea.
	
XXX There is also something somewhere that pushes the hg repository to sourceforge.
