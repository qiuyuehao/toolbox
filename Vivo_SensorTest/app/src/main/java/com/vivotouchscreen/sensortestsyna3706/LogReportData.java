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

class LogReportData extends LogFileManager {

    private final String SYNA_TAG = "syna-apk";

    private byte report_type;

    /********************************************************
     * constructor
     ********************************************************/
    LogReportData(NativeWrapper lib, byte rt_type ,String version) {
        this.str_log_path = null;
        this.str_log_file_name = null;

        this.native_lib = lib;

        this.vector_data_buf = new Vector<>();

        this.str_version_apk = version;

        report_type = rt_type;
    }

    /********************************************************
     * generate the header string for report image log using
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

        if (report_type == this.native_lib.SYNA_DELTA_REPORT_IMG) {
            if (this.native_lib.isDevRMI()) {
                header +=  "Report Type               : Delta Image (RT2)\n";
            }
            else if (this.native_lib.isDevTCM()) {
                header +=  "Report Type               : Delta Image (RID 0x12)\n";
            }
        }
        else if (report_type == this.native_lib.SYNA_RAW_REPORT_IMG) {
            if (this.native_lib.isDevRMI()) {
                header +=  "Report Type               : Raw Image (RT3)\n";
            }
            else if (this.native_lib.isDevTCM()) {
                header +=  "Report Type               : Raw Image (RID 0x13)\n";
            }
        }
        else
            header +=  "Report Type               : unknown report type " + report_type + "\n";

        header +=  "Total Frames Collected    : " + this.vector_data_buf.size() + " \n";
        header +=  "Image Rows                : " + this.native_lib.getDevImageRow(false) + " \n";
        header +=  "Image Columns             : " + this.native_lib.getDevImageCol(false) + " \n";
        header +=  "Buttons                   : " + this.native_lib.getDevButtonNum() + " \n";
        if (this.native_lib.isDevImageHasHybrid()) {
            header +=  "Number of Force Electrode : " + this.native_lib.getDevForceChannels() + " \n";
            header +=  "Has Hybrid                : true \n";
        }

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

            Log.i(SYNA_TAG, "LogReportData onCreateLogFile() log file is saved, "
                    + this.str_log_file_name);
        } catch(IOException e) {
            Log.e(SYNA_TAG, "LogReportData onCreateLogFile() fail to create file,"
                    + this.str_log_file_name);
            return false;
        }

        return true;
    }

    /********************************************************
     * add one image data to the buffer queue
     * place the data in the landscape layout always
     ********************************************************/
    boolean onAddLogData(int[] data, String str_frame_idx) {

        if(data == null) {
            Log.e(SYNA_TAG, "LogReportData onAddLogData() data_image is null.");
            return false;
        }

        int image_row = native_lib.getDevImageRow(true);
        int image_col = native_lib.getDevImageCol(true);
        boolean has_hybrid = native_lib.isDevImageHasHybrid();

        StringBuilder sb = new StringBuilder();
        int max_val, min_val;
        int d;
        int i, j, offset;

        /* attach the timestamp and the frame index */
        sb.append(String.format(Locale.getDefault(), "< %s > %s\n",
                onGetTimeStampMs(), str_frame_idx));

        /* write the report data */
        min_val = max_val = data[0];
        for(i = 0; i < image_col; i++) {
            if (i == 0) {
                sb.append("         ");
                for (j = 0 ; j < image_row; j ++)
                    sb.append(String.format(Locale.getDefault(), "[%02d]   ", j));
                sb.append("\n");
            }
            sb.append(String.format(Locale.getDefault(), "[%02d]:  ", i));
            for(j = 0; j < image_row; j++) {
                offset = i*image_row + j;

                d = data[offset];
                sb.append(String.format(Locale.getDefault(), "% 6d ", d));

                max_val = (short)Math.max(max_val, d);
                min_val = (short)Math.min(min_val, d);
            }
            sb.append("\n");
        }
        sb.append(String.format(Locale.getDefault(), "max. = %d, min. = %d\n",
                max_val, min_val));

        /* handle the hybrid data */
        int line = 0;
        if (has_hybrid) {
            sb.append("\n");
            offset = image_col * image_row;
            for (i = 0; i < (data.length - offset); i++) {
                if (i == 0) {
                    sb.append("Profile data\n");
                    sb.append("[00]:  ");
                }
                d = data[offset + i];
                sb.append(String.format(Locale.getDefault(), "% 6d ", d));
                if ((i != 0) && (((i+1)%image_row) == 0)) {
                    line += 1;
                    sb.append(String.format(Locale.getDefault(), "\n[%02d]:  ", line));
                }
            }
            sb.append("\n");
        }

        /* push the string to the queue */
        vector_data_buf.add(sb.toString());

        return true;
    } /* end onAddLogData() */

    boolean onAddLogData(double[] data, String description) {
        return false;
    }
    boolean onAddLogData(String str) {
        return false;
    }

}

