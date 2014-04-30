# prepare-ndk-build.sh
#
# This script downloads external third party packages, links and patches
# some files as needed to build AmbulantSDLPlayer for android
#
set -x
#
# Check we're at the correct place
#
# Check essential commands
#
if [ `basename $PWD` != jni ] ;
then echo "This script can only be used from 'jni' directory"; exit -1;
fi
if ! hash ndk-build 2>/dev/null
then echo "Command 'ndk-build' not found in PATH"; exit -1; fi
if  ! hash ant 2>/dev/null
then echo "Command 'ant' not found in PATH"; exit -1; fi
if  ! hash git 2>/dev/null
then echo "Command 'git' not found in PATH"; exit -1; fi
if  ! hash hg 2>/dev/null
then echo "Command 'hg' not found in PATH"; exit -1; fi
# download expat
git clone https://github.com/android/platform_external_expat expat
# download SDL
hg clone http://hg.libsdl.org/SDL SDL
# download SDL_image
hg clone http://hg.libsdl.org/SDL_image SDL_image
# download SDL_ttf
hg clone  http://hg.libsdl.org/SDL_ttf SDL_ttf
# cp Ambulant-Android.mk ambulant/Android.mk
cp expat-Android.mk expat/Android.mk
# link needed ambulant files
if [ ! -e ambulant ]
then
	mkdir -p ambulant
	AMBULANT=`(cd ../../../..;echo $PWD)`
	cp AmbulantToplevel-Android.mk ambulant/Android.mk
	ln -s $AMBULANT/include ambulant
	ln -s $AMBULANT/src ambulant
fi
# patch SDL_image
(cd SDL_image;patch -p1 < ../SDL_image.patch)
