$(warning ######  lib Start  ######)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libusbipd
LOCAL_SRC_FILES := names.c \
	names.h \
	stub_driver.c \
	stub_driver.h \
	usbip.h \x
	usbip_common.c \
	usbip_common.h \
	vhci_driver.c \
	vhci_driver.h
LOCAL_CFLAGS := -Wall -W -Wstrict-prototypes -std=gnu99
LOCAL_C_INCLUDES := external/usbipd/lib/
LOCAL_MODULE_TAGS := eng
LOCAL_STATIC_LIBRARIES := libc
include $(BUILD_STATIC_LIBRARY)
$(warning ######  lib end  ######)


