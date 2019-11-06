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
#include <sys/ioctl.h>

#include "syna_dev_manager.h"
#include "tcm_control.h"

#ifdef SAVE_ERR_MSG
#include "err_msg_ctrl.h"
#endif

/* character device node of tcm device */
#define TCM_DEV_PATH "/dev/tcm"

#define DEVICE_IOC_MAGIC 's'
#define DEVICE_IOC_RESET _IO(DEVICE_IOC_MAGIC, 0) /* 0x00007000 */
#define DEVICE_IOC_IRQ _IOW(DEVICE_IOC_MAGIC, 1, int) /* 0x40047001 */
#define DEVICE_IOC_RAW _IOW(DEVICE_IOC_MAGIC, 2, int) /* 0x40047002 */
#define DEVICE_IOC_CONCURRENT _IOW(DEVICE_IOC_MAGIC, 3, int) /* 0x40047303 */

int tcm_enable_raw_mode(bool enable);

/*
 * Function:  tcm_find_dev
 * --------------------
 * called by syna_dev_manager
 * look for the valid tcm device node
 *
 * return: true, tcm device is detected
 *         false, otherwise
 */
bool tcm_find_dev(char *dev_node)
{
    int i;
    struct stat st;
    bool is_found = false;

    /* scan the possible device node */
    for (i = 0; i < NUMBER_OF_SYNADEV_TO_SCAN; i++) {
        memset(dev_node, 0x00, MAX_STRING_LEN);
        /* retrieve the path of tcm character device */
        snprintf(dev_node, MAX_STRING_LEN, "%s%d", TCM_DEV_PATH, i);
        if (0 == stat(dev_node, &st)) {
            is_found = true;
            break;
        }
    }
    if (is_found) {
        printf_i("%s: synaptics tcm device node = %s\n",
                 __func__, dev_node);
    }

    return is_found;
}

/*
 * Function:  tcm_open_dev
 * --------------------
 * open the tcm device node
 *
 * return:  0, device is occupied
 *         <0, fail to open device
 *         otherwise, succeed
 */
int tcm_open_dev(const char *dev_node)
{
    int retval;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (g_dev_file_descriptor > 0) {
        printf_i("%s info: %s has been open\n", __func__, dev_node);
        return 0;
    }

    g_dev_file_descriptor = open(dev_node, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (g_dev_file_descriptor < 0) {
        printf_e("%s error: fail to open %s (err: %s)\n",
                 __func__, dev_node, strerror(errno));
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to open %s (err: %s)\n",
                __func__, dev_node, strerror(errno));
        add_error_msg(err);
#endif
        return -EIO;
    }

    printf_i("%s info: open %s (fd = %d)\n",
             __func__, dev_node, g_dev_file_descriptor);

    /* configure the tcm device into raw mode */
    retval = tcm_enable_raw_mode(true);
    if ( retval < 0) {
        printf_e("%s error: fail to config the tcm device to raw mode.\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to config the tcm device to raw mode\n", __func__);
        add_error_msg(err);
#endif
        close(g_dev_file_descriptor);
        return -EIO;
    }
    /* to get the touch config */
    retval = tcm_get_touch_config();
    if ( retval < 0) {
        printf_e("%s error: fail to get tcm touch config.\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get tcm touch config\n", __func__);
        add_error_msg(err);
#endif
        return -EIO;
    }

    return g_dev_file_descriptor;
}

/*
 * Function:  tcm_close_dev
 * --------------------
 * close the file descriptor
 *
 * return: file descriptor
 */
