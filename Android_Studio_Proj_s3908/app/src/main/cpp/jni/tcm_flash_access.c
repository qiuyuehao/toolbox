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
 * Function:  tcm_get_data_location
 * --------------------
 * query the location of requested data code
 *
 * return: <0, fail to get the requested location
 *         otherwise, succeed
 */
int tcm_get_data_location(unsigned char tcm_data_code,
                          unsigned short* start_addr_in_blocks, unsigned short* length_in_blocks)
{
    int retval = 0;
    unsigned char cmd_packet[4] = {CMD_GET_DATA_LOCATION, 1, 0, 0};
    unsigned char* data_buf;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* assign the data code */
    cmd_packet[3] = tcm_data_code;

    /* send command to tcm device */
    retval = tcm_write_message(cmd_packet, sizeof(cmd_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_GET_DATA_LOCATION 0x%x 0x%x 0x%x\n",
                 __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3]);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send command, CMD_GET_DATA_LOCATION 0x%x 0x%x 0x%x\n",
                __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3]);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf("%s error: fail to read upp data location \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read upp data location \n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        return retval;
    }

    data_buf = calloc((size_t)retval + 3, sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* get payload */
    retval = tcm_get_payload(data_buf, retval);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload \n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* retrieve the start address in blocks and the data length in blocks */
    *start_addr_in_blocks = (unsigned short)data_buf[0] | (unsigned short)(data_buf[1] << 8);
    *length_in_blocks = (unsigned short)data_buf[2] | (unsigned short)(data_buf[3] << 8);

    printf_i("%s info: data code= 0x%02x, start addr in blocks= 0x%04x, length in blocks= 0x%04x\n",
             __func__, tcm_data_code, *start_addr_in_blocks, *length_in_blocks);

exit:
    if (data_buf)
        free(data_buf);

    return retval;
}
/*
 * Function:  tcm_get_boot_info
 * --------------------
 * retrieve the bootloader information
 *
 * return: <0 - fail to get boot info
 *         otherwise, succeed
 */
