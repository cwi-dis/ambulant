SUFFIXES=-moc.cpp
-moc.cpp.cpp:
	moc -c -o $@ $<
if WITH_QT
build_player_unix=player_unix
else
build_player_unix=
endif
if WITH_COCOA
# Cannot built Cocoa player with automake yet
build_player_cocoa=
else
build_player_cocoa=
endif
SUBDIRS = libambulant $(build_player_unix) $(build_player_cocoa)
DIST_SUBDIRS = libambulant player_unix player_cocoa

