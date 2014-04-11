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
# copy needed ambulant files
if [ ! -e ambulant ]
then
	AMBULANT=`pwd`/../../../../../ambulant
	mkdir -p ambulant
	cp -r $AMBULANT/include ambulant 
	cp -r $AMBULANT/src ambulant 
	cp -r $AMBULANT/Android.mk ambulant
fi
# patch SDL_image
(cd SDL_image;patch -p1 < ../SDL_image.patch)
# patch the ambulant source to be NDK-compilable
# patch -p1<ambulant-android.diff
