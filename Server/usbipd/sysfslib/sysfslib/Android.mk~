$(warning ######  sysfs Start  ######)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libsysfsd
LOCAL_SRC_FILES := sysfs_utils.c \
	sysfs_attr.c \
	sysfs_class.c \
	dlist.c \
	sysfs_device.c \
	sysfs_driver.c \
	sysfs_bus.c \
	sysfs_module.c \
	sysfs.h \
	libsysfs.h \
	dlist.h
#LOCAL_STATIC_LIBRARIES := libc libsysfs
LOCAL_SHARED_LIBRARIES := libc
LOCAL_C_INCLUDES := external/usbipd/sysfslib/include/
LOCAL_CFLAGS := -Wall -W -Wstrict-prototypes
LOCAL_MODULE_TAGS := eng
include $(BUILD_STATIC_LIBRARY)

#LOCAL_CFLAGS := -Wall -W -Wstrict-prototypes -std=gnu99
#LOCAL_SHARED_LIBRARIES := libc
#LOCAL_STATIC_LIBRARIES := libusbip \
#			libglib_static \
#			libsysfsd
$(warning ######   sysfs end   ######)


