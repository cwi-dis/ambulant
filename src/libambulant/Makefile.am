SUBDIRS=pc-uninstalled
ambulantlibdir=./.libs
if WITH_AMBULANT_SHARED_LIB
#
# If we link with shared libs: use these. Needed for plugin support.
#
libambulant_core=-L$(ambulantlibdir) -lambulant
if WITH_FFMPEG
libambulant_ffmpeg=-L$(ambulantlibdir) -lambulant_ffmpeg
else
libambulant_ffmpeg=
endif
libambulant_sdl=-L$(ambulantlibdir) -lambulant_sdl
libambulant_gstreamer=-L$(ambulantlibdir) -lambulant_gstreamer
libambulant_gtk=-L$(ambulantlibdir) -lambulant_gtk
else
#
# Otherwise we link the ambulant libraries statically, and we
# do not include these here (the final executable will
# link with them).
#
libambulant_core=
libambulant_ffmpeg=
libambulant_sdl=
libambulant_gstreamer=
libambulant_gtk=
endif

# See which toolkit-dependent libraries we want to make


if WITH_EXPAT
expat_parser_source = lib/expat_parser.cpp
expat_core_libs = $(EXPAT_LIBS)
else
expat_parser_source = 
expat_core_libs =
endif

if WITH_XERCES_BUILTIN
xerces_parser_source = lib/xerces_parser.cpp
xerces_includes = $(XERCES_CFLAGS)
xerces_core_libs = $(XERCES_LIBS)
else
xerces_parser_source = 
xerces_includes =
xerces_core_libs = 
endif

if WITH_GCD_EVENT_PROCESSOR
gcd_event_processor_core_libs = $(GCD_LIBS)
else
gcd_event_processor_core_libs = 
endif

if WITH_CG
toolkit_cg_libraries = libambulant_cg.la
# XXX Should LIBINTL depend on USE_NLS?
platlibs = $(LIBINTL) -framework CoreFoundation
libambulant_cg_la_SOURCES = \
	gui/cg/cg_fill.m \
	gui/cg/cg_gui.m \
	gui/cg/cg_image.m \
	gui/cg/cg_renderer.m \
	gui/cg/cg_smiltext.m \
	gui/cg/cg_transition.m \
	gui/cg/cg_text.m \
	gui/cg/cg_dsvideo.m
libambulant_cg_la_LDFLAGS=$(libambulant_core)
libambulant_cg_la_DEPENDENCIES=libambulant.la
else
toolkit_cg_libraries =
platlibs =
endif
extradist_cg = \
    gui/cg/atsui_text.m \
	gui/cg/cg_fill.m \
	gui/cg/cg_gui.m \
	gui/cg/cg_image.m \
	gui/cg/cg_renderer.m \
	gui/cg/cg_smiltext.m \
	gui/cg/cg_text.m \
	gui/cg/cg_transition.m \
	gui/cg/cg_dsvideo.m \
	gui/cg/atsui_text.mm \
	gui/cg/cg_avfoundation_video.mm \
	gui/cg/cg_fill.mm \
	gui/cg/cg_gui.mm \
	gui/cg/cg_image.mm \
	gui/cg/cg_renderer.mm \
	gui/cg/cg_smiltext.mm \
	gui/cg/cg_text.mm \
	gui/cg/cg_transition.mm \
	gui/cg/cg_dsvideo.mm

if WITH_GTK
toolkit_gtk_libraries = libambulant_gtk.la
gtkincludes =$(GTK_CFLAGS)

libambulant_gtk_la_SOURCES = \
	gui/gtk/gtk_renderer.cpp \
	gui/gtk/gtk_fill.cpp \
	gui/gtk/gtk_image_renderer.cpp \
	gui/gtk/gtk_smiltext.cpp \
	gui/gtk/gtk_text_renderer.cpp \
	gui/gtk/gtk_transition.cpp \
	gui/gtk/gtk_util.cpp \
	gui/gtk/gtk_video_renderer.cpp \
	gui/gtk/gtk_factory.cpp
libambulant_gtk_la_LDFLAGS=$(libambulant_ffmpeg) $(libambulant_core) $(GTK_LIBS) $(gcd_event_processor_core_libs)
libambulant_gtk_la_DEPENDENCIES=$(toolkit_ffmpeg_libraries) libambulant.la
else
gtkincludes=
toolkit_gtk_libraries =
libambulant_gtk_la_SOURCES =
endif

