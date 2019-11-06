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
#include <unistd.h>

#include "syna_dev_manager.h"
#include "rmi_control.h"

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
 * Function:  rmi_do_test_noise_rt2
 * --------------------
 * perform the noise test (RT02)
 *
 * return: <0 - fail to perform the testing
 *         =0 - pass the test
 *         >0 - failure count
 */
int rmi_do_test_noise_rt02(int *p_result_img, int result_img_col, int result_img_row,
                           int *limit_max, int size_limit_max, int num_frames_testing)
{
    int retval = 0;
    int i, j, idx_limit;
    int failure_frame_cnt = 0;
    int failure_cnt = 0;
    int *p_data_rt2 = NULL;
    int data;
    int *max_delta_frame = NULL;
    int *sum_buf = NULL;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
    int frame_size = sizeof(int) * col * row;
    int idx;
    int max_delta_sum = 0;
    int delta_sum;
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


    p_data_rt2 = malloc((size_t)frame_size);
    if (!p_data_rt2) {
        printf_e("%s error: can't allocate memory for p_data_rt2\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for p_data_rt2\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }
    memset(p_data_rt2, 0x00, (size_t)frame_size);

    max_delta_frame = malloc((size_t)frame_size);
    if (!max_delta_frame) {
        printf_e("%s error: can't allocate memory for max_delta_frame\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for max_delta_frame\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }
    memset(max_delta_frame, 0x00, (size_t)frame_size);

    sum_buf = malloc((size_t)frame_size);
    if (!sum_buf) {
        printf_e("%s error: can't allocate memory for sum_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for sum_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }
    memset(sum_buf, 0x00, (size_t)frame_size);

    /* to read the number of delta frames and validate every tixels */
    for (idx = 0; idx < num_frames_testing; idx++) {
        failure_cnt = 0;
        delta_sum = 0;

        /* read delta rt image in landscape format */
        retval = rmi_f54_read_report_frame(2, p_data_rt2, (col * row), true);
        if ( retval < 0) {
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read rt2 report at %d times\n", __func__, idx);
            add_error_msg(err);
#endif
            retval = -EIO;
            goto exit;
        }
        /* data copying and verification                                 */
        /* the format should be in landscape always                      */
        /* for verification. the value should be within the limit range  */
        idx_limit = 0;
        for(i = 0; i < col; i++) {
            for(j = 0; j < row; j++) {

                sum_buf[i * row + j] += p_data_rt2[i * row + j];
                delta_sum += abs(p_data_rt2[i * row + j]);

                data = p_data_rt2[i * row + j];

                /* data verification, value should not be higher than the test limit */
                if (size_limit_max == col * row) {
                    /* if it is a entire limit frame, check every tixels */
                    if (abs(data) > limit_max[idx_limit]) {
                        failure_cnt += 1;
                        printf_e("%s error: fail at frame %2d (%2d, %2d), data = %5d, limit max = %5d\n",
                                 __func__, idx, i, j, data, limit_max[idx_limit]);
#ifdef SAVE_ERR_MSG
                        sprintf(err, "fail at frame %2d (%-2d, %-2d), data = %5d, limit max = %5d\n",
                                idx, i, j, data, limit_max[idx_limit]);
                        add_error_msg(err);
#endif
                    }
                }
                else {
                    /* if it is a single vale, compare with it only */
                    if (abs(data) > limit_max[0]) {
                        failure_cnt += 1;
                        printf_e("%s error: fail at frame %2d (%2d, %2d) data = %5d, limit max = %5d\n",
                                 __func__, idx, i, j, data, limit_max[0]);
#ifdef SAVE_ERR_MSG
                        sprintf(err, "fail at frame %2d (%2d, %-d), data = %5d, limit max = %5d\n",
                                idx, i, j, data, limit_max[0]);
                        add_error_msg(err);
#endif
                    }
                }

                idx_limit += 1;
            }
        }

        if (delta_sum > max_delta_sum) {
            memcpy(max_delta_frame, p_data_rt2, (size_t)frame_size);
            max_delta_sum = delta_sum;
        }
        if (failure_cnt > 0) {
            failure_frame_cnt += 1;
        }
    } /* end for loop */

    /* copy the final data to the result_img */
    if (failure_frame_cnt > 0) {
        for(i = 0; i < result_img_col; i++) {
            for (j = 0; j < result_img_row; j++) {
                p_result_img[i * result_img_row + j] = max_delta_frame[i * result_img_row + j];
            }
        }
    }
    else {
        for(i = 0; i < result_img_col; i++) {
            for (j = 0; j < result_img_row; j++) {
                p_result_img[i * result_img_row + j] =
                        (sum_buf[i * result_img_row + j] / num_frames_testing);
            }
        }
    }

    printf_i("%s info: %s (failure_frame_cnt = %d)\n",
             __func__, (failure_frame_cnt == 0)?"pass":"fail", failure_frame_cnt);
exit:
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if(p_data_rt2)
        free(p_data_rt2);
    if(max_delta_frame)
        free(max_delta_frame);
    if(sum_buf)
        free(sum_buf);

    return (retval < 0)? retval : failure_frame_cnt;
}

/*
 * Function:  rmi_do_test_full_raw_cap_rt20
 * --------------------
 * perform the full raw capacitance test (RT20)
 *
 * return: <0 - fail to perform the testing
 *         =0 - pass the test
 *         >0 - failure count
 */
int rmi_do_test_full_raw_cap_rt20(int *p_result_img, int result_img_col, int result_img_row,
                                  int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int *p_data_rt20 = NULL;
    int data;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
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


    p_data_rt20 = calloc((size_t)(col * row), sizeof(int));
    if (!p_data_rt20) {
        printf_e("%s error: can't allocate memory for p_data_rt20\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for p_data_rt20\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* do on-preparation before reading the rt20 */
    retval = rmi_disable_cbc_cdm();
    if (retval < 0) {
        printf_e("%s error: fail to do preparation before running the rt20\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to do preparation before running the rt20\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read report rt20 in landscape format */
    retval = rmi_f54_read_report_frame(20, p_data_rt20, (col * row), true);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read report rt20\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            p_result_img[i * row + j] = p_data_rt20[i * row + j];
            data = p_result_img[i * row + j];

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
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if(p_data_rt20)
        free(p_data_rt20);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_do_test_full_raw_tddi_rt92
 * --------------------
 * perform the full raw test for tddi (RT92)
 *
 * return: <0 - fail to perform the testing
 *         =0 - pass the test
 *         >0 - failure count
 */
int rmi_do_test_full_raw_tddi_rt92(int *p_result_img, int result_img_col, int result_img_row,
                                  int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int *p_data_rt92 = NULL;
    int data;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
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


    p_data_rt92 = calloc((size_t)(col * row), sizeof(int));
    if (!p_data_rt92) {
        printf_e("%s error: can't allocate memory for p_data_rt92\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for p_data_rt92\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read report rt92 frame in landscape format */
    retval = rmi_f54_read_report_frame(92, p_data_rt92, (col * row), true);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read report rt92\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* data copying and verification                                 */
    /* the format should be in landscape always                      */
    /* for verification. the value should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            p_result_img[i * row + j] = p_data_rt92[i * row + j];
            data = p_data_rt92[i * row + j];

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
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if(p_data_rt92)
        free(p_data_rt92);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_do_test_ex_high_resistance_rt20
 * --------------------
 * perform the extended high resistance test
 * use discrete full raw test RT20 for the calculation
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_ex_high_resistance_rt20(short *p_result_tixel, short *p_result_rx, short *p_result_tx,
                                        short *p_ref_frame, int col, int row,
                                        int limit_tixel, int limit_rxroe, int limit_txroe)
{
    int retval = 0;
    int i, j;
    int failure_cnt = 0;
    int *p_data_rt20 = NULL;
    short *frame_delta = NULL;
    short *frame_baseline = NULL;
    unsigned char tx = g_rmi_pdt.tx_assigned;
    unsigned char rx = g_rmi_pdt.rx_assigned;
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
    if ((0 == col) || (0 == row)) {
        printf_e("%s error: invalid parameter (row,col) = (%d, %d)\n",
                 __func__, col, row);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter (row,col) = (%d, %d)\n",
                __func__, col, row);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }


    frame_delta = calloc((size_t)(col * row), sizeof(short));
    if (!frame_delta) {
        printf_e("%s error: fail to allocate memory for frame_delta\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for frame_delta\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    frame_baseline = calloc((size_t)(col * row), sizeof(short));
    if (!frame_baseline) {
        printf_e("%s error: fail to allocate memory for frame_baseline\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for frame_baseline\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    p_data_rt20 = calloc((size_t)(col * row), sizeof(int));
    if (!p_data_rt20) {
        printf_e("%s error: can't allocate memory for p_data_rt20\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for p_data_rt20\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }


    /* do on-preparation before reading the rt20 */
    retval = rmi_disable_cbc_cdm();
    if (retval < 0) {
        printf_e("%s error: fail to do preparation before running the rt20\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to do preparation before running the rt20\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read report rt20 into frame_baseline */
    retval = rmi_f54_read_report_frame(20, p_data_rt20, (col * row), true);
    if ( retval < 0) {
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read report rt20\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* create the baseline frame */
    for (i = 0; i < (col * row); i++) {
        frame_baseline[i] = (short)p_data_rt20[i];
    }

    /* calculate the delta frame */
    for (i = 0; i < (col * row); i++) {
        frame_delta[i] = frame_baseline[i] - p_ref_frame[i];
    }

    /* call library to run extended high resistance */
    extended_high_resistance_test(rx, tx, frame_delta, frame_baseline, p_ref_frame,
                                  p_result_rx, p_result_tx, p_result_tixel);

    /* do data verification */
    for(i = 0; i < tx; i++) {
        for(j = 0; j < rx; j++) {

            if(	p_result_tixel[i * rx + j] <= limit_tixel) {

                failure_cnt += 1;
                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit_tixel = %5d\n",
                         __func__, i, j, p_result_tixel[i*  rx + j], limit_tixel);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit_tixel = %5d\n",
                        i, j, p_result_tixel[i * rx + j], limit_tixel);
                add_error_msg(err);
#endif
            }
        }
    }
    for (i = 0; i < tx; i++){
        if(p_result_tx[i] >= limit_txroe){
            failure_cnt += 1;
            printf_e("%s error: fail at (tx %2d) data = %5d, limit_txroe = %5d\n",
                     __func__, i, p_result_tx[i], limit_txroe);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at (tx %2d) data = %5d, limit_txroe = %5d\n",
                    i, p_result_tx[i], limit_txroe);
            add_error_msg(err);
#endif
        }
    }
    for (j = 0; j <rx; j++) {
        if (p_result_rx[j] >= limit_rxroe) {
            failure_cnt += 1;
            printf_e("%s error: fail at (rx %2d) data = %5d, limit_rxroe = %5d\n",
                     __func__, i, p_result_rx[i], limit_rxroe);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at (rx %2d) data = %5d, limit_rxroe = %5d\n",
                    i, p_result_rx[i], limit_rxroe);
            add_error_msg(err);
#endif
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if(p_data_rt20)
        free(p_data_rt20);
    if(frame_baseline)
        free(frame_baseline);
    if(frame_delta)
        free(frame_delta);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_pin_is_rx_assigned
 * --------------------
 * check whether the assigned pin is rx or not
 *
 * return: true, rx assignment
 *         otherwise, not rx assignment
 */
bool rmi_pin_is_rx_assigned(int pin)
{
    int i;
    for (i = 0; i < g_rmi_pdt.rx_assigned; i++) {
        if (pin == g_rmi_pdt.rx_assignment[i]){
            return true;
        }
    }
    return false;
}
/*
 * Function:  rmi_pin_is_tx_assigned
 * --------------------
 * check whether the assigned pin is tx or not
 *
 * return: true, tx assignment
 *         otherwise, not tx assignment
 */
bool rmi_pin_is_tx_assigned(int pin)
{
    int i;
    for (i = 0; i < g_rmi_pdt.tx_assigned; i++) {
        if (pin == g_rmi_pdt.tx_assignment[i]){
            return true;
        }
    }

    return false;
}
/*
 * Function:  rmi_do_test_trx_short_rt26
 * --------------------
 * perform the TRX short test (RT26)
 * use input size_of_result_data to define the retrieved data size for testing
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_trx_short_rt26(int *p_result_data, int size_of_result_data, int *pins_result,
                               int size_of_pins_result, int *limit, int size_of_limit)
{
    int retval;
    unsigned char p_rt26_data[TRX_OPEN_SHORT_DATA_SIZE] = {0};
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
    if (size_of_result_data != size_of_limit) {
        printf_e("%s error: size is mismatching\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: size is mismatching\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if (size_of_result_data > TRX_OPEN_SHORT_DATA_SIZE)
        size_of_result_data = TRX_OPEN_SHORT_DATA_SIZE;

    printf_i("%s: size of result is %d-byte\n", __func__, size_of_result_data);

    if (size_of_result_data*8 > size_of_pins_result) {
        printf_e("%s error: size_of_pins_result is mismatching (size_of_pins_result = %d)\n",
                 __func__, size_of_pins_result);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: size_of_pins_result is mismatching (size_of_pins_result = %d)\n",
                __func__, size_of_pins_result);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* read rt26 data */
    retval = rmi_f54_read_report_ucarray(26, p_rt26_data, size_of_result_data);
    if ( retval < 0) {
        printf_e("%s error: fail to read report rt26\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read report rt26\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* rt26 data verification */
    /* go through each bit to check its value */
    /*  bit = 0 means passing the pin test; otherwise, fail if bit = 1  */
    for (i = 0; i < size_of_result_data; i++) {

        p_result_data[i] = p_rt26_data[i];
        printf_i("%s info: rt26 data - byte %2d, value = 0x%02x\n",
                 __func__, i, p_result_data[i]);

        for (j = 0; j < 8; j ++) {
            phy_pin = (i*8 + j);

            /* perform test only when target pin is assigned */
            do_pin_test =
                    rmi_pin_is_rx_assigned(phy_pin) || rmi_pin_is_tx_assigned(phy_pin);
            if (do_pin_test) {

                /* update the pin result  */
                pins_result[phy_pin] = GET_BIT(p_rt26_data[i], j);

                if (GET_BIT(limit[i], j) == 1) {
                    printf_i("%s info: pin-%2d - ignore. because limit = 0x%02x\n",
                             __func__, phy_pin, limit[i]);
                    continue;
                }

                // if it is extended pin, pin-0, pin-1, pin-32, pin-33
                if (( 0 == phy_pin) || ( 1 == phy_pin) || (32 == phy_pin)|| (33 == phy_pin))
                {
                    if (pins_result[phy_pin] == 1) {
                        if (rmi_pin_is_rx_assigned(phy_pin)) {
                            printf_e("%s error: pin-%2d - fail, need rt100 testing.\n",
                                     __func__, phy_pin);
                        }
                        else {
                            printf_i("%s error: pin-%2d - fail\n", __func__, phy_pin);
#ifdef SAVE_ERR_MSG
                            sprintf(err, "fail at pin-%2d, rt26 data = 0x%x, limit = 0x%x\n",
                                    i, p_rt26_data[i], limit[i]);
                            add_error_msg(err);
#endif
                            failure_cnt += 1;
                        }
                    }
                }
                // other pins
                else {
                    if (pins_result[phy_pin] == 1) {
                        printf_i("%s error: pin-%2d - fail\n", __func__, phy_pin);
#ifdef SAVE_ERR_MSG
                        sprintf(err, "fail at pin-%2d, rt26 data = 0x%x, limit = 0x%x\n",
                                i, p_rt26_data[i], limit[i]);
                        add_error_msg(err);
#endif
                        failure_cnt += 1;
                    }
                }
            }
            else {
                pins_result[phy_pin] = -1; /* no assigned */
            }
        } /* end inner for loop */
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_do_test_ex_trx_short_rt26100
 * --------------------
 * perform the extended TRX short test (RT26 + RT100)
 * will call rmi_do_test_trx_short_rt26 to perform the rt26 testing
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_ex_trx_short_rt26100(int *p_result_data_26, int size_of_result_data_26,
                                     int *p_result_data_100, int size_of_result_data_100,
                                     int *pins_result, int size_of_pins_result,
                                     int *limit_rt26, int size_of_limit_rt26,
                                     int *limit_rt100, int size_of_limit_rt100)
{
    int retval = 0;
    int pin, i, j, rx_idx;
    unsigned char extended_pin[4] = {0, 1, 32, 33};
    int failure_cnt_100 = 0;
    int failure_cnt_26 = 0;
    int tx = g_rmi_pdt.tx_assigned;
    int rx = g_rmi_pdt.rx_assigned;
    bool do_pin_rt100_test = false;
    int *p_rt100_img1 = NULL;
    int *p_rt100_img2 = NULL;
    int *p_rt100_delta = NULL;
    unsigned char logical_pin;
    unsigned char data = 0;
    unsigned char temp_data[TRX_OPEN_SHORT_DATA_SIZE*8] = {0};
    int *max_val = NULL;
    int *min_val = NULL;
    int limit_target;
    int limit_others;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_result_data_26) {
        printf_e("%s error: p_result_data_26 is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_data_26 is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if (!p_result_data_100) {
        printf_e("%s error: p_result_data_100 is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: p_result_data_100 is NULL.\n", __func__);
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

    if (size_of_result_data_26 > TRX_OPEN_SHORT_DATA_SIZE)
        size_of_result_data_26 = TRX_OPEN_SHORT_DATA_SIZE;

    printf_i("%s: size of rt26 is %d-byte\n", __func__, size_of_result_data_26);

    if (size_of_result_data_26*8 > size_of_pins_result) {
        printf_e("%s error: size_of_result_data_26 is mismatching (size_of_pins_result = %d)\n",
                 __func__, size_of_pins_result);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: size_of_result_data_26 is mismatching (size_of_pins_result = %d)\n",
                __func__, size_of_pins_result);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if (size_of_result_data_100 != 4*rx) {
        printf_e("%s error: size_of_result_data_100 is mismatching (size = %d)\n",
                 __func__, size_of_result_data_100);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: size_of_result_data_100 is mismatching (size = %d)\n",
                __func__, size_of_result_data_100);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if ((!limit_rt26) || (size_of_limit_rt26 != size_of_result_data_26)) {
        printf_e("%s error: limit_rt26 is invalid (size = %d)\n", __func__, size_of_limit_rt26);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: limit_rt26 is invalid (size = %d)\n", __func__, size_of_limit_rt26);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if ((!limit_rt100) || (size_of_limit_rt100 != 2)) {
        printf_e("%s error: limit_rt100 is invalid (size = %d)\n", __func__, size_of_limit_rt100);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: limit_rt26 is invalid (size = %d)\n", __func__, size_of_limit_rt100);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* call function to perform rt26 testing */
    failure_cnt_26 = rmi_do_test_trx_short_rt26(p_result_data_26, size_of_result_data_26,
                                                pins_result, size_of_pins_result,
                                                limit_rt26, size_of_limit_rt26);
    /* check whether the rt100 test is needed or not */
    for (i = 0; i < (sizeof(extended_pin)/sizeof(unsigned char)); i++) {
        do_pin_rt100_test |= (pins_result[extended_pin[i]] == 1);
    }


    /* set no_scan bit */
    retval = rmi_read_reg(g_rmi_pdt.F54.control_base_addr, &data, 1);
    if (retval < 0) {
        printf_e("%s error: fail to read f54 control base register\n",
                 __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to read f54 control base register\n");
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    data = (uint8_t)( data | 0x02 );
    retval = rmi_write_reg(g_rmi_pdt.F54.control_base_addr, &data, 1);
    if (retval < 0) {
        printf_e("%s error: fail to set no_scan\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to set no_scan\n");
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* set all local CBC rxs to 0 */
    retval = rmi_write_reg(g_rmi_pdt.f54_control.reg_96.address,
                           &temp_data[0], g_rmi_pdt.rx_assigned);
    if (retval < 0) {
        printf_e("%s error: fail to configure local cbc to 0\n",
                 __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to configure local cbc to 0\n");
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    /* force update */
    retval = rmi_f54_force_update();
    if ( retval < 0 ) {
        printf_e("%s error: fail to do force update\n",
                 __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to do force update\n");
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }


    p_rt100_img1 = calloc((size_t)(tx * rx), sizeof(int));
    if (!p_rt100_img1) {
        printf_e("%s error: fail to allocate memory for p_rt100_img1\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to allocate memory for p_rt100_img1\n");
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    p_rt100_img2 = calloc((size_t)(tx * rx), sizeof(int));
    if (!p_rt100_img2) {
        printf_e("%s error: fail to allocate memory for p_rt100_img2\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to allocate memory for p_rt100_img2\n");
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    p_rt100_delta = calloc((size_t)(tx * rx), sizeof(int));
    if (!p_rt100_delta) {
        printf_e("%s error: fail to allocate memory for p_rt100_delta\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to allocate memory for p_rt100_delta\n");
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    max_val = calloc((size_t)rx, sizeof(int));
    if (!max_val) {
        printf_e("%s error: fail to allocate memory for max_val\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to allocate memory for max_val\n");
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    min_val = calloc((size_t)rx, sizeof(int));
    if (!min_val) {
        printf_e("%s error: fail to allocate memory for min_val\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to allocate memory for min_val\n");
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read report type 100 image, take this frame be the baseline */
    retval = rmi_f54_read_report_frame(100, p_rt100_img1, (tx * rx), false);
    if ( retval < 0) {
        printf_e("%s error: fail to read report rt100 image1\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "error: fail to read report rt100 image1\n");
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    limit_target = limit_rt100[0];
    limit_others = limit_rt100[1];

    failure_cnt_100 = 0;

    /* for each extended pin,                    */
    /* adjust cbc and read the other rt100 image */
    for (pin = 0; pin < (sizeof(extended_pin)/sizeof(unsigned char)); pin++) {

        for (i = 0; i < rx; i++) {
            max_val[i] = 0;
            min_val[i] = 5000;
        }

        logical_pin = rmi_f54_get_logical_rx(extended_pin[pin]);
        if ( logical_pin == 0xff ) {
            pins_result[extended_pin[pin]] = -1; /* no assigned */
            continue;
        }

        printf_i("%s info: pin-%d, logical pin %d \n",
                 __func__, extended_pin[pin], logical_pin);

        /* set local CBC to 8pf(2D) 3.5pf(0D) */
        temp_data[logical_pin] = 0x0f;
        retval = rmi_write_reg(g_rmi_pdt.f54_control.reg_96.address,
                               &temp_data[0], g_rmi_pdt.rx_assigned);
        if (retval < 0) {
            printf_e("%s error: fail to adjust local cbc for logical_pin = %d\n",
                     __func__, logical_pin);
#ifdef SAVE_ERR_MSG
            sprintf(err, "error: fail to adjust local cbc for logical_pin = %d\n",
                    logical_pin);
            add_error_msg(err);
#endif
            retval = -EINVAL;
            goto exit;
        }
        memset(temp_data, 0x00, sizeof(temp_data));

        /* force update */
        retval = rmi_f54_force_update();
        if ( retval < 0 ) {
            printf_e("%s error: fail to do force update on logical_pin = %d\n",
                     __func__, logical_pin);
#ifdef SAVE_ERR_MSG
            sprintf(err, "error: fail to do force update on logical_pin = %d\n",
                    logical_pin);
            add_error_msg(err);
#endif
            retval = -EINVAL;
            goto exit;
        }

        /* read the other rt100 image */
        retval = rmi_f54_read_report_frame(100, p_rt100_img2, (tx * rx), false);
        if ( retval < 0) {
            printf_e("%s error: fail to read report rt100 image2\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "error: fail to read report rt100 image2\n");
            add_error_msg(err);
#endif
            retval = -EIO;
            goto exit;
        }

        /* calculate the delta image from image1 and image2 */
        for ( i = 0; i < tx; i++ ) {
            for ( j = 0; j < rx; j++ ) {
                p_rt100_delta[i * rx + j] =
                        abs( p_rt100_img1[i * rx + j] - p_rt100_img2[i * rx + j] );

                if ( max_val[j] < p_rt100_delta[i * rx + j] )
                    max_val[j] = p_rt100_delta[i * rx + j];

                if ( min_val[j] > p_rt100_delta[i * rx + j] )
                    min_val[j] = p_rt100_delta[i * rx + j];
            }
        }

        /* data verification */
        /* testing failure if TRx w/o CBC raised changes >= 200, or */
        /* TRx w/ CBC raised changes < 2000, as well as any other RX changes are >=200 */
        for ( rx_idx = 0; rx_idx < rx; rx_idx++ )
        {
            /* the current pin is the target test pin,                */
            /* data w/ CBC raised changes should be higher than limit_1 */
            if ( rx_idx == logical_pin )
            {
                p_result_data_100[pin*rx + rx_idx] = min_val[rx_idx];

                if ( min_val[rx_idx] < limit_target )
                {
                    printf_e("%s error: fail at extended pin %2d (target logical pin-%2d) data = %5d, limit min = %5d\n",
                             __func__, extended_pin[pin], rx_idx, min_val[rx_idx], limit_target);
#ifdef SAVE_ERR_MSG
                    sprintf(err, "fail at extended pin %2d (target logical pin-%2d) data = %5d, limit min = %5d\n",
                            extended_pin[pin], rx_idx, min_val[rx_idx], limit_target);
                    add_error_msg(err);
#endif
                    failure_cnt_100 += 1;
                }
            }
            /* other pins,                                             */
            /* data w/o CBC raised changes should be less than limit_2 */
            else
            {
                p_result_data_100[pin*rx + rx_idx] = max_val[rx_idx];

                if ( max_val[rx_idx] >= limit_others )
                {
                    printf_e("%s error: fail at extended pin %2d (other logical pin-%2d) data = %5d, limit max = %5d\n",
                             __func__, extended_pin[pin], rx_idx, max_val[rx_idx], limit_others);
#ifdef SAVE_ERR_MSG
                    sprintf(err, "fail at extended pin %2d (other logical pin-%2d) data = %5d, limit max = %5d\n",
                            extended_pin[pin], rx_idx, max_val[rx_idx], limit_others);
                    add_error_msg(err);
#endif
                    failure_cnt_100 += 1;
                }
            }
        }/* end for (rx_idx = 0; rx_idx < g_rmi_pdt.rx_assigned; rx_idx++) */

        if (failure_cnt_100 == 0) {
            pins_result[extended_pin[pin]] = 0;  /* pass */
        }
        else {
            pins_result[extended_pin[pin]] = 1;  /* fail */
        }

    } /* end for extended pin for loop */

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt_26 + failure_cnt_100 == 0)?"pass":"fail", failure_cnt_26 + failure_cnt_100);

exit:
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if(p_rt100_img1)
        free(p_rt100_img1);
    if(p_rt100_img2)
        free(p_rt100_img2);
    if(p_rt100_delta)
        free(p_rt100_delta);
    if(max_val)
        free(max_val);
    if(min_val)
        free(min_val);

    return (retval < 0)? retval : (failure_cnt_100 + failure_cnt_26);
}

/*
 * Function:  rmi_do_test_abs_open_rt63
 * --------------------
 * perform the abs sensing open test (RT63)
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_abs_open_rt63(int *p_result_img, int size_of_result_data,
                              int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    int *p_data_32;
    unsigned char *p_rt63_data = NULL;
    int i, j;
    int failure_cnt = 0;
    unsigned char tx = g_rmi_pdt.tx_assigned;
    unsigned char rx = g_rmi_pdt.rx_assigned;
    int data_size = (tx + rx) * 4;
    int TEST_SAMPLE_COUNT = 10;
    int *p_test_buffer = NULL;
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
    if (size_of_result_data != rx + tx) {
        printf_e("%s error: size of result is mismatching, (rx, tx) = (%d, %d)\n",
                 __func__, rx, tx);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: size of result is mismatching, (rx, tx) = (%d, %d)\n",
                __func__, rx, tx);
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

    p_rt63_data = calloc((size_t)data_size, sizeof(unsigned char));
    if (!p_rt63_data) {
        printf_e("%s error: fail to allocate memory for p_rt63_data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for p_rt63_data\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    p_test_buffer = calloc((size_t)(tx + rx), sizeof(int));
    if (!p_test_buffer) {
        printf_e("%s error: fail to allocate memory for p_test_buffer\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for p_test_buffer\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* repeatedly read report 63 images, and then calculate the average */
    for (i = 0; i < TEST_SAMPLE_COUNT; i++) {
        /* read report rt63 */
        retval = rmi_f54_read_report_ucarray(63, p_rt63_data, data_size);
        if ( retval < 0) {
            printf_e("%s error: fail to read report rt63\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read report rt63\n", __func__);
            add_error_msg(err);
#endif
            retval = -EIO;
            goto exit;
        }

        p_data_32 = (int *)&p_rt63_data[0];
        for (j = 0; j < (tx + rx); j++) {
            p_test_buffer[j] += (*p_data_32) >> 2;
            p_data_32++;
        }
    } // end for (i < TEST_SAMPLE_COUNT)

    /* calculate the average */
    for (i = 0; i < (tx + rx); i++) {
        p_test_buffer[i] = p_test_buffer[i]/TEST_SAMPLE_COUNT;
    }

    /* data verification */
    /* the testing data should be within the limit range */
    for (i = 0; i < (tx + rx); i++) {

        p_result_img[i] = p_test_buffer[i];

        // determine teh limit_min
        if (size_limit_min == (tx + rx)) {
            limit_min_data = limit_min[i];
        }
        else {
            limit_min_data = limit_min[0];
        }
        // determine teh limit_max
        if (size_limit_max == (tx + rx)) {
            limit_max_data = limit_max[i];
        }
        else {
            limit_max_data = limit_max[0];
        }

        /* lower bound checking, value should not be lower than the test limit */
        if (p_test_buffer[i] < limit_min_data) {
            failure_cnt += 1;
            printf_e("%s error: fail at ch%2d data = %5d, limit min = %5d\n", __func__,
                     i, p_test_buffer[i], limit_min_data);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at ch%2d data = %5d, limit min = %5d\n",
                    i, p_test_buffer[i], limit_min_data);
            add_error_msg(err);
#endif
        }
        /* upper bound checking, value should not be higher than the test limit */
        if (p_test_buffer[i] > limit_max_data) {
            failure_cnt += 1;
            printf_e("%s error: fail at ch%2d data = %5d, limit max = %5d\n", __func__,
                     i, p_test_buffer[i], limit_max_data);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at ch%2d data = %5d, limit max = %5d\n",
                    i, p_test_buffer[i], limit_max_data);
            add_error_msg(err);
#endif
        }

    } // end data verification

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if(p_rt63_data)
        free(p_rt63_data);
    if(p_test_buffer)
        free(p_test_buffer);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_do_test_adc_range_rt23
 * --------------------
 * perform the adc range test (RT23)
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_adc_range_rt23(int *p_result_img, int result_img_col, int result_img_row,
                               int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    int i, j, offset, idx_limit;
    unsigned char *p_rt23_data = NULL;
    int failure_cnt = 0;
    unsigned char tx = g_rmi_pdt.tx_assigned;
    unsigned char rx = g_rmi_pdt.rx_assigned;
    int data_size;
    short *p_data_16;
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
    if (((tx != result_img_col) && (tx != result_img_row)) ||
            ((rx != result_img_col) && (rx != result_img_row))) {
        printf_e("%s error: invalid parameter (tx,rx) = (%d, %d)\n", __func__, tx, rx);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: invalid parameter (tx,rx) = (%d, %d)\n", __func__, tx, rx);
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

    if (g_rmi_pdt.f54_query.has_signal_clarity) {
        retval = rmi_read_reg(g_rmi_pdt.f54_control.reg_41.address,
                              g_rmi_pdt.f54_control.reg_41.data,
                              sizeof(g_rmi_pdt.f54_control.reg_41.data));
        if ( retval < 0) {
            printf_e("%s error: fail to read f54_ctrl41\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read f54_ctrl41\n", __func__);
            add_error_msg(err);
#endif
            retval = -EIO;
            goto exit;
        }
        if (!g_rmi_pdt.f54_control.reg_41.no_signal_clarity) {
            if (tx % 4)
                tx += 4 - (tx % 4);
        }
    }
    data_size = 2 * tx * rx;
    printf_i("%s info: data_size = %d (tx,rx) = (%d,%d)\n", __func__, data_size, tx , rx);

    p_rt23_data = calloc((size_t)data_size, sizeof(unsigned char));
    if (!p_rt23_data) {
        printf_e("%s error: fail to allocate memory for p_rt23_data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for p_rt23_data\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read report rt23, read byte array because the size is not rx*tx only */
    retval = rmi_f54_read_report_ucarray(23, p_rt23_data, data_size);
    if ( retval < 0) {
        printf_e("%s error: fail to read report rt23\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read report rt23\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    printf_i("%s info: (col,row) = (%d,%d)\n", __func__, result_img_col, result_img_row);
    /* put data into the result buffer in landscape layout */
    p_data_16 = (short *)&p_rt23_data[0];
    for(i = 0; i < g_rmi_pdt.tx_assigned; i++) {
        for(j = 0; j < g_rmi_pdt.rx_assigned; j++) {

            if (g_rmi_pdt.rx_assigned > g_rmi_pdt.tx_assigned)
                offset = i*g_rmi_pdt.rx_assigned + j;
            else
                offset = j*g_rmi_pdt.tx_assigned + i;

            p_result_img[offset] = *p_data_16;

            // printf_i(" (%d, %d) %4d\n", i, j, *p_data_16);

            p_data_16++;
        }
    }

    /* data verification */
    /* the testing data should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < result_img_col; i++) {
        for (j = 0; j < result_img_row; j++) {

            // determine teh limit_min
            if (size_limit_min == (result_img_col* result_img_row)) {
                limit_min_data = limit_min[idx_limit];
            }
            else {
                limit_min_data = limit_min[0];
            }
            // determine teh limit_max
            if (size_limit_max == (result_img_col* result_img_row)) {
                limit_max_data = limit_max[idx_limit];
            }
            else {
                limit_max_data = limit_max[0];
            }

            /* lower bound checking, value should not be lower than the test limit */
            if (p_result_img[i * result_img_row + j] < limit_min_data) {
                failure_cnt += 1;

                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit min = %5d\n", __func__,
                         i, j, p_result_img[i * result_img_row + j], limit_min_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit min = %5d\n",
                        i, j, p_result_img[i * result_img_row + j], limit_min_data);
                add_error_msg(err);
#endif
            }
            /* upper bound checking, value should not be higher than the test limit */
            if (p_result_img[i * result_img_row + j] > limit_max_data) {
                failure_cnt += 1;

                printf_e("%s error: fail at (%2d, %2d) data = %5d, limit max = %5d\n", __func__,
                         i, j, p_result_img[i * result_img_row + j], limit_max_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at (%2d, %2d) data = %5d, limit max = %5d\n",
                        i, j, p_result_img[i * result_img_row + j], limit_max_data);
                add_error_msg(err);
#endif
            }

            idx_limit += 1;
        }
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if (p_rt23_data)
        free(p_rt23_data);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_do_test_sensor_speed_rt22
 * --------------------
 * perform the sensor speed test (RT22)
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_sensor_speed_rt22(int *p_result_img, int result_img_col, int result_img_row,
                                  int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int *p_data_rt22 = NULL;
    int data;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
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

    p_data_rt22 =  calloc((size_t)(col * row), sizeof(int));
    if (!p_data_rt22) {
        printf_e("%s error: can't allocate memory for p_data_rt22\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for p_data_rt22\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read report rt22 */
    retval = rmi_f54_read_report_frame(22, p_data_rt22, (col * row), true);
    if ( retval < 0) {
        printf_e("%s error: fail to read report rt22\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read report rt22\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* data verification */
    /* the testing data should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            p_result_img[i * row + j] = p_data_rt22[i * row + j];
            data = p_result_img[i * row + j];

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

                printf_e("%s: fail at (%2d, %2d) data = %5d, limit min = %5d\n", __func__,
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

                printf_e("%s: fail at (%2d, %2d) data = %5d, limit max = %5d\n", __func__,
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
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if (p_data_rt22)
        free(p_data_rt22);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_do_test_tagsmoisture_rt76
 * --------------------
 * perform the tags moisture test (RT76)
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_tagsmoisture_rt76(int *p_result_img, int result_img_col, int result_img_row,
                                  int *limit_max, int size_limit_max)
{
    int retval = 0;
    int i, j, idx_limit;
    int failure_cnt = 0;
    int *p_data_rt76 = NULL;
    int data;
    int col = syna_get_image_cols(true);
    int row = syna_get_image_rows(true);
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
    if ((!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, size limit max = %d\n", __func__, size_limit_max);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }


    p_data_rt76 = calloc((size_t)(col * row), sizeof(int));
    if (!p_data_rt76) {
        printf_e("%s error: fail to allocate memory for p_data_rt76\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for p_data_rt76\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read report rt76 */
    retval = rmi_f54_read_report_frame(76, p_data_rt76, (col * row), true);
    if ( retval < 0) {
        printf_e("%s error: fail to read report rt76\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read report rt76\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* data verification */
    /* the testing data should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < col; i++) {
        for(j = 0; j < row; j++) {
            p_result_img[i * row + j] = p_data_rt76[i * row + j];
            data = p_result_img[i * row + j];

            // determine the limit max
            if (size_limit_max == (col* row)) {
                limit_max_data = limit_max[idx_limit];
            }
            else {
                limit_max_data = limit_max[0];
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
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if (p_data_rt76)
        free(p_data_rt76);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_do_test_rt133
 * --------------------
 * perform the rt133 test
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_rt133(int *p_result_img, int size_of_result_data,
                      int *limit_max, int size_limit_max)
{
    int retval = 0;
    int i, j, idx_limit;
    unsigned char rx = g_rmi_pdt.rx_assigned;
    int failure_cnt = 0;
    unsigned char *p_rt133_data = NULL;
    int limit_max_data;
    int data_size;
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
    if (rx != size_of_result_data) {
        printf_e("%s error: result size is mismatching\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: result size is mismatching\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    if ((!limit_max) || (0 == size_limit_max)) {
        printf_e("%s error: invalid parameter, size limit max = %d\n", __func__, size_limit_max);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, no test limit input\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    data_size = 2 * rx;

    p_rt133_data = calloc((size_t)data_size, sizeof(unsigned char));
    if (!p_rt133_data) {
        printf_e("%s error: fail to allocate memory for p_rt133_data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for p_rt133_data\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    retval = rmi_f54_read_report_ucarray(133, p_rt133_data, data_size);
    if ( retval < 0) {
        printf_e("%s error: fail to read report rt133\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read report rt133\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* put valid data to the buffer */
    j = 0;
    for(i = 0; i < rx; i++) {
        p_result_img[j] = ( short )p_rt133_data[j] | ( ( short )p_rt133_data[j + 1] << 8 );
        j += 2;
    }

    /* data verification */
    /* the testing data should be within the limit range  */
    idx_limit = 0;
    for(i = 0; i < rx; i++) {

        // determine the limit max
        if (size_limit_max == rx) {
            limit_max_data = limit_max[idx_limit];
        }
        else {
            limit_max_data = limit_max[0];
        }

        /* upper bound checking, value should not be higher than the test limit */
        if (p_result_img[i] > limit_max_data) {
            failure_cnt += 1;

            printf_e("%s error: fail at ch%2d data = %5d, limit max = %5d\n", __func__,
                     i, p_result_img[i], limit_max_data);
#ifdef SAVE_ERR_MSG
            sprintf(err, "fail at ch%2d data = %5d, limit max = %5d\n",
                    i, p_result_img[i], limit_max_data);
            add_error_msg(err);
#endif
        }

        idx_limit += 1;
    }

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if (p_rt133_data)
        free(p_rt133_data);

    return (retval < 0)? retval : failure_cnt;
}

/*
 * Function:  rmi_do_test_abs_delta_rt59
 * --------------------
 * perform the absolute delta test (RT59)
 *
 * return: <0, fail to perform the testing
 *         =0, pass the test
 *         >0, failure count
 */
int rmi_do_test_abs_delta_rt59(int *p_result_img, int size_of_result_data,
                               int *limit_min, int size_limit_min, int *limit_max, int size_limit_max)
{
    int retval = 0;
    int *p_data_32;
    unsigned char *p_rt59_data = NULL;
    int frame_idx, i;
    int failure_cnt = 0;
    int pre_failure_cnt = 0;
    unsigned char tx = g_rmi_pdt.tx_assigned;
    unsigned char rx = g_rmi_pdt.rx_assigned;
    int data_size = (tx + rx) * 4;
    int TEST_SAMPLE_COUNT = 100;
    int *p_test_buffer = NULL;
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
    if (size_of_result_data != rx + tx) {
        printf_e("%s error: size of result is mismatching, (rx, tx) = (%d, %d)\n",
                 __func__, rx, tx);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: size of result is mismatching, (rx, tx) = (%d, %d)\n",
                __func__, rx, tx);
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

    p_rt59_data = calloc((size_t)data_size, sizeof(unsigned char));
    if (!p_rt59_data) {
        printf_e("%s error: fail to allocate memory for p_rt59_data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for p_rt59_data\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    p_test_buffer = calloc((size_t)(tx + rx), sizeof(int));
    if (!p_test_buffer) {
        printf_e("%s error: can't allocate memory for p_test_buffer\n", __func__);
        sprintf(err, "%s: can't allocate memory for p_test_buffer\n", __func__);
        add_error_msg(err);
        retval = -ENOMEM;
        goto exit;
    }

    /* repeatedly read report 63 images, and then calculate the average */
    for (frame_idx = 0; frame_idx < TEST_SAMPLE_COUNT; frame_idx++) {

        memset(p_rt59_data, 0x00 , (size_t)data_size);

        /* read report rt59 */
        retval = rmi_f54_read_report_ucarray(59, p_rt59_data, data_size);
        if ( retval < 0) {
            printf_e("%s error: fail to read report rt59\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read report rt59\n", __func__);
            add_error_msg(err);
#endif
            retval = -EIO;
            goto exit;
        }

        p_data_32 = (int *)&p_rt59_data[0];
        for (i = 0; i < (tx + rx); i++) {
            // printf_i(" (rx %d) data = %4d\n", j, *p_data_32);
            p_test_buffer[i] = (*p_data_32);

            p_data_32++;
        }

        /* data verification */
        /* the testing data should be within the limit range */
        for (i = 0; i < (tx + rx); i++) {

            // determine teh limit_min
            if (size_limit_min == (tx + rx)) {
                limit_min_data = limit_min[i];
            }
            else {
                limit_min_data = limit_min[0];
            }
            // determine teh limit_max
            if (size_limit_max == (tx + rx)) {
                limit_max_data = limit_max[i];
            }
            else {
                limit_max_data = limit_max[0];
            }

            /* lower bound checking, value should not be lower than the test limit */
            if (p_test_buffer[i] < limit_min_data) {
                failure_cnt += 1;

                printf_e("%s error: fail at ch%2d (frame %2d) data = %5d, limit min = %5d\n", __func__,
                         i, frame_idx, p_test_buffer[i], limit_min_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at ch%2d (frame %2d) data = %5d, limit min = %5d\n",
                        i, frame_idx, p_test_buffer[i], limit_min_data);
                add_error_msg(err);
#endif
            }
            /* upper bound checking, value should not be higher than the test limit */
            if (p_test_buffer[i] > limit_max_data) {
                failure_cnt += 1;

                printf_e("%s error: fail at ch%2d (frame %2d) data = %5d, limit max = %5d\n", __func__,
                         i, frame_idx, p_test_buffer[i], limit_max_data);
#ifdef SAVE_ERR_MSG
                sprintf(err, "fail at ch%2d (frame %2d) data = %5d, limit max = %5d\n",
                        i, frame_idx, p_test_buffer[i], limit_max_data);
                add_error_msg(err);
#endif
            }
        } // end for (i < rx + tx)

        if (failure_cnt != 0) {
            if (failure_cnt != pre_failure_cnt) {
                for (i = 0; i < (tx + rx); i++) {
                    p_result_img[i] = p_test_buffer[i];
                }

                pre_failure_cnt = failure_cnt;
            }
        }
        else {
            for (i = 0; i < (tx + rx); i++) {
                p_result_img[i] =
                        ((p_result_img[i] > p_test_buffer[i])?p_result_img[i] : p_test_buffer[i]);
            }
        }

    } // end for (frame_idx < TEST_SAMPLE_COUNT)

    printf_i("%s info: %s (fail_cnt = %d)\n",
             __func__, (failure_cnt == 0)?"pass":"fail", failure_cnt);
exit:
    /* issue a sw reset at the end */
    syna_do_sw_reset();

    if(p_rt59_data)
        free(p_rt59_data);
    if(p_test_buffer)
        free(p_test_buffer);

    return (retval < 0)? retval : failure_cnt;
}

