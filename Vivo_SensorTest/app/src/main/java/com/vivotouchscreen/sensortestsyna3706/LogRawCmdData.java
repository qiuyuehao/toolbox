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

class LogRawCmdData extends LogFileManager {

    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * constructor
     ********************************************************/
    LogRawCmdData(NativeWrapper lib, String version) {
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
        header +=  "Total Commands Logged     : " + vector_data_buf.size()/2 + " \n";

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
            writer.newLine();

            /* writing all stored data into the log file */
            for (int i = 0; i < vector_data_buf.size(); i+= 2) {
                writer.write("commands " + ((i/2) + 1) + "\n" );
                writer.write(vector_data_buf.elementAt(i));
                writer.newLine();
                writer.write(vector_data_buf.elementAt(i+1));
                writer.newLine();
                writer.newLine();
            }

            /* close the file */
            writer.close();
            /* clear the buffer */
            vector_data_buf.clear();

            Log.i(SYNA_TAG, "LogRawCmdData onCreateLogFile() log file is saved, "
                    + this.str_log_file_name);
        } catch(IOException e) {
            Log.e(SYNA_TAG, "LogRawCmdData onCreateLogFile() fail to create file,"
                    + this.str_log_file_name);
            return false;
        }

        return true;
    }

    /********************************************************
     * add one image data to the buffer queue
     ********************************************************/
    boolean onAddLogData(String str) {
        if (str == null) {
            Log.e(SYNA_TAG, "LogRawCmdData onAddLogData() str is null.");
            return false;
        }

        str = str.replaceAll("\n", "");
        if (str.length() == 0) {
            vector_data_buf.add(str);
        }
        else {
            String t = String.format(Locale.getDefault(), "[ %s ] ", onGetTimeStampMs());
            StringBuilder spaces = new StringBuilder();
            for(int i = 0; i < t.length(); i++)
                spaces.append(" ");

            str = str.replaceAll("-", "\n"+spaces.toString());
            vector_data_buf.add(t + str);
        }

        return true;
    }

    boolean onAddLogData(int[] data, String str_test_result) {
        return false;
    }
    boolean onAddLogData(double[] data, String description) {
        return false;
    }
}

