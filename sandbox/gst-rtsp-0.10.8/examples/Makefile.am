noinst_PROGRAMS = test-screen test-webcam test-video test-ogg test-mp4 test-readme test-launch test-sdp test-uri test-auth

INCLUDES = -I$(top_srcdir) -I$(srcdir)

AM_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_CFLAGS)
AM_LDFLAGS = \
	$(GST_LIBS) \
	$(top_builddir)/gst/rtsp-server/libgstrtspserver-@GST_MAJORMINOR@.la

