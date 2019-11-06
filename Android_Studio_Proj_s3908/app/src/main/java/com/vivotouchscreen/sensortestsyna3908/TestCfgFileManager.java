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
package com.vivotouchscreen.sensortestsyna3908;

import android.util.Log;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Vector;

class TestCfgFileManager {

    private final String SYNA_TAG = "syna-apk";

    private String str_test_ini_file;

    /********************************************************
     * the buffer to save the data in configuration file
     *******************************************************/
    private Vector<String> test_cfg_buf;

    /********************************************************
     * constructor
     ********************************************************/
    TestCfgFileManager() {
        str_test_ini_file = null;
        test_cfg_buf = new Vector<>();
    }

    /********************************************************
     * fixed string defined in the test configuration file
     *******************************************************/
    final String STR_TCM_CONFIG_IMAGE_RXES_OFFSET = "TCM_CONFIG_IMAGE_RXES_OFFSET";
    final String STR_TCM_CONFIG_IMAGE_RXES_LENGTH = "TCM_CONFIG_IMAGE_RXES_LENGTH";
    final String STR_TCM_CONFIG_IMAGE_TXES_OFFSET = "TCM_CONFIG_IMAGE_TXES_OFFSET";
    final String STR_TCM_CONFIG_IMAGE_TXES_LENGTH = "TCM_CONFIG_IMAGE_TXES_LENGTH";

