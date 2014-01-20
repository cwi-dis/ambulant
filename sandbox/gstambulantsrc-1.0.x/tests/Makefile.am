# noinst_PROGRAMS = test2t
# est2_LDADD = $(GST_OBJ_LIBS) $(GST_LIBS)
# test2_CFLAGS = $(GST_OBJ_CFLAGS) $(GST_CFLAGS)
# TESTS=test2

export GST_PLUGIN_PATH=$(top_builddir)/src/.libs
TESTS_ENVIRONMENT = CFLAGS='@CFLAGS@'
TESTS=./test1.sh 
EXTRA_DIST=test1.sh Welcome-audio-raw Welcome-video-raw
