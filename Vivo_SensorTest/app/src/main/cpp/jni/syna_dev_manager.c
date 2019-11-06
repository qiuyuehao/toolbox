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
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "syna_dev_manager.h"
#include "rmi_control.h"
#include "tcm_control.h"

#ifdef SAVE_ERR_MSG
#include "err_msg_ctrl.h"
#endif

#define RAW_CMD_WRITE  0x11
#define RAW_CMD_READ   0x12

#define MAX_FINGER   10

/* enumerate the supported syna interface */
enum SYNA_DEV {
    SYNA_DEV_NONE,
    SYNA_RMI_DEV,
    SYNA_TCM_DEV,
};

/* global variables to indicate the device being used */
static enum SYNA_DEV g_syna_dev = SYNA_DEV_NONE;

/* enumerate the supported production test items */
/* must be equivalent to the same id in java layer */
enum TEST_ID {
    // RMI testing items
    TEST_RMI_NOISE_RT02 = 0x200,
    TEST_RMI_NOISE_TDDI_RT94 = 0x201,
    TEST_RMI_FULL_RAW_RT20 = 0x202,
    TEST_RMI_FULL_RAW_TDDI_RT92 = 0x203,
    TEST_RMI_EX_HIGH_RESISTANCE_RT20 = 0x204,
    TEST_RMI_TRX_SHORT_RT26 = 0x205,
    TEST_RMI_EX_TRX_SHORT_RT26100 = 0x206,
    TEST_RMI_ABS_OPEN_RT63 = 0x207,
    TEST_RMI_ADC_RANGE_RT23 = 0x208,
    TEST_RMI_TAGSMOISTURE_RT76 = 0x209,
    TEST_RMI_RT133 = 0x20A,
    TEST_RMI_ABS_DELTA_RT59 = 0x20B,
    TEST_RMI_SENSOR_SPEED_RT22 = 0x20C,


    // TCM testing items
    TEST_TCM_NOISE_PID0A = 0x300,
    TEST_TCM_DRT_PID07 = 0x301,
    TEST_TCM_PT11_PID0B = 0x302,
    TEST_TCM_PT12_PID0C = 0x303,
    TEST_TCM_PT13_PID0D = 0x304,
    TEST_TCM_FULL_RAW_PID05 = 0x305,
    TEST_TCM_TRX_TRX_SHORT_PID01 = 0x306,
    TEST_TCM_TRX_GROUND_PID03 = 0x307,
    TEST_TCM_ADC_RANGE_PID11 = 0x308,
    TEST_TCM_ABS_RAWCAP_PID12 = 0x309,
    TEST_TCM_HYBRID_ABS_NOISE_PID1D = 0x30A,
    TEST_TCM_EX_HIGH_RESISTANCE_PID05 = 0x30B,
};

/* global variables as a string of config id */
static char g_str_config_id[MAX_STRING_LEN];
static bool g_report_img_stream_en;
static int g_finger_status[MAX_FINGER];

/*
 * Function:  syna_find_dev
 * --------------------
 * search the synaptics device being installed
 * record the type of device in g_syna_dev as well
 *
 * return: true, device node is available
 *         false, otherwise
 */
bool syna_find_dev(char *dev_node)
{
    if (rmi_find_dev(dev_node)) {
        g_syna_dev = SYNA_RMI_DEV;
        return true;
    }
    else if (tcm_find_dev(dev_node)) {
        g_syna_dev = SYNA_TCM_DEV;
        return true;
    }

    return false;
}

/*
 * Function:  syna_set_dev
 * --------------------
 * setup the specified syna device node
 *
 * return: true, the input node is available
 *         false, otherwise
 */
bool syna_set_dev(const char *dev_node, bool is_rmi, bool is_tcm)
{
    int retval;
    struct stat st;
    bool is_found;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!dev_node) {
        printf_e("%s error: invalid input node\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid input string\n", __func__);
        add_error_msg(err);
#endif
        return false;
    }

    if (is_rmi && is_tcm) {
        printf_e("%s error: invalid input dev type, (is_rmi = %d, (is_tcm = %d)\n",
                 __func__, is_rmi, is_tcm);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid input dev type, (is_rmi = %d, (is_tcm = %d)\n",
                 __func__, is_rmi, is_tcm);
        add_error_msg(err);
#endif
        return false;
    }

    memset(g_dev_node, 0x00, sizeof(g_dev_node));
    snprintf(g_dev_node, sizeof(g_dev_node), "%s", dev_node);

    printf_i("%s info: input device node, %s\n", __func__, g_dev_node);

    retval = stat(g_dev_node, &st);
    is_found = (0 == retval);

    if (is_found && is_rmi) {
        g_syna_dev = SYNA_RMI_DEV;
        printf_i("%s synaptics rmi device node = %s\n", __func__, g_dev_node);
    }
    else if (is_found && is_tcm) {
        g_syna_dev = SYNA_TCM_DEV;
        printf_i("%s synaptics tcm device node = %s\n", __func__, g_dev_node);
    }
    else {
        g_syna_dev = SYNA_DEV_NONE;
        printf_e("%s error: fail to find input device node, %s (retval = %d) (err: %s)\n",
                 __func__, g_dev_node, retval, strerror(errno));
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to find input device node, %s (retval = %d) (err: %s)\n",
                __func__, g_dev_node, retval, strerror(errno));
        add_error_msg(err);
