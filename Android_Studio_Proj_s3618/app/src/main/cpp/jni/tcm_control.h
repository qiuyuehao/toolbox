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

#ifndef _TCM_CONTROL_H__
#define _TCM_CONTROL_H__

#define TCM_POLLING_DELAY_MS (20)
#define TCM_POLLING_TIMOUT (150)  /* 20 (ms) * 150 = 3000 ms = 3s */
#define TCM_RESET_DELAY_MS (250)
#define TCM_ERASE_FLASH_DELAY_MS (500)
#define TCM_WRITE_FLASH_DELAY_MS (200)

#define TCM_TOUCH_CONFIG_SIZE 256
#define TCM_MAX_STATIC_CONFIG_SIZE 7680

#define TCM_FINGERS_TO_SUPPORT 10

#define TCM_MAX_PINS (64)

enum tcm_identify_mode {
    MODE_APPLICATION = 0x01,
    MODE_BOOTLOADER = 0x0B,
    MODE_BOOTLOADER_TDDI = 0x0C,
};

enum tcm_command {
    CMD_NONE = 0x00,
    CMD_CONTINUE_WRITE = 0x01,
    CMD_IDENTIFY = 0x02,
    CMD_DEBUG = 0x03,
    CMD_RESET = 0x04,
    CMD_ENABLE_REPORT = 0x05,
    CMD_DISABLE_REPORT = 0x06,
    CMD_GET_BOOT_INFO = 0x10,
    CMD_ERASE_FLASH = 0x11,
    CMD_WRITE_FLASH = 0x12,
    CMD_READ_FLASH = 0x13,
    CMD_RUN_APPLICATION_FIRMWARE = 0x14,
    CMD_SPI_MASTER_WRITE_THEN_READ = 0x15,
    CMD_REBOOT_TO_ROM_BOOTLOADER = 0x16,
    CMD_RUN_BOOTLOADER_FIRMWARE = 0x1f,
    CMD_GET_APPLICATION_INFO = 0x20,
    CMD_GET_STATIC_CONFIG = 0x21,
    CMD_SET_STATIC_CONFIG = 0x22,
    CMD_GET_DYNAMIC_CONFIG = 0x23,
    CMD_SET_DYNAMIC_CONFIG = 0x24,
    CMD_GET_TOUCH_REPORT_CONFIG = 0x25,
    CMD_SET_TOUCH_REPORT_CONFIG = 0x26,
    CMD_REZERO = 0x27,
    CMD_COMMIT_CONFIG = 0x28,
    CMD_DESCRIBE_DYNAMIC_CONFIG = 0x29,
    CMD_PRODUCTION_TEST = 0x2a,
    CMD_SET_CONFIG_ID = 0x2b,
    CMD_ENTER_DEEP_SLEEP = 0x2c,
    CMD_EXIT_DEEP_SLEEP = 0x2d,
    CMD_GET_TOUCH_INFO = 0x2e,
    CMD_GET_DATA_LOCATION = 0x2f,
    CMD_DOWNLOAD_CONFIG = 0x46,
    CMD_UNLOCK_PRIVATE = 0x80,
    CMD_READ_RAM = 0x81,
    CMD_WRITE_REGISTER = 0x82,
};

enum tcm_report_code {
    TCM_REPORT_IDENTIFY = 0x10,
    TCM_REPORT_TOUCH = 0x11,
    TCM_REPORT_DELTA = 0x12,
    TCM_REPORT_RAW = 0x13,
    TCM_REPORT_PRINTF = 0x82,
    TCM_REPORT_STATUS = 0xc0,
    TCM_REPORT_FORCE = 0xc1,
    TCM_REPORT_HDL = 0xfe,
};

enum tcm_status_code {
    STATUS_IDLE = 0x00,
    STATUS_OK = 0x01,               /* when a command response is ready */
    STATUS_BUSY = 0x02,             /* when a command response is not yet ready */
    STATUS_CONTINUED_READ = 0x03,   /* when a message is split between multiple packets */
    STATUS_COMMAND_STATUS_RECEIVE_BUFFER_OVERFLOW = 0x0C,
    STATUS_COMMAND_PREVIOUS_COMMAND_PENDING = 0x0D,
    STATUS_COMMAND_NOT_IMPLEMENTED = 0x0E,
    STATUS_ERROR = 0x0f,
    STATUS_INVALID = 0xff
};

