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

#define POLLING_TOUCH_REPORT_CNT 50
#define POLLING_TOUCH_REPORT_DELAY_MS 10

enum touch_report_code {
    TOUCH_END = 0,
    TOUCH_FOREACH_ACTIVE_OBJECT,
    TOUCH_FOREACH_OBJECT,
    TOUCH_FOREACH_END,
    TOUCH_PAD_TO_NEXT_BYTE,
    TOUCH_TIMESTAMP,
    TOUCH_OBJECT_N_INDEX,
    TOUCH_OBJECT_N_CLASSIFICATION,
    TOUCH_OBJECT_N_X_POSITION,
    TOUCH_OBJECT_N_Y_POSITION,
    TOUCH_OBJECT_N_Z,
    TOUCH_OBJECT_N_X_WIDTH,
    TOUCH_OBJECT_N_Y_WIDTH,
    TOUCH_OBJECT_N_TX_POSITION_TIXELS,
    TOUCH_OBJECT_N_RX_POSITION_TIXELS,
    TOUCH_0D_BUTTONS_STATE,
    TOUCH_GESTURE_DOUBLE_TAP,
    TOUCH_FRAME_RATE,
    TOUCH_POWER_IM,
    TOUCH_CID_IM,
    TOUCH_RAIL_IM,
    TOUCH_CID_VARIANCE_IM,
    TOUCH_NSM_FREQUENCY,
    TOUCH_NSM_STATE,
    TOUCH_NUM_OF_ACTIVE_OBJECTS,
    TOUCH_NUM_OF_CPU_CYCLES_USED_SINCE_LAST_FRAME,
    TOUCH_TUNING_GAUSSIAN_WIDTHS = 0x80,
    TOUCH_OBJECT_N_FORCE = 0x1c,
    TOUCH_TUNING_SMALL_OBJECT_PARAMS,
    TOUCH_TUNING_0D_BUTTONS_VARIANCE,
    TOUCH_REPORT_OBJECT_N_FORCE = 0xc9,
};

/*
 * Function:  tcm_get_touch_config
 * --------------------
 * get the format of touch report
 *
 * return: <0, fail to get the touch config
 *         otherwise, succeed
 */
int tcm_get_touch_config()
{
    int retval = 0;
    unsigned char command_packet[3] = {0, 0, 0};
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* command code */
    command_packet[0] = CMD_GET_TOUCH_REPORT_CONFIG;
    /* command length */
    /* length = 0 */
    command_packet[1] = 0x00;
    command_packet[2] = 0x00;

    /* send command to tcm device */
    retval = tcm_write_message(command_packet, sizeof(command_packet));
    if (retval < 0) {
        printf_e("%s error: fail to send command, GET_TOUCH_REPORT_CONFIG 0x%x 0x%x\n", __func__,
                 command_packet[1], command_packet[2]);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: fail to send command, GET_TOUCH_REPORT_CONFIG 0x%x 0x%x\n", __func__,
                command_packet[1], command_packet[2]);
        add_error_msg(err);
#endif
        retval = -EINVAL;
        return retval;
    }

    /* wait for the command completion */
    retval = tcm_wait_for_command_ready();
    if ( retval < 0) {
        printf("%s error: fail to GET_TOUCH_REPORT_CONFIG\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: error: fail to GET_TOUCH_REPORT_CONFIG\n", __func__);
        add_error_msg(err);
#endif
        retval = -EIO;
        return retval;
    }

    /* read touch config */
    retval = tcm_get_payload(g_tcm_handler.touch_config, retval);
    if (retval < 0) {
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, retval);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s: fail to get data payload (size = %d)\n", __func__, retval);
        add_error_msg(err);
#endif
        retval = -EIO;
        return retval;
    }

    printf_i("%s info: GET_TOUCH_REPORT_CONFIG is completed. (payload = %d)", __func__, retval);

    g_tcm_handler.size_of_finger_report = 0;

    return retval;
}

/*
 * Function:  tcm_get_data_from_report
 * --------------------
 * get teh specified data from the touch report
 *
 * return: <0, fail to get the data
 *         otherwise, succeed
 */
