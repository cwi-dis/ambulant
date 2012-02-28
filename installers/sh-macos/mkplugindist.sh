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
# A slash is added here at the end to force the desired behaviour of the 'cp' program
plugin_srcdir="$installroot/Library/Internet Plug-Ins/$plugin_name/"
#
# Check arguments and build environment sanity
#
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
# This installer is constructed from a template 'npambulant-template.dmg', which was created
# using 'Disk Utility', were its Volume name (npambulant-plugin) was defined (read-write).
# The background image was defined by creating and copying an initial background image
# in /Volumes/npambulant-plugin/.background/background.png.
# Next, in 'Terminal' the command 'open  /Volumes/npambulant-plugin/.background' and using the 'Finder'
# window 'npambulant-plugin' select 'View->Show View Option'. Then, in the 'View Options' window
# under 'Background' select 'Image' and drag file 'backgound.png' from the '.background' window
# into the provided space.
# Finally, the disk 'npambulant-plugin' was 'Eject'ed and its corresponding .dmg was compressed using:
# 'hdiutil convert npambulant-plugin-rw.dmg -format UDZO -o npambulant-plugin.dmg'
# The template has all visuals (icons and background), but the files are empty.
# Their contents are copied in by executing this script.
# All content files are easily modifyable and under version control.
# To change the disk layout, repeat the disk creation procedure described above.
#
# This method seems to be more reliable than fiddling appearnace using 'osascript', because some
# 'Finder' versions seem to use "optimized" techniques to handle folder backgrounds, etc. Based on:
# http://stackoverflow.com/questions/96882/how-do-i-create-a-nice-looking-dmg-for-mac-os-x-using-command-line-tools, 8th answer
#
# Create the new .dmg by copying the template
#
cp npambulant-template.dmg $dirname.dmg 
#
# Convert it into a writable image and attach it to the file system (mount)
#
hdiutil convert $dirname.dmg -format UDRW -o $dirname-rw.dmg
hdiutil attach $dirname-rw.dmg
#
# Copy the files, overwriting placeholders
#
cp npambulant-installer-README /Volumes/npambulant-plugin/README
cp npambulant-installer-bg.png /Volumes/npambulant-plugin/.background/background.png
# Copy recursively the Contents of $plugin_srcdir to the target directory
cp -R "$plugin_srcdir" /Volumes/npambulant-plugin/npambulant.plugin
#
# Done, detach and compress the new disk
#
hdiutil detach /Volumes/npambulant-plugin
rm $dirname.dmg
sleep 5
hdiutil convert $dirname-rw.dmg -format UDZO -o $dirname.dmg
#
# Cleanup
#
rm $dirname-rw.dmg