enum tcm_test_type {
    TEST_TRX_TRX_SHORTS = 0x00,
    TEST_TRX_SENSOR_OPEN = 0x01,
    TEST_TRX_GND_SHORTS = 0x02,
    TEST_TRX_GROUND = 0x03,
    TEST_FULLRAW = 0x05,
    TEST_DRT = 0x07,
    TEST_OPEN_SHORT = 0x08,
    TEST_NOISE = 0x0A,
    TEST_PT11 = 0x0B,
    TEST_PT12 = 0x0C,
    TEST_PT13 = 0x0D,
    TEST_ADC_RANGE = 0x11,
    TEST_ABS_RAW = 0x12,
    TEST_HYBRID_ABS_NOISE = 0x1D,
};

enum tcm_dynamic_config_id {
    DC_UNKNOWN = 0x00,
    DC_NO_DOZE,
    DC_DISABLE_NOISE_MITIGATION,
    DC_INHIBIT_FREQUENCY_SHIFT,
    DC_REQUESTED_FREQUENCY,
    DC_DISABLE_HSYNC,
    DC_REZERO_ON_EXIT_DEEP_SLEEP,
    DC_CHARGER_CONNECTED,
    DC_NO_BASELINE_RELAXATION,
    DC_IN_WAKEUP_GESTURE_MODE,
    DC_STIMULUS_FINGERS,
    DC_GRIP_SUPPRESSION_ENABLED,
    DC_ENABLE_THICK_GLOVE,
    DC_ENABLE_GLOVE,
};

enum tcm_data_area {
    TCM_LCM_DATA = 1,
    TCM_OEM_DATA,
    TCM_PPDT_DATA,
    TCM_FORCE_CALIBRATION_DATA
};


struct tcm_message_header {
    unsigned char marker;
    unsigned char code;
    unsigned char length[2];
};

struct tcm_identify_report {
    unsigned char version;
    unsigned char mode;
    unsigned char part_number[16];
    unsigned char build_id[4];
    unsigned char max_write_size[2];
};

struct tcm_app_info {
    unsigned char version[2];
    unsigned char status[2];
    unsigned char static_config_size[2];
    unsigned char dynamic_config_size[2];
    unsigned char app_config_start_write_block[2];
    unsigned char app_config_size[2];
    unsigned char max_touch_report_config_size[2];
    unsigned char max_touch_report_payload_size[2];
    unsigned char customer_config_id[16];
    unsigned char max_x[2];
    unsigned char max_y[2];
    unsigned char max_objects[2];
    unsigned char num_of_buttons[2];
    unsigned char num_of_image_rows[2];
    unsigned char num_of_image_cols[2];
    unsigned char has_hybrid_data[2];
    unsigned char num_of_force_elecs[2];
};

struct tcm_boot_info {
    unsigned char version;
    unsigned char status;
    unsigned char mask[2];
    unsigned char write_block_size_words;
    unsigned char erase_page_size_words[2];
    unsigned char max_write_payload_size_bytes[2];
    unsigned char last_reset_reason;
    unsigned char pc_at_time_of_last_reset[2];
    unsigned char boot_config_start_block[2];
    unsigned char boot_config_size_blocks[2];
    unsigned char display_config_start_block[4];
    unsigned char display_config_length_blocks[2];
    unsigned char backup_display_config_start_block[4];
    unsigned char backup_display_config_length_blocks[2];
    unsigned char custom_otp_start_block[2];
    unsigned char custom_otp_length_blocks[2];
};

/* define the basic touch structure */
struct tcm_finger_data {
    int classification;
    int timestamp;
    int x;
    int y;
    int z;
    int x_w;
    int y_w;
    int tx_pos;
    int rx_pos;
    int od;
    int gesture_double_tap;
    int prs;
};

/* global tcm structure for syna_dev_manager using */
struct tcm_handler {
    struct tcm_identify_report identify_report;
    struct tcm_app_info app_info_report;
    struct tcm_boot_info boot_info_report;
    unsigned char touch_config[TCM_TOUCH_CONFIG_SIZE];
    unsigned char static_config[TCM_MAX_STATIC_CONFIG_SIZE];

    int current_finger_idx;
    int size_of_finger_report;
    struct tcm_finger_data finger[TCM_FINGERS_TO_SUPPORT];

