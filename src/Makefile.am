SUFFIXES=-moc.cpp
-moc.cpp.cpp:
	$(QT_PREFIX)/bin/moc -c -o $@ $<
if WITH_GTK
build_player_gtk=player_gtk
else
build_player_gtk=
endif
if WITH_SDL_FRONTEND
build_player_sdl=player_sdl
else
build_player_sdl=
endif
if WITH_CG
build_player_macosx=player_macosx
else
build_player_macosx=
endif
if WITH_NPAMBULANT
build_npambulant=npambulant
else 
build_npambulant=
endif
if WITH_PYTHON
build_pyambulant=pyambulant
else
build_pyambulant=
endif
# NOTE: because of incomplete dependencies the order of building is important:
# 1. library
# 2. python module (optional)
# 3. plugins
# 4. applications
SUBDIRS = libambulant  $(build_pyambulant) plugins $(build_player_gtk)  $(build_player_sdl) $(build_player_macosx) $(build_npambulant)
DIST_SUBDIRS = libambulant plugins player_gtk player_sdl player_macosx pyambulant npambulant
EXTRA_DIST= \
	player_mfc \
	player_iphone \
	ieambulant
