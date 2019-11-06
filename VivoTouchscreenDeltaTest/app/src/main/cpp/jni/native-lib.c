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

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "native-lib.h"

JavaVM * g_jni_vm = NULL;
JNIEnv * g_jni_env = NULL;
jobject g_jni_obj = NULL;

/* help functions */
extern bool is_rmi_dev_existed(void);
extern bool is_tcm_dev_existed(void);
extern int open_dev(const char* dev_path);
extern void close_dev(const char* dev_path);
extern int get_asic_id(void);
extern int get_build_id(void);
extern int get_gear_info(char *info);
extern int get_finger_cap(void);
extern int enable_one_specified_gear(int gear);
extern int write_log_header(FILE *pfile, int total_frames, int num_gears);
extern bool do_noise_test(FILE *pfile, char *timestamp, int frame_id, int gear_idx);
extern int do_test_preparation();
extern int do_test_completion();
extern int get_rmi_tx_info();
extern int get_rmi_rx_info();
extern int get_tcm_row_info();
extern int get_tcm_col_info();
extern bool start_report(bool is_delta, bool is_raw);
extern bool stop_report(bool is_delta, bool is_raw);
extern int read_report(bool is_delta, bool is_raw, short *p_image);

FILE *pfile;
FILE *pfile_fail_log;

/*
 * Function:  JNI_OnLoad
 * --------------------
 * The VM calls JNI_OnLoad when the native library is loaded
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    // if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
    //    return -1;

    printf("%s: +\n", __FUNCTION__);
    g_jni_vm = vm;

    return JNI_VERSION_1_6;
}

/*
 * Function:  createVIVOFolderJNI
 * --------------------
 * The VM calls JNI_OnLoad when the native library is loaded
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_createVIVOFolderJNI(
        JNIEnv *env, jobject obj)
{
    bool ret;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* create the requested folder */
    if (system("mkdir /sdcard/Android") == -1) {
        printf_e("%s: can not create /data/Android/\n", __FUNCTION__);
        ret = false;
        goto exit;
    }
    else {
        if (system("mkdir /sdcard/Android/data/") == -1) {
            printf_e("%s: can not create /data/Android/data\n", __FUNCTION__);
            ret = false;
            goto exit;
        }
        else {
            if (system("mkdir /sdcard/Android/data/com.touchscreen.tptest/") == -1) {
                printf_e("%s: can not create /sdcard/Android/data/com.touchscreen.tptest/\n", __FUNCTION__);
                ret = false;
                goto exit;
            }
            else {
                printf_i("%s: path is created /sdcard/Android/data/com.touchscreen.tptest/\n", __FUNCTION__);
                ret = true;
            }
        }
    }
exit:
    return (jboolean)ret;
}
/*
 * Function:  checkDevJNI
 * --------------------
 * to check whether the /dev/rmi0 or /dev/tcm0
 * is registered or not
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_checkDevJNI(
        JNIEnv *env, jobject obj)
{
    bool is_found;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    is_found = is_rmi_dev_existed();
    if (is_found) {
        printf_i("%s: find rmi device %s\n", __FUNCTION__, g_dev_node);
        return (jboolean)true;
    }
    else {
        is_found = is_tcm_dev_existed();
        if (is_found) {
            printf_i("%s: find tcm device %s\n", __FUNCTION__, g_dev_node);
            return (jboolean)true;
        }
        else {
            return (jboolean)false;
        }
    }
}
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_isRMIDevJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return (jboolean) g_is_rmi_dev;
}
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_isTCMDevJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return (jboolean) g_is_tcm_dev;
}
/*
 * Function:  getDevIDJNI
 * --------------------
 * to return the device id
 */
JNIEXPORT jstring JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_getDevIDJNI(
        JNIEnv *env, jobject obj)
{
    char dev_id[128] = {0};
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    printf_i("%s: getDevIDJNI\n", __FUNCTION__);

    sprintf(dev_id, "%d", get_asic_id());

    return (*env)->NewStringUTF(env, dev_id);
}
/*
 * Function:  getBuildIDJNI
 * --------------------
 * to return the firmware id
 */
JNIEXPORT jstring JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_getBuildIDJNI(
        JNIEnv *env, jobject obj)
{
    char fw_id[32] = {0};
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    sprintf(fw_id, "%d", get_build_id());

    return (*env)->NewStringUTF(env, fw_id);
}
/*
 * Function:  setReportTypeJNI
 * --------------------
 * to save the selection of report type configured by UI settings
 */