#endif
    }

    return is_found;
}
/*
 * Function:  syna_open_dev
 * --------------------
 * open the synaptics device node
 *
 * return: 0  , file descriptor is already existed
 *         <0 , fail to open device
 *         otherwise, succeed
 */
int syna_open_dev(const char *dev_node)
{
    int retval;
    int i;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];

    clear_all_error_msg();
#endif

    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = rmi_open_dev(dev_node);
            break;
        case SYNA_TCM_DEV:
            retval = tcm_open_dev(dev_node);
            break;
        default:
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: device node is not found.\n", __func__);
            add_error_msg(err);
#endif
            retval = -EINVAL;
            break;
    }

    /* initialize the global parameters */
    for (i = 0; i < MAX_FINGER; i++)
        g_finger_status[i] = 0x00;

    g_report_img_stream_en = false;

    return retval;
}
/*
 * Function:  syna_close_dev
 * --------------------
 * close the synaptics device node
 *
 * return: file descriptor
 */
int syna_close_dev(const char *dev_node)
{
    int retval;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif


    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = rmi_close_dev(dev_node);
            break;
        case SYNA_TCM_DEV:
            retval = tcm_close_dev(dev_node);
            break;
        default:
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: device node is found.\n", __func__);
            add_error_msg(err);
#endif
            retval = -EINVAL;
            break;
    }

    return retval;
}
/*
 * Function:  syna_get_device_id
 * --------------------
 * inquiry the device id
 *
 * return: string of device id
 */
char* syna_get_device_id()
{
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            return (char *)g_rmi_pdt.asic_type;
        case SYNA_TCM_DEV:
            return (char *)g_tcm_handler.identify_report.part_number;
        default:
            printf_e("%s error: unknown device\n", __func__);
            return NULL;
    }
}
/*
 * Function:  syna_get_fw_id
 * --------------------
 * inquiry the firmware id
 *
 * return: firmware id
 */
int syna_get_fw_id()
{
    int fw_id = 0;

    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            fw_id = g_rmi_pdt.build_id;
            break;
        case SYNA_TCM_DEV:
            fw_id = ((unsigned int)g_tcm_handler.identify_report.build_id[0]) +
                    ((unsigned int)g_tcm_handler.identify_report.build_id[1] * 0x100) +
                    ((unsigned int)g_tcm_handler.identify_report.build_id[2] * 0x10000) +
                    ((unsigned int)g_tcm_handler.identify_report.build_id[3] * 0x1000000);
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }

    return fw_id;
}
/*
 * Function:  syna_get_config_id
 * --------------------
 * inquiry the firmware config id
 *
 * return: string of config id
 */
char* syna_get_config_id()
{
    int i;
    memset(g_str_config_id, 0x00, sizeof(g_str_config_id));

    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            for (i = 0; i < g_rmi_pdt.size_of_config_id - 1; i++) {
                sprintf(g_str_config_id + strlen(g_str_config_id),
                        "%02X-", g_rmi_pdt.config_id[i]);
            }
            sprintf(g_str_config_id + strlen(g_str_config_id),
                    "%02X", g_rmi_pdt.config_id[g_rmi_pdt.size_of_config_id - 1]);
            break;
        case SYNA_TCM_DEV:
            for (i = 0; i < 15; i++) {
                sprintf(g_str_config_id + strlen(g_str_config_id),
                        "%02X-", g_tcm_handler.app_info_report.customer_config_id[i]);
            }
            sprintf(g_str_config_id + strlen(g_str_config_id),
                    "%02X", g_tcm_handler.app_info_report.customer_config_id[15]);
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            return NULL;
    }

    return (char *)g_str_config_id;
}
/*
 * Function:  syna_get_image_rows
 * --------------------
 * inquiry the number of rows in the image
 *
 * set out_in_landscape to true to indicate the landscape layout,
 * which always returns the larger value
 *
 * return: the number of row
 */
int syna_get_image_rows(bool out_in_landscape)
{
    int rows = 0;
    int cols = 0;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            if (out_in_landscape)
                rows = MAX(g_rmi_pdt.tx_assigned, g_rmi_pdt.rx_assigned);
            else
                rows = g_rmi_pdt.tx_assigned;
            break;

        case SYNA_TCM_DEV:
            rows = (g_tcm_handler.app_info_report.num_of_image_rows[0] |
                    g_tcm_handler.app_info_report.num_of_image_rows[1] << 8);
            cols = (g_tcm_handler.app_info_report.num_of_image_cols[0] |
                    g_tcm_handler.app_info_report.num_of_image_cols[1] << 8);

            if (out_in_landscape)
                rows = MAX(rows, cols);

            break;

        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return rows;
}
/*
 * Function:  syna_get_image_cols
 * --------------------
 * inquiry the number of columns in the image
 *
 * set out_in_landscape to true to indicate the landscape layout,
 * which always returns the smaller value
 *
 * return: the number of column
 */
