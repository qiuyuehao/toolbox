/*
 * Copyright (c)  2012-2018 Synaptics Incorporated. All rights reserved.
 * This file contains information that is proprietary to Synaptics
 * Incorporated ("Synaptics"). The holder of this file shall treat all
 * information contained herein as confidential, shall use the
 * information only for its intended purpose, and shall not duplicate,
 * disclose, or disseminate any of this information in any manner unless
 * Synaptics has otherwise provided express, written permission.
 * Use of the materials may require a license of intellectual property
 * from a third party or from Synaptics. Receipt or possession of this
 * file conveys no express or implied licenses to any intellectual
 * property rights belonging to Synaptics.
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND
 * SYNAPTICS EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE, AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY
 * INTELLECTUAL PROPERTY RIGHTS. IN NO EVENT SHALL SYNAPTICS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION WITH THE USE OF
 * THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED AND BASED
 * ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT
 * JURISDICTION DOES NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY
 * OTHER DAMAGES, SYNAPTICS' TOTAL CUMULATIVE LIABILITY TO ANY PARTY
 * SHALL NOT EXCEED ONE HUNDRED U.S. DOLLARS.
 */
#include <jni.h>
#include <stdio.h>
#include <string.h>

#include "native_syna_lib.h"
#include "syna_dev_manager.h"

#ifdef SAVE_ERR_MSG
#include "err_msg_ctrl.h"
#endif

JavaVM * g_jni_vm = NULL;
JNIEnv * g_jni_env = NULL;
jobject g_jni_obj = NULL;

/*
 * Function:  JNI_OnLoad
 * --------------------
 * The VM calls JNI_OnLoad when the native library is loaded
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{

    printf_i("%s: +\n", __FUNCTION__);
    g_jni_vm = vm;

    return JNI_VERSION_1_6;
}
/*
 * Function:  getNumErrMsgJNI
 * --------------------
 * retrieve the number of error messages being stored
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getNumErrMsgJNI(
        JNIEnv *env, jobject obj)
{
    int retval = 0;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

#ifdef SAVE_ERR_MSG
    retval = get_num_err_msg();
#endif
    return retval;
}
/*
 * Function:  getErrMsgJNI
 * --------------------
 * retrieve the appointed error messages
 */
JNIEXPORT jstring JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getErrMsgJNI(
        JNIEnv *env, jobject obj, jint idx)
{
    char *err;

#ifdef SAVE_ERR_MSG
    err = get_err_msg(idx);
#endif

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    if (err == NULL) {
        printf_e("%s error: unable to get the appointed error message\n", __FUNCTION__);
        return NULL;
    }
    return (*env)->NewStringUTF(env, err);
}
/*
 * Function:  clearErrMsgJNI
 * --------------------
 * clear all stored error messages
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_clearErrMsgJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

#ifdef SAVE_ERR_MSG
    clear_all_error_msg();
#endif

    return (jboolean) true;
}
/*
 * Function:  findSynaDevJNI
 * --------------------
 * check whether the /dev/tcm or /dev/rmi is installed
 */
JNIEXPORT jstring JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_findSynaDevJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    if (syna_find_dev(g_dev_node)) {
        return (*env)->NewStringUTF(env, g_dev_node);
    }
    else {
        printf_e("%s error: can't find proper synaptics device\n", __FUNCTION__);
        return NULL;
    }
}
/*
 * Function:  setupSynaDevJNI
 * --------------------
 * setup the specific device node
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_setupSynaDevJNI(
        JNIEnv *env, jobject obj, jstring node, jboolean is_rmi, jboolean is_tcm)
{
    const char* str_node;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    if (!node) {
        printf_e("%s error: invalid parameter.\n", __FUNCTION__);
        return (jboolean)false;
    }

    str_node = (*env)->GetStringUTFChars(env, node, NULL);

    if (syna_set_dev(str_node, is_rmi, is_tcm)) {
        (*env)->ReleaseStringUTFChars(env, node, str_node);
        return (jboolean)true;
    }
    else {
        printf_e("%s error: can't find proper synaptics device\n", __FUNCTION__);
        (*env)->ReleaseStringUTFChars(env, node, str_node);
        return (jboolean)false;
    }
}
/*
 * Function:  openSynaDevJNI
 * --------------------
 * open a Synaptics device node
 */
