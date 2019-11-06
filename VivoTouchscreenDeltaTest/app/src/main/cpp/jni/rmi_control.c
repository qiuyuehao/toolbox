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
#include <sys/stat.h>
#include <time.h>

#include "native-lib.h"
#include "rmi_control.h"

#define RMI_SW_RESET_DELAY_MS 500

static int g_rmi_fd;

int rmi_f54_scan_freq_gear();

static bool rmi_is_initialized;

static int g_rmi_available_gears;
static unsigned char gear_en[MAX_RMI_FREUENCY_GEAR]; /* 1: enable; 0: disable*/


static bool str_starts_with(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

/*
 * Function:  rmi_open_dev
 * --------------------
 * initialize the rmi access function and
 * get the file descriptor
 *
 * return: 0  - rmi_file_descriptor already exist
 *         <0 - fail to open device
 *         otherwise, succeed
 */
int rmi_open_dev(const char* rmi_path)
{
    if (g_rmi_fd > 0)
        printf_e("%s: g_rmi_fd (%d) is not 0\n", __func__, g_rmi_fd);

    g_rmi_fd = open(rmi_path, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (g_rmi_fd < 0) {
        printf_e("%s: failed to open %s, (err: %s)\n",
                 __func__, rmi_path, strerror(errno));
        sprintf(err_msg_out, "%s: failed to open %s, (err: %s)\n",
                __func__, rmi_path, strerror(errno));
        return -EIO;
    }

    printf_i("%s: open %s successfully (rmi_fd = %d)\n", __func__, rmi_path, g_rmi_fd);

    return g_rmi_fd;
}

/*
 * Function:  rmi_close_dev
 * --------------------
 * close the file descriptor
 *
 * return: n\a
 *
 */
void rmi_close_dev(const char* rmi_path)
{
    close(g_rmi_fd);
    printf_i("%s: close %s\n", __func__, rmi_path);

    g_rmi_fd = 0;
}

/*
 * Function:  rmi_read_reg
 * --------------------
 * read the rmi register
 *
 * parameter
 *  addr: reg address
 *  p_rd_data: buffer of data reading
 *  bytes_to_read: number of bytes for reading
 *
 * return: <0 - fail to read rmi reg
 *         otherwise, number of bytes of p_rd_data
 */
int rmi_read_reg(unsigned short address, unsigned char *p_rd_data, int bytes_to_read)
{
    int retval;

    if (!p_rd_data)  {
        printf_e("%s: p_rd_data buffer is null\n", __func__);
        return -1;
    }

    if(g_rmi_fd < 0) {
        printf_e("%s error: file descriptor is initialized (%d)\n", __func__, g_rmi_fd);
        return (-EINVAL);
    }

    lseek(g_rmi_fd, address, SEEK_SET);

    retval = (int) read(g_rmi_fd, (void *)p_rd_data, (size_t)bytes_to_read);
    if (retval < 0)  {
        printf_e("%s: failed to read data. addr= 0x%x, bytes_to_read= %d (retval = %d)\n",
                 __func__, address, bytes_to_read, retval);
        return retval;
    }

    return retval;
}

/*
 * Function:  rmi_write_reg
 * --------------------
 * write the rmi data
 *
 * parameter
 *  addr: reg address
 *  p_rd_data: data for writing
 *  bytes_to_write: number of bytes for writing
 *
 * return: <0 - fail to write rmi reg
 *         otherwise, number of writing bytes
 */
int rmi_write_reg(unsigned short address, unsigned char *p_wr_data, int bytes_to_write)
{
    int retval = 0;

    if (!p_wr_data)  {
        printf_e("%s: p_wr_data can't be null\n", __func__);
        return -1;
    }

    if(g_rmi_fd < 0) {
        printf_e("%s error: file descriptor is initialized (%d)\n", __func__, g_rmi_fd);
        return (-EINVAL);
    }

    lseek(g_rmi_fd, address, SEEK_SET);

    retval = (int) write(g_rmi_fd, p_wr_data, (size_t)bytes_to_write);
    if (retval < 0)  {
        printf_e("%s: failed to write data. addr= 0x%x, bytes_to_write= %d, (retval = %d)",
                 __func__, address, bytes_to_write, retval);
        return retval;
    }

    return retval;
}


/*
 * Function:  rmi_parse_f01_information
 * --------------------
 * parse rmi function 01 including build ID and ASIC ID
 *
 * return: n\a
 */
void rmi_parse_f01_information(void)
{
    uint8_t buffer_asic[4] = {0};
    unsigned char buffer_id[3] = {0};

    rmi_read_reg((unsigned short) (g_rmi_pdt.F01.query_base_addr + 17), &buffer_asic[0], 4);
    sprintf((char *)g_rmi_pdt.asic_type, "%d", buffer_asic[1] << 8 | buffer_asic[0]);

    g_rmi_pdt.is_tddi_dev = str_starts_with("4", (char *)g_rmi_pdt.asic_type);

    printf_i("%s: ASIC type = %s (is_tddi_dev = %s)\n", __func__, g_rmi_pdt.asic_type, (g_rmi_pdt.is_tddi_dev)?"true":"false");

    rmi_read_reg((unsigned short)(g_rmi_pdt.F01.query_base_addr + 18), &buffer_id[0], 3);
    g_rmi_pdt.build_id = (unsigned int)buffer_id[0] +
                   (unsigned int)buffer_id[1] * 0x100 +
                   (unsigned int)buffer_id[2] * 0x10000;
    printf_i("%s: Build_ID = %d\n", __func__, g_rmi_pdt.build_id);
}

/*
 * Function:  rmi_parse_f12_information
 * --------------------
 * parse rmi function 12
 *
 * return: n\a
 */
void rmi_parse_f12_information(void)
{
    unsigned char size_of_query5;
    unsigned char query_5[9] = {0};
    unsigned char offset = 0;

    // read size of size of control presence
    rmi_read_reg((unsigned short) (g_rmi_pdt.F12.query_base_addr + 4), &size_of_query5, sizeof(size_of_query5));

    if (size_of_query5 > sizeof(query_5))
        size_of_query5 = sizeof(query_5);

    // read control presence
    rmi_read_reg((unsigned short) (g_rmi_pdt.F12.query_base_addr + 5), &query_5[0], size_of_query5);

    // f12_ctrl_0 ~ f12_ctrl_7 reserved

    // f12_ctrl_8 ~ f12_ctrl_15
    if (CHECK_BIT(query_5[2], 0))  // ctrl 8 is present
        offset += 1;

    if (CHECK_BIT(query_5[2], 1))  // ctrl 9 is present
        offset += 1;

    if (CHECK_BIT(query_5[2], 2))  // ctrl 10 is present
        offset += 1;

    if (CHECK_BIT(query_5[2], 3))  // ctrl 11 is present
        offset += 1;

    if (CHECK_BIT(query_5[2], 4))  // ctrl 12 is present
        offset += 1;

    if (CHECK_BIT(query_5[2], 5))  // ctrl 13 is present
        offset += 1;

    if (CHECK_BIT(query_5[2], 6))  // ctrl 14 is present
        offset += 1;

    if (CHECK_BIT(query_5[2], 7)){ // ctrl 15 is present
        // read Finger Amplitude Threshold
        rmi_read_reg(g_rmi_pdt.F12.control_base_addr + offset, &g_rmi_pdt.finger_threshold, 1);
        // offset += 1;
    }

}
/*
 * Function:  rmi_parse_f54_data
 * --------------------
 * parse rmi function 54 data registers
 *
 * return: n\a
 */
void rmi_parse_f54_data(void)
{
    unsigned short reg_addr;
    reg_addr = (unsigned short)(g_rmi_pdt.F54.data_base_addr + RMI_REPROT_DATA_OFFSET + 1);

    /* data 4 */
    if (g_rmi_pdt.f54_query.has_sense_frequency_control)
        reg_addr++;

    /* data 5 reserved */

    /* data 6 */
    if (g_rmi_pdt.f54_query.has_interference_metric)
        reg_addr += 2;

    /* data 7 */
    if (g_rmi_pdt.f54_query.has_one_byte_report_rate |
            g_rmi_pdt.f54_query.has_two_byte_report_rate)
        reg_addr++;
    if (g_rmi_pdt.f54_query.has_two_byte_report_rate)
        reg_addr++;

    /* data 8 */
    if (g_rmi_pdt.f54_query.has_variance_metric)
        reg_addr += 2;

    /* data 9 */
    if (g_rmi_pdt.f54_query.has_multi_metric_state_machine)
        reg_addr += 2;

    /* data 10 */
    if (g_rmi_pdt.f54_query.has_multi_metric_state_machine |
            g_rmi_pdt.f54_query.has_noise_state)
        reg_addr++;

    /* data 11 */
    if (g_rmi_pdt.f54_query.has_status)
        reg_addr++;

    /* data 12 */
    if (g_rmi_pdt.f54_query.has_slew_metric)
        reg_addr += 2;

    /* data 13 */
    if (g_rmi_pdt.f54_query.has_multi_metric_state_machine)
        reg_addr += 2;

    /* data 14 */
    if (g_rmi_pdt.f54_query_13.has_cidim)
        reg_addr++;

    /* data 15 */
    if (g_rmi_pdt.f54_query_13.has_rail_im)
        reg_addr++;

    /* data 16 */
    if (g_rmi_pdt.f54_query_13.has_noise_mitigation_enhancement)
        reg_addr++;

    /* data 17 */
    if (g_rmi_pdt.f54_query_16.has_data17)
        reg_addr++;

    /* data 18 */
    if (g_rmi_pdt.f54_query_21.has_query24_data18)
        reg_addr++;

    /* data 19 */
    if (g_rmi_pdt.f54_query_21.has_data19)
        reg_addr++;

    /* data_20 */
    if (g_rmi_pdt.f54_query_25.has_ctrl109)
        reg_addr++;

    /* data 21 */
    if (g_rmi_pdt.f54_query_27.has_data21)
        reg_addr++;

    /* data 22 */
    if (g_rmi_pdt.f54_query_27.has_data22)
        reg_addr++;

    /* data 23 */
    if (g_rmi_pdt.f54_query_29.has_data23)
        reg_addr++;

    /* data 24 */
    if (g_rmi_pdt.f54_query_32.has_data24)
        reg_addr++;

    /* data 25 */
    if (g_rmi_pdt.f54_query_35.has_data25)
        reg_addr++;

    /* data 26 */
    if (g_rmi_pdt.f54_query_35.has_data26)
        reg_addr++;

    /* data 27 */
    if (g_rmi_pdt.f54_query_46.has_data27)
        reg_addr++;

    /* data 28 */
    if (g_rmi_pdt.f54_query_46.has_data28)
        reg_addr++;

    /* data 29 30 reserved */

    /* data 31 */
    if (g_rmi_pdt.f54_query_49.has_data31) {
        g_rmi_pdt.f54_data_31.address = reg_addr;
        // reg_addr++;
    }

}
/*
 * Function:  rmi_parse_f54_control
 * --------------------
 * parse rmi function 54 control registers
 *
 * return: n\a
 */
void rmi_parse_f54_control(void)
{
    int retval;
    unsigned char length = 0;
    unsigned short reg_addr = g_rmi_pdt.F54.control_base_addr;
    unsigned char num_of_sensing_freqs = g_rmi_pdt.number_of_sensing_frequencies;

    /* control 0 */
    reg_addr += F54_CONTROL_0_SIZE;

    /* control 1 */
    if ((g_rmi_pdt.f54_query.touch_controller_family == 0) ||
        (g_rmi_pdt.f54_query.touch_controller_family == 1))
        reg_addr += F54_CONTROL_1_SIZE;

    /* control 2 */
    g_rmi_pdt.f54_control.reg_2.address = reg_addr;
    reg_addr += F54_CONTROL_2_SIZE;

    /* control 3 */
    if (g_rmi_pdt.f54_query.has_pixel_touch_threshold_adjustment)
        reg_addr += F54_CONTROL_3_SIZE;

    /* controls 4 5 6 */
    if ((g_rmi_pdt.f54_query.touch_controller_family == 0) ||
        (g_rmi_pdt.f54_query.touch_controller_family == 1))
        reg_addr += F54_CONTROL_4_6_SIZE;

    /* control 7 */
    if (g_rmi_pdt.f54_query.touch_controller_family == 1) {
        g_rmi_pdt.f54_control.reg_7.address = reg_addr;
        reg_addr += F54_CONTROL_7_SIZE;
    }

    /* controls 8 9 */
    if ((g_rmi_pdt.f54_query.touch_controller_family == 0) ||
        (g_rmi_pdt.f54_query.touch_controller_family == 1))
        reg_addr += F54_CONTROL_8_9_SIZE;

    /* control 10 */
    if (g_rmi_pdt.f54_query.has_interference_metric)
        reg_addr += F54_CONTROL_10_SIZE;

    /* control 11 */
    if (g_rmi_pdt.f54_query.has_ctrl11)
        reg_addr += F54_CONTROL_11_SIZE;

    /* controls 12 13 */
    if (g_rmi_pdt.f54_query.has_relaxation_control)
        reg_addr += F54_CONTROL_12_13_SIZE;

    /* controls 14 15 16 */
    if (g_rmi_pdt.f54_query.has_sensor_assignment) {
        reg_addr += F54_CONTROL_14_SIZE;
        reg_addr += F54_CONTROL_15_SIZE * g_rmi_pdt.f54_query.num_of_rx_electrodes;
        reg_addr += F54_CONTROL_16_SIZE * g_rmi_pdt.f54_query.num_of_tx_electrodes;
    }

    /* controls 17 18 19 */
    if (g_rmi_pdt.f54_query.has_sense_frequency_control) {
        reg_addr += F54_CONTROL_17_SIZE * num_of_sensing_freqs;
        reg_addr += F54_CONTROL_18_SIZE * num_of_sensing_freqs;
        reg_addr += F54_CONTROL_19_SIZE * num_of_sensing_freqs;
    }

    /* control 20 */
    reg_addr += F54_CONTROL_20_SIZE;

    /* control 21 */
    if (g_rmi_pdt.f54_query.has_sense_frequency_control)
        reg_addr += F54_CONTROL_21_SIZE;

    /* controls 22 23 24 25 26 */
    if (g_rmi_pdt.f54_query.has_firmware_noise_mitigation)
        reg_addr += F54_CONTROL_22_26_SIZE;

    /* control 27 */
    if (g_rmi_pdt.f54_query.has_iir_filter)
        reg_addr += F54_CONTROL_27_SIZE;

    /* control 28 */
    if (g_rmi_pdt.f54_query.has_firmware_noise_mitigation)
        reg_addr += F54_CONTROL_28_SIZE;

    /* control 29 */
    if (g_rmi_pdt.f54_query.has_cmn_removal)
        reg_addr += F54_CONTROL_29_SIZE;

    /* control 30 */
    if (g_rmi_pdt.f54_query.has_cmn_maximum)
        reg_addr += F54_CONTROL_30_SIZE;

    /* control 31 */
    if (g_rmi_pdt.f54_query.has_touch_hysteresis)
        reg_addr += F54_CONTROL_31_SIZE;

    /* controls 32 33 34 35 */
    if (g_rmi_pdt.f54_query.has_edge_compensation)
        reg_addr += F54_CONTROL_32_35_SIZE;

    /* control 36 */
    if ((g_rmi_pdt.f54_query.curve_compensation_mode == 1) ||
        (g_rmi_pdt.f54_query.curve_compensation_mode == 2)) {
        if (g_rmi_pdt.f54_query.curve_compensation_mode == 1) {
            length = (g_rmi_pdt.f54_query.num_of_rx_electrodes > g_rmi_pdt.f54_query.num_of_tx_electrodes)?
                     g_rmi_pdt.f54_query.num_of_rx_electrodes : g_rmi_pdt.f54_query.num_of_tx_electrodes;
        } else if (g_rmi_pdt.f54_query.curve_compensation_mode == 2) {
            length = g_rmi_pdt.f54_query.num_of_rx_electrodes;
        }
        reg_addr += F54_CONTROL_36_SIZE * length;
    }

    /* control 37 */
    if (g_rmi_pdt.f54_query.curve_compensation_mode == 2)
        reg_addr += F54_CONTROL_37_SIZE * g_rmi_pdt.f54_query.num_of_tx_electrodes;

    /* controls 38 39 40 */
    if (g_rmi_pdt.f54_query.has_per_frequency_noise_control) {
        reg_addr += F54_CONTROL_38_SIZE * num_of_sensing_freqs;
        reg_addr += F54_CONTROL_39_SIZE * num_of_sensing_freqs;
        reg_addr += F54_CONTROL_40_SIZE * num_of_sensing_freqs;
    }

    /* control 41 */
    if (g_rmi_pdt.f54_query.has_signal_clarity) {
        g_rmi_pdt.f54_control.reg_41.address = reg_addr;
        reg_addr += F54_CONTROL_41_SIZE;
    }

    /* control 42 */
    if (g_rmi_pdt.f54_query.has_variance_metric)
        reg_addr += F54_CONTROL_42_SIZE;

    /* controls 43 44 45 46 47 48 49 50 51 52 53 54 */
    if (g_rmi_pdt.f54_query.has_multi_metric_state_machine)
        reg_addr += F54_CONTROL_43_54_SIZE;

    /* controls 55 56 */
    if (g_rmi_pdt.f54_query.has_0d_relaxation_control)
        reg_addr += F54_CONTROL_55_56_SIZE;

    /* control 57 */
    if (g_rmi_pdt.f54_query.has_0d_acquisition_control) {
        g_rmi_pdt.f54_control.reg_57.address = reg_addr;
        reg_addr += F54_CONTROL_57_SIZE;
    }

    /* control 58 */
    if (g_rmi_pdt.f54_query.has_0d_acquisition_control)
        reg_addr += F54_CONTROL_58_SIZE;

    /* control 59 */
    if (g_rmi_pdt.f54_query.has_h_blank)
        reg_addr += F54_CONTROL_59_SIZE;

    /* controls 60 61 62 */
    if ((g_rmi_pdt.f54_query.has_h_blank) ||
        (g_rmi_pdt.f54_query.has_v_blank) ||
        (g_rmi_pdt.f54_query.has_long_h_blank))
        reg_addr += F54_CONTROL_60_62_SIZE;

    /* control 63 */
    if ((g_rmi_pdt.f54_query.has_h_blank) ||
        (g_rmi_pdt.f54_query.has_v_blank) ||
        (g_rmi_pdt.f54_query.has_long_h_blank) ||
        (g_rmi_pdt.f54_query.has_slew_metric) ||
        (g_rmi_pdt.f54_query.has_slew_option) ||
        (g_rmi_pdt.f54_query.has_noise_mitigation2))
        reg_addr += F54_CONTROL_63_SIZE;

    /* controls 64 65 66 67 */
    if (g_rmi_pdt.f54_query.has_h_blank)
        reg_addr += F54_CONTROL_64_67_SIZE * 7;
    else if ((g_rmi_pdt.f54_query.has_v_blank) ||
             (g_rmi_pdt.f54_query.has_long_h_blank))
        reg_addr += F54_CONTROL_64_67_SIZE;

    /* controls 68 69 70 71 72 73 */
    if ((g_rmi_pdt.f54_query.has_h_blank) ||
        (g_rmi_pdt.f54_query.has_v_blank) ||
        (g_rmi_pdt.f54_query.has_long_h_blank))
        reg_addr += F54_CONTROL_68_73_SIZE;

    /* control 74 */
    if (g_rmi_pdt.f54_query.has_slew_metric)
        reg_addr += F54_CONTROL_74_SIZE;

    /* control 75 */
    if (g_rmi_pdt.f54_query.has_enhanced_stretch)
        reg_addr += F54_CONTROL_75_SIZE * num_of_sensing_freqs;

    /* control 76 */
    if (g_rmi_pdt.f54_query.has_startup_fast_relaxation)
        reg_addr += F54_CONTROL_76_SIZE;

    /* controls 77 78 */
    if (g_rmi_pdt.f54_query.has_esd_control)
        reg_addr += F54_CONTROL_77_78_SIZE;

    /* controls 79 80 81 82 83 */
    if (g_rmi_pdt.f54_query.has_noise_mitigation2)
        reg_addr += F54_CONTROL_79_83_SIZE;

    /* controls 84 85 */
    if (g_rmi_pdt.f54_query.has_energy_ratio_relaxation)
        reg_addr += F54_CONTROL_84_85_SIZE;

    /* control 86 */
    if (g_rmi_pdt.f54_query_13.has_ctrl86) {
        g_rmi_pdt.f54_control.reg_86.address = reg_addr;
        retval = rmi_read_reg(g_rmi_pdt.f54_control.reg_86.address,
                              g_rmi_pdt.f54_control.reg_86.data,
                              sizeof(g_rmi_pdt.f54_control.reg_86.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_control_86 (0x%x)\n", __func__, g_rmi_pdt.f54_control.reg_86.address);
            return;
        }
        reg_addr += F54_CONTROL_86_SIZE;
    }

    /* control 87 */
    if (g_rmi_pdt.f54_query_13.has_ctrl87)
        reg_addr += F54_CONTROL_87_SIZE;

    /* control 88 */
    if (g_rmi_pdt.f54_query.has_ctrl88) {
        g_rmi_pdt.f54_control.reg_88.address = reg_addr;
        reg_addr += F54_CONTROL_88_SIZE;
    }

    /* control 89 */
    if (g_rmi_pdt.f54_query_13.has_cidim ||
            g_rmi_pdt.f54_query_13.has_noise_mitigation_enhancement ||
            g_rmi_pdt.f54_query_13.has_rail_im)
        reg_addr += F54_CONTROL_89_SIZE;

    /* control 90 */
    if (g_rmi_pdt.f54_query_15.has_ctrl90)
        reg_addr += F54_CONTROL_90_SIZE;

    /* control 91 */
    if (g_rmi_pdt.f54_query_21.has_ctrl91) {
        g_rmi_pdt.f54_control.reg_91.address = reg_addr;
        reg_addr += F54_CONTROL_91_SIZE;
    }

    /* control 92 */
    if (g_rmi_pdt.f54_query_16.has_ctrl92)
        reg_addr += F54_CONTROL_92_SIZE;

    /* control 93 */
    if (g_rmi_pdt.f54_query_16.has_ctrl93)
        reg_addr += F54_CONTROL_93_SIZE;

    /* control 94 */
    if (g_rmi_pdt.f54_query_16.has_ctrl94_query18)
        reg_addr += F54_CONTROL_94_SIZE;

    /* control 95 */
    if (g_rmi_pdt.f54_query_16.has_ctrl95_query19) {
        g_rmi_pdt.f54_control_reg95_offset = reg_addr;
        reg_addr += F54_CONTROL_95_SIZE;;
    }


    /* control 96 */
    if (g_rmi_pdt.f54_query_21.has_ctrl96) {
        g_rmi_pdt.f54_control.reg_96.address = reg_addr;
        reg_addr += F54_CONTROL_96_SIZE;
    }

    /* control 97 */
    if (g_rmi_pdt.f54_query_21.has_ctrl97)
        reg_addr += F54_CONTROL_97_SIZE;

    /* control 98 */
    if (g_rmi_pdt.f54_query_21.has_ctrl98)
        reg_addr += F54_CONTROL_98_SIZE;

    /* control 99 */
    if (g_rmi_pdt.f54_query.touch_controller_family == 2) {
        g_rmi_pdt.f54_control.reg_99.address = reg_addr;
        reg_addr += F54_CONTROL_99_SIZE;
    }

    /* control 100 */
    if (g_rmi_pdt.f54_query_16.has_ctrl100)
        reg_addr += F54_CONTROL_100_SIZE;

    /* control 101 */
    if (g_rmi_pdt.f54_query_22.has_ctrl101)
        reg_addr += F54_CONTROL_101_SIZE;


    /* control 102 */
    if (g_rmi_pdt.f54_query_23.has_ctrl102)
        reg_addr += F54_CONTROL_102_SIZE;

    /* control 103 */
    if (g_rmi_pdt.f54_query_22.has_ctrl103_query26) {
        g_rmi_pdt.f54_skip_preparation = true;
        reg_addr += F54_CONTROL_103_SIZE;
    }

    /* control 104 */
    if (g_rmi_pdt.f54_query_22.has_ctrl104)
        reg_addr += F54_CONTROL_104_SIZE;

    /* control 105 */
    if (g_rmi_pdt.f54_query_22.has_ctrl105)
        reg_addr += F54_CONTROL_105_SIZE;

    /* control 106 */
    if (g_rmi_pdt.f54_query_25.has_ctrl106)
        reg_addr += F54_CONTROL_106_SIZE;

    /* control 107 */
    if (g_rmi_pdt.f54_query_25.has_ctrl107)
        reg_addr += F54_CONTROL_107_SIZE;

    /* control 108 */
    if (g_rmi_pdt.f54_query_25.has_ctrl108)
        reg_addr += F54_CONTROL_108_SIZE;

    /* control 109 */
    if (g_rmi_pdt.f54_query_25.has_ctrl109)
        reg_addr += F54_CONTROL_109_SIZE;

    /* control 110 */
    if (g_rmi_pdt.f54_query_27.has_ctrl110) {
        g_rmi_pdt.f54_control.reg_110.address = reg_addr;
        reg_addr += F54_CONTROL_110_SIZE;
    }

    /* control 111 */
    if (g_rmi_pdt.f54_query_27.has_ctrl111)
        reg_addr += F54_CONTROL_111_SIZE;

    /* control 112 */
    if (g_rmi_pdt.f54_query_27.has_ctrl112)
        reg_addr += F54_CONTROL_112_SIZE;

    /* control 113 */
    if (g_rmi_pdt.f54_query_27.has_ctrl113)
        reg_addr += F54_CONTROL_113_SIZE;

    /* control 114 */
    if (g_rmi_pdt.f54_query_27.has_ctrl114)
        reg_addr += F54_CONTROL_114_SIZE;

    /* control 115 */
    if (g_rmi_pdt.f54_query_29.has_ctrl115)
        reg_addr += F54_CONTROL_115_SIZE;

    /* control 116 */
    if (g_rmi_pdt.f54_query_29.has_ctrl116)
        reg_addr += F54_CONTROL_116_SIZE;

    /* control 117 */
    if (g_rmi_pdt.f54_query_29.has_ctrl117)
        reg_addr += F54_CONTROL_117_SIZE;

    /* control 118 */
    if (g_rmi_pdt.f54_query_30.has_ctrl118)
        reg_addr += F54_CONTROL_118_SIZE;

    /* control 119 */
    if (g_rmi_pdt.f54_query_30.has_ctrl119)
        reg_addr += F54_CONTROL_119_SIZE;

    /* control 120 */
    if (g_rmi_pdt.f54_query_30.has_ctrl120)
        reg_addr += F54_CONTROL_120_SIZE;

    /* control 121 */
    if (g_rmi_pdt.f54_query_30.has_ctrl121)
        reg_addr += F54_CONTROL_121_SIZE;

    /* control 122 */
    if (g_rmi_pdt.f54_query_30.has_ctrl122_query31)
        reg_addr += F54_CONTROL_122_SIZE;

    /* control 123 */
    if (g_rmi_pdt.f54_query_30.has_ctrl123)
        reg_addr += F54_CONTROL_123_SIZE;

    /* control 124 */
    if (g_rmi_pdt.f54_query_30.has_ctrl124)
        reg_addr += F54_CONTROL_124_SIZE;

    /* control 125 */
    if (g_rmi_pdt.f54_query_32.has_ctrl125)
        reg_addr += F54_CONTROL_125_SIZE;

    /* control 126 */
    if (g_rmi_pdt.f54_query_32.has_ctrl126)
        reg_addr += F54_CONTROL_126_SIZE;

    /* control 127 */
    if (g_rmi_pdt.f54_query_32.has_ctrl127)
        reg_addr += F54_CONTROL_127_SIZE;

    /* control 128 */
    if (g_rmi_pdt.f54_query_33.has_ctrl128)
        reg_addr += F54_CONTROL_128_SIZE;

    /* control 129 */
    if (g_rmi_pdt.f54_query_33.has_ctrl129)
        reg_addr += F54_CONTROL_129_SIZE;

    /* control 130 */
    if (g_rmi_pdt.f54_query_33.has_ctrl130)
        reg_addr += F54_CONTROL_130_SIZE;

    /* control 131 */
    if (g_rmi_pdt.f54_query_33.has_ctrl131)
        reg_addr += F54_CONTROL_131_SIZE;

    /* control 132 */
    if (g_rmi_pdt.f54_query_33.has_ctrl132)
        reg_addr += F54_CONTROL_132_SIZE;

    /* control 133 */
    if (g_rmi_pdt.f54_query_33.has_ctrl133)
        reg_addr += F54_CONTROL_133_SIZE;

    /* control 134 */
    if (g_rmi_pdt.f54_query_33.has_ctrl134)
        reg_addr += F54_CONTROL_134_SIZE;

    /* control 135 */
    if (g_rmi_pdt.f54_query_35.has_ctrl135)
        reg_addr += F54_CONTROL_135_SIZE;

    /* control 136 */
    if (g_rmi_pdt.f54_query_35.has_ctrl136)
        reg_addr += F54_CONTROL_136_SIZE;

    /* control 137 */
    if (g_rmi_pdt.f54_query_35.has_ctrl137)
        reg_addr += F54_CONTROL_137_SIZE;

    /* control 138 */
    if (g_rmi_pdt.f54_query_35.has_ctrl138)
        reg_addr += F54_CONTROL_138_SIZE;

    /* control 139 */
    if (g_rmi_pdt.f54_query_35.has_ctrl139)
        reg_addr += F54_CONTROL_139_SIZE;

    /* control 140 */
    if (g_rmi_pdt.f54_query_35.has_ctrl140)
        reg_addr += F54_CONTROL_140_SIZE;

    /* control 141 */
    if (g_rmi_pdt.f54_query_36.has_ctrl141)
        reg_addr += F54_CONTROL_141_SIZE;

    /* control 142 */
    if (g_rmi_pdt.f54_query_36.has_ctrl142)
        reg_addr += F54_CONTROL_142_SIZE;

    /* control 143 */
    if (g_rmi_pdt.f54_query_36.has_ctrl143)
        reg_addr += F54_CONTROL_143_SIZE;

    /* control 144 */
    if (g_rmi_pdt.f54_query_36.has_ctrl144)
        reg_addr += F54_CONTROL_144_SIZE;

    /* control 145 */
    if (g_rmi_pdt.f54_query_36.has_ctrl145)
        reg_addr += F54_CONTROL_145_SIZE;

    /* control 146 */
    if (g_rmi_pdt.f54_query_36.has_ctrl146)
        reg_addr += F54_CONTROL_146_SIZE;

    /* control 147 */
    if (g_rmi_pdt.f54_query_38.has_ctrl147)
        reg_addr += F54_CONTROL_147_SIZE;

    /* control 148 */
    if (g_rmi_pdt.f54_query_38.has_ctrl148)
        reg_addr += F54_CONTROL_148_SIZE;

    /* control 149 */
    if (g_rmi_pdt.f54_query_38.has_ctrl149) {
        g_rmi_pdt.f54_control.reg_149.address = reg_addr;
        reg_addr += F54_CONTROL_149_SIZE;
    }

    /* control 150 */
    if (g_rmi_pdt.f54_query_38.has_ctrl150)
        reg_addr += F54_CONTROL_150_SIZE;

    /* control 151 */
    if (g_rmi_pdt.f54_query_38.has_ctrl151)
        reg_addr += F54_CONTROL_151_SIZE;

    /* control 152 */
    if (g_rmi_pdt.f54_query_38.has_ctrl152)
        reg_addr += F54_CONTROL_152_SIZE;

    /* control 153 */
    if (g_rmi_pdt.f54_query_38.has_ctrl153)
        reg_addr += F54_CONTROL_153_SIZE;

    /* control 154 */
    if (g_rmi_pdt.f54_query_39.has_ctrl154)
        reg_addr += F54_CONTROL_154_SIZE;

    /* control 155 */
    if (g_rmi_pdt.f54_query_39.has_ctrl155)
        reg_addr += F54_CONTROL_155_SIZE;

    /* control 156 */
    if (g_rmi_pdt.f54_query_39.has_ctrl156)
        reg_addr += F54_CONTROL_156_SIZE;

    /* controls 157 158 */
    if (g_rmi_pdt.f54_query_39.has_ctrl157_ctrl158)
        reg_addr += F54_CONTROL_157_158_SIZE;

    /* controls 159 to 162 reserved */

    /* control 163 */
    if (g_rmi_pdt.f54_query_40.has_ctrl163_query41)
        reg_addr += F54_CONTROL_163_SIZE;

    /* control 164 reserved */

    /* control 165 */
    if (g_rmi_pdt.f54_query_40.has_ctrl165_query42)
        reg_addr += F54_CONTROL_165_SIZE;

    /* control 166 */
    if (g_rmi_pdt.f54_query_40.has_ctrl166)
        reg_addr += F54_CONTROL_166_SIZE;

    /* control 167 */
    if (g_rmi_pdt.f54_query_40.has_ctrl167)
        reg_addr += F54_CONTROL_167_SIZE;

    /* control 168 */
    if (g_rmi_pdt.f54_query_40.has_ctrl168)
        reg_addr += F54_CONTROL_168_SIZE;

    /* control 169 */
    if (g_rmi_pdt.f54_query_40.has_ctrl169)
        reg_addr += F54_CONTROL_169_SIZE;

    /* control 170 reserved */

    /* control 171 */
    if (g_rmi_pdt.f54_query_43.has_ctrl171)
        reg_addr += F54_CONTROL_171_SIZE;

    /* control 172 */
    if (g_rmi_pdt.f54_query_43.has_ctrl172_query44_query45)
        reg_addr += F54_CONTROL_172_SIZE;

    /* control 173 */
    if (g_rmi_pdt.f54_query_43.has_ctrl173)
        reg_addr += F54_CONTROL_173_SIZE;

    /* control 174 */
    if (g_rmi_pdt.f54_query_43.has_ctrl174)
        reg_addr += F54_CONTROL_174_SIZE;

    /* control 175 */
    if (g_rmi_pdt.f54_query_43.has_ctrl175)
        reg_addr += F54_CONTROL_175_SIZE;

    /* control 176 */
    if (g_rmi_pdt.f54_query_46.has_ctrl176)
        reg_addr += F54_CONTROL_176_SIZE;

    /* controls 177 178 */
    if (g_rmi_pdt.f54_query_46.has_ctrl177_ctrl178)
        reg_addr += F54_CONTROL_177_178_SIZE;

    /* control 179 */
    if (g_rmi_pdt.f54_query_46.has_ctrl179)
        reg_addr += F54_CONTROL_179_SIZE;

    /* controls 180 to 181 reserved */

    /* control 182 */
    if (g_rmi_pdt.f54_query_47.has_ctrl182) {
        g_rmi_pdt.f54_control.reg_182.address = reg_addr;
        reg_addr += F54_CONTROL_182_SIZE;
    }

    /* control 183 */
    if (g_rmi_pdt.f54_query_47.has_ctrl183)
        reg_addr += F54_CONTROL_183_SIZE;

    /* control 184 reserved */

    /* control 185 */
    if (g_rmi_pdt.f54_query_47.has_ctrl185)
        reg_addr += F54_CONTROL_185_SIZE;

    /* control 186 */
    if (g_rmi_pdt.f54_query_47.has_ctrl186)
        reg_addr += F54_CONTROL_186_SIZE;

    /* control 187 */
    if (g_rmi_pdt.f54_query_47.has_ctrl187)
        reg_addr += F54_CONTROL_187_SIZE;

    /* control 188 */
    if (g_rmi_pdt.f54_query_49.has_ctrl188) {
        g_rmi_pdt.f54_control.reg_188.address = reg_addr;
        reg_addr += F54_CONTROL_188_SIZE;
    }

    /* control 189 - 195 reserved */

    /* control 196 */
    if (g_rmi_pdt.f54_query_51.has_ctrl196)
        reg_addr += F54_CONTROL_196_SIZE;

    /* control 197 - 217 reserved */

    /* control 218 reserved */
    if (g_rmi_pdt.f54_query_61.has_ctrl218)
        reg_addr += F54_CONTROL_218_SIZE;

    /* control 219 - 222 reserved */

    /* control 223 reserved */
    if (g_rmi_pdt.f54_query_64.has_ctrl103_sub3) {
        g_rmi_pdt.f54_control.reg_223.address = reg_addr;
        // reg_addr += F54_CONTROL_223_SIZE;
    }
}
/*
 * Function:  rmi_parse_f54_information
 * --------------------
 * parse rmi function 54 query registers and
 * store the necessary information
 *
 * return: n\a
 */
void rmi_parse_f54_information(void)
{
    int retval;
    unsigned char offset;

    /* query 0 - 12 */
    retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr,
                          g_rmi_pdt.f54_query.data,
                          sizeof(g_rmi_pdt.f54_query.data));
    if (retval < 0) {
        printf_e("%s: fail to read f54_query (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr);
        return;
    }
    offset = sizeof(g_rmi_pdt.f54_query.data);

    /* query 12 */
    if (g_rmi_pdt.f54_query.has_sense_frequency_control == 0)
        offset -= 1;

    /* query 13 */
    if (g_rmi_pdt.f54_query.has_query13) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_13.data,
                              sizeof(g_rmi_pdt.f54_query_13.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_13 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 14 */
    if (g_rmi_pdt.f54_query_13.has_ctrl87)
        offset += 1;

    /* query 15 */
    if (g_rmi_pdt.f54_query.has_query15) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_15.data,
                              sizeof(g_rmi_pdt.f54_query_15.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_15 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 16 */
    if (g_rmi_pdt.f54_query_15.has_query16) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_16.data,
                              sizeof(g_rmi_pdt.f54_query_16.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_16 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 17 */
    if (g_rmi_pdt.f54_query_16.has_query17) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              &g_rmi_pdt.number_of_sensing_frequencies,
                              1);
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_16 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        printf_i("%s: total gears support = %d\n", __func__, g_rmi_pdt.number_of_sensing_frequencies);
        offset += 1;
    }

    /* query 18 */
    if (g_rmi_pdt.f54_query_16.has_ctrl94_query18)
        offset += 1;

    /* query 19 */
    if (g_rmi_pdt.f54_query_16.has_ctrl95_query19)
        offset += 1;

    /* query 20 */
    if (g_rmi_pdt.f54_query_15.has_query20)
        offset += 1;

    /* query 21 */
    if (g_rmi_pdt.f54_query_15.has_query21) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_21.data,
                              sizeof(g_rmi_pdt.f54_query_21.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_21 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 22 */
    if (g_rmi_pdt.f54_query_15.has_query22) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_22.data,
                              sizeof(g_rmi_pdt.f54_query_22.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_22 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 23 */
    if (g_rmi_pdt.f54_query_22.has_query23) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_23.data,
                              sizeof(g_rmi_pdt.f54_query_23.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_23 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 24 */
    if (g_rmi_pdt.f54_query_21.has_query24_data18)
        offset += 1;

    /* query 25 */
    if (g_rmi_pdt.f54_query_15.has_query25) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_25.data,
                              sizeof(g_rmi_pdt.f54_query_25.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_25 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 26 */
    if (g_rmi_pdt.f54_query_22.has_ctrl103_query26)
        offset += 1;

    /* query 27 */
    if (g_rmi_pdt.f54_query_25.has_query27) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_27.data,
                              sizeof(g_rmi_pdt.f54_query_27.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_27 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 28 */
    if (g_rmi_pdt.f54_query_22.has_query28)
        offset += 1;

    /* query 29 */
    if (g_rmi_pdt.f54_query_27.has_query29) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_29.data,
                              sizeof(g_rmi_pdt.f54_query_29.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_29 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 30 */
    if (g_rmi_pdt.f54_query_29.has_query30) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_30.data,
                              sizeof(g_rmi_pdt.f54_query_30.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_30 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 31 */
    if (g_rmi_pdt.f54_query_30.has_ctrl122_query31)
        offset += 1;

    /* query 32 */
    if (g_rmi_pdt.f54_query_30.has_query32) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_32.data,
                              sizeof(g_rmi_pdt.f54_query_32.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_32 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 33 */
    if (g_rmi_pdt.f54_query_32.has_query33) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_33.data,
                              sizeof(g_rmi_pdt.f54_query_33.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_33 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 34 */
    if (g_rmi_pdt.f54_query_32.has_query34)
        offset += 1;

    /* query 35 */
    if (g_rmi_pdt.f54_query_32.has_query35) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_35.data,
                              sizeof(g_rmi_pdt.f54_query_35.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_35 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 36 */
    if (g_rmi_pdt.f54_query_33.has_query36) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_36.data,
                              sizeof(g_rmi_pdt.f54_query_36.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_36 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 37 */
    if (g_rmi_pdt.f54_query_36.has_query37)
        offset += 1;

    /* query 38 */
    if (g_rmi_pdt.f54_query_36.has_query38) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_38.data,
                              sizeof(g_rmi_pdt.f54_query_38.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_38 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 39 */
    if (g_rmi_pdt.f54_query_38.has_query39) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_39.data,
                              sizeof(g_rmi_pdt.f54_query_39.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_39 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 40 */
    if (g_rmi_pdt.f54_query_39.has_query40) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_40.data,
                              sizeof(g_rmi_pdt.f54_query_40.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_40 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 41 */
    if (g_rmi_pdt.f54_query_40.has_ctrl163_query41)
        offset += 1;

    /* query 42 */
    if (g_rmi_pdt.f54_query_40.has_ctrl165_query42)
        offset += 1;

    /* query 43 */
    if (g_rmi_pdt.f54_query_40.has_query43) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_43.data,
                              sizeof(g_rmi_pdt.f54_query_43.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_43 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    if (g_rmi_pdt.f54_query_43.has_ctrl172_query44_query45)
        offset += 2;

    /* query 46 */
    if (g_rmi_pdt.f54_query_43.has_query46) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_46.data,
                              sizeof(g_rmi_pdt.f54_query_46.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_46 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 47 */
    if (g_rmi_pdt.f54_query_46.has_query47) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_47.data,
                              sizeof(g_rmi_pdt.f54_query_47.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_47 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 48 reserved */

    /* query 49 */
    if (g_rmi_pdt.f54_query_47.has_query49) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_49.data,
                              sizeof(g_rmi_pdt.f54_query_49.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_49 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 50 */
    if (g_rmi_pdt.f54_query_49.has_query50) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_50.data,
                              sizeof(g_rmi_pdt.f54_query_50.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_50 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 51 */
    if (g_rmi_pdt.f54_query_50.has_query51) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_51.data,
                              sizeof(g_rmi_pdt.f54_query_51.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_51 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* tddi f54 test reporting +  */

    /* query 52 reserved */

    /* queries 53 54 */
    if (g_rmi_pdt.f54_query_51.has_query53_query54_ctrl198)
        offset += 2;

    /* query 55 */
    if (g_rmi_pdt.f54_query_51.has_query55) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_55.data,
                              sizeof(g_rmi_pdt.f54_query_55.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_55 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 56 */
    if (g_rmi_pdt.f54_query_55.has_query56)
        offset += 1;

    /* query 57 */
    if (g_rmi_pdt.f54_query_55.has_query57) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_57.data,
                              sizeof(g_rmi_pdt.f54_query_57.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_57 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 58 */
    if (g_rmi_pdt.f54_query_57.has_query58) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_58.data,
                              sizeof(g_rmi_pdt.f54_query_58.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_58 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 59 */
    if (g_rmi_pdt.f54_query_58.has_query59)
        offset += 1;

    /* queries 60 */
    if (g_rmi_pdt.f54_query_58.has_query60)
        offset += 1;

    /* queries 61 */
    if (g_rmi_pdt.f54_query_58.has_query61) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_61.data,
                              sizeof(g_rmi_pdt.f54_query_61.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_61 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 62 63 */
    if (g_rmi_pdt.f54_query_61.has_ctrl215_query62_query63)
        offset += 2;

    /* queries 64 */
    if (g_rmi_pdt.f54_query_61.has_query64) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_64.data,
                              sizeof(g_rmi_pdt.f54_query_64.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_64 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 65 */
    if (g_rmi_pdt.f54_query_64.has_query65) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_65.data,
                              sizeof(g_rmi_pdt.f54_query_65.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_65 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 66 */
    if (g_rmi_pdt.f54_query_65.has_query66_ctrl231)
        offset += 1;

    /* queries 67 */
    if (g_rmi_pdt.f54_query_65.has_query67) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_67.data,
                              sizeof(g_rmi_pdt.f54_query_67.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_67 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 68 */
    if (g_rmi_pdt.f54_query_67.has_query68) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_68.data,
                              sizeof(g_rmi_pdt.f54_query_68.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_68 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 69 */
    if (g_rmi_pdt.f54_query_68.has_query69) {
        retval = rmi_read_reg(g_rmi_pdt.F54.query_base_addr + offset,
                              g_rmi_pdt.f54_query_69.data,
                              sizeof(g_rmi_pdt.f54_query_69.data));
        if (retval < 0) {
            printf_e("%s: fail to read f54_query_68 (0x%x)\n", __func__, g_rmi_pdt.F54.query_base_addr + offset);
            return;
        }
        // offset += 1;
    }

    g_rmi_pdt.tx_assigned = g_rmi_pdt.f54_query.num_of_tx_electrodes;
    g_rmi_pdt.rx_assigned = g_rmi_pdt.f54_query.num_of_rx_electrodes;

    /* parse f54 control register */
    rmi_parse_f54_control();
    /* parse f54 data registers */
    rmi_parse_f54_data();
}
/*
 * Function:  rmi_parse_f55_control
 * --------------------
 * parse rmi function 55 control registers
 *
 * return: n\a
 */
void rmi_parse_f55_control(void)
{
    unsigned char offset = 0;

    /* controls 0 1 2 */
    if (g_rmi_pdt.f55_query.has_sensor_assignment)
        offset += 3;

    /* control 3 */
    if (g_rmi_pdt.f55_query.has_edge_compensation)
        offset++;

    /* control 4 */
    if (g_rmi_pdt.f55_query.curve_compensation_mode == 0x1 ||
            g_rmi_pdt.f55_query.curve_compensation_mode == 0x2)
        offset++;

    /* control 5 */
    if (g_rmi_pdt.f55_query.curve_compensation_mode == 0x2)
        offset++;

    /* control 6 */
    if (g_rmi_pdt.f55_query.has_ctrl6)
        offset++;

    /* control 7 */
    if (g_rmi_pdt.f55_query.has_alternate_transmitter_assignment)
        offset++;

    /* control 8 */
    if (g_rmi_pdt.f55_query_3.has_ctrl8)
        offset++;

    /* control 9 */
    if (g_rmi_pdt.f55_query_3.has_ctrl9)
        offset++;

    /* control 10 */
    if (g_rmi_pdt.f55_query_5.has_corner_compensation)
        offset++;

    /* control 11 */
    if (g_rmi_pdt.f55_query.curve_compensation_mode == 0x3)
        offset++;

    /* control 12 */
    if (g_rmi_pdt.f55_query_5.has_ctrl12)
        offset++;

    /* control 13 */
    if (g_rmi_pdt.f55_query_5.has_ctrl13)
        offset++;

    /* control 14 */
    if (g_rmi_pdt.f55_query_5.has_ctrl14)
        offset++;

    /* control 15 */
    if (g_rmi_pdt.f55_query_5.has_basis_function)
        offset++;

    /* control 16 */
    if (g_rmi_pdt.f55_query_17.has_ctrl16)
        offset++;

    /* control 17 */
    if (g_rmi_pdt.f55_query_17.has_ctrl17)
        offset++;

    /* controls 18 19 */
    if (g_rmi_pdt.f55_query_17.has_ctrl18_ctrl19)
        offset += 2;

    /* control 20 */
    if (g_rmi_pdt.f55_query_17.has_ctrl20)
        offset++;

    /* control 21 */
    if (g_rmi_pdt.f55_query_17.has_ctrl21)
        offset++;

    /* control 22 */
    if (g_rmi_pdt.f55_query_17.has_ctrl22)
        offset++;

    /* control 23 */
    if (g_rmi_pdt.f55_query_18.has_ctrl23)
        offset++;

    /* control 24 */
    if (g_rmi_pdt.f55_query_18.has_ctrl24)
        offset++;

    /* control 25 */
    if (g_rmi_pdt.f55_query_18.has_ctrl25)
        offset++;

    /* control 26 */
    if (g_rmi_pdt.f55_query_18.has_ctrl26)
        offset++;

    /* control 27 */
    if (g_rmi_pdt.f55_query_18.has_ctrl27_query20)
        offset++;

    /* control 28 */
    if (g_rmi_pdt.f55_query_18.has_ctrl28_query21)
        offset++;

    /* control 29 */
    if (g_rmi_pdt.f55_query_22.has_ctrl29)
        offset++;

    /* control 30 */
    if (g_rmi_pdt.f55_query_22.has_ctrl30)
        offset++;

    /* control 31 */
    if (g_rmi_pdt.f55_query_22.has_ctrl31)
        offset++;

    /* control 32 */
    if (g_rmi_pdt.f55_query_22.has_ctrl32)
        offset++;

    /* controls 33 34 35 36 reserved */

    /* control 37 */
    if (g_rmi_pdt.f55_query_28.has_ctrl37)
        offset++;

    /* control 38 */
    if (g_rmi_pdt.f55_query_30.has_ctrl38)
        offset++;

    /* control 39 */
    if (g_rmi_pdt.f55_query_30.has_ctrl39)
        offset++;

    /* control 40 */
    if (g_rmi_pdt.f55_query_30.has_ctrl40)
        offset++;

    /* control 41 */
    if (g_rmi_pdt.f55_query_30.has_ctrl41)
        offset++;

    /* control 42 */
    if (g_rmi_pdt.f55_query_30.has_ctrl42)
        offset++;

    /* controls 43 44 */
    if (g_rmi_pdt.f55_query_30.has_ctrl43_ctrl44) {
        g_rmi_pdt.afe_mux_offset = offset;
        // offset += 2;
    }

}
/*
 * Function:  rmi_parse_f55_information
 * --------------------
 * parse rmi function 55 query registers and
 * store the necessary information
 *
 * return: n\a
 */
void rmi_parse_f55_information(void)
{
    int retval;
    unsigned char offset;
    unsigned char rx_electrodes;
    unsigned char tx_electrodes;
    struct f55_control_43 ctrl_43;
    int ii;

    /* query 0 - 2 */
    retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr,
                          g_rmi_pdt.f55_query.data,
                          sizeof(g_rmi_pdt.f55_query.data));
    if (retval < 0) {
        printf_e("%s: fail to read f55_query (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr);
        return;
    }
    offset = sizeof(g_rmi_pdt.f55_query.data);

    /* query 3 */
    if (g_rmi_pdt.f55_query.has_single_layer_multi_touch) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_3.data,
                              sizeof(g_rmi_pdt.f55_query_3.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_3 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 4 */
    if (g_rmi_pdt.f55_query_3.has_ctrl9)
        offset += 1;

    /* query 5 */
    if (g_rmi_pdt.f55_query.has_query5) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_5.data,
                              sizeof(g_rmi_pdt.f55_query_5.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_5 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 6 7 */
    if (g_rmi_pdt.f55_query.curve_compensation_mode == 0x3)
        offset += 2;

    /* query 8 */
    if (g_rmi_pdt.f55_query_3.has_ctrl8)
        offset += 1;

    /* query 9 */
    if (g_rmi_pdt.f55_query_3.has_query9)
        offset += 1;

    /* queries 10 11 12 13 14 15 16 */
    if (g_rmi_pdt.f55_query_5.has_basis_function)
        offset += 7;

    /* query 17 */
    if (g_rmi_pdt.f55_query_5.has_query17) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_17.data,
                              sizeof(g_rmi_pdt.f55_query_17.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_17 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 18 */
    if (g_rmi_pdt.f55_query_17.has_query18) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_18.data,
                              sizeof(g_rmi_pdt.f55_query_18.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_18 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 19 */
    if (g_rmi_pdt.f55_query_18.has_query19)
        offset += 1;

    /* query 20 */
    if (g_rmi_pdt.f55_query_18.has_ctrl27_query20)
        offset += 1;

    /* query 21 */
    if (g_rmi_pdt.f55_query_18.has_ctrl28_query21)
        offset += 1;

    /* query 22 */
    if (g_rmi_pdt.f55_query_18.has_query22) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_22.data,
                              sizeof(g_rmi_pdt.f55_query_22.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_22 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 23 */
    if (g_rmi_pdt.f55_query_22.has_query23) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_23.data,
                              sizeof(g_rmi_pdt.f55_query_23.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_23 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        offset += 1;

        g_rmi_pdt.amp_sensor = g_rmi_pdt.f55_query_23.amp_sensor_enabled;
        g_rmi_pdt.size_of_column2mux = g_rmi_pdt.f55_query_23.size_of_column2mux;
    }

    /* queries 24 25 26 27 reserved */

    /* query 28 */
    if (g_rmi_pdt.f55_query_22.has_query28) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_28.data,
                              sizeof(g_rmi_pdt.f55_query_28.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_28 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* query 29 */
    if (g_rmi_pdt.f55_query_28.has_query29)
        offset += 1;

    /* query 30 */
    if (g_rmi_pdt.f55_query_28.has_query30) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_30.data,
                              sizeof(g_rmi_pdt.f55_query_30.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_30 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        offset += 1;
    }

    /* queries 31 32 */
    if (g_rmi_pdt.f55_query_30.has_query31_query32)
        offset += 2;

    /* query 33 */
    if (g_rmi_pdt.f55_query_30.has_query33) {
        retval = rmi_read_reg(g_rmi_pdt.F55.query_base_addr + offset,
                              g_rmi_pdt.f55_query_33.data,
                              sizeof(g_rmi_pdt.f55_query_33.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55_query_33 (0x%x)\n", __func__, g_rmi_pdt.F55.query_base_addr + offset);
            return;
        }
        // offset += 1;

        g_rmi_pdt.extended_amp = g_rmi_pdt.f55_query_33.has_extended_amp_pad;
        g_rmi_pdt.extended_amp_btn = g_rmi_pdt.f55_query_33.has_extended_amp_btn;
    }

    if (!g_rmi_pdt.f55_query.has_sensor_assignment) {
        printf_e("%s: no sensor assignment defined\n", __func__);
        return;
    }

    /* parse f55 control register */
    rmi_parse_f55_control();

    tx_electrodes = g_rmi_pdt.f55_query.num_of_tx_electrodes;
    rx_electrodes = g_rmi_pdt.f55_query.num_of_rx_electrodes;

    /* tx assignment */
    if (tx_electrodes > MAX_SENSOR_MAP_SIZE){
        printf_e("%s: unexpected tx electrodes = %d", __func__, tx_electrodes);
        return;
    }
    retval = rmi_read_reg((unsigned short)(g_rmi_pdt.F55.control_base_addr + SENSOR_TX_MAPPING_OFFSET),
                          g_rmi_pdt.tx_assignment,
                          tx_electrodes);
    if (retval < 0) {
        printf_e("%s: fail to read f55 tx assignment", __func__);
        return;
    }
    /* rx assignment */
    if (rx_electrodes > MAX_SENSOR_MAP_SIZE){
        printf_e("%s: unexpected rx electrodes = %d", __func__, rx_electrodes);
        return;
    }
    retval = rmi_read_reg((unsigned short)(g_rmi_pdt.F55.control_base_addr + SENSOR_RX_MAPPING_OFFSET),
                          g_rmi_pdt.rx_assignment,
                          rx_electrodes);
    if (retval < 0) {
        printf_e("%s: fail to read f55 rx assignment", __func__);
        return;
    }

    g_rmi_pdt.tx_assigned = 0;
    for (ii = 0; ii < tx_electrodes; ii++) {
        if (g_rmi_pdt.tx_assignment[ii] != 0xff)
            g_rmi_pdt.tx_assigned++;
    }

    g_rmi_pdt.rx_assigned = 0;
    for (ii = 0; ii < rx_electrodes; ii++) {
        if (g_rmi_pdt.rx_assignment[ii] != 0xff)
            g_rmi_pdt.rx_assigned++;
    }

    if (g_rmi_pdt.amp_sensor) {
        g_rmi_pdt.tx_assigned = g_rmi_pdt.size_of_column2mux;
        g_rmi_pdt.rx_assigned /= 2;
    }

    if (g_rmi_pdt.extended_amp) {
        retval = rmi_read_reg(g_rmi_pdt.F55.control_base_addr + g_rmi_pdt.afe_mux_offset,
                              ctrl_43.data,
                              sizeof(ctrl_43.data));
        if (retval < 0) {
            printf_e("%s: fail to read f55 afe mux sizes", __func__);
            return;
        }

        g_rmi_pdt.tx_assigned = ctrl_43.afe_l_mux_size + ctrl_43.afe_r_mux_size;

        g_rmi_pdt.swap_sensor_side = ctrl_43.swap_sensor_side;
        g_rmi_pdt.left_mux_size = ctrl_43.afe_l_mux_size;
        g_rmi_pdt.right_mux_size = ctrl_43.afe_r_mux_size;
    }

}

/*
 * Function:  scan_rmi_pdt
 * --------------------
 * scan and parse the PDT to determine
 * if that f54 and f 55 function exists.
 * yes, collect the information in f54 and f55
 *
 * return: 0 - succeed
 *         otherwise, fail
 */
int rmi_scan_pdt(void)
{
    char err[MAX_STRING_LEN * 2];
    unsigned short address;
    unsigned char buffer[6];
    unsigned char page = 0;
    unsigned char pageAddress = 0;

    if (rmi_is_initialized) {
        printf_i("%s: rmi has been initialized\n", __func__);
        return 0;
    }

    memset(&g_rmi_pdt.F01, 0, sizeof(struct FunctionDescriptor));
    memset(&g_rmi_pdt.F12, 0, sizeof(struct FunctionDescriptor));
    memset(&g_rmi_pdt.F1A, 0, sizeof(struct FunctionDescriptor));
    memset(&g_rmi_pdt.F34, 0, sizeof(struct FunctionDescriptor));
    memset(&g_rmi_pdt.F54, 0, sizeof(struct FunctionDescriptor));
    memset(&g_rmi_pdt.F55, 0, sizeof(struct FunctionDescriptor));

    /* Scan Page Description Table (pdt) to find all RMI functions presented by this device.
     * The Table starts at $00EE. This and every sixth register (decrementing) is a function number
     * except when this "function number" is $00, meaning end of pdt.
     * In an actual use case this scan might be done only once on first run or before compile.
     */
    for (page = 0x0; page < 6; page++) {
        for (pageAddress = 0xe9; pageAddress > 0xc0; pageAddress -= 6) {
            address = (page << 8) | pageAddress;
            rmi_read_reg(address, buffer, sizeof(buffer));

            if (buffer[5] == 0x01){
                g_rmi_pdt.F01.query_base_addr = (page << 8) | buffer[0];
                g_rmi_pdt.F01.command_base_addr = (page << 8) | buffer[1];
                g_rmi_pdt.F01.control_base_addr = (page << 8) | buffer[2];
                g_rmi_pdt.F01.data_base_addr = (page << 8) | buffer[3];
                g_rmi_pdt.F01.Version = (unsigned char) ((buffer[4] >> 5) & 0x03);
                g_rmi_pdt.F01.InterruptSourceCount = (unsigned char) (buffer[4] & 0x07);
                g_rmi_pdt.F01.ID = buffer[5];
            }
            else if (buffer[5] == 0x11){
                g_rmi_pdt.F11.query_base_addr = (page << 8) | buffer[0];
                g_rmi_pdt.F11.command_base_addr = (page << 8) | buffer[1];
                g_rmi_pdt.F11.control_base_addr = (page << 8) | buffer[2];
                g_rmi_pdt.F11.data_base_addr = (page << 8) | buffer[3];
                g_rmi_pdt.F11.Version = (unsigned char) ((buffer[4] >> 5) & 0x03);
                g_rmi_pdt.F11.InterruptSourceCount = (unsigned char) (buffer[4] & 0x07);
                g_rmi_pdt.F11.ID = buffer[5];
            }
            else if (buffer[5] == 0x12){
                g_rmi_pdt.F12.query_base_addr = (page << 8) | buffer[0];
                g_rmi_pdt.F12.command_base_addr = (page << 8) | buffer[1];
                g_rmi_pdt.F12.control_base_addr = (page << 8) | buffer[2];
                g_rmi_pdt.F12.data_base_addr = (page << 8) | buffer[3];
                g_rmi_pdt.F12.Version = (unsigned char) ((buffer[4] >> 5) & 0x03);
                g_rmi_pdt.F12.InterruptSourceCount = (unsigned char) (buffer[4] & 0x07);
                g_rmi_pdt.F12.ID = buffer[5];
            }
            else if (buffer[5] == 0x1A){
                g_rmi_pdt.F1A.query_base_addr = (page << 8) | buffer[0];
                g_rmi_pdt.F1A.command_base_addr = (page << 8) | buffer[1];
                g_rmi_pdt.F1A.control_base_addr = (page << 8) | buffer[2];
                g_rmi_pdt.F1A.data_base_addr = (page << 8) | buffer[3];
                g_rmi_pdt.F1A.Version = (unsigned char) ((buffer[4] >> 5) & 0x03);
                g_rmi_pdt.F1A.InterruptSourceCount = (unsigned char) (buffer[4] & 0x07);
                g_rmi_pdt.F1A.ID = buffer[5];
            }
            else if (buffer[5] == 0x34){
                g_rmi_pdt.F34.query_base_addr = (page << 8) | buffer[0];
                g_rmi_pdt.F34.command_base_addr = (page << 8) | buffer[1];
                g_rmi_pdt.F34.control_base_addr = (page << 8) | buffer[2];
                g_rmi_pdt.F34.data_base_addr = (page << 8) | buffer[3];
                g_rmi_pdt.F34.Version = (unsigned char) ((buffer[4] >> 5) & 0x03);
                g_rmi_pdt.F34.InterruptSourceCount = (unsigned char) (buffer[4] & 0x07);
                g_rmi_pdt.F34.ID = buffer[5];
            }
            else if (buffer[5] == 0x54){
                // address = (page << 8) | buffer[0];
                g_rmi_pdt.F54.query_base_addr = (page << 8) | buffer[0];
                g_rmi_pdt.F54.command_base_addr = (page << 8) | buffer[1];
                g_rmi_pdt.F54.control_base_addr = (page << 8) | buffer[2];
                g_rmi_pdt.F54.data_base_addr = (page << 8) | buffer[3];
                g_rmi_pdt.F54.Version = (unsigned char) ((buffer[4] >> 5) & 0x03);
                g_rmi_pdt.F54.InterruptSourceCount = (unsigned char) (buffer[4] & 0x07);
                g_rmi_pdt.F54.ID = buffer[5];
            }
            else if (buffer[5] == 0x55){
                g_rmi_pdt.F55.query_base_addr = (page << 8) | buffer[0];
                g_rmi_pdt.F55.command_base_addr = (page << 8) | buffer[1];
                g_rmi_pdt.F55.control_base_addr = (page << 8) | buffer[2];
                g_rmi_pdt.F55.data_base_addr = (page << 8) | buffer[3];
                g_rmi_pdt.F55.Version = (unsigned char) ((buffer[4] >> 5) & 0x03);
                g_rmi_pdt.F55.InterruptSourceCount = (unsigned char) (buffer[4] & 0x07);
                g_rmi_pdt.F55.ID = buffer[5];
            }
            else if (buffer[5] == 0x00) {
                /* no function in this page, go to next page */
                break;
            }
        }
    }

    if (g_rmi_pdt.F01.ID != 0x01) {
        printf_e("%s: F$01 is not found, exit\n", __func__);

        sprintf(err, "%s error: F$01 is not found, exit\n", __func__);
        sprintf(err_msg_out, "%s error: F$01 is not found\n", __func__);
        return -1;
    }
    if (g_rmi_pdt.F54.ID != 0x54){
        printf_e("%s: F$54 is not found, exit\n", __func__);

        sprintf(err, "%s error: F$54 is not found, exit\n", __func__);
        sprintf(err_msg_out, "%s error: F$54 is not found\n", __func__);
        return -1;
    }

    /* prepare all necessary information in f01 */
    rmi_parse_f01_information();
    /* prepare all necessary information in f54 */
    rmi_parse_f54_information();
    /* prepare all necessary information in f55 */
    rmi_parse_f55_information();
    /* find all available gears for later using */
    rmi_f54_scan_freq_gear();

    /* try to read the finger classifier info from f12 */
    if (g_rmi_pdt.F12.ID == 0x12){
        rmi_parse_f12_information();
    }

    /* indicate the 0d button existed if f1a is found */
    if (g_rmi_pdt.F1A.ID == 0x1a){
        g_rmi_pdt.has_button = true;
    }
    else {
        g_rmi_pdt.has_button = false;
    }

    /* read and save saturation capacitance */
    memset(buffer, 0x00, sizeof(buffer));
    rmi_read_reg(g_rmi_pdt.f54_control.reg_2.address, buffer, 2);
    g_rmi_pdt.saturation_cap = ((unsigned short)buffer[0]) | ((unsigned short)buffer[1] << 8);


    /* set flag to true to indicate the pdt has been parsed */
    rmi_is_initialized = true;

    return 0;
}
/*
 * Function:  rmi_f01_sw_reset
 * --------------------
 * issue a software reset command to function 01 commadn register
 * input report type
 *
 * return: 0<, fail to issue a sw reset
 *         otherwise, succeed
 */
int rmi_f01_sw_reset()
{
    int retval;
    unsigned char command = 0x01;

    printf_i("rmi_f01_sw_reset");

    retval = rmi_write_reg(g_rmi_pdt.F01.command_base_addr, &command, sizeof(command));
    if (retval < 0)
        printf_e("%s: fail to do sw reset\n", __func__);

    usleep(RMI_SW_RESET_DELAY_MS * 1000);  // delay 500 ms
    return retval;
}
/*
 * Function:  rmi_f01_set_no_sleep
 * --------------------
 * to configure the rmi device into no_sleep
 * mode
 *
 * return: 0<, fail to change to no_sleep mode
 *         otherwise, succeed
 */
int rmi_f01_set_no_sleep()
{
    int retval;
    unsigned char device_ctrl;

    retval = rmi_read_reg(g_rmi_pdt.F01.control_base_addr,
                          &device_ctrl,
                          sizeof(device_ctrl));
    if (retval < 0) {
        printf_e("%s: fail to read device ctrl\n", __func__);
        return retval;
    }

    device_ctrl |= RMI_NO_SLEEP_ON;

    retval = rmi_write_reg(g_rmi_pdt.F01.control_base_addr,
                           &device_ctrl,
                           sizeof(device_ctrl));
    if (retval < 0) {
        printf_e("%s: fail to set no sleep\n", __func__);
        return retval;
    }

    return retval;
}

/*
 * Function:  rmi_f54_read_report
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
int rmi_f54_read_report(int report_type)
{
    int retval = 0;
    int time_count = 0;
    unsigned char fifo_idx_data[2] = {0,0};
    unsigned char cmd_data;

    /* step 1 */
    /* check the get report flag at first */
    retval = rmi_read_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
    if (retval < 0) {
        printf_e("%s: fail to check F54 command base register\n", __func__);
        sprintf(err_msg_out, "%s error: fail to check F54 command base register(0x%x)\n",
                __func__, g_rmi_pdt.F54.command_base_addr);
        goto exit;
    }
    if (cmd_data & 0x01) {
        printf_e("%s error: F54_CMD(bit 0) GetReport = 1 (data=0x%04x)\n", __func__, cmd_data);
        sprintf(err_msg_out, "%s F54_CMD(bit 0) GetReport is not cleared (data=0x%04x)\n",
                __func__, cmd_data);
        retval = -EIO;
        goto exit;
    }

    /* step 2 */
    /* set report type to F54 data register */
    cmd_data = (unsigned char)(report_type);
    retval = rmi_write_reg(g_rmi_pdt.F54.data_base_addr, &cmd_data, sizeof(cmd_data));
    if (retval < 0) {
        printf_e("%s: fail to set report type to F54 data register\n", __func__);
        sprintf(err_msg_out, "%s error: fail to set report type to F54 data register(reg_addr=0x%x, write_data=0x%x)\n",
                __func__, g_rmi_pdt.F54.data_base_addr, cmd_data);
        goto exit;
    }

    /* step 3 */
    /* set fifo index */
    retval = rmi_write_reg((unsigned short) (g_rmi_pdt.F54.data_base_addr + 1), fifo_idx_data, sizeof(fifo_idx_data));
    if (retval < 0) {
        printf_e("%s: fail to set fifo index register\n", __func__);
        sprintf(err_msg_out, "%s error: fail to set fifo index register(reg_addr=0x%x, write_data=0x%x, 0x%x)\n",
                __func__, (g_rmi_pdt.F54.data_base_addr + 1), fifo_idx_data[0], fifo_idx_data[1]);
        goto exit;
    }

    /* step 4 */
    /* get report, setting the get report flag to '1' to request a report image reading
     * wait until the flag has been cleared to '0' */
    cmd_data = RMI_COMMAND_GET_REPORT;
    retval = rmi_write_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
    if (retval < 0) {
        printf_e("%s: fail to set get report flag\n", __func__);
        sprintf(err_msg_out, "%s error: fail to set get_report flag to 1(reg_addr=0x%x, write_data=0x%x)\n",
                __func__, g_rmi_pdt.F54.command_base_addr, cmd_data);
        goto exit;
    }

    do {
        usleep(50000);  // polling every 50 ms
        retval = rmi_read_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
        if (retval < 0) {
            printf_e("%s: fail to read F54 command base register\n", __func__);
            sprintf(err_msg_out, "%s error: fail to read F54_CMD base register(addr=0x%x) in polling\n",
                    __func__, g_rmi_pdt.F54.command_base_addr);
            retval = -ETIMEDOUT;
            goto exit;
        }
        time_count += 1;
    } while ((cmd_data & 0x01) && (time_count < RMI_GET_REPORT_TIMEOUT));

    if (time_count == RMI_GET_REPORT_TIMEOUT) {
        printf_e("%s: timeout! fail to get report image (rt %d) (f54_cmd: %x)\n",
                 __func__, report_type, cmd_data);
        sprintf(err_msg_out, "%s error: timeout! fail to get report image (rt %d)\n",
                __func__, report_type);
        retval = -ETIMEDOUT;
        goto exit;
    }
    exit:
    return retval;
}

/*
 * Function:  rmi_f54_read_report_image
 * --------------------
 * get the report image according to the
 * input report type
 * the expected output will be an entire frame, size = image_col*image_row
 *
 * parameter
 *  report_type: report type
 *               which should be defined in enum F54_REPORT_TYPES
 *  p_image: a buffer to store the output image
 *  image_size: size of report image
 *
 * return: < 0, fail to get report
 *         otherwise, succeed
 */
int rmi_f54_read_report_image(int report_type, short *p_image, int image_col, int image_row)
{
    int retval = 0;
    short *p_data_16;
    int image_size = image_col * image_row * sizeof(short);
    unsigned char *data_buf = NULL;
    int i, j, offset;

    if (!p_image) {
        printf_e("%s: p_image is NULL\n", __func__);
        retval = -EINVAL;
        goto exit;
    }
    /* create a local buffer to fill the report data */
    data_buf = malloc(sizeof(unsigned char) * image_size);
    if (!data_buf) {
        printf_e("%s error: can't allocate memory for data_buf\n", __func__);
        sprintf(err_msg_out, "%s: can't allocate memory for data_buf\n", __func__);
        retval = -ENOMEM;
        goto exit;
    }
    memset(data_buf, 0x00, (size_t)image_size);

    /* move step 1 - 4 to a function */
    retval = rmi_f54_read_report(report_type);
    if (retval < 0) {
        printf_e("%s: fail to request the report %d \n", __func__, report_type);
        sprintf(err_msg_out, "%s fail to request the report %d \n", __func__, report_type);
        goto exit;
    }

    /* step 5 */
    /* get report data */
    retval = rmi_read_reg((unsigned short) (g_rmi_pdt.F54.data_base_addr + RMI_REPROT_DATA_OFFSET), data_buf, image_size);
    if (retval < 0) {
        printf_e("%s: fail to get report data \n", __func__);
        sprintf(err_msg_out, "%s error: fail to get report data (reg_addr=0x%x, size=%d)\n",
                __func__, (g_rmi_pdt.F54.data_base_addr + RMI_REPROT_DATA_OFFSET), image_size);
        goto exit;
    }

    p_data_16 = (short *)&data_buf[0];
    for(i = 0; i < g_rmi_pdt.tx_assigned; i++) {
        for(j = 0; j < g_rmi_pdt.rx_assigned; j++) {

            if (g_rmi_pdt.rx_assigned < g_rmi_pdt.tx_assigned)
                offset = i*g_rmi_pdt.rx_assigned + j;
            else
                offset = j*g_rmi_pdt.tx_assigned + i;

            p_image[offset] = *p_data_16;

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
 * Function:  rmi_get_build_id
 * --------------------
 * return the firmware build id, which is parsed from the PDT
 */
int rmi_get_build_id(void)
{
    return g_rmi_pdt.build_id;
}
/*
 * Function:  rmi_get_asic_type
 * --------------------
 * return the asic type, which is parsed from the PDT
 */
int rmi_get_asic_type(void)
{
    int id;
    char* asic = (char *)&g_rmi_pdt.asic_type[0];
    sscanf(asic, "%d", &id);

    return id;
}

/*
 * Function:  rmi_get_image_cols
 * --------------------
 * return number of column, typically it will be the short side
 */
int rmi_get_image_cols(void)
{
    return (g_rmi_pdt.tx_assigned < g_rmi_pdt.rx_assigned)? g_rmi_pdt.tx_assigned : g_rmi_pdt.rx_assigned ;
}
/*
 * Function:  rmi_get_image_rows
 * --------------------
 * return number of row, typically it will be the long side
 */
int rmi_get_image_rows(void)
{
    return (g_rmi_pdt.tx_assigned > g_rmi_pdt.rx_assigned)? g_rmi_pdt.tx_assigned : g_rmi_pdt.rx_assigned ;
}
/*
 * Function:  rmi_get_finger_cap
 * --------------------
 * return the default capacitance being reported as fingre object
 * it is calculated by the following formula
 *      Saturation Capacitance * Finger Amplitude Threshold
 */
int rmi_get_finger_cap(void)
{
    int finger_delta = 0;
    printf_i("%s: saturation_cap = %d, finger_threshold = %d\n", __func__, g_rmi_pdt.saturation_cap, g_rmi_pdt.finger_threshold);
    finger_delta = g_rmi_pdt.saturation_cap * g_rmi_pdt.finger_threshold / 256;

    return finger_delta;
}
/*
 * Function:  read_report_entry
 * --------------------
 * the entry to read report image
 */
int read_rmi_f54_report(int type, short *buf)
{
    int retval = -1;
    int tx_num = g_rmi_pdt.tx_assigned;
    int rx_num = g_rmi_pdt.rx_assigned;

    if(!buf) {
        printf_e("%s: invalid parameter\n", __FUNCTION__);
        sprintf(err_msg_out, "%s error: invalid parameter. data buffer is null\n", __FUNCTION__);

        retval = -1;
        goto exit;
    }

    switch(type){
        case 2: /* report type = 2 - normalized 16-bit image report */
        case 3: /* report type = 3 - raw 16-bit image report */
            /* get report image */
            retval = rmi_f54_read_report_image(type, buf, rmi_get_image_cols(), rmi_get_image_rows());
            if (retval < 0) {
                printf_e("%s: fail to get report type %d from func54 (retval=%d)\n", __FUNCTION__, type, retval);
                goto exit;
            }

            break;
        default:
            printf_e("%s: unknown input type (type=%d)\n", __FUNCTION__, type);
            sprintf(err_msg_out, "%s error: unknown input type (type= %d)\n", __FUNCTION__, type);
            break;
    }

exit:

    if (retval < 0) {
        sprintf(err_msg_out, "%s%s error: fail to read report image %d (size=%d)\n",
                err_msg_out, __FUNCTION__, type, (tx_num * rx_num * 2));
    }

    return retval;
}

/*
 * Function:  rmi_f54_force_update
 * --------------------
 * issue a force update command through
 * f54 command register
 *
 * return: < 0, fail to issue force update
 *         otherwise, succeed
 */
int rmi_f54_force_update()
{
    int retval = 0;
    unsigned char cmd_data;
    int time_count = 0;

    cmd_data = RMI_COMMAND_FORCE_UPDATE;

    retval = rmi_write_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
    if (retval < 0) {
        printf_e("%s: fail to set force update flag\n", __func__);
        sprintf(err_msg_out, "%s error: fail to write force_update command(reg_addr=0x%x, write_data=0x%x)\n",
                __func__, g_rmi_pdt.F54.command_base_addr, cmd_data);
        goto exit;
    }

    usleep(100000); // delay 100 ms before polling the flag

    do {
        retval = rmi_read_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
        if (retval < 0) {
            printf_e("%s: fail to read command register\n", __func__);

            sprintf(err_msg_out, "%s error: fail to read F54_CMD base register(addr=0x%x) in polling\n",
                    __func__, g_rmi_pdt.F54.command_base_addr);
            goto exit;
        }

        if ((cmd_data & 0x04) == 0x00)
            break;

        usleep(100000);  // polling every 100 ms
        time_count += 1;

    } while (time_count < RMI_COMMAND_TIMEOUT);

    if (time_count == RMI_COMMAND_TIMEOUT) {
        printf_e("%s: timeout! fail to set command (f54_cmd data=0x%x)\n", __func__, cmd_data);
        sprintf(err_msg_out, "%s error: timeout! fail to do force_update (f54_cmd=0x%x)\n",
                __func__, cmd_data);
        retval = -ETIMEDOUT;
        goto exit;
    }

    exit:
    return retval;
}
/*
 * Function:  rmi_f54_force_cal
 * --------------------
 * issue a force cal command through
 * f54 command register
 *
 * return: < 0, fail to issue force cal
 *         otherwise, succeed
 */
int rmi_f54_force_cal()
{
    int retval = 0;
    unsigned char cmd_data;
    int time_count = 0;

    cmd_data = RMI_COMMAND_FORCE_CAL;

    retval = rmi_write_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
    if (retval < 0) {
        printf_e("%s: fail to set force update flag\n", __func__);
        sprintf(err_msg_out, "%s error: fail to write force_cal command\nreg_addr=0x%x, write_data=0x%x\n",
                __func__, g_rmi_pdt.F54.command_base_addr, cmd_data);
        goto exit;
    }

    usleep(100000); // delay 100 ms before polling the flag

    do {
        retval = rmi_read_reg(g_rmi_pdt.F54.command_base_addr, &cmd_data, sizeof(cmd_data));
        if (retval < 0) {
            printf_e("%s: fail to read command register\n", __func__);

            sprintf(err_msg_out, "%s error: fail to read F54_CMD base register(addr=0x%x) in polling\n",
                    __func__, g_rmi_pdt.F54.command_base_addr);
            goto exit;
        }

        if ((cmd_data & 0x04) == 0x00)
            break;

        usleep(100000);  // polling every 100 ms
        time_count += 1;

    } while (time_count < RMI_COMMAND_TIMEOUT);

    if (time_count == RMI_COMMAND_TIMEOUT) {
        printf_e("%s: timeout! fail to set command (f54_cmd data=0x%x)\n", __func__, cmd_data);
        sprintf(err_msg_out, "%s error: timeout! fail to do force_cal (f54_cmd=0x%x)\n",
                __func__, cmd_data);
        retval = -ETIMEDOUT;
        goto exit;
    }

exit:
    return retval;
}

/*
 * Function:  rmi_f54_scan_freq_gear
 * --------------------
 * helper function to find available frequency gear
 * set 1 to FreqGearEn[] if it is enabled
 *
 */
int rmi_f54_scan_freq_gear()
{
    int retval = 0;
    int size = sizeof(struct F54FreqCtrl);
    unsigned char freqCtrl[g_rmi_pdt.number_of_sensing_frequencies * size];
    int idx;

    if (g_rmi_pdt.number_of_sensing_frequencies == 0) {
        printf_i("%s: invalid parameter\n", __func__);
        return -EINVAL;
    }

    printf_i("%s: total gears = %d\n", __func__, g_rmi_pdt.number_of_sensing_frequencies);
    g_rmi_available_gears = 0;

    retval = rmi_read_reg(g_rmi_pdt.f54_control_reg95_offset, freqCtrl, sizeof(freqCtrl));
    if (retval < 0) {
        printf_e("%s: fail to read F54Ctr95 register\n", __func__);
        sprintf(err_msg_out, "%s error: fail to read F54_ctrl95 register (addr = 0x%x)\ntotal gears defined in F54_Query17 = %d, size to read = %d\n",
                __func__, g_rmi_pdt.f54_control_reg95_offset, g_rmi_pdt.number_of_sensing_frequencies, (int)sizeof(freqCtrl));
        goto exit;
    }

    for (idx = 0; idx < g_rmi_pdt.number_of_sensing_frequencies; idx++ ) {
        if ( freqCtrl[idx * size] & 0x80 ) {  /* gear is disabled */
            gear_en[idx] = 0;
        }
        else {  /* gear is enabled */
            gear_en[idx] = 1;
            g_rmi_available_gears++;
        }
    }
    printf_i("%s: number of available gear = %d\n", __func__, g_rmi_available_gears);

exit:
    return retval;
}
/*
 * Function:  rmi_f54_enable_one_gear
 * --------------------
 * helper function to enable the specified gear and disable others
 */
int rmi_f54_enable_one_gear(int gear)
{
    int retval = 0;
    int size = sizeof(struct F54FreqCtrl);
    unsigned char freqCtrl[g_rmi_pdt.number_of_sensing_frequencies * size];

    retval = rmi_read_reg(g_rmi_pdt.f54_control_reg95_offset, freqCtrl, sizeof(freqCtrl));
    if (retval < 0) {
        printf_e("%s: fail to read F54Ctr95 register\n", __func__);
        sprintf(err_msg_out, "%s error: fail to read F54_ctrl95 register (addr=0x%x)\n"
                        "total gears defined in F54_Query17 = %d, size to read = %d\n",
                __func__, g_rmi_pdt.f54_control_reg95_offset,
                g_rmi_pdt.number_of_sensing_frequencies, (int)sizeof(freqCtrl));
        goto exit;
    }

    for (int i = 0; i < g_rmi_pdt.number_of_sensing_frequencies; i++ ) {
        if (i == gear) { /* enable, set bit-7 to 0 */
            freqCtrl[i * size] = (unsigned char) (freqCtrl[i * size] & 0x7F);
        }
        else { /* disable, set bit-7 to 1 */
            freqCtrl[i * size] = (unsigned char) (freqCtrl[i * size] | 0x80);
        }
    }
    retval = rmi_write_reg(g_rmi_pdt.f54_control_reg95_offset, freqCtrl, sizeof(freqCtrl));
    if (retval < 0) {
        printf_e("%s: fail to write data to the f54Ctrl95, try to enable gear %d \n", __func__, gear);
        sprintf(err_msg_out, "%s error: fail to write data to the f54Ctrl95 (addr=0x%x), attend to enable gear %d\n"
                        "total gears defined in F54_Query17 = %d, size to write = %d\n",
                __func__, g_rmi_pdt.f54_control_reg95_offset,
                gear, g_rmi_pdt.number_of_sensing_frequencies, (int)sizeof(freqCtrl));
        goto exit;
    }

    retval = rmi_f54_force_update();
    if (retval < 0) {
        sprintf(err_msg_out + strlen(err_msg_out), "%s error: fail to do force_update\n", __func__);
    }

exit:
    if (retval < 0) {
        sprintf(err_msg_out + strlen(err_msg_out), "%s error: can't change to frequency gear %d \n", __func__, gear);
    }
    else {
        printf_i("%s: change to frequency gear-%d\n", __func__, gear);
    }

    return retval;
}
/*
 * Function:  rmi_get_f54_gear_info
 * --------------------
 * helper function to get the gear information
 */
void rmi_get_f54_gear_info(char *p_info)
{
    int i = 0;

    for (i =0; i < g_rmi_pdt.number_of_sensing_frequencies; i++) {
        if (gear_en[i] == 1) {
            sprintf(p_info, "%sGear %2d\n", p_info, i);
        }
    }
}
/*
 * Function:  rmi_write_log_header
 * --------------------
 * write necessary information to the header of log file
 */
void rmi_write_log_header(FILE *pfile, int total_frames, int num_gears)
{
    long sec;
    time_t t_time_header;

    gettimeofday(&t_val_1, NULL);
    sec = t_val_1.tv_sec;
    t_time_header = (time_t)sec;

    fprintf(pfile, "Synaptics APK     recorded at %s\n\n", ctime(&t_time_header));
    fprintf(pfile, "Interface                : RMI4\n");
    fprintf(pfile, "Device ID                : %4d\n", rmi_get_asic_type());
    fprintf(pfile, "Firmware Packrat ID      : %7d\n", rmi_get_build_id());
    switch(g_report_type){
        case 2:
            fprintf(pfile, "Data Type                : RT 2, delta image\n");
            break;
        case 94:
            fprintf(pfile, "Data Type                : RT 94, AMP abs delta image\n");
            break;
        default:
            break;
    }
    fprintf(pfile, "Tx Channel               : %d\n", g_rmi_pdt.tx_assigned);
    fprintf(pfile, "Rx Channel               : %d\n", g_rmi_pdt.rx_assigned);
    fprintf(pfile, "Total Frames Captured    : %d\n", total_frames);
    fprintf(pfile, "Number of Frames per Gear: %d\n", total_frames/num_gears);
    fprintf(pfile, "Number of Frequency Gears Enabled: %d\n", num_gears);
    fprintf(pfile, "\n");
    fprintf(pfile, "Delta Threshold          : %4d\n", g_threshold);
    fprintf(pfile, "\n\n");
}
/*
 * Function:  rmi_do_noise_test
 * --------------------
 * entry function to perform noise testing
 * to get several delta frames and compare each tixle with threshold
 * fail, if the value is over the threshold
 */
bool rmi_do_noise_test(FILE *pfile, char *timestamp, int frame_id, int gear_idx)
{
    int retval;
    short *rt_image;
    bool result = true;

    printf_i("%s: frame id = %d\n", __FUNCTION__, frame_id);

    /* create buffer to store the report data */
    int tx_num = g_rmi_pdt.tx_assigned;
    int rx_num = g_rmi_pdt.rx_assigned;
    rt_image = (short *) calloc((size_t) (tx_num * rx_num), sizeof(short));
    if(!rt_image) {
        printf_e("%s: fail to allocate the buffer for report image acquisition\n", __FUNCTION__);
        sprintf(err_msg_out, "%s: fail to allocate the buffer for report image acquisition\n", __FUNCTION__);
        result = false;
        goto write_file;
    }

    /* call read_f54_report function to get the RT image */
    retval = read_rmi_f54_report(g_report_type, rt_image);
    if(retval < 0){
        printf_e("%s: fail to read report\n", __FUNCTION__);
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

    for(int i = 0; i < tx_num; i++) {
        if (i == 0) {
            fprintf(pfile, "        ");
            for (int j = 0 ; j < rx_num; j ++)
                fprintf(pfile, "[R%2d]  ", j);
            fprintf(pfile, "\n");
        }
        fprintf(pfile, "[T%2d]: ", i);

        for(int j = 0; j < rx_num; j++) {
            fprintf(pfile, "%6d ", rt_image[i*rx_num+ j]);

            /* determine it pass or fail on this Tixel */
            if (rt_image[i*rx_num+ j] > g_threshold) {
                result = false;
            }

            max_val = (max_val > rt_image[i*rx_num+ j])? max_val : rt_image[i*rx_num+ j];
            min_val = (min_val < rt_image[i*rx_num+ j])? min_val : rt_image[i*rx_num+ j];
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

            for(int i = 0; i < tx_num; i++) {
                if (i == 0) {
                    fprintf(pfile_fail_log, "        ");
                    for (int j = 0 ; j < rx_num; j ++)
                        fprintf(pfile_fail_log, "[R%2d]  ", j);
                    fprintf(pfile_fail_log, "\n");
                }
                fprintf(pfile_fail_log, "[T%2d]: ", i);

                for(int j = 0; j < rx_num; j++) {
                    fprintf(pfile_fail_log, "%6d ", rt_image[i*rx_num+ j]);
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