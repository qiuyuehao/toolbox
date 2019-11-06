/*
 * Copyright (C) 2017 Synaptics Incorporated. All rights reserved.
 *
 * This file contains information that is proprietary to Synaptics Incorporated
 * ("Synaptics"). The holder of this file shall treat all information contained
 * herein as confidential, shall use the information only for its intended
 * purpose, and shall not duplicate, disclose, or disseminate any of this
 * information in any manner unless Synaptics has otherwise provided express,
 * written permission.
 *
 * Use of the materials may require a license of intellectual property from a
 * third party or from Synaptics. Receipt or possession of this file conveys no
 * express or implied licenses to any intellectual property rights belonging to
 * Synaptics.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND SYNAPTICS
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL SYNAPTICS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>

#include "native-lib.h"
#include "tcm_control.h"

#define DEVICE_IOC_MAGIC 's'
#define DEVICE_IOC_RESET _IO(DEVICE_IOC_MAGIC, 0) /* 0x00007000 */
#define DEVICE_IOC_IRQ _IOW(DEVICE_IOC_MAGIC, 1, int) /* 0x40047001 */
#define DEVICE_IOC_RAW _IOW(DEVICE_IOC_MAGIC, 2, int) /* 0x40047002 */
#define DEVICE_IOC_CONCURRENT _IOW(DEVICE_IOC_MAGIC, 3, int) /* 0x40047303 */

#define MAX_I2C_READ_BYTES (32)


static int g_tcm_fd;
static int g_tcm_asic_id;

static short g_tcm_gear_enabled_table;
static short g_tcm_finger_threshold;

static short UC2SHORT(unsigned char lsb, unsigned char msb) {
    return (msb << 8) | lsb;
}

/*
 * Function:  tcm_enable_raw_mode
 * --------------------
 * enable/disable the raw mode
 *     - raw mode : driver will bypass all interactivity and not handle interrupt
 *     - irq mode : driver will behave normally
 *
 * return: <0 - fail to change the driver into raw mode
 *         otherwise, succeed
 */
int tcm_enable_raw_mode(bool enable)
{
    int retval = 0;

    if(g_tcm_fd < 0) {
        printf_e("%s error: file descriptor is initialized (%d)\n", __func__, g_tcm_fd);
        return (-EINVAL);
    }

    if (enable) {
        retval = ioctl(g_tcm_fd, DEVICE_IOC_RAW, true);
        if (retval < 0) {
            printf_e("fail to enable the IOC_RAW ioctl command to /dev/tcm0\n");
            retval = -EINVAL;
            goto exit;
        }
        retval = ioctl(g_tcm_fd, DEVICE_IOC_IRQ, false);
        if (retval < 0) {
            printf_e("fail to disable the IOC_IRQ ioctl command to /dev/tcm0\n");
            retval = -EINVAL;
            goto exit;
        }
        printf_i("%s info: set to RAW mode\n", __func__);
    }
    else {
        retval = ioctl(g_tcm_fd, DEVICE_IOC_RAW, false);
        if (retval < 0) {
            printf_e("fail to disable the IOC_RAW ioctl command to /dev/tcm0\n");
            retval = -EINVAL;
            goto exit;
        }
        retval = ioctl(g_tcm_fd, DEVICE_IOC_IRQ, true);
        if (retval < 0) {
            printf_e("fail to enable the IOC_IRQ ioctl command to /dev/tcm0\n");
            retval = -EINVAL;
            goto exit;
        }
        printf_i("%s info: set to IRQ mode\n", __func__);
    }
    exit:
    return retval;
}

/*
 * Function:  tcm_open_dev
 * --------------------
 * initialize the tcm access function and
 * get the file descriptor
 *
 * return: 0  - tcm_file_descriptor already exist
 *         <0 - fail to open device
 *         otherwise, succeed
 */
