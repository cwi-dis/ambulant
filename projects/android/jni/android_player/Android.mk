LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libAmbulantSDLPlayer

SDL_PATH := ../SDL

SDL_IMAGE_PATH := ../SDL_image

AMBULANT_PATH := ../ambulant

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
                    $(LOCAL_PATH)/$(AMBULANT_PATH)/include

LOCAL_CFLAGS := -DWITH_SDL2 -DANDROID

# Add your application source files here...
LOCAL_SRC_FILES := $(AMBULANT_PATH)/src/player_sdl/sdl_gui_player.cpp \
                   $(AMBULANT_PATH)/src/player_sdl/unix_preferences.cpp \
                   $(AMBULANT_PATH)/src/player_sdl/sdl_gui.cpp


LOCAL_SHARED_LIBRARIES := SDL2 libambulant libexpat SDL2_image

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
