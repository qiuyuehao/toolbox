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
#ifndef _RMI_ACCESS_H__
#define _RMI_ACCESS_H__

#define TRX_MAPPING_MAX 64
#define MUX_MAX 32

#define NOISE_TEST_FRAMES (20)

#define RMI_GET_REPORT_TIMEOUT 500
#define RMI_COMMAND_TIMEOUT 30

#define RMI_REPROT_DATA_OFFSET 3

#define MAX_SENSOR_MAP_SIZE 128
#define SENSOR_RX_MAPPING_OFFSET 1
#define SENSOR_TX_MAPPING_OFFSET 2

#define RMI_COMMAND_GET_REPORT 1
#define RMI_COMMAND_FORCE_CAL 2
#define RMI_COMMAND_FORCE_UPDATE 4

#define MAX_RMI_FREUENCY_GEAR 20

#define TRX_OPEN_SHORT_DATA_SIZE (7)

#define RMI_NORMAL_OPERATION (0 << 0)
#define RMI_SENSOR_SLEEP (1 << 0)
#define RMI_NO_SLEEP_OFF (0 << 2)
#define RMI_NO_SLEEP_ON (1 << 2)
#define RMI_CONFIGURED (1 << 7)

#define F54_CONTROL_0_SIZE 1
#define F54_CONTROL_1_SIZE 1
#define F54_CONTROL_2_SIZE 2
#define F54_CONTROL_3_SIZE 1
#define F54_CONTROL_4_6_SIZE 3
#define F54_CONTROL_7_SIZE 1
#define F54_CONTROL_8_9_SIZE 3
#define F54_CONTROL_10_SIZE 1
#define F54_CONTROL_11_SIZE 2
#define F54_CONTROL_12_13_SIZE 2
#define F54_CONTROL_14_SIZE 1
#define F54_CONTROL_15_SIZE 1
#define F54_CONTROL_16_SIZE 1
#define F54_CONTROL_17_SIZE 1
#define F54_CONTROL_18_SIZE 1
#define F54_CONTROL_19_SIZE 1
#define F54_CONTROL_20_SIZE 1
#define F54_CONTROL_21_SIZE 2
#define F54_CONTROL_22_26_SIZE 7
#define F54_CONTROL_27_SIZE 1
#define F54_CONTROL_28_SIZE 2
#define F54_CONTROL_29_SIZE 1
#define F54_CONTROL_30_SIZE 1
#define F54_CONTROL_31_SIZE 1
#define F54_CONTROL_32_35_SIZE 8
#define F54_CONTROL_36_SIZE 1
#define F54_CONTROL_37_SIZE 1
#define F54_CONTROL_38_SIZE 1
#define F54_CONTROL_39_SIZE 1
#define F54_CONTROL_40_SIZE 1
#define F54_CONTROL_41_SIZE 1
#define F54_CONTROL_42_SIZE 2
#define F54_CONTROL_43_54_SIZE 13
#define F54_CONTROL_55_56_SIZE 2
#define F54_CONTROL_57_SIZE 1
#define F54_CONTROL_58_SIZE 1
#define F54_CONTROL_59_SIZE 2
#define F54_CONTROL_60_62_SIZE 3
#define F54_CONTROL_63_SIZE 1
#define F54_CONTROL_64_67_SIZE 4
#define F54_CONTROL_68_73_SIZE 8
#define F54_CONTROL_74_SIZE 2
#define F54_CONTROL_75_SIZE 1
#define F54_CONTROL_76_SIZE 1
#define F54_CONTROL_77_78_SIZE 2
#define F54_CONTROL_79_83_SIZE 5
#define F54_CONTROL_84_85_SIZE 2
#define F54_CONTROL_86_SIZE 1
#define F54_CONTROL_87_SIZE 1
#define F54_CONTROL_88_SIZE 1
#define F54_CONTROL_89_SIZE 1
#define F54_CONTROL_90_SIZE 1
#define F54_CONTROL_91_SIZE 1
#define F54_CONTROL_92_SIZE 1
#define F54_CONTROL_93_SIZE 1
#define F54_CONTROL_94_SIZE 1
#define F54_CONTROL_95_SIZE 1
#define F54_CONTROL_96_SIZE 1
#define F54_CONTROL_97_SIZE 1
#define F54_CONTROL_98_SIZE 1
#define F54_CONTROL_99_SIZE 1
#define F54_CONTROL_100_SIZE 1
#define F54_CONTROL_101_SIZE 1
#define F54_CONTROL_102_SIZE 1
#define F54_CONTROL_103_SIZE 1
#define F54_CONTROL_104_SIZE 1
#define F54_CONTROL_105_SIZE 1
#define F54_CONTROL_106_SIZE 1
#define F54_CONTROL_107_SIZE 1
#define F54_CONTROL_108_SIZE 1
#define F54_CONTROL_109_SIZE 1
#define F54_CONTROL_110_SIZE 1
#define F54_CONTROL_111_SIZE 1
#define F54_CONTROL_112_SIZE 1
#define F54_CONTROL_113_SIZE 1
#define F54_CONTROL_114_SIZE 1
#define F54_CONTROL_115_SIZE 1
#define F54_CONTROL_116_SIZE 1
#define F54_CONTROL_117_SIZE 1
#define F54_CONTROL_118_SIZE 1
#define F54_CONTROL_119_SIZE 1
#define F54_CONTROL_120_SIZE 1
#define F54_CONTROL_121_SIZE 1
#define F54_CONTROL_122_SIZE 1
#define F54_CONTROL_123_SIZE 1
#define F54_CONTROL_124_SIZE 1
#define F54_CONTROL_125_SIZE 1
#define F54_CONTROL_126_SIZE 1
#define F54_CONTROL_127_SIZE 1
#define F54_CONTROL_128_SIZE 1
#define F54_CONTROL_129_SIZE 1
#define F54_CONTROL_130_SIZE 1
#define F54_CONTROL_131_SIZE 1
#define F54_CONTROL_132_SIZE 1
#define F54_CONTROL_133_SIZE 1
#define F54_CONTROL_134_SIZE 1
#define F54_CONTROL_135_SIZE 1
#define F54_CONTROL_136_SIZE 1
#define F54_CONTROL_137_SIZE 1
#define F54_CONTROL_138_SIZE 1
#define F54_CONTROL_139_SIZE 1
#define F54_CONTROL_140_SIZE 1
#define F54_CONTROL_141_SIZE 1
#define F54_CONTROL_142_SIZE 1
#define F54_CONTROL_143_SIZE 1
#define F54_CONTROL_144_SIZE 1
#define F54_CONTROL_145_SIZE 1
#define F54_CONTROL_146_SIZE 1
#define F54_CONTROL_147_SIZE 1
#define F54_CONTROL_148_SIZE 1
#define F54_CONTROL_149_SIZE 1
#define F54_CONTROL_150_SIZE 1
#define F54_CONTROL_151_SIZE 1
#define F54_CONTROL_152_SIZE 1
#define F54_CONTROL_153_SIZE 1
#define F54_CONTROL_154_SIZE 1
#define F54_CONTROL_155_SIZE 1
#define F54_CONTROL_156_SIZE 1
#define F54_CONTROL_157_158_SIZE 2
#define F54_CONTROL_163_SIZE 1
#define F54_CONTROL_165_SIZE 1
#define F54_CONTROL_166_SIZE 1
#define F54_CONTROL_167_SIZE 1
#define F54_CONTROL_168_SIZE 1
#define F54_CONTROL_169_SIZE 1
#define F54_CONTROL_171_SIZE 1
#define F54_CONTROL_172_SIZE 1
#define F54_CONTROL_173_SIZE 1
#define F54_CONTROL_174_SIZE 1
#define F54_CONTROL_175_SIZE 1
#define F54_CONTROL_176_SIZE 1
#define F54_CONTROL_177_178_SIZE 2
#define F54_CONTROL_179_SIZE 1
#define F54_CONTROL_182_SIZE 1
#define F54_CONTROL_183_SIZE 1
#define F54_CONTROL_185_SIZE 1
#define F54_CONTROL_186_SIZE 1
#define F54_CONTROL_187_SIZE 1
#define F54_CONTROL_188_SIZE 1
#define F54_CONTROL_196_SIZE 1
#define F54_CONTROL_218_SIZE 1
#define F54_CONTROL_223_SIZE 1

