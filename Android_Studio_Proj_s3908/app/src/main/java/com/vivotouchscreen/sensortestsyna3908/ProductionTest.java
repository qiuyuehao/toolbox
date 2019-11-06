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
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;
import java.util.Collections;
import java.util.Locale;
import java.util.Vector;

class ProductionTest {

    private final String SYNA_TAG = "syna-apk";

    /* id to indicate a particular testing item */
    /* all IDs are available in the ProductionTestItems */
    int id;

    /* flag to indicate the testing item is enabled */
    boolean is_enabled;

    /* flag to indicate the previous testing is succeed */
    boolean is_test_succeed;

    /* vector to store the limit parsing from the external .ini file */
    private Vector<Integer> v_limit_ini_max;
    private Vector<Integer> v_limit_ini_min;
    private Vector<Integer> v_limit_ini_custom;
    /* value to store the limit being assigned on ui */
    private int n_limit_max;
    private int n_limit_min;

    /* flag to indicate the limit comes from the external .ini file */
    private boolean is_limit_ini_input;

    /* UI components */
    LinearLayout ui_layout;
    CheckBox ui_cbtn;
    EditText ui_edit_limit_max;
    EditText ui_edit_limit_min;
    EditText ui_edit_limit_custom;
    TextView ui_text_result;
    RadioButton ui_rbtn_ini_input;

    /********************************************************
     * constructor
     ********************************************************/
    ProductionTest() {
        id = 0;
        is_enabled = false;
        is_test_succeed = false;

        n_limit_max = 0;
        n_limit_min = 0;

        v_limit_ini_max = new Vector<>();
        v_limit_ini_min = new Vector<>();
        v_limit_ini_custom= new Vector<>();
        is_limit_ini_input = false;

        ui_layout = null;
        ui_cbtn = null;
        ui_edit_limit_max = null;
        ui_edit_limit_min = null;
        ui_edit_limit_custom = null;
        ui_text_result = null;
        ui_rbtn_ini_input = null;
    }

    /********************************************************
     * helpers to set/get the test limit
     ********************************************************/
    void onSetLimit(int min, int max) {
        n_limit_max = max;
        n_limit_min = min;

        Log.i(SYNA_TAG, "ProductionTest onSetLimit() " +
                String.format(Locale.getDefault(), "%s limit (min, max) = (%d, %d)",
                        getTestName(), onGetLimitMin(), onGetLimitMax()));
    }

    int onGetLimitMax() {
        if (v_limit_ini_max.size() == 0)
            return n_limit_max;
        else
            return Collections.max(v_limit_ini_max);
    }

    int onGetLimitMin() {
        if (v_limit_ini_min.size() == 0)
            return n_limit_min;
        else
            return Collections.min(v_limit_ini_min);
    }

    void onResetLimit() {
        is_limit_ini_input = false;

        v_limit_ini_max.clear();
        v_limit_ini_min.clear();
    }

    /********************************************************
     * helpers to get the limit string for the following test
	 *      rmi ex trx short test
     *      tcm trx trx short test
     *      tcm trx ground test
     ********************************************************/
    String onGetCustomLimitStr(boolean in_hex) {
        if (v_limit_ini_custom.size() == 0)
            return null;

        StringBuilder str = new StringBuilder();
        for (int i = 0; i < v_limit_ini_custom.size(); i++) {
            if (in_hex)
                str.append(String.format(Locale.getDefault(),"0x%02x,",
                        v_limit_ini_custom.get(i)));
            else
                str.append(String.format(Locale.getDefault(),"%d,",
                        v_limit_ini_custom.get(i)));
        }


        return str.toString();
    }

