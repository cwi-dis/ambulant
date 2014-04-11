
JNI_DIR := $(call my-dir)

LOCAL_PATH:= $(JNI_DIR)
include $(CLEAR_VARS)

include expat/Android.mk
include SDL/Android.mk
include SDL_ttf/Android.mk
include SDL_image/Android.mk
include ambulant/Android.mk


#$(call all-subdir-makefiles)
