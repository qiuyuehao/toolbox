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
#include <string.h>

#include "syna_dev_manager.h"
#include "tcm_control.h"

#ifdef SAVE_ERR_MSG
#include "err_msg_ctrl.h"
#endif

extern void extended_high_resistance_test(
        unsigned char rx_2d_channel, 	/* IN: Rx Channel number for 2D area*/
        unsigned char tx_2d_channel, 	/* IN: Tx Channel number for 2D area */
        signed short * delta_2d_image, 	/* IN: Pointer to the delta image for 2D area */
        signed short * baseline_image, 	/* IN: Pointer to the baseline image for 2D area */
        signed short * ref_2d_image, 	/* IN: Pointer to the reference raw image for 2D area */
        signed short * rx_Result, 		/* OUT: Pointer to Rx_Result arry, Rx result will be kept in this arry */
        signed short * tx_Result, 		/* OUT: Pointer to Tx_Result arry, Tx result will be kept in this arry */
        signed short * surface_Result	/* OUT: Pointer to Surface_Result arry, surface result will be kept in this arry */
);


/*
 * Function:  tcm_send_test_command
 * --------------------
 * send the tcm command to perform the specific testing
 *
 * return: <0, fail to issue a test request
 *         otherwise, the size of testing data
 */
static int tcm_send_test_command(enum tcm_test_type test_id)
{
    int retval = 0;
    unsigned char command_packet[4] = {0, 0, 0, 0};
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* command code */
    command_packet[0] = CMD_PRODUCTION_TEST;

    /* command length */
    /* length = 1 */
    command_packet[1] = 0x01;
    command_packet[2] = 0x00;
    /* command payload, 1 byte*/
    /* byte[1] = report type */
    command_packet[3] = (unsigned char) test_id;
    /* send command to tcm device */
    retval = tcm_write_message(command_packet, sizeof(command_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_PRODUCTION_TEST 0x%x 0x%x 0x%x\n",
                 __func__, command_packet[1], command_packet[2], command_packet[3]);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send command, CMD_PRODUCTION_TEST 0x%x 0x%x 0x%x\n",
                __func__, command_packet[1], command_packet[2], command_packet[3]);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }
    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: fail to get the response of command, (0x%x 0x%x 0x%x 0x%x) \n",
                 __func__, command_packet[0], command_packet[1], command_packet[2], command_packet[3]);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get the response of command, (0x%x 0x%x 0x%x 0x%x) \n",
                __func__, command_packet[0], command_packet[1], command_packet[2], command_packet[3]);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }
    printf_i("%s info: CMD_PRODUCTION_TEST is completed. (test_id:0x%x)", __func__, (int)test_id);

    return retval;
}

/*
 * Function:  tcm_reorder_test_data_frame
 * --------------------
 * helper to reorder the data format in landscape layout
 *
 * return: n/a
 */
static int tcm_reorder_test_data_frame(unsigned char *p_in, unsigned short *p_out)
{
    unsigned short *p_data_16;
    int i, j, offset;
    int rows = (g_tcm_handler.app_info_report.num_of_image_rows[0] |
                g_tcm_handler.app_info_report.num_of_image_rows[1] << 8);
    int cols = (g_tcm_handler.app_info_report.num_of_image_cols[0] |
                g_tcm_handler.app_info_report.num_of_image_cols[1] << 8);

    if ((!p_in) || (!p_out)) {
        printf_e("%s error: invalid parameter\n", __func__);
        return -EINVAL;
    }

    /* data copy into landscape format */
    p_data_16 = (unsigned short *)&p_in[0];
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {

            if (cols > rows)
                offset = i*cols + j;
            else
                offset = j*rows + i;

            p_out[offset] = *p_data_16;
            p_data_16++;
        }
    }

    return 0;
}


/*
 * Function:  tcm_do_test_drt_pid07
 * --------------------
 * perform the dynamic range test PID 0x07
 *
 * return: <0, fail to perform the testing
 *         =0, pass the dynamic range test
 *         >0, failure count
 */