JNIEXPORT jboolean JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_openSynaDevJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* open synaptics device interface */
    return (jboolean) (syna_open_dev(g_dev_node) >= 0);
}

/*
 * Function:  closeSynaDevJNI
 * --------------------
 * close the Synaptics device node
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_closeSynaDevJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* close synaptics device interface */
    syna_close_dev(g_dev_node);

    return (jboolean) true;
}
/*
 * Function:  doDevPreparationJNI
 * --------------------
 * do preparation before testing
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_doDevPreparationJNI(
        JNIEnv *env, jobject obj, jboolean do_nosleep, jboolean do_rezero)
{
    int retval;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    retval = syna_do_preparation(do_nosleep, do_rezero);
    if (retval < 0) {
        printf_e("%s error: fail to do preparation", __FUNCTION__);
        return (jboolean)false;
    }

    return (jboolean)true;
}
/*
 * Function:  getIdentifyDataJNI
 * --------------------
 * retrieve the device identification information
 */
JNIEXPORT jstring JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getIdentifyDataJNI(
        JNIEnv *env, jobject obj)
{
    char identify_info[MAX_STRING_LEN * 4] = {0};
    int retval;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    retval = syna_do_identify(identify_info);
    if (retval < 0) {
        printf_e("%s error: fail to do identify\n", __FUNCTION__);
        return NULL;
    }

    return (*env)->NewStringUTF(env, identify_info);
}
/*
 * Function:  getFirmwareIdJNI
 * --------------------
 * retrieve the firmware id
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getFirmwareIdJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return syna_get_fw_id();
}
/*
 * Function:  getImageRowJNI
 * --------------------
 * retrieve the number of row in the report image
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getImageRowJNI(
        JNIEnv *env, jobject obj, jboolean is_landscape)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return syna_get_image_rows(is_landscape);
}
/*
 * Function:  getImageColJNI
 * --------------------
 * retrieve the number of column in the report image
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getImageColJNI(
        JNIEnv *env, jobject obj, jboolean is_landscape)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return syna_get_image_cols(is_landscape);
}
/*
 * Function:  getButtonCntJNI
 * --------------------
 * retrieve the number of 0D buttons
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getButtonCntJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return syna_get_num_btns();
}
/*
 * Function:  getFeatureHasHybridJNI
 * --------------------
 * retrieve the number of 0D buttons
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getFeatureHasHybridJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return syna_get_image_has_hybrid();
}
/*
 * Function:  getForceElecsJNI
 * --------------------
 * retrieve the number of force electrodes assigned
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getForceElecsJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return syna_get_num_force_elecs();
}
/*
 * Function:  getProductIDJNI
 * --------------------
 * retrieve the device id
 */
JNIEXPORT jstring JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getProductIDJNI(
        JNIEnv *env, jobject obj)
{
    char device_id[MAX_STRING_LEN * 4] = {0};

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    sprintf(device_id, "%s", syna_get_device_id());

    return (*env)->NewStringUTF(env, device_id);
}
/*
 * Function:  getConfigIDJNI
 * --------------------
 * retrieve the device id
 */
