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
package com.vivotouchscreen.sensortestsyna3618f;

public class ProductionTestItems {

    /********************************************************
     * testing cases
     ********************************************************/
    public static final int TEST_RMI_NOISE_RT02          = 0x200;
    public static final int TEST_RMI_NOISE_TDDI_RT94     = 0x201;
    public static final int TEST_RMI_FULL_RAW_RT20       = 0x202;
    public static final int TEST_RMI_TDDI_FULL_RAW_RT92  = 0x203;
    public static final int TEST_RMI_EX_HIGH_RESISTANCE_RT20 = 0x204;
    public static final int TEST_RMI_TRX_SHORT_RT26      = 0x205;
    public static final int TEST_RMI_EX_TRX_SHORT_RT26100 = 0x206;
    public static final int TEST_RMI_ABS_OPEN_RT63       = 0x207;
    public static final int TEST_RMI_ADC_RANGE_RT23      = 0x208;
    public static final int TEST_RMI_TAGSMOISTURE_RT76   = 0x209;
    public static final int TEST_RMI_RT133               = 0x20A;
    public static final int TEST_RMI_ABS_DELTA_RT59      = 0x20B;
    public static final int TEST_RMI_SENSOR_SPEED_RT22   = 0x20C;

    public static final int TEST_TCM_NOISE_PID0A         = 0x300;
    public static final int TEST_TCM_DRT_PID07           = 0x301;
    public static final int TEST_TCM_PT11_PID0B          = 0x302;
    public static final int TEST_TCM_PT12_PID0C          = 0x303;
    public static final int TEST_TCM_PT13_PID0D          = 0x304;
    public static final int TEST_TCM_FULL_RAW_PID05      = 0x305;
    public static final int TEST_TCM_TRX_TRX_SHORT_PID01 = 0x306;
    public static final int TEST_TCM_TRX_GROUND_PID03    = 0x307;
    public static final int TEST_TCM_ADC_RANGE_PID11     = 0x308;
    public static final int TEST_TCM_ABS_RAWCAP_PID12    = 0x309;
    public static final int TEST_TCM_HYBRID_ABS_NOISE_PID1D = 0x30A;
    public static final int TEST_TCM_EX_HIGH_RESISTANCE_PID05 = 0x30B;


    /********************************************************
     * default test limit
     ********************************************************/
    // rmi noise test (RT2) and tcm noise test (PID0A)
    public static final int DEFAULT_TEST_LIMIT_NOISE_MAX = 100;

    // rmi full raw test (RT20) (RT92)
    public static final int DEFAULT_TEST_LIMIT_RMI_FULL_RAW_MAX = 3000;
    public static final int DEFAULT_TEST_LIMIT_RMI_FULL_RAW_MIN = 100;

    // rmi extended high resistance (RT20)
    public static final int DEFAULT_TEST_LIMIT_RMI_EX_HIGH_RESISTANCE = -40;
    public static final int DEFAULT_TEST_LIMIT_RMI_EX_HIGH_RESISTANCE_RXROE = 40;
    public static final int DEFAULT_TEST_LIMIT_RMI_EX_HIGH_RESISTANCE_TXROE = 40;

    // rmi extended trx short (RT26 + RT100)
    public static final int DEFAULT_TEST_LIMIT_RMI_EX_TRX_SHORT_MIN = 200;
    public static final int DEFAULT_TEST_LIMIT_RMI_EX_TRX_SHORT_MAX = 600;

    // rmi abs open test (RT63)
    public static final int DEFAULT_TEST_LIMIT_RMI_ABS_OPEN_RT63_MIN = 5000;
    public static final int DEFAULT_TEST_LIMIT_RMI_ABS_OPEN_RT63_MAX =20000;

    // rmi adc range test (RT23)
    public static final int DEFAULT_TEST_LIMIT_RMI_ADC_RANGE_RT23_MIN = 50;
    public static final int DEFAULT_TEST_LIMIT_RMI_ADC_RANGE_RT23_MAX = 200;

    // rmi sensor speed test (RT22)
    public static final int DEFAULT_TEST_LIMIT_RMI_SENSOR_SPEED_RT22_MIN = 0;
    public static final int DEFAULT_TEST_LIMIT_RMI_SENSOR_SPEED_RT22_MAX = 50;

    // rmi tagsmoisture  (RT76)
    public static final int DEFAULT_TEST_LIMIT_RMI_TAGSMOISTURE_RT76 = 50;

    // rmi rt133 test (RT133)
    public static final int DEFAULT_TEST_LIMIT_RMI_RT133 = 100;

    // rmi abs delta test (RT59)
    public static final int DEFAULT_TEST_LIMIT_RMI_ABS_DELTA_RT59_MIN =-100;
    public static final int DEFAULT_TEST_LIMIT_RMI_ABS_DELTA_RT59_MAX =100;


    // tcm dynamic range test (PID07)
    public static final int DEFAULT_TEST_LIMIT_TCM_DRT_MAX = 2000;
    public static final int DEFAULT_TEST_LIMIT_TCM_DRT_MIN = 300;

