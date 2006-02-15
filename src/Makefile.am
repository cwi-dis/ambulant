SUFFIXES=-moc.cpp
-moc.cpp.cpp:
	$(QT_PREFIX)/bin/moc -c -o $@ $<
if WITH_QT
build_player_unix=player_unix
else
build_player_unix=
endif
if WITH_GTK
build_player_gtk=player_gtk
else
build_player_gtk=
endif
if WITH_COCOA
build_player_macosx=player_macosx
else
build_player_macosx=
endif
if WITH_PYTHON
build_pyambulant=pyambulant
else
build_pyambulant=
endif
SUBDIRS = libambulant plugins $(build_player_unix) $(build_player_gtk) $(build_player_macosx) $(build_pyambulant)
DIST_SUBDIRS = libambulant plugins player_unix  player_gtk  player_macosx pyambulant
EXTRA_DIST= \
	Ambulant-win32.sln \
	Ambulant-wince.vcw \
	player_mfc \
	player_wince