int tcm_close_dev(const char *dev_node)
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (g_dev_file_descriptor <= 0) {
        printf_i("%s info: %s has been closed\n", __func__, dev_node);
        return 0;
    }

    /* configure the tcm device into IRQ mode */
    retval = tcm_enable_raw_mode(false);
    if (retval < 0) {
        printf_e("%s error: fail to config the tcm device to irq mode.\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to config the tcm device to irq mode\n", __func__);
        add_error_msg(err);
#endif
    }

    /* do reset after IRQ mode is enabled,  */
    /* the reason is to let driver perform its initialization process */
    retval = ioctl(g_dev_file_descriptor, DEVICE_IOC_RESET);
    if (retval < 0) {
        printf_e("%s error: fail to send the DEVICE_IOC_RESET command to %s\n",
                 __func__, g_dev_node);
    }

    usleep(TCM_RESET_DELAY_MS * 1000);

    /* close the device node */
    close(g_dev_file_descriptor);

    g_dev_file_descriptor = 0;

    printf_i("%s info: close %s (fd = %d)\n",
             __func__, dev_node, g_dev_file_descriptor);

    return g_dev_file_descriptor;
}

/*
 * Function:  tcm_enable_raw_mode
 * --------------------
 * enable/disable the raw mode
 *     - raw mode : driver will bypass all interactivity and not handle interrupt
 *     - irq mode : driver will behave normally
 *
 * return: <0, fail to change the driver mode
 *         otherwise, succeed
 */