static int tcm_get_data_from_report(unsigned char *entry, unsigned int size,
                    unsigned int offset, unsigned int bits, unsigned int *data)
{
    unsigned char mask;
    unsigned char byte_data;
    unsigned int output_data;
    unsigned int bit_offset;
    unsigned int byte_offset;
    unsigned int data_bits;
    unsigned int available_bits;
    unsigned int remaining_bits;

    if (bits == 0 || bits > 32) {
        printf_e("%s error: invalid number of bits\n", __func__);
        return -EINVAL;
    }

    if (offset + bits > size * 8) {
        *data = 0;
        return 0;
    }

    output_data = 0;
    remaining_bits = bits;

    bit_offset = offset % 8;
    byte_offset = offset / 8;

    while (remaining_bits) {
        byte_data = entry[byte_offset];
        byte_data >>= bit_offset;

        available_bits = 8 - bit_offset;
        data_bits = MIN(available_bits, remaining_bits);
        mask = (unsigned char) 0xff >> (8 - data_bits);

        byte_data &= mask;

        output_data |= byte_data << (bits - remaining_bits);

        bit_offset = 0;
        byte_offset += 1;
        remaining_bits -= data_bits;
    }

    *data = output_data;

    return 0;
}
/*
 * Function:  tcm_parse_touch_report
 * --------------------
 * based on the touch config, parse all fields to generate the touch report
 * the g_tcm_touch_config must be prepared ready
 *
 * return: <0, fail to get the data
 *         otherwise, succeed
 */
