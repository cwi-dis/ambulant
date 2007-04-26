#!/bin/sh
scriptdir=`dirname $0`
topdir=$scriptdir/../..
dirname=$1
if [ x$dirname == x ]; then
	echo Usage: $0 dirname
	echo dirname is where the distribution will be created.
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
mkdir $dirname

cp $topdir/README $dirname/README
cp -r "/Applications/Ambulant Player.app" "$dirname/Ambulant Player.app"
ln -s /Applications $dirname/Applications
cp -r $topdir/Extras/DemoPresentation $dirname/DemoPresentation
find $dirname/DemoPresentation -name 'CVS' -a -exec rm -r '{}' ';'
cp $topdir/COPYING $dirname/COPYING
mkdir $dirname/.folderbg
cp $scriptdir/folderbg.png $dirname/.folderbg/folderbg.png

hdiutil create -srcfolder $dirname $dirname-rw.dmg -format UDRW -attach
echo Please give the folder the layout you want, select bgimage from \".folderbg\", unmount, press return.
read a

hdiutil convert $dirname-rw.dmg -format UDZO -o $dirname.dmg
rm $dirname-rw.dmg
rm -r $dirname
