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

/*
 * Function:  tcm_get_identify_info
 * --------------------
 * do identify and retrieve the identify report
 *
 * return: <0 - fail to get identification info
 *         otherwise, succeed
 */
int tcm_get_identify_info(char *p_buf)
{
    int retval = 0;
    unsigned char command = CMD_IDENTIFY;
    unsigned char *data_buf;
    struct tcm_identify_report *packet;
    int payload_len;
    int size;
    int str_offset = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    memset(&g_tcm_handler.identify_report, 0x00, sizeof(struct tcm_identify_report));

    size = sizeof(struct tcm_message_header) + sizeof(struct tcm_identify_report);
    data_buf = calloc((size_t)size, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
        retval = -ENOMEM;
        goto exit;
    }

    /* send identify command to tcm device */
    retval = tcm_write_message(&command, sizeof(command));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_IDENTIFY\n", __func__);
        retval = -EINVAL;
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send command, CMD_IDENTIFY\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }
    /* wait for the command completion */
    payload_len = tcm_wait_for_command_ready();
    if ( payload_len < 0) {
        printf_e("%s error: fail to get the response of CMD_IDENTIFY\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get the response of CMD_IDENTIFY\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    printf_i("%s info: CMD_IDENTIFY is completed.", __func__);

    /* get the identify information                            */
    /*   packet header : 2 bytes (0xA5 + STATUS_CONTINUED_READ)*/
    /*   identify info : payload_len                           */
    /*   ending        : 1 bytes (0x5A)                        */
    size = payload_len + 3;
    retval = tcm_read_message(data_buf, (unsigned int) size);
    if (retval < 0) {
        printf_e("%s error: fail to read remaining data\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read remaining data\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if (STATUS_CONTINUED_READ == data_buf[1]) {
        packet = (struct tcm_identify_report *)(data_buf + 2);

        size = (payload_len < sizeof(struct tcm_identify_report))?
               payload_len:(int)sizeof(struct tcm_identify_report);

        memcpy(&g_tcm_handler.identify_report,
               packet,
               (size_t)size);
    }
    else {
        printf_e("%s error: unknown read transaction (0x%x 0x%x)\n", __func__, data_buf[0], data_buf[1]);
        retval = -EINVAL;
        goto exit;
    }

    if (!p_buf)
        goto exit;

    str_offset += sprintf(p_buf + str_offset, "[ TCM Identification ]\n");
    str_offset += sprintf(p_buf + str_offset, " Packet Version  = %d\n",
                          g_tcm_handler.identify_report.version);
    if (MODE_APPLICATION == g_tcm_handler.identify_report.mode) {
        str_offset += sprintf(p_buf + str_offset, " Firmware Mode  = application (0x%x)\n",
                              g_tcm_handler.identify_report.mode);
    }
    else if (MODE_BOOTLOADER == g_tcm_handler.identify_report.mode) {
        str_offset += sprintf(p_buf + str_offset, " Firmware Mode  = bootloader (0x%x)\n",
                              g_tcm_handler.identify_report.mode);
    }
    else if (MODE_BOOTLOADER_TDDI == g_tcm_handler.identify_report.mode) {
        str_offset += sprintf(p_buf + str_offset, " Firmware Mode  = tddi bootloader (0x%x)\n",
                              g_tcm_handler.identify_report.mode);
    }
    else {
        str_offset += sprintf(p_buf + str_offset, " Firmware Mode  = unknown (0x%x)\n",
                              g_tcm_handler.identify_report.mode);
    }
    str_offset += sprintf(p_buf + str_offset, " Part Number  = %s\n",
                          g_tcm_handler.identify_report.part_number);
    str_offset += sprintf(p_buf + str_offset, " FW Build ID = %d\n",
                          ((unsigned int)g_tcm_handler.identify_report.build_id[0]) +
                          ((unsigned int)g_tcm_handler.identify_report.build_id[1] * 0x100) +
                          ((unsigned int)g_tcm_handler.identify_report.build_id[2] * 0x10000) +
                          ((unsigned int)g_tcm_handler.identify_report.build_id[3] * 0x1000000) );
    str_offset += sprintf(p_buf + str_offset, " Max Written Size  = %d\n",
                          convert_uc_to_short(g_tcm_handler.identify_report.max_write_size[0],
                                   g_tcm_handler.identify_report.max_write_size[1]));
    sprintf(p_buf + str_offset, "\n");

exit:
    if (data_buf)
        free(data_buf);

    return retval;
}

/*
 * Function:  tcm_get_app_info
 * --------------------
  * retrieve the application information
 *
 * return: <0 - fail to get application info
 *         otherwise, succeed
 */
int tcm_get_app_info(char *p_buf)
{
    int retval = 0;
    unsigned char command = CMD_GET_APPLICATION_INFO;
    unsigned char *data_buf;
    int payload_len;
    int size;
    short app_status;
    struct tcm_app_info *packet;
    int str_offset = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    memset(&g_tcm_handler.app_info_report, 0x00, sizeof(struct tcm_app_info));

    size = sizeof(struct tcm_message_header) + sizeof(struct tcm_app_info);
    data_buf = calloc((size_t)size, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
        retval = -ENOMEM;
        goto exit;
    }

    /* send command to tcm device */
    retval = tcm_write_message(&command, sizeof(command));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_GET_APPLICATION_INFO\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send command, CMD_GET_APPLICATION_INFO\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    /* wait for the command completion */
    payload_len = tcm_wait_for_command_ready();
    if ( payload_len < 0) {
        printf_e("%s error: fail to get the response of CMD_GET_APPLICATION_INFO\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get the response of CMD_GET_APPLICATION_INFO\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }
    printf_i("%s info: CMD_GET_APPLICATION_INFO is completed.", __func__);

    /* retrieve the application information                    */
    /*   packet header : 2 bytes (0xA5 + STATUS_CONTINUED_READ)*/
    /*   application info : payload_len                        */
    /*   ending        : 1 bytes (0x5A)                        */
    size = payload_len + 3;
    retval = tcm_read_message(data_buf, (unsigned int) size);
    if (retval < 0) {
        printf_e("%s error: fail to read response\n", __func__);
        retval = -EINVAL;
        goto exit;
    }

    if (STATUS_CONTINUED_READ == data_buf[1]) {
        packet = (struct tcm_app_info *)(data_buf + 2);

        size = (payload_len < sizeof(struct tcm_app_info))?
               payload_len:(int)sizeof(struct tcm_app_info);

        memcpy(&g_tcm_handler.app_info_report,
               packet,
               (size_t)size);
    }
    else {
        printf_e("%s error: unknown read transaction (0x%x 0x%x)\n", __func__,
                 data_buf[0], data_buf[1]);
        retval = -EINVAL;
        goto exit;
    }

    if (!p_buf)
        goto exit;

    str_offset += sprintf(p_buf + str_offset, "[ TCM Application Info ]\n");
    str_offset += sprintf(p_buf + str_offset, " Version  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.version[0],
                                   g_tcm_handler.app_info_report.version[1]));
    app_status = convert_uc_to_short(g_tcm_handler.app_info_report.status[0],
                          g_tcm_handler.app_info_report.status[1]);
    if ( app_status == 0x0000 ) {
        str_offset += sprintf(p_buf + str_offset, " Status  = OK\n");
    }
    else {
        str_offset += sprintf(p_buf + str_offset, " Status  = 0x%x\n",
                              convert_uc_to_short(g_tcm_handler.app_info_report.status[0],
                                       g_tcm_handler.app_info_report.status[1]));
    }

    str_offset += sprintf(p_buf + str_offset, " FW Config ID  = %x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x\n",
                          g_tcm_handler.app_info_report.customer_config_id[0],
                          g_tcm_handler.app_info_report.customer_config_id[1],
                          g_tcm_handler.app_info_report.customer_config_id[2],
                          g_tcm_handler.app_info_report.customer_config_id[3],
                          g_tcm_handler.app_info_report.customer_config_id[4],
                          g_tcm_handler.app_info_report.customer_config_id[5],
                          g_tcm_handler.app_info_report.customer_config_id[6],
                          g_tcm_handler.app_info_report.customer_config_id[7],
                          g_tcm_handler.app_info_report.customer_config_id[8],
                          g_tcm_handler.app_info_report.customer_config_id[9],
                          g_tcm_handler.app_info_report.customer_config_id[10],
                          g_tcm_handler.app_info_report.customer_config_id[11],
                          g_tcm_handler.app_info_report.customer_config_id[12],
                          g_tcm_handler.app_info_report.customer_config_id[13],
                          g_tcm_handler.app_info_report.customer_config_id[14],
                          g_tcm_handler.app_info_report.customer_config_id[15]);

    str_offset += sprintf(p_buf + str_offset, " App. config size  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.app_config_size[0],
                                              g_tcm_handler.app_info_report.app_config_size[1]));
    str_offset += sprintf(p_buf + str_offset, " Static config size  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.static_config_size[0],
                                              g_tcm_handler.app_info_report.static_config_size[1]));

    str_offset += sprintf(p_buf + str_offset, " Max. X coordinate  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.max_x[0],
                                   g_tcm_handler.app_info_report.max_x[1]));
    str_offset += sprintf(p_buf + str_offset, " Max. Y coordinate  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.max_y[0],
                                   g_tcm_handler.app_info_report.max_y[1]));
    str_offset += sprintf(p_buf + str_offset, " Number of objects supported  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.max_objects[0],
                                   g_tcm_handler.app_info_report.max_objects[1]));
    str_offset += sprintf(p_buf + str_offset, " Image Rows  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.num_of_image_rows[0],
                                   g_tcm_handler.app_info_report.num_of_image_rows[1]));
    str_offset += sprintf(p_buf + str_offset, " Image Columns  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.num_of_image_cols[0],
                                   g_tcm_handler.app_info_report.num_of_image_cols[1]));
    str_offset += sprintf(p_buf + str_offset, " Num of Buttons  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.num_of_buttons[0],
                                   g_tcm_handler.app_info_report.num_of_buttons[1]));
    str_offset += sprintf(p_buf + str_offset, " Image has profile data  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.has_hybrid_data[0],
                                   g_tcm_handler.app_info_report.has_hybrid_data[1]));
    str_offset += sprintf(p_buf + str_offset, " Num of Force Electrodes  = %d\n",
                          convert_uc_to_short(g_tcm_handler.app_info_report.num_of_force_elecs[0],
                                   g_tcm_handler.app_info_report.num_of_force_elecs[1]));
    sprintf(p_buf + str_offset, "\n\n");

exit:
    if (data_buf)
        free(data_buf);

    return retval;
}