static void tcm_parse_touch_report(unsigned char *entry, unsigned int size)
{
    int retval = 0;
    bool active_only = false;
    bool num_of_active_objects;
    unsigned char code;
    unsigned int idx;
    unsigned int obj = 0;
    unsigned int next = 0;
    unsigned int data;
    unsigned int bits;
    unsigned int offset;
    unsigned int objects;
    unsigned int active_objects = 0;
    static unsigned int end_of_foreach;
    const int max_reported_obj = convert_uc_to_short(g_tcm_handler.app_info_report.max_objects[0],
                                          g_tcm_handler.app_info_report.max_objects[1]);

    num_of_active_objects = false;

    idx = 0;
    offset = 0;
    objects = 0;
    while (idx < TCM_TOUCH_CONFIG_SIZE) {
        code = g_tcm_handler.touch_config[idx++];
        switch (code) {
            case TOUCH_END:
                goto exit;
            case TOUCH_FOREACH_ACTIVE_OBJECT:
                obj = 0;
                next = idx;
                active_only = true;
                break;
            case TOUCH_FOREACH_OBJECT:
                obj = 0;
                next = idx;
                active_only = false;
                break;
            case TOUCH_FOREACH_END:
                end_of_foreach = idx;
                if (active_only) {
                    if (num_of_active_objects) {
                        objects++;
                        if (objects < active_objects)
                            idx = next;
                    } else if (offset < size * 8) {
                        idx = next;
                    }
                } else {
                    obj++;
                    if (obj < max_reported_obj)
                        idx = next;
                }
                break;
            case TOUCH_PAD_TO_NEXT_BYTE:
                offset = (offset + 8 - 1) / 8 * 8;
                break;
            case TOUCH_TIMESTAMP:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get timestamp\n", __func__);
                    goto exit;
                }
                //printf_i("%s Timestamp: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].timestamp = data;
                break;
            case TOUCH_OBJECT_N_INDEX:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object index\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object index: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.current_finger_idx = data;
                break;
            case TOUCH_OBJECT_N_CLASSIFICATION:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object classification\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object classification: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].classification = data;
                break;
            case TOUCH_OBJECT_N_X_POSITION:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object x position\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object x position: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].x = data;
                break;
            case TOUCH_OBJECT_N_Y_POSITION:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object y position\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object y position: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].y = data;
                break;
            case TOUCH_REPORT_OBJECT_N_FORCE:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object y position\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object force value: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].prs = data;
                break;
            case TOUCH_OBJECT_N_Z:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object z\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object z: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].z = data;
                break;
            case TOUCH_OBJECT_N_X_WIDTH:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object x width\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object x width: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].x_w = data;
                break;
            case TOUCH_OBJECT_N_Y_WIDTH:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object y width\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object y width: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].y_w = data;
                break;
            case TOUCH_OBJECT_N_TX_POSITION_TIXELS:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object tx position\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object tx position: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].tx_pos = data;
                break;
            case TOUCH_OBJECT_N_RX_POSITION_TIXELS:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get object rx position\n", __func__);
                    goto exit;
                }
                //printf_i("%s Object rx position: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].rx_pos = data;
                break;
            case TOUCH_0D_BUTTONS_STATE:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get 0D buttons state\n", __func__);
                    goto exit;
                }
                //printf_i("%s 0D buttons state: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].od = data;
                break;
            case TOUCH_GESTURE_DOUBLE_TAP:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get gesture double tap\n", __func__);
                    goto exit;
                }
                //printf_i("%s Gesture double tap: %u\n", __func__, data);
                offset += bits;

                g_tcm_handler.finger[g_tcm_handler.current_finger_idx].gesture_double_tap = data;
                break;
            case TOUCH_FRAME_RATE:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get frame rate\n", __func__);
                    goto exit;
                }
                //printf_i("%s Frame rate: %u\n", __func__, data);
                offset += bits;
                break;
            case TOUCH_POWER_IM:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get power IM\n", __func__);
                    goto exit;
                }
                //printf_i("%s Power IM: %u\n", __func__, data);
                offset += bits;
                break;
            case TOUCH_CID_IM:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get CID IM\n", __func__);
                    goto exit;
                }
                //printf_i("%s CID IM: %u\n", __func__, data);
                offset += bits;
                break;
            case TOUCH_RAIL_IM:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get rail IM\n", __func__);
                    goto exit;
                }
                //printf_i("%s Rail IM: %u\n", __func__, data);
                offset += bits;
                break;
            case TOUCH_CID_VARIANCE_IM:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get CID variance IM\n", __func__);
                    goto exit;
                }
                //printf_i("%s CID variance IM: %u\n", __func__, data);
                offset += bits;
                break;
            case TOUCH_NSM_FREQUENCY:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get NSM frequency\n", __func__);
                    goto exit;
                }
                //printf_i("%s NSM frequency: %u\n", __func__, data);
                offset += bits;
                break;
            case TOUCH_NSM_STATE:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get NSM state\n", __func__);
                    goto exit;
                }
                //printf_i("%s NSM state: %u\n", __func__, data);
                offset += bits;
                break;
            case TOUCH_NUM_OF_ACTIVE_OBJECTS:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get number of active objects\n", __func__);
                    goto exit;
                }
                //printf_i("%s Number of active objects: %u\n", __func__, data);
                active_objects = data;
                num_of_active_objects = true;
                offset += bits;
                if (active_objects == 0)
                    idx = end_of_foreach;
                break;
            case TOUCH_NUM_OF_CPU_CYCLES_USED_SINCE_LAST_FRAME:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get number of CPU cycles since last frame\n",
                             __func__);
                    goto exit;
                }
                //printf_i("%s Number of CPU cycles since last frame: %u\n", __func__, data);
                offset += bits;
                break;
            case TOUCH_TUNING_GAUSSIAN_WIDTHS:
                bits = g_tcm_handler.touch_config[idx++];
                offset += bits;
                break;
            case TOUCH_TUNING_SMALL_OBJECT_PARAMS:
                bits = g_tcm_handler.touch_config[idx++];
                offset += bits;
                break;
            case TOUCH_TUNING_0D_BUTTONS_VARIANCE:
                bits = g_tcm_handler.touch_config[idx++];
                offset += bits;
                break;
            case TOUCH_OBJECT_N_FORCE:
                bits = g_tcm_handler.touch_config[idx++];
                retval = tcm_get_data_from_report(entry, size, offset, bits, &data);
                if (retval < 0) {
                    printf_e("%s error: fail to get force data\n", __func__);
                    goto exit;
                }
                offset += bits;
                break;
            default:
                printf_e("%s error: fail unknown report code 0x%x\n",
                         __func__, code);
                break;
        }
    }
