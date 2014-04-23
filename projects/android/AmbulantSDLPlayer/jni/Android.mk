
JNI_DIR := $(call my-dir)

LOCAL_PATH:= $(JNI_DIR)
include $(CLEAR_VARS)

include $(JNI_DIR)/expat/Android.mk
include $(JNI_DIR)/SDL/Android.mk
include $(JNI_DIR)/SDL_ttf/Android.mk
include $(JNI_DIR)/SDL_image/Android.mk
include $(JNI_DIR)/ambulant/Android.mk


#$(call all-subdir-makefiles)