int syna_get_image_cols(bool out_in_landscape)
{
    int rows = 0;
    int cols = 0;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            if (out_in_landscape)
                cols = MIN(g_rmi_pdt.tx_assigned, g_rmi_pdt.rx_assigned);
            else
                cols = g_rmi_pdt.rx_assigned;
            break;

        case SYNA_TCM_DEV:
            rows = (g_tcm_handler.app_info_report.num_of_image_rows[0] |
                    g_tcm_handler.app_info_report.num_of_image_rows[1] << 8);
            cols = (g_tcm_handler.app_info_report.num_of_image_cols[0] |
                    g_tcm_handler.app_info_report.num_of_image_cols[1] << 8);

            if (out_in_landscape)
                cols = MIN(rows, cols);

            break;

        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return cols;
}
/*
 * Function:  syna_get_num_btns
 * --------------------
 * inquiry the number of 0d buttons supported
 *
 * return: the number of 0d buttons
 */
int syna_get_num_btns()
{
    int btns = 0;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            if (g_rmi_pdt.has_button)
                btns = 1;
            break;
        case SYNA_TCM_DEV:
            btns = (g_tcm_handler.app_info_report.num_of_buttons[0] |
                    g_tcm_handler.app_info_report.num_of_buttons[1] << 8);
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return btns;
}
/*
 * Function:  syna_get_image_has_hybrid
 * --------------------
 * check whether the image containing hybrid data
 *
 * return: 1 : enabled / 0 : disabled
 */
int syna_get_image_has_hybrid()
{
    int retval = 0;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = 0;
            break;
        case SYNA_TCM_DEV:
            retval = convert_uc_to_short(g_tcm_handler.app_info_report.has_hybrid_data[0],
                                         g_tcm_handler.app_info_report.has_hybrid_data[1]);
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return retval;
}
/*
 * Function:  syna_get_num_force_elecs
 * --------------------
 * inquiry the number of force electrodes assigned
 *
 * return: the number of force electrodes
 */
int syna_get_num_force_elecs(void)
{
    int retval = 0;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = 0;
            break;
        case SYNA_TCM_DEV:
            retval = convert_uc_to_short(g_tcm_handler.app_info_report.num_of_force_elecs[0],
                                         g_tcm_handler.app_info_report.num_of_force_elecs[1]);
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return retval;
}
/*
 * Function:  syna_do_sw_reset
 * --------------------
 * perform a sw reset
 *
 * return: 0<, fail to issue sw reset
 *         otherwise, succeed
 */
int syna_do_sw_reset()
{
    int retval = -EINVAL;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = rmi_f01_sw_reset();
            break;
        case SYNA_TCM_DEV:
            retval = tcm_do_reset();
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return retval;
}
/*
 * Function:  syna_set_no_sleep
 * --------------------
 * configure the device into no_sleep mode
 *
 * return: 0<, fail to set no sleep
 *         otherwise, succeed
 */
int syna_set_no_sleep()
{
    int retval = -EINVAL;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = rmi_f01_set_no_sleep();
            break;
        case SYNA_TCM_DEV:
            retval = tcm_set_no_sleep(1);
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return retval;
}
/*
 * Function:  set_syna_no_relax
 * --------------------
 * configure the device into no_relax mode
 *
 * return: 0<, fail to set no relax
 *         otherwise, succeed
 */
int set_syna_no_relax()
{
    int retval = -EINVAL;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = rmi_f54_no_relax(true);
            break;
        case SYNA_TCM_DEV:
            retval = 0;  // do nothing
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return retval;
}

/*
 * Function:  syna_set_rezero
 * --------------------
 * rezero the baseline
 *
 * return: 0<, fail to issue sw reset
 *         otherwise, succeed
 */
int syna_set_rezero()
{
    int retval = -EINVAL;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = rmi_f54_force_cal();
            break;
        case SYNA_TCM_DEV:
            retval = tcm_set_rezero();
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return retval;
}

/*
 * Function:  syna_do_identify
 * --------------------
 * identify the device
 *
 * return:  <0, fail to perform device identification
 *         otherwise, succeed
 */
int syna_do_identify(char *p_out)
{
    int retval = -EINVAL;
    int string_offset = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_out) {
        printf_e("%s error: p_out is empty \n", __func__);
        return -EINVAL;
    }

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        return -EINVAL;
    }

    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = rmi_get_identify(p_out);
            if (retval < 0) {
                printf_e("%s error: fail to get the identification info\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to get the identification info\n", __func__);
                add_error_msg(err);
#endif
                goto exit;
            }
            break;
        case SYNA_TCM_DEV:
            retval = tcm_get_identify_info(p_out);
            if (retval < 0) {
                printf_e("%s error: fail to request the identify report\n", __func__);
                goto exit;
            }
            string_offset = (int) strlen(p_out);
            retval = tcm_get_app_info(p_out + string_offset);
            if (retval < 0) {
                printf_e("%s error: fail to request the identify report\n", __func__);
                goto exit;
            }

            retval = tcm_get_static_config();
            if (retval < 0) {
                printf_e("%s error: fail to get static config\n", __func__);
                goto exit;
            }
        default:
            break;
    }
exit:
    return retval;
}


/*
 * Function:  syna_do_preparation
 * --------------------
 * perform the following preparation if needed
 *    - no sleep
 *    - rezero
 *
 * return: <0, fail to perform preparation
 *         otherwise, succeed
 */
int syna_do_preparation(bool nosleep_en, bool rezero_en)
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* configure the device into no_sleep mode */
    if (nosleep_en) {
        retval = syna_set_no_sleep();
        if (retval < 0) {
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to set to no_sleep mode\n",
                    __func__);
            add_error_msg(err);
#endif
            return retval;
        }
    }

    /* do rezero */
    if (rezero_en) {
        retval = syna_set_rezero();
        if (retval < 0) {
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to perform rezero\n",
                    __func__);
            add_error_msg(err);
#endif
            return retval;
        }
    }

    return retval;
}

