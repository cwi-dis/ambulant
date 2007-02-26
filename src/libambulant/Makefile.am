# See which toolkit-dependent libraries we want to make



if WITH_LIVE 
toolkit_live_libraries = libambulant_live.la
libambulant_live_la_SOURCES = 	\
	net/rtsp_datasource.cpp \
	net/rtsp_factory.cpp
	
live_includes = -I $(LIVE_ROOT)/BasicUsageEnvironment/include \
		   -I$(LIVE_ROOT)/groupsock/include \
		   -I$(LIVE_ROOT)/liveMedia/include \
		   -I$(LIVE_ROOT)/UsageEnvironment/include
	
livelibs = -L$(LIVE_ROOT)/liveMedia/ -lliveMedia \
		  -L$(LIVE_ROOT)/BasicUsageEnvironment/ -lBasicUsageEnvironment  \
		  -L$(LIVE_ROOT)/groupsock/ -lgroupsock  \
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
	gui/cocoa/cocoa_text.m \
	gui/cocoa/cocoa_renderer.m \
	gui/cocoa/cocoa_transition.m \
	gui/cocoa/cocoa_audio.m \
	gui/cocoa/cocoa_video.m \
	gui/cocoa/cocoa_dsvideo.m
else
toolkit_cocoa_libraries =
platlibs =
endif

if WITH_GTK
toolkit_gtk_libraries = libambulant_gtk.la
gtkincludes =$(GTK_CFLAGS)

libambulant_gtk_la_SOURCES = 		\
	gui/gtk/gtk_renderer.cpp	\
	gui/gtk/gtk_fill.cpp		\
	gui/gtk/gtk_image_renderer.cpp	\
	gui/gtk/gtk_text_renderer.cpp 	\
	gui/gtk/gtk_transition.cpp 	\
	gui/gtk/gtk_video_renderer.cpp	\
	gui/gtk/gtk_factory.cpp

else
gtkincludes=
toolkit_gtk_libraries =
libambulant_gtk_la_SOURCES =
endif



if WITH_QT
toolkit_qt_libraries = libambulant_qt.la
qtincludes = -I$(QT_PREFIX)/include

libambulant_qt_la_SOURCES = 		\
	gui/qt/qt_renderer.cpp		\
	gui/qt/qt_fill.cpp		\
	gui/qt/qt_image_renderer.cpp	\
	gui/qt/qt_text_renderer.cpp 	\
	gui/qt/qt_transition.cpp 	\
	gui/qt/qt_video_renderer.cpp	\
	gui/qt/qt_factory.cpp

else
qtincludes=
toolkit_qt_libraries =
libambulant_qt_la_SOURCES =
endif

if WITH_QT_HTML_WIDGET
libambulant_qt_la_SOURCES += 		\
	gui/qt/qt_html_renderer.cpp
endif

if WITH_ARTS
toolkit_arts_libraries = libambulant_arts.la

libambulant_arts_la_LDFLAGS = `artsc-config --libs`
artsincludes = `artsc-config --cflags`
libambulant_arts_la_SOURCES =             \
	gui/arts/arts.cpp \
	gui/arts/arts_audio.cpp
else
artsincludes=
toolkit_arts_libraries =
endif

if WITH_SDL
toolkit_sdl_libraries = libambulant_sdl.la
libambulant_sdl_la_LDFLAGS =  -static ` sdl-config --libs`
sdlincludes = `sdl-config --cflags`
libambulant_sdl_la_SOURCES =             \
        gui/SDL/sdl.cpp \
        gui/SDL/sdl_audio.cpp
else
sdlincludes=
toolkit_sdl_libraries =
endif

if WITH_DIRAC
dirac_libraries=`pkg-config --libs dirac`
else
dirac_libraries=
endif

if WITH_FFMPEG
toolkit_ffmpeg_libraries = libambulant_ffmpeg.la

libambulant_ffmpeg_la_LDFLAGS = $(FFMPEG_LIBS) $(dirac_libraries)
ffmpegincludes = -DWITH_FFMPEG -DWITH_FFMPEG_AVFORMAT $(FFMPEG_CFLAGS)

libambulant_ffmpeg_la_SOURCES = 	\
		net/ffmpeg_common.cpp \
		net/ffmpeg_audio.cpp \
		net/ffmpeg_video.cpp \
		net/ffmpeg_raw.cpp	\
		net/demux_datasource.cpp