    /* for TEST_RMI_NOISE_RT02 and TEST_TCM_NOISE_PID0A */
    final String STR_NOISE_TEST_LIMIT = "NOISE_TEST_LIMIT";
    /* for TEST_TCM_DRT_PID07 */
    final String STR_DRT_TEST_LIMIT_MIN = "DRT_TEST_LIMIT_MIN";
    final String STR_DRT_TEST_LIMIT_MAX = "DRT_TEST_LIMIT_MAX";
    /* for TEST_RMI_FULL_RAW_RT20 and TEST_RMI_TDDI_FULL_RAW_RT92 */
    final String STR_FULL_RAW_CAP_LIMIT_MIN = "FULL_RAW_CAP_LIMIT_MIN";
    final String STR_FULL_RAW_CAP_LIMIT_MAX = "FULL_RAW_CAP_LIMIT_MAX";
    /* for TEST_TCM_FULL_RAW_PID05 */
    final String STR_FULL_RAW_PT05_LIMIT_MIN = "FULLRAW_PT05_LIMIT_MIN";
    final String STR_FULL_RAW_PT05_LIMIT_MAX = "FULLRAW_PT05_LIMIT_MAX";
    /* for TEST_TCM_TRX_TRX_SHORT_PID01 */
    final String STR_TRX_TRX_SHORT_PT01_LIMIT = "TRX_TRX_SHORT_PT01_LIMIT";
    /* for TEST_TCM_TRX_GROUND_PID03 */
    final String STR_TRX_GROUND_PT03_LIMIT = "TRX_GROUND_PT03_LIMIT";
    /* for TEST_TCM_ADC_RANGE_PID11 */
    final String STR_ADC_RANGE_PT11_LIMIT_MIN = "ADC_RANGE_PT11_LIMIT_MIN";
    final String STR_ADC_RANGE_PT11_LIMIT_MAX = "ADC_RANGE_PT11_LIMIT_MAX";
    /* for TEST_TCM_ABS_RAWCAP_PID12 */
    final String STR_ABS_RAW_PT12_LIMIT_MIN = "ABS_RAW_PT12_LIMIT_MIN";
    final String STR_ABS_RAW_PT12_LIMIT_MAX = "ABS_RAW_PT12_LIMIT_MAX";
    /* for TEST_TCM_HYBRID_ABS_NOISE_PID1D */
    final String STR_ABS_NOISE_PT1D_LIMIT_MIN = "ABS_NOISE_PT1D_LIMIT_MIN";
    final String STR_ABS_NOISE_PT1D_LIMIT_MAX = "ABS_NOISE_PT1D_LIMIT_MAX";
    /* for TEST_TCM_EX_HIGH_RESISTANCE_PID05 */
    final String STR_EX_HIGH_RESISTANCE_PT05_REF = "EX_HIGH_RESISTANCE_PT05_REF";
    final String STR_EX_HIGH_RESISTANCE_PT05_LIMIT_TIXELS = "EX_HIGH_RESISTANCE_PT05_LIMIT_TIXELS";
    final String STR_EX_HIGH_RESISTANCE_PT05_LIMIT_RX_ROE = "EX_HIGH_RESISTANCE_PT05_LIMIT_RX_ROE";
    final String STR_EX_HIGH_RESISTANCE_PT05_LIMIT_TX_ROE = "EX_HIGH_RESISTANCE_PT05_LIMIT_TX_ROE";
    /* for TEST_RMI_EX_HIGH_RESISTANCE_RT20 */
    final String STR_EX_HIGH_RESISTANCE_REF = "EX_HIGH_RESISTANCE_REF";
    final String STR_EX_HIGH_RESISTANCE_LIMIT_SURFACE = "EX_HIGH_RESISTANCE_LIMIT_SURFACE";
    final String STR_EX_HIGH_RESISTANCE_LIMIT_RX_ROE = "EX_HIGH_RESISTANCE_LIMIT_RX_ROE";
    final String STR_EX_HIGH_RESISTANCE_LIMIT_TX_ROE = "EX_HIGH_RESISTANCE_LIMIT_TX_ROE";
    /* for TEST_RMI_EX_TRX_SHORT_RT26100 */
    final String STR_EX_TRX_SHORT_RT26_LIMIT = "EX_TRX_SHORT_RT26_LIMIT";
    final String STR_EX_TRX_SHORT_RT100_LIMIT_1 = "EX_TRX_SHORT_RT100_LIMIT_1";
    final String STR_EX_TRX_SHORT_RT100_LIMIT_2 = "EX_TRX_SHORT_RT100_LIMIT_2";
    /* for TEST_RMI_ABS_OPEN_RT63 */
    final String STR_ABS_OPEN_LIMIT_MIN = "ABS_OPEN_LIMIT_MIN";
    final String STR_ABS_OPEN_LIMIT_MAX = "ABS_OPEN_LIMIT_MAX";
    /* for TEST_RMI_ADC_RANGE_RT23 */
    final String STR_ADC_RANGE_LIMIT_MIN = "ADC_RANGE_LIMIT_MIN";
    final String STR_ADC_RANGE_LIMIT_MAX = "ADC_RANGE_LIMIT_MAX";
    /* for TEST_RMI_SENSOR_SPEED_RT22 */
    final String STR_SENSOR_SPEED_LIMIT_MIN = "SENSOR_SPEED_LIMIT_MIN";
    final String STR_SENSOR_SPEED_LIMIT_MAX = "SENSOR_SPEED_LIMIT_MAX";
    /* for TEST_RMI_TAGSMOISTURE_RT76 */
    final String STR_TAGSMOISTURE_TEST_LIMIT = "TAGSMOISTURE_TEST_LIMIT";
    /* for TEST_RMI_RT133 */
    final String STR_RT133_TEST_LIMIT = "RT133_TEST_LIMIT";
    /* for TEST_RMI_ABS_DELTA_RT59 */
    final String STR_ABS_DELTA_LIMIT_MIN = "ABS_DELTA_LIMIT_MIN";
    final String STR_ABS_DELTA_LIMIT_MAX = "ABS_DELTA_LIMIT_MAX";

    /********************************************************
     * helper function to ensure appointed .ini file is available
     *******************************************************/
    boolean onCheckTestCfgFile(String str_file) {
        /* handle the empty string */
        if(str_file.length() < 1){
            return false;
        }
        /* if the last character is not '.ini' */
        if(!str_file.endsWith(".ini")) {
            Log.e(SYNA_TAG, "TestCfgFileManager onCheckTestCfgFile() file extension is not .ini");
            return false;
        }
        File file = new File(str_file);
        if (!file.exists()) {
            Log.e(SYNA_TAG, "TestCfgFileManager onCheckTestCfgFile() file not exist, " + str_file );
            return false;
        }
        str_test_ini_file = str_file;
        Log.i(SYNA_TAG, "TestCfgFileManager onCheckTestCfgFile() done, " + str_test_ini_file );
        return true;
    } /* end onCheckTestCfgFile() */

