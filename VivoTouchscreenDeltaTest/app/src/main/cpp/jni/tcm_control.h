/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* Copyright (c) 2012-2016 Synaptics Incorporated. All rights reserved.
*
* The information in this file is confidential under the terms
* of a non-disclosure agreement with Synaptics and is provided
* AS IS without warranties or guarantees of any kind.
*
* The information in this file shall remain the exclusive property
* of Synaptics and may be the subject of Synaptics patents, in
* whole or part. Synaptics intellectual property rights in the
* information in this file are not expressly or implicitly licensed
* or otherwise transferred to you as a result of such information
* being made available to you.
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
#ifndef _TCM_ACCESS_H__
#define _TCM_ACCESS_H__

#define TCM_POLLING_DELAY_MS (20)
#define TCM_POLLING_TIMOUT (150)  /* 20 (ms) * 150 = 3000 ms = 3s */

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
    CMD_DOWNLOAD_CONFIG = 0x46,
    CMD_UNLOCK_PRIVATE = 0x80,
    CMD_READ_RAM = 0x81,
    CMD_WRITE_REGISTER = 0x82,
};

enum tcm_report_code {
    REPORT_IDENTIFY = 0x10,
    REPORT_TOUCH = 0x11,
    REPORT_DELTA = 0x12,
    REPORT_RAW = 0x13,
    REPORT_PRINTF = 0x82,
    REPORT_STATUS = 0xc0,
    REPORT_HDL = 0xfe,
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
    DC_GET_ENABLED_FREQ = 197,
    DC_GET_FINGER_THRESHOLD = 198,
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
};

/* global variables for syna_dev_manager using */
struct tcm_identify_report g_tcm_identify_report;
struct tcm_app_info g_tcm_app_info_report;

/* helper to perform read/write operations */
int tcm_open_dev(const char* tcm_path);
void tcm_close_dev(const char* tcm_path);
int tcm_enable_raw_mode(bool enable);
int tcm_read_message(unsigned char *p_rd_data, unsigned int bytes_to_read);
int tcm_gat_payload(unsigned char *p_rd_data, int payload_size);
int tcm_write_message(unsigned char *p_wr_data, unsigned int  bytes_to_write);
int tcm_wait_for_command_ready(void);
int tcm_set_no_sleep(int state);

/* helper to perform device identify */
int tcm_do_identify();
int tcm_get_identify_info(void);
int tcm_get_app_info(void);
int tcm_get_image_cols(void);
int tcm_get_image_rows(void);
int tcm_get_asic_id(void);
int tcm_get_build_id(void);
int tcm_get_normal_finger_threshold();
int tcm_get_enabled_gear_table();
void tcm_get_gear_info(char *p_info);
int tcm_get_finger_cap();

/* helper to perform report reading */
int tcm_enable_report(bool enable, enum tcm_report_code report_code);
int tcm_read_report_image(short *p_image, unsigned char type, int image_col, int image_row);

/* helper to perform frequency shifting */
int tcm_request_freq_gear(int gear_idx);

/* perform noise test with delta report */
bool tcm_do_noise_test(FILE *pfile, char *timestamp, int frame_id, int gear_idx);

/* helper to write data log */
void tcm_write_log_header(FILE *pfile, int total_frames, int num_gears);


#endif // _TCM_ACCESS_H__