int tcm_enable_raw_mode(bool enable)
{
    int retval = 0;

    if(g_dev_file_descriptor < 0) {
        printf_e("%s error: file descriptor is initialized (%d)\n",
                 __func__, g_dev_file_descriptor);
        return (-EINVAL);
    }

    if (enable) {
        retval = ioctl(g_dev_file_descriptor, DEVICE_IOC_RAW, true);
        if (retval < 0) {
            printf_e("%s error: fail to enable the IOC_RAW ioctl command to %s\n",
                     __func__, g_dev_node);
            retval = -EINVAL;
            goto exit;
        }
        retval = ioctl(g_dev_file_descriptor, DEVICE_IOC_IRQ, false);
        if (retval < 0) {
            printf_e("%s error: fail to disable the IOC_IRQ ioctl command to %s\n",
                     __func__, g_dev_node);
            retval = -EINVAL;
            goto exit;
        }
        printf_i("%s info: set to RAW mode\n", __func__);
    }
    else {
        retval = ioctl(g_dev_file_descriptor, DEVICE_IOC_RAW, false);
        if (retval < 0) {
            printf_e("%s error: fail to disable the IOC_RAW ioctl command to %s\n",
                     __func__, g_dev_node);
            retval = -EINVAL;
            goto exit;
        }
        retval = ioctl(g_dev_file_descriptor, DEVICE_IOC_IRQ, true);
        if (retval < 0) {
            printf_e("%s error: fail to enable the IOC_IRQ ioctl command to %s\n",
                     __func__, g_dev_node);
            retval = -EINVAL;
            goto exit;
        }
        printf_i("%s info: set to IRQ mode\n", __func__);
    }
exit:
    return retval;
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
 * return: <0, fail to read a tcm message
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

    if(g_dev_file_descriptor < 0) {
        printf_e("%s error: file descriptor is initialized (%d)\n",
                 __func__, g_dev_file_descriptor);
        return (-EINVAL);
    }

    retval = (int) read(g_dev_file_descriptor, p_rd_data, bytes_to_read);
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
 * return: <0, fail to write a tcm command
 *         otherwise, number of writing bytes
 */
int tcm_write_message(unsigned char *p_wr_data, unsigned int bytes_to_write)
{
    int retval = 0;
    int i;

    if (!p_wr_data)  {
        printf_e("%s error: p_wr_data can't be null\n", __func__);
        retval = -EINVAL;
        return retval;
    }

    if(g_dev_file_descriptor < 0) {
        printf_e("%s error: file descriptor is initialized (%d)\n",
                 __func__, g_dev_file_descriptor);
        return (-EINVAL);
    }

    retval = (int) write(g_dev_file_descriptor, p_wr_data, bytes_to_write);
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
 * Function:  tcm_drop_report
 * --------------------
 * function to drop one tcm package
 *
 * return: <0, error when i2c read operation
 *         otherwise, succeed
 */
int tcm_drop_package(int payload_size) {
    int retval = 0;
    int size = payload_size + 3;  // include 2-byte header + 1-byte ending
    unsigned char *buf;

    buf = calloc((size_t)size, sizeof(unsigned char));

    retval = tcm_read_message(&buf[0], (unsigned int)size);
    if (retval < 0) {
        printf_e("%s error: fail to read package (remaining = %d)\n",
                 __func__, payload_size);
        retval = -EINVAL;
    }

    free(buf);
    printf_i("%s: %d bytes are dropped\n", __func__, payload_size);

    return retval;
}

/*
 * Function:  tcm_wait_for_command_ready
 * --------------------
 * function to wait for the command completion by polling the tcm package
 *
 * return: the payload length if succeed
 *         otherwise, error out, retval < 0
 */
int tcm_wait_for_command_ready(void)
{
    int retval = 0;
    bool command_is_ready = false;
    int timeout_cnt = 0;
    int size = sizeof(struct tcm_message_header);
    struct tcm_message_header header;
    int payload_size;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    do {
        usleep(TCM_POLLING_DELAY_MS * 1000);

        retval = tcm_read_message((unsigned char *)&header, (unsigned int) size);
        if (retval < 0) {
            printf_e("%s error: fail to read header from tcm device\n", __func__);
            return retval;
        }

        if ( 0xA5 == header.marker) {
            if ( STATUS_OK == header.code) {
                command_is_ready = true;
            }
            /* if the return code belongs to report, drop this report */
            else if ((header.code & 0x10) == 0x10) {
                payload_size = header.length[0] | (header.length[1] << 8);
                tcm_drop_package(payload_size);
            }
            /* if the return code belongs to report, drop this report */
            else if (STATUS_COMMAND_NOT_IMPLEMENTED == header.code) {
                printf_e("%s error: command is not implemented\n", __func__);
#ifdef SAVE_ERR_MSG
                sprintf(err, "%s: error: command is not implemented, status code: 0x%x\n",
                        __func__, header.code);
                add_error_msg(err);
#endif
                return (-ENOSYS);
            }
        }
        timeout_cnt += 1;

    } while( (!command_is_ready) && (timeout_cnt < TCM_POLLING_TIMOUT) );

    if (!command_is_ready) {
        printf_e("%s error: command timeout\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: command timeout\n", __func__);
        add_error_msg(err);
#endif
        return (-ETIMEDOUT);
    }

    size = convert_uc_to_short(header.length[0], header.length[1]);
    return size;
}


/*
 * Function:  tcm_get_payload
 * --------------------
 * read the data payload from the tcm device
 *
 * parameter
 *  p_rd_data: buffer of data reading
 *  payload: payload size in byte
 *
 * return: <0, fail to save payload data
 *         otherwise, succeed
 */
int tcm_get_payload(unsigned char *p_rd_data, int payload_size)
{
    int retval = 0;
    unsigned char *buf = NULL;
    int size = payload_size + 3; // 2-byte header (0xa5 0x03) + payload_size + 1-byte ending (0x5a)

    if (!p_rd_data) {
        printf_e("%s error: p_rd_data is NULL\n", __func__);
        retval = -EINVAL;
        goto exit;
    }

    /* each i2c read contains 2-byte header + payload_size + 1-byte ending */
    buf = calloc((size_t)size, sizeof(unsigned char));
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
            printf_e("%s error: payload packet is not ended (last byte: 0x%x)\n",
                     __func__, buf[size-1]);

        if (STATUS_CONTINUED_READ != buf[1])
            printf_e("%s error: not STATUS_CONTINUED_READ (byte 0: 0x%x, byte 1: 0x%x)\n",
                     __func__, buf[0], buf[1]);
    }
    /* copy to the input buffer */
    memcpy(p_rd_data, &buf[2], (size_t)payload_size); /* skip the first 2 bytes, 0xa5 0x03 */

exit:
    if(buf)
        free(buf);

    return retval;
}

/*
 * Function:  tcm_do_reset
 * --------------------
 * helper to send a sw reset
 *
 * return: <0, fail to do reset
 *         otherwise, succeed
 */
int tcm_do_reset()
{
    int retval = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    printf_i("%s info: do reset", __func__);

    /* first, configure the into the IRQ mode */
    retval = tcm_enable_raw_mode(false);
    if (retval < 0) {
        printf_e("%s error: fail to switch to irq mode\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to switch to irq mode\n", __func__);
        add_error_msg(err);
#endif
        return retval;
    }

    /* perform a reset via the IOCTL */
    retval = ioctl(g_dev_file_descriptor, DEVICE_IOC_RESET);
    if (retval < 0) {
        printf_e("%s error: fail to send the DEVICE_IOC_RESET command to %s\n",
                 __func__, g_dev_node);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send the DEVICE_IOC_RESET command to %s\n",
                __func__, g_dev_node);
        add_error_msg(err);
#endif
    }

    usleep(TCM_RESET_DELAY_MS * 1000);

    /* at the end, switch back to the raw mode */
    retval = tcm_enable_raw_mode(true);
    if (retval < 0) {
        printf_e("%s error: fail to switch back raw mode\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to switch back raw mode\n", __func__);
        add_error_msg(err);
#endif
    }

    return retval;
}

