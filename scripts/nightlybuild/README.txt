This directory contains the scripts and auxiliary files to do nightly builds of Ambulant.
It is only directly useful for the Ambulant folks at CWI, because some of the required
components (such as certificates for signing installers) are obviously not available
outside the group. But if you want to fork Ambulant you'll just have to change
references to use your own certificates:-)

As of this writing, nightly builds run on the following machines:

debian:
	"Ubuntu-1104" Parallels VM on Jack's workstation.
	Should run on any Ubuntu machine that has the right PGP key (with
	the right passphrase).

iphone:
	Jack's workstation. Requires Jack to be logged in, so the iphone keys
	can be accessed (through Keychain).
	NOTE: the key used for signing must be accessible without password,
	otherwise you will get an error when running from cron during code-signing.
	Then you should find the key in keychain access and enable all access to it.

linux:
	desktop.cwi.nl and Ubuntu-1104 VM. The first machine builds the 64bit Firefox plugin,
	the latter the 32bit plugin. Should run on any linux machine.
	Here are the packages I know you need to install:
		curl
		devscripts
		pgpgpg
		gpgkeys
		chrpath
	Your pgp key for signing the firefox plugin should be accessible without
	password/user intervention.
	
mac:
	Jack's workstation. Should run on any Mac.

win32:
	"Win7-VS2010-Nightly" Parallels VM on Jack's workstation. This has the nightlyrun.win32.bat
	script sitting in "My Documents" of user Jack. Then, in "Computer Management",
	System Tools->Task Scheduler->Task Scheduler Library, there's a task "Nightly Build"
	which has the action to run the script, with a trigger of Jack logging in (which
	happens automatically on boot). It also has the "run with highest privileges" flag set.
	There may be other things in this VM that are important that I forgot:-)
