#!/bin/sh
if [ x$1 = x ]; then
	echo Usage: $0 distname
	echo distname will be something like Ambulant-2.0-webkitplugin.
	exit
fi
if [ ! -f src/webkit_plugin/README ]; then
	echo Please run only in toplevel AMbulant source directory
	exit
fi
if [ ! -d ~/Library/Internet\ Plug-Ins/AmbulantWebKitPlugin.webplugin ]; then
	echo No plugin in ~/Library/Internet\ Plug-Ins/AmbulantWebKitPlugin.webplugin
	exit
fi
rm -rf $1
mkdir $1
cp -R ~/Library/Internet\ Plug-Ins/AmbulantWebKitPlugin.webplugin $1
cp README $1
zip -r $1.zip $1