    /********************************************************
     * perform the test
     * function will prepare the test limit by calling
     * prepareTestLimit()
     *
     * next, call native function to perform the actual testing procedure
     *
     * at the end, use writeTestResultStr() to generate the string
     * containing the result aw well as the testing data
     ********************************************************/
    boolean doTest(NativeWrapper lib, StringBuilder sb)
    {
        int row = lib.getDevImageRow(true);
        int col = lib.getDevImageCol(true);
        int data_size = 0;

        final int TCM_TRX_TRX_SHORT_PIN_SIZE = 64;
        final int TCM_TRX_GROUND_PIN_SIZE = 55;

        /* buffers to store the test limit        */
        /* the data size could be 1 or data_size  */
        boolean is_limit1_valid = false;
        int[] limit1 = null;
        boolean is_limit2_valid = false;
        int[] limit2 = null;

        int size_limit_1 = 0;
        int size_limit_2 = 0;

        /* determine the size of testing data */
        switch (id) {
            case ProductionTestItems.TEST_RMI_NOISE_RT02:
            case ProductionTestItems.TEST_TCM_NOISE_PID0A:
                data_size = row * col;

                size_limit_1 = 0;
                is_limit1_valid = false;

                limit2 = prepareTestLimit(v_limit_ini_max, n_limit_max, data_size);
                is_limit2_valid = ((limit2 != null) && (limit2.length > 0));
                size_limit_2 = (is_limit2_valid)?limit2.length : 0;

                break;

            case ProductionTestItems.TEST_RMI_FULL_RAW_RT20:
            case ProductionTestItems.TEST_RMI_TDDI_FULL_RAW_RT92:
            case ProductionTestItems.TEST_RMI_ADC_RANGE_RT23:
            case ProductionTestItems.TEST_RMI_SENSOR_SPEED_RT22:
            case ProductionTestItems.TEST_RMI_TAGSMOISTURE_RT76:
            case ProductionTestItems.TEST_TCM_DRT_PID07:
            case ProductionTestItems.TEST_TCM_FULL_RAW_PID05:
            case ProductionTestItems.TEST_TCM_ADC_RANGE_PID11:
                data_size = row * col;

                limit1 = prepareTestLimit(v_limit_ini_min, n_limit_min, data_size);
                is_limit1_valid = ((limit1 != null) && (limit1.length > 0));
                size_limit_1 = (is_limit1_valid)?limit1.length : 0;

                limit2 = prepareTestLimit(v_limit_ini_max, n_limit_max, data_size);
                is_limit2_valid = ((limit2 != null) && (limit2.length > 0));
                size_limit_2 = (is_limit2_valid)?limit2.length : 0;

                break;

            case ProductionTestItems.TEST_TCM_TRX_TRX_SHORT_PID01:
                data_size = 8;

                limit1 = new int[TCM_TRX_TRX_SHORT_PIN_SIZE]; // to store the testing pins result
                is_limit1_valid = false;
                size_limit_1 = limit1.length;

                limit2 = prepareTestLimit(v_limit_ini_custom, TCM_TRX_TRX_SHORT_PIN_SIZE);
                is_limit2_valid = ((limit2 != null) && (limit2.length > 0));
                size_limit_2 = (is_limit2_valid)?limit2.length : 0;

                break;

            case ProductionTestItems.TEST_TCM_TRX_GROUND_PID03:
                data_size = 8;

                limit1 = new int[TCM_TRX_GROUND_PIN_SIZE]; // to store the testing pins result
                is_limit1_valid = false;
                size_limit_1 = limit1.length;

                limit2 = prepareTestLimit(v_limit_ini_custom, TCM_TRX_GROUND_PIN_SIZE);
                is_limit2_valid = ((limit2 != null) && (limit2.length > 0));
                size_limit_2 = (is_limit2_valid)?limit2.length : 0;

                break;

            case ProductionTestItems.TEST_RMI_ABS_OPEN_RT63:
            case ProductionTestItems.TEST_RMI_ABS_DELTA_RT59:
            case ProductionTestItems.TEST_TCM_ABS_RAWCAP_PID12:
            case ProductionTestItems.TEST_TCM_HYBRID_ABS_NOISE_PID1D:
                data_size = row + col;

                limit1 = prepareTestLimit(v_limit_ini_min, n_limit_min, data_size);
                is_limit1_valid = ((limit1 != null) && (limit1.length > 0));
                size_limit_1 = (is_limit1_valid)?limit1.length : 0;

                limit2 = prepareTestLimit(v_limit_ini_max, n_limit_max, data_size);
                is_limit2_valid = ((limit2 != null) && (limit2.length > 0));
                size_limit_2 = (is_limit2_valid)?limit2.length : 0;

                break;

            case ProductionTestItems.TEST_RMI_RT133:
                data_size = lib.getDevImageCol(false);

                size_limit_1 = 0;
                is_limit1_valid = false;

                limit2 = prepareTestLimit(v_limit_ini_max, n_limit_max, data_size);
                is_limit2_valid = ((limit2 != null) && (limit2.length > 0));
                size_limit_2 = (is_limit2_valid)?limit2.length : 0;

                break;

            default:
                Log.e(SYNA_TAG, "ProductionTest doTest() unknown id = " + id);
                sb.append("Error: unknown test id");
                break;
        }

        if (data_size == 0) {
            Log.e(SYNA_TAG, "ProductionTest doTest() invalid data size" );
            sb.append("Error: invalid data size");
            return false;
        }

        Log.i(SYNA_TAG, "ProductionTest doTest() " +
                String.format(Locale.getDefault(), "%s, size (limit_1 , limit_2)=(%d , %d)",
                        getTestName(), size_limit_1, size_limit_2));

        /* create a buffer to record testing data and result */
        int[] result = new int[data_size];

        Log.i(SYNA_TAG, "ProductionTest doTest() " +
                String.format(Locale.getDefault(), "%s, result data size = %d",
                        getTestName(), result.length));

        /* call function to perform the production test */
        /* retval = 0, pass the test                    */
        /*        > 0, testing failure                  */
        /*        < 0, fail to do the test, error out   */
        int retval = lib.onRunProductionTest(id, row, col, limit1, size_limit_1,
                limit2, size_limit_2, result, result.length);

        StringBuilder err = new StringBuilder();

        /* fail to do the test */
        if (retval < 0) {
            err.append(String.format(Locale.getDefault(), "Error: Fail to do test, %s\n",
                    getTestName()));
            err.append("\n").append("error messages\n");
            err.append(lib.getErrMessage());
        }
        /* testing failure occurs, get all messages from the native layer */
        else if (retval > 0) {
            err.append(lib.getErrMessage());
        }

        boolean is_pass = (retval == 0);

        /* create the test result string */
        sb.append(writeTestResultStr(lib, result, limit1, is_limit1_valid, limit2,
                is_limit2_valid, is_pass, err.toString(), retval));

        return is_pass;
    }

    /********************************************************
     * perform the extend high resistance test only
     * this function is separated from the doTest() because
     * the parameters and flow are different
     ********************************************************/
    boolean doTestExHighResistance(NativeWrapper lib, StringBuilder sb)
    {
        int row = lib.getDevImageRow(true);
        int col = lib.getDevImageCol(true);

        StringBuilder err = new StringBuilder();

        int[] result = new int[row * col];
        short[] ref;
        int temp;

        int[] result_tx = new int[row + col];
        int[] size_result_tx = new int[1];
        int[] result_rx = new int[row + col];
        int[] size_result_rx = new int[1];

        /* clone the reference frame */
        ref = new short[row * col];
        for (int i = 0; i < ref_frame.size(); i++) {
            temp = ref_frame.elementAt(i);
            ref[i] = (short)temp;
        }

        /* call function to perform the production  */
        int retval = lib.onRunProductionTestExHR(row, col, ref, limit_tixel, limit_txroe, limit_rxroe,
                result, result.length, result_tx, size_result_tx, result_rx, size_result_rx);

        /* fail to do the test */
        if (retval < 0) {
            err.append(String.format(Locale.getDefault(), "Error: Fail to do test, %s\n",
                    getTestName()));
            err.append("\n").append("error messages\n");
            err.append(lib.getErrMessage());
        }
        /* testing failure occurs, get all messages from the native layer */
        else if (retval > 0) {
            err.append(lib.getErrMessage());
        }

        boolean is_pass = (retval == 0);

        /* create the test result string */
        sb.append(writeTestResultStrExHighResistance(lib, result, result_tx, size_result_tx[0],
                result_rx, size_result_rx[0], is_pass, err.toString(), retval));

        return is_pass;
    }

    /********************************************************
     * perform the extend trx short test only
     * this function is separated from the doTest() because
     * the parameters and flow are different
     ********************************************************/
    boolean doTestExTRxShort(NativeWrapper lib, StringBuilder sb)
    {
        final int EX_TRX_SHORT_PIN_SIZE = 64;
        int rx = lib.getDevImageCol(false);

        StringBuilder err = new StringBuilder();

        if (v_limit_ini_custom.size() == 0) {
            Log.e(SYNA_TAG, "ProductionTest doTestExTRxShort() invalid limit size" );
            sb.append("Error: invalid limit size");
            return false;
        }

        int[] limit = new int[v_limit_ini_custom.size()];
        for (int i = 0; i < v_limit_ini_custom.size(); i++) {
            limit[i] = v_limit_ini_custom.elementAt(i);
        }

        int[] limit_ex_pin = new int[2];
        limit_ex_pin[0] = limit_min_ex_pin;
        limit_ex_pin[1] = limit_max_ex_pin;

        int[] result_data = new int[v_limit_ini_custom.size()];
        int[] result_data_ex_pin = new int[4 * rx];
        int[] result_pin = new int[EX_TRX_SHORT_PIN_SIZE];
        for (int i = 0; i < EX_TRX_SHORT_PIN_SIZE; i++)
            result_pin[i] = -1;

        // call function to perform the production test
        int retval = lib.onRunProductionTestExTRx(limit, limit_ex_pin, result_data,
                result_data_ex_pin, result_pin);

        /* fail to do the test */
        if (retval < 0) {
            err.append(String.format(Locale.getDefault(), "Error: Fail to do test, %s\n",
                    getTestName()));
            err.append("\n").append("error messages\n");
            err.append(lib.getErrMessage());
        }
        /* testing failure occurs, get all messages from the native layer */
        else if (retval > 0) {
            err.append(lib.getErrMessage());
        }

        boolean is_pass = (retval == 0);

        /* create the test result string */
        sb.append(writeTestResultStrExTRxShort(lib, result_data, result_data_ex_pin, result_pin,
                limit, limit_ex_pin, is_pass, err.toString(), retval));

        return is_pass;
    }

