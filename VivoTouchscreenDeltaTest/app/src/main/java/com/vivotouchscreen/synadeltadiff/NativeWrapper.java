package com.vivotouchscreen.synadeltadiff;

import android.util.Log;

class NativeWrapper {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native_syna");
    }

    /**
     * Global variables
     */
    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * constructor
     ********************************************************/
    NativeWrapper() {

    }

    /********************************************************
     * a method to find the Synaptics device node
     * return the path of device node if existed
     ********************************************************/
    boolean onCheckDev() {
        return checkDevJNI();
    }
    boolean isRMIDev() {
        return isRMIDevJNI();
    }
    boolean isTCMDev() {
        return isTCMDevJNI();
    }
    /**
     * a native method to check whether the /dev/rmi0 or /dev/tcm0 exists or not.
     * return true to find the /dev/rmi or /dev/tcm0
     */
    private native boolean checkDevJNI();
    private native boolean isRMIDevJNI();
    private native boolean isTCMDevJNI();

    /********************************************************
     * a method to provide the device ID
     ********************************************************/
    String getDevID() {
        return getDevIDJNI();
    }
    /**
     * a native method to get the device id parsed from the pdt
     * return a string of device id
     */
    private native String getDevIDJNI();

    /********************************************************
     * a method to provide the firmware id ID
     ********************************************************/
    String getFirmwareID() {
        return getBuildIDJNI();
    }
    /**
     * a native method to get the firmware id parsed from the pdt
     * return a string of firmware id
     */
    private native String getBuildIDJNI();


    /********************************************************
     * a method to save the report type
     ********************************************************/
    void setReportType(int type) {
        setReportTypeJNI(type);
    }
    /**
     * a native method to save the report type configured by user settings
     */
    private native void setReportTypeJNI(int value);

    /********************************************************
     * a method to save delta threshold settings
     ********************************************************/
    void setThreshold(int threshold) {
        setThresholdJNI(threshold);
    }
    /**
     * a native method to save delta threshold configured by user settings
     */
    private native void setThresholdJNI(int value);

    /********************************************************
     * a method to configure the output file
     ********************************************************/
    boolean setOutputFile(String path) {
        return setOutputFileJNI(path);
    }
    /**
     * a native method to set the path of log file
     */
    private native boolean setOutputFileJNI(String path);

    /********************************************************
     * a method to get the supported freq gears
     ********************************************************/
    String getGearInfo() {
        return getGearInfoJNI();
    }
    /**
     * a native method to get the information of available gears
     * return a string of all available gear
     */
    private native String getGearInfoJNI();

    /********************************************************
     * a method to get the threshold
     ********************************************************/
    int getThreshold() {
        return getFingerThresholdJNI();
    }
    /**
     * a native method to get the default finger delta defined in RMI
     * return the delta of finger object
     */
    private native int getFingerThresholdJNI();

    /********************************************************
     * a method to do all preparations before the testing
     ********************************************************/
    boolean onOpenTest(int total_frames, int num_gears) {
        return openTestJNI(total_frames, num_gears);
    }
    /**
     * a native method to do all preparations before the testing
     * such as open the /dev/rmi, and write the header to the log file
     * return true if all preparations are ready
     */
    private native boolean openTestJNI(int total_frames, int num_gears);


    /********************************************************
     * a method to finish the testing
     ********************************************************/
    void onCloseTest(int ng_points) {
        closeTestJNI(ng_points);
    }
    /**
     * a native method to finish the testing
     * return true to close the testing, and then the lof file is created.
     */
    private native boolean closeTestJNI(int ng_points);

    /********************************************************
     * a method to perform actual noise testing
     ********************************************************/
    boolean onNoiseTest(int frame_id, int gear_idx) {
        return doTestJNI(frame_id, gear_idx);
    }
    /**
     * a native method to perform the noise testing
     * return true to indicate the testing round is pass
     */
    private native boolean doTestJNI(int frame_id, int gear_idx);


    /********************************************************
     * a method to perform actual noise testing
     ********************************************************/
    boolean setGear(int gear_id) {
        return setGearJNI(gear_id);
    }
    /**
     * a native method to enable the specified fear
     */
    private native boolean setGearJNI(int gear_id);

    /********************************************************
     * a method to reate the specific folder for vivo using
     ********************************************************/
    boolean onCreateVIVOFolder() {
        return createVIVOFolderJNI();
    }
    /**
     * a native method to create the specific folder for vivo using
     */
    private native boolean createVIVOFolderJNI();

    /********************************************************
     * a method to retrieve the layout information of report image
     ********************************************************/
    boolean retrieveImageLayout() {

        if (isTCMDev()) {
            int row = getTCMRowInfoJNI();
            int col = getTCMColInfoJNI();

            if ((row < 0) || (col < 0))
                return false;

            /* tcm is stored in row-major */
            n_num_in_major_order = row;
            n_num_in_minor_order = col;

            return true;
        }
        else if (isRMIDevJNI()) {
            int tx = getRMITxInfoJNI();
            int rx = getRMIRxInfoJNI();

            if ((tx < 0) || (rx < 0))
                return false;

            /* tcm is stored in rx-major */
            n_num_in_major_order = rx;
            n_num_in_minor_order = tx;

            return true;
        }

        Log.i(SYNA_TAG, "retrieveImageLayout(): size = (" + n_num_in_major_order + "," +
                n_num_in_minor_order + ")");

        return false;
    }
    /**
     * a native method to get the trx/row-col info
     */
    private native int getRMITxInfoJNI();
    private native int getRMIRxInfoJNI();
    private native int getTCMRowInfoJNI();
    private native int getTCMColInfoJNI();

    /**
     * the value of NumCellinColumn is the larger value
     * the smaller one is getNumCellinRow
     */
    int getNumCellinRow() {
        return ( n_num_in_major_order < n_num_in_minor_order)?
                n_num_in_major_order : n_num_in_minor_order;
    }

    int getNumCellinColumn() {
        return ( n_num_in_major_order > n_num_in_minor_order)?
                n_num_in_major_order : n_num_in_minor_order;
    }

    /********************************************************
     * a method to retrieve the a report image
     ********************************************************/
    private int n_num_in_major_order;
    private int n_num_in_minor_order;

    final int IMG_DELTA = 1;
    final int IMG_RAW = 2;

    boolean prepareReportFrame(int type) {
        boolean is_delta = false;
        boolean is_raw = false;

        if (type == IMG_DELTA) is_delta = true;
        else if (type == IMG_RAW) is_raw = true;

        return startImgReportJNI(is_delta, is_raw);
    }

    boolean completeReportFrame(int type) {
        boolean is_delta = false;
        boolean is_raw = false;

        if (type == IMG_DELTA) is_delta = true;
        else if (type == IMG_RAW) is_raw = true;

        return stopImgReportJNI(is_delta, is_raw);
    }

    boolean requestReportFrame(int type, short[] data) {

        short[] report_img = new short[(n_num_in_major_order * n_num_in_minor_order)];

        boolean is_delta = false;
        boolean is_raw = false;

        if (type == IMG_DELTA) is_delta = true;
        else if (type == IMG_RAW) is_raw = true;

        boolean ret = requestImgReportJNI(is_delta, is_raw, report_img);
        if (!ret) {
            Log.i(SYNA_TAG, "requestReportFrame(): fail to request a report image");
            return false;
        }

        // re-ordering
        int n = getNumCellinColumn();
        int i, j;
        if (n_num_in_major_order == n) {
            for (i = 0; i < n_num_in_minor_order; i++) {
                for (j = 0; j < n_num_in_major_order; j++) {
                    data[i * n + j] = report_img[i * n_num_in_major_order + j];
                }
            }
        }
        else {
            for (i = 0; i < n_num_in_minor_order; i++) {
                for (j = 0; j < n_num_in_major_order; j++) {
                    data[j * n + i] = report_img[i * n_num_in_major_order + j];
                }
            }
        }

        return true;
    }

    private native boolean startImgReportJNI(boolean is_delta, boolean is_raw);
    private native boolean stopImgReportJNI(boolean is_delta, boolean is_raw);
    private native boolean requestImgReportJNI(boolean is_delta, boolean is_raw, short[] report_img);
}
