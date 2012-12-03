#!/bin/sh
scriptdir=`dirname $0`
topdir=$scriptdir/../..
interactive=false
appname="Ambulant Player.app"
tmplappname="Ambulant Player.app"
if [ x$1 == x-a ]; then
	appname=$2
	shift
	shift
fi
dirname=$1
installroot=$2
if [ x$dirname == x ]; then
	echo Usage: $0 [-a appname] dirname [installroot]
	echo dirname is where the distribution will be created.
	echo Default for appname is \"AmbulantPlayer.app\"
	echo Something like Ambulant-1.8-mac is what we usually use.
	exit 2
fi
if [ -d $dirname ]; then
	echo $dirname already exists
	exit 1
fi
if [ -f $dirname.dmg ]; then
	echo $dirname.dmg already exists
	exit 1
fi
#
# Create the directory
#
mkdir $dirname
#
# Unpack the template (which contains icon position, etc)
#
(cd $dirname ; tar xf ../foldertemplate.tar; mv placeholder.app $appname)
#
# Copy all other files
#
cp $topdir/README $dirname/README
rm -rf "$dirname/$tmplappname"
cp -r "$installroot/Applications/$appname" "$dirname/$tmplappname"
case $appname in
"$tmplappname") ;;
*) mv "$dirname/tmplappname" "$dirname/$appname"
   ;;
esac
ln -s /Applications $dirname/Applications
cp -r $topdir/Extras/DemoPresentation $dirname/DemoPresentation
find $dirname/DemoPresentation -name 'CVS' -a -exec rm -r '{}' ';'
cp $topdir/COPYING $dirname/COPYING
#mkdir $dirname/.folderbg
#cp $scriptdir/folderbg.png $dirname/.folderbg/folderbg.png

hdiutil create -srcfolder $dirname $dirname-rw.dmg -format UDRW -attach
if $interactive; then
	echo Please give the folder the layout you want, select bgimage from \".folderbg\", unmount, press return.
	read a
	
else
	osascript - << END-OF-SCRIPT
tell application "Finder"
	open disk "$dirname"
	set the background picture of icon view options of Finder window 1 to document file "folderbg.png" of item ".folderbg" of disk "$dirname"
	get the background picture of icon view options of Finder window 1
	set the background picture of icon view options of Finder window 1 to document file "folderbg.png" of item ".folderbg" of disk "$dirname"
	get the background picture of icon view options of Finder window 1
end tell
END-OF-SCRIPT
	sleep 2
	hdiutil detach /Volumes/$dirname
	sleep 2
fi
hdiutil convert $dirname-rw.dmg -format UDZO -o $dirname.dmg
rm $dirname-rw.dmg
rm -r $dirname
