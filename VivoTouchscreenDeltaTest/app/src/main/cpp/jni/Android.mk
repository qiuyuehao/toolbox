LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# give module name
LOCAL_MODULE    := native_syna

# list your C files to compile
LOCAL_SRC_FILES := native-lib.c \
                   native-datalog.c \
                   rmi_control.c \
                   syna_control.c \
                   tcm_control.c \
                   monitor_monitor.c \

LOCAL_LDLIBS    := -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)