int tcm_open_dev(const char* tcm_path)
{
    if (g_tcm_fd > 0)
        printf_e("%s: g_tcm_fd (%d) is not 0\n", __func__, g_tcm_fd);

    g_tcm_fd = open(tcm_path, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (g_tcm_fd < 0) {
        printf_e("%s: failed to open %s, (err: %s)\n",
                 __func__, tcm_path, strerror(errno));
        sprintf(err_msg_out, "%s: failed to open %s, (err: %s)\n",
                __func__, tcm_path, strerror(errno));
        return -EIO;
    }

    printf_i("%s: open %s successfully (rmi_fd = %d)\n", __func__, tcm_path, g_tcm_fd);

    /* to set the tcm device into raw mode */
   if (tcm_enable_raw_mode(true) < 0) {
       printf_e("%s error: fail to config the tcm device to raw mode.\n", __func__);
       sprintf(err_msg_out + strlen((const char*)err_msg_out),
                    "%s: fail to config the tcm device to raw mode\n", __func__);
       return -EIO;
   }

    return g_tcm_fd;
}

/*
 * Function:  tcm_close_dev
 * --------------------
 * close the file descriptor
 *
 * return: n\a
 *
 */
void tcm_close_dev(const char* tcm_path)
{
    /* to set the tcm device into raw mode */
    if (tcm_enable_raw_mode(false) < 0) {
        printf_e("%s error: fail to config the tcm device to irq mode.\n", __func__);
        sprintf(err_msg_out + strlen((const char*)err_msg_out),
                "%s: fail to config the tcm device to irq mode\n", __func__);
    }

    close(g_tcm_fd);
    printf_i("%s: close %s\n", __FUNCTION__, tcm_path);

    g_tcm_fd = 0;
}

/*
 * Function:  tcm_read_message
 * --------------------
 * read the data from the tcm device
 *
 * parameter
 *  p_rd_data: buffer of data reading
 *  bytes_to_read: number of bytes for reading
 *
 * return: <0 - fail to read the i2c data
 *         otherwise, return the number of bytes as p_rd_data
 */
int tcm_read_message(unsigned char *p_rd_data, unsigned int bytes_to_read)
{
    int retval = 0;

    if (!p_rd_data)  {
        printf_e("%s error: p_rd_data buffer is null\n", __func__);
        retval = -EINVAL;
        return retval;
    }

    if(g_tcm_fd < 0) {
        printf_e("%s error: file descriptor is initialized (%d)\n", __func__, g_tcm_fd);
        return (-EINVAL);
    }

    retval = (int) read(g_tcm_fd, p_rd_data, bytes_to_read);
    if (retval < 0)  {
        printf_e("%s error: fail to read data from %s, bytes_to_read= %d (retval = %d)\n",
               __func__, g_dev_node, bytes_to_read, retval);
        retval = -EIO;
        return retval;
    }

    return retval;
}

/*
 * Function:  tcm_write_message
 * --------------------
 * write data to the tcm device
 *
 * parameter
 *  p_wr_data: data for writing
 *  bytes_to_write: number of bytes for writing
 *
 * return: <0 - fail to write the i2c data
 *         otherwise, number of writing bytes
 */
int tcm_write_message(unsigned char *p_wr_data, unsigned int  bytes_to_write)
{
    int retval = 0;
    int i;

    if (!p_wr_data)  {
        printf_e("%s error: p_wr_data can't be null\n", __func__);
        retval = -EINVAL;
        return retval;
    }

    if(g_tcm_fd < 0) {
        printf_e("%s error: file descriptor is initialized (%d)\n", __FUNCTION__, g_tcm_fd);
        return (-EINVAL);
    }

    retval = (int) write(g_tcm_fd, p_wr_data, bytes_to_write);
    if (retval < 0)  {
        printf_e("%s error: fail to write data to %s, bytes_to_write= %d, data: ",
               __func__, g_dev_node, bytes_to_write);
        for(i = 0; i<bytes_to_write; i++)
            printf_e("0x%x ", p_wr_data[i]);
        printf_e(")\n");
        printf_e("%s error: retval= %d\n", __func__, retval);

        retval = -EIO;
        return retval;
    }

    return retval;
}
/*
 * Function:  tcm_drop_one_frame
 * --------------------
 * function to drop one tcm report when waiting the command response
 *
 * return: <0 - error when i2c read operation
 *         otherwise, succeed
 */
int tcm_drop_one_frame(int payload_size) {
    int retval = 0;
    unsigned char *buf;

    buf = malloc(sizeof(unsigned char) * (payload_size + 2));

    retval = tcm_read_message(&buf[0], (unsigned int)payload_size);
    if (retval < 0) {
        printf_e("%s error: fail to read data (remaining = %d)\n", __func__, payload_size);
        retval = -EINVAL;
    }

    free(buf);
    return retval;
}

/*
 * Function:  tcm_wait_for_command_ready
 * --------------------
 * function to wait for the command completion
 *
 * return: <0 - error/out
 *         otherwise, return the payload length
 */
int tcm_wait_for_command_ready(void)
{
    int retval = 0;
    bool commadn_is_ready = false;
    int timeout_cnt = 0;
    int size = sizeof(struct tcm_message_header);
    unsigned char buf[4] = {0};
    struct tcm_message_header *header = NULL;
    int payload_size;

    header = (struct tcm_message_header *)buf;
    do {
        usleep(TCM_POLLING_DELAY_MS * 1000);

        retval = tcm_read_message(buf, (unsigned int) size);
        if (retval < 0) {
            printf_e("%s error: fail to read header from tcm device\n", __func__);
            sprintf(err_msg_out, "%s: error: fail to read header from tcm device\n",
                    __func__);
            return retval;
        }

        if ( 0xA5 == header->marker) {
            if ( STATUS_OK == header->code) {
                commadn_is_ready = true;
            }
            /* if the return code belongs to report, drop this report */
            else if ((header->code & 0x10) == 0x10) {
                payload_size = header->length[0] | (header->length[1] << 8);
                printf_i("%s info: drop one tcm report (report: 0x%x, payload:%d)\n",
                         __func__, header->code, payload_size);
                tcm_drop_one_frame(payload_size);
            }
            /* if the return code belongs to report, drop this report */
            else if (STATUS_COMMAND_NOT_IMPLEMENTED == header->code) {
                printf_e("%s error: command is not implemented\n", __func__);
                sprintf(err_msg_out, "%s: error: command is not implemented, STATUS_COMMAND_NOT_IMPLEMENTED\n",
                        __func__);
                return (-ENOSYS);
            }
        }
        timeout_cnt += 1;

    } while( (!commadn_is_ready) && (timeout_cnt < TCM_POLLING_TIMOUT) );

    if (!commadn_is_ready) {
        printf_e("%s error: command timeout\n", __func__);
        sprintf(err_msg_out, "%s: error: command timeout\n", __func__);
        return (-ETIMEDOUT);
    }

    size = UC2SHORT(header->length[0], header->length[1]);
    return size;
}


/*
 * Function:  tcm_gat_payload
 * --------------------
 * read the data payload from the tcm device
 *
 * parameter
 *  p_rd_data: buffer of data reading
 *  payload: payload size in byte
 *
 * return: <0 - fail to save payload data
 *         otherwise, succeed
 */
int tcm_gat_payload(unsigned char *p_rd_data, int payload_size)
{
    int retval = 0;
    unsigned char *buf = NULL;
    int size = payload_size + 3;

    if (!p_rd_data) {
        printf_e("%s: p_rd_data is NULL\n", __func__);
        retval = -EINVAL;
        goto exit;
    }

    /* each i2c read contains 2-byte header + payload_size + 1-byte ending */
    buf = malloc(sizeof(unsigned char) * size);
    if (!buf) {
        printf_e("%s error: can't allocate memory for buf\n", __func__);
        retval = -ENOMEM;
        goto exit;
    }

    retval = tcm_read_message(buf, (unsigned int)size);
    if (retval < 0) {
        printf_e("%s error: fail to read data (remaining = %d)\n", __func__, payload_size);
        retval = -EINVAL;
        goto exit;
    }

    /* to check header */
    if (0xA5 != buf[0]) {
        printf_e("%s error: unknown data header (byte 0: 0x%x, byte 1: 0x%x)\n",
                 __func__, buf[0], buf[1]);
        retval = -EINVAL;
        goto exit;
    }
    else {
        if (0x5A != buf[size-1])
            printf_e("%s warning: payload packet is not ended (last byte: 0x%x)\n",
                     __func__, buf[size-1]);

        if (STATUS_CONTINUED_READ != buf[1])
            printf_e("%s warning: not STATUS_CONTINUED_READ (byte 0: 0x%x, byte 1: 0x%x)\n",
                     __func__, buf[0], buf[1]);
    }
    /* copy to the input buffer */
    memcpy(p_rd_data, &buf[2], (size_t)payload_size); /* skip the first 2 bytes */

exit:
    if(buf)
        free(buf);

    return retval;
}

/*
 * Function:  tcm_get_identify_info
 * --------------------
 * to get identify information
 *
 * return: <0 - fail to get identification info
 *         otherwise, succeed
 */
int tcm_get_identify_info()
{
    int retval = 0;
    unsigned char command = CMD_IDENTIFY;
    unsigned char *data_buf;
    int length = sizeof(struct tcm_message_header) + sizeof(struct tcm_identify_report);
    struct tcm_identify_report *packet;

    memset(&g_tcm_identify_report, 0x00, sizeof(struct tcm_identify_report));

    data_buf = malloc(sizeof(unsigned char) * length);
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __FUNCTION__);
        retval = -ENOMEM;
        goto exit;
    }
    memset(data_buf, 0x00, (size_t) length);
    /* send identify command to tcm device */
    retval = tcm_write_message(&command, sizeof(command));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_IDENTIFY\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }
    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: timeout while running the command, CMD_IDENTIFY\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }
    printf_i("%s info: CMD_IDENTIFY is completed.", __FUNCTION__);

    /* to get the identify information                         */
    /*   packet header : 2 bytes (0xA5 + STATUS_CONTINUED_READ)*/
    /*   identify info : 24 bytes                              */
    length = retval + 2;
    retval = tcm_read_message(data_buf, (unsigned int) length);
    if (retval < 0) {
        printf_e("%s error: fail to read response\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }

    if (STATUS_CONTINUED_READ == data_buf[1]) {
        packet = (struct tcm_identify_report *)(data_buf + 2);
        memcpy(&g_tcm_identify_report, packet, sizeof(struct tcm_identify_report));
    }
    else {
        printf_e("%s error: unknown read transaction (0x%x 0x%x)\n", __FUNCTION__, data_buf[0], data_buf[1]);
        retval = -EINVAL;
        goto exit;
    }

    printf_i("[ app identify ]\n");
    printf_i(" packet version  = %d\n", packet->version);
    if (MODE_APPLICATION == packet->mode) {
        printf_i(" firmware mode  = application (0x%x)\n", packet->mode);
    }
    else if (MODE_BOOTLOADER == packet->mode) {
        printf_i(" firmware mode  = bootloader (0x%x)\n", packet->mode);
    }
    else if (MODE_BOOTLOADER_TDDI == packet->mode) {
        printf_i(" firmware mode  = tddi bootloader (0x%x)\n", packet->mode);
    }
    else {
        printf_i(" firmware mode  = unknown (0x%x)\n", packet->mode);
    }
    printf_i(" part number  = %s\n", packet->part_number);
    printf_i(" build id\t\t= %d\n", ((unsigned int)packet->build_id[0]) +
                                     ((unsigned int)packet->build_id[1] * 0x100) +
                                     ((unsigned int)packet->build_id[2] * 0x10000) +
                                     ((unsigned int)packet->build_id[3] * 0x1000000) );
    printf_i(" max write size  = %d\n", packet->max_write_size[0]|packet->max_write_size[1]<<8);
    printf_i("\n");

exit:
    if (data_buf)
        free(data_buf);

    return retval;
}

/*
 * Function:  tcm_get_app_info
 * --------------------
  * to get the application information
 *
 * return: <0 - fail to get application info
 *         otherwise, succeed
 */
int tcm_get_app_info()
{
    int retval = 0;
    unsigned char command = CMD_GET_APPLICATION_INFO;
    unsigned char *data_buf;
    int length = sizeof(struct tcm_message_header) + sizeof(struct tcm_app_info);
    short app_status;
    struct tcm_app_info *packet;

    memset(&g_tcm_app_info_report, 0x00, sizeof(struct tcm_app_info));

    data_buf = malloc(sizeof(unsigned char) * length);
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __FUNCTION__);
        retval = -ENOMEM;
        goto exit;
    }
    memset(data_buf, 0x00, (size_t) length);
    /* send command to tcm device */
    retval = tcm_write_message(&command, sizeof(command));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_GET_APPLICATION_INFO\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }
    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: timeout while running the command, CMD_GET_APPLICATION_INFO\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }
    printf_i("%s info: CMD_GET_APPLICATION_INFO is completed.", __FUNCTION__);

    /* to get the application information                      */
    /*   packet header : 2 bytes (0xA5 + STATUS_CONTINUED_READ)*/
    /*   application info : 58 bytes                           */
    length = retval + 2;
    retval = tcm_read_message(data_buf, (unsigned int) length);
    if (retval < 0) {
        printf_e("%s error: fail to read response\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }

    if (STATUS_CONTINUED_READ == data_buf[1]) {
        packet = (struct tcm_app_info *)(data_buf + 2);
        memcpy(&g_tcm_app_info_report, packet, sizeof(struct tcm_app_info));
    }
    else {
        printf_e("%s error: unknown read transaction (0x%x 0x%x)\n", __func__, data_buf[0], data_buf[1]);
        retval = -EINVAL;
        goto exit;
    }

    printf_i("[ application info ]\n");
    printf_i(" application version  = %d\n", UC2SHORT(packet->version[0], packet->version[1]));
    app_status = UC2SHORT(packet->status[0], packet->status[1]);
    if ( app_status == 0x0000 ) {
        printf_i(" application status  = OK\n");
    }
    else {
        printf_i(" application status  = 0x%x\n", UC2SHORT(packet->status[0], packet->status[1]));
    }
    printf_i(" app config size  = %d\n", UC2SHORT(packet->app_config_size[0], packet->app_config_size[1]));
    printf_i(" max touch config size  = %d\n", UC2SHORT(packet->max_touch_report_config_size[0], packet->max_touch_report_config_size[1]));
    printf_i(" max touch payload size  = %d\n", UC2SHORT(packet->max_touch_report_payload_size[0], packet->max_touch_report_payload_size[1]));
    printf_i(" max. x coordinate  = %d\n", UC2SHORT(packet->max_x[0], packet->max_x[1]));
    printf_i(" max. y coordinate  = %d\n", UC2SHORT(packet->max_y[0], packet->max_y[1]));
    printf_i(" max. num of reported obj = %d\n", UC2SHORT(packet->max_objects[0], packet->max_objects[1]));
    printf_i(" num of image rows  = %d\n", UC2SHORT(packet->num_of_image_rows[0], packet->num_of_image_rows[1]));
    printf_i(" num of image columns  = %d\n", UC2SHORT(packet->num_of_image_cols[0], packet->num_of_image_cols[1]));
    printf_i(" num of buttons  = %d\n", UC2SHORT(packet->num_of_buttons[0], packet->num_of_buttons[1]));
    printf_i(" Config ID  = %x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x\n",
                          packet->customer_config_id[0], packet->customer_config_id[1], packet->customer_config_id[2], packet->customer_config_id[3], packet->customer_config_id[4], packet->customer_config_id[5],
                          packet->customer_config_id[6], packet->customer_config_id[7], packet->customer_config_id[8], packet->customer_config_id[9], packet->customer_config_id[10], packet->customer_config_id[11],
                          packet->customer_config_id[12], packet->customer_config_id[13], packet->customer_config_id[14], packet->customer_config_id[15]);
    printf_i("\n");

exit:
    if (data_buf)
        free(data_buf);

    return retval;
}

/*
 * Function:  tcm_do_identify
 * --------------------
 * run identify and get app info
 *
 * return: <0 - fail
 *         otherwise, succeed
 */
int tcm_do_identify()
{
    int retval;

    retval = tcm_get_identify_info();
    if (retval < 0) {
        printf_e("%s error: fail on tcm identify\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }

    retval = tcm_get_app_info();
    if (retval < 0) {
        printf_e("%s error: fail to get tcm app info\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }

    retval = tcm_get_enabled_gear_table();
    if (retval < 0) {
        printf_e("%s error: fail to get enabled gear table\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }

    retval = tcm_get_normal_finger_threshold();
    if (retval < 0) {
        printf_e("%s error: fail to get normal finger threshold\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }
exit:
    return retval;
}
/*
 * Function:  tcm_get_asic_id
 * --------------------
 * return the asic type
 */
int tcm_get_asic_id(void)
{
    int id;
    char asic_type[16] = {0};

    sprintf(asic_type, "%c%c%c%c",
            g_tcm_identify_report.part_number[2], g_tcm_identify_report.part_number[3],
            g_tcm_identify_report.part_number[4], g_tcm_identify_report.part_number[5]);
    sscanf(asic_type, "%d", &id);

    g_tcm_asic_id = id;

    return id;
}

/*
 * Function:  tcm_get_build_id
 * --------------------
 * return the firmware build id, which is parsed from the PDT
 */
int tcm_get_build_id(void)
{
    int id = (g_tcm_identify_report.build_id[0]) +
             (g_tcm_identify_report.build_id[1] * 0x100) +
             (g_tcm_identify_report.build_id[2] * 0x10000) +
             (g_tcm_identify_report.build_id[3] * 0x1000000);

    return id;
}

/*
 * Function:  tcm_get_image_cols
 * --------------------
 * return number of columns
 */
int tcm_get_image_cols(void)
{
    return (UC2SHORT(g_tcm_app_info_report.num_of_image_cols[0], g_tcm_app_info_report.num_of_image_cols[1]));
}
/*
 * Function:  tcm_get_image_rows
 * --------------------
 * return number of rows
 */
int tcm_get_image_rows(void)
{
    return (UC2SHORT(g_tcm_app_info_report.num_of_image_rows[0], g_tcm_app_info_report.num_of_image_rows[1]));
}

/*
 * Function:  tcm_enable_report
 * --------------------
 * to enable/disable the report output
 *
 * return: <0 - fail to enable/disable the report output
 *         otherwise, succeed
 */
int tcm_enable_report(bool enable, enum tcm_report_code report_code)
{
    int retval = 0;
    unsigned char command_packet[4] = {0, 0, 0, 0};

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
        retval = -EINVAL;
        return retval;
    }

    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf("%s error: fail to %s the report (report:0x%x)\n", __func__, (enable)?"enable":"disable", (int)report_code);
        retval = -EIO;
        return retval;
    }
    printf_i("%s info: %s is completed. (report:0x%x)", __func__, (enable)?"CMD_ENABLE_REPORT":"CMD_DISABLE_REPORT", (int)report_code);

    return retval;
}


/*
 * Function:  tcm_set_dynamic_config
 * --------------------
 * helper to set the dynamic config
 *
 * parameter
 *  id: the field_id to be configured
 *  value: value to be written
 *
 * return: <0 - fail to set the dynamic config
 *         otherwise, succeed
 */
int tcm_set_dynamic_config(unsigned char field_id, unsigned short value)
{
    int retval = 0;
    unsigned char command_packet[6] = {0, 0, 0, 0, 0, 0};

    /* command code */
    command_packet[0] = CMD_SET_DYNAMIC_CONFIG;

    /* command length */
    /* length = 3 */
    command_packet[1] = 0x03;
    command_packet[2] = 0x00;
    /* command payload, 3 byte*/
    /* byte[3] = report type */
    command_packet[3] = field_id;
    command_packet[4] = (unsigned char)value;
    command_packet[5] = (unsigned char)(value >> 8);
    /* send command to tcm device */
    retval = tcm_write_message(command_packet, sizeof(command_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_SET_DYNAMIC_CONFIG 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__,
                 command_packet[1], command_packet[2], command_packet[3], command_packet[4], command_packet[5]);
        retval = -EINVAL;
        return retval;
    }
    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: fail to get the response of command, (0x%x 0x%x 0x%x 0x%x) \n",
                 __func__, command_packet[0], command_packet[1], command_packet[2], command_packet[3]);
        retval = -EINVAL;
        return retval;
    }
    printf_i("%s info: CMD_SET_DYNAMIC_CONFIG is completed. (field_id:0x%x) (value:%d)", __func__, (int)field_id, value);

    return retval;
}
/*
 * Function:  tcm_get_dynamic_config
 * --------------------
 * helper to get the value through dynamic config
 *
 * parameter
 *  id: the field_id to be configured
 *  value: read value
 *
 * return: <0 - fail to get the dynamic config
 *         otherwise, succeed
 */
int tcm_get_dynamic_config(unsigned char field_id, short* value)
{
    int retval = 0;
    unsigned char command_packet[4] = {0, 0, 0, 0};
    unsigned char read_buf[5] = {0, 0, 0, 0, 0};

    /* command code */
    command_packet[0] = CMD_GET_DYNAMIC_CONFIG;

    /* command length */
    /* length = 3 */
    command_packet[1] = 0x01;
    command_packet[2] = 0x00;
    /* command payload, 3 byte*/
    /* byte[3] = report type */
    command_packet[3] = field_id;
    /* send command to tcm device */
    retval = tcm_write_message(command_packet, sizeof(command_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_GET_DYNAMIC_CONFIG 0x%x 0x%x 0x%x\n", __func__,
                 command_packet[1], command_packet[2], command_packet[3]);
        retval = -EINVAL;
        return retval;
    }
    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: fail to get the response of command, (0x%x 0x%x 0x%x 0x%x) \n",
                 __func__, command_packet[0], command_packet[1], command_packet[2], command_packet[3]);
        retval = -EINVAL;
        return retval;
    }
    printf_i("%s info: CMD_GET_DYNAMIC_CONFIG is completed. (field_id:0x%x)", __func__, (int)field_id);

    retval = tcm_read_message(read_buf, sizeof(read_buf));
    if (retval < 0) {
        printf_e("%s error: fail to read data\n", __func__);
        sprintf(err_msg_out, "%s: fail to read data\n", __func__);
        retval = -EINVAL;
        return retval;
    }

    *value = UC2SHORT(read_buf[2], read_buf[3]);

    return retval;
}
/*
 * Function:  tcm_set_no_sleep
 * --------------------
 * helper to config the device as no_sleep mode
 *
 * parameter
 *    state - 1: enable
 *            0: disable
 *
 * return: <0 - fail to config into no_sleep mode
 *         otherwise, succeed
 */
int tcm_set_no_sleep(int state)
{
    unsigned short value = (0 == state)?(unsigned short)0:(unsigned short)1;
    int retval = tcm_set_dynamic_config(DC_NO_DOZE, value);
    if (retval < 0) {
        printf_e("%s error: fail to config with DC_NO_DOZE (value=%d)", __func__, value);
        retval = -EINVAL;
        return retval;
    }
    return 0;
}
/*
 * Function:  tcm_get_enabled_gear_table
 * --------------------
 * helper function to get the enabled gear table
 */
int tcm_get_enabled_gear_table()
{
    int retval = 0;

    retval = tcm_get_dynamic_config(DC_GET_ENABLED_FREQ, &g_tcm_gear_enabled_table);
    if ( retval < 0) {
        printf_e("%s error: fail to get the gear table with DC_GET_ENABLED_FREQ\n", __func__);
        printf_e("%s warning: may not support DC_GET_ENABLED_FREQ, set to 1\n",
                 __func__);
        g_tcm_gear_enabled_table = 0x01;
    }
    else {
        if (g_tcm_gear_enabled_table == 0x00) {
            printf_e("%s warning: may not support DC_GET_ENABLED_FREQ, set to 1\n",
                     __func__);
            g_tcm_gear_enabled_table = 0x01;
        }
    }

    printf_i("%s info: g_tcm_gear_enabled_table = 0x%x\n",
             __func__, g_tcm_gear_enabled_table);

    return g_tcm_gear_enabled_table;
}
/*
 * Function:  tcm_get_normal_finger_threshold
 * --------------------
 * helper function to get the normal finger level
 */
int tcm_get_normal_finger_threshold()
{
    int retval = 0;

    retval = tcm_get_dynamic_config(DC_GET_FINGER_THRESHOLD, &g_tcm_finger_threshold);
    if ( retval < 0) {
        printf_e("%s error: fail to get the gear table with DC_GET_FINGER_THRESHOLD\n", __func__);
        printf_e("%s warning: may not support DC_GET_FINGER_THRESHOLD, set to 200\n",
                 __func__);
        g_tcm_finger_threshold = 200;
        retval = 0;
    }
    printf_i("%s info: g_tcm_finger_threshold = 0x%x\n", __func__, g_tcm_finger_threshold);

    if (g_tcm_finger_threshold > 1000)
        g_tcm_finger_threshold = 200;

    return retval;
}
/*
 * Function:  tcm_get_gear_info
 * --------------------
 * helper function to get the gear information
 */
void tcm_get_gear_info(char *p_info)
{
    int i;
    for (i = 0; i < 16; i++) {
        if (CHECK_BIT(g_tcm_gear_enabled_table, i) != 0x00) {
            sprintf(p_info, "%sGear %2d\n", p_info, i);
        }
    }
}
/*
 * Function:  tcm_get_finger_cap
 * --------------------
 * helper function to get the gear information
 */
int tcm_get_finger_cap()
{
    return g_tcm_finger_threshold;
}
/*
 * Function:  tcm_request_freq_gear
 * --------------------
 * helper function to get the gear information
 */
int tcm_request_freq_gear(int gear_idx)
{
    int retval;
    unsigned short value = 0x01;

    /* to inhibit frequency shift */
    retval = tcm_set_dynamic_config(DC_INHIBIT_FREQUENCY_SHIFT, value);
    if (retval < 0) {
        printf_e("%s error: fail to config with DC_INHIBIT_FREQUENCY_SHIFT (value=%d)", __func__, value);
        retval = -EINVAL;
        return retval;
    }
    /* to change to requested freq gear */
    retval = tcm_set_dynamic_config(DC_REQUESTED_FREQUENCY, (unsigned short)gear_idx);
    if (retval < 0) {
        printf_e("%s error: fail to config with DC_REQUESTED_FREQUENCY (gear_idx=%d)", __func__, gear_idx);
        retval = -EINVAL;
        return retval;
    }

    printf_i("%s info: change to frequency gear-%d", __func__, gear_idx);
    return 1;
}
/*
 * Function:  tcm_read_report_image
 * --------------------
 * to read the report data from tcm firmware
 * this function should be called after enable_tcm_report
 *
 * return: <0 - fail to read a tcm report
 *         otherwise, succeed
 */
int tcm_read_report_image(short *p_image, unsigned char type, int image_col, int image_row)
{
    int retval = 0;
    short *p_data_16;
    unsigned char *data_buf = NULL;
    int report_size = 2 * image_col * image_row;
    int report_timeout = 0;
    int report_payload = 0;
    int i, j, offset;
    int rows = (g_tcm_app_info_report.num_of_image_rows[0] |
                g_tcm_app_info_report.num_of_image_rows[1] << 8);
    int cols = (g_tcm_app_info_report.num_of_image_cols[0] |
                g_tcm_app_info_report.num_of_image_cols[1] << 8);


    if (!p_image) {
        printf_e("%s: p_image is NULL\n", __FUNCTION__);
        sprintf(err_msg_out, "%s: p_image is NULL.\n", __FUNCTION__);
        retval = -EINVAL;
        goto exit;
    }
    if ((0 == image_col) || (0 == image_row)) {
        printf_e("%s: invalid parameter (row,col) = (%d, %d)\n", __FUNCTION__, image_col, image_row);
        sprintf(err_msg_out, "%s: invalid parameter (row,col) = (%d, %d)\n", __FUNCTION__, image_col, image_row);
        retval = -EINVAL;
        goto exit;
    }
    /* create a local buffer to fill the report data */
    data_buf = malloc(sizeof(unsigned char) * report_size);
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __FUNCTION__);
        sprintf(err_msg_out, "%s: can't allocate memory for data_buf\n", __FUNCTION__);
        retval = -ENOMEM;
        goto exit;
    }
    memset(data_buf, 0x00, (size_t)report_size);

    /* polling the interrupt to wait for the requested type */
    do {
        usleep(TCM_POLLING_DELAY_MS * 1000);
        /* check the header */
        retval = tcm_read_message(data_buf, sizeof(struct tcm_message_header));
        if (retval < 0) {
            printf_e("%s error: fail to read tcm message header\n", __func__);
            sprintf(err_msg_out + strlen(err_msg_out), "%s: fail to read tcm message header\n", __func__);
            retval = -EINVAL;
            goto exit;
        }
        if ( 0xA5 == data_buf[0]) {
            /* once get the acknowledge of requested report type */
            /* get the size of data payload  */
            if ( type == data_buf[1]) {
                report_payload = data_buf[2] | data_buf[3] << 8;
                break;
            }
        }
        report_timeout++ ;
    } while(report_timeout < TCM_POLLING_TIMOUT);

    if (report_timeout == TCM_POLLING_TIMOUT) {
        retval = -EINVAL;
        printf_e("%s error: read delta report timeout\n", __FUNCTION__);
        sprintf(err_msg_out + strlen(err_msg_out), "%s: read delta report timeout\n", __FUNCTION__);
        goto exit;
    }

    printf_i("%s info: delta report (row = %d, column = %d) (payload size = %d)\n",
             __FUNCTION__, image_row, image_col, report_payload);

    /* read report image */
    retval = tcm_gat_payload(data_buf, report_payload);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, report_payload);
        sprintf(err_msg_out + strlen(err_msg_out), "%s: fail to get data payload (size = %d)\n", __func__, report_payload);
        goto exit;
    }

    p_data_16 = (short *)&data_buf[0];
    for(i = 0; i < rows; i++) {
        for(j = 0; j < cols; j++) {

            if (cols > rows)
                offset = i*cols + j;
            else
                offset = j*rows + i;

            p_image[offset] = *p_data_16;

            p_data_16++;
        }
    }

