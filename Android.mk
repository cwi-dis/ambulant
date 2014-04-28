# This file is for temporary/experimental android builds.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SDL_PATH := ../SDL

SDL_IMAGE_PATH := ../SDL_image

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../expat/lib \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/$(SDL_PATH)/include \
                    $(LOCAL_PATH)/$(SDL_IMAGE_PATH)


LOCAL_CPP_FEATURES := rtti exceptions

LOCAL_CFLAGS := -DWITH_EXPAT -DWITH_SDL2 -DANDROID -DWITH_SDL_IMAGE -DWITH_SDL_TTF  -fno-strict-aliasing 

LOCAL_SHARED_LIBRARIES := ambulant SDL2 SDL2_image SDL2_ttf expat gnust_shared

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -lc

LOCAL_MODULE    := libambulant

LOCAL_SRC_FILES := src/libambulant/lib/gpaths.cpp \
	src/libambulant/lib/colors.cpp \
	src/libambulant/lib/logger.cpp \
	src/libambulant/lib/parselets.cpp \
	src/libambulant/lib/event_processor.cpp \
	src/libambulant/lib/nfa.cpp \
	src/libambulant/lib/string_util.cpp \
	src/libambulant/lib/tree_builder.cpp \
	src/libambulant/lib/memfile.cpp \
	src/libambulant/lib/transition_info.cpp \
	src/libambulant/lib/parser_factory.cpp \
	src/libambulant/lib/document.cpp \
	src/libambulant/lib/timer.cpp \
	src/libambulant/lib/delta_timer.cpp \
	src/libambulant/lib/unix/unix_thread.cpp \
	src/libambulant/lib/unix/unix_timer.cpp \
	src/libambulant/lib/unix/unix_mtsync.cpp \
	src/libambulant/lib/node_impl.cpp \
	src/libambulant/lib/nscontext.cpp \
	src/libambulant/lib/expat_parser.cpp \
	src/libambulant/net/posix_datasource.cpp \
	src/libambulant/net/databuffer.cpp \
	src/libambulant/net/datasource.cpp \
	src/libambulant/net/url.cpp \
	src/libambulant/get_version.cpp \
	src/libambulant/gui/none/none_gui.cpp \
	src/libambulant/gui/none/none_factory.cpp \
	src/libambulant/gui/none/none_video_renderer.cpp \
	src/libambulant/gui/none/none_area.cpp \
	src/libambulant/gui/SDL/sdl_audio.cpp \
	src/libambulant/gui/SDL/sdl_ttf_smiltext.cpp \
    src/libambulant/gui/SDL/sdl_video.cpp \
	src/libambulant/gui/SDL/sdl_text_renderer.cpp \
	src/libambulant/gui/SDL/sdl_fill.cpp \
	src/libambulant/gui/SDL/sdl_renderer.cpp \
	src/libambulant/gui/SDL/sdl_factory.cpp \
	src/libambulant/gui/SDL/sdl_transition.cpp \
	src/libambulant/gui/SDL/sdl.cpp \
	src/libambulant/gui/SDL/sdl_window.cpp \
	src/libambulant/gui/SDL/sdl_image_renderer.cpp \
	src/libambulant/smil2/trace_player.cpp \
	src/libambulant/smil2/animate_n.cpp \
	src/libambulant/smil2/time_node.cpp \
	src/libambulant/smil2/transition.cpp \
	src/libambulant/smil2/animate_e.cpp \
	src/libambulant/smil2/region_node.cpp \
	src/libambulant/smil2/animate_a.cpp \
	src/libambulant/smil2/time_state.cpp \
	src/libambulant/smil2/smil_time.cpp \
	src/libambulant/smil2/time_sched.cpp \
	src/libambulant/smil2/timegraph.cpp \
	src/libambulant/smil2/params.cpp \
	src/libambulant/smil2/smil_player.cpp \
	src/libambulant/smil2/test_attrs.cpp \
	src/libambulant/smil2/smil_layout.cpp \
	src/libambulant/smil2/time_calc.cpp \
	src/libambulant/smil2/time_attrs.cpp \
	src/libambulant/smil2/smiltext.cpp \
	src/libambulant/smil2/sync_rule.cpp \
	src/libambulant/common/renderer_select.cpp \
	src/libambulant/common/gui_player.cpp \
	src/libambulant/common/region.cpp \
	src/libambulant/common/state.cpp \
	src/libambulant/common/smil_alignment.cpp \
	src/libambulant/common/renderer_impl.cpp \
	src/libambulant/common/plugin_engine.cpp \
	src/libambulant/common/preferences.cpp \
	src/libambulant/common/schema.cpp \
	src/libambulant/common/factory.cpp \
	src/libambulant/common/video_renderer.cpp \
	src/libambulant/common/smil_handler.cpp \
	src/player_sdl/sdl_logger.cpp \
	src/player_sdl/sdl_gui.cpp \
	src/player_sdl/sdl_gui_player.cpp \
	src/player_sdl/unix_preferences.cpp

include $(BUILD_SHARED_LIBRARY)
