#!/bin/sh
case x$1 in
x)
	helpdir=`pwd`
	;;
*)
	helpdir=$1
	;;
esac
osascript << XYZZY
tell application "Finder"
    activate
    set machelpdir to POSIX file "$helpdir"
    set folderAlias to machelpdir as alias 
end tell

tell application "Apple Help Indexing Tool"
    activate 
    use tokenizer "English" 
    turn anchor indexing "on" 
    open folderAlias 
end tell
XYZZY