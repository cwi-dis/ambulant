#!/bin/sh
# set -x # uncomment for debug

# By default, use wget to get URLs
wgetorcurl() {
    wget $1
}

case "$OSTYPE" in
linux-gnu)
    AMBULANT_PLAYER=AmbulantPlayer 
    AMBULANT_EXTRAS=$HOME/share/ambulant
    ;;
cygwin)
    AMBULANT_EXTRAS=/cygdrive/c/Program\ Files/AmbulantPlayer-2.0/Extras
	AMBULANT_PLAYER=/cygwin/c/Program\ Files/AmbulantPlayer-2.0/AmbulantPlayer.exe
	;;
darwin*)
    AMBULANT_DEMOS=/Volumes/Ambulant-2.0.2-mac/DemoPresentation
    AMBULANT_PLAYER=/Applications/Ambulant\ Player.app/Contents/MacOS/Ambulant-mac
    AMBULANT_WELCOME=/Applications/Ambulant\ Player.app/Contents/Resources/Welcome.smil
    
    wgetorcurl() {
        curl -o $2 $1
    }   
    ;;
*)
    echo Unsupported platform \"$OSTYPE\", please edit Run-tests.sh
    exit 1
    ;;
esac 

if test ! -f "$AMBULANT_PLAYER" ; then
    echo Cannot find ambulant player in $AMBULANT_PLAYER
    echo Edit Run-tests.sh
    exit 1
fi

#
# test all sepcified files/urls 
testall() {
       for i in $* ; do
		   echo $i
		   "$AMBULANT_PLAYER" $i
	   done
}

# get specified zip file from web, unpack and test all contained .smil files
testzip() {
	dir=`basename $1 .zip`
	echo wget:  $1 $dir
	wgetorcurl $1 $dir.zip
	zipfile=$dir.zip
	mkdir -p $dir
	cd $dir
	unzip -qq ../$zipfile
#	echo *.smil
	testall *.smil
	cd ..
	rm -fr $zipfile $dir
}


if test -d "$AMBULANT_EXTRAS" ; then
    (cd "$AMBULANT_EXTRAS/Welcome";testall *.smil)
    (cd "$AMBULANT_EXTRAS/DemoPresentation";testall *.smil)
elif test -d "$AMBULANT_DEMOS" ; then
    for i in "$AMBULANT_DEMOS"/*.smil
    do
        testall $i
    done
else
    echo Warning: Cannot find DemoPresentation, edit Run-tests.sh
fi

testzip http://www.ambulantplayer.org/Demos/smilText.zip
testzip http://www.ambulantplayer.org/Demos/PanZoom.zip
testzip http://www.ambulantplayer.org/Demos/VideoTests.zip
testzip http://www.ambulantplayer.org/Demos/Birthday.zip
testzip http://www.ambulantplayer.org/Demos/Euros.zip
testzip http://www.ambulantplayer.org/Demos/Flashlight.zip
testzip http://www.ambulantplayer.org/Demos/News.zip

testall http://www.ambulantplayer.org/Demos/smilText/NYC-sT.smil
testall http://www.ambulantplayer.org/Demos/smilText/NYC-sT-rtsp.smil
testall http://www.ambulantplayer.org/Demos/PanZoom/Fruits-4s.smil
testall http://www.ambulantplayer.org/Demos/VideoTests/VideoTests.smil
testall http://www.ambulantplayer.org/Demos/VideoTests/VideoTests-rtsp.smil
testall http://www.ambulantplayer.org/Demos/Birthday/HappyBirthday.smil
testall http://www.ambulantplayer.org/Demos/Birthday/HappyBirthday-rtsp.smil
testall http://www.ambulantplayer.org/Demos/Euros/Euros.smil
testall http://www.ambulantplayer.org/Demos/Euros/Euros-rtsp.smil 
testall http://www.ambulantplayer.org/Demos/Flashlight/Flashlight-US.smil
testall http://www.ambulantplayer.org/Demos/Flashlight/Flashlight-US-rtsp.smil
testall http://www.ambulantplayer.org/Demos/News/DanesV2-Desktop.smil
testall http://www.ambulantplayer.org/Demos/News/DanesV2-Desktop-rtsp.smil

echo $0 " finished"