struct F54FreqCtrl {
    unsigned char Control;
    unsigned char FirstBurst_lsb;
    unsigned char FirstBurst_msb;
    unsigned char AddBurst_lsb;
    unsigned char AddBurst_msb;
    unsigned char IStretch;
    unsigned char RStretch;
    unsigned char NoiseCtrl_1;
    unsigned char NoiseCtrl_2;
    unsigned char NoiseCtrl_3;
    unsigned char NoiseCtrl_4;
};

struct FunctionDescriptor {
    /* note that the address is 16 bit */
    unsigned short query_base_addr;
    unsigned short command_base_addr;
    unsigned short control_base_addr;
    unsigned short data_base_addr;
    unsigned char Version;
    unsigned char InterruptSourceCount;
    unsigned short ID;
};

struct f54_query {
    union {
        struct {
            /* query 0 */
            unsigned char num_of_rx_electrodes;

            /* query 1 */
            unsigned char num_of_tx_electrodes;

            /* query 2 */
            unsigned char f54_query2_b0__1:2;
            unsigned char has_baseline:1;
            unsigned char has_image8:1;
            unsigned char f54_query2_b4__5:2;
            unsigned char has_image16:1;
            unsigned char f54_query2_b7:1;

            /* queries 3.0 and 3.1 */
            unsigned short clock_rate;

