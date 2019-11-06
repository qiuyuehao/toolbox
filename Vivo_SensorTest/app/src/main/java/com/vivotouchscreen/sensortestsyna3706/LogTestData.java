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
package com.vivotouchscreen.sensortestsyna3706;

import android.util.Log;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.Vector;

class LogTestData extends LogFileManager {

    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * constructor
     ********************************************************/
    LogTestData(NativeWrapper lib, String version) {
        this.str_log_path = null;
        this.str_log_file_name = null;

        this.native_lib = lib;

        this.vector_data_buf = new Vector<>();

        this.str_version_apk = version;
    }

    /********************************************************
     * generate the header string for test log using
     ********************************************************/
    private String writeHeader() {
        SimpleDateFormat timeformat =
                new SimpleDateFormat("yyyy-MM-dd hh:mm:ss", Locale.getDefault());
        String header = "Synaptics APK ( version " + this.str_version_apk + " ) " +
                "     recorded at " + timeformat.format(new java.util.Date()) + "\n";

        if (this.native_lib.isDevRMI()) {
            header += "Interface                 : RMI4 ( " + this.native_lib.getDevNodeStr() + " )\n";
        }
        else if (this.native_lib.isDevTCM()) {
            header += "Interface                 : TouchComm ( " + this.native_lib.getDevNodeStr() + " )\n";
        }

        header +=  "Device ID                 : " + this.native_lib.getDevId() + " \n";
        header +=  "Firmware Packrat ID       : " + this.native_lib.getDevFwId() + " \n";
        header +=  "Config ID                 : " + this.native_lib.getDevConfigID() + " \n";
        header +=  "Image Rows                : " + this.native_lib.getDevImageRow(false) + " \n";
        header +=  "Image Columns             : " + this.native_lib.getDevImageCol(false) + " \n";
        header +=  "Buttons                   : " + this.native_lib.getDevButtonNum() + " \n";
        if (this.native_lib.isDevImageHasHybrid()) {
            header +=  "Number of Force Electrode : " + this.native_lib.getDevForceChannels() + " \n";
            header +=  "Has Hybrid                : true \n";
        }
        header +=  "Number of Items Tested    : " + vector_data_buf.size() + " \n";

        return header;
    }

    /********************************************************
     * create the log file
     ********************************************************/
    boolean onCreateLogFile() {

        try {
            FileWriter file = new FileWriter(str_log_file_name, false);
            BufferedWriter writer = new BufferedWriter(file);

            /* writing header */
            writer.write(writeHeader());
            writer.newLine();

            /* writing all stored data into the log file */
            for (int i = 0; i < vector_data_buf.size(); i++) {
                writer.write(vector_data_buf.get(i));
                writer.newLine();
            }

            /* close the file */
            writer.close();
            /* clear the buffer */
            vector_data_buf.clear();

            Log.i(SYNA_TAG, "LogTestData onCreateLogFile() log file is saved, "
                    + this.str_log_file_name);
        } catch(IOException e) {
            Log.e(SYNA_TAG, "LogTestData onCreateLogFile() fail to create file,"
                    + this.str_log_file_name);
            return false;
        }

        return true;
    }

    /********************************************************
     * add one image data to the buffer queue
     ********************************************************/
    boolean onAddLogData(String str_test_result) {
        if (str_test_result == null) {
            Log.e(SYNA_TAG, "LogTestData onAddLogData() test_str is null.");
            return false;
        }

        /* combine the timestamp and the str_test_result */
        String sb = String.format(Locale.getDefault(), "< %s > \n", onGetTimeStampMs()) +
                str_test_result;

        /* push the string to the queue */
        vector_data_buf.add(sb);

        return true;
    }

    boolean onAddLogData(int[] data, String str_test_result) {
        return false;
    }
    boolean onAddLogData(double[] data, String description) {
        return false;
    }
}