int tcm_do_test_drt_pid07(int *p_result_img, int result_img_col, int result_img_row,
                          int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    short data;
    short *data_buf_16 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int limit_min_data;
    int limit_max_data;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((!limit_min) || (0 == size_limit_min) || (!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, size limit (min,max) = (%d, %d)\n",
                 __func__, size_limit_min, size_limit_max);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_DRT);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_DRT\n",
                __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: dynamic range pid07 test (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_16 = calloc((size_t)(col * row), sizeof(short));
    if (!data_buf_16) {
        printf_e("%s error: can't allocate memory for data_buf_16\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_16\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* reorder the data into landscape mode */
    retval = tcm_reorder_test_data_frame(data_buf, (unsigned short *)data_buf_16);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to reorder the testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to reorder the testing data\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            data = data_buf_16[i * row + j] ;
            p_result_img[i * row + j] = data;

            // determine the limit min
            if (size_limit_min == (col * row)) {
                limit_min_data = limit_min[idx_limit];
            }
            else {
                limit_min_data = limit_min[0];
            }
            // determine the limit max
            if (size_limit_max == (col * row)) {
                limit_max_data = limit_max[idx_limit];
            }
            else {
                limit_max_data = limit_max[0];
            }

            /* lower bound checking, value should not be lower than the test limit */
            if (data < limit_min_data) {
                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit min = %5d\n", __func__,
                         i, j, data, limit_min_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                        i, j, data, limit_min_data);
                add_error_msg(err);
#endif
            }
            /* upper bound checking, value should not be higher than the test limit */
            if (data > limit_max_data) {
                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit max = %5d\n", __func__,
                         i, j, data, limit_max_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit max = %5d\n",
                        i, j, data, limit_max_data);
                add_error_msg(err);
#endif
            }


            idx_limit += 1;
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_16)
        free(data_buf_16);

    return (retval < 0)? retval : failure_cnt;
}


/*
 * Function:  tcm_do_test_noise_pid0a
 * --------------------
 * perform the noise test PID 0x0A
 *
 * return: <0, fail to perform the testing
 *         =0, pass the dynamic range test
 *         >0, failure count
 */
