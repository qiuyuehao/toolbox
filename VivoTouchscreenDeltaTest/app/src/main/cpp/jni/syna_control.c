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
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "native-lib.h"
#include "rmi_control.h"
#include "tcm_control.h"

#define RMI_PATH "/dev/rmi"
#define TCM_PATH "/dev/tcm"

#define NUMBER_OF_NODES_TO_SCAN (10)


bool is_rmi_dev_existed()
{
    int i;
    struct stat st;

    g_is_rmi_dev = false;
    g_is_rmi_dev_initialized = false;

    for (i = 0; i < NUMBER_OF_NODES_TO_SCAN; i++) {
        memset(g_dev_node, 0x00, MAX_STRING_LEN);
        // grep the path of rmi character device
        snprintf(g_dev_node, MAX_STRING_LEN, "%s%d", RMI_PATH, i);
        if (0 == stat(g_dev_node, &st)) {
            g_is_rmi_dev = true;
            printf_i("%s: dev_rmi_path = %s\n", __FUNCTION__, g_dev_node);
            break;
        }
    }

    if(!g_is_rmi_dev){
        g_is_rmi_dev = false;
        snprintf(g_dev_node, MAX_STRING_LEN, "%s0", RMI_PATH);
        printf_e("%s: can't find the proper %s device\n", __FUNCTION__, g_dev_node);
    }
    else {
        /* open rmi interface and parse the pdt */
        if (rmi_open_dev(g_dev_node) < 0) {
            return false;
        }

        if (rmi_scan_pdt() < 0) {
            rmi_close_dev(g_dev_node);
            g_is_rmi_dev_initialized = false;
            return false;
        }

        g_is_rmi_dev_initialized = true; /* set true to avoid re-parsing the pdt */
        rmi_close_dev(g_dev_node);
    }

    return g_is_rmi_dev;
}

bool is_tcm_dev_existed()
{
    int i;
    struct stat st;

    g_is_tcm_dev = false;
    g_is_tcm_dev_initialized = false;

    /* scan the possible device node */
    for (i = 0; i < NUMBER_OF_NODES_TO_SCAN; i++) {
        memset(g_dev_node, 0x00, MAX_STRING_LEN);
        /* grep the path of rmi character device */
        snprintf(g_dev_node, MAX_STRING_LEN, "%s%d", TCM_PATH, i);
        if (0 == stat(g_dev_node, &st)) {
            g_is_tcm_dev = true;
            break;
        }
    }

    if(!g_is_tcm_dev){
        g_is_tcm_dev = false;
        snprintf(g_dev_node, MAX_STRING_LEN, "%s0", TCM_PATH);
        printf_e("%s: can't find the proper %s device\n", __FUNCTION__, g_dev_node);
    }
    else {
        /* open tcm interface and get the identify report */
        if (tcm_open_dev(g_dev_node) < 0) {
            return false;
        }

        if (tcm_do_identify() < 0) {
            tcm_close_dev(g_dev_node);
            g_is_tcm_dev_initialized = false;
            return false;
        }

        g_is_tcm_dev_initialized = true; /* set true to avoid re-parsing the pdt */
        tcm_close_dev(g_dev_node);
    }

    return g_is_tcm_dev;
}

int open_dev(const char* dev_path)
{
    int retval = 0;

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            retval = rmi_open_dev(dev_path);
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            retval = tcm_open_dev(dev_path);
        }
    }

    return retval;
}

void close_dev(const char* dev_path)
{
    if (g_is_rmi_dev) {
        rmi_close_dev(dev_path);
    }
    else if (g_is_tcm_dev) {
        tcm_close_dev(dev_path);
    }
}

int get_asic_id(void)
{
    int retval = 0;

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            retval = rmi_get_asic_type();
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            retval = tcm_get_asic_id();
        }
    }

    return retval;
}

int get_build_id(void)
{
    int retval = 0;

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            retval = rmi_get_build_id();
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            retval = tcm_get_build_id();
        }
    }

    return retval;
}

int get_gear_info(char *info)
{
    if (!info) {
        printf_e("%s: input buffer is null\n", __FUNCTION__);
        return -EINVAL;
    }

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            rmi_get_f54_gear_info(info);
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            tcm_get_gear_info(info);
        }
    }

    return 0;
}

int get_finger_cap(void)
{
    int retval = 0;

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            retval = rmi_get_finger_cap();
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            retval = tcm_get_finger_cap();
        }
    }
    return retval;
}

int get_rmi_tx_info(void)
{
    int data = -1;
    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            data = g_rmi_pdt.tx_assigned;
        }
    }
    return data;
}