JNIEXPORT void JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_setReportTypeJNI(
        JNIEnv *env, jobject obj, jint jvalue)
{
    g_report_type = (int)jvalue;
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    printf_i("%s: Report Type = %d\n", __FUNCTION__, g_report_type);
}
/*
 * Function:  setThresholdJNI
 * --------------------
 * to save the delta threshold configured by UI settings
 * the threshold is used to determine the failure case
 */
JNIEXPORT void JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_setThresholdJNI(
        JNIEnv *env, jobject obj, jint jvalue)
{
    g_threshold = (int)jvalue;
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    printf_i("%s: Delta Threshold = %d\n", __FUNCTION__, g_threshold);
}
/*
 * Function:  setOutputFileJNI
 * --------------------
 * to save the output file name
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_setOutputFileJNI(
        JNIEnv *env, jobject obj, jstring jfile)
{
    const char *str_temp_path;
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    if (!jfile) {
        printf_e("%s: invalid parameter.\n", __FUNCTION__);
        return (jboolean)false;
    }

    str_temp_path = (*env)->GetStringUTFChars(env, jfile, NULL);
    sprintf(g_log_file, "%s_all.csv", str_temp_path);
    sprintf(g_fail_log_file, "%s_fail.csv", str_temp_path);

    (*env)->ReleaseStringUTFChars(env, jfile, str_temp_path);
    printf_i("%s: Path of Log File: %s\n", __FUNCTION__, g_log_file);
    printf_i("%s: Path of Failure Log File: %s\n", __FUNCTION__, g_fail_log_file);

    return (jboolean)true;
}
/*
 * Function:  openTest
 * --------------------
 * to open the RMI device, and write header to the file pointer
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_openTestJNI(
        JNIEnv *env, jobject obj, jint total_frames, jint num_gears)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    if (open_dev(g_dev_node) < 0) {
        return (jboolean)false;
    }

    pfile = fopen(g_log_file, "w");
    if (!pfile){
        printf_e("%s: unable to open file (name: %s)\n", __FUNCTION__, g_log_file);
        return (jboolean)false;
    }

    if (write_log_header(pfile, total_frames, num_gears) != 0) {
        printf_e("%s: fail to write header to the log file (name: %s)\n", __FUNCTION__, g_log_file);
        return (jboolean)false;
    }

    pfile_fail_log = fopen(g_fail_log_file, "w");
    if (!pfile){
        printf_e("%s: unable to open file (name: %s)\n", __FUNCTION__, g_fail_log_file);
        return (jboolean)false;
    }

    if (write_log_header(pfile_fail_log, total_frames, num_gears) != 0) {
        printf_e("%s: fail to write header to the log file (name: %s)\n", __FUNCTION__, g_fail_log_file);
        return (jboolean)false;
    }

    if (do_test_preparation() < 0) {
        printf_e("%s: fail to start the noise test\n", __FUNCTION__);
        return (jboolean)false;
    }

    return (jboolean)true;
}
/*
 * Function:  closeTestJNI
 * --------------------
 * to close the RMI device, and create the log file
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_closeTestJNI(
        JNIEnv *env, jobject obj, jint jerr_cnt)
{
    int ret;
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    if (do_test_completion() < 0) {
        printf_e("%s: fail to stop the noise test\n", __FUNCTION__);
    }

    if (pfile) {
        fclose(pfile_fail_log);

        if (jerr_cnt == 0) {
            fprintf(pfile, "\nSyna Noise Test     \t: Pass\n");

            ret = remove(g_fail_log_file);  // remove file log
            if (ret == 0)
                printf_i("%s: %s is removed\n", __FUNCTION__, g_fail_log_file);
        }
        else {
            fprintf(pfile, "\nSyna Noise Test     \t: Fail (Error Count = %d)\n", jerr_cnt);

            printf_i("%s: failure log is created ! %s\n", __FUNCTION__, g_fail_log_file);
        }
        fclose(pfile);
        printf_i("%s: log file is created ! %s\n", __FUNCTION__, g_log_file);
    }

    close_dev(g_dev_node);

    return (jboolean) true;
}
/*
 * Function:  doTestJNI
 * --------------------
 * to close the RMI device
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_doTestJNI(
        JNIEnv *env, jobject obj, jint jframe_id, jint jgear_idx)
{
    bool result;
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    char timestamp[MAX_STRING_LEN/2];
    /* get the timestamp */
    gettimeofday(&t_val_2, NULL);
    timersub(&t_val_2, &t_val_1, &t_diff);
    sprintf(timestamp, "< %f >", (t_diff.tv_sec+(1.0*t_diff.tv_usec)/1000000));

    result = do_noise_test(pfile, timestamp, jframe_id, jgear_idx);

    return (jboolean)result;
}
/*
 * Function:  getGearInfoJNI
 * --------------------
 * to return the string of actual enabled gears
 */
