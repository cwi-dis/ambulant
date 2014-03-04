# download expat
git clone https://github.com/android/platform_external_expat expat
# download SDL
hg clone http://hg.libsdl.org/SDL SDL
# download SDL_image
hg clone http://hg.libsdl.org/SDL_image SDL_image
# download ambulant
hg clone http://hg.code.sf.net/p/ambulant/code ambulant
# download SDL_ttf
hg clone http://hg.libsdl.org/SDL_ttf SDL_ttf
# copy the build scripts
cp Ambulant-Android.mk ambulant/Android.mk
cp expat-Android.mk expat/Android.mk

# patch the ambulant source to be NDK-compilable
patch -p1<ambulant-android.diff
