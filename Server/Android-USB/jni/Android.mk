LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := ndk-usbip
LOCAL_SRC_FILES := USBIPConnect.c
include $(BUILD_SHARED_LIBRARY)