JNIEXPORT jstring JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getConfigIDJNI(
        JNIEnv *env, jobject obj)
{
    char config_id[MAX_STRING_LEN * 4] = {0};

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    sprintf(config_id, "%s", syna_get_config_id());

    return (*env)->NewStringUTF(env, config_id);
}
/*
 * Function:  startReportJNI
 * --------------------
 * do preparation before requesting the report image
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_startReportJNI(
        JNIEnv *env, jobject obj, jbyte type, jboolean en_touch,
        jboolean do_no_sleep, jboolean do_rezero)
{
    int retval;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    printf_i("%s info: enable the report\n", __FUNCTION__);

    /* enable the requested report */
    retval = syna_start_image_stream((unsigned char)type, en_touch, do_no_sleep, do_rezero);

    if (retval < 0) {
        printf_e("%s error: fail to enable the report\n", __FUNCTION__);
        return (jboolean)false;
    }

    return (jboolean)true;
}
/*
 * Function:  stopReportJNI
 * --------------------
 * terminate the report image stream
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_stopReportJNI(
        JNIEnv *env, jobject obj, jbyte type)
{
    int retval;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    printf_i("%s info: disable the report\n", __FUNCTION__);

    /* disable the requested report */
    retval = syna_stop_image_stream((unsigned char)type);

    if (retval < 0) {
        printf_e("%s error: fail to enable the report\n", __FUNCTION__);
        return (jboolean)false;
    }

    return (jboolean)true;
}
/*
 * Function:  requestReportImageJNI
 * --------------------
 * request a report image
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_requestReportImageJNI(
        JNIEnv *env, jobject obj, jbyte type, jint row, jint col,
        jintArray array, jint size_of_array)
{
    int retval;
    int *native_array;
    jsize len_data_array;
    jboolean isCopy;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* retrieve a short array from java layer */
    native_array = (*env)->GetIntArrayElements(env, array, &isCopy);
    len_data_array = (*env)->GetArrayLength(env, array);
    if (len_data_array <= 0) {
        printf_e("%s error: invalid parameter. (len_data_array = %d)\n",
                 __FUNCTION__, len_data_array);
        return (jboolean)false;
    }
    /* based on type, request the report image */
    retval = syna_read_report_image_entry((unsigned char)type, native_array,
                                     (int)size_of_array, (int)col, (int)row, true);

    if (retval < 0) {
        printf_e("%s error: fail to read delta report report \n", __FUNCTION__);
        /* release the java array */
        (*env)->ReleaseIntArrayElements(env, array, native_array, 0);
        return (jboolean)false;
    }

    /* release the java array */
    (*env)->ReleaseIntArrayElements(env, array, native_array, 0);
    return (jboolean)true;
}

/*
 * Function:  runProductionTestJNI
 * --------------------
 * perform the specific production testing
 * based on the variable, item
 *
 * return  0, pass
 *        >0, number of failure
 *        <0, error out
 */
JNIEXPORT jint JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_runProductionTestJNI(
        JNIEnv *env, jobject obj, jint item, jint col, jint row,
        jintArray limit_1, jint limit_1_size, jintArray limit_2, jint limit_2_size,
        jintArray result, jint result_size)
{
    int retval;
    int *native_limit_1 = NULL;
    int *native_limit_2 = NULL;
    int *native_result = NULL;
    jsize len_array;
    jboolean isCopy;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* get a array of test limit parameter 1 from java layer */
    if (limit_1_size > 0) {
        native_limit_1 = (*env)->GetIntArrayElements(env, limit_1, &isCopy);
        len_array = (*env)->GetArrayLength(env, limit_1);
        if ((len_array <= 0) || (len_array !=  limit_1_size)) {
            printf_e("%s error: invalid parameter. (limit_1_size = %d, len_array = %d)\n",
                     __FUNCTION__, limit_1_size, len_array);
            retval = -10;
            goto exit;
        }
    }
    /* get a array of test limit parameter 2 from java layer */
    if (limit_2_size > 0) {
        native_limit_2 = (*env)->GetIntArrayElements(env, limit_2, &isCopy);
        len_array = (*env)->GetArrayLength(env, limit_2);
        if ((len_array <= 0) || (len_array !=  limit_2_size)) {
            printf_e("%s error: invalid parameter. (limit_2_size = %d, len_array = %d)\n",
                     __FUNCTION__, limit_2_size, len_array);
            retval = -10;
            goto exit;
        }
    }

    /* get a int array from java layer */
    if (result_size > 0) {
        native_result = (*env)->GetIntArrayElements(env, result, &isCopy);
        len_array = (*env)->GetArrayLength(env, result);
        if (len_array != result_size) {
            retval = -10;
            goto exit;
        }
    }

    /* call function to perform the requested production test */
    retval = syna_run_test_entry(item, native_result, result_size, col, row,
                                 native_limit_1, limit_1_size, native_limit_2, limit_2_size);
    if (retval < 0) {
        printf_e("%s error: fail to run production test, test item (0x%x) \n", __FUNCTION__, item);
    }

exit:
    if (limit_1_size > 0)
        (*env)->ReleaseIntArrayElements(env, limit_1, native_limit_1, 0);
    if (limit_2_size > 0)
        (*env)->ReleaseIntArrayElements(env, limit_2, native_limit_2, 0);
    if (result_size > 0)
        (*env)->ReleaseIntArrayElements(env, result, native_result, 0);

    return retval;
}