    /********************************************************
     * helper function to parse the content in test
     * configuration file
     *  1. to skip the line if '#' existed
     *  2. to skip the line if length <= 1
     *  3. spilt with equal sign '=' and save to the vector
     *******************************************************/
    boolean onParseTestConfigurationFile(String cfg_file, int row, int col, StringBuilder err) {
        /* clear the vector buffer always */
        if (!test_cfg_buf.isEmpty())
            test_cfg_buf.clear();

        Log.i(SYNA_TAG, "TestCfgFileManager: onParseTestConfigurationFile: +");

        StringBuilder sb = new StringBuilder();

        try {
            FileReader file_reader = new FileReader(cfg_file);
            BufferedReader buffered_reader = new BufferedReader(file_reader);
            String text_line = buffered_reader.readLine();

            do {
                /* replace all blanking in the string */
                text_line = text_line.replaceAll("\\s+", "");

                /* save the string without "#" to the buffer */
                if ((!text_line.contains("#")) && (text_line.length() > 1)) {

                    /* once the line containing the '=' */
                    int equal_sign_idx = text_line.lastIndexOf('=');
                    if (equal_sign_idx > 0) {
                        // convert the builder to string and save into the vector
                        if (sb.length() > 0) {
                            test_cfg_buf.add(sb.toString());
                            sb.delete(0, sb.length());
                        }
                        // add token to the vector
                        test_cfg_buf.add(text_line.substring(0, equal_sign_idx));
                        // add following string at the same line to the builder
                        String temp = text_line.substring(equal_sign_idx + 1);
                        if (temp.isEmpty()) {
                            Log.e(SYNA_TAG, "invalid configuration");
                        }
                        else
                            sb.append(temp);
                    }
                    else {
                        // add the entire line to the builder because it will be data always
                        sb.append(text_line);
                    }
                }

                /* read next line */
                text_line = buffered_reader.readLine();
            } while( text_line != null );

            /* add last line to the vector */
            if (sb.length() > 0) {
                test_cfg_buf.add(sb.toString());
            }

            /* close file */
            file_reader.close();

        } catch(Exception e) {
            Log.e(SYNA_TAG, "TestCfgFileManager onParseTestConfigurationFile() " +
                    "FileReader Exception, " + str_test_ini_file);
            err.append("FileReader Exception, ").append(str_test_ini_file).append(" is not found");
            return false;
        }

        /* confirm the row and col defined in configuration file are valid */
        Vector<Integer> data = new Vector<>();
        if (getTestCfgData("NUM_OF_ROW", data) != 1)  {
            Log.e(SYNA_TAG, "TestCfgFileManager onParseTestConfigurationFile() " +
                    "NUM_OF_ROW is undefined.");
            err.append("NUM_OF_ROW is undefined.");
            return false;
        }
        if (data.get(0) != row)  {
            Log.e(SYNA_TAG, "TestCfgFileManager onParseTestConfigurationFile() " +
                    "NUM_OF_ROW is incorrect. " + data.get(0));
            err.append("NUM_OF_ROW is incorrect, (").append(String.valueOf(data.get(0))).append(")");
            return false;
        }

        Log.i(SYNA_TAG, "TestCfgFileManager onParseTestConfigurationFile() " +
                "row = " + data.get(0));

        data.clear();
        if (getTestCfgData("NUM_OF_COL", data) != 1)  {
            Log.e(SYNA_TAG, "TestCfgFileManager onParseTestConfigurationFile() " +
                    "NUM_OF_COL is undefined.");
            err.append("NUM_OF_COL is undefined.");
            return false;
        }
        if (data.get(0) != col)  {
            Log.e(SYNA_TAG, "TestCfgFileManager onParseTestConfigurationFile() " +
                    "NUM_OF_COL is incorrect. " + data.get(0));
            err.append("NUM_OF_COL is incorrect, (").append(String.valueOf(data.get(0))).append(")");
            return false;
        }

        Log.i(SYNA_TAG, "TestCfgFileManager onParseTestConfigurationFile() " +
                "col = " + data.get(0));

        return true;
    } /* end of onParseTestConfigurationFile */

