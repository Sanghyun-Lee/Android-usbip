#
#usbipd
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := usbipd
LOCAL_SRC_FILES := stub_server.c \
	usbip_network.c \
	usbip_network.h
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../glib \
	$(LOCAL_PATH)/../sysfslib \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib \
	$(call include-path-for, sysfslib) \
	$(call include-path-for, sysfslib)/sysfslib
LOCAL_CFLAGS := -Wall -W -Wstrict-prototypes -std=gnu99 -g
LOCAL_MODULE_TAGS := eng
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := libc
LOCAL_STATIC_LIBRARIES := libusbipd \
			libglib_static \
			libsysfsd
include $(BUILD_EXECUTABLE)
$(warning ######   cmd end   ######)

#
#usbip_bind_driver
#
$(warning ######  cmd Start  ######)
include $(CLEAR_VARS)
LOCAL_MODULE := usbip_bind_driver
LOCAL_SRC_FILES := bind-driver.c \
	utils.c \
	utils.h \
	usbip_network.h \
	usbip_network.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../glib \
	$(LOCAL_PATH)/../sysfslib \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib \
	$(call include-path-for, sysfslib) \
	$(call include-path-for, sysfslib)/sysfslib
LOCAL_CFLAGS := -Wall -W -Wstrict-prototypes -std=gnu99
LOCAL_MODULE_TAGS := eng
LOCAL_PRELINK_MODULE:=false
LOCAL_SHARED_LIBRARIES := libc
LOCAL_STATIC_LIBRARIES := libusbipd \
		libglib_static \
		libsysfsd
include $(BUILD_EXECUTABLE)
$(warning ######   cmd end   ######)