/*
 * Function:  tcm_get_static_config
 * --------------------
 * helper to get the static config
 *
 * parameter
 *  buf: buffer for data reading
 *  size_buf: size of input buffer in byte
 *
 * return: <0, fail to get the static config
 *         otherwise, succeed
 */
int tcm_get_static_config()
{
    int retval = 0;
    unsigned char command_packet[3] = {0, 0, 0};
    int size_payload;
    unsigned char *buf = g_tcm_handler.static_config;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* command code */
    command_packet[0] = CMD_GET_STATIC_CONFIG;

    /* command payload length */
    /* length = 0 */
    command_packet[1] = 0x00;
    command_packet[2] = 0x00;
    /* send command to tcm device */
    retval = tcm_write_message(command_packet, sizeof(command_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_GET_STATIC_CONFIG\n",__func__);
        retval = -EINVAL;
        return retval;
    }
    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: fail to get the response of CMD_GET_STATIC_CONFIG \n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: fail to get the response of CMD_GET_STATIC_CONFIG\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    if (retval > TCM_MAX_STATIC_CONFIG_SIZE) {
        printf_e("%s error: size of static config is mismatching (payload size = %d)\n",
                 __func__, retval);
        retval = -EINVAL;
        return retval;
    }

    size_payload = retval;

    retval = tcm_get_payload(buf, retval);
    if (retval < 0) {
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, size_payload);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, size_payload);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

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
 * return: <0, fail to set the dynamic config
 *         otherwise, succeed
 */
int tcm_set_dynamic_config(unsigned char field_id, unsigned short value)
{
    int retval = 0;
    unsigned char command_packet[6] = {0, 0, 0, 0, 0, 0};
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

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
        printf_e("%s error: fail to send command, CMD_SET_DYNAMIC_CONFIG 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                 __func__, command_packet[1], command_packet[2], command_packet[3],
                 command_packet[4], command_packet[5]);
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
    printf_i("%s info: CMD_SET_DYNAMIC_CONFIG is completed. (field_id:0x%x) (value:%d)",
             __func__, (int)field_id, value);

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
 * return: <0, fail to config into no_sleep mode
 *         otherwise, succeed
 */
int tcm_set_no_sleep(int state)
{
    unsigned short value = (0 == state)?(unsigned short)0:(unsigned short)1;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    int retval = tcm_set_dynamic_config(DC_NO_DOZE, value);
    if (retval < 0) {
        printf_e("%s error: fail to set DC_NO_DOZE to %d", __func__, value);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to set DC_NO_DOZE to %d\n",
                __func__,value);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }
    printf_i("%s info: set DC_NO_DOZE to %d", __func__, value);
    return 0;
}

/*
 * Function:  tcm_set_rezero
 * --------------------
 * helper to send rezero command
 *
 * return: <0, fail to send rezero command
 *         otherwise, succeed
 */
int tcm_set_rezero()
{
    int retval = 0;
    unsigned char command = CMD_REZERO;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* send command to tcm device */
    retval = tcm_write_message(&command, sizeof(command));
    if (retval < 0) {
        printf_e("%s error: fail to send command, CMD_REZERO\n", __func__);
        retval = -EINVAL;
        return retval;
    }
    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf_e("%s error: timeout while running the command, CMD_REZERO\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get the response of command, CMD_REZERO\n", __func__);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }
    printf_i("%s info: CMD_REZERO is completed", __func__);
    return retval;
}


