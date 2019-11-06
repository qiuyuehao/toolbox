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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "native_syna_lib.h"

#ifndef _SYNA_DEV_MANAGER_H__
#define _SYNA_DEV_MANAGER_H__

#define SAVE_ERR_MSG

#define NUMBER_OF_SYNADEV_TO_SCAN (10)

/* define the value of test result */
enum TEST_RESULT {
    TEST_RESULT_NONE,
    TEST_RESULT_ERROROUT,
    TEST_RESULT_FAIL,
    TEST_RESULT_PASS,
};

/* define the finger status */
enum FINGER_STATE {
    FINGER_UP,
    FINGER_DOWN,
};

/* the path of synaptics character device */
char g_dev_node[128];
int g_dev_file_descriptor;

/* utilities */
#define GET_BIT(var,k) (((var) & (1<<(k))) >> k)

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

static short convert_uc_to_short(unsigned char lsb, unsigned char msb) {
    return (msb << 8) | lsb;
}

static bool check_str_starts_with(const char *pre, const char *str)
{
    size_t len_pre = strlen(pre);
    size_t len_str = strlen(str);
    return len_str < len_pre ? false : strncmp(pre, str, len_pre) == 0;
}

/* basic functions to open/close synaptics device */
bool syna_find_dev(char *dev_node);
bool syna_set_dev(const char *dev_node, bool is_rmi, bool is_tcm);
int syna_open_dev(const char *dev_node);
int syna_close_dev(const char *dev_node);

/* helper functions to perform the general control */
int syna_do_identify(char *p_out);
int syna_do_sw_reset(void);
int syna_do_preparation(bool nosleep_en, bool rezero_en);

/* helper functions to get the device information */
char* syna_get_device_id(void);
char* syna_get_config_id(void);
int syna_get_fw_id(void);
int syna_get_image_rows(bool out_in_landscape);
int syna_get_image_cols(bool out_in_landscape);
int syna_get_num_btns(void);
int syna_get_image_has_hybrid(void);
int syna_get_num_force_elecs(void);

/* helper functions to get the firmware configuration */
int syna_get_firmware_config_size(void);
int syna_get_firmware_config(unsigned char* buf, int size_buf);

/* helper functions for report image logging */
int syna_start_image_stream(unsigned char report_type, bool touch_en,
                            bool nosleep_en, bool rezero_en);
int syna_stop_image_stream(unsigned char report_type);
int syna_read_report_image_entry(unsigned char report_type, int *p_report_image, int size_of_image,
                                 int image_col, int image_row, bool out_in_landscape);

/* helper functions for production test */
int syna_run_test_entry(int test_id, int *p_result, int size_result, int result_col, int result_row,
                        int *p_param_1, int size_param_1, int *p_param_2, int size_param_2);
int syna_run_test_ex_high_resistance_entry(int *p_result, int size_result,
                                           int *p_txroe_result, int *p_size_txroe_result,
                                           int *p_rxroe_result, int *p_size_rxroe_result,
                                           short *ref_frame, int col, int row,
                                           int limit_surface, int limit_txroe, int limit_rxroe);
int syna_run_test_ex_trx_short_entry(int *p_result_data, int size_result_data,
                                     int *p_result_data_ex_pin, int size_result_data_ex_pin,
                                     int *p_result_pin, int size_result_pin,
                                     int *p_limit_1, int size_limit_1,
                                     int *p_limit_2, int size_limit_2);

/* helper functions to perform the raw command operation */
int syna_run_raw_command(unsigned char type, int cmd,
                         unsigned char* in, int size_in, unsigned char* resp, int size_resp);

/* helper functions to get the touch response */
int syna_query_touch_response_entry(int max_fingers_to_process);

/* helper functions to get the pins mapping */
int syna_get_pins_mapping(int rxes_offset, int rxes_len, int txes_offset, int txes_len);


#endif // _SYNA_DEV_MANAGER_H__

