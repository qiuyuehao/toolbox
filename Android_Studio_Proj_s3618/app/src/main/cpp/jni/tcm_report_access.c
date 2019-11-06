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
#include "tcm_control.h"

#ifdef SAVE_ERR_MSG
#include "err_msg_ctrl.h"
#endif

/*
 * Function:  tcm_enable_report
 * --------------------
 * enable/disable the report image streaming
 *
 * return: <0, fail to enable/disable the report output
 *         otherwise, succeed
 */
int tcm_enable_report(bool enable, enum tcm_report_code report_code)
{
    int retval = 0;
    unsigned char command_packet[4] = {0, 0, 0, 0};
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* command code */
    /* CMD_ENABLE_REPORT or CMD_DISABLE_REPORT */
    if (enable) {
        command_packet[0] = CMD_ENABLE_REPORT;
    }
    else {
        command_packet[0] = CMD_DISABLE_REPORT;
    }
    /* command length */
    /* length = 1 */
    command_packet[1] = 0x01;
    command_packet[2] = 0x00;
    /* command payload, 1 byte*/
    /* byte[1] = report type */
    command_packet[3] = report_code;

    /* send command to tcm device */
    retval = tcm_write_message(command_packet, sizeof(command_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, %s 0x%x 0x%x 0x%x\n", __func__,
               (command_packet[0] == CMD_ENABLE_REPORT)?"CMD_ENABLE_REPORT":"CMD_DISABLE_REPORT",
                 command_packet[1], command_packet[2], command_packet[3]);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: fail to send command, %s 0x%x 0x%x 0x%x\n", __func__,
                (command_packet[0] == CMD_ENABLE_REPORT)?"CMD_ENABLE_REPORT":"CMD_DISABLE_REPORT",
                command_packet[1], command_packet[2], command_packet[3]);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: fail to get the response of %s_report (report:0x%x)\n",
               __func__, (enable)?"enable":"disable", (int)report_code);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: fail to get the response of %s_report (report:0x%x)\n",
                __func__, (enable)?"enable":"disable", (int)report_code);
        add_error_msg(err);
#endif
        retval = -EIO;
        return retval;
    }
    printf_i("%s info: %s is completed. (report:0x%x)",
             __func__, (enable)?"CMD_ENABLE_REPORT":"CMD_DISABLE_REPORT", (int)report_code);

    return retval;
}

/*
 * Function:  tcm_read_report_frame
 * --------------------
 * retrieve the report data from tcm firmware
 * this function should be called after enable_tcm_report
 *
 * return: <0, fail to read a tcm report
 *         otherwise, succeed
 */