if WITH_SDL2
toolkit_sdl_libraries = libambulant_sdl.la
libambulant_sdl_la_LDFLAGS =  $(SDL_LIBS) $(FFMPEG_LIBS) $(libambulant_core) 
libambulant_sdl_la_DEPENDENCIES=libambulant.la
if WITH_SDL_PANGO
sdlincludes = -DWITH_SDL2 -DWITH_SDL_PANGO $(SDL_CFLAGS)
else
if WITH_SDL_TTF
sdlincludes = -DWITH_SDL2 -DWITH_SDL_TTF $(SDL_CFLAGS)
else
sdlincludes = -DWITH_SDL2 $(SDL_CFLAGS)
endif
endif
libambulant_sdl_la_SOURCES =          \
    gui/SDL/sdl.cpp                   \
    gui/SDL/sdl_audio.cpp             \
    gui/SDL/sdl_factory.cpp           \
    gui/SDL/sdl_fill.cpp              \
    gui/SDL/sdl_image_renderer.cpp    \
    gui/SDL/sdl_renderer.cpp          \
    gui/SDL/sdl_pango_smiltext.cpp    \
    gui/SDL/sdl_ttf_smiltext.cpp      \
    gui/SDL/sdl_text_renderer.cpp     \
    gui/SDL/sdl_transition.cpp        \
    gui/SDL/sdl_window.cpp            \
    gui/SDL/sdl_video.cpp

#   gui/SDL/sdl_renderer.cpp 
else
if WITH_SDL
toolkit_sdl_libraries = libambulant_sdl.la
libambulant_sdl_la_LDFLAGS =  $(SDL_LIBS) $(libambulant_core) 
libambulant_sdl_la_DEPENDENCIES=libambulant.la
sdlincludes = -DWITH_SDL $(SDL_CFLAGS)
libambulant_sdl_la_SOURCES = \
    gui/SDL/sdl.cpp \
    gui/SDL/sdl_audio.cpp
else
sdlincludes=
toolkit_sdl_libraries =
endif
endif

if WITH_FFMPEG
toolkit_ffmpeg_libraries = libambulant_ffmpeg.la

libambulant_ffmpeg_la_LDFLAGS = $(FFMPEG_LIBS) $(libambulant_core)
libambulant_ffmpeg_la_DEPENDENCIES=libambulant.la

ffmpegincludes = -DWITH_FFMPEG $(FFMPEG_CFLAGS)

libambulant_ffmpeg_la_SOURCES = \
    net/ffmpeg_common.cpp \
    net/ffmpeg_audio.cpp \
    net/ffmpeg_video.cpp \
    net/ffmpeg_raw.cpp \
    net/demux_datasource.cpp
else
ffmpegincludes=
toolkit_ffmpeg_libraries=
endif

if WITH_GSTREAMER
toolkit_gstreamer_libraries = libambulant_gstreamer.la

libambulant_gstreamer_la_LDFLAGS = -lgstreamer-0.10 $(libambulant_core)
libambulant_gstreamer_la_DEPENDENCIES=libambulant.la
gstreamer_includes = \
    -DWITH_GSTREAMER \
    -I/usr/include/gstreamer-0.10 \
    -I/usr/include/glib-2.0 \
    -I/usr/lib/glib-2.0/include \
    -I/usr/include/libxml2 \
    $(GSTREAMER_CFLAGS)

libambulant_gstreamer_la_SOURCES = \
    gui/gstreamer/gstreamer_audio_renderer.cpp \
    gui/gstreamer/gstreamer_renderer_factory.cpp \
    gui/gstreamer/gstreamer_player.cpp
else
gstreamer_includes=
toolkit_gstreamer_libraries=
endif

AM_CPPFLAGS = \
	-DAMBULANT_DATADIR=\"$(pkgdatadir)\" \
	-DAMBULANT_PLUGINDIR=\"$(pkglibdir)\" \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	$(EXPAT_INCLUDES) \
	$(xerces_includes) \
	$(gstreamer_includes) \
	$(gtkincludes) \
	$(sdlincludes) \
	$(ffmpegincludes) \
	$(INCLTDL)

lib_LTLIBRARIES = \
	$(toolkit_gstreamer_libraries) \
	$(toolkit_sdl_libraries) \
	$(toolkit_ffmpeg_libraries) \
	$(toolkit_cg_libraries) \
	$(toolkit_gtk_libraries) \
	libambulant.la

libambulant_la_LDFLAGS= \
	-rpath $(libdir) \
	-rpath $(libdir) \
	-export-dynamic $(LIBLTDL) \
	$(expat_core_libs) \
	$(xerces_core_libs) \
	$(platlibs) \
	$(PTHREAD_LIBS) \
	$(gcd_event_processor_core_libs)