/*
 * Function:  runProductionTestExHighResistanceJNI
 * --------------------
 * perform the extended high resistance test
 *
 * return  0, pass
 *        >0, number of failure
 *        <0, error out
 */
JNIEXPORT jint JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_runProductionTestExHighResistanceJNI(
        JNIEnv *env, jobject obj, jint jcols, jint jrows, jshortArray jref_frame,
        jint jlimit_surface, jint jlimit_txroe, jint jlimit_rxroe,
        jintArray jresult, jint jsize_result, jintArray jresult_txroe, jintArray jsize_result_txroe,
        jintArray jresult_rxroe, jintArray jsize_result_rxroe)
{
    int retval;
    short *ref_frame_array = NULL;
    int *result_array = NULL;
    int *result_txroe_array = NULL;
    int *result_rxroe_array = NULL;
    int *size_txroe_array = NULL;
    int *size_rxroe_array = NULL;
    jsize len_array;
    jboolean isCopy;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* get a short array, reference frame, from java layer */
    ref_frame_array = (*env)->GetShortArrayElements(env, jref_frame, &isCopy);
    len_array = (*env)->GetArrayLength(env, jref_frame);
    if (len_array <= 0) {
        printf_e("%s error: invalid parameter. (len_array = %d)\n", __FUNCTION__, len_array);
        retval = -10;
        goto exit;
    }

    /* get a int array from java layer */
    if (jsize_result > 0) {
        result_array = (*env)->GetIntArrayElements(env, jresult, &isCopy);
        len_array = (*env)->GetArrayLength(env, jresult);
        if (len_array != jsize_result) {
            printf_e("%s error: invalid parameter. (jresult_size = %d, len_array = %d)\n",
                     __FUNCTION__, jsize_result, len_array);
            retval = -10;
            goto exit;
        }
    }
    result_txroe_array = (*env)->GetIntArrayElements(env, jresult_txroe, &isCopy);
    len_array = (*env)->GetArrayLength(env, jresult_txroe);
    if (len_array < 0) {
        printf_e("%s error: invalid parameter. (jresult_txroe = %d)\n", __FUNCTION__, len_array);
        retval = -10;
        goto exit;
    }
    result_rxroe_array = (*env)->GetIntArrayElements(env, jresult_rxroe, &isCopy);
    len_array = (*env)->GetArrayLength(env, jresult_rxroe);
    if (len_array < 0) {
        printf_e("%s error: invalid parameter. (jresult_rxroe = %d)\n", __FUNCTION__, len_array);
        retval = -10;
        goto exit;
    }

    size_txroe_array = (*env)->GetIntArrayElements(env, jsize_result_txroe, &isCopy);
    len_array = (*env)->GetArrayLength(env, jsize_result_txroe);
    if (len_array < 0) {
        printf_e("%s error: invalid parameter. (jsize_result_txroe = %d)\n", __FUNCTION__, len_array);
        retval = -10;
        goto exit;
    }

    size_rxroe_array = (*env)->GetIntArrayElements(env, jsize_result_rxroe, &isCopy);
    len_array = (*env)->GetArrayLength(env, jsize_result_rxroe);
    if (len_array < 0) {
        printf_e("%s error: invalid parameter. (jsize_result_rxroe = %d)\n", __FUNCTION__, len_array);
        return (jboolean)false;
    }

    /* call function to perform the requested production test */
    retval = syna_run_test_ex_high_resistance_entry(result_array, jsize_result,
                                                    result_txroe_array, size_txroe_array,
                                                    result_rxroe_array, size_rxroe_array,
                                                    ref_frame_array, jcols, jrows,
                                                    jlimit_surface, jlimit_txroe, jlimit_rxroe);
    if (retval < 0) {
        printf_e("%s error: fail to run extended high resistance test\n", __FUNCTION__);
    }
exit:
    (*env)->ReleaseShortArrayElements(env, jref_frame, ref_frame_array, 0);
    (*env)->ReleaseIntArrayElements(env, jresult, result_array, 0);
    (*env)->ReleaseIntArrayElements(env, jresult_txroe, result_txroe_array, 0);
    (*env)->ReleaseIntArrayElements(env, jresult_rxroe, result_rxroe_array, 0);
    (*env)->ReleaseIntArrayElements(env, jsize_result_txroe, size_txroe_array, 0);
    (*env)->ReleaseIntArrayElements(env, jsize_result_rxroe, size_rxroe_array, 0);
    return retval;
}