            /* query 4 */
            unsigned char touch_controller_family;

            /* query 5 */
            unsigned char has_pixel_touch_threshold_adjustment:1;
            unsigned char f54_query5_b1__7:7;

            /* query 6 */
            unsigned char has_sensor_assignment:1;
            unsigned char has_interference_metric:1;
            unsigned char has_sense_frequency_control:1;
            unsigned char has_firmware_noise_mitigation:1;
            unsigned char has_ctrl11:1;
            unsigned char has_two_byte_report_rate:1;
            unsigned char has_one_byte_report_rate:1;
            unsigned char has_relaxation_control:1;

            /* query 7 */
            unsigned char curve_compensation_mode:2;
            unsigned char f54_query7_b2__7:6;

            /* query 8 */
            unsigned char f54_query8_b0:1;
            unsigned char has_iir_filter:1;
            unsigned char has_cmn_removal:1;
            unsigned char has_cmn_maximum:1;
            unsigned char has_touch_hysteresis:1;
            unsigned char has_edge_compensation:1;
            unsigned char has_per_frequency_noise_control:1;
            unsigned char has_enhanced_stretch:1;

            /* query 9 */
            unsigned char has_force_fast_relaxation:1;
            unsigned char has_multi_metric_state_machine:1;
            unsigned char has_signal_clarity:1;
            unsigned char has_variance_metric:1;
            unsigned char has_0d_relaxation_control:1;
            unsigned char has_0d_acquisition_control:1;
            unsigned char has_status:1;
            unsigned char has_slew_metric:1;

            /* query 10 */
            unsigned char has_h_blank:1;
            unsigned char has_v_blank:1;
            unsigned char has_long_h_blank:1;
            unsigned char has_startup_fast_relaxation:1;
            unsigned char has_esd_control:1;
            unsigned char has_noise_mitigation2:1;
            unsigned char has_noise_state:1;
            unsigned char has_energy_ratio_relaxation:1;

            /* query 11 */
            unsigned char has_excessive_noise_reporting:1;
            unsigned char has_slew_option:1;
            unsigned char has_two_overhead_bursts:1;
            unsigned char has_query13:1;
            unsigned char has_one_overhead_burst:1;
            unsigned char f54_query11_b5:1;
            unsigned char has_ctrl88:1;
            unsigned char has_query15:1;

            /* query 12 */
            unsigned char number_of_sensing_frequencies:4;
            unsigned char f54_query12_b4__7:4;
        } __attribute__((packed));;
        unsigned char data[14];
    };
};

