# See which toolkit-dependent libraries we want to make



if WITH_LIVE 
toolkit_live_libraries = libambulant_live.la
libambulant_live_la_SOURCES = \
	net/rtsp_datasource.cpp \
	net/rtsp_factory.cpp

live_includes = \
    -I$(LIVE_ROOT)/BasicUsageEnvironment/include \
    -I$(LIVE_ROOT)/groupsock/include \
    -I$(LIVE_ROOT)/liveMedia/include \
    -I$(LIVE_ROOT)/UsageEnvironment/include

livelibs = \
    -L$(LIVE_ROOT)/liveMedia/ -lliveMedia \
    -L$(LIVE_ROOT)/BasicUsageEnvironment/ -lBasicUsageEnvironment \
    -L$(LIVE_ROOT)/groupsock/ -lgroupsock \
    -L$(LIVE_ROOT)/UsageEnvironment/ -lUsageEnvironment
else
live_includes =
livelibs =
toolkit_live_libraries =
endif



if WITH_EXPAT
expat_parser_source = lib/expat_parser.cpp
expatlibs = $(EXPAT_LIBS)
else
expat_parser_source = 
expatlibs =
endif

if WITH_XERCES_BUILTIN
xerces_parser_source = lib/xerces_parser.cpp
xerces_includes = $(XERCES_INCLUDES)
else
xerces_parser_source = 
xerces_includes =
endif

if WITH_COCOA
toolkit_cocoa_libraries = libambulant_cocoa.la
# XXX Should LIBINTL depend on USE_NLS?
platlibs = $(LIBINTL) -framework WebKit -framework QTKit -framework CoreFoundation
libambulant_cocoa_la_SOURCES = \
	gui/cocoa/cocoa_fill.m \
	gui/cocoa/cocoa_gui.m \
	gui/cocoa/cocoa_html.m \
	gui/cocoa/cocoa_image.m \
	gui/cocoa/cocoa_ink.m \
	gui/cocoa/cocoa_text.m \
	gui/cocoa/cocoa_renderer.m \
	gui/cocoa/cocoa_smiltext.m \
	gui/cocoa/cocoa_transition.m \
	gui/cocoa/cocoa_audio.m \
	gui/cocoa/cocoa_video.m \
	gui/cocoa/cocoa_dsvideo.m
else
toolkit_cocoa_libraries =
platlibs =
endif

if WITH_CG
toolkit_cg_libraries = libambulant_cg.la
# XXX Should LIBINTL depend on USE_NLS?
platlibs = $(LIBINTL) -framework CoreGraphics -framework CoreFoundation
libambulant_cg_la_SOURCES = \
	gui/cg/cg_fill.m \
	gui/cg/cg_gui.m \
	gui/cg/cg_image.m \
	gui/cg/cg_renderer.m \
	gui/cg/cg_text.m \
	gui/cg/cg_dsvideo.m
#	gui/cg/cg_html.m
#	gui/cg/cg_ink.m
#	gui/cg/cg_smiltext.m
#	gui/cg/cg_transition.m
#	gui/cg/cg_audio.m
#	gui/cg/cg_video.m
#	gui/cg/atsui_text.m # No longer works (if it ever did...)
else
toolkit_cg_libraries =
platlibs =
endif

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

else
gtkincludes=
toolkit_gtk_libraries =
libambulant_gtk_la_SOURCES =
endif



if WITH_QT
toolkit_qt_libraries = libambulant_qt.la
qtincludes = $(LIBQT_INCLUDES)

libambulant_qt_la_SOURCES = \
	gui/qt/qt_renderer.cpp \
	gui/qt/qt_fill.cpp \
	gui/qt/qt_image_renderer.cpp \
	gui/qt/qt_smiltext.cpp \
	gui/qt/qt_text_renderer.cpp \
	gui/qt/qt_transition.cpp \
	gui/qt/qt_util.cpp \
	gui/qt/qt_video_renderer.cpp \
	gui/qt/qt_factory.cpp

else
qtincludes=
toolkit_qt_libraries =
libambulant_qt_la_SOURCES =
endif

if WITH_QT_HTML_WIDGET
libambulant_qt_la_SOURCES += \
	gui/qt/qt_html_renderer.cpp
endif

if WITH_SDL
toolkit_sdl_libraries = libambulant_sdl.la
libambulant_sdl_la_LDFLAGS =  $(SDL_LIBS)
sdlincludes = -DWITH_SDL $(SDL_CFLAGS)
libambulant_sdl_la_SOURCES = \
    gui/SDL/sdl.cpp \
    gui/SDL/sdl_audio.cpp
else
sdlincludes=
toolkit_sdl_libraries =
endif

if WITH_FFMPEG
toolkit_ffmpeg_libraries = libambulant_ffmpeg.la

libambulant_ffmpeg_la_LDFLAGS = $(FFMPEG_LIBS)

ffmpegincludes = -DWITH_FFMPEG -DWITH_FFMPEG_AVFORMAT $(FFMPEG_CFLAGS)

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

libambulant_gstreamer_la_LDFLAGS = -lgstreamer-0.10
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

if WITH_NOKIA770
nokia770_includes=-DWITH_NOKIA770 #$(HILDON_CFLAGS) $(OSSO_CFLAGS)
toolkit_nokia770_libraries= #$(OSSO_LIBS)
else
nokia770_includes=
toolkit_nokia770_libraries=
endif # WITH_NOKIA770


AM_CPPFLAGS = \
	-DAMBULANT_DATADIR=\"$(pkgdatadir)\" \
	-DAMBULANT_PLUGINDIR=\"$(pkglibdir)\" \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	$(EXPAT_INCLUDES) \
	$(xerces_includes) \
	$(qtincludes) \
	$(gstreamer_includes) \
	$(gtkincludes) \
	$(nokia770_includes) \
	$(sdlincludes) \
	$(ffmpegincludes) \
	$(live_includes) \
	$(INCLTDL)

lib_LTLIBRARIES = \
	libambulant.la \
	$(toolkit_cocoa_libraries) \
	$(toolkit_cg_libraries) \
	$(toolkit_qt_libraries) \
	$(toolkit_gstreamer_libraries) \
	$(toolkit_gtk_libraries) \
	$(toolkit_sdl_libraries) \
	$(toolkit_nokia770_libraries) \
	$(toolkit_ffmpeg_libraries) \
	$(toolkit_live_libraries)

libambulant_la_LDFLAGS= \
	-export-dynamic $(LIBLTDL) \
	$(expatlibs) \
	$(livelibs) \
	$(platlibs)

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
	lib/node.cpp \
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
    lib/expat_parser.cpp \
    lib/xerces_parser.cpp \
	gui/SDL \
	gui/cocoa \
	gui/cg \
	gui/dx \
	gui/qt \
	gui/gstreamer \
	gui/gtk \
	net/win32_datasource.cpp \
	lib/memfile.cpp \
	lib/win32

CLEANFILES=demux_datasource.loT ffmpeg_common.loT sdl.loT

if WITH_NPAMBULANT
# when selinux is enblaed, identify the installed shared library
install-exec-hook:
	export PATH=$$PATH:/sbin:/usr/sbin ; \
	which chcon selinuxenabled  && selinuxenabled 2>1 1>/dev/null ; \
	if [ $$? == 0 ] ; then chcon -t texrel_shlib_t "$(DESTDIR)$(libdir)"/* ; fi
else
install-exec-hook:
endif # WITH_NPAMBULANT