/*
 * Function:  runProductionTestExTRxShortJNI
 * --------------------
 * perform the extended trx short test
 *
 * return  0, pass
 *        >0, number of failure
 *        <0, error out
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_runProductionTestExTRxShortJNI(
        JNIEnv *env, jobject obj, jintArray jlimit, jint jsize_limit, jintArray jlimit_ex_pin,
        jint jsize_limit_ex_pin, jintArray jresult_data, jint jsize_result_data,
        jintArray jresult_data_ex_pin, jint jsize_result_data_ex_pin,
        jintArray jresult_pin, jint jsize_result_pin)
{
    int retval;
    int *native_limit = NULL;
    int *native_limit_ex_pin = NULL;
    int *native_result_data = NULL;
    int *native_result_data_ex_pin = NULL;
    int *native_result_pin = NULL;
    jsize len_array;
    jboolean isCopy;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* get an array of test limit from java layer */
    if (jsize_limit > 0) {
        native_limit = (*env)->GetIntArrayElements(env, jlimit, &isCopy);
        len_array = (*env)->GetArrayLength(env, jlimit);
        if ((len_array <= 0) || (len_array !=  jsize_limit)) {
            printf_e("%s: invalid parameter. (jsize_limit = %d, array_len = %d)\n",
                     __FUNCTION__, jsize_limit, len_array);
            retval = -10;
            goto exit;
        }
    }

    /* get an array of test limit for extended pins from java layer */
    if (jsize_limit_ex_pin > 0) {
        native_limit_ex_pin = (*env)->GetIntArrayElements(env, jlimit_ex_pin, &isCopy);
        len_array = (*env)->GetArrayLength(env, jlimit_ex_pin);
        if ((len_array <= 0) || (len_array !=  jsize_limit_ex_pin)) {
            printf_e("%s: invalid parameter. (jsize_limit_ex_pin = %d, array_len = %d)\n",
                     __FUNCTION__, jsize_limit_ex_pin, len_array);
            retval = -10;
            goto exit;
        }
    }

    /* get an int array of test result data from java layer */
    if (jsize_result_data > 0) {
        native_result_data = (*env)->GetIntArrayElements(env, jresult_data, &isCopy);
        len_array = (*env)->GetArrayLength(env, jresult_data);
        if (len_array != jsize_result_data) {
            printf_e("%s: invalid parameter. (jsize_result_data = %d, len_array = %d)\n",
                     __FUNCTION__, jsize_result_data, len_array);
            retval = -10;
            goto exit;
        }
    }

    /* get an int array of test result data ex pin from java layer */
    if (jsize_result_data_ex_pin > 0) {
        native_result_data_ex_pin = (*env)->GetIntArrayElements(env, jresult_data_ex_pin, &isCopy);
        len_array = (*env)->GetArrayLength(env, jresult_data_ex_pin);
        if (len_array != jsize_result_data_ex_pin) {
            printf_e("%s: invalid parameter. (jsize_result_data_ex_pin = %d, len_array = %d)\n",
                     __FUNCTION__, jsize_result_data_ex_pin, len_array);
            retval = -10;
            goto exit;
        }
    }

    /* get an int array of pin result from java layer */
    if (jsize_result_pin > 0) {
        native_result_pin = (*env)->GetIntArrayElements(env, jresult_pin, &isCopy);
        len_array = (*env)->GetArrayLength(env, jresult_pin);
        if (len_array != jsize_result_pin) {
            printf_e("%s: invalid parameter. (jsize_result_pin = %d, len_array = %d)\n",
                     __FUNCTION__, jsize_result_pin, len_array);
            retval = -10;
            goto exit;
        }
    }

    retval = syna_run_test_ex_trx_short_entry(native_result_data, jsize_result_data,
                                              native_result_data_ex_pin, jsize_result_data_ex_pin,
                                              native_result_pin, jsize_result_pin,
                                              native_limit, jsize_limit,
                                              native_limit_ex_pin, jsize_limit_ex_pin);

    if (retval < 0) {
        printf_e("%s error: fail to run extended trx short test\n", __FUNCTION__);
    }