struct f54_query_13 {
    union {
        struct {
            unsigned char has_ctrl86:1;
            unsigned char has_ctrl87:1;
            unsigned char has_ctrl87_sub0:1;
            unsigned char has_ctrl87_sub1:1;
            unsigned char has_ctrl87_sub2:1;
            unsigned char has_cidim:1;
            unsigned char has_noise_mitigation_enhancement:1;
            unsigned char has_rail_im:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_15 {
    union {
        struct {
            unsigned char has_ctrl90:1;
            unsigned char has_transmit_strength:1;
            unsigned char has_ctrl87_sub3:1;
            unsigned char has_query16:1;
            unsigned char has_query20:1;
            unsigned char has_query21:1;
            unsigned char has_query22:1;
            unsigned char has_query25:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_16 {
    union {
        struct {
            unsigned char has_query17:1;
            unsigned char has_data17:1;
            unsigned char has_ctrl92:1;
            unsigned char has_ctrl93:1;
            unsigned char has_ctrl94_query18:1;
            unsigned char has_ctrl95_query19:1;
            unsigned char has_ctrl99:1;
            unsigned char has_ctrl100:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_21 {
    union {
        struct {
            unsigned char has_abs_rx:1;
            unsigned char has_abs_tx:1;
            unsigned char has_ctrl91:1;
            unsigned char has_ctrl96:1;
            unsigned char has_ctrl97:1;
            unsigned char has_ctrl98:1;
            unsigned char has_data19:1;
            unsigned char has_query24_data18:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_22 {
    union {
        struct {
            unsigned char has_packed_image:1;
            unsigned char has_ctrl101:1;
            unsigned char has_dynamic_sense_display_ratio:1;
            unsigned char has_query23:1;
            unsigned char has_ctrl103_query26:1;
            unsigned char has_ctrl104:1;
            unsigned char has_ctrl105:1;
            unsigned char has_query28:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_23 {
    union {
        struct {
            unsigned char has_ctrl102:1;
            unsigned char has_ctrl102_sub1:1;
            unsigned char has_ctrl102_sub2:1;
            unsigned char has_ctrl102_sub4:1;
            unsigned char has_ctrl102_sub5:1;
            unsigned char has_ctrl102_sub9:1;
            unsigned char has_ctrl102_sub10:1;
            unsigned char has_ctrl102_sub11:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_25 {
    union {
        struct {
            unsigned char has_ctrl106:1;
            unsigned char has_ctrl102_sub12:1;
            unsigned char has_ctrl107:1;
            unsigned char has_ctrl108:1;
            unsigned char has_ctrl109:1;
            unsigned char has_data20:1;
            unsigned char f54_query25_b6:1;
            unsigned char has_query27:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_27 {
    union {
        struct {
            unsigned char has_ctrl110:1;
            unsigned char has_data21:1;
            unsigned char has_ctrl111:1;
            unsigned char has_ctrl112:1;
            unsigned char has_ctrl113:1;
            unsigned char has_data22:1;
            unsigned char has_ctrl114:1;
            unsigned char has_query29:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_29 {
    union {
        struct {
            unsigned char has_ctrl115:1;
            unsigned char has_ground_ring_options:1;
            unsigned char has_lost_bursts_tuning:1;
            unsigned char has_aux_exvcom2_select:1;
            unsigned char has_ctrl116:1;
            unsigned char has_data23:1;
            unsigned char has_ctrl117:1;
            unsigned char has_query30:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_30 {
    union {
        struct {
            unsigned char has_ctrl118:1;
            unsigned char has_ctrl119:1;
            unsigned char has_ctrl120:1;
            unsigned char has_ctrl121:1;
            unsigned char has_ctrl122_query31:1;
            unsigned char has_ctrl123:1;
            unsigned char has_ctrl124:1;
            unsigned char has_query32:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_32 {
    union {
        struct {
            unsigned char has_ctrl125:1;
            unsigned char has_ctrl126:1;
            unsigned char has_ctrl127:1;
            unsigned char has_abs_charge_pump_disable:1;
            unsigned char has_query33:1;
            unsigned char has_data24:1;
            unsigned char has_query34:1;
            unsigned char has_query35:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_33 {
    union {
        struct {
            unsigned char has_ctrl128:1;
            unsigned char has_ctrl129:1;
            unsigned char has_ctrl130:1;
            unsigned char has_ctrl131:1;
            unsigned char has_ctrl132:1;
            unsigned char has_ctrl133:1;
            unsigned char has_ctrl134:1;
            unsigned char has_query36:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_35 {
    union {
        struct {
            unsigned char has_data25:1;
            unsigned char has_ctrl135:1;
            unsigned char has_ctrl136:1;
            unsigned char has_ctrl137:1;
            unsigned char has_ctrl138:1;
            unsigned char has_ctrl139:1;
            unsigned char has_data26:1;
            unsigned char has_ctrl140:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_36 {
    union {
        struct {
            unsigned char has_ctrl141:1;
            unsigned char has_ctrl142:1;
            unsigned char has_query37:1;
            unsigned char has_ctrl143:1;
            unsigned char has_ctrl144:1;
            unsigned char has_ctrl145:1;
            unsigned char has_ctrl146:1;
            unsigned char has_query38:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_38 {
    union {
        struct {
            unsigned char has_ctrl147:1;
            unsigned char has_ctrl148:1;
            unsigned char has_ctrl149:1;
            unsigned char has_ctrl150:1;
            unsigned char has_ctrl151:1;
            unsigned char has_ctrl152:1;
            unsigned char has_ctrl153:1;
            unsigned char has_query39:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_39 {
    union {
        struct {
            unsigned char has_ctrl154:1;
            unsigned char has_ctrl155:1;
            unsigned char has_ctrl156:1;
            unsigned char has_ctrl160:1;
            unsigned char has_ctrl157_ctrl158:1;
            unsigned char f54_query39_b5__6:2;
            unsigned char has_query40:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_40 {
    union {
        struct {
            unsigned char has_ctrl169:1;
            unsigned char has_ctrl163_query41:1;
            unsigned char f54_query40_b2:1;
            unsigned char has_ctrl165_query42:1;
            unsigned char has_ctrl166:1;
            unsigned char has_ctrl167:1;
            unsigned char has_ctrl168:1;
            unsigned char has_query43:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_43 {
    union {
        struct {
            unsigned char f54_query43_b0__1:2;
            unsigned char has_ctrl171:1;
            unsigned char has_ctrl172_query44_query45:1;
            unsigned char has_ctrl173:1;
            unsigned char has_ctrl174:1;
            unsigned char has_ctrl175:1;
            unsigned char has_query46:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_46 {
    union {
        struct {
            unsigned char has_ctrl176:1;
            unsigned char has_ctrl177_ctrl178:1;
            unsigned char has_ctrl179:1;
            unsigned char f54_query46_b3:1;
            unsigned char has_data27:1;
            unsigned char has_data28:1;
            unsigned char f54_query46_b6:1;
            unsigned char has_query47:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_47 {
    union {
        struct {
            unsigned char f54_query47_b0:1;
            unsigned char has_ctrl182:1;
            unsigned char has_ctrl183:1;
            unsigned char f54_query47_b3:1;
            unsigned char has_ctrl185:1;
            unsigned char has_ctrl186:1;
            unsigned char has_ctrl187:1;
            unsigned char has_query49:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_49 {
    union {
        struct {
            unsigned char f54_query49_b0__1:2;
            unsigned char has_ctrl188:1;
            unsigned char has_data31:1;
            unsigned char f54_query49_b4__6:3;
            unsigned char has_query50:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_50 {
    union {
        struct {
            unsigned char f54_query50_b0__6:7;
            unsigned char has_query51:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_51 {
    union {
        struct {
            unsigned char f54_query51_b0:1;
            unsigned char has_ctrl196:1;
            unsigned char f54_query51_b2:1;
            unsigned char f54_query51_b3:1;
            unsigned char f54_query51_b4:1;
            unsigned char has_query53_query54_ctrl198:1;
            unsigned char has_ctrl199:1;
            unsigned char has_query55:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_55 {
    union {
        struct {
            unsigned char has_query56:1;
            unsigned char has_data33_data34:1;
            unsigned char has_alternate_report_rate:1;
            unsigned char has_ctrl200:1;
            unsigned char has_ctrl201_ctrl202:1;
            unsigned char has_ctrl203:1;
            unsigned char has_ctrl204:1;
            unsigned char has_query57:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_57 {
    union {
        struct {
            unsigned char has_ctrl205:1;
            unsigned char has_ctrl206:1;
            unsigned char has_usb_bulk_read:1;
            unsigned char has_ctrl207:1;
            unsigned char has_ctrl208:1;
            unsigned char has_ctrl209:1;
            unsigned char has_ctrl210:1;
            unsigned char has_query58:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_58 {
    union {
        struct {
            unsigned char has_query59:1;
            unsigned char has_query60:1;
            unsigned char has_ctrl211:1;
            unsigned char has_ctrl212:1;
            unsigned char has_hybrid_abs_tx_axis_filtering:1;
            unsigned char f54_query58_b5:1;
            unsigned char has_ctrl213:1;
            unsigned char has_query61:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_61 {
    union {
        struct {
            unsigned char has_ctrl214:1;
            unsigned char has_ctrl215_query62_query63:1;
            unsigned char f54_query61_b2__4:3;
            unsigned char has_ctrl218:1;
            unsigned char has_hybrid_abs_buttons:1;
            unsigned char has_query64:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_64 {
    union {
        struct {
            unsigned char f54_query64_b0:1;
            unsigned char has_ctrl220:1;
            unsigned char f54_query64_b2__3:2;
            unsigned char has_ctrl219_sub1:1;
            unsigned char has_ctrl103_sub3:1;
            unsigned char has_ctrl224_ctrl226_ctrl227:1;
            unsigned char has_query65:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_65 {
    union {
        struct {
            unsigned char f54_query65_b0__4:5;
            unsigned char has_query66_ctrl231:1;
            unsigned char has_ctrl232:1;
            unsigned char has_query67:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_67 {
    union {
        struct {
            unsigned char has_abs_doze_spatial_filter_enable:1;
            unsigned char has_abs_doze_average_filter_enable:1;
            unsigned char has_single_display_pulse:1;
            unsigned char f54_query67_b3__6:4;
            unsigned char has_query68:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_68 {
    union {
        struct {
            unsigned char f54_query68_b0__4:5;
            unsigned char has_freq_filter_bw_ext:1;
            unsigned char f54_query68_b6:1;
            unsigned char has_query69:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_query_69 {
    union {
        struct {
            unsigned char has_ctrl240_sub0:1;
            unsigned char has_ctrl240_sub1_sub2:1;
            unsigned char has_ctrl240_sub3:1;
            unsigned char has_ctrl240_sub4:1;
            unsigned char burst_mode_report_type_enabled:1;
            unsigned char f54_query69_b5__7:3;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f54_data_31 {
    union {
        struct {
            unsigned char is_calibration_crc:1;
            unsigned char calibration_crc:1;
            unsigned char short_test_row_number:5;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_2 {
    union {
        struct {
            unsigned char saturation_cap;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_7 {
    union {
        struct {
            unsigned char cbc_cap:3;
            unsigned char cbc_polarity:1;
            unsigned char cbc_tx_carrier_selection:1;
            unsigned char f54_ctrl7_b5__7:3;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_41 {
    union {
        struct {
            unsigned char no_signal_clarity:1;
            unsigned char f54_ctrl41_b1__7:7;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_57 {
    union {
        struct {
            unsigned char cbc_cap:3;
            unsigned char cbc_polarity:1;
            unsigned char cbc_tx_carrier_selection:1;
            unsigned char f54_ctrl57_b5__7:3;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_86 {
    union {
        struct {
            unsigned char enable_high_noise_state:1;
            unsigned char dynamic_sense_display_ratio:2;
            unsigned char f54_ctrl86_b3__7:5;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_88 {
    union {
        struct {
            unsigned char tx_low_reference_polarity:1;
            unsigned char tx_high_reference_polarity:1;
            unsigned char abs_low_reference_polarity:1;
            unsigned char abs_polarity:1;
            unsigned char cbc_polarity:1;
            unsigned char cbc_tx_carrier_selection:1;
            unsigned char charge_pump_enable:1;
            unsigned char cbc_abs_auto_servo:1;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_91 {
    union {
        struct {
            unsigned char reflo_transcap_capacitance;
            unsigned char refhi_transcap_capacitance;
            unsigned char receiver_feedback_capacitance;
            unsigned char reference_receiver_feedback_capacitance;
            unsigned char gain_ctrl;
        } __attribute__((packed));;
        struct {
            unsigned char data[5];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_96 {
    union {
        struct {
            unsigned char cbc_transcap[64];
        } __attribute__((packed));;
        struct {
            unsigned char data[64];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_99 {
    union {
        struct {
            unsigned char integration_duration_lsb;
            unsigned char integration_duration_msb;
            unsigned char reset_duration;
        } __attribute__((packed));;
        struct {
            unsigned char data[3];
            unsigned short address;
        } __attribute__((packed));;
    };
};


struct f54_control_110 {
    union {
        struct {
            unsigned char active_stylus_rx_feedback_cap;
            unsigned char active_stylus_rx_feedback_cap_reference;
            unsigned char active_stylus_low_reference;
            unsigned char active_stylus_high_reference;
            unsigned char active_stylus_gain_control;
            unsigned char active_stylus_gain_control_reference;
            unsigned char active_stylus_timing_mode;
            unsigned char active_stylus_discovery_bursts;
            unsigned char active_stylus_detection_bursts;
            unsigned char active_stylus_discovery_noise_multiplier;
            unsigned char active_stylus_detection_envelope_min;
            unsigned char active_stylus_detection_envelope_max;
            unsigned char active_stylus_lose_count;
        } __attribute__((packed));;
        struct {
            unsigned char data[13];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_149 {
    union {
        struct {
            unsigned char trans_cbc_global_cap_enable:1;
            unsigned char f54_ctrl149_b1__7:7;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_182 {
    union {
        struct {
            unsigned char cbc_timing_ctrl_tx_lsb;
            unsigned char cbc_timing_ctrl_tx_msb;
            unsigned char cbc_timing_ctrl_rx_lsb;
            unsigned char cbc_timing_ctrl_rx_msb;
        } __attribute__((packed));;
        struct {
            unsigned char data[4];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_188 {
    union {
        struct {
            unsigned char start_calibration:1;
            unsigned char start_is_calibration:1;
            unsigned char frequency:2;
            unsigned char start_production_test:1;
            unsigned char short_test_calibration:1;
            unsigned char f54_ctrl188_b7:1;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control_223 {
    union {
        struct {
            unsigned char voltages_for_0d:8;
        } __attribute__((packed));;
        struct {
            unsigned char data[1];
            unsigned short address;
        } __attribute__((packed));;
    };
};

struct f54_control {
    struct f54_control_2 reg_2;
    struct f54_control_7 reg_7;
    struct f54_control_41 reg_41;
    struct f54_control_57 reg_57;
    struct f54_control_86 reg_86;
    struct f54_control_88 reg_88;
    struct f54_control_91 reg_91;
    struct f54_control_96 reg_96;
    struct f54_control_99 reg_99;
    struct f54_control_110 reg_110;
    struct f54_control_149 reg_149;
    struct f54_control_182 reg_182;
    struct f54_control_188 reg_188;
    struct f54_control_223 reg_223;
};

struct f55_query {
    union {
        struct {
            /* query 0 */
            unsigned char num_of_rx_electrodes;

            /* query 1 */
            unsigned char num_of_tx_electrodes;

            /* query 2 */
            unsigned char has_sensor_assignment:1;
            unsigned char has_edge_compensation:1;
            unsigned char curve_compensation_mode:2;
            unsigned char has_ctrl6:1;
            unsigned char has_alternate_transmitter_assignment:1;
            unsigned char has_single_layer_multi_touch:1;
            unsigned char has_query5:1;
        } __attribute__((packed));;
        unsigned char data[3];
    };
};

struct f55_query_3 {
    union {
        struct {
            unsigned char has_ctrl8:1;
            unsigned char has_ctrl9:1;
            unsigned char has_oncell_pattern_support:1;
            unsigned char has_data0:1;
            unsigned char has_single_wide_pattern_support:1;
            unsigned char has_mirrored_tx_pattern_support:1;
            unsigned char has_discrete_pattern_support:1;
            unsigned char has_query9:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_query_5 {
    union {
        struct {
            unsigned char has_corner_compensation:1;
            unsigned char has_ctrl12:1;
            unsigned char has_trx_configuration:1;
            unsigned char has_ctrl13:1;
            unsigned char f55_query5_b4:1;
            unsigned char has_ctrl14:1;
            unsigned char has_basis_function:1;
            unsigned char has_query17:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_query_17 {
    union {
        struct {
            unsigned char f55_query17_b0:1;
            unsigned char has_ctrl16:1;
            unsigned char has_ctrl18_ctrl19:1;
            unsigned char has_ctrl17:1;
            unsigned char has_ctrl20:1;
            unsigned char has_ctrl21:1;
            unsigned char has_ctrl22:1;
            unsigned char has_query18:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_query_18 {
    union {
        struct {
            unsigned char has_ctrl23:1;
            unsigned char has_ctrl24:1;
            unsigned char has_query19:1;
            unsigned char has_ctrl25:1;
            unsigned char has_ctrl26:1;
            unsigned char has_ctrl27_query20:1;
            unsigned char has_ctrl28_query21:1;
            unsigned char has_query22:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_query_22 {
    union {
        struct {
            unsigned char has_ctrl29:1;
            unsigned char has_query23:1;
            unsigned char has_guard_disable:1;
            unsigned char has_ctrl30:1;
            unsigned char has_ctrl31:1;
            unsigned char has_ctrl32:1;
            unsigned char has_query24_through_query27:1;
            unsigned char has_query28:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_query_23 {
    union {
        struct {
            unsigned char amp_sensor_enabled:1;
            unsigned char image_transposed:1;
            unsigned char first_column_at_left_side:1;
            unsigned char size_of_column2mux:5;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_query_28 {
    union {
        struct {
            unsigned char f55_query28_b0__4:5;
            unsigned char has_ctrl37:1;
            unsigned char has_query29:1;
            unsigned char has_query30:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_query_30 {
    union {
        struct {
            unsigned char has_ctrl38:1;
            unsigned char has_query31_query32:1;
            unsigned char has_ctrl39:1;
            unsigned char has_ctrl40:1;
            unsigned char has_ctrl41:1;
            unsigned char has_ctrl42:1;
            unsigned char has_ctrl43_ctrl44:1;
            unsigned char has_query33:1;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_query_33 {
    union {
        struct {
            unsigned char has_extended_amp_pad:1;
            unsigned char has_extended_amp_btn:1;
            unsigned char has_ctrl45_ctrl46:1;
            unsigned char f55_query33_b3:1;
            unsigned char has_ctrl47_sub0_sub1:1;
            unsigned char f55_query33_b5__7:3;
        } __attribute__((packed));;
        unsigned char data[1];
    };
};

struct f55_control_43 {
    union {
        struct {
            unsigned char swap_sensor_side:1;
            unsigned char f55_ctrl43_b1__7:7;
            unsigned char afe_l_mux_size:4;
            unsigned char afe_r_mux_size:4;
        } __attribute__((packed));;
        unsigned char data[2];
    };
};

struct PDT {
    struct FunctionDescriptor F01;
    struct FunctionDescriptor F11;
    struct FunctionDescriptor F12;
    struct FunctionDescriptor F1A;
    struct FunctionDescriptor F34;
    struct FunctionDescriptor F54;
    struct FunctionDescriptor F55;

    /* function 54 */
    struct f54_query f54_query;
    struct f54_query_13 f54_query_13;
    struct f54_query_15 f54_query_15;
    struct f54_query_16 f54_query_16;
    struct f54_query_21 f54_query_21;
    struct f54_query_22 f54_query_22;
    struct f54_query_23 f54_query_23;
    struct f54_query_25 f54_query_25;
    struct f54_query_27 f54_query_27;
    struct f54_query_29 f54_query_29;
    struct f54_query_30 f54_query_30;
    struct f54_query_32 f54_query_32;
    struct f54_query_33 f54_query_33;
    struct f54_query_35 f54_query_35;
    struct f54_query_36 f54_query_36;
    struct f54_query_38 f54_query_38;
    struct f54_query_39 f54_query_39;
    struct f54_query_40 f54_query_40;
    struct f54_query_43 f54_query_43;
    struct f54_query_46 f54_query_46;
    struct f54_query_47 f54_query_47;
    struct f54_query_49 f54_query_49;
    struct f54_query_50 f54_query_50;
    struct f54_query_51 f54_query_51;
    struct f54_query_55 f54_query_55;
    struct f54_query_57 f54_query_57;
    struct f54_query_58 f54_query_58;
    struct f54_query_61 f54_query_61;
    struct f54_query_64 f54_query_64;
    struct f54_query_65 f54_query_65;
    struct f54_query_67 f54_query_67;
    struct f54_query_68 f54_query_68;
    struct f54_query_69 f54_query_69;
    struct f54_data_31 f54_data_31;
    struct f54_control f54_control;
    unsigned short f54_control_reg95_offset;

    /* function 55 */
    struct f55_query f55_query;
    struct f55_query_3 f55_query_3;
    struct f55_query_5 f55_query_5;
    struct f55_query_17 f55_query_17;
    struct f55_query_18 f55_query_18;
    struct f55_query_22 f55_query_22;
    struct f55_query_23 f55_query_23;
    struct f55_query_28 f55_query_28;
    struct f55_query_30 f55_query_30;
    struct f55_query_33 f55_query_33;

    /* variables */
    unsigned char tx_assigned;
    unsigned char rx_assigned;
    unsigned char swap_sensor_side;
    unsigned char left_mux_size;
    unsigned char right_mux_size;
    unsigned char asic_type[16];
    unsigned int build_id;
    unsigned char number_of_sensing_frequencies;
    bool has_button;
    bool f54_skip_preparation;
    bool amp_sensor;
    bool extended_amp;
    bool extended_amp_btn;
    unsigned char size_of_column2mux;
    unsigned char afe_mux_offset;
    unsigned char tx_assignment[MAX_SENSOR_MAP_SIZE];
    unsigned char rx_assignment[MAX_SENSOR_MAP_SIZE];
    unsigned short saturation_cap;
    unsigned char finger_threshold;

    bool is_tddi_dev;
};

/* global variables for syna_dev_manager using */
struct PDT g_rmi_pdt;

/* helper to perform read/write operations */
int rmi_open_dev(const char* rmi_path);
void rmi_close_dev(const char* rmi_path);
int rmi_read_reg(unsigned short address, unsigned char *p_rd_data, int bytes_to_read);
int rmi_write_reg(unsigned short address, unsigned char *p_wr_data, int bytes_to_write);
int rmi_scan_pdt(void);

/* to issue a sw reset */
int rmi_f01_sw_reset();
/* to configure into no sleep mode */
int rmi_f01_set_no_sleep();
/* to perform the force update */
int rmi_f54_force_update();

/* helper to perform device identify and requested information */
int rmi_get_identify_info(char *info_buffer);
int rmi_get_asic_type(void);
int rmi_get_build_id(void);
void rmi_get_f54_gear_info(char *p_info);
int rmi_get_finger_cap(void);
int rmi_f54_enable_one_gear(int gear);

/* helper to read the report image */
int rmi_f54_read_report_image(int report_type, short *p_image, int image_col, int image_row);
int read_rmi_f54_report(int type, short *buf);

/* perform noise test with RT2 */
bool rmi_do_noise_test(FILE *pfile, char *timestamp, int frame_id, int gear_idx);

/* helper to write data log */
void rmi_write_log_header(FILE *pfile, int total_frames, int num_gears);


#endif // _RMI_ACCESS_H__