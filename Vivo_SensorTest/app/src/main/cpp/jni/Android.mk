LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# give module name
LOCAL_MODULE    := native_syna

# list your C files to compile
LOCAL_SRC_FILES := native_syna_lib.c \
                   err_msg_ctrl.c \
                   syna_dev_manager.c \
                   rmi_control.c \
                   rmi_identify.c \
                   rmi_report_access.c \
                   rmi_production_test.c \
                   rmi_touch_data.c \
                   tcm_control.c \
                   tcm_identify.c \
                   tcm_report_access.c \
                   tcm_production_test.c \
                   tcm_touch_data.c \
                   extended_high_resistance.c

LOCAL_LDLIBS    := -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)
