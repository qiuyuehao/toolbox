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

import android.util.Log;
import java.util.Locale;

class NativeWrapper {

    /* Used to load the 'native-lib' library on application startup. */
    /* A native method that is implemented by the 'native-lib' native library */
    static {
        System.loadLibrary("native_syna");
    }

    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * constructor
     ********************************************************/
    NativeWrapper() {
        str_syna_dev_node = null;
        is_syna_dev_rmi = false;
        is_syna_dev_tcm = false;

        is_initialized = false;

        touch_pos_x = new int[MAX_FINGER_NUMBER];
        touch_pos_y = new int[MAX_FINGER_NUMBER];
        touch_status = new int[MAX_FINGER_NUMBER];
    }

    /********************************************************
     * basic helper functions to check the installed syna device
     *
     * onCheckDev() is used to search whether the available node exists
     * if want to assign a specific node, please use onSetupDev() instead
     ********************************************************/

    private String str_syna_dev_node;
    private boolean is_syna_dev_rmi;
    private boolean is_syna_dev_tcm;

    private boolean is_initialized;

    boolean onCheckDev() {
        /* to search the synaptics device */
        str_syna_dev_node = findSynaDevJNI();
        if (str_syna_dev_node != null){
            Log.i(SYNA_TAG, "NativeWrapper onCheckDev() device node is installed, "
                    + str_syna_dev_node );

            is_syna_dev_rmi = str_syna_dev_node.contains("rmi");
            is_syna_dev_tcm = str_syna_dev_node.contains("tcm");
            return true;
        }
        else {
            Log.e(SYNA_TAG, "NativeWrapper onCheckDev() fail to find syna device");
            is_syna_dev_rmi = false;
            is_syna_dev_tcm = false;
            return false;
        }
    }  /* end onCheckDev() */

    private native String findSynaDevJNI();

    boolean onSetupDev(String node, String str_rmi_en, String str_rtcm_en) {

        if (node.length() == 0) {
            return onCheckDev();
        }

        str_syna_dev_node = node;

        try {
            is_syna_dev_rmi = Boolean.valueOf(str_rmi_en);
            is_syna_dev_tcm = Boolean.valueOf(str_rtcm_en);
        } catch (NumberFormatException e) {
            Log.e(SYNA_TAG, "MainActivity onStart() fail to convert to boolean");
            is_syna_dev_rmi = false;
            is_syna_dev_tcm = false;
        }

        boolean ret = setupSynaDevJNI(str_syna_dev_node, is_syna_dev_rmi, is_syna_dev_tcm);
        if (ret){
            Log.i(SYNA_TAG, "NativeWrapper onSetupDev() device node is installed, "
                    + str_syna_dev_node );
            Log.i(SYNA_TAG, "NativeWrapper onSetupDev() device type, rmi: "
                    + is_syna_dev_rmi + " , tcm: " + is_syna_dev_tcm );
        }
        else {
            Log.e(SYNA_TAG, "NativeWrapper onSetupDev() fail to find the device, "
                    + str_syna_dev_node);
            is_syna_dev_rmi = false;
            is_syna_dev_tcm = false;
        }

        return ret;
    }  /* end onSetupDev() */

    private native boolean setupSynaDevJNI(String node, boolean is_rmi, boolean is_tcm);

    String getDevNodeStr() {
        return str_syna_dev_node;
    }

    boolean isDevRMI() {
        return is_syna_dev_rmi;
    }
    boolean isDevRMITDDI() {
        String str = getDevId();
        return (str.startsWith("td") || str.startsWith("4"));
    }
    boolean isDevTCM() {
        return is_syna_dev_tcm;
    }
    boolean isDevTCMTDDI() {
        String str = getDevId();
        return (str.startsWith("td") || str.startsWith("4"));
    }

    /********************************************************
     * helper functions to open / close the syna device
     *
     * onOpenDev() open the device node
     * onCloseDev() close the device node
     ********************************************************/
    boolean onOpenDev() {

        boolean ret = openSynaDevJNI();
        if(!ret){
            Log.e(SYNA_TAG, "NativeWrapper onOpenDev() fail to open the device ");
            return false;
        }
        return true;
    } /* end onOpenDev() */

    boolean onCloseDev() {

        boolean ret = closeSynaDevJNI();
        if(!ret){
            Log.e(SYNA_TAG, "NativeWrapper onCloseDev() fail to close the device ");
            return false;
        }
        return true;
    } /* end onCloseDev() */