JNIEXPORT jstring JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_getGearInfoJNI(
        JNIEnv *env, jobject obj)
{
    char out[MAX_STRING_LEN] = {0};
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    if (get_gear_info(out) == 0) {
        return (*env)->NewStringUTF(env, out);
    }
    else {
        sprintf(out, "%sGear 1\n", out);
        return (*env)->NewStringUTF(env, out);
    }
}
/*
 * Function:  setGearJNI
 * --------------------
 * to enable the specified frequency gear
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_setGearJNI(
        JNIEnv *env, jobject obj, jint jgear)
{
    int retval = enable_one_specified_gear(jgear);
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    if (retval < 0) {
        if (pfile) {
            fprintf(pfile, "%s",  err_msg_out);
            fprintf(pfile, "error: stop the testing in frequency gear-%d\n\n", jgear);
        }
    }

    return (jboolean)(retval > 0);
}
/*
 * Function:  getFingerThresholdJNI
 * --------------------
 * to get the default finger delta defined in RMI
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_getFingerThresholdJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return get_finger_cap();
}
/*
 * Function:  getRMITxInfoJNI
 * --------------------
 * to get the number of tx channels on rmi dev
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_getRMITxInfoJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return get_rmi_tx_info();
}
/*
 * Function:  getRMIRxInfoJNI
 * --------------------
 * to get the number of rx channels on rmi dev
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_getRMIRxInfoJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return get_rmi_rx_info();
}
/*
 * Function:  getTCMRowInfoJNI
 * --------------------
 * to get the number of row channels on tcm dev
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_getTCMRowInfoJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return get_tcm_row_info();
}
/*
 * Function:  getTCMColInfoJNI
 * --------------------
 * to get the number of col channels on tcm dev
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_getTCMColInfoJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return get_tcm_col_info();
}
/*
 * Function:  startImgReportJNI
 * --------------------
 * to prepare the image report streaming
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_startImgReportJNI(
        JNIEnv *env, jobject obj, jboolean is_delta, jboolean is_raw)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    printf_i("%s info: start report type: delta (%s) , raw (%s)\n", __FUNCTION__,
             (is_delta)?"true" : "false",
             (is_raw)?"true" : "false");

    return (jboolean) start_report(is_delta, is_raw);
}
/*
 * Function:  stopImgReportJNI
 * --------------------
 * to complete the image report streaming
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_stopImgReportJNI(
        JNIEnv *env, jobject obj, jboolean is_delta, jboolean is_raw)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    printf_i("%s info: start report type: delta (%s) , raw (%s)\n", __FUNCTION__,
             (is_delta)?"true" : "false",
             (is_raw)?"true" : "false");

    return (jboolean) stop_report(is_delta, is_raw);
}
/*
 * Function:  requestImgReportJNI
 * --------------------
 * to request a report frame
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_synadeltadiff_NativeWrapper_requestImgReportJNI(
        JNIEnv *env, jobject obj, jboolean is_delta, jboolean is_raw, jshortArray jarray)
{
    int retval;
    short *data_array;
    jsize len_data_array;
    jboolean isCopy;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* get a short array from java layer */
    data_array = (*env)->GetShortArrayElements(env, jarray, &isCopy);
    len_data_array = (*env)->GetArrayLength(env, jarray);
    if (len_data_array <= 0) {
        printf_e("%s: invalid parameter. (len_data_array = %d)\n", __FUNCTION__, len_data_array);
        return (jboolean)false;
    }
    /* request a delta report */
    retval = read_report(is_delta, is_raw, data_array);
    if (retval < 0) {
        printf_e("%s: fail to read delta report report \n", __FUNCTION__);
        /* release the java array */
        (*env)->ReleaseShortArrayElements(env, jarray, data_array, 0);
        return (jboolean)false;
    }

    /* release the java array */
    (*env)->ReleaseShortArrayElements(env, jarray, data_array, 0);
    return (jboolean)true;
}