    /********************************************************
     * function to find whether the key is defined in configuration file or not
     *
     * return true if the data of key word is found
     * otherwise, return false
     ********************************************************/
    boolean onGetTestConfigurationData(String key, Vector<Integer> v,
                                       int data_size_least, int data_size_most)
    {

        if (test_cfg_buf.isEmpty()) {
            Log.e(SYNA_TAG, "TestCfgFileManager onGetCustomTestConfigurationData() " +
                    "test_cfg_buf is empty");
            return false;
        }
        boolean is_para_ready;
        int retval;

        // clear the vector
        v.clear();

        // get the data of key
        retval = getTestCfgData(key, v);
        if ((retval == data_size_least) || (retval == data_size_most)) {
            is_para_ready = true;
        }
        else {
            Log.e(SYNA_TAG, "TestCfgFileManager onGetTestConfigurationData() " +
                    "the content of " + key + " is incorrect. (" + retval + ")" );
            is_para_ready = false;
        }

        return is_para_ready;
    }
    boolean onGetTestConfigurationData(String key_1, Vector<Integer> v_1,
                                       String key_2, Vector<Integer> v_2,
                                       int data_size_least, int data_size_most)
    {
        boolean is_para1_ready;
        boolean is_para2_ready;
        int retval;
        // clear the vector
        v_1.clear();
        v_2.clear();

        // get the data of key_1
        retval = getTestCfgData(key_1, v_1);
        if ((retval == data_size_least) || (retval == data_size_most)) {
            is_para1_ready = true;
        }
        else {
            Log.e(SYNA_TAG, "TestCfgFileManager onGetTestConfigurationData() " +
                    "the content of " + key_1 + " is incorrect. (" + retval + ")" );
            is_para1_ready = false;
        }
        // get the data of key_2
        retval = getTestCfgData(key_2, v_2);
        if ((retval == data_size_least) || (retval == data_size_most)) {
            is_para2_ready = true;
        }
        else {
            Log.e(SYNA_TAG, "TestCfgFileManager onGetTestConfigurationData() " +
                    "the content of " + key_2 + " is incorrect. (" + retval + ")" );
            is_para2_ready = false;
        }

        return (is_para1_ready && is_para2_ready);
    }

    /********************************************************
     * function to get the custom configuration
     *
     * return the length of data if founded
     ********************************************************/
    int onGetCustomTestConfigurationData(String key, Vector<Integer> v) {

        if (test_cfg_buf.isEmpty()) {
            Log.e(SYNA_TAG, "TestCfgFileManager onGetCustomTestConfigurationData() " +
                    "test_cfg_buf is empty");
            return -1;
        }
        int retval;

        // clear the vector
        v.clear();

        // get the data of key
        retval = getTestCfgData(key, v);

        return retval;
    }
    /********************************************************
     * function to get the value with particular key
     * return the size of data
     ********************************************************/
    private int getTestCfgData(String key, Vector<Integer> v) {

        if (test_cfg_buf.isEmpty()) {
            Log.e(SYNA_TAG, "TestCfgFileManager getTestCfgData() " +
                    "test_cfg_buf is empty");
            return 0;
        }

        for (int i = 0; i < test_cfg_buf.size(); i++) {
            String token = test_cfg_buf.get(i);

            if (token.equals(key)) {
                Log.i(SYNA_TAG, "token = " + token);

                String data_str = "a";
                if (i+1 < test_cfg_buf.size()) {
                    data_str = test_cfg_buf.get(i+1);
                }

                if (data_str.matches("[a-zA-Z]+")) {
                    Log.e(SYNA_TAG, "TestCfgFileManager getTestCfgData() invalid configuration");
                    return 0;
                }
                else {
                    String[] parts = data_str.split(",");
                    for (String value: parts) {
                        if (value.contains("0x"))
                            v.add(Integer.decode(value));
                        else
                            v.add(Integer.valueOf(value));
                    }
                }

           } // end token.equals(key)
        } // end for loop

        return v.size();
    }

}