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

#include "syna_dev_manager.h"
#include "rmi_control.h"


/*
 * Function:  rmi_get_identify
 * --------------------
 * do identify
 *
 * return: <0, fail to get identification info
 *         otherwise, succeed
 */
int rmi_get_identify(char *p_buf)
{
    int i;
    int str_offset = 0;

    str_offset += sprintf(p_buf + str_offset, "[ RMI Dev Identification ]\n");
    str_offset += sprintf(p_buf + str_offset, " Firmware Mode  = ui\n");
    str_offset += sprintf(p_buf + str_offset, " Device ID  = %s\n", g_rmi_pdt.asic_type);
    str_offset += sprintf(p_buf + str_offset, " FW Build ID  = %d\n", g_rmi_pdt.build_id);
    str_offset += sprintf(p_buf + str_offset, " FW Config ID  = ");
    for (i = 0; i < g_rmi_pdt.size_of_config_id-1; i++) {
        str_offset += sprintf(p_buf + str_offset, "%02x-", g_rmi_pdt.config_id[i]);
        if ((i != 0) && ((i+1)%12 == 0)) str_offset += sprintf(p_buf + str_offset, "\n                      ");
    }
    str_offset += sprintf(p_buf + str_offset, "%02x\n", g_rmi_pdt.config_id[g_rmi_pdt.size_of_config_id-1]);

    str_offset += sprintf(p_buf + str_offset, " Functions  = $34\n");
    str_offset += sprintf(p_buf + str_offset, "                       $01\n");
    if (g_rmi_pdt.F12.ID == 0x12)
        str_offset += sprintf(p_buf + str_offset, "                       $12\n");
    if (g_rmi_pdt.F54.ID == 0x54)
        str_offset += sprintf(p_buf + str_offset, "                       $54\n");
    if (g_rmi_pdt.F55.ID == 0x55)
        str_offset += sprintf(p_buf + str_offset, "                       $55\n");
    if (g_rmi_pdt.F1A.ID == 0x1a)
        str_offset += sprintf(p_buf + str_offset, "                       $1A\n");

    str_offset += sprintf(p_buf + str_offset, " Max. X coordinate  = %d\n", g_rmi_pdt.sensor_max_x);
    str_offset += sprintf(p_buf + str_offset, " Max. Y coordinate  = %d\n", g_rmi_pdt.sensor_max_y);
    str_offset += sprintf(p_buf + str_offset, " Number of Fingers Supported = 10\n");
    str_offset += sprintf(p_buf + str_offset, " Tx Channels = %d\n", syna_get_image_rows(false));
    str_offset += sprintf(p_buf + str_offset, " Rx Channels = %d\n", syna_get_image_cols(false));


    sprintf(p_buf + str_offset, "\n");

    return str_offset;
}