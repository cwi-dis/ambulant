SUFFIXES=-moc.cpp
-moc.cpp.cpp:
	$(QT_PREFIX)/bin/moc -c -o $@ $<
if WITH_QT
build_player_unix=player_unix
else
build_player_unix=
endif
if WITH_COCOA
build_player_macosx=player_macosx
else
build_player_macosx=
endif
SUBDIRS = libambulant $(build_player_unix) $(build_player_macosx)
DIST_SUBDIRS = libambulant player_unix player_macosx