exit:
    (*env)->ReleaseIntArrayElements(env, jlimit, native_limit, 0);
    (*env)->ReleaseIntArrayElements(env, jlimit_ex_pin, native_limit_ex_pin, 0);
    (*env)->ReleaseIntArrayElements(env, jresult_data, native_result_data, 0);
    (*env)->ReleaseIntArrayElements(env, jresult_data_ex_pin, native_result_data_ex_pin, 0);
    (*env)->ReleaseIntArrayElements(env, jresult_pin, native_result_pin, 0);
    return retval;
}

/*
 * Function:  getFwConfigSizeJNI
 * --------------------
 * retrieve the size of firmware config in byte
 */
JNIEXPORT jint JNICALL Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getFwConfigSizeJNI(
        JNIEnv *env, jobject obj)
{
    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    return syna_get_firmware_config_size();
}
/*
 * Function:  getFwConfigJNI
 * --------------------
 * retrieve the firmware configuration
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getFwConfigJNI(
        JNIEnv *env, jobject obj, jbyteArray array, jint size_of_array)
{
    int retval;
    jbyte *native_array;
    jsize len_data_array;
    jboolean isCopy;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* retrieve a short array from java layer */
    native_array = (*env)->GetByteArrayElements(env, array, &isCopy);
    len_data_array = (*env)->GetArrayLength(env, array);
    if (len_data_array != size_of_array) {
        (*env)->ReleaseByteArrayElements(env, array, native_array, 0);
        printf_e("%s error: invalid parameter. (len_data_array = %d)\n", __FUNCTION__, len_data_array);
        return (jboolean)false;
    }

    retval = syna_get_firmware_config((unsigned char*)native_array, size_of_array);
    if (retval < 0) {
        printf_e("%s error: fail to get firmware config\n", __FUNCTION__);
        /* release the java array */
        (*env)->ReleaseByteArrayElements(env, array, native_array, 0);
        return (jboolean)false;
    }

    /* release the java array */
    (*env)->ReleaseByteArrayElements(env, array, native_array, 0);
    return (jboolean)true;
}
/*
 * Function:  queryTouchResponseJNI
 * --------------------
 * query a touch report
 * it can assign the maximum number of fingers being reported
 */
JNIEXPORT jint JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_queryTouchResponseJNI(
        JNIEnv *env, jobject obj, jint max_fingers)
{
    int retval;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* try to query the force data */
    retval = syna_query_touch_response_entry(max_fingers);
    if (retval < 0) {
        printf_e("%s error: fail to query the touch response\n", __FUNCTION__);
        return retval;
    }

    return retval;
}

/*
 * Function:  callback_finger_down
 * --------------------
 * the callback function to notify the activity a finger-down event
 */