int tcm_do_test_noise_pid0a(int *p_result_img, int result_img_col, int result_img_row,
                            int *limit_max, int size_limit_max)
{
    int retval = 0;
    short data;
    short *data_buf_16 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, no test limit input\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_NOISE);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_NOISE\n",
                __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: noise test pid0a (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_16 = calloc((size_t)(col * row), sizeof(short));
    if (!data_buf_16) {
        printf_e("%s error: can't allocate memory for data_buf_16\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_16\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }


    /* reorder the data into landscape mode */
    retval = tcm_reorder_test_data_frame(data_buf, (unsigned short *)data_buf_16);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to reorder the testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to reorder the testing data\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            data = data_buf_16[i * row + j] ;
            p_result_img[i * row + j] = data;

            /* data verification, value should not be higher than the test limit */
            if (size_limit_max == (col * row)) {
                /* if it is a entire limit frame, check every tixels */
                if (abs(data) > limit_max[idx_limit]) {
                    failure_cnt += 1;
                    printf_e("%s error: fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                             __func__, i, j, data, limit_max[idx_limit]);
#ifdef SAVE_ERR_MSG
                    sprintf(err, "fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                            i, j, data, limit_max[idx_limit]);
                    add_error_msg(err);
#endif
                }
            }
            else {
                /* if it is a single vale, compare with it only */
                if (abs(data) > limit_max[0]) {
                    failure_cnt += 1;
                    printf_e("%s error: fail at (%2d, %2d) data = %5d, limit max = %5d\n",
                             __func__, i, j, data, limit_max[0]);
#ifdef SAVE_ERR_MSG
                    sprintf(err, "fail at (%2d, %2d) data = %5d, limit max = %5d\n",
                            i, j, data, limit_max[0]);
                    add_error_msg(err);
#endif
                }
            }

            idx_limit += 1;
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_16)
        free(data_buf_16);

    return (retval < 0)? retval : failure_cnt;
}


/*
 * Function:  tcm_do_test_pt11_pid0b
 * --------------------
 * perform the pt11 test PID 0x0B
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int tcm_do_test_pt11_pid0b(int *p_result_img, int result_img_col, int result_img_row,
                           int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    short data;
    short *data_buf_16 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int limit_min_data;
    int limit_max_data;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((!limit_min) || (0 == size_limit_min) || (!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, size limit (min,max) = (%d, %d)\n",
                 __func__, size_limit_min, size_limit_max);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_PT11);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_PT11\n",
                __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: tcm pt11 test pid0b (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_16 = calloc((size_t)(col * row), sizeof(short));
    if (!data_buf_16) {
        printf_e("%s error: can't allocate memory for data_buf_16\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_16\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* reorder the data into landscape mode */
    retval = tcm_reorder_test_data_frame(data_buf, (unsigned short *)data_buf_16);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to reorder the testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to reorder the testing data\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            data = data_buf_16[i * row + j] ;
            p_result_img[i * row + j] = data;

            // determine the limit min
            if (size_limit_min == (col * row)) {
                limit_min_data = limit_min[idx_limit];
            }
            else {
                limit_min_data = limit_min[0];
            }
            // determine the limit max
            if (size_limit_max == (col * row)) {
                limit_max_data = limit_max[idx_limit];
            }
            else {
                limit_max_data = limit_max[0];
            }

            /* lower bound checking, value should not be lower than the test limit */
            if (data < limit_min_data) {
                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit min = %5d\n", __func__,
                         i, j, data, limit_min_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                        i, j, data, limit_min_data);
                add_error_msg(err);
#endif
            }
            /* upper bound checking, value should not be higher than the test limit */
            if (data > limit_max_data) {
                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit max = %5d\n", __func__,
                         i, j, data, limit_max_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit max = %5d\n",
                        i, j, data, limit_max_data);
                add_error_msg(err);
#endif
            }


            idx_limit += 1;
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_16)
        free(data_buf_16);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  tcm_do_test_pt12_pid0c
 * --------------------
 * perform the pt12 test PID 0x0C
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int tcm_do_test_pt12_pid0c(int *p_result_img, int result_img_col, int result_img_row,
                           int *limit_min, int size_limit_min)
{
    int retval = 0;
    short data;
    short *data_buf_16 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int limit_min_data;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if (0 == size_limit_min) {
        printf_e("%s error: invalid parameter, size limit min = %d\n", __func__, size_limit_min);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_PT12);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_PT12\n",
                __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: tcm pt12 test pid0c (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_16 = calloc((size_t)(col * row), sizeof(short));
    if (!data_buf_16) {
        printf_e("%s error: can't allocate memory for data_buf_16\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_16\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* reorder the data into landscape mode */
    retval = tcm_reorder_test_data_frame(data_buf, (unsigned short *)data_buf_16);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to reorder the testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to reorder the testing data\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            data = data_buf_16[i * row + j] ;
            p_result_img[i * row + j] = data;

            // determine the limit min
            if (size_limit_min == (col * row)) {
                limit_min_data = limit_min[idx_limit];
            }
            else {
                limit_min_data = limit_min[0];
            }

            /* lower bound checking, value should not be lower than the test limit */
            if (data < limit_min_data) {
                failure_cnt += 1;
                printf_e("%s: fail at (%2d, %2d) data = %5d, limit min = %5d\n", __func__,
                         i, j, data, limit_min_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                        i, j, data, limit_min_data);
                add_error_msg(err);
#endif
            }

            idx_limit += 1;
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_16)
        free(data_buf_16);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  tcm_do_test_pt13_pid0d
 * --------------------
 * perform the pt13 test PID 0x0D
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int tcm_do_test_pt13_pid0d(int *p_result_img, int result_img_col, int result_img_row,
                           int *limit_min, int size_limit_min)
{
    int retval = 0;
    short data;
    short *data_buf_16 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int limit_min_data;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if (0 == size_limit_min) {
        printf_e("%s error: invalid parameter, size limit min = %d\n", __func__, size_limit_min);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }


    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_PT13);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%serror: fail to perform production_test command TEST_PT13\n",
                __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: tcm pt13 test pid0d (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_16 = calloc((size_t)(col * row), sizeof(short));
    if (!data_buf_16) {
        printf_e("%s error: can't allocate memory for data_buf_16\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_16\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* reorder the data into landscape mode */
    retval = tcm_reorder_test_data_frame(data_buf, (unsigned short *)data_buf_16);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to reorder the testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to reorder the testing data\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            data = data_buf_16[i * row + j] ;
            p_result_img[i * row + j] = data;

            // determine the limit min
            if (size_limit_min == (col * row)) {
                limit_min_data = limit_min[idx_limit];
            }
            else {
                limit_min_data = limit_min[0];
            }

            /* lower bound checking, value should not be lower than the test limit */
            if (data < limit_min_data) {
                failure_cnt += 1;
                printf_e("%s: fail at (%2d, %2d) data = %5d, limit min = %5d\n", __func__,
                         i, j, data, limit_min_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                        i, j, data, limit_min_data);
                add_error_msg(err);
#endif
            }

            idx_limit += 1;
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_16)
        free(data_buf_16);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  tcm_do_test_full_raw_pid05
 * --------------------
 * perform the discrete full raw test PID 0x05
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0,- failure count
 */
int tcm_do_test_full_raw_pid05(int *p_result_img, int result_img_col, int result_img_row,
                               int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    short data;
    short *data_buf_16 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int limit_min_data;
    int limit_max_data;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((!limit_min) || (0 == size_limit_min) || (!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, size limit (min,max) = (%d, %d)\n",
                 __func__, size_limit_min, size_limit_max);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_FULLRAW);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_FULLRAW\n",
                __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: full raw test pid05 (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_16 = calloc((size_t)(col * row), sizeof(short));
    if (!data_buf_16) {
        printf_e("%s error: can't allocate memory for data_buf_16\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_16\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* reorder the data into landscape mode */
    retval = tcm_reorder_test_data_frame(data_buf, (unsigned short *)data_buf_16);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to reorder the testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to reorder the testing data\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            data = data_buf_16[i * row + j] ;
            p_result_img[i * row + j] = data;

            // determine the limit min
            if (size_limit_min == (col * row)) {
                limit_min_data = limit_min[idx_limit];
            }
            else {
                limit_min_data = limit_min[0];
            }
            // determine the limit max
            if (size_limit_max == (col * row)) {
                limit_max_data = limit_max[idx_limit];
            }
            else {
                limit_max_data = limit_max[0];
            }

            /* lower bound checking, value should not be lower than the test limit */
            if (data < limit_min_data) {
                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit min = %5d\n", __func__,
                         i, j, data, limit_min_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                        i, j, data, limit_min_data);
                add_error_msg(err);
#endif
            }
            /* upper bound checking, value should not be higher than the test limit */
            if (data > limit_max_data) {
                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit max = %5d\n", __func__,
                         i, j, data, limit_max_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit max = %5d\n",
                        i, j, data, limit_max_data);
                add_error_msg(err);
#endif
            }


            idx_limit += 1;
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_16)
        free(data_buf_16);

    return (retval < 0)? retval : failure_cnt;
}


/*
 * Function:  tcm_is_pins_assigned
 * --------------------
 * check whether the given is assigned
 *
 * return: false - no assiged
 *         true - is assiged
 */
static bool tcm_is_pins_assigned(int pin,
                                 int *tx_pins, int tx_assigned,
                                 int *rx_pins, int rx_assigned,
                                 int *guard_pins, int guard_assigned)
{
    int i;

    for (i = 0; i < tx_assigned; i++) {
        if (pin == tx_pins[i]) {
            return true;
        }
    }
    for (i = 0; i < rx_assigned; i++) {
        if (pin == rx_pins[i]) {
            return true;
        }
    }
    for (i = 0; i < guard_assigned; i++) {
        if (pin == guard_pins[i]) {
            return true;
        }
    }

    return false;
}

/*
 * Function:  tcm_do_test_trx_trx_short_pid01
 * --------------------
 * to perform the trx short test PID 0x01
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int tcm_do_test_trx_trx_short_pid01(int *p_result_data, int *pins_result, int size_of_pins_result,
                                    int *limit, int size_of_limit)
{
    int retval;
    unsigned char *data_buf = NULL;
    int data_payload = 0;
    int *tx_pins = g_tcm_handler.tx_pins;
    int tx_assigned = g_tcm_handler.tx_assigned;
    int *rx_pins = g_tcm_handler.rx_pins;
    int rx_assigned = g_tcm_handler.rx_assigned;
    int *guard_pins = g_tcm_handler.guard_pins;
    int guard_assigned = g_tcm_handler.guard_assigned;
    int failure_cnt = 0;
    int i, j;
    int phy_pin;
    bool do_pin_test;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_data) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if (!pins_result) {
        printf_e("%s error: pins_result is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: pins_result is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if (!limit) {
        printf_e("%s error: limit is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: limit is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if (size_of_pins_result != size_of_limit) {
        printf_e("%s error: size is mismatching (%d, %d)\n",
                 __func__, size_of_pins_result, size_of_limit);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: size is mismatching (%d, %d)\n",
                __func__, size_of_pins_result, size_of_limit);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_TRX_SENSOR_OPEN);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_TRX_SENSOR_OPEN\n",
                __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: trx trx short test pid01 (payload size = %d)\n", __func__, data_payload);

    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    if (data_payload > TCM_MAX_PINS/8) {
        printf_e("%s error: payload size is over 8 bytes\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: payload size is over 8 bytes (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    for (i = 0; i < data_payload; i++) {

        printf_i("%s info: pt01[%d]: 0x%2x\n", __func__, i , data_buf[i]);

        for (j = 0; j < 8; j++) {
            phy_pin = (i*8 + j);
            do_pin_test = tcm_is_pins_assigned(phy_pin,
                                               tx_pins, tx_assigned,
                                               rx_pins, rx_assigned,
                                               guard_pins, guard_assigned);
            if (do_pin_test) {

                if (phy_pin <= size_of_pins_result)
                    pins_result[phy_pin] = GET_BIT(data_buf[i], j);

            }
            else {
                if (phy_pin <= size_of_pins_result)
                    pins_result[phy_pin] = -1; /* no assigned */
            }
        }

        p_result_data[i] = data_buf[i];
    }

    for (i = 0; i < size_of_pins_result; i++) {
        if ((pins_result[i] != -1) && (pins_result[i] != limit[i])) {
            failure_cnt += 1;

            printf_e("%s error: fail at pin-%2d, data = %5d, limit = %5d\n",
                     __func__, i, pins_result[i], limit[i]);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at pin-%2d, data = %5d, limit = %5d\n",
                    i, pins_result[i], limit[i]);
            add_error_msg(err);
#endif
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  tcm_do_test_trx_ground_pid03
 * --------------------
 * perform the trx ground test PID 0x03
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0,- failure count
 */
int tcm_do_test_trx_ground_pid03(int *p_result_data, int *pins_result, int size_of_pins_result,
                                 int *limit, int size_of_limit)
{
    int retval;
    unsigned char *data_buf = NULL;
    int data_payload = 0;
    int failure_cnt = 0;
    int i, j;
    int phy_pin;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_data) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if (!pins_result) {
        printf_e("%s error: pins_result is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: pins_result is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if (!limit) {
        printf_e("%s error: limit is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: limit is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if (size_of_pins_result != size_of_limit) {
        printf_e("%s error: size is mismatching (%d, %d)\n",
                 __func__, size_of_pins_result, size_of_limit);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: size is mismatching (%d, %d)\n",
                __func__, size_of_pins_result, size_of_limit);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_TRX_GROUND);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command EST_TRX_SENSOR_OPEN\n",
                __func__);
        add_error_msg(err);
#endif
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: trx ground test pid03 (payload size = %d)\n", __func__, data_payload);

    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    if (data_payload > TCM_MAX_PINS/8) {
        printf_e("%s error: payload size is over 8 bytes\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: payload size is over 8 bytes (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    for (i = 0; i < data_payload; i++) {

        printf_i("%s info: pt03[%d]: 0x%2x\n", __func__, i , data_buf[i]);

        for (j = 0; j < 8; j++) {
            phy_pin = (i * 8 + j);

            if (phy_pin <= size_of_pins_result) {
                pins_result[phy_pin] = GET_BIT(data_buf[i], j);
            }
        }

        p_result_data[i] = data_buf[i];
    }

    for (i = 0; i < size_of_pins_result; i++) {
        if (pins_result[i] != limit[i]) {
            failure_cnt += 1;

            printf_e("%s error: fail at pin-%2d, data = %5d, limit = %5d\n",
                     __func__, i, pins_result[i], limit[i]);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at pin-%2d, data = %5d, limit = %5d\n",
                    i, pins_result[i], limit[i]);
            add_error_msg(err);
#endif
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  tcm_do_test_adc_range_pid11
 * --------------------
 * perform the adc range test PID 0x11
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int tcm_do_test_adc_range_pid11(int *p_result_img, int result_img_col, int result_img_row,
                                int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    unsigned short data;
    unsigned short *data_buf_16 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int limit_min_data;
    int limit_max_data;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((!limit_min) || (0 == size_limit_min) || (!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, size limit (min,max) = (%d, %d)\n",
                 __func__, size_limit_min, size_limit_max);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_ADC_RANGE);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_ADC_RANGE\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: adc range test pid11 (row = %d, col= %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_16 = calloc((size_t)(col * row), sizeof(unsigned short));
    if (!data_buf_16) {
        printf_e("%s error: can't allocate memory for data_buf_16\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_16\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }


    /* reorder the data into landscape mode */
    retval = tcm_reorder_test_data_frame(data_buf, data_buf_16);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to reorder the testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to reorder the testing data\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            data = data_buf_16[i * row + j] ;
            p_result_img[i * row + j] = data;

            // determine the limit min
            if (size_limit_min == (col * row)) {
                limit_min_data = limit_min[idx_limit];
            }
            else {
                limit_min_data = limit_min[0];
            }
            // determine the limit max
            if (size_limit_max == (col * row)) {
                limit_max_data = limit_max[idx_limit];
            }
            else {
                limit_max_data = limit_max[0];
            }

            /* lower bound checking, value should not be lower than the test limit */
            if (data < limit_min_data) {
                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                         __func__, i, j, data, limit_min_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                        i, j, data, limit_min_data);
                add_error_msg(err);
#endif
            }
            /* upper bound checking, value should not be higher than the test limit */
            if (data > limit_max_data) {
                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit max = %5d\n",
                         __func__, i, j, data, limit_max_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit max = %5d\n",
                        i, j, data, limit_max_data);
                add_error_msg(err);
#endif
            }


            idx_limit += 1;
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_16)
        free(data_buf_16);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  tcm_do_test_abs_rawcap_pid12
 * --------------------
 * perform the abs raw cap test PID 0x12
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int tcm_do_test_abs_rawcap_pid12(int *p_result_img, int result_img_col, int result_img_row,
                                 int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    int *p_data_int;
    int *data_buf_32 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i;
    int failure_cnt = 0;
    int limit_min_data;
    int limit_max_data;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((!limit_min) || (0 == size_limit_min) || (!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, size limit (min,max) = (%d, %d)\n",
                 __func__, size_limit_min, size_limit_max);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_ABS_RAW);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_ABS_RAW\n", __func__);
        add_error_msg(err);
#endif
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: abs raw cap test pid12 (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_32 = calloc((size_t)(col + row), sizeof(int));
    if (!data_buf_32) {
        printf_e("%s error: can't allocate memory for data_buf_32\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_32\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copy with right orientation */
    p_data_int = (int *)&data_buf[0];
    for(i = 0; i < col + row; i++) {
        data_buf_32[i] = *p_data_int;

        p_data_int++;
    }
    /* data verification */
    /* the testing data should be within the limit range  */
    for(i = 0; i < col + row; i++) {
        p_result_img[i] = data_buf_32[i] ;

        // determine the limit min
        if (size_limit_min == (col + row)) {
            limit_min_data = limit_min[i];
        }
        else {
            limit_min_data = limit_min[0];
        }
        // determine the limit max
        if (size_limit_max == col + row) {
            limit_max_data = limit_max[i];
        }
        else {
            limit_max_data = limit_max[0];
        }

        /* lower bound checking, value should not be lower than the test limit */
        if (p_result_img[i] < limit_min_data) {
            failure_cnt += 1;
            printf_e("%s error: fail at ch%2d, data = %5d, limit min = %5d\n",
                     __func__, i, p_result_img[i], limit_min_data);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at ch%2d, data = %5d, limit min = %5d\n",
                    i, p_result_img[i], limit_min_data);
            add_error_msg(err);
#endif
        }
        /* upper bound checking, value should not be higher than the test limit */
        if (p_result_img[i] > limit_max_data) {
            failure_cnt += 1;
            printf_e("%s error: fail at ch%2d, data = %5d, limit max = %5d\n",
                     __func__, i, p_result_img[i], limit_max_data);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at ch%2d, data = %5d, limit max = %5d\n",
                    i, p_result_img[i], limit_max_data);
            add_error_msg(err);
#endif
        }

    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_32)
        free(data_buf_32);

    return (retval < 0)? retval : failure_cnt;
}


/*
 * Function:  tcm_do_test_hy_abs_nose_pid1d
 * --------------------
 * perform the hybrid abs noise test PID 0x1D
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int tcm_do_test_hy_abs_nose_pid1d(int *p_result_img, int result_img_col, int result_img_row,
                                  int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    short *p_data_16;
    short *data_buf_16 = NULL;
    unsigned char *data_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int data_payload = 0;
    int i;
    int failure_cnt = 0;
    int limit_min_data;
    int limit_max_data;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_img) {
        printf_e("%s error: p_result_img is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_img is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((col != result_img_col) || (row != result_img_row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                 __func__, row, col, result_img_row, result_img_col);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (row,col) = (%d, %d), result = (%d, %d)\n",
                __func__, row, col, result_img_row, result_img_col);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((!limit_min) || (0 == size_limit_min) || (!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, size limit (min,max) = (%d, %d)\n",
                 __func__, size_limit_min, size_limit_max);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_HYBRID_ABS_NOISE);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perfrom production_test command TEST_HYBRID_ABS_NOISE\n",
                __func__);
        add_error_msg(err);
#endif
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: hybrid abs noise test pid1d (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    data_buf_16 = calloc((size_t)(col + row), sizeof(short));
    if (!data_buf_16) {
        printf_e("%s error: can't allocate memory for data_buf_16\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf_16\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* data copy with right orientation */
    p_data_16 = (short *)&data_buf[0];
    for(i = 0; i < (col + row); i++) {
        data_buf_16[i] = *p_data_16;

        p_data_16++;
    }
    /* data verification */
    /* the testing data should be within the limit range  */
    for(i = 0; i < (col + row); i++) {
        p_result_img[i] = data_buf_16[i] ;

        // determine the limit min
        if (size_limit_min == (col + row)) {
            limit_min_data = limit_min[i];
        }
        else {
            limit_min_data = limit_min[0];
        }
        // determine the limit max
        if (size_limit_max == col + row) {
            limit_max_data = limit_max[i];
        }
        else {
            limit_max_data = limit_max[0];
        }

        /* lower bound checking, value should not be lower than the test limit */
        if (data_buf_16[i] < limit_min_data) {
            failure_cnt += 1;
            printf_e("%s error: fail at ch%2d, data = %5d, limit min = %5d\n", __func__,
                     i, data_buf_16[i], limit_min_data);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at ch%2d, data = %5d, limit min = %5d\n",
                    i, data_buf_16[i], limit_min_data);
            add_error_msg(err);
#endif
        }
        /* upper bound checking, value should not be higher than the test limit */
        if (data_buf_16[i] > limit_max_data) {
            failure_cnt += 1;
            printf_e("%s error: fail at ch%2d, data = %5d, limit max = %5d\n", __func__,
                     i, data_buf_16[i], limit_max_data);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at ch%2d, data = %5d, limit max = %5d\n",
                    i, data_buf_16[i], limit_max_data);
            add_error_msg(err);
#endif
        }

    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(data_buf_16)
        free(data_buf_16);

    return (retval < 0)? retval : failure_cnt;
}


/*
 * Function:  tcm_do_test_ex_high_resistance_pid05
 * --------------------
 * perform the extended high resistance test
 * use discrete full raw test PID 0x05 for the calculation
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int tcm_do_test_ex_high_resistance_pid05(short *p_result_tixel, short *p_result_rx, short *p_result_tx,
                                         short *p_ref_frame, int col, int row,
                                         int limit_tixel, int limit_rxroe, int limit_txroe)
{
    int retval = 0;
    unsigned char *data_buf = NULL;
    int data_payload = 0;
    int i, j;
    int failure_cnt = 0;
    short *frame_delta = NULL;
    short *frame_baseline = NULL;
    short tx = convert_uc_to_short(g_tcm_handler.app_info_report.num_of_image_rows[0],
                        g_tcm_handler.app_info_report.num_of_image_rows[1]);
    short rx = convert_uc_to_short(g_tcm_handler.app_info_report.num_of_image_cols[0],
                        g_tcm_handler.app_info_report.num_of_image_cols[1]);
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_tixel) {
        printf_e("%s error: p_result_tixel is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_tixel is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if (!p_result_rx) {
        printf_e("%s error: p_result_rx is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_rx is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if (!p_result_tx) {
        printf_e("%s error:  p_result_tx is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_tx is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if (!p_ref_frame) {
        printf_e("%s error: p_ref_frame is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_ref_frame is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* send test command to request the testing data  */
    retval = tcm_send_test_command(TEST_FULLRAW);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to perform production_test command TEST_FULLRAW\n",
                __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* if succeed, save the data payload */
    data_payload = retval;
    printf_i("%s info: full raw test pid05 (row = %d, col = %d) (payload size = %d)\n",
             __func__, row, col, data_payload);

    /* create a local buffer to fill the testing data */
    data_buf = calloc((size_t)data_payload, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read payload */
    retval = tcm_get_payload(data_buf, data_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, data_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    frame_baseline = calloc((size_t)(row * col), sizeof(short));
    if (!frame_baseline) {
        printf_e("%s error: can't allocate memory for frame_baseline\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for frame_baseline\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    frame_delta = calloc((size_t)(row * col), sizeof(short));
    if (!frame_delta) {
        printf_e("%s error: can't allocate memory for frame_delta\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for frame_delta\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* data copy with right orientation */
    retval = tcm_reorder_test_data_frame(data_buf, (unsigned short *)frame_baseline);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to reorder the testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to reorder the testing data\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* calculate the delta frame */
    for (i = 0; i < (row * col); i++) {
        frame_delta[i] = frame_baseline[i] - p_ref_frame[i];
    }

    /* call library to run extended high resistance */
    extended_high_resistance_test((unsigned char)rx, (unsigned char)tx,
                                  frame_delta, frame_baseline,
                                  p_ref_frame, p_result_rx, p_result_tx, p_result_tixel);

    /* do data verification */
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {

            if(	p_result_tixel[i * row + j] <= limit_tixel) {

                failure_cnt += 1;
                printf_e("%s error: fail at (%d, %d) data = %5d, limit_tixel = %5d\n",
                         __func__, i, j, p_result_tixel[i*  row + j], limit_tixel);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit_tixel = %5d\n",
                        i, j, p_result_tixel[i * row + j], limit_tixel);
                add_error_msg(err);
#endif
            }
        }
    }

    for (i = 0; i < tx; i++){
        if(p_result_tx[i] >= limit_txroe){
            failure_cnt += 1;
            printf_e("%s error: fail at (tx  %d) data = %5d, limit_txroe = %5d\n",
                     __func__, i, p_result_tx[i], limit_txroe);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at (tx  %2d) data = %5d, limit_txroe = %5d\n",
                    i, p_result_tx[i], limit_txroe);
            add_error_msg(err);
#endif
        }
    }
    for (i = 0; i < rx; i++) {
        if (p_result_rx[i] >= limit_rxroe) {
            failure_cnt += 1;
            printf_e("%s error: fail at (rx  %d) data = %5d, limit_rxroe = %5d\n",
                     __func__, i, p_result_rx[i], limit_rxroe);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at (rx  %2d) data = %5d, limit_rxroe = %5d\n",
                    i, p_result_rx[i], limit_rxroe);
            add_error_msg(err);
#endif
        }
    }


    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    if(data_buf)
        free(data_buf);
    if(frame_baseline)
        free(frame_baseline);
    if(frame_delta)
        free(frame_delta);

    return (retval < 0)? retval : failure_cnt;
}



