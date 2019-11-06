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
#include <string.h>
#include <unistd.h>

#include "syna_dev_manager.h"
#include "rmi_control.h"

#ifdef SAVE_ERR_MSG
#include "err_msg_ctrl.h"
#endif

#define GET_REPORT_REG_CLEAR_RETRY 100
#define GET_REPORT_REG_CLEAR_DELAY 20000

/*
 * Function:  rmi_f54_read_report_data
 * --------------------
 * get the report image according to the
 * input report type
 *
 * parameter
 *  report_type: report type
 *               which should be defined in enum F54_REPORT_TYPES
 *
 * return: < 0, fail to get report
 *         otherwise, succeed
 */
static int rmi_f54_read_report_data(int report_type)
{
    int retval = 0;
    int time_count = 0;
    unsigned char fifo_idx_data[2] = {0,0};
    unsigned char cmd_data;
    int retry = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* step 1 */
    /* check the get report flag at first */
    retval = rmi_read_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
    if (retval < 0) {
        printf_e("%s error: fail to check F54 command base register\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read F54_CMD base reg(addr=0x%x) before report image reading\n",
                __func__, g_rmi_pdt.F54.command_base_addr);
        add_error_msg(err);
#endif
        goto exit;
    }
    if (cmd_data & 0x01) {
        do {
            printf_e("%s error: F54_CMD(bit 0) GetReport = 1 (data=0x%04x) (retry=%d)\n",
                     __func__, cmd_data, retry);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: F54_CMD(addr=0x%x) GetReport is not cleared (data=0x%04x) (retry=%d)\n",
                            __func__, g_rmi_pdt.F54.command_base_addr, cmd_data, retry);
            add_error_msg(err);
#endif
            usleep(GET_REPORT_REG_CLEAR_DELAY);
            retry += 1;
            retval = rmi_read_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
        } while( (retry < GET_REPORT_REG_CLEAR_RETRY) && ((cmd_data & 0x01) != 0x00) );

        if (( retry == GET_REPORT_REG_CLEAR_RETRY ) || (retval < 0)) {
            printf_e("%s error: F54_CMD is not cleared (data=0x%04x)\n",
                     __func__, cmd_data);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: F54_CMD(addr=0x%x) F54_CMD is not cleared (data=0x%04x)\n",
                            __func__, g_rmi_pdt.F54.command_base_addr, cmd_data);
            add_error_msg(err);
#endif
            retval = -EIO;
            goto exit;
        }
    }

    /* step 2 */
    /* set report type to F54 data register */
    cmd_data = (unsigned char)(report_type);
    retval = rmi_write_reg(g_rmi_pdt.F54.data_base_addr, &cmd_data, sizeof(cmd_data));
    if (retval < 0) {
        printf_e("%s error: fail to set report type to F54 data register\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to set report type to F54 data register (addr=0x%x, wr_data=0x%x)\n",
                __func__, g_rmi_pdt.F54.data_base_addr, cmd_data);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* step 3 */
    /* set fifo index */
    retval = rmi_write_reg((unsigned short) (g_rmi_pdt.F54.data_base_addr + 1), fifo_idx_data, sizeof(fifo_idx_data));
    if (retval < 0) {
        printf_e("%s error: fail to set fifo index register\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to set fifo index register (addr=0x%x, wr_data=0x%x, 0x%x)\n",
                __func__, (g_rmi_pdt.F54.data_base_addr + 1), fifo_idx_data[0], fifo_idx_data[1]);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* step 4 */
    /* get report, setting the get report flag to '1' to request a report image reading
     * wait until the flag has been cleared to '0' */
    cmd_data = RMI_COMMAND_GET_REPORT;
    retval = rmi_write_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
    if (retval < 0) {
        printf_e("%s error: fail to set get report flag\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to set get_report flag to 1 (addr=0x%x, wr_data=0x%x)\n",
                __func__, g_rmi_pdt.F54.command_base_addr, cmd_data);
        add_error_msg(err);
#endif
        goto exit;
    }

    do {
        usleep(10000);  // polling every 10 ms

        retval = rmi_read_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
        if (retval < 0) {
            printf_e("%s error: fail to read F54 command base register\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read F54_CMD base reg(addr=0x%x) in polling loop\n",
                    __func__, g_rmi_pdt.F54.command_base_addr);
            add_error_msg(err);
#endif
            goto exit;
        }
        time_count += 1;

    } while ((cmd_data & 0x01) && (time_count < RMI_GET_REPORT_TIMEOUT));

    if (time_count == RMI_GET_REPORT_TIMEOUT) {
        printf_e("%s error: fail to get report image (rt %d), timeout! (f54_cmd: %x)\n",
                 __func__, report_type, cmd_data);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get report image (rt %d), timeout!\n",
                __func__, report_type);
        add_error_msg(err);
#endif
        retval = -ETIMEDOUT;
        goto exit;
    }
exit:
    return retval;
}

