#!/bin/sh
# set -x
scriptdir=`dirname $0`
topdir=$scriptdir/../..
plugin_name="npambulant.plugin"
if [ x$1 == x-p ]; then
	plugin_name=$2
	shift
	shift
fi
dirname=$1
installroot=$2
plugin_srcdir="$installroot/Library/Internet Plug-Ins/$plugin_name"
if [ x$dirname == x ]; then
	echo Usage: $0 [-p pluginname] dirname [installroot]
	echo dirname is where the distribution will be created.
	echo Something like npambulant-2.4-mac is what we usually use.
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
if [ ! -d "$plugin_srcdir" ]; then
	echo "$plugin_name must be built and installed in $installroot before building the installer"
	exit 3
fi
#
# Create the directory
#
mkdir $dirname
#
# Unpack the template (which contains files needed and their icon position, etc)
#

(cd $dirname ; tar xf $scriptdir/plugintemplate.tar; mv placeholder.plugin $plugin_name)

#
# Copy the files, overwriting placeholders
#
cp -r $plugin_srcdir $dirname/$plugin_name
#
# Create temporary disk image (.dmg) and mount it
#
hdiutil create -srcfolder $dirname $dirname-rw.dmg -format UDRW -attach
#
# Modify the appearance of the new disk (size, background image)
#
osascript - << END-OF-SCRIPT
tell application "Finder"
    tell disk "$dirname"
	open 
        tell container window
            set current view to icon view
            set toolbar visible to false
            set statusbar visible to false
            set x to 100
            set y to 100
            set w to 640
            set h to 360
            set bounds to {x, y, x + w, y + h}
        end tell
	set opts to the icon view options of container window
     	tell opts
       	    set icon size to 64
            set shows icon preview to false
            set shows item info to false
            set icon size to 64
            set text size to 16
            set arrangement to not arranged
            set label position to bottom
     	end tell
       	set background picture of opts to file ".folderbg:folderbg.png"		
   end tell
end tell
END-OF-SCRIPT
#
# Done, unmount and compress the new disk
#
hdiutil detach /Volumes/$dirname
sleep 5
hdiutil convert $dirname-rw.dmg -format UDZO -o $dirname.dmg
#
# Cleanup
#
#rm $dirname-rw.dmg
#rm -r $dirname
