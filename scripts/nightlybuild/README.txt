This directory contains the scripts and auxiliary files to do nightly builds of Ambulant.
It is only directly useful for the Ambulant folks at CWI, because some of the required
components (such as certificates for signing installers) are obviously not available
outside the group. But if you want to fork Ambulant you'll just have to change
references to use your own certificates:-)

As of this writing, nightly builds run on the following machines:

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
	Another Parallels VM running OSX 10.8 currently, on maunakea. User nightlybuilds.
	To prepare a new VM (for example a new OSX release):
	- Create Parallels VM from recovery partition. Sign in to appstore/icloud
	  using Jack's account (jackjansen@mac.com).
	
	
mac and iphone (old):
	Jack's workstation, user nightlybuilds. Has especially crafter keychain file.
	NOTE: the key used for signing must be accessible without password,
	otherwise you will get an error when running from cron during code-signing.
	Then you should find the key in keychain access and enable all access to it.
	
	XXX need more detailed instructions on setting up.
	
win32:
	"Win7-VS2010-Nightly" Parallels VM on maunakea. This has the nightlyrun.win32.bat
	script sitting in "My Documents" of user Jack. Then, in "Computer Management",
	System Tools->Task Scheduler->Task Scheduler Library, there's a task "Nightly Build"
	which has the action to run the script, with a trigger of Jack logging in (which
	happens automatically on boot). It also has the "run with highest privileges" flag set.
	There may be other things in this VM that are important that I forgot:-)

nightlyclean:
	Jack's workstation, user nightlybuilds. Runs end-of-day just before midnight.
	
nightlycheck:
	Jack's workstation, user nightlybuilds. Runs after all other builds.
	
nightlytestsuite:
	XXX Don't think this is used at the moment.
	
nightlychangelog:
	XXX Absolutely no idea.
	
XXX There is also something somewhere that pushes the hg repository to sourceforge.