int tcm_get_boot_info(void)
{
    int retval = 0;
    unsigned char cmd_packet[3] = {CMD_GET_BOOT_INFO, 0, 0};
    unsigned char *data_buf;
    int size;
    int payload_len;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif
    memset(&g_tcm_handler.boot_info_report, 0x00, sizeof(struct tcm_boot_info));

    /* send command to tcm device */
    retval = tcm_write_message(cmd_packet, sizeof(cmd_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_GET_BOOT_INFO\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send command, CMD_GET_BOOT_INFO\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* wait for the command completion */
    payload_len = tcm_wait_for_command_ready();
    if (payload_len < 0) {
        printf_e("%s error: fail to get boot info \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get boot info \n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        return retval;
    }
    printf_i("%s info: CMD_GET_BOOT_INFO is completed.", __func__);

    data_buf = calloc((size_t)(payload_len + 3), sizeof(unsigned char));
    if (!data_buf) {
        printf_e("%s error: fail to allocate data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate data_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* retrieve boot info */
    retval = tcm_get_payload(data_buf, (unsigned int)payload_len);
    if (retval < 0) {
        printf_e("%s error: fail to get boot info \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get boot info \n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        goto exit;
    }

    size = (payload_len < sizeof(struct tcm_boot_info))?
           payload_len : (int)sizeof(struct tcm_boot_info);

    memcpy(&g_tcm_handler.boot_info_report, data_buf, (size_t)size);

    printf_i("%s info: write_block_size_words = 0x%04x", __func__,
             g_tcm_handler.boot_info_report.write_block_size_words);
    printf_i("%s info: erase_page_size_words = 0x%04x", __func__,
             convert_uc_to_short(g_tcm_handler.boot_info_report.erase_page_size_words[0],
                                 g_tcm_handler.boot_info_report.erase_page_size_words[1]));
exit:
    if (data_buf)
        free(data_buf);

    return retval;
}

/*
 * Function:  tcm_erase_flash_data
 * --------------------
 * function to erase the appointed data area
 * the unit is based on erase page size
 *
 * return: <0, fail to erase the data
 *         otherwise, succeed
 */
int tcm_erase_flash_data(unsigned short start_address_in_blocks, unsigned short length_in_blocks,
                         bool is_4byte_format)
{
    int retval = 0;
    struct tcm_boot_info* boot_info = &g_tcm_handler.boot_info_report;
    unsigned short block_size_words = boot_info->write_block_size_words;
    unsigned short erase_page_size_words = (unsigned short) convert_uc_to_short(
            boot_info->erase_page_size_words[0], boot_info->erase_page_size_words[1]);
    unsigned char cmd_packet[7] = {0};
    unsigned int cmd_len = 0;
    unsigned short first_erase_page;
    unsigned short num_erase_page;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (( 0 == block_size_words) || (0 == erase_page_size_words)) {
        printf_e("%s error: invalid parameter. block size words= 0x%04x, erase page size words= 0x%04x\n\n",
                 __func__, block_size_words, erase_page_size_words);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter. block size words= 0x%04x, erase page size words= 0x%04x\n",
                __func__, block_size_words, erase_page_size_words);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* the first erase page = ( address_in_blocks * block size ) / erase page size */
    first_erase_page = (start_address_in_blocks * block_size_words) / erase_page_size_words;
    /* number of erase page = ( length in blocks * block size ) / erase page size */
    num_erase_page = (length_in_blocks * block_size_words) / erase_page_size_words;
    if (num_erase_page == 0)
        num_erase_page = 1; /* the minimum size should be 1 erase page */

    cmd_packet[0] = CMD_ERASE_FLASH;
    cmd_packet[1] = (unsigned char) ((is_4byte_format)? 0x04 : 0x02);
    if (is_4byte_format) {
        cmd_packet[3] = (unsigned char) (first_erase_page & 0xff);
        cmd_packet[4] = (unsigned char) (first_erase_page >> 8);
        cmd_packet[5] = (unsigned char) (num_erase_page & 0xff);
        cmd_packet[6] = (unsigned char) (num_erase_page >> 8);

        cmd_len = 7;
        printf_i("%s info: send command CMD_ERASE_FLASH 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                 __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3], cmd_packet[4], cmd_packet[5], cmd_packet[6]);
    }
    else {
        cmd_packet[3] = (unsigned char) (first_erase_page & 0xff);
        cmd_packet[4] = (unsigned char) (num_erase_page & 0xff);

        cmd_len = 5;
        printf_i("%s info: send command CMD_ERASE_FLASH 0x%02x 0x%02x 0x%02x 0x%02x\n",
                 __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3], cmd_packet[4]);
    }

    /* send command to tcm device */
    retval = tcm_write_message(cmd_packet, cmd_len);
    if (retval < 0) {
        printf_e("%s error: fail to send command CMD_ERASE_FLASH\n", __func__);
#ifdef SAVE_ERR_MSG
        if (is_4byte_format)
            sprintf(err, "%s error: fail to send command CMD_ERASE_FLASH 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3], cmd_packet[4], cmd_packet[5], cmd_packet[6]);
        else
            sprintf(err, "%s error: fail to send command CMD_ERASE_FLASH 0x%02x 0x%02x 0x%02x 0x%02x\n",
                    __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3], cmd_packet[4]);

        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    usleep(TCM_ERASE_FLASH_DELAY_MS * 1000);

    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: fail to erase data in the flash, address in blocks= 0x%4x, length in blocks= 0x%4x\n",
                 __func__, start_address_in_blocks, length_in_blocks);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to erase data in the flash, address in blocks= 0x%4x, length in blocks= 0x%4x\n",
                __func__, start_address_in_blocks, length_in_blocks);
        add_error_msg(err);
#endif
        retval = -EIO;
        return retval;
    }
    printf_i("%s done", __func__);
    return retval;
}
/*
 * Function:  tcm_write_flash_data
 * --------------------
 * function to write data into flash
 *
 * return: <0, fail to write data into flash
 *         otherwise, succeed
 */
int tcm_write_flash_data(unsigned short start_address_in_blocks,
                         unsigned char *wr_data, int wr_data_bytes)
{
    int retval = 0;
    unsigned char *cmd_packet = NULL;
    unsigned char *data_buf = NULL;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!wr_data) {
        printf_e("%s error: invalid parameter, wr_data is null\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, wr_data is null\n", __func__);
        add_error_msg(err);
#endif
        return -EIO;
    }

    cmd_packet = calloc((size_t)(wr_data_bytes + 5), sizeof(unsigned char));
    if (!cmd_packet) {
        printf_e("%s error: can't allocate memory for command packet\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for command packet\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    cmd_packet[0] = CMD_WRITE_FLASH;
    cmd_packet[1] = (unsigned char)((wr_data_bytes + 2) & 0xff);
    cmd_packet[2] = (unsigned char)((wr_data_bytes + 2) >> 8);
    cmd_packet[3] = (unsigned char)(start_address_in_blocks & 0xff);
    cmd_packet[4] = (unsigned char)(start_address_in_blocks >> 8);
    memcpy(&cmd_packet[5], wr_data, (size_t)wr_data_bytes);

    /* send command to write data into flash */
    retval = tcm_write_message(cmd_packet, (unsigned int)(wr_data_bytes + 5));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_WRITE_FLASH 0x%02x 0x%02x 0x%02x 0x%02x\n",
                 __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3], cmd_packet[4]);
        retval = -EINVAL;
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send command, CMD_WRITE_FLASH 0x%02x 0x%02x 0x%02x 0x%02x\n",
                __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3], cmd_packet[4]);
        add_error_msg(err);
#endif
        goto exit;
    }

    usleep(TCM_WRITE_FLASH_DELAY_MS * 1000);

    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: fail to write data to the flash\n", __func__);
        retval = -EIO;
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to write data to the flash\n", __func__);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* retrieve the response payload */
    if (retval > 0) {
        data_buf = calloc((size_t)(retval + 3), sizeof(unsigned char));
        if (!data_buf) {
            printf_e("%s error: fail to allocate data_buf\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to allocate data_buf\n", __func__);
            add_error_msg(err);
#endif
            retval = -EIO;
            goto exit;
        }

        retval = tcm_get_payload(data_buf, (unsigned int)retval);
        if (retval < 0) {
            printf_e("%s error: fail to get the payload\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to get the payload\n", __func__);
            add_error_msg(err);
#endif
            retval = -EINVAL;
            goto exit;
        }
    }

    printf_i("%s info: done", __func__);

exit:
    if (cmd_packet)
        free(cmd_packet);
    if (data_buf)
        free(data_buf);

    return retval;
}
/*
 * Function:  tcm_read_flash_data
 * --------------------
 * function to read UPP data
 *
 * return: <0, fail to read the upp data
 *         otherwise, succeed
 */