    /********************************************************
     * prepare and convert the test limit from vector to int[]
     * returning true means the test limit is valid
     ********************************************************/
    private int[] prepareTestLimit(Vector<Integer> v_limit, int n_limit, int max_size)
    {
        int[] out = null;

        if (is_limit_ini_input) {

            if (v_limit.size() > 0) {
                if (v_limit.size() == max_size) {

                    out = new int[max_size];
                    for (int i = 0; i < max_size; i++) {
                        out[i] = v_limit.elementAt(i);
                    }
                }
                else {
                    out = new int[1];
                    out[0] = v_limit.elementAt(0);
                }
            }
        }
        else {
            out = new int[1];
            out[0] = n_limit;
        }

        return out;
    }
    private int[] prepareTestLimit(Vector<Integer> v_limit, int max_size)
    {
        int[] out = null;

        if (is_limit_ini_input) {

            if (v_limit.size() > 0) {
                if (v_limit.size() == max_size) {

                    out = new int[max_size];
                    for (int i = 0; i < max_size; i++) {
                        out[i] = v_limit.elementAt(i);
                    }
                }
                else {
                    out = new int[1];
                    out[0] = v_limit.elementAt(0);
                }
            }
        }

        return out;
    }
    /********************************************************
     * helpers to return the flag whether the limit from the
     * cfg file is valid or not
     ********************************************************/
    boolean onHasLimitFromTestCfg() {
        return is_limit_ini_input;
    }
    void onClearFlagFromTestCfg() {
        is_limit_ini_input = false;
    }
    /********************************************************
     * the entry function to load the test limit
     * from the test cfg (.ini) file
     ********************************************************/
    void onParseLimitFromTestCfg(TestCfgFileManager test_cfg, int row, int col,
                                 StringBuilder err, NativeWrapper native_lib)
    {
        boolean ret;
        int data_size_least;
        int data_size_most;
        switch (id) {
            case ProductionTestItems.TEST_RMI_NOISE_RT02:
            case ProductionTestItems.TEST_TCM_NOISE_PID0A:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        "", test_cfg.STR_NOISE_TEST_LIMIT,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_RMI_FULL_RAW_RT20:
            case ProductionTestItems.TEST_RMI_TDDI_FULL_RAW_RT92:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_FULL_RAW_CAP_LIMIT_MIN, test_cfg.STR_FULL_RAW_CAP_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_TCM_DRT_PID07:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_DRT_TEST_LIMIT_MIN, test_cfg.STR_DRT_TEST_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_TCM_FULL_RAW_PID05:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_FULL_RAW_PT05_LIMIT_MIN, test_cfg.STR_FULL_RAW_PT05_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_TCM_TRX_TRX_SHORT_PID01:

                ret = getParameterFromTestCfgPinsMapping(test_cfg, err);
                if (!ret) {
                    Log.e(SYNA_TAG, "ProductionTest onParseLimitFromTestCfg() " +
                            "fail to get the valid info for pin assignment");
                    is_limit_ini_input = false;
                    break;
                }
                data_size_least = 1;
                data_size_most = 64;
                is_limit_ini_input = getCustomLimitFromTestCfg(test_cfg,
                        test_cfg.STR_TRX_TRX_SHORT_PT01_LIMIT,data_size_least, data_size_most, err);

                if (is_limit_ini_input) {
                    ret = native_lib.onCheckChannelsAssignment(cfg_image_rxes_offset,
                            cfg_image_rxes_length, cfg_image_txes_offset, cfg_image_txes_length);
                    if (!ret)
                    {
                        is_limit_ini_input = false;
                        v_limit_ini_custom.clear();
                    }
                }
                break;

            case ProductionTestItems.TEST_TCM_TRX_GROUND_PID03:

                ret = getParameterFromTestCfgPinsMapping(test_cfg, err);
                if (!ret) {
                    Log.e(SYNA_TAG, "ProductionTest onParseLimitFromTestCfg() " +
                            "fail to get the valid info for pin assignment");
                    is_limit_ini_input = false;
                    break;
                }
                data_size_least = 1;
                data_size_most = 55;
                is_limit_ini_input = getCustomLimitFromTestCfg(test_cfg,
                        test_cfg.STR_TRX_GROUND_PT03_LIMIT,data_size_least, data_size_most, err);

                if (is_limit_ini_input) {
                    ret = native_lib.onCheckChannelsAssignment(cfg_image_rxes_offset,
                            cfg_image_rxes_length, cfg_image_txes_offset, cfg_image_txes_length);
                    if (!ret)
                    {
                        is_limit_ini_input = false;
                        v_limit_ini_custom.clear();
                    }
                }
                break;

            case ProductionTestItems.TEST_TCM_ADC_RANGE_PID11:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_ADC_RANGE_PT11_LIMIT_MIN, test_cfg.STR_ADC_RANGE_PT11_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_TCM_ABS_RAWCAP_PID12:
                data_size_least = 1;
                data_size_most = row + col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_ABS_RAW_PT12_LIMIT_MIN, test_cfg.STR_ABS_RAW_PT12_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_TCM_HYBRID_ABS_NOISE_PID1D:
                data_size_least = 1;
                data_size_most = row + col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_ABS_NOISE_PT1D_LIMIT_MIN, test_cfg.STR_ABS_NOISE_PT1D_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_RMI_EX_HIGH_RESISTANCE_RT20:
            case ProductionTestItems.TEST_TCM_EX_HIGH_RESISTANCE_PID05:

                is_limit_ini_input = getLimitFromTestCfgExHighResistance(test_cfg, native_lib,
                        row, col, err);

                break;

            case ProductionTestItems.TEST_RMI_EX_TRX_SHORT_RT26100:

                is_limit_ini_input = getLimitFromTestCfgExTRxShort(test_cfg, native_lib, err);

                break;

            case ProductionTestItems.TEST_RMI_ABS_OPEN_RT63:
                data_size_least = 1;
                data_size_most = row + col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_ABS_OPEN_LIMIT_MIN, test_cfg.STR_ABS_OPEN_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_RMI_ADC_RANGE_RT23:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_ADC_RANGE_LIMIT_MIN, test_cfg.STR_ADC_RANGE_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_RMI_SENSOR_SPEED_RT22:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_SENSOR_SPEED_LIMIT_MIN, test_cfg.STR_SENSOR_SPEED_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_RMI_TAGSMOISTURE_RT76:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        "", test_cfg.STR_TAGSMOISTURE_TEST_LIMIT,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_RMI_RT133:
                data_size_least = 1;
                data_size_most = row * col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        "", test_cfg.STR_RT133_TEST_LIMIT,
                        data_size_least, data_size_most, err);
                break;

            case ProductionTestItems.TEST_RMI_ABS_DELTA_RT59:
                data_size_least = 1;
                data_size_most = row + col;
                is_limit_ini_input = getLimitFromTestCfg(test_cfg,
                        test_cfg.STR_ABS_DELTA_LIMIT_MIN, test_cfg.STR_ABS_DELTA_LIMIT_MAX,
                        data_size_least, data_size_most, err);
                break;

            default:
                Log.e(SYNA_TAG, "ProductionTest onGetLimitFromTestCfg() unknown item = " + id);
                break;
        }