    boolean onDevPreparation(boolean do_no_sleep, boolean do_rezero) {

        boolean ret = doDevPreparationJNI(do_no_sleep, do_rezero);
        if(!ret){
            Log.e(SYNA_TAG, "NativeWrapper onDevPreparation() fail to do preparation");
            return false;
        }
        return true;
    }

    private native boolean openSynaDevJNI();
    private native boolean closeSynaDevJNI();
    private native boolean doDevPreparationJNI(boolean do_no_sleep, boolean do_rezero);


    /********************************************************
     * helper functions to perform the device identification
     * as well as the various device information
     *
     * onIdentifyDev() can perform the device identification process
     * if completed, the is_initialized will be true
     * otherwise, false to indicate the device info is not available
     ********************************************************/

    boolean onIdentifyDev(StringBuilder sb) {
        boolean ret;

        /* open syna device */
        ret = onOpenDev();
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onIdentifyDev() fail to open device" );
            is_initialized = false;
            return false;
        }

        /* call native to perform the identify and get all information back */
        String info = getIdentifyDataJNI();
        if (info == null) {
            Log.e(SYNA_TAG, "NativeWrapper onIdentifyDev() fail to do identify");
            is_initialized = false;
            onCloseDev();
            return false;
        }

        /* close syna device */
        ret = onCloseDev();
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onIdentifyDev() fail to close device" );
            is_initialized = false;
            return false;
        }

        /* attach the identification info */
        if (sb != null)
            sb.append(info);

        is_initialized = true;

        return true;
    } /* end onIdentifyDev() */

    private native String getIdentifyDataJNI();

    int getDevImageRow(boolean is_in_landscape) {
        if (!is_initialized)
            return 0;

        return getImageRowJNI(is_in_landscape);
    }
    private native int getImageRowJNI(boolean is_in_landscape);

    int getDevImageCol(boolean is_in_landscape) {
        if (!is_initialized)
            return 0;

        return getImageColJNI(is_in_landscape);
    }
    private native int getImageColJNI(boolean is_in_landscape);

    int getDevButtonNum() {
        if (!is_initialized)
            return 0;

        return getButtonCntJNI();
    }
    private native int getButtonCntJNI();

    int getDevFwId() {
        if (!is_initialized)
            return 0;

        return getFirmwareIdJNI();
    }
    private native int getFirmwareIdJNI();

    String getDevId() {
        if (!is_initialized)
            return null;

        return getProductIDJNI();
    }
    private native String getProductIDJNI();

    String getDevConfigID() {
        if (!is_initialized)
            return null;

        return getConfigIDJNI();
    }
    private native String getConfigIDJNI();

    boolean isDevImageHasHybrid() {
        return is_initialized && (getFeatureHasHybridJNI() == 1);
    }
    private native int getFeatureHasHybridJNI();

    int getDevForceChannels() {
        if (!is_initialized)
            return 0;

        return getForceElecsJNI();
    }
    private native int getForceElecsJNI();

    int getDevFwConfigSize() {
        if (!is_initialized)
            return 0;

        return getFwConfigSizeJNI();
    }
    private native int getFwConfigSizeJNI();

    boolean getDevFwConfig(byte[] buf) {
        return is_initialized && getFwConfigJNI(buf, buf.length);
    }
    private native boolean getFwConfigJNI(byte[] array, int size_of_array);

    /********************************************************
     * helper functions to retrieve the report image
     * for a successful report image reading, the steps are as follows
     *   - do identification
     *   - call onStartReport()
     *   - loop onRequestReport() to retrieve images
     *   - call onStopReport()
     *
     * onStartReport() and onStopReport() are used to enable and
     * disable the report image streaming.
     * onRequestReport() is a wrapper function to call the native
     * helper to retrieve a requested report image.
     ********************************************************/

    private byte getReportId(byte type) {
        byte code = (byte)0xff;

        if (!is_initialized) {
            Log.e(SYNA_TAG, "NativeWrapper getReportCode() device is not initialized yet");
            return code;
        }

        switch(type) {
            case SYNA_DELTA_REPORT_IMG:
                if (isDevRMI())
                    code = 0x02;
                else if (isDevTCM())
                    code = 0x12;
                break;
            case SYNA_RAW_REPORT_IMG:
                if (isDevRMI())
                    code = 0x03;
                else if (isDevTCM())
                    code = 0x13;
                break;
            default:
                Log.e(SYNA_TAG, "NativeWrapper onRequestReport() unknown report type, "
                        + type);
                break;
        }
        return code;
    } /* end getReportId() */

    final byte SYNA_DELTA_REPORT_IMG = 0x11;
    final byte SYNA_RAW_REPORT_IMG = 0x12;

    boolean onStartReport(byte report_type, boolean b_touch_en,
                          boolean b_do_nosleep, boolean b_do_rezero)
    {

        /* first of all, make sure the identification is completed */
        if (!is_initialized) {
            Log.e(SYNA_TAG, "NativeWrapper onStartReport() device is not initialized yet");
            return false;
        }

        /* open syna device */
        boolean ret = onOpenDev();
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onStartReport() fail to open device" );
            return false;
        }

        byte rt_id = getReportId(report_type);
        if (rt_id == (byte)0xff) {
            Log.e(SYNA_TAG, "NativeWrapper onStartReport() unknown report id");
            onCloseDev();
            return false;
        }

        Log.i(SYNA_TAG, "NativeWrapper onStartReport() report id = " + rt_id);

        /* call native to start the report image */
        ret = startReportJNI(rt_id, b_touch_en, b_do_nosleep, b_do_rezero);
        if(!ret){
            Log.e(SYNA_TAG, "NativeWrapper onStartReport() fail to enable the requested report");
            onCloseDev();
            return false;
        }

        return true;
    } /* end onStartReport() */

    boolean onStopReport(byte report_type) {

        byte rt_id = getReportId(report_type);
        if (rt_id == (byte)0xff) {
            Log.e(SYNA_TAG, "NativeWrapper onStopReport() unknown report id");
            return false;
        }

        Log.i(SYNA_TAG, "NativeWrapper onStopReport() report id = " + rt_id);

        /* to stop the report image */
        boolean ret = stopReportJNI(rt_id);
        if(!ret){
            Log.e(SYNA_TAG, "NativeWrapper onStopReport() fail to disable the requested report");
            return false;
        }

        /* close syna device */
        ret = onCloseDev();
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onStopReport() fail to close device" );
            return false;
        }

        return true;
    } /* end onStopReport() */

    boolean onRequestReport(byte type, int row, int col, int[] buf, int size_of_buf) {

        if(buf == null) {
            Log.e(SYNA_TAG, "NativeWrapper onRequestReport() data_buf is null.");
            return false;
        }

        if (!is_initialized) {
            Log.e(SYNA_TAG, "NativeWrapper onRequestReport() device is not initialized yet");
            return false;
        }

        byte rt_id = getReportId(type);
        if (rt_id == (byte)0xff) {
            Log.e(SYNA_TAG, "NativeWrapper onRequestReport() unknown report id");
            return false;
        }

        boolean ret = requestReportImageJNI(rt_id, row, col, buf, size_of_buf);
        if(!ret){
            Log.e(SYNA_TAG, "NativeWrapper onRequestReport() fail to request the report image "
                    + String.format(Locale.getDefault(),"rt 0x%02x", rt_id));
        }

        return ret;
    } /* end onRequestReport() */

    private native boolean startReportJNI(byte report_type, boolean touch_en,
                                          boolean do_no_sleep, boolean do_rezero);
    private native boolean stopReportJNI(byte report_type);
    private native boolean requestReportImageJNI(byte type, int row, int column,
                                                 int[] array, int size_of_array);

    /********************************************************
     * helper functions to perform the production tests
     * for a proper testing, the steps are as follows
     *   - do identification
     *   - call onStartProductionTest()
     *   - loop onRunProductionTest() to run the production test
     *   - call onStopProductionTest()
     *
     * onStartProductionTest() and onStopProductionTest() are used
     * to do the preparation before the testing
     * onRunProductionTest() is a wrapper function to call the native
     * helper to execute the selected test process
     ******************************************************/
    boolean onStartProductionTest(boolean b_do_nosleep, boolean b_do_rezero) {

        /* open syna device */
        boolean ret = onOpenDev();
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onStartProductionTest() fail to open device" );
            return false;
        }

        ret = onDevPreparation(b_do_nosleep, b_do_rezero);
        if(!ret){
            Log.e(SYNA_TAG, "NativeWrapper onStartProductionTest() fail to start testing");
            onCloseDev();
            return false;
        }
        return true;
    }
    boolean onStopProductionTest() {

        /* close syna device */
        boolean ret = onCloseDev();
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onStopProductionTest() fail to close device" );
            return false;
        }

        return true;
    }

    int onRunProductionTest(int test_id, int row, int col,
                            int[] limit_1, int size_limit_1, int[] limit_2, int size_limit_2,
                            int[] result, int size_result)
    {
        if (result == null) {
            Log.e(SYNA_TAG, "NativeWrapper onRunProductionTest() result is null.");
            return -1;
        }

        if (!is_initialized) {
            Log.e(SYNA_TAG, "NativeWrapper onRunProductionTest() device is not initialized yet");
            return -1;
        }

        return runProductionTestJNI(test_id, col, row, limit_1, size_limit_1, limit_2, size_limit_2,
                                    result, size_result);
    }
    private native int runProductionTestJNI(int id, int col, int row,
                                            int[] limit_min, int size_limit_min,
                                            int[] limit_max, int size_limit_max,
                                            int[] result, int size_result);


    int onRunProductionTestExHR(int row, int column, short[] ref, int limit_surface,
                                int limit_txroe, int limit_rxroe, int[] result, int size_result,
                                int[] result_txroe, int[] result_txroe_size,
                                int[] result_rxroe, int[] result_rxroe_size)
    {
        if (result == null) {
            Log.e(SYNA_TAG, "NativeWrapper onRunProductionTestExHR() result is null.");
            return -1;
        }

        if (!is_initialized) {
            Log.e(SYNA_TAG, "NativeWrapper onRunProductionTestExHR() device is not initialized yet");
            return -1;
        }

        return runProductionTestExHighResistanceJNI(column, row, ref, limit_surface, limit_txroe,
                limit_rxroe, result, size_result, result_txroe, result_txroe_size, result_rxroe,
                result_rxroe_size);
    }

    private native int runProductionTestExHighResistanceJNI(int col, int row, short[] ref_frame,
                                                            int limit_surface, int limit_txroe, int limit_rxroe,
                                                            int[] result, int size_result,
                                                            int[] result_txroe, int[] size_result_txroe,
                                                            int[] result_rxroe, int[] size_result_rxroe);


    int onRunProductionTestExTRx(int[] limit, int[] limit_ex_pin, int[] result_data,
                                 int[] result_data_ex_pin, int[] result_pin)
    {
        if ((result_data == null) || (result_pin == null) || (result_data_ex_pin == null)) {
            Log.e(SYNA_TAG, "NativeWrapper onRunProductionTestExTRx() result buffer is null.");
            return -1;
        }

        if (!is_initialized) {
            Log.e(SYNA_TAG, "NativeWrapper onRunProductionTestExTRx() device is not initialized yet");
            return -1;
        }

        return runProductionTestExTRxShortJNI(limit, limit.length, limit_ex_pin, limit_ex_pin.length,
                result_data, result_data.length, result_data_ex_pin, result_data_ex_pin.length,
                result_pin, result_pin.length);
    }

    private native int runProductionTestExTRxShortJNI(int[] limit, int size_limit, int[] limit_ex_pin,
                                                      int size_limit_ex_pin, int[] result_data, int size_result_data,
                                                      int[] result_data_ex_pin, int size_result_data_ex_pin,
                                                      int[] result_pin, int size_result_pin);

    /********************************************************
     * helper functions to retrieve all error messages which occurs
     * in the native layer
     *
     * getErrMessage() will return a set of error messages
     ********************************************************/
    String getErrMessage() {
        // get all error messages
        StringBuilder sb = new StringBuilder();
        int num = getNumErrMsgJNI();

        for (int i = 0; i < num; i++)
            sb.append(getErrMsgJNI(i));

        // clear all queued messages
        clearErrMsgJNI();

        return sb.toString();
    } /* end getErrMessage() */

    private native int getNumErrMsgJNI();
    private native String getErrMsgJNI(int index);
    private native boolean clearErrMsgJNI();


    /********************************************************
     * helper functions to send the command packet and
     * perform raw read/write operation in the native layer
     *
     * onRunRawCommands is the entry function for the dispatching
     ********************************************************/
    int onRunRawCommands(byte type, int command, byte[] buf, byte[] buf_resp) {

        Log.i(SYNA_TAG, "NativeWrapper onRunRawCommand()" );
        int retval;
        if (buf_resp == null) {
            retval = runRawCommandJNI(type, command, buf, buf.length, null, 0);
        }
        else {
            retval = runRawCommandJNI(type, command, buf, buf.length, buf_resp, buf_resp.length);
        }

        return retval;
    }
    private native int runRawCommandJNI(byte type, int cmd, byte[] in, int size_of_in,
                                        byte[] out, int size_of_out);


    /********************************************************
     * helper functions being used to monitor the touch event
     *
     * onTouchDown() return '>0' represents a touch down event occurs
     *               return '0'  means nothing happened, should keep polling
     *               return '-1' means something wrong
     *
     * onTouchUp()   return '1' represents a touch up event occurs
     *               return '0' means nothing happened, should keep polling
     *               return '-1' means something wrong
     ********************************************************/

    int onTouchDown(int num_finger_to_process, int[] pos_x, int[] pos_y)
    {
        if ((pos_x.length != num_finger_to_process) || (pos_y.length != num_finger_to_process)) {
            Log.e(SYNA_TAG, "NativeWrapper onTouchDown() size is mismatching, "
                    + "num_finger_to_process =  " + num_finger_to_process +
                    String.format(Locale.getDefault(),"(%d, %d)", pos_x.length, pos_y.length));
            return -1;
        }

        int retval = queryTouchResponseJNI(num_finger_to_process);
        if (retval < 0)
            return -1;

        int num_finger_down = 0;
        for (int i = 0; i < num_finger_to_process; i++) {
            if (touch_status[i] == TOUCH_DOWN) {
                pos_x[i] = touch_pos_x[i];
                pos_y[i] = touch_pos_y[i];
            }
            num_finger_down += touch_status[i];
        }

        return num_finger_down;
    }
    int onTouchUp(int num_finger_to_process) {

        int retval = 0;
        for (int i = 0; i < num_finger_to_process; i++) {
            retval += touch_status[i];
        }

        if (retval == 0)  /* return directly because there is no touched finger */
            return 0;

        retval = queryTouchResponseJNI(num_finger_to_process);
        if (retval < 0)
            return -1;

        retval = 1;
        for (int i = 0; i < num_finger_to_process; i++) {
            if (touch_status[i] != TOUCH_UP) {
                retval = 0;
            }
        }

        return retval;
    }

    private native int queryTouchResponseJNI(int num);

    private int[] touch_pos_x;
    private int[] touch_pos_y;
    private int[] touch_status;

    /********************************************************
     * callback functions
     * these functions are used to notify the application
     * finger event is detected in native layer
     ********************************************************/
    void callbackFingerDown(int fnger_idx, int pos_x, int pos_y) {

        Log.i(SYNA_TAG, "NativeWrapper callbackFingerDown() Finger " +
                String.format(Locale.getDefault(),"Idx = %d (%d, %d)", fnger_idx, pos_x, pos_y));

        if (fnger_idx < MAX_FINGER_NUMBER) {
            touch_pos_x[fnger_idx] = pos_x;
            touch_pos_y[fnger_idx] = pos_y;
            touch_status[fnger_idx] = TOUCH_DOWN;
        }
    }
    void callbackFingerUp(int fnger_idx) {
        Log.i(SYNA_TAG, "NativeWrapper callbackFingerUp() Finger " +
                String.format(Locale.getDefault(),"Idx = %d ", fnger_idx));

        if (fnger_idx < MAX_FINGER_NUMBER) {
            touch_status[fnger_idx] = TOUCH_UP;
        }

    }

    private final int MAX_FINGER_NUMBER = 10;
    private final int TOUCH_UP = 0;
    private final int TOUCH_DOWN = 1;


    /********************************************************
     * helper functions to check whether the channels assignment
     * is valid or not
     ********************************************************/
    boolean onCheckChannelsAssignment(int rxes_offset, int rxes_len, int txes_offset, int txes_len)
    {
        if (!is_initialized)
            return false;

        boolean ret;

        /* open syna device */
        ret = onOpenDev();
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onCheckChannelsAssignment() " +
                    "fail to open syna device" );
            return false;
        }
        /* call the native function to get channel mapping  */
        ret = getPinsMappingJNI(rxes_offset, rxes_len, txes_offset, txes_len);
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onCheckChannelsAssignment() " +
                    "fail to get the trx pin mapping" );

            onCloseDev();

            return false;
        }

        /* close syna device */
        ret = onCloseDev();
        if (!ret) {
            Log.e(SYNA_TAG, "NativeWrapper onCheckChannelsAssignment() " +
                    "fail to close syna device" );
            return false;
        }

        return true;
    }
    private native boolean getPinsMappingJNI(int rxes_offset, int rxes_len,
                                             int txes_offset, int txes_len);

}