/*
 * Function:  rmi_f54_read_report_frame
 * --------------------
 * get the appointed report image in a frame
 *
 * parameter
 *  report_type : report type
 *  p_out : a buffer to store the output image
 *  size_out : size of output buffer
 *  out_in_landscape : true to place the data in the landscape layout
 *                     false, use the fw layout
 *
 * return: < 0, fail to get report
 *         otherwise, succeed
 */
int rmi_f54_read_report_frame(int report_type, int *p_out, int size_out, bool out_in_landscape)
{
    int retval = 0;
    short *p_data_16;
    int tx = g_rmi_pdt.tx_assigned;
    int rx = g_rmi_pdt.rx_assigned;
    int size = tx * rx * sizeof(short);
    unsigned char *data_buf = NULL;
    int i, j, offset;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_out) {
        printf_e("%s error: p_out is NULL\n", __func__);
        retval = -EINVAL;
        goto exit;
    }
    if (size_out != tx * rx) {
        printf_e("%s error: size is mismatching\n", __func__);
        retval = -EINVAL;
        goto exit;
    }
    /* create a local buffer to fill the report data */
    data_buf = calloc((size_t)size, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: fail to allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* move step 1 - 4 to a function */
    retval = rmi_f54_read_report_data(report_type);
    if (retval < 0) {
        printf_e("%s error: fail to request the report %d \n", __func__, report_type);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to request the report %d \n", __func__, report_type);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* step 5 */
    /* retrieve report data */
    retval = rmi_read_reg((unsigned short) (g_rmi_pdt.F54.data_base_addr + RMI_REPROT_DATA_OFFSET),
                          data_buf, size);
    if (retval < 0) {
        printf_e("%s error: fail to retrieve report data \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to retrieve report data (reg_addr=0x%x, size=%d)\n",
                __func__, (g_rmi_pdt.F54.data_base_addr + RMI_REPROT_DATA_OFFSET), size);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* copy data to the output buffer                                                   */
    /* however, there are two different layout in f54 report                            */
    /*   (1) landscape (rx > tx)                          (2) portrait (rx < tx)        */
    /*         rx0   rx1   rx2  ...  rx(n-1)  rx(n)             rx0    rx1  ...  rx(n)  */
    /*   tx0    0     1     2   ...   (n-1)    n           tx0   0      1   ...    n    */
    /*   tx1  1*n+0 1*n+1 1*n+2 ... 1*n+(n-1) 1*n+n        tx1 1*n+0  1*n+1 ...  1*n+n  */
    /*    .                     ...                        tx2 2*n+0  2*n+1 ...  2*n+n  */
    /*  tx(m) m*n+0 m*n+1 m*n+2 ... m*n+(n-1) m*n+n         .               ...         */
    /*                                                      .               ...         */
    /*                                                      .               ...         */
    /*                                                   tx(m) m*n+0  m*n+1 ...  m*n+n  */
    /*                                                                                  */
    /* if out_in_landscape is true, place the data to the landscape layout              */
    /* otherwise, use the firmware layout                                               */
    /*                                                                                  */
    p_data_16 = (short *)&data_buf[0];
    for(i = 0; i < g_rmi_pdt.tx_assigned; i++) {
        for(j = 0; j < g_rmi_pdt.rx_assigned; j++) {

            if (out_in_landscape) {
                if (g_rmi_pdt.rx_assigned > g_rmi_pdt.tx_assigned)
                    offset = i*g_rmi_pdt.rx_assigned + j;
                else
                    offset = j*g_rmi_pdt.tx_assigned + i;
            }
            else
                offset = i*g_rmi_pdt.rx_assigned + j;


            p_out[offset] = *p_data_16;

            // printf_i(" (%d, %d) %4d\n", i, j, *p_data_16);

            p_data_16++;
        }
    }
exit:
    if(data_buf)
        free(data_buf);

    return retval;
}
/*
 * Function:  rmi_f54_read_report_ucarray
 * --------------------
 * get the appointed report image
 * the output will be an unsigned char array
 *
 * parameter
 *  report_type: report type
 *               which should be defined in enum F54_REPORT_TYPES
 *  p_out: a buffer to store the output char data
 *  size_out: size of array length
 *
 * return: < 0, fail to get report
 *         otherwise, succeed
 */
int rmi_f54_read_report_ucarray(int report_type, unsigned char *p_out, int size_out)
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_out) {
        printf_e("%s: p_out is NULL\n", __func__);
        retval = -EINVAL;
        goto exit;
    }

    /* move step 1 - 4 to a function */
    retval = rmi_f54_read_report_data(report_type);
    if (retval < 0) {
        printf_e("%s error: fail to request the report %d \n", __func__, report_type);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to request the report %d \n", __func__, report_type);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* step 5 */
    /* retrieve report data */
    retval = rmi_read_reg((unsigned short) (g_rmi_pdt.F54.data_base_addr + RMI_REPROT_DATA_OFFSET),
                          p_out, size_out);
    if (retval < 0) {
        printf_e("%s error: fail to get report data \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get report data (reg_addr=0x%x, size=%d)\n",
                __func__, (g_rmi_pdt.F54.data_base_addr + RMI_REPROT_DATA_OFFSET), size_out);
        add_error_msg(err);
#endif
        goto exit;
    }

exit:
    return retval;
}