libambulant_la_SOURCES = \
	get_version.cpp \
 \
	$(expat_parser_source) \
	$(xerces_parser_source) \
 \
	lib/colors.cpp \
	lib/delta_timer.cpp \
	lib/document.cpp \
	lib/event_processor.cpp \
	lib/gpaths.cpp \
	lib/logger.cpp \
	lib/nfa.cpp \
	lib/node_impl.cpp \
	lib/nscontext.cpp \
	lib/parselets.cpp \
	lib/string_util.cpp \
	lib/timer.cpp \
	lib/transition_info.cpp \
	lib/tree_builder.cpp \
 \
	lib/parser_factory.cpp \
	lib/unix/unix_mtsync.cpp \
	lib/unix/unix_thread.cpp \
	lib/unix/unix_timer.cpp \
 \
	net/posix_datasource.cpp \
	net/stdio_datasource.cpp \
	net/datasource.cpp \
	net/databuffer.cpp \
	net/url.cpp \
 \
	common/factory.cpp \
	common/gui_player.cpp \
	common/preferences.cpp \
	common/region.cpp \
	common/renderer_impl.cpp \
	common/renderer_select.cpp \
	common/schema.cpp \
	common/state.cpp \
	common/smil_alignment.cpp \
	common/smil_handler.cpp \
	common/plugin_engine.cpp \
	common/video_renderer.cpp \
 \
	smil2/animate_a.cpp \
	smil2/animate_e.cpp \
	smil2/animate_n.cpp \
	smil2/params.cpp \
	smil2/region_node.cpp \
	smil2/smiltext.cpp \
	smil2/smil_layout.cpp \
	smil2/smil_player.cpp \
	smil2/smil_time.cpp \
	smil2/sync_rule.cpp \
	smil2/test_attrs.cpp \
	smil2/time_attrs.cpp \
	smil2/time_calc.cpp \
	smil2/time_node.cpp \
	smil2/time_sched.cpp \
	smil2/time_state.cpp \
	smil2/timegraph.cpp \
	smil2/trace_player.cpp \
	smil2/transition.cpp \
 \
	gui/none/none_area.cpp \
	gui/none/none_gui.cpp \
	gui/none/none_video_renderer.cpp \
	gui/none/none_factory.cpp

EXTRA_DIST= \
    Ambulant.png \
    AmbulantDoc.png \
    lib/expat_parser.cpp \
    lib/xerces_parser.cpp \
	$(libambulant_sdl_la_SOURCES) \
	$(libambulant_cg_la_SOURCES) \
	$(extradist_cg) \
	$(libambulant_gtk_la_SOURCES) \
	$(libambulant_gstreamer_la_SOURCES) \
	gui/d2 \
	net/win32_datasource.cpp \
	lib/memfile.cpp \
	lib/win32

CLEANFILES=demux_datasource.loT ffmpeg_common.loT sdl.loT

install-data-local:
# install icon for use by the players
	mkdir -p $(DESTDIR)$(datadir)/ambulant/icons/
	cp $(srcdir)/Ambulant.png  $(DESTDIR)$(datadir)/ambulant/icons/
# install gnome document icon for mimetype application/smil
	mkdir -p $(DESTDIR)$(datadir)/icons/gnome/64x64/mimetypes
	cp $(srcdir)/AmbulantDoc.png $(DESTDIR)$(datadir)/icons/gnome/64x64/mimetypes/gnome-mime-application-smil.png
	mkdir -p $(DESTDIR)$(datadir)/icons/hicolor/64x64/mimetypes
	cp $(srcdir)/AmbulantDoc.png $(DESTDIR)$(datadir)/icons/hicolor/64x64/mimetypes/gnome-mime-application-smil.png
	mkdir -p $(DESTDIR)$(datadir)/icons/Humanity/mimes/64
	cp $(srcdir)/AmbulantDoc.png $(DESTDIR)$(datadir)/icons/Humanity/mimes/64/gnome-mime-application-smil.png
	$(LIBTOOL) --finish $(DESTDIR)$(libdir)
 
uninstall-local:
	-rm -f $(DESTDIR)$(datadir)/ambulant/icons/Ambulant.png
	-rm -f $(DESTDIR)$(datadir)/icons/gnome/64x64/mimetypes/gnome-mime-application-smil.png
	-rm -f $(DESTDIR)$(datadir)/icons/hicolor/64x64/mimetypes/gnome-mime-application-smil.png
	-rm -f $(DESTDIR)$(datadir)/icons/Humanity/mimes/64/gnome-mime-application-smil.png

if WITH_NPAMBULANT
# when selinux is enblaed, identify the installed shared library
install-exec-hook:
	export PATH=$$PATH:/sbin:/usr/sbin ; \
	which chcon selinuxenabled  && selinuxenabled 2>&1 1>/dev/null ; \
	if [ $$? == 0 ] ; then chcon -t texrel_shlib_t "$(DESTDIR)$(libdir)"/* ; fi
else
install-exec-hook:
endif # WITH_NPAMBULANT