int get_rmi_rx_info(void)
{
    int data = -1;
    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            data = g_rmi_pdt.rx_assigned;
        }
    }
    return data;
}

int get_tcm_row_info(void)
{
    int data = -1;
    if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            data = tcm_get_image_rows();
        }
    }
    return data;
}

int get_tcm_col_info(void)
{
    int data = -1;
    if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            data = tcm_get_image_cols();
        }
    }
    return data;
}

int enable_one_specified_gear(int gear)
{
    int retval = 0;

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            retval = rmi_f54_enable_one_gear(gear);
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            retval = tcm_request_freq_gear(gear);
        }
    }

    return retval;
}

int write_log_header(FILE *pfile, int total_frames, int num_gears)
{
    if (!pfile) {
        printf_e("%s: input file pointer is null\n", __FUNCTION__);
        return -EINVAL;
    }

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            rmi_write_log_header(pfile, total_frames, num_gears);
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            tcm_write_log_header(pfile, total_frames, num_gears);
        }
    }

    return 0;
}

int do_test_preparation()
{
    int retval = 0;

    printf_i("%s: entry + \n", __FUNCTION__);

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            rmi_f01_set_no_sleep();
            retval = 0;
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            tcm_enable_report(false, REPORT_TOUCH);
            retval = tcm_enable_report(true, REPORT_DELTA);

            tcm_set_no_sleep(1);
        }
    }

    return retval;
}

int do_test_completion()
{
    int retval = 0;

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            retval = 0;
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            retval = tcm_enable_report(false, REPORT_DELTA);
            tcm_enable_report(true, REPORT_TOUCH);
        }
    }

    return retval;
}

bool do_noise_test(FILE *pfile, char *timestamp, int frame_id, int gear_idx)
{
    bool result = false;

    if (!pfile) {
        printf_e("%s: input file pointer is null\n", __FUNCTION__);
        return false;
    }

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            result = rmi_do_noise_test(pfile, timestamp, frame_id, gear_idx);
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            result = tcm_do_noise_test(pfile, timestamp, frame_id, gear_idx);
        }
    }
    return result;
}

bool start_report(bool is_delta, bool is_raw)
{
    printf_i("%s: entry + \n", __FUNCTION__);

    int retval = open_dev(g_dev_node);
    if ( retval < 0) {
        return false;
    }

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            retval = rmi_f01_set_no_sleep();
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            tcm_enable_report(false, REPORT_TOUCH);
            if (is_delta)
                tcm_enable_report(true, REPORT_DELTA);
            if (is_raw)
                tcm_enable_report(true, REPORT_RAW);

            retval = tcm_set_no_sleep(1);
        }
    }

    if ( retval < 0) {
        printf_e("%s: fail to do preparation \n", __FUNCTION__);
        return false;
    }

    return true;
}

bool stop_report(bool is_delta, bool is_raw)
{
    printf_i("%s: entry + \n", __FUNCTION__);
    int retval = 0;

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            retval = 0;
        }
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {
            if (is_delta)
                tcm_enable_report(false, REPORT_DELTA);
            if (is_raw)
                tcm_enable_report(false, REPORT_RAW);

            retval = tcm_enable_report(true, REPORT_TOUCH);
        }
    }

    if ( retval < 0) {
        printf_e("%s: fail to do preparation \n", __FUNCTION__);
        return false;
    }


    /* close synaptics device interface */
    close_dev(g_dev_node);

    return true;
}

int read_report(bool is_delta, bool is_raw, short *p_image)
{
    int retval = -1;
    unsigned char type = 0;

    if (g_is_rmi_dev) {
        if (g_is_rmi_dev_initialized) {
            if (is_delta)
                type = 0x02;
            if (is_raw)
                type = 0x03;

            /* get one requested report frame */
            retval = read_rmi_f54_report((int)type, p_image);
            if (retval < 0) {
                printf_e("%s error: fail to read the rmi report\n", __func__);
                goto exit;
            }
        }
        else
            printf_e("%s error: rmi device is not initialized\n", __func__);
    }
    else if (g_is_tcm_dev) {
        if (g_is_tcm_dev_initialized) {

            if (is_delta)
                type = REPORT_DELTA;
            if (is_raw)
                type = REPORT_RAW;

            retval = tcm_read_report_image(p_image, type, tcm_get_image_cols(), tcm_get_image_rows());
            if (retval < 0) {
                printf_e("%s error: fail to read the tcm report\n", __func__);
                goto exit;
            }
        }
        else
            printf_e("%s error: rmi device is not initialized\n", __func__);
    }

exit:
    return retval;
}