exit:
    if(data_buf)
        free(data_buf);

    return retval;
}

/*
 * Function:  tcm_do_noise_test
 * --------------------
 * function to perform noise test in tcm
 */
bool tcm_do_noise_test(FILE *pfile, char *timestamp, int frame_id, int gear_idx)
{
    int retval = 0;
    short *rt_image;
    bool result = true;

    printf_i("%s: frame id = %d\n", __FUNCTION__, frame_id);

    /* create buffer to store the report data */
    int rows = tcm_get_image_rows();
    int cols = tcm_get_image_cols();
    rt_image = (short *) calloc((size_t) (rows * cols), sizeof(short));
    if(!rt_image) {
        printf_e("%s: fail to allocate the buffer for report image acquisition\n", __FUNCTION__);
        sprintf(err_msg_out, "%s: fail to allocate the buffer for report image acquisition\n", __FUNCTION__);
        result = false;
        goto write_file;
    }

    /* to get the delta report */
    retval = tcm_read_report_image(rt_image, REPORT_DELTA, cols, rows);
    if(retval < 0){
        printf_e("%s: fail to read delta report\n", __FUNCTION__);
        result = false;
        goto write_file;
    }


write_file:
/* write data to the log file */
    if (!pfile){
        printf_e("%s: pfile is null.\n", __FUNCTION__);
        result = false;
        goto exit_func;
    }

    fprintf(pfile, "%s\n", timestamp);
    fprintf(pfile, "frame id = %d\n", frame_id);
    fprintf(pfile, "frequency gear = Gear.%d\n", gear_idx);

    /* save error message to the log */
    if(retval < 0){
        fprintf(pfile, "%s",  err_msg_out);
        fprintf(pfile, "result = fail\n\n");
        result = false;

        if (pfile_fail_log) {
            fprintf(pfile_fail_log, "%s\n", timestamp);
            fprintf(pfile_fail_log, "frame id = %d\n", frame_id);
            fprintf(pfile_fail_log, "frequency gear = Gear.%d\n", gear_idx);
            fprintf(pfile_fail_log, "%s",  err_msg_out);
            fprintf(pfile_fail_log, "result = fail\n\n");
        }

        goto exit_func;
    }

    /* write testing data */
    short max_val, min_val;

    max_val = rt_image[0];
    min_val = max_val;

    for(int i = 0; i < cols; i++) {
        if (i == 0) {
            fprintf(pfile, "        ");
            for (int j = 0 ; j < rows; j ++)
                fprintf(pfile, "[R%2d]  ", j);
            fprintf(pfile, "\n");
        }
        fprintf(pfile, "[T%2d]: ", i);

        for(int j = 0; j < rows; j++) {
            fprintf(pfile, "%6d ", rt_image[i*rows+ j]);

            /* determine it pass or fail on this Tixel */
            if (rt_image[i*rows+ j] > g_threshold) {
                result = false;
            }

            max_val = (max_val > rt_image[i*rows+ j])? max_val : rt_image[i*rows+ j];
            min_val = (min_val < rt_image[i*rows+ j])? min_val : rt_image[i*rows+ j];
        }
        fprintf(pfile, "\n");
    }
    fprintf(pfile, "max. = %d, min. = %d\n", max_val, min_val);
    fprintf(pfile, "result = %s \n", (result)?"pass":"fail");
    fprintf(pfile, "\n");

    if (result == false) {
        if (pfile_fail_log) {
            fprintf(pfile_fail_log, "%s\n", timestamp);
            fprintf(pfile_fail_log, "frame id = %d\n", frame_id);
            fprintf(pfile_fail_log, "frequency gear = Gear.%d\n", gear_idx);

            for(int i = 0; i < cols; i++) {
                if (i == 0) {
                    fprintf(pfile_fail_log, "        ");
                    for (int j = 0 ; j < rows; j ++)
                        fprintf(pfile_fail_log, "[R%2d]  ", j);
                    fprintf(pfile_fail_log, "\n");
                }
                fprintf(pfile_fail_log, "[T%2d]: ", i);

                for(int j = 0; j < rows; j++) {
                    fprintf(pfile_fail_log, "%6d ", rt_image[i*rows+ j]);
                }
                fprintf(pfile_fail_log, "\n");
            }
            fprintf(pfile_fail_log, "max. = %d, min. = %d\n", max_val, min_val);
            fprintf(pfile_fail_log, "result = fail \n");
            fprintf(pfile_fail_log, "\n");
        }
    }
exit_func:
    free(rt_image);

    return result;
}