    // tcm full raw test (PID05)
    public static final int DEFAULT_TEST_LIMIT_TCM_FULL_RAW_PT05_MAX = 4000;
    public static final int DEFAULT_TEST_LIMIT_TCM_FULL_RAW_PT05_MIN = 300;

    // tcm trx trx short test (PID01)

    // tcm trx ground short (PID03)

    // tcm adc range test (PID11)
    public static final int DEFAULT_TEST_LIMIT_TCM_ADC_RANGE_PT11_MAX = 230;
    public static final int DEFAULT_TEST_LIMIT_TCM_ADC_RANGE_PT11_MIN = 20;

    // tcm abs raw cap test (PID12)
    public static final int DEFAULT_TEST_LIMIT_TCM_ABS_RAW_CAP_PT12_MAX = 100000;
    public static final int DEFAULT_TEST_LIMIT_TCM_ABS_RAW_CAP_PT12_MIN = 20000;

    // tcm hybrid abs noise test (PID1D)
    public static final int DEFAULT_TEST_LIMIT_TCM_HYBRID_ABS_NOISE_PT1D_MAX = 100;
    public static final int DEFAULT_TEST_LIMIT_TCM_HYBRID_ABS_NOISE_PT1D_MIN = -100;

    // tcm extended high resistance test (PID05)
    public static final int DEFAULT_TEST_LIMIT_TCM_EX_HIGH_RESISTANCE_TIXEL = -40;
    public static final int DEFAULT_TEST_LIMIT_TCM_EX_HIGH_RESISTANCE_RXROE = 450;
    public static final int DEFAULT_TEST_LIMIT_TCM_EX_HIGH_RESISTANCE_TXROE = 450;


    /********************************************************
     * helpers to return a string of test item based on its id
     ********************************************************/
    static String getTestNameStr(int id) {
        String str;

        switch (id) {
            case TEST_RMI_NOISE_RT02:
                str = "RMI Noise Test (RT02)";
                break;
            case TEST_RMI_NOISE_TDDI_RT94:
                str = "RMI TDDI Noise Test (RT94)";
                break;
            case TEST_RMI_FULL_RAW_RT20:
                str = "RMI Full Raw Capacitance Test (RT20)";
                break;
            case TEST_RMI_TDDI_FULL_RAW_RT92:
                str = "RMI TDDI Full Raw Test (RT92)";
                break;
            case TEST_RMI_EX_HIGH_RESISTANCE_RT20:
                str = "RMI Extended High Resistance Test (RT20)";
                break;
            case TEST_RMI_TRX_SHORT_RT26:
                str = "RMI TRX SHort Test (RT26)";
                break;
            case TEST_RMI_EX_TRX_SHORT_RT26100:
                str = "RMI Extended TRX SHort Test (RT26 + RT100)";
                break;
            case TEST_RMI_ABS_OPEN_RT63:
                str = "RMI Abs Sensing Open Test (RT63)";
                break;
            case TEST_RMI_ADC_RANGE_RT23:
                str = "RMI ADC Range Test (RT23)";
                break;
            case TEST_RMI_TAGSMOISTURE_RT76:
                str = "RMI Tags Moisture Test (RT76)";
                break;
            case TEST_RMI_RT133:
                str = "RMI RT133 Test (RT133)";
                break;
            case TEST_RMI_ABS_DELTA_RT59:
                str = "RMI Abs Sensing Delta Test (RT59)";
                break;
            case TEST_RMI_SENSOR_SPEED_RT22:
                str = "RMI Sensor Speed Test (RT22)";
                break;
            case TEST_TCM_NOISE_PID0A:
                str = "TCM Noise Test (PT0A)";
                break;
            case TEST_TCM_DRT_PID07:
                str = "TCM Dynamic Range Test (PT07)";
                break;
            case TEST_TCM_PT11_PID0B:
                str = "TCM Pt11 Test (PT0B)";
                break;
            case TEST_TCM_PT12_PID0C:
                str = "TCM Pt12 Test (PT0C)";
                break;
            case TEST_TCM_PT13_PID0D:
                str = "TCM Pt13 Test (PT0D)";
                break;
            case TEST_TCM_FULL_RAW_PID05:
                str = "TCM Full Raw Test (PT05)";
                break;
            case TEST_TCM_TRX_TRX_SHORT_PID01:
                str = "TCM Trx Trx Short Test (PT01)";
                break;
            case TEST_TCM_TRX_GROUND_PID03:
                str = "TCM Trx Ground Test (PT03)";
                break;
            case TEST_TCM_ADC_RANGE_PID11:
                str = "TCM ADC Range Test (PT11)";
                break;
            case TEST_TCM_ABS_RAWCAP_PID12:
                str = "TCM Abs Raw Cap Test (PT12)";
                break;
            case TEST_TCM_HYBRID_ABS_NOISE_PID1D:
                str = "TCM Hybrid Abs Noise Test (PT1D)";
                break;
            case TEST_TCM_EX_HIGH_RESISTANCE_PID05:
                str = "TCM Extended High Resistance Test (PT05)";
                break;

            default:
                return null;
        }
        return str;
    }

}