int tcm_read_report_frame(int type, int *p_out, int size_out, bool out_in_landscape)
{
    int retval = 0;
    short *p_data_16;
    unsigned short *p_u_data_16;
    unsigned char *data_buf = NULL;
    int rows = (g_tcm_handler.app_info_report.num_of_image_rows[0] |
                g_tcm_handler.app_info_report.num_of_image_rows[1] << 8);
    int cols = (g_tcm_handler.app_info_report.num_of_image_cols[0] |
                g_tcm_handler.app_info_report.num_of_image_cols[1] << 8);
    int buttons = (g_tcm_handler.app_info_report.num_of_buttons[0] |
                   g_tcm_handler.app_info_report.num_of_buttons[1] << 8);
    int has_hybrid = (g_tcm_handler.app_info_report.has_hybrid_data[0] |
                      g_tcm_handler.app_info_report.has_hybrid_data[1] << 8);
    int force_elecs = (g_tcm_handler.app_info_report.num_of_force_elecs[0] |
                       g_tcm_handler.app_info_report.num_of_force_elecs[1] << 8);
    int report_size;
    int report_timeout = 0;
    int report_payload = 0;
    int i, j, offset;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_out) {
        printf_e("%s: p_out is NULL\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: p_out is NULL.\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    if (has_hybrid) {
        report_size = (2 * cols * rows) + 2 * (cols + rows + buttons + force_elecs);
    }
    else {
        report_size = (2 * cols * rows);
    }

    if (report_size > (size_out * sizeof(short))) {
        printf_i("%s warning: size of report image is mismatching. report_size = %d, size_out = %d\n",
                 __func__, report_size, size_out);
    }

    /* create a local buffer to fill the report data */
    data_buf = calloc((size_t)report_size, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* polling the interrupt to wait for the requested type */
    do {
        usleep(TCM_POLLING_DELAY_MS * 1000);
        /* check the header */
        retval = tcm_read_message(data_buf, sizeof(struct tcm_message_header));
        if (retval < 0) {
            printf_e("%s error: fail to read tcm message header\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read tcm message header\n", __func__);
            add_error_msg(err);
#endif
            retval = -EINVAL;
            goto exit;
        }
        if ( 0xA5 == data_buf[0]) {
            report_payload = data_buf[2] | data_buf[3] << 8;

            /* once get the acknowledge of requested report type */
            /* get the size of data payload  */
            if ( type == data_buf[1]) {
                break;
            }
            /* if a touch report is coming, parse the touch report */
            else if (TCM_REPORT_TOUCH == data_buf[1]) {
                tcm_get_touch_report(report_payload);
            }
            /* drop the tcm package if it is not required report */
            else {
                tcm_drop_package(report_payload);
            }
        }
        report_timeout++ ;
    } while(report_timeout < TCM_POLLING_TIMOUT);

    if (report_timeout == TCM_POLLING_TIMOUT) {
        retval = -EINVAL;
        printf_e("%s error: read report timeout (type: 0x%x)\n", __func__, type);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: read report timeout (type: 0x%x)\n", __func__, type);
        add_error_msg(err);
#endif
        goto exit;
    }

    printf_i("%s info: report = 0x%x (payload size = %d)\n", __func__, type, report_payload);

    /* retrieve report image */
    retval = tcm_get_payload(data_buf, report_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, report_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, report_payload);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* copy data to the output image buffer                                             */
    /*                                                                                  */
    /* in order to fulfill the display, the output format will be always placed in      */
    /* the landscape mode, which row > col                                              */
    /*         row_0  row_1  row_2  ...  ow_(n-2) row_(n-1) row_n                       */
    /*  col_0    0      1      2    ...   (n-2)     (n-1)     n                         */
    /*  col_1  1*n+0  1*n+1  1*n+2  ... 1*n+(n-2) 1*n+(n-1) 1*n+n                       */
    /*  col_2  2*n+0  2*n+1  2*n+2  ... 2*n+(n-2) 2*n+(n-1) 2*n+n                       */
    /*    .                                                                             */
    /*    .                     ...                                                     */
    /*    .                                                                             */
    /* col_(m) m*n+0  m*n+1  m*n+2  ... m*n+(n-2) m*n+(n-1) m*n+n                       */
    /*                                                                                  */
    /* if out_in_landscape is true, place the data to the landscape layout              */
    /* otherwise, use the firmware layout                                               */
    /*                                                                                  */
    p_data_16 = (short *)&data_buf[0];
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {

            if (out_in_landscape) {
                if (cols > rows)
                    offset = i*cols + j;
                else
                    offset = j*rows + i;
            }
            else
                offset = i*cols + j;

            p_out[offset] = *p_data_16;

            p_data_16++;
        }
    }

    offset = rows * cols;
    if (has_hybrid && (size_out > offset) ) {

        j = (int)(report_payload/sizeof(short)) - offset;

        if (type == TCM_REPORT_RAW) {
            p_u_data_16 = (unsigned short *)p_data_16;

            for(i = 0; i < j; i++) {
                p_out[offset + i] = *p_u_data_16;

                p_u_data_16++;
            }
        }
        else {
            for(i = 0; i < j; i++) {
                p_out[offset + i] = *p_data_16;

                p_data_16++;
            }
        }

    }

exit:
    if(data_buf)
        free(data_buf);

    return retval;
}
