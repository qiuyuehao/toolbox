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
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.Vector;

class LogSNRData extends LogFileManager {

    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * constructor
     ********************************************************/
    LogSNRData(NativeWrapper lib, String version) {
        this.str_log_path = null;
        this.str_log_file_name = null;

        this.native_lib = lib;

        this.vector_data_buf = new Vector<>();

        this.str_version_apk = version;
    }

    /********************************************************
     * generate the header string for snr image log using
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

        header +=  "Image for Calculation     : delta image\n";

        return header;
    }

    /********************************************************
     * create the log file
     ********************************************************/
    boolean onCreateLogFile() {

        try {
            FileWriter file = new FileWriter(this.str_log_file_name, false);
            BufferedWriter writer = new BufferedWriter(file);

            /* writing header */
            writer.write(writeHeader());
            writer.newLine();

            /* writing all stored data into the log file */
            for (int i = 0; i < this.vector_data_buf.size(); i++) {
                writer.write(this.vector_data_buf.get(i));
                writer.newLine();
            }

            /* close the file */
            writer.close();
            /* clear the buffer */
            this.vector_data_buf.clear();

            Log.i(SYNA_TAG, "LogSNRData onCreateLogFile() log file is saved, "
                    + this.str_log_file_name);
        } catch(IOException e) {
            Log.e(SYNA_TAG, "LogSNRData onCreateLogFile() fail to create file,"
                    + this.str_log_file_name);
            return false;
        }

        return true;
    }

    /********************************************************
     * add one image data to the buffer queue
     ********************************************************/
    boolean onAddLogData(int[] data, String description) {

        if(data == null) {
            Log.e(SYNA_TAG, "LogSNRData onAddLogData() data_image is null.");
            return false;
        }

        int image_row = native_lib.getDevImageRow(true);
        int image_col = native_lib.getDevImageCol(true);

        StringBuilder sb = new StringBuilder();
        int i, j, offset;

        /* attach the data description */
        sb.append(description).append("\n");

        /* write the image data */
        for(i = 0; i < image_col; i++) {
            if (i == 0) {
                sb.append("          ");
                for (j = 0 ; j < image_row; j ++)
                    sb.append(String.format(Locale.getDefault(), "[%02d]     ", j));
                sb.append("\n");
            }
            sb.append(String.format(Locale.getDefault(), "[%02d]: ", i));
            for(j = 0; j < image_row; j++) {
                offset = i*image_row + j;

                sb.append(String.format(Locale.getDefault(), "% 8d ", data[offset]));
            }
            sb.append("\n");
        }

        /* push the string to the queue */
        this.vector_data_buf.add(sb.toString());

        return true;
    }

    boolean onAddLogData(double[] data, String description) {

        if(data == null) {
            Log.e(SYNA_TAG, "LogSNRData onAddLogData() data_image is null.");
            return false;
        }

        int image_row = native_lib.getDevImageRow(true);
        int image_col = native_lib.getDevImageCol(true);

        StringBuilder sb = new StringBuilder();
        int i, j, offset;

        /* attach the data description */
        sb.append(description).append("\n");

        /* write the image data */
        for(i = 0; i < image_col; i++) {
            if (i == 0) {
                sb.append("          ");
                for (j = 0 ; j < image_row; j ++)
                    sb.append(String.format(Locale.getDefault(), "[%02d]     ", j));
                sb.append("\n");
            }
            sb.append(String.format(Locale.getDefault(), "[%02d]: ", i));
            for(j = 0; j < image_row; j++) {
                offset = i*image_row + j;

                sb.append(String.format(Locale.getDefault(), "% 8.2f ", data[offset]));
            }
            sb.append("\n");
        }

        /* push the string to the queue */
        this.vector_data_buf.add(sb.toString());

        return true;
    }

    boolean onAddLogData(String str) {

        /* push the string to the queue */
        this.vector_data_buf.add(str);

        return true;
    }
}