else
ffmpegincludes=
toolkit_ffmpeg_libraries=
endif

if WITH_GSTREAMER
toolkit_gstreamer_libraries = libambulant_gstreamer.la

libambulant_gstreamer_la_LDFLAGS = -lgstreamer-0.10
gstreamer_includes = -DWITH_GSTREAMER \
		-I/usr/include/gstreamer-0.10 \
		-I/usr/include/glib-2.0	\
		-I/usr/lib/glib-2.0/include \
		-I/usr/include/libxml2 \
		 $(GSTREAMER_CFLAGS)

libambulant_gstreamer_la_SOURCES = 	\
	        gui/gstreamer/gstreamer_audio_renderer.cpp	\
	        gui/gstreamer/gstreamer_renderer_factory.cpp	\
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


INCLUDES = \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	$(EXPAT_INCLUDES) \
	$(xerces_includes) \
	$(qtincludes) \
	$(gstreamer_includes) \
	$(gtkincludes) \
	$(nokia770_includes) \
	$(artsincludes) \
	$(sdlincludes) \
	$(ffmpegincludes) \
	$(live_includes) \
	$(INCLTDL)

lib_LTLIBRARIES = \
	libambulant.la \
	$(toolkit_cocoa_libraries) \
	$(toolkit_qt_libraries) \
	$(toolkit_gstreamer_libraries) \
	$(toolkit_gtk_libraries) \
	$(toolkit_arts_libraries) \
	$(toolkit_sdl_libraries) \
	$(toolkit_nokia770_libraries) \
	$(toolkit_ffmpeg_libraries) \
	$(toolkit_live_libraries)

# AM_CPPFLAGS=-rdynamic
AM_CPPFLAGS = \
		-DAMBULANT_DATADIR=\"$(pkgdatadir)\" \
		-DAMBULANT_PLUGINDIR=\"$(pkglibdir)\"

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
	lib/parselets.cpp \
	lib/string_util.cpp \
	lib/timer.cpp \
	lib/transition_info.cpp \
	lib/tree_builder.cpp \
	\
	lib/parser_factory.cpp \
	lib/unix/unix_event_processor.cpp \
	lib/unix/unix_mtsync.cpp \
	lib/unix/unix_thread.cpp \
	lib/unix/unix_timer.cpp \
	\
	net/posix_datasource.cpp \
	net/stdio_datasource.cpp \
	net/raw_video_datasource.cpp \
	net/datasource.cpp \
	net/databuffer.cpp \
	net/url.cpp \
	\
	common/factory.cpp \
	common/gui_player.cpp \
	common/preferences.cpp \
	common/region.cpp \
	common/renderer_impl.cpp \
	common/schema.cpp \
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
	mms/mms_player.cpp \
	mms/timeline_builder.cpp \
	mms/timelines.cpp \
	\
	gui/none/none_area.cpp \
	gui/none/none_gui.cpp \
	gui/none/none_video_renderer.cpp \
	gui/none/none_factory.cpp

## EXTRA_DIST= \
## 	gui/cocoa/cocoa_fill.m gui/cocoa/cocoa_fill.mm \
## 	gui/cocoa/cocoa_gui.m gui/cocoa/cocoa_gui.mm \
## 	gui/cocoa/cocoa_image.m gui/cocoa/cocoa_image.mm \
## 	gui/cocoa/cocoa_mouse.m gui/cocoa/cocoa_mouse.mm \
## 	gui/cocoa/cocoa_text.m gui/cocoa/cocoa_text.mm \
## 	gui/cocoa/cocoa_audio.m gui/cocoa/cocoa_audio.mm \
## 	gui/cocoa/cocoa_audio.m gui/cocoa/cocoa_video.mm
EXTRA_DIST= \
    lib/expat_parser.cpp \
    lib/xerces_parser.cpp \
	libambulant_win32.dsp \
	libambulant_win32.vcproj \
	libambulant_wince.vcp \
	gui/SDL \
	gui/arts \
	gui/cocoa \
	gui/dg \
	gui/dx \
	gui/qt \
	gui/gstreamer \
	gui/gtk \
	net/win32_datasource.cpp \
	lib/memfile.cpp \
	lib/win32

CLEANFILES=demux_datasource.loT ffmpeg_common.loT sdl.loT

	lib/win32

CLEANFILES=demux_datasource.loT ffmpeg_common.loT sdl.loT
