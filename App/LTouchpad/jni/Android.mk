LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := usb-mouse
LOCAL_SRC_FILES := UsbipMouse.c
include $(BUILD_SHARED_LIBRARY)