/*
 * Function:  syna_start_image_stream
 * --------------------
 * enable the report image streaming
 *
 * return: <0, fail to start the report streaming
 *         otherwise, succeed
 */
int syna_start_image_stream(unsigned char report_type, bool touch_en,
                            bool nosleep_en, bool rezero_en)
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        printf_e("%s error: unknown device\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    /* enable no_sleep oe do re-zero before image reading */
    retval = syna_do_preparation(nosleep_en, rezero_en);
    if (retval < 0) {
        printf_e("%s error: fail to do preparation \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to do preparation\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    switch (g_syna_dev)
    {
        case SYNA_RMI_DEV:
            g_report_img_stream_en = true;
            break;

        case SYNA_TCM_DEV:

            if (touch_en == false) {

                /* disable the touch report */
                retval = tcm_enable_report(false, TCM_REPORT_TOUCH);
                if (retval < 0) {
                    printf_e("%s error: fail to disable the tcm touch report\n", __func__);
#ifdef SAVE_ERR_MSG
                    sprintf(err, "%s error: fail to disable the tcm report.\n", __func__);
                    add_error_msg(err);
#endif
                    goto exit;
                }
                printf_i("%s info: disable the tcm touch report (report-0x%x)\n", __func__,
                         TCM_REPORT_TOUCH);

            }

            /* enable the requested report */
            retval = tcm_enable_report(true, (enum tcm_report_code) report_type);
            if (retval < 0) {
                printf_e("%s error: fail to enable the tcm report\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to enable the tcm report.\n", __func__);
                add_error_msg(err);
#endif
                goto exit;
            }
            printf_i("%s info: enable the tcm report (report-0x%x)\n", __func__,
                     (enum tcm_report_code) report_type);

            g_report_img_stream_en = true;

            break;

        default:
            break;
    }

exit:
    return retval;
}


/*
 * Function:  syna_stop_image_stream
 * --------------------
 * disable the report image streaming
 *
 * return: <0, fail to stop the report streaming
 *         otherwise, succeed
 */
int syna_stop_image_stream(unsigned char report_type)
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        printf_e("%s error: unknown device\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            g_report_img_stream_en = false;
            break;

        case SYNA_TCM_DEV:

            g_report_img_stream_en = false;

            /* disable the requested report */
            retval = tcm_enable_report(false, (enum tcm_report_code) report_type);
            if (retval < 0) {
                printf_e("%s error: fail to disable the tcm report\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s: fail to disable the tcm report.\n", __func__);
                add_error_msg(err);
#endif
                goto exit;
            }
            printf_i("%s info: disable the tcm report (report-0x%x)\n", __func__,
                     (enum tcm_report_code) report_type);


            /* enable the touch report always */
            retval = tcm_enable_report(true, TCM_REPORT_TOUCH);
            if (retval < 0) {
                printf_e("%s error: fail to enable the tcm touch report\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s: fail to enable the tcm report.\n", __func__);
                add_error_msg(err);
#endif
                goto exit;
            }
            printf_i("%s info: enable the tcm touch report (report-0x%x)\n", __func__,
                     TCM_REPORT_TOUCH);
            break;

        default:
            break;
    }

exit:
    return retval;
}

/*
 * Function:  syna_read_report_image_entry
 * --------------------
 * retrieve the report data from syna device
 * this function should be called after syna_start_image_stream
 *
 * return: <0, fail to retrieve a report
 *         otherwise, succeed
 */
int syna_read_report_image_entry(unsigned char report_type, int *p_report_image, int size_of_image,
                            int image_col, int image_row, bool out_in_landscape)
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        printf_e("%s error: unknown device\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    if (!g_report_img_stream_en) {
        printf_e("%s error: report image stream is not enabled\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: report image stream is not enabled\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    if ((!p_report_image) || (size_of_image <= 0)) {
        printf_e("%s error: invalid parameter\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            /* get one requested report frame */
            retval = rmi_f54_read_report_frame(report_type, p_report_image,
                                               (image_col * image_row), out_in_landscape);
            if (retval < 0) {
                printf_e("%s error: fail to read the rmi report\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to read the rmi report\n", __func__);
                add_error_msg(err);
#endif
                goto exit;
            }
            break;
        case SYNA_TCM_DEV:
            /* get one requested report frame */
            retval = tcm_read_report_frame((int)report_type, p_report_image,
                                           (image_col * image_row), out_in_landscape);
            if (retval < 0) {
                printf_e("%s error: fail to read the tcm report\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to read the tcm report\n", __func__);
                add_error_msg(err);
#endif
                goto exit;
            }
        default:
            break;
    }

exit:
    return retval;
}


/*
 * Function:  syna_run_rmi_test_entry
 * --------------------
 * perform the requested production testing
 *
 * return: =0, pass
 *         otherwise, fail
 */
int syna_run_rmi_test_entry(int test_id, int *p_result, int size_result, int result_col, int result_row,
                            int *p_param_1, int size_param_1, int *p_param_2, int size_param_2)
{
    int retval;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    switch (test_id) {
        case TEST_RMI_NOISE_RT02:

            /* call function to perform noise test RT2 */
            /* param_1 = no used       */
            /* param_2 = limit maximum */
            retval = rmi_do_test_noise_rt02(p_result, result_col, result_row,
                                            p_param_2, size_param_2, 20);
            if (retval < 0) {
                printf_e("%s error: fail to run noise rt2 test\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run noise rt2 test\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_RMI_FULL_RAW_RT20:

            /* call function to perform full raw test RT20 */
            /* param_1 = limit minimum */
            /* param_2 = limit maximum */
            retval = rmi_do_test_full_raw_cap_rt20(p_result, result_col, result_row,
                                                   p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run full raw cap rt20 test\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run full raw cap rt20 test\n", __func__);
                add_error_msg(err);
#endif
            }

            syna_do_sw_reset(); // do reset always
            break;


        case TEST_RMI_FULL_RAW_TDDI_RT92:

            /* call function to perform tddi full raw test RT92 */
            /* param_1 = limit minimum */
            /* param_2 = limit maximum */
            retval = rmi_do_test_full_raw_tddi_rt92(p_result, result_col, result_row,
                                                    p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run full raw tddi rt92 test\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run full raw tddi rt92 test\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_RMI_ABS_OPEN_RT63:

            /* call function to perform abs open test RT63 */
            /* limit_1 = limit minimum */
            /* limit_2 = limit maximum */
            retval = rmi_do_test_abs_open_rt63(p_result, size_result,
                                               p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run abs sensing open rt63 test\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run abs sensing open rt63 test\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_RMI_ADC_RANGE_RT23:

            /* call function to perform adc range test RT23 */
            /* limit_1 = limit minimum */
            /* limit_2 = limit maximum */
            retval = rmi_do_test_adc_range_rt23(p_result, result_col, result_row,
                                                p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run adc range rt23 test\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run adc range rt23 test\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_RMI_SENSOR_SPEED_RT22:
            /* call function to perform sensor speed test RT22 */
            /* limit_1 = limit minimum */
            /* limit_2 = limit maximum */
            retval = rmi_do_test_sensor_speed_rt22(p_result, result_col, result_row,
                                                   p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run sensor speed rt22 test\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run sensor speed rt22 test\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_RMI_TAGSMOISTURE_RT76:

            /* call function to perform tag moisture test RT76 */
            /* param_1 = no used       */
            /* param_2 = limit maximum */
            retval = rmi_do_test_tagsmoisture_rt76(p_result, result_col, result_row,
                                                   p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run tag moisture rt76 test\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run tag moisture rt76 test\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_RMI_RT133:

            /* call function to perform rt133 test RT133 */
            /* param_1 = no used       */
            /* param_2 = limit maximum */
            retval = rmi_do_test_rt133(p_result, size_result,
                                       p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run rt133 test\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run rt133 test\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_RMI_ABS_DELTA_RT59:
            /* call function to perform abs delta test RT59 */
            /* limit_1 = limit minimum */
            /* limit_2 = limit maximum */
            retval = rmi_do_test_abs_delta_rt59(p_result, size_result,
                                                p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run abs sensing open rt59 test\n", __func__)
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run abs sensing open rt59 test\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_RMI_TRX_SHORT_RT26:

            /* call function to perform trx short test (RT26) */
            /* param_1 = store the pin's result */
            /* param_2 = limit */
            retval = rmi_do_test_trx_short_rt26(p_result, size_result, p_param_1, size_param_1,
                                                p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run trx short test rt26\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to trx short test rt26\n", __func__);
                add_error_msg(err);
#endif
            }

            syna_do_sw_reset();

            break;

        default:
            printf_e("%s error: unsupported test item (%x)\n", __func__, test_id);
            retval = -EINVAL;
            break;
    }

    return retval;
}

/*
 * Function:  syna_run_tcm_test_entry
 * --------------------
 * run the requested production testing
 *
 * return: =0, pass
 *         otherwise, fail
 */
int syna_run_tcm_test_entry(int test_id, int *p_result, int result_col, int result_row,
                       int *p_param_1, int size_param_1, int *p_param_2, int size_param_2)
{
    int retval;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    switch (test_id) {

        case TEST_TCM_NOISE_PID0A:

            /* call function to perform noise test (PID 0x0A) */
            /* param_1 = no used       */
            /* param_2 = limit maximum */

            retval = tcm_do_test_noise_pid0a(p_result, result_col, result_row,
                                             p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run noise test pid0a\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run noise test pid0a\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }
            break;

        case TEST_TCM_DRT_PID07:

            /* call function to perform dynamic range test (PID 0x07) */
            /* param_1 = limit minimum */
            /* param_2 = limit maximum */
            retval = tcm_do_test_drt_pid07(p_result, result_col, result_row,
                                           p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run dynamic range test pid07\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run dynamic range test pid07\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }

            break;

        case TEST_TCM_FULL_RAW_PID05:

            /* call function to perform discrete full raw test (PID 0x05) */
            /* param_1 = limit minimum */
            /* param_2 = limit maximum */
            retval = tcm_do_test_full_raw_pid05(p_result, result_col, result_row,
                                                p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run discrete full raw test pid05\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run discrete full raw test pid05\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }

            break;

        case TEST_TCM_TRX_TRX_SHORT_PID01:

            /* call function to perform trx trx short test (PID 0x01) */
            /* param_1 = store the pin's result */
            /* param_2 = limit */
            retval = tcm_do_test_trx_trx_short_pid01(p_result, p_param_1, size_param_1,
                                                     p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run trx trx short test pid01\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run trx trx short test pid01\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }

            break;


        case TEST_TCM_TRX_GROUND_PID03:

            /* call function to perform trx ground test (PID 0x03) */
            /* param_1 = store the pin's result */
            /* param_2 = limit */
            retval = tcm_do_test_trx_ground_pid03(p_result, p_param_1, size_param_1,
                                                  p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run trx ground test pid03\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run trx ground test pid03\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }

            break;

        case TEST_TCM_ADC_RANGE_PID11:

            /* call function to perform adc range test (PID 0x11) */
            /* param_1 = limit minimum */
            /* param_2 = limit maximum */
            retval = tcm_do_test_adc_range_pid11(p_result, result_col, result_row,
                                                 p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run adc range test pid11\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run adc range test pid11\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }

            break;

        case TEST_TCM_ABS_RAWCAP_PID12:

            /* call function to perform abs raw cap test (PID 0x12) */
            /* param_1 = limit minimum */
            /* param_2 = limit maximum */
            retval = tcm_do_test_abs_rawcap_pid12(p_result, result_col, result_row,
                                                  p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run abs raw cap test pid12\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run abs raw cap test pid12\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }

            break;

        case TEST_TCM_HYBRID_ABS_NOISE_PID1D:

            /* call function to perform hybrid abs noise test (PID 0x1D) */
            /* param_1 = limit minimum */
            /* param_2 = limit maximum */
            retval = tcm_do_test_hy_abs_nose_pid1d(p_result, result_col, result_row,
                                                   p_param_1, size_param_1, p_param_2, size_param_2);
            if (retval < 0) {
                printf_e("%s error: fail to run hybrid abs noise test pid1d\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: fail to run hybrid abs noise test pid1d\n", __func__);
                add_error_msg(err);
#endif
                syna_do_sw_reset(); // reset if error occurs
            }

            break;


        default:
            printf_e("%s error: unknown test item (0x%x)\n", __func__, test_id);
            retval = -EINVAL;
            break;
    }

    return retval;
}
/*
 * Function:  syna_run_test_entry
 * --------------------
 * the entry function to run the requested production testing
 *
 * return: =0, pass
 *         otherwise, fail
 */
int syna_run_test_entry(int test_id, int *p_result, int size_result, int result_col, int result_row,
                        int *p_param_1, int size_param_1, int *p_param_2, int size_param_2)
{
    int retval = -1;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        printf_e("%s error: unknown device\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    if (SYNA_RMI_DEV == g_syna_dev) {
        retval = syna_run_rmi_test_entry(test_id, p_result, size_result, result_col, result_row,
                                         p_param_1, size_param_1, p_param_2, size_param_2);
    }
    else if (SYNA_TCM_DEV == g_syna_dev) {
        retval = syna_run_tcm_test_entry(test_id, p_result, result_col, result_row,
                                         p_param_1, size_param_1, p_param_2, size_param_2);
    }

    return retval;
}

/*
 * Function:  syna_run_test_ex_high_resistance_entry
 * --------------------
 * run the extended high resistance test
 * this function is separated from syna_run_test_entry
 * because the required parameters are different
 *
 * return: =0, pass
 *         otherwise, fail
 */
int syna_run_test_ex_high_resistance_entry(int *p_result, int size_result,
                                           int *p_txroe_result, int *p_size_txroe_result,
                                           int *p_rxroe_result, int *p_size_rxroe_result,
                                           short *ref_frame, int col, int row,
                                           int limit_surface, int limit_txroe, int limit_rxroe)
{
    int retval;
    short *testing_data_frame = NULL;
    int testing_data_size = 0;
    short *testing_tx_data_frame = NULL;
    short *testing_rx_data_frame = NULL;
    short tx = (short)syna_get_image_rows(false);
    short rx = (short)syna_get_image_cols(false);
    int i;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    testing_data_size = tx * rx;

    testing_data_frame = calloc((size_t)testing_data_size, sizeof(short));
    testing_tx_data_frame = calloc((size_t)tx, sizeof(short));
    testing_rx_data_frame = calloc((size_t)rx, sizeof(short));
    if ((!testing_data_frame) || (!testing_tx_data_frame) || (!testing_rx_data_frame)) {
        printf_e("%s error: fail to allocate memory for testing data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for testing data\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* call function to perform extended high resistance test */
    if (SYNA_RMI_DEV == g_syna_dev) {
        retval = rmi_do_test_ex_high_resistance_rt20(testing_data_frame, testing_rx_data_frame,
                                                     testing_tx_data_frame, ref_frame, col, row,
                                                     limit_surface, limit_rxroe, limit_txroe);

        /* issue a sw reset at the end */
        syna_do_sw_reset();

    }
    else if (SYNA_TCM_DEV == g_syna_dev) {

        retval = tcm_do_test_ex_high_resistance_pid05(testing_data_frame, testing_rx_data_frame,
                                                      testing_tx_data_frame, ref_frame, col, row,
                                                      limit_surface, limit_rxroe, limit_txroe);
    }
    else {
        printf_e("%s error: unknown device, exit.\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device, exit.\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    if (retval < 0) {
        printf_e("%s error: fail to run ex high resistance test\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to run ex high resistance test\n", __func__);
        add_error_msg(err);
#endif
    }

    /* copy the testing data to the p_result array */
    if ((p_result) && (size_result > 0)) {
        if (size_result != testing_data_size) {
            printf_e("%s error: size is mismatch (size_result = %d, testing_data_size = %d)\n",
                     __func__, size_result, testing_data_size);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: size is mismatch (size_result = %d, testing_data_size = %d)\n",
                    __func__, size_result, testing_data_size);
            add_error_msg(err);
#endif
            goto exit;
        }
        /* clone testing data */
        for (i = 0; i < (tx * rx); i++) {
            p_result[i] = testing_data_frame[i];
        }
    }
    if (p_txroe_result) {
        *p_size_txroe_result = (int)tx;
        for (i = 0; i < tx; i++) {
            p_txroe_result[i] = testing_tx_data_frame[i];
        }
    }
    if (p_rxroe_result) {
        *p_size_rxroe_result = (int)rx;
        for (i = 0; i < rx; i++) {
            p_rxroe_result[i] = testing_rx_data_frame[i];
        }
    }

exit:
    if (testing_data_frame)
        free(testing_data_frame);
    if (testing_tx_data_frame)
        free(testing_tx_data_frame);
    if (testing_rx_data_frame)
        free(testing_rx_data_frame);

    return retval;
}
/*
 * Function:  syna_run_test_ex_trx_short_entry
 * --------------------
 * run the extended trx short test
 * this function is separated from syna_run_test_entry
 * because the required parameters are different
 *
 * return: =0, pass
 *         otherwise, fail
 */
int syna_run_test_ex_trx_short_entry(int *p_result_data, int size_result_data, int *p_result_data_ex_pin,
                                     int size_result_data_ex_pin, int *p_result_pin, int size_result_pin,
                                     int *p_limit_1, int size_limit_1, int *p_limit_2, int size_limit_2)
{
    int retval;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((!p_result_data) || (!p_result_pin) || (!p_result_data_ex_pin)) {
        printf_e("%s error: invalid parameter\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: invalid parameter\n", __func__);
        add_error_msg(err);
#endif
        return -ENOMEM;
    }

    /* call function to perform extended trx short test */
    if (SYNA_RMI_DEV == g_syna_dev) {
        retval = rmi_do_test_ex_trx_short_rt26100(p_result_data, size_result_data,
                                                  p_result_data_ex_pin, size_result_data_ex_pin,
                                                  p_result_pin, size_result_pin,
                                                  p_limit_1, size_limit_1, p_limit_2, size_limit_2);
        if (retval < 0) {
            printf_e("%s error: fail to run ex trx short test\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to run ex trx short test\n", __func__);
            add_error_msg(err);
#endif
        }

        /* issue a sw reset at the end */
        syna_do_sw_reset();

    }
    else {
        printf_e("%s error: unknown device, exit.\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device, exit.\n", __func__);
        add_error_msg(err);
#endif
        return -ENOMEM;
    }

    return retval;
}

/*
 * Function:  syna_get_firmware_config_size
 * --------------------
 * inquiry the size of firmware config
 *
 * return: the size of config
 */
int syna_get_firmware_config_size(void)
{
    int retval = 0;
    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = 0;
            break;
        case SYNA_TCM_DEV:
            retval = convert_uc_to_short(g_tcm_handler.app_info_report.static_config_size[0],
                              g_tcm_handler.app_info_report.static_config_size[1]);
            break;
        default:
            printf_e("%s error: unknown syan device\n", __func__);
            break;
    }
    return retval;
}
/*
 * Function:  syna_get_firmware_config
 * --------------------
 * get the firmware configuration
 *
 * return: <0,fail to read a report
 *         otherwise, succeed
 */
int syna_get_firmware_config(unsigned char* buf, int size_buf)
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        printf_e("%s error: unknown device\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            retval = 0;
            break;
        case SYNA_TCM_DEV:
            retval = tcm_get_static_config();
            if (retval < 0) {
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: failed to get the static config\n", __func__);
                add_error_msg(err);
#endif
                return -EINVAL;
            }

            memcpy(buf, g_tcm_handler.static_config, (size_t)size_buf);
            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return retval;
}


/*
 * Function:  syna_run_raw_command
 * --------------------
 * send a raw command to device
 *
 * return: =0 - timeout
 *         <0, fail
 *         >0, bytes access
 */
int syna_run_raw_command(unsigned char type, int cmd,
                         unsigned char* in, int size_in,
                         unsigned char* resp, int size_resp)
{
    int retval = 0;
    unsigned short reg_addr;
    int len = 0;
    unsigned char *packet;
    int i, j;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        printf_e("%s error: unknown device\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }


    switch (g_syna_dev) {
        case SYNA_RMI_DEV:

            if (RAW_CMD_READ == type) {
                reg_addr = (unsigned short)cmd;
                len = size_in;

                printf_i("%s info: rmi read, reg 0x%x (len = %d)\n",
                         __func__, reg_addr, len);

                retval = rmi_read_reg(reg_addr, in, len);
                if (retval < 0) {
#ifdef SAVE_ERR_MSG
                    sprintf(err, "%s error: failed to read rmi reg 0x%x (len = %d)\n",
                            __func__, reg_addr, len);
                    add_error_msg(err);
#endif
                    return -EINVAL;
                }
            }
            else if (RAW_CMD_WRITE == type) {
                reg_addr = (unsigned short) cmd;
                len = size_in;

                printf_i("%s info: rmi write, reg 0x%x (len = %d)\n",
                         __func__, reg_addr, len);

                retval = rmi_write_reg(reg_addr, in, len);
                if (retval < 0) {
#ifdef SAVE_ERR_MSG
                    sprintf(err, "%s error: failed to write rmi reg 0x%x (len = %d)\n",
                            __func__, reg_addr, len);
                    add_error_msg(err);
#endif
                    return -EINVAL;
                }
            }

            break;
        case SYNA_TCM_DEV:

            if (RAW_CMD_READ == type) {
                printf_i("%s info: tcm read, size_in = %d \n", __func__, size_in);

                retval = tcm_read_message(in, (unsigned int)size_in);
                if (retval < 0) {
#ifdef SAVE_ERR_MSG
                    sprintf(err, "%s error: failed to read tcm packet (len = %d)\n",
                            __func__, size_in);
                    add_error_msg(err);
#endif
                    return -EINVAL;
                }
            }
            if (RAW_CMD_WRITE == type) {
                printf_i("%s info: tcm write, cmd 0x%x (size_in = %d)\n",
                         __func__, cmd, size_in);

                len = size_in + 3;
                packet = calloc((size_t) len, sizeof(unsigned char));
                packet[0] = (unsigned char)cmd;
                packet[1] = (unsigned char)(size_in & 0xff);
                packet[2] = (unsigned char)((size_in >> 8) & 0xff);

                printf_i("%s info: tcm write, packet[0 - 2] = 0x%x 0x%x 0x%x\n",
                         __func__, packet[0], packet[1], packet[2]);

                for (i = 3, j = 0; i < len; i++, j++) {
                    packet[i] = in[j];

                    printf_i("%s info: tcm write, packet[%d] = 0x%x\n",
                             __func__, i, packet[i] );
                }

                retval = tcm_raw_control(packet, (unsigned int)len, resp, size_resp);
                if (retval < 0) {
#ifdef SAVE_ERR_MSG
                    sprintf(err, "%s error: failed to write tcm packet, 0x%x 0x%x 0x%x\n",
                            __func__, packet[0], packet[1], packet[2]);
                    add_error_msg(err);
#endif
                    return -EINVAL;
                }

                free(packet);
            }

            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }


    return retval;
}

/*
 * Function:  syna_query_touch_response_entry
 * --------------------
 * this is the entry function to allow the upper application/function
 * to monitor the touch report
 *
 * once touched object is reported, use callback function to
 * notify the function on java layer
 *
 * return: < 0, fail to get touch response
 *         = 0, no touch response detected
 *         > 0, successfully collect the data of 1 points
 */
int syna_query_touch_response_entry(int max_fingers_to_process)
{
    int retval = 0;
    int touch_x[MAX_FINGER] = {0};
    int touch_y[MAX_FINGER] = {0};
    int touch_status[MAX_FINGER] = {0};
    int i;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        printf_e("%s error: unknown device\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    if (SYNA_RMI_DEV == g_syna_dev) {
        retval = rmi_query_touch_response(touch_x, touch_y, touch_status, max_fingers_to_process);
        if (retval < 0) {
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: failed to get the touch response\n", __func__);
            add_error_msg(err);
#endif
            return -EINVAL;
        }
    }
    else if (SYNA_TCM_DEV == g_syna_dev) {
        retval = tcm_query_touch_response(touch_x, touch_y, touch_status, max_fingers_to_process);
        if (retval < 0) {
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: failed to get the touch response\n", __func__);
            add_error_msg(err);
#endif
            return -EINVAL;
        }
    }

    /* use callback function to notify the upper application that a touch event occurs */
#ifdef TOUCH_CALLBACK
    for (i = 0; i < max_fingers_to_process; i++ ) {
        if (g_finger_status[i] != touch_status[i]) {

            if ((touch_status[i] != 0x00) && g_finger_status[i] == 0x00) { /* finger down */
                callback_finger_down(i, touch_x[i], touch_y[i]);
            }
            else if ((touch_status[i] == 0x00) && g_finger_status[i] != 0x00) { /* finger up */
                callback_finger_up(i);
            }

            g_finger_status[i] = touch_status[i];
        }
    }
#endif

    return retval;
}

/*
 * Function:  syna_get_pins_mapping
 * --------------------
 * inquiry the pin mapping
 *
 * return: <0 - fail to get the pin mapping
 *         otherwise, succeed
 */
int syna_get_pins_mapping(int rxes_offset, int rxes_len, int txes_offset, int txes_len)
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if ((SYNA_RMI_DEV != g_syna_dev) && (SYNA_TCM_DEV != g_syna_dev)) {
        printf_e("%s error: unknown device\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: unknown device\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    switch (g_syna_dev) {
        case SYNA_RMI_DEV:
            printf_i("%s info: pins mapping has been stored in rmi_scan_pdt.\n", __func__);
            retval = 0;
            break;
        case SYNA_TCM_DEV:
            retval = tcm_get_pins_mapping(rxes_offset, rxes_len, txes_offset, txes_len,
                                          0, 0, 0, 0, 0, 0, 0, 0);
            if (retval < 0) {
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s error: failed to get the pins mapping\n", __func__);
                add_error_msg(err);
#endif
                return -EINVAL;
            }

            break;
        default:
            printf_e("%s error: unknown device\n", __func__);
            break;
    }
    return retval;
}