int tcm_read_flash_data(unsigned short start_address_in_blocks,
                        unsigned char *rd_data, int rd_data_bytes)
{
    int retval = 0;
    struct tcm_boot_info* boot_info = &g_tcm_handler.boot_info_report;
    unsigned short block_size_words = boot_info->write_block_size_words;
    unsigned int address_in_words;
    unsigned int length_in_words = rd_data_bytes / sizeof(short);
    unsigned char cmd_packet[9] = {0};
    unsigned char *temp_buf;
    int payload_len;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!rd_data) {
        printf_e("%s error: invalid parameter, rd_data is null\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid parameter, rd_data is null\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    address_in_words = start_address_in_blocks * block_size_words;

    cmd_packet[0] = CMD_READ_FLASH;
    cmd_packet[1] = 6;
    cmd_packet[2] = 0;
    cmd_packet[3] = (unsigned char)(address_in_words & 0xff);
    cmd_packet[4] = (unsigned char)((address_in_words >> 8) & 0xff);
    cmd_packet[5] = (unsigned char)((address_in_words >> 16) & 0xff);
    cmd_packet[6] = (unsigned char)(address_in_words >> 24);
    cmd_packet[7] = (unsigned char)length_in_words;
    cmd_packet[8] = (unsigned char)(length_in_words >> 8);

    /* send command to retrieve data from flash */
    retval = tcm_write_message(cmd_packet, sizeof(cmd_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_READ_FLASH 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                 __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3], cmd_packet[4], cmd_packet[5],
                 cmd_packet[6], cmd_packet[7], cmd_packet[8]);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send command, CMD_READ_FLASH 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                __func__, cmd_packet[1], cmd_packet[2], cmd_packet[3], cmd_packet[4], cmd_packet[5],
                cmd_packet[6], cmd_packet[7], cmd_packet[8]);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* wait for the command completion */
    payload_len = tcm_wait_for_command_ready();
    if ( payload_len < 0) {
        printf_e("%s error: fail to read upp data\n", __func__);
        retval = -EIO;
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to read upp data  \n", __func__);
        add_error_msg(err);
#endif
        return retval;
    }

    temp_buf = calloc((size_t)(payload_len + 3), sizeof(unsigned char));
    if (!temp_buf) {
        printf_e("%s error: fail to allocate temp_buf\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to allocate temp_buf\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        goto exit;
    }

    /* read upp data */
    retval = tcm_get_payload(temp_buf, payload_len);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload \n", __func__);
        add_error_msg(err);
#endif
        return retval;
    }

    rd_data_bytes = (rd_data_bytes < payload_len)? rd_data_bytes : payload_len;
    memcpy(rd_data, temp_buf, (size_t)rd_data_bytes);

    printf_i("%s info: done", __func__);

exit:
    if (temp_buf)
        free(temp_buf);

    return retval;
}