        Log.i(SYNA_TAG, "ProductionTest onGetLimitFromTestCfg() " +
                String.format(Locale.getDefault(), "%s : is_limit_ini_input = %s",
                        getTestName(), (is_limit_ini_input)?"true":"false") );

    }

    /********************************************************
     * retrieve test limits from the test configuration (.in) file
     ********************************************************/
    private boolean getLimitFromTestCfg(TestCfgFileManager test_cfg, String key_str_min,
                                        String key_str_max, int size_least, int size_most,
                                        StringBuilder err)
    {
        boolean ret ;

        /* get the max. limit only */
        if ((key_str_min.length() <= 0) && (key_str_max.length() > 0)) {

            ret = test_cfg.onGetTestConfigurationData( key_str_max, v_limit_ini_max,
                                                       size_least, size_most);
            if (ret) {
                Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfg() limit_ini_max size = " +
                        v_limit_ini_max.size() );
            }
            else {
                Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfg() size of limit_ini_max is incorrect, " +
                        v_limit_ini_max.size() );

                if (err != null)
                    err.append(String.format(Locale.getDefault(), "Limit in %s is incorrect.",
                            key_str_max));

                v_limit_ini_max.clear();
            }
        }
        /* get the min. limit only */
        else if ((key_str_min.length() > 0) && (key_str_max.length() <= 0)) {

            ret = test_cfg.onGetTestConfigurationData( key_str_min, v_limit_ini_min,
                                                       size_least, size_most);
            if (ret) {
                Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfg() limit_ini_min size = " +
                        v_limit_ini_min.size() );
            }
            else {
                Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfg() size of limit_ini_min is incorrect, " +
                        v_limit_ini_min.size() );

                if (err != null)
                    err.append(String.format(Locale.getDefault(), "Limit in %s is incorrect.",
                            key_str_min));

                v_limit_ini_min.clear();
            }
        }
        /* get both the max. and the min. limit */
        else {

            ret = test_cfg.onGetTestConfigurationData( key_str_min, v_limit_ini_min,
                                                       key_str_max, v_limit_ini_max,
                                                       size_least, size_most);
            if (ret) {
                Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfg() limit_ini_max size = " +
                        v_limit_ini_max.size() );
                Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfg() limit_ini_min size = " +
                        v_limit_ini_min.size() );
            }
            else {
                Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfg() size of limit_ini_max is incorrect, " +
                        v_limit_ini_max.size() );
                Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfg() size of limit_ini_min is incorrect, " +
                        v_limit_ini_min.size() );

                if (err != null)
                    err.append(String.format(Locale.getDefault(), "Limit in %s is incorrect.",
                            key_str_max));
            }
        }

        return ret;
    }

    /********************************************************
     * retrieve test limits from the test configuration (.in) file
     ********************************************************/
    private boolean getCustomLimitFromTestCfg(TestCfgFileManager test_cfg, String key_str,
                                              int size_least, int size_most, StringBuilder err)
    {
        boolean ret = false ;

        /* get the custom limit */
        if (key_str.length() > 0) {

            ret = test_cfg.onGetTestConfigurationData( key_str, v_limit_ini_custom,
                    size_least, size_most);
            if (ret) {
                Log.i(SYNA_TAG, "ProductionTest getCustomLimitFromTestCfg() " +
                        "limit_ini_custom size = " + v_limit_ini_custom.size() );
            }
            else {
                Log.e(SYNA_TAG, "ProductionTest getCustomLimitFromTestCfg() " +
                        "size of limit_ini_custom is incorrect, " + v_limit_ini_custom.size() );

                if (err != null)
                    err.append(String.format(Locale.getDefault(), "Limit in %s is incorrect.",
                            key_str));

                v_limit_ini_custom.clear();
            }
        }

        return ret;
    }

    /********************************************************
     * helpers to return a string based on its id
     ********************************************************/
    String getTestName() {
        String str = ProductionTestItems.getTestNameStr(id);
        if (str == null)
            Log.e(SYNA_TAG, "ProductionTest getTestName() unknown id = " + id);

        return str;
    }

    /********************************************************
     * convert the test result into a string
     * so that it could put into the log buffer directly
     ********************************************************/
    private String writeTestResultStr(NativeWrapper lib, int[] result, int[] buf_1,
                                      boolean buf_1_valid, int[] buf_2, boolean buf_2_valid,
                                      boolean b_is_pass, String err_str, int err_val)
    {

        StringBuilder sb = new StringBuilder();
        int row = lib.getDevImageRow(true);
        int col = lib.getDevImageCol(true);

        /* testing name */
        String s = getTestName();
        if (s == null) {
            sb.append("warning: unknown testing\n");
            return null;
        }

        sb.append(String.format(Locale.getDefault(), "test item = %s\n", s));

        /* testing result, error out */
        if (err_val < 0) {
            sb.append("result = Error Out\n\n");
            sb.append(err_str).append("\n");

            sb.append("\n");
            return sb.toString();
        }

        /* testing result, pass  */
        if (b_is_pass) {
            sb.append("result = Pass\n");
        }
        /* testing result, fail  */
        else {
            sb.append("result = Fail\n");
            if (id == ProductionTestItems.TEST_RMI_NOISE_RT02) {
                sb.append("failed frames = ").append(err_val).append(" / 20 \n");
            }
            else
                sb.append("failed count = ").append(err_val).append("\n");
        }

        /* testing data  */
        sb.append("testing data \n");
        switch (id) {
            case ProductionTestItems.TEST_RMI_NOISE_RT02:
            case ProductionTestItems.TEST_RMI_FULL_RAW_RT20:
            case ProductionTestItems.TEST_RMI_TDDI_FULL_RAW_RT92:
            case ProductionTestItems.TEST_RMI_ADC_RANGE_RT23:
            case ProductionTestItems.TEST_RMI_SENSOR_SPEED_RT22:
            case ProductionTestItems.TEST_RMI_TAGSMOISTURE_RT76:
            case ProductionTestItems.TEST_TCM_NOISE_PID0A:
            case ProductionTestItems.TEST_TCM_DRT_PID07:
            case ProductionTestItems.TEST_TCM_FULL_RAW_PID05:
            case ProductionTestItems.TEST_TCM_ADC_RANGE_PID11:
                sb.append(writeTestDataInFrameLayout(result, row, col));
                sb.append(writeTestLimit(row, buf_1, buf_1_valid, buf_2, buf_2_valid));
               break;
            case ProductionTestItems.TEST_RMI_ABS_OPEN_RT63:
            case ProductionTestItems.TEST_RMI_ABS_DELTA_RT59:
            case ProductionTestItems.TEST_RMI_RT133:
            case ProductionTestItems.TEST_TCM_ABS_RAWCAP_PID12:
            case ProductionTestItems.TEST_TCM_HYBRID_ABS_NOISE_PID1D:
                sb.append(writeTestDataInAbsLayout(result, lib.getDevImageRow(true)));
                sb.append(writeTestLimit(lib.getDevImageRow(false),
                        buf_1, buf_1_valid, buf_2, buf_2_valid));
                break;
            case ProductionTestItems.TEST_TCM_TRX_TRX_SHORT_PID01:
            case ProductionTestItems.TEST_TCM_TRX_GROUND_PID03:
                sb.append(writeTestDataInPins(result, buf_1, 32));
                sb.append(writeTestLimit(32, buf_2, buf_2_valid));
                break;
            default:
                sb.append("ProductionTest writeTestResultStr() unknown id, ").append(id);
                sb.append("\n");
                return sb.toString();
        }

        /* error messages during the testing  */
        if (!b_is_pass) {
            sb.append("\n").append("error messages\n");
            sb.append(err_str).append("\n");
        }

        sb.append("\n");
        return sb.toString();
    }

    /********************************************************
     * write the testing data with a frame layout into the string buffer
     ********************************************************/
    private String writeTestDataInFrameLayout(int[] result_data, int row, int column) {
        int min_val, max_val;
        StringBuilder sb = new StringBuilder();
        int i, j, offset;

        if (result_data.length != (row * column)) {
            Log.e(SYNA_TAG, "ProductionTest writeTestDataInFrameLayout() size is mismatching");
            return null;
        }

        /* write the testing data in 2d layout */
        min_val = max_val = result_data[0];
        for(i = 0; i < column; i++) {
            if (i == 0) {
                sb.append("        ");
                for (j = 0 ; j < row; j ++)
                    sb.append(String.format(Locale.getDefault(), "[%02d]   ", j));
                sb.append("\n");
            }
            sb.append(String.format(Locale.getDefault(), "[%02d]: ", i));

            for(j = 0; j < row; j++) {
                offset = i*row + j;

                sb.append(String.format(Locale.getDefault(), "%5d, ", result_data[offset]));

                max_val = Math.max(max_val, result_data[offset]);
                min_val = Math.min(min_val, result_data[offset]);
            }
            sb.append("\n");
        }
        sb.append(String.format(Locale.getDefault(), "max. = %d, min. = %d\n\n",
                max_val, min_val));
        return sb.toString();
    }

    /********************************************************
     * write the testing data with a profile layout into the string buffer
     ********************************************************/
    private String writeTestDataInAbsLayout(int[] result_data, int num) {
        int min_val, max_val;
        StringBuilder sb = new StringBuilder();

        /* write the testing data in abs layout */
        int n = (result_data.length > num)? num : result_data.length;
        min_val = max_val = result_data[0];

        sb.append("      ");
        for (int i = 0 ; i < n ; i++) {
            sb.append(String.format(Locale.getDefault(), "[%02d]   ", i));
        }
        sb.append("\n").append("      ");
        for (int i = 0 ; i < n ; i++) {
            sb.append(String.format(Locale.getDefault(), "%5d, ", result_data[i]));

            max_val = Math.max(max_val, result_data[i]);
            min_val = Math.min(min_val, result_data[i]);
        }
        sb.append("\n").append("      ");
        for (int i = n ; i < result_data.length ; i++) {
            sb.append(String.format(Locale.getDefault(), "[%02d]   ", i));
        }
        sb.append("\n").append("      ");
        for (int i = n ; i < result_data.length ; i++) {
            sb.append(String.format(Locale.getDefault(), "%5d, ", result_data[i]));

            max_val = Math.max(max_val, result_data[i]);
            min_val = Math.min(min_val, result_data[i]);
        }
        sb.append("\n");
        sb.append(String.format(Locale.getDefault(), "max. = %d, min. = %d\n\n",
                max_val, min_val));

        return sb.toString();
    }

    /********************************************************
     * write the testing data in pins into the string buffer
     ********************************************************/
    private String writeTestDataInPins(int[] result_data, int[] result_pin, int num) {
        StringBuilder sb = new StringBuilder();

        sb.append("byte     =");
        for (int i = 0; i < result_data.length; i++) {
            sb.append(String.format(Locale.getDefault(), " [%d] 0x%02x ",
                    i, (byte)(result_data[i] & 0xff)));
        }
        sb.append("\n\n");
        if ((id == ProductionTestItems.TEST_TCM_TRX_TRX_SHORT_PID01) ||
                (id == ProductionTestItems.TEST_RMI_EX_TRX_SHORT_RT26100))
            sb.append("pins data ( '0' or '1' : data , '-1' : no assigned )\n");
        else
            sb.append("pins data \n");

        int total = result_pin.length;

        sb.append("     pin =");
        for (int i = 0; i < num; i++) {
            sb.append(String.format(Locale.getDefault(), " [%02d]", i));
        }
        sb.append("\n").append("          ");
        for (int i = 0; i < num; i++) {
            sb.append(String.format(Locale.getDefault(), "  %2d ", result_pin[i]));
        }
        sb.append("\n").append("     pin =");
        for (int i = num; i < total; i++) {
            sb.append(String.format(Locale.getDefault(), " [%02d]", i));
        }
        sb.append("\n").append("          ");
        for (int i = num; i < total; i++) {
            sb.append(String.format(Locale.getDefault(), "  %2d ", result_pin[i]));
        }
        sb.append("\n");

        return sb.toString();
    }

    /********************************************************
     * write the testing data of extended high resistance test
     * into the string buffer
     ********************************************************/
    private String writeTestResultStrExHighResistance(NativeWrapper lib, int[] result_data,
                                                      int[] result_tx, int size_result_tx,
                                                      int[] result_rx, int size_result_rx,
                                                      boolean b_is_pass, String err_str, int err_val)
    {
        StringBuilder sb = new StringBuilder();
        int row = lib.getDevImageRow(true);
        int col = lib.getDevImageCol(true);

        /* testing name */
        sb.append(String.format(Locale.getDefault(), "test item = %s\n", getTestName()));

        /* testing result, error out */
        if (err_val < 0) {
            sb.append("result = Error Out\n\n");
            sb.append(err_str).append("\n");

            sb.append("\n");
            return sb.toString();
        }

        /* testing result, pass  */
        if (b_is_pass) {
            sb.append("result = Pass\n");
        }
        /* testing result, fail  */
        else {
            sb.append("result = Fail\n");
            sb.append("failed count = ").append(err_val).append("\n");
        }

        /* testing data  */
        sb.append("testing data \n");

        int min_val, max_val;
        int i, j, offset;

        /* write the testing data in 2d layout */
        min_val = max_val = result_data[0];
        for(i = 0; i < col; i++) {
            if (i == 0) {
                sb.append("        ");
                for (j = 0 ; j < row; j ++)
                    sb.append(String.format(Locale.getDefault(), "[%02d]   ", j));
                sb.append("\n");
            }
            sb.append(String.format(Locale.getDefault(), "[%02d]: ", i));

            for(j = 0; j < row; j++) {
                offset = i*row + j;

                sb.append(String.format(Locale.getDefault(), "%5d, ", result_data[offset]));

                max_val = Math.max(max_val, result_data[offset]);
                min_val = Math.min(min_val, result_data[offset]);
            }
            sb.append("\n");
        }
        sb.append(String.format(Locale.getDefault(), "max. = %d, min. = %d\n\n",
                max_val, min_val));


        sb.append("testing data, tx roe\n");
        sb.append("size of tx roe =  ");
        sb.append(String.format(Locale.getDefault(), "%d\n", size_result_tx));
        sb.append("        ");
        for (i=0 ; i < size_result_tx; i++)
            sb.append(String.format(Locale.getDefault(), "[%02d]   ", i));
        sb.append("\n");
        sb.append("        ");
        for (i=0 ; i < size_result_tx; i++)
            sb.append(String.format(Locale.getDefault(), "%5d, ", result_tx[i]));
        sb.append("\n");

        sb.append("testing data, rx roe\n");
        sb.append("size of rx roe =  ");
        sb.append(String.format(Locale.getDefault(), "%d\n", size_result_rx));
        sb.append("        ");
        for (i=0 ; i < size_result_rx; i++)
            sb.append(String.format(Locale.getDefault(), "[%02d]   ", i));
        sb.append("\n");
        sb.append("        ");
        for (i=0 ; i < size_result_rx; i++)
            sb.append(String.format(Locale.getDefault(), "%5d, ", result_rx[i]));
        sb.append("\n\n");

        // save testing limit
        sb.append("test limit\n");
        sb.append("tixel limit =  ").append(
                String.format(Locale.getDefault(), "%d\n", limit_tixel));
        sb.append("txroe limit =  ").append(
                String.format(Locale.getDefault(), "%d\n", limit_txroe));
        sb.append("rxroe limit =  ").append(
                String.format(Locale.getDefault(), "%d\n", limit_rxroe));

        // save error log
        if (!b_is_pass) {
            sb.append("\n").append("error messages\n");
            sb.append(err_str).append("\n");
        }

        sb.append("\n");
        return sb.toString();
    }

    /********************************************************
     * write the testing data of extended high resistance test
     * into the string buffer
     ********************************************************/
    private String writeTestResultStrExTRxShort(NativeWrapper lib, int[] result_data, int[] result_data_ex_pin,
                                                int[] result_pin, int[] limit, int[] limit_ex_pin,
                                                boolean b_is_pass, String err_str, int err_val)
    {
        int num_ch_in_line = 32;
        StringBuilder sb = new StringBuilder();

        /* testing name */
        sb.append(String.format(Locale.getDefault(), "test item = %s\n", getTestName()));

        /* testing result, error out */
        if (err_val < 0) {
            sb.append("result = Error Out\n\n");
            sb.append(err_str).append("\n");

            sb.append("\n");
            return sb.toString();
        }

        /* testing result, pass  */
        if (b_is_pass) {
            sb.append("result = Pass\n");
        }
        /* testing result, fail  */
        else {
            sb.append("result = Fail\n");
            sb.append("failed count = ").append(err_val).append("\n");
        }

        /* testing data  */
        sb.append("testing data \n");
        sb.append(writeTestDataInPins(result_data, result_pin, num_ch_in_line));
        sb.append("\n");

        sb.append("extended pins data \n");

        StringBuilder info_ex_pin = new StringBuilder();
        int rx = lib.getDevImageCol(false);

        for (int i = 0; i < 4; i++) {
            String temp_str = "";

            if ((i == 0) && (result_pin[0] != -1))
                temp_str = "     ex pin = 0";
            else if ((i == 1) && (result_pin[1] != -1))
                temp_str = "     ex pin = 1";
            else if ((i == 2) && (result_pin[32] != -1))
                temp_str = "     ex pin = 32";
            else if ((i == 3) && (result_pin[33] != -1))
                temp_str = "     ex pin = 33";

            if (temp_str.length() > 0) {
                if (i == 0) {
                    info_ex_pin.append("          ");
                    for (int j = 0; j < rx; j++) {
                        info_ex_pin.append(String.format(Locale.getDefault(), " [%2d]",j));
                    }
                    info_ex_pin.append("\n");
                }

                info_ex_pin.append(temp_str).append("\n").append("          ");
                for (int j = 0; j < rx; j++) {
                    info_ex_pin.append(String.format(Locale.getDefault(), " %4d",
                            result_data_ex_pin[i*rx + j]));
                }
                info_ex_pin.append("\n");
            }
        }
        if (info_ex_pin.length() <= 0) {
            sb.append("     no extended pins are assigned\n");
        }
        else {
            sb.append(info_ex_pin.toString());
        }

        sb.append("\n");

        /* save testing limit */
        sb.append("test limit\n");
        for(int l: limit) {
            sb.append(String.format(Locale.getDefault(), "0x%02x ", l));
        }
        sb.append("\n\n");
        sb.append("test limit of extended pin\n");
        sb.append("limit_1 = ").append(String.format(Locale.getDefault(), "%d ", limit_ex_pin[0]));
        sb.append(", for target logical pin\n");
        sb.append("limit_2 = ").append(String.format(Locale.getDefault(), "%d ", limit_ex_pin[1]));
        sb.append(", for other pins\n");

        /* save error log */
        if (!b_is_pass) {
            sb.append("\n").append("error messages\n");
            sb.append(err_str).append("\n");
        }

        sb.append("\n");
        return sb.toString();
    }

    /********************************************************
     * write the testing limit into the string buffer
     ********************************************************/
    private String writeTestLimit(int n_in_row,
                                  int[] limit_1, boolean limit_1_valid,
                                  int[] limit_2, boolean limit_2_valid)
    {
        StringBuilder sb = new StringBuilder();

        /* testing limit  */
        if (limit_1_valid) {
            sb.append("test limit min.").append(" \n");

            for (int i = 0; i < limit_1.length; i++) {
                if ((i % n_in_row) == 0)
                    sb.append("      ");
                sb.append(String.format(Locale.getDefault(), "%5d, ", limit_1[i]));
                if ((i != 0) && (((i + 1) % n_in_row) == 0) && ((i + 1) != limit_1.length))
                    sb.append("\n");
            }

            sb.append("\n");
        }
        if (limit_2_valid) {
            sb.append("test limit max.").append(" \n");
            for (int i = 0; i < limit_2.length; i++) {
                if ((i % n_in_row) == 0)
                    sb.append("      ");
                sb.append(String.format(Locale.getDefault(), "%5d, ", limit_2[i]));
                if ((i != 0) && (((i + 1) % n_in_row) == 0) && ((i + 1) != limit_2.length))
                    sb.append("\n");
            }

            sb.append("\n");
        }

        return sb.toString();
    }

    private String writeTestLimit(int n_in_row, int[] limit, boolean limit_valid)
    {
        StringBuilder sb = new StringBuilder();

        /* testing limit  */
        if (limit_valid) {
            sb.append("test limit \n");

            if ((limit != null) && (limit.length > 0)) {
                for (int i = 0 ; i < limit.length ; i++) {
                    if ((i % n_in_row) == 0)
                        sb.append("          ");

                    sb.append(String.format(Locale.getDefault(), "  %2d ", limit[i]));

                    if ((i != 0) && (((i + 1) % n_in_row) == 0))
                        sb.append("\n");
                }
            }
            sb.append("\n");
        }

        return sb.toString();
    }
    /********************************************************
     * helpers to setup the test limit
     * specified for extended high resistance test
     ********************************************************/
    void onSetLimitExHighResistance(int tixel, int txroe, int rxroe) {
        limit_tixel = tixel;
        limit_txroe = txroe;
        limit_rxroe = rxroe;

        Log.i(SYNA_TAG, "ProductionTest onSetLimitExHighResistance() " +
                String.format(Locale.getDefault(), "%s limit (tixel, txroe, rxroe) = (%d, %d, %d)",
                        getTestName(), limit_tixel, limit_txroe, limit_rxroe));
    }

    int onGetLimitTixel() {
        return limit_tixel;
    }
    int onGetLimitTxRoe() {
        return limit_txroe;
    }
    int onGetLimitRxRoe() {
        return limit_rxroe;
    }

    private int limit_tixel;
    private int limit_txroe;
    private int limit_rxroe;
    private Vector<Integer> ref_frame = new Vector<>();

    /********************************************************
     * helpers to setup the test limit
     * specified for extended high resistance test using
     ********************************************************/
    private boolean getLimitFromTestCfgExHighResistance(TestCfgFileManager test_cfg, NativeWrapper native_lib,
                                                        int row, int col, StringBuilder err)
    {
        int size = row * col;
        String str_ref_frame;
        String str_limit_tixels;
        String str_limit_rxroe;
        String str_limit_txroe;

        if (native_lib.isDevTCM()) {
            str_ref_frame = test_cfg.STR_EX_HIGH_RESISTANCE_PT05_REF;
            str_limit_tixels = test_cfg.STR_EX_HIGH_RESISTANCE_PT05_LIMIT_TIXELS;
            str_limit_rxroe = test_cfg.STR_EX_HIGH_RESISTANCE_PT05_LIMIT_RX_ROE;
            str_limit_txroe = test_cfg.STR_EX_HIGH_RESISTANCE_PT05_LIMIT_TX_ROE;
        }
        else if (native_lib.isDevRMI()) {
            str_ref_frame = test_cfg.STR_EX_HIGH_RESISTANCE_REF;
            str_limit_tixels = test_cfg.STR_EX_HIGH_RESISTANCE_LIMIT_SURFACE;
            str_limit_rxroe = test_cfg.STR_EX_HIGH_RESISTANCE_LIMIT_RX_ROE;
            str_limit_txroe = test_cfg.STR_EX_HIGH_RESISTANCE_LIMIT_TX_ROE;
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfgExHighResistance() unknown device");

            if (err != null)
                err.append("Unknown device type while parsing the configuration of ex high resistance test");
            return false;
        }

        boolean ret_ref = test_cfg.onGetTestConfigurationData(str_ref_frame, ref_frame, size, size);
        if (ret_ref) {
            Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfgExHighResistance() " +
                    "ref_frame size = " + ref_frame.size() );
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfgExHighResistance() " +
                    "ref_frame size is incorrect. size = " + ref_frame.size() );
            ref_frame.clear();

            if (err != null)
                err.append(String.format(Locale.getDefault(), "Ref frame in %s is incorrect.",
                        str_ref_frame));
        }

        Vector<Integer> v_limit_tixel = new Vector<>();
        boolean ret_limit_tixel = test_cfg.onGetTestConfigurationData( str_limit_tixels,
                v_limit_tixel, 1, 1);
        if (ret_limit_tixel) {
            Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfgExHighResistance() " +
                    "v_limit_tixel size = " + v_limit_tixel.size() );
            limit_tixel = v_limit_tixel.elementAt(0);
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfgExHighResistance() " +
                    "v_limit_tixel is incorrect. size = " + v_limit_tixel.size() );
            limit_tixel = 0;

            if (err != null)
                err.append(String.format(Locale.getDefault(), "Limit in %s is incorrect.",
                        str_limit_tixels));
        }

        Vector<Integer> v_limit_rxroe = new Vector<>();
        Vector<Integer> v_limit_txroe = new Vector<>();
        boolean ret_limit_roe = test_cfg.onGetTestConfigurationData(
                str_limit_rxroe, v_limit_rxroe, str_limit_txroe, v_limit_txroe,
                1, 1);
        if (ret_limit_roe) {
            Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfgExHighResistance() " +
                    "v_limit_rxroe size = " + v_limit_rxroe.size());
            Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfgExHighResistance() " +
                    "v_limit_txroe size = " + v_limit_txroe.size());

            limit_rxroe = v_limit_rxroe.elementAt(0);
            limit_txroe = v_limit_txroe.elementAt(0);
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfgExHighResistance() " +
                    "limit is incorrect. rxroe size = " + v_limit_rxroe.size() +
                    " ,  txroe size = " + v_limit_txroe.size());
            limit_rxroe = 0;
            limit_txroe = 0;

            if (err != null)
                err.append(String.format(Locale.getDefault(), "Limit in %s or %s is incorrect.",
                        str_limit_rxroe, str_limit_txroe));
        }

        return ret_ref && ret_limit_tixel && ret_limit_roe;
    }

    /********************************************************
     * helpers to get the information of pin mapping
     * specified for the following test using
     *      trx trx short test
     *      trx ground test
     ********************************************************/
    private boolean getParameterFromTestCfgPinsMapping(TestCfgFileManager test_cfg, StringBuilder err)
    {
        Vector<Integer> tmp_v = new Vector<>();

        /* parse the imageRxes offset in static config */
        int ret_rx_off = test_cfg.onGetCustomTestConfigurationData(
                test_cfg.STR_TCM_CONFIG_IMAGE_RXES_OFFSET, tmp_v);
        if (ret_rx_off > 0) {
            cfg_image_rxes_offset = tmp_v.elementAt(0);
            Log.i(SYNA_TAG, "ProductionTest getParameterFromTestCfgPinsMapping() " +
                    "rxes_offset = " + cfg_image_rxes_offset );
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getParameterFromTestCfgPinsMapping() " +
                    "TCM_CONFIG_IMAGE_RXES_OFFSET is not defined");

            if (err != null)
                err.append("Test config TCM_CONFIG_IMAGE_RXES_OFFSET is not defined.");
        }
        /* parse the imageRxes length in static config */
        int ret_rx_len = test_cfg.onGetCustomTestConfigurationData(
                test_cfg.STR_TCM_CONFIG_IMAGE_RXES_LENGTH, tmp_v);
        if (ret_rx_len > 0) {
            cfg_image_rxes_length = tmp_v.elementAt(0);
            Log.i(SYNA_TAG, "ProductionTest getParameterFromTestCfgPinsMapping() " +
                    "rxes_length = " + cfg_image_rxes_length );
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getParameterFromTestCfgPinsMapping() " +
                    "TCM_CONFIG_IMAGE_RXES_LENGTH is not defined");

            if (err != null)
                err.append("Test config TCM_CONFIG_IMAGE_RXES_LENGTH is not defined.");
        }
        /* parse the imageTxes offset in static config */
        int ret_tx_off = test_cfg.onGetCustomTestConfigurationData(
                test_cfg.STR_TCM_CONFIG_IMAGE_TXES_OFFSET, tmp_v);
        if (ret_tx_off > 0) {
            cfg_image_txes_offset = tmp_v.elementAt(0);
            Log.i(SYNA_TAG, "ProductionTest getParameterFromTestCfgPinsMapping() " +
                    "txes_offset = " + cfg_image_txes_offset );
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getParameterFromTestCfgPinsMapping() " +
                    "TCM_CONFIG_IMAGE_TXES_OFFSET is not defined");

            if (err != null)
                err.append("Test config TCM_CONFIG_IMAGE_TXES_OFFSET is not defined.");
        }
        /* parse the imageRxes length in static config */
        int ret_tx_len = test_cfg.onGetCustomTestConfigurationData(
                test_cfg.STR_TCM_CONFIG_IMAGE_TXES_LENGTH, tmp_v);
        if (ret_tx_len > 0) {
            cfg_image_txes_length = tmp_v.elementAt(0);
            Log.i(SYNA_TAG, "ProductionTest getParameterFromTestCfgPinsMapping() " +
                    "txes_length = " + cfg_image_txes_length );
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getParameterFromTestCfgPinsMapping() " +
                    "TCM_CONFIG_IMAGE_TXES_LENGTH is not defined");

            if (err != null)
                err.append("Test config TCM_CONFIG_IMAGE_TXES_LENGTH is not defined.");
        }

        return (ret_rx_off > 0) && (ret_rx_len > 0) && (ret_tx_off > 0) && (ret_tx_len > 0);
    }

    private int cfg_image_rxes_offset;
    private int cfg_image_rxes_length;
    private int cfg_image_txes_offset;
    private int cfg_image_txes_length;

    /********************************************************
     * helpers to setup the test limit
     * specified for extended trx short test
     ********************************************************/
    void onSetLimitExTRxShort(int ex_limit_1, int ex_limit_2) {
        limit_min_ex_pin = ex_limit_1;
        limit_max_ex_pin = ex_limit_2;

        Log.i(SYNA_TAG, "ProductionTest onSetLimitExTRxShort() " +
                String.format(Locale.getDefault(), "%s limit (min, max) = (%d, %d)",
                        getTestName(), limit_min_ex_pin, limit_max_ex_pin));
    }

    int onGetLimitMinExPin() {
        return limit_min_ex_pin;
    }
    int onGetLimitMaxExPin() {
        return limit_max_ex_pin;
    }

    private int limit_min_ex_pin;
    private int limit_max_ex_pin;

    /********************************************************
     * helpers to setup the test limit
     * specified for extended high resistance test using
     ********************************************************/
    private boolean getLimitFromTestCfgExTRxShort(TestCfgFileManager test_cfg, NativeWrapper native_lib,
                                                  StringBuilder err)
    {
        String str_limit;
        String str_limit_1_ex_pin;
        String str_limit_2_ex_pin;

        if (native_lib.isDevRMI()) {
            str_limit = test_cfg.STR_EX_TRX_SHORT_RT26_LIMIT;
            str_limit_1_ex_pin = test_cfg.STR_EX_TRX_SHORT_RT100_LIMIT_1;
            str_limit_2_ex_pin = test_cfg.STR_EX_TRX_SHORT_RT100_LIMIT_2;
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfgExTRxShort() unknown device");

            if (err != null)
                err.append("Unknown device type while parsing the configuration of ex high resistance test");
            return false;
        }

        int ret_limit = test_cfg.onGetCustomTestConfigurationData(str_limit, v_limit_ini_custom);
        if (ret_limit > 0) {
            Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfgExTRxShort() " +
                    "v_limit_ini_custom size = " + v_limit_ini_custom.size() );
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfgExTRxShort() " +
                    "v_limit_ini_custom is incorrect. size = " + v_limit_ini_custom.size() );
            v_limit_ini_custom.clear();

            if (err != null)
                err.append(String.format(Locale.getDefault(), "Limit in %s is incorrect.",
                        str_limit));
        }

        Vector<Integer> v_limit_1 = new Vector<>();
        Vector<Integer> v_limit_2 = new Vector<>();
        boolean ret_limit_ex_pin = test_cfg.onGetTestConfigurationData(
                str_limit_1_ex_pin, v_limit_1, str_limit_2_ex_pin, v_limit_2,
                1, 1);
        if (ret_limit_ex_pin) {
            Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfgExTRxShort() " +
                    "limit_min_ex_pin size = " + v_limit_1.size());
            Log.i(SYNA_TAG, "ProductionTest getLimitFromTestCfgExTRxShort() " +
                    "limit_max_ex_pin size = " + v_limit_2.size());

            limit_min_ex_pin = v_limit_1.elementAt(0);
            limit_max_ex_pin = v_limit_2.elementAt(0);
        }
        else {
            Log.e(SYNA_TAG, "ProductionTest getLimitFromTestCfgExTRxShort() " +
                    "limit is incorrect. limit_min_ex_pin size = " + v_limit_1.size() +
                    " ,  limit_max_ex_pin size = " + v_limit_2.size());
            limit_min_ex_pin = 0;
            limit_max_ex_pin = 0;

            if (err != null)
                err.append(String.format(Locale.getDefault(), "Limit in %s or %s is incorrect.",
                        str_limit_1_ex_pin, str_limit_2_ex_pin));
        }

        return (ret_limit > 0) && ret_limit_ex_pin;
    }
}