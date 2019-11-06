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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "syna_dev_manager.h"
#include "rmi_control.h"

#ifdef SAVE_ERR_MSG
#include "err_msg_ctrl.h"
#endif

#define POLLING_TOUCH_REPORT_CNT 50
#define POLLING_TOUCH_REPORT_DELAY_MS 10

#define MAX_RMI_REPORTED_FINGERS 10

/*
 * Function:  rmi_f12_get_touch_report
 * --------------------
 * retrieve the reported touch position
 * from rmi function 12
 *
 * return: 0<, fail to get the touch report
 *         otherwise, return the number of touch points
 */
static int rmi_f12_get_touch_report(int *pos_x, int size_of_pos_x, int *pos_y, int size_of_pos_y,
                             int *pos_status, int size_of_pos_status, int max_points_reported)
{
    int retval;
    unsigned char touch_count = 0; /* number of touch points */
    unsigned char index;
    unsigned char finger;
    unsigned char fingers_to_process;
    unsigned char finger_status;
    unsigned char size_of_2d_data;
    unsigned short data_addr = g_rmi_pdt.F12.data_base_addr;
    int x;
    int y;
    struct f12_finger_data *data;
    struct f12_finger_data *finger_data;
    static unsigned char finger_presence;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* F12_DATA_15_WORKAROUND */
    static unsigned char objects_already_present;

    if ((!pos_x) || (!pos_y) || (!pos_status)) {
        printf_e("%s error: invalid parameter\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter\n", __func__);
        add_error_msg(err);
#endif
        return (-EINVAL);
    }

    if ((size_of_pos_x < max_points_reported) || (size_of_pos_y < max_points_reported) ||
        (size_of_pos_status < max_points_reported)) {
        printf_e("%s error: invalid buf size\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid buf size\n", __func__);
        add_error_msg(err);
#endif
        return (-EINVAL);
    }

    fingers_to_process = (unsigned char)(g_rmi_pdt.num_of_fingers_supported);

    size_of_2d_data = sizeof(struct f12_finger_data);

    /* determine the total number of fingers to process */
    if (g_rmi_pdt.f12_extra.data15_size) {
        retval = rmi_read_reg(data_addr + g_rmi_pdt.f12_extra.data15_offset,
                              g_rmi_pdt.f12_extra.data15_data,
                              g_rmi_pdt.f12_extra.data15_size);
        if (retval < 0) {
            printf_e("%s error: fail to read f12 data_15\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read f12 data_15\n", __func__);
            add_error_msg(err);
#endif
            return retval;
        }
        /* start checking from the highest bit */
        index = (unsigned char)(g_rmi_pdt.f12_extra.data15_size - 1); /* highest byte */
        finger = (unsigned char)((fingers_to_process - 1) % 8); /* highest bit */
        do {
            if (g_rmi_pdt.f12_extra.data15_data[index] & (1 << finger))
                break;

            if (finger) {
                finger--;
            } else if (index > 0) {
                index--; /* move to the next lower byte */
                finger = 7;
            }

            fingers_to_process--;
        } while (fingers_to_process);

        printf_i("%s: number of fingers to process = %d\n",
                 __func__, fingers_to_process);
    }

    /* F12_DATA_15_WORKAROUND */
    fingers_to_process = (fingers_to_process > objects_already_present)?
                         fingers_to_process : objects_already_present;

    if (!fingers_to_process) {
        return 0;
    }

    retval = rmi_read_reg(data_addr + g_rmi_pdt.f12_extra.data1_offset,
                          (unsigned char *)g_rmi_pdt.f12_finger_data,
                          fingers_to_process * size_of_2d_data);
    if (retval < 0) {
        printf_e("%s error: fail to read f12 data_01\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read f12 data_01\n", __func__);
        add_error_msg(err);
#endif
        return retval;
    }

    data = g_rmi_pdt.f12_finger_data;

    for (finger = 0; finger < max_points_reported; finger++) {
        finger_data = data + finger;
        finger_status = finger_data->object_type_and_status;

        /* F12_DATA_15_WORKAROUND */
        objects_already_present = (unsigned char)(finger + 1);

        x = (finger_data->x_msb << 8) | (finger_data->x_lsb);
        y = (finger_data->y_msb << 8) | (finger_data->y_lsb);

        switch (finger_status) {
            case F12_FINGER_STATUS:

                printf_i("%s: finger %d: status = 0x%02x, x = %d, y = %d\n",
                         __func__, finger,  finger_status, x, y);

                pos_x[finger] = x;
                pos_y[finger] = y;
                pos_status[finger] = finger_status;

                finger_presence = 1;
                touch_count++;
                break;
            default:
                break;
        }
    }

    if (touch_count == 0) {
        finger_presence = 0;
        /* F12_DATA_15_WORKAROUND */
        objects_already_present = 0;
    }

    return (finger_presence)? touch_count : 0;
}
/*
 * Function:  rmi_query_touch_response
 * --------------------
 * the procedure will be
 *   1. polling the interrupt status
 *   2. return 0 if nothing
 *   3. once detecting a object landing
 *      return the number of object reported
 *
 *
 * return: < 0 - fail to get touch data
 *         = 0 - no data
 *         > 0 - successfully collect the data of 1 points
 */
int rmi_query_touch_response(int *touch_x, int* touch_y, int* touch_status,
                             int max_fingers_to_process)
{
    int retval;
    int retry = 0;
    unsigned char data[MAX_INTR_REGISTERS + 1] = {0};
    int x[MAX_RMI_REPORTED_FINGERS] = {0};
    int y[MAX_RMI_REPORTED_FINGERS] = {0};
    int status[MAX_RMI_REPORTED_FINGERS] = {0};
    bool is_finger_event = false;
    int idx;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    do {
        /*
         * get interrupt status information from F01 Data1 register to
         * determine the source(s) that are flagging the interrupt.
         */
        retval = rmi_read_reg(g_rmi_pdt.F01.data_base_addr,
                              data,
                              2);
        if (retval < 0) {
            printf_e("%s error: fail to read interrupt status\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read interrupt status\n", __func__);
            add_error_msg(err);
#endif
            return (-EIO);
        }

        if (data[1] & 0x04) {
            retval = rmi_f12_get_touch_report(x, MAX_RMI_REPORTED_FINGERS,
                                              y, MAX_RMI_REPORTED_FINGERS,
                                              status, MAX_RMI_REPORTED_FINGERS,
                                              max_fingers_to_process);
            if (retval < 0){
                printf_e("%s error: fail to get the touch report\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to get the touch report\n", __func__);
                add_error_msg(err);
#endif
                return (-EIO);
            }

            is_finger_event = true;
            break;
        }
        else {
            retval = 0;
        }

        retry += 1;
        usleep(POLLING_TOUCH_REPORT_DELAY_MS * 1000);

    } while( (retry < POLLING_TOUCH_REPORT_CNT) );

    /* once detecting touched object */
    if (is_finger_event) {

        for (idx = 0; idx < MAX_RMI_REPORTED_FINGERS; idx++) {
            touch_x[idx] = x[idx];
            touch_y[idx] = y[idx];
            touch_status[idx] = status[idx];
        }

    }

    return retval;
}