/*
 * Function:  tcm_raw_control
 * --------------------
 * helper to perform a raw control
 *
 * parameter
 *  p_in: input buffer, tcm packets
 *  size_in: size of input buffer
 *  p_out: output buffer, response
 *  size_out: size of output buffer
 *
 * return: <0, fail to perform a raw control
 *         =0, timeout
 *         otherwise, return payload length
 */
int tcm_raw_control(unsigned char* p_in, int size_in,
                    unsigned char* p_out, int size_out)
{
    int retval = 0;
    struct tcm_message_header header;
    bool stop_polling = false;
    int timeout_cnt = 0;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (!p_in) {
        printf_e("%s error: invalid parameter, p_in is null\n", __func__);
        retval = -EINVAL;
        return retval;
    }

    if (!p_out) {
        printf_e("%s error: invalid parameter, p_out is null\n", __func__);
        retval = -EINVAL;
        return retval;
    }

    if (size_out < 4) {
        printf_e("%s error: invalid parameter, size_out < 4\n", __func__);
        retval = -EINVAL;
        return retval;
    }

    printf_i("%s info: command = 0x%x (length = 0x%x 0x%x)\n",
             __func__, p_in[0], p_in[1], p_in[2]);

    /* send command to tcm device */
    retval = tcm_write_message(p_in, (unsigned int )size_in);
    if (retval < 0) {
        printf_e("%s error: fail to send command, 0x%x\n",__func__,  p_in[0]);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to send command, 0x%x\n",__func__,  p_in[0]);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }
    /* wait for the command completion */
    do {
        usleep(TCM_POLLING_DELAY_MS * 1000);

        retval = tcm_read_message((unsigned char *)&header, sizeof(struct tcm_message_header));
        if (retval < 0) {
            printf_e("%s error: fail to read header from tcm device\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to read header from tcm device", __func__);
            add_error_msg(err);
#endif
            return retval;
        }

        if ( 0xA5 == header.marker) {
            if ( STATUS_OK == header.code) {
                stop_polling = true;
            }
            else if (STATUS_COMMAND_NOT_IMPLEMENTED == header.code) {
                printf_e("%s error: command no implemented\n", __func__);
                stop_polling = true;
            }
            else if (STATUS_ERROR == header.code) {
                printf_e("%s error: command error\n", __func__);
                stop_polling = true;
            }
            printf_i("%s: tcm header = 0x%x 0x%x 0x%x 0x%x\n",
                     __func__, header.marker, header.code, header.length[0], header.length[1]);
        }
        timeout_cnt += 1;

    } while( (!stop_polling) && (timeout_cnt < TCM_POLLING_TIMOUT) );

    if (timeout_cnt == TCM_POLLING_TIMOUT) {
        printf_e("%s error: command 0x%x timeout\n", __func__, p_in[0]);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: command 0x%x timeout\n", __func__, p_in[0]);
        add_error_msg(err);
#endif
    }

    p_out[0] = header.marker;
    p_out[1] = header.code;
    p_out[2] = header.length[0];
    p_out[3] = header.length[1];

    retval = convert_uc_to_short(header.length[0], header.length[1]);

    return retval;
}

/*
 * Function:  tcm_get_pins_mapping
 * --------------------
 * helper to parse the trx pins mapping from the static config
 *
 * return: <0, invalid parameter
 *         otherwise, success
 */
int tcm_get_pins_mapping(int rxes_offset, int rxes_len, int txes_offset, int txes_len,
                         int num_rxguard_offset, int num_rxguard_len,
                         int rxguard_pins_offset, int rxguard_pins_len,
                         int num_txguard_offset, int num_txguard_len,
                         int txguard_pins_offset, int txguard_pins_len)
{
    unsigned char* cfg_data = g_tcm_handler.static_config;
    int cfg_data_len = convert_uc_to_short(g_tcm_handler.app_info_report.static_config_size[0],
                                g_tcm_handler.app_info_report.static_config_size[1]);
    int i, j = 0, idx = 0;
    int offset_rx_pin = rxes_offset/8;
    int length_rx_pin = rxes_len/8;
    int offset_tx_pin = txes_offset/8;
    int length_tx_pin = txes_len/8;
    int num_rx_guard = 0;
    int offset_num_rx_guard= num_rxguard_offset/8;
    int length_num_rx_guard= num_rxguard_len/8;
    int offset_rx_guard= rxguard_pins_offset/8;
    int length_rx_guard= rxguard_pins_len/8;
    int num_tx_guard = 0;
    int offset_num_tx_guard= num_txguard_offset/8;
    int length_num_tx_guard= num_txguard_len/8;
    int offset_tx_guard= txguard_pins_offset/8;
    int length_tx_guard= txguard_pins_len/8;
    bool is_err = false;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    if (cfg_data_len < 0) {
        printf_e("%s error: invalid size of static config\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid size of static config\n", __func__);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    g_tcm_handler.tx_assigned = 0;
    memset(g_tcm_handler.tx_pins, 0x00, sizeof(g_tcm_handler.tx_pins));
    g_tcm_handler.rx_assigned = 0;
    memset(g_tcm_handler.rx_pins, 0x00, sizeof(g_tcm_handler.rx_pins));
    g_tcm_handler.guard_assigned = 0;
    memset(g_tcm_handler.guard_pins, 0x00, sizeof(g_tcm_handler.guard_pins));

    /* get tx pins mapping */
    if ((cfg_data_len > offset_tx_pin + length_tx_pin) && (offset_tx_pin != 0)) {

        g_tcm_handler.tx_assigned = (length_tx_pin/2);

        printf_i("%s info: get tx pins mapping, tx assigned = %2d\n",
                 __func__, g_tcm_handler.tx_assigned);

        idx = 0;
        for (i = 0; i < (length_tx_pin/2); i++) {
            g_tcm_handler.tx_pins[i] = (short)cfg_data[offset_tx_pin + idx] |
                                       (short)(cfg_data[offset_tx_pin + idx + 1] << 8);
            idx += 2;

            printf_i("%s info: tx[%d] = %2d\n", __func__, i, g_tcm_handler.tx_pins[i]);
            if (g_tcm_handler.tx_pins[i] > TCM_MAX_PINS) {
                printf_e("%s error: pin number %d is over 64\n",
                         __func__, g_tcm_handler.tx_pins[i]);
                is_err = true;
            }
        }
    }
    else {
        printf_e("%s error: invalid tx pins info (offset, length) = (%d, %d) cfg_data_len = %d\n",
                 __func__, offset_tx_pin, length_tx_pin, cfg_data_len);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid tx pins info (offset, length) = (%d, %d) cfg_data_len = %d\n",
                __func__, offset_tx_pin, length_tx_pin, cfg_data_len);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    /* get rx pins mapping */
    if ((cfg_data_len > offset_rx_pin + length_rx_pin) && (offset_rx_pin != 0)) {

        g_tcm_handler.rx_assigned = (length_rx_pin/2);

        printf_i("%s get rx pins mapping, rx assigned = %2d\n",
                 __func__, g_tcm_handler.rx_assigned);

        idx = 0;
        for (i = 0; i < (length_rx_pin/2); i++) {
            g_tcm_handler.rx_pins[i] = (short)cfg_data[offset_rx_pin + idx] |
                                       (short)(cfg_data[offset_rx_pin + idx + 1] << 8);
            idx += 2;

            printf_i("%s rx[%d] = %2d\n", __func__, i, g_tcm_handler.rx_pins[i])
            if (g_tcm_handler.rx_pins[i] > TCM_MAX_PINS) {
                printf_e("%s error: pin number %d is over 64\n",
                         __func__, g_tcm_handler.rx_pins[i]);
                is_err = true;
            }
        }
    }
    else {
        printf_e("%s error: invalid rx pins info (offset, length) = (%d, %d) cfg_data_len = %d\n",
                 __func__, offset_rx_pin, length_rx_pin, cfg_data_len);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: invalid rx pins info (offset, length) = (%d, %d) cfg_data_len = %d\n",
                __func__, offset_rx_pin, length_rx_pin, cfg_data_len);
        add_error_msg(err);
#endif
        return -EINVAL;
    }

    if (is_err)
        return -EINVAL;

    /* get number of tx guards */
    if ((cfg_data_len > offset_num_tx_guard + length_num_tx_guard) && (offset_num_tx_guard != 0)) {

        num_tx_guard = (short)cfg_data[offset_num_tx_guard] |
                       (short)(cfg_data[offset_num_tx_guard + 1] << 8);

        g_tcm_handler.guard_assigned += num_tx_guard;
    }

    /* get number of rx guards */
    if ((cfg_data_len > offset_num_rx_guard + length_num_rx_guard) && (offset_num_rx_guard != 0)) {

        num_rx_guard = (short)cfg_data[offset_num_rx_guard] |
                       (short)(cfg_data[offset_num_rx_guard + 1] << 8);

        g_tcm_handler.guard_assigned += num_rx_guard;
    }

    if (g_tcm_handler.guard_assigned > 0) {

        printf_i("%s get ground pins mapping, ground assigned = %2d (tx: %d, rx: %d)\n",
                 __func__, g_tcm_handler.guard_assigned, num_tx_guard, num_rx_guard);
        j = 0;
    }

    /* get tx guards mapping */
    if ((num_tx_guard > 0) &&
        (cfg_data_len > offset_tx_guard + length_tx_guard)) {
        idx = 0;
        for (i = 0; i < num_tx_guard; i++) {
            g_tcm_handler.guard_pins[j] = (short)cfg_data[offset_tx_guard + idx] |
                                          (short)(cfg_data[offset_tx_guard + idx + 1] << 8);

            printf_i("%s tx_ground[%d] = %2d\n", __func__, i, g_tcm_handler.guard_pins[i]);
            idx += 2;
            j += 1;

        }
    }

    /* get rx guards mapping */
    if ((num_rx_guard > 0) &&
        (cfg_data_len > offset_rx_guard + length_rx_guard)) {
        for (i = 0; i < num_rx_guard; i++) {
            g_tcm_handler.guard_pins[j] = (short)cfg_data[offset_rx_guard + idx] |
                                          (short)(cfg_data[offset_rx_guard + idx + 1] << 8);

            printf_i("%s rx_ground[%d] = %2d\n", __func__, i, g_tcm_handler.guard_pins[i]);
            idx += 2;
            j += 1;
        }
    }

    return 0;
}