void callback_finger_down(int index, int x_pos, int y_pos)
{
    printf_i("callback_finger_down + \n");

    jclass clazz = (*g_jni_env)->FindClass(g_jni_env, "com/vivotouchscreen/sensortestsyna3618f/NativeWrapper");
    if (clazz == 0) {
        printf_e("%s error: fail on FindClass, com/vivotouchscreen/sensortestsyna3618f/NativeWrapper\n", __FUNCTION__);
        return;
    }
    jmethodID javamethod = (*g_jni_env)->GetMethodID(g_jni_env, clazz, "callbackFingerDown", "(III)V");
    if (javamethod == 0) {
        printf_e("%s error: fail on GetMethodID, callbackFingerDown\n", __FUNCTION__);
        return;
    }
    (*g_jni_env)->CallVoidMethod(g_jni_env, g_jni_obj, javamethod, index, x_pos, y_pos);

}

/*
 * Function:  callback_finger_up
 * --------------------
 * the callback function to notify the activity a finger-up event
 */
void callback_finger_up(int index)
{
    printf_i("callback_one_finger_up + \n");

    jclass clazz = (*g_jni_env)->FindClass(g_jni_env, "com/vivotouchscreen/sensortestsyna3618f/NativeWrapper");
    if (clazz == 0) {
        printf_e("%s error: fail on FindClass, com/vivotouchscreen/sensortestsyna3618f/NativeWrapper\n", __FUNCTION__);
        return;
    }
    jmethodID javamethod = (*g_jni_env)->GetMethodID(g_jni_env, clazz, "callbackFingerUp", "(I)V");
    if (javamethod == 0) {
        printf_e("%s error: fail on GetMethodID, callbackFingerUp\n", __FUNCTION__);
        return;
    }
    (*g_jni_env)->CallVoidMethod(g_jni_env, g_jni_obj, javamethod, index);

}
/*
 * Function:  runRawCommandJNI
 * --------------------
 * execute the given raw command
 */
JNIEXPORT jint JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_runRawCommandJNI(
        JNIEnv *env, jobject obj, jbyte type, jint cmd,
        jbyteArray in, jint size_of_in, jbyteArray resp, jint size_of_resp)
{
    int retval;
    jbyte *native_in;
    jbyte *native_resp = NULL;
    jsize len_array;
    jboolean isCopy;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    /* get a char array from java layer */
    native_in = (*env)->GetByteArrayElements(env, in, &isCopy);
    len_array = (*env)->GetArrayLength(env, in);
    if (len_array != size_of_in) {
        printf_e("%s error: invalid parameter of in array. (len_array = %d)\n", __FUNCTION__, len_array);
        retval = -1;
        goto exit;
    }

    /* get a char array from java layer */
    if (size_of_resp > 0) {
        native_resp = (*env)->GetByteArrayElements(env, resp, &isCopy);
        len_array = (*env)->GetArrayLength(env, resp);
        if (len_array != size_of_resp) {
            printf_e("%s error: invalid parameter of out array (len_array = %d)\n", __FUNCTION__, len_array);
            retval = -1;
            goto exit;
        }
    }

    retval = syna_run_raw_command((unsigned char)type, cmd,
                                  (unsigned char*)native_in, size_of_in,
                                  (unsigned char*)native_resp, size_of_resp);
    if (retval < 0) {
        printf_e("%s error: fail to run the command\n", __FUNCTION__);
    }

exit:
    /* release the java array */
    (*env)->ReleaseByteArrayElements(env, in, native_in, 0);
    if (native_resp)
        (*env)->ReleaseByteArrayElements(env, resp, native_resp, 0);
    return retval;
}
/*
 * Function:  getPinsMappingJNI
 * --------------------
 * inquiry the pin assignment
 */
JNIEXPORT jboolean JNICALL  Java_com_vivotouchscreen_sensortestsyna3618f_NativeWrapper_getPinsMappingJNI(
        JNIEnv *env, jobject obj, jint rxes_offset, jint rxes_len, jint txes_offset, jint txes_len)
{
    int retval;

    /* save JNIEnv */
    g_jni_env = env;
    g_jni_obj = obj;

    retval = syna_get_pins_mapping(rxes_offset, rxes_len, txes_offset, txes_len);
    if (retval < 0) {
        printf_e("%s error: fail to retrieve trx pins mapping\n", __FUNCTION__);
        return (jboolean)false;
    }

    return (jboolean)true;
}


