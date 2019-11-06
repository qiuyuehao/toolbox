/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 2012-2016 Synaptics Incorporated. All rights reserved.
*
* The information in this file is confidential under the terms
* of a non-disclosure agreement with Synaptics and is provided
* AS IS without warranties or guarantees of any kind.
*
* The information in this file shall remain the exclusive property
* of Synaptics and may be the subject of Synaptics patents, in
* whole or part. Synaptics intellectual property rights in the
* information in this file are not expressly or implicitly licensed
* or otherwise transferred to you as a result of such information
* being made available to you.
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <android/log.h>

#ifndef _NATIVE_LIB_H__
#define _NATIVE_LIB_H__

#define printf_i(...) __android_log_print(ANDROID_LOG_INFO, "syna-apk-native", __VA_ARGS__);
#define printf_e(...) __android_log_print(ANDROID_LOG_ERROR, "syna-apk-native", __VA_ARGS__);

#define MAX_STRING_LEN (512)

/* utility */
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

/* configuration from UI */
int g_report_type;
int g_num_frames;
int g_threshold;
char g_log_file[MAX_STRING_LEN];
char g_fail_log_file[MAX_STRING_LEN];

/* the path of syna character device */
char g_dev_node[MAX_STRING_LEN/8];

/* keep the error message during the rmi process */
char err_msg_out[MAX_STRING_LEN*40];


bool g_is_rmi_dev;
bool g_is_rmi_dev_initialized;

bool g_is_tcm_dev;
bool g_is_tcm_dev_initialized;

struct timeval t_val_1, t_val_2, t_diff;

extern FILE *pfile_fail_log;

#endif // _NATIVE_LIB_H__