exit:
    return;
}


/*
 * Function:  tcm_get_touch_report
 * --------------------
 * retrieve the touch report from tcm device
 *
 * return: <0, fail to get the touch report
 *         otherwise, succeed
 */
int tcm_get_touch_report(int report_size)
{
    int retval;
    unsigned char *touch_report = NULL;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    /* allocate the memory */
    touch_report = calloc((size_t)report_size, sizeof(unsigned char));
    if (!touch_report) {
        printf_e("%s error: can't allocate memory for touch_report\n", __func__);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: can't allocate memory for touch_report\n", __func__);
        add_error_msg(err);
#endif
        retval = -ENOMEM;
        goto exit;
    }

    /* read touch report */
    retval = tcm_get_payload(touch_report, report_size);
    if (retval < 0) {
        retval = -EINVAL;
        printf_e("%s error: fail to get data payload (size = %d)\n", __func__, report_size);
#ifdef SAVE_ERR_MSG
        sprintf(err, "%s error: fail to get data payload (size = %d)\n", __func__, report_size);
        add_error_msg(err);
#endif
        goto exit;
    }

    /* parse touch report */
    tcm_parse_touch_report(touch_report, (unsigned int)report_size);

exit:
    if(touch_report)
        free(touch_report);

    return retval;
}

/*
 * Function:  tcm_query_touch_response
 * --------------------
 * continue to monitor the available touch report within 500ms
 * once detected, parse the data
 *
 * return: <0, fail to get the touch report
 *         otherwise, succeed
 */
int tcm_query_touch_response(int *touch_x, int* touch_y, int* touch_status,
                           int max_fingers_to_process)
{
    struct tcm_message_header header;
    int retry = 0;
    int retval = 0;
    int size_payload = 0;
    bool is_finger_event = false;
    int idx;
#ifdef SAVE_ERR_MSG
    char err[MAX_ERR_STRING_LEN];
#endif

    do {
        retval = tcm_read_message((unsigned char *)&header, sizeof(struct tcm_message_header));
        if (retval < 0) {
            printf_e("%s error: fail to read header from tcm device\n", __func__);
            return retval;
        }

        if ( 0xA5 == header.marker) {

            size_payload = convert_uc_to_short(header.length[0], header.length[1]);

            if ( TCM_REPORT_TOUCH == header.code) {
                /* if get a touch report, save the data payload */
                if (0 == g_tcm_handler.size_of_finger_report)
                    g_tcm_handler.size_of_finger_report = size_payload;

                is_finger_event = true;
                break;
            }
            /* drop the tcm package if it is not touch report */
            else {
                tcm_drop_package(size_payload);
            }
        }
        retry += 1;
        usleep(POLLING_TOUCH_REPORT_DELAY_MS * 1000);

    } while( (retry < POLLING_TOUCH_REPORT_CNT) );

    /* once a touched event coming */
    if (is_finger_event) {

        /* retrieve a touch report */
        retval = tcm_get_touch_report(size_payload);
        if (retval < 0) {
            retval = -EINVAL;
            printf_e("%s error: fail to get the touch report\n", __func__);
#ifdef SAVE_ERR_MSG
            sprintf(err, "%s error: fail to get the touch report\n", __func__);
            add_error_msg(err);
#endif
            goto exit;
        }

        /* prepare the returned data, the number of finger reported */
        retval = g_tcm_handler.current_finger_idx + 1;

        if (size_payload != g_tcm_handler.size_of_finger_report)
            retval = 0;

        for (idx = 0; idx < retval; idx++) {
            if (idx+1 > max_fingers_to_process )
                continue;

            touch_x[idx] = g_tcm_handler.finger[idx].x;
            touch_y[idx] = g_tcm_handler.finger[idx].y;
            touch_status[idx] = g_tcm_handler.finger[idx].classification;
        }

    } // end of if (is_finger_event)

exit:
    return retval;
}