    int tx_pins[TCM_MAX_PINS];
    int tx_assigned;
    int rx_pins[TCM_MAX_PINS];
    int rx_assigned;
    int guard_pins[TCM_MAX_PINS];
    int guard_assigned;
};

struct tcm_handler g_tcm_handler;


/* helper to detect the valid tcm device node */
bool tcm_find_dev(char *dev_node);
int tcm_open_dev(const char *dev_node);
int tcm_close_dev(const char *dev_node);

/* helper to perform read/write operations */
int tcm_read_message(unsigned char *p_rd_data, unsigned int bytes_to_read);
int tcm_get_payload(unsigned char *p_rd_data, int payload_size);
int tcm_write_message(unsigned char *p_wr_data, unsigned int  bytes_to_write);
int tcm_wait_for_command_ready(void);
int tcm_drop_package(int payload_size);
int tcm_do_reset();
int tcm_set_no_sleep(int state);
int tcm_set_rezero();

/* helper to change the firmware mode */
int tcm_run_bootloader(void);
int tcm_run_application(void);

/* helper to perform device identify */
int tcm_get_identify_info(char *p_buf);
int tcm_get_app_info(char *p_buf);

/* helper to set/get the tcm configuration */
int tcm_get_touch_config();
int tcm_get_static_config();

/* helper to perform report reading */
int tcm_enable_report(bool enable, enum tcm_report_code report_code);
int tcm_read_report_frame(int type, int *p_out, int size_out, bool out_in_landscape);

/* helper to run production test */
int tcm_do_test_drt_pid07(int *p_result_img, int result_img_col, int result_img_row,
                          int *limit_min, int size_limit_min, int *limit_max, int size_limit_max);
int tcm_do_test_noise_pid0a(int *p_result_img, int result_img_col, int result_img_row,
                            int *limit_max, int size_limit_max);
int tcm_do_test_pt11_pid0b(int *p_result_img, int result_img_col, int result_img_row,
                           int *limit_min, int size_limit_min, int *limit_max, int size_limit_max);
int tcm_do_test_pt12_pid0c(int *p_result_img, int result_img_col, int result_img_row,
                           int *limit_min, int size_limit_min);
int tcm_do_test_pt13_pid0d(int *p_result_img, int result_img_col, int result_img_row,
                           int *limit_min, int size_limit_min);
int tcm_do_test_full_raw_pid05(int *p_result_img, int result_img_col, int result_img_row,
                               int *limit_min, int size_limit_min, int *limit_max, int size_limit_max);
int tcm_do_test_trx_trx_short_pid01(int *p_result_data, int *pins_result, int size_of_pins_result,
                                    int *limit, int size_of_limit);
int tcm_do_test_trx_ground_pid03(int *p_result_data, int *pins_result, int size_of_pins_result,
                                 int *limit, int size_of_limit);
int tcm_do_test_adc_range_pid11(int *p_result_img, int result_img_col, int result_img_row,
                                int *limit_min, int size_limit_min, int *limit_max, int size_limit_max);
int tcm_do_test_abs_rawcap_pid12(int *p_result_img, int result_img_col, int result_img_row,
                                 int *limit_min, int size_limit_min, int *limit_max, int size_limit_max);
int tcm_do_test_hy_abs_nose_pid1d(int *p_result_img, int result_img_col, int result_img_row,
                                  int *limit_min, int size_limit_min, int *limit_max, int size_limit_max);
int tcm_do_test_ex_high_resistance_pid05(short *p_result_tixel, short *p_result_rx, short *p_result_tx,
                                         short *p_ref_frame, int col, int row,
                                         int limit_tixel, int limit_rxroe, int limit_txroe);

/* helper to poll the touch response */
int tcm_query_touch_response(int *touch_x, int* touch_y, int* touch_status,
                             int max_fingers_to_process);
int tcm_get_touch_report(int report_size);

/* helper to perform raw tcm packet sending */
int tcm_raw_control(unsigned char* p_in, int size_in, unsigned char* p_out, int size_out);


/* helper to get the pins mapping */
int tcm_get_pins_mapping(int rxes_offset, int rxes_len, int txes_offset, int txes_len,
                         int num_rxguard_offset, int num_rxguard_len,
                         int rxguard_pins_offset, int rxguard_pins_len,
                         int num_txguard_offset, int num_txguard_len,
                         int txguard_pins_offset, int txguard_pins_len);


#endif // _TCM_CONTROL_H__