void tcm_write_log_header(FILE *pfile, int total_frames, int num_gears)
{
    long sec;
    time_t t_time_header;

    gettimeofday(&t_val_1, NULL);
    sec = t_val_1.tv_sec;
    t_time_header = (time_t)sec;

    fprintf(pfile, "Synaptics APK     recorded at %s\n\n", ctime(&t_time_header));
    fprintf(pfile, "Interface                : ToucnComm\n");
    fprintf(pfile, "Device ID                : %4d\n", g_tcm_asic_id);
    fprintf(pfile, "Firmware Packrat ID      : %7d\n", tcm_get_build_id());
    fprintf(pfile, "Data Type                : RT 2, delta image\n");
    fprintf(pfile, "Image Columns            : %d\n", tcm_get_image_cols());
    fprintf(pfile, "Image Rows               : %d\n", tcm_get_image_rows());
    fprintf(pfile, "Total Frames Captured    : %d\n", total_frames);
    fprintf(pfile, "Number of Frames per Gear: %d\n", total_frames/num_gears);
    fprintf(pfile, "Number of Frequency Gears Enabled: %d\n", num_gears);
    fprintf(pfile, "\n");
    fprintf(pfile, "Delta Threshold          : %4d\n", g_threshold);
    fprintf(pfile, "\n\n");
}

