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
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.Vector;

abstract class LogFileManager {

    private final String SYNA_TAG = "syna-apk";

    /* reference for the native library */
    NativeWrapper native_lib;

    /* a buffer queue, which is used to record the data,
     * which will be written into the log file */
    Vector<String> vector_data_buf;

    /* save the starting time to create this log */
    private long ts_start;

    /* save the string of log file */
    String str_log_path;
    String str_log_file_name;
    String str_version_apk;

    final String FILE_CSV = ".csv";
    final String FILE_TXT = ".txt";

    /********************************************************
     * prepare a log file
     ********************************************************/
    boolean onPrepareLogFile(String file_name, String file_extensions) {

        SimpleDateFormat timeformat =
                new SimpleDateFormat("yyMMddhhmmss", Locale.getDefault());
        String timestamp = timeformat.format(new java.util.Date());

        str_log_file_name = str_log_path + "SynaAPK_" + file_name + "_" + timestamp;
        str_log_file_name += file_extensions;

        /* starting time */
        ts_start = System.currentTimeMillis();

        Log.i(SYNA_TAG, "LogFileManager onPrepareLogFile() done, " + str_log_file_name );

        return true;
    }
    /********************************************************
     * return the string of log file created
     ********************************************************/
    String onGetLogFileName() {
        return str_log_file_name;
    }
    /********************************************************
     * add error messages to the buffer queue
     * messages may be a direct string or queued in the native layer
     ********************************************************/
    void onAddErrorMessages(String msg, NativeWrapper native_lib) {
        /* get all error messages which occurs in the native layer */
        String str = msg + "\n" + native_lib.getErrMessage();
        vector_data_buf.add(str);
    }

    void onAddErrorMessages(String msg) {
        /* add error message to the queue directly */
        vector_data_buf.add(msg + "\n");
    }

    /********************************************************
     * get the timestamp in millisecond
     * timestamp = current ts - ts_start
     ********************************************************/
    String onGetTimeStampMs() {
        long diff = System.currentTimeMillis() - ts_start;
        double time = (diff/1000) + (1.0*(diff%1000)/1000);

        return String.format(Locale.getDefault(),"%.3f", time);
    }

    /********************************************************
     * helper function to ensure the input path is accessible
     ********************************************************/
    boolean onCheckLogFilePath(String path) {
        /* check the input string */
        /* replace all blanking in the string */
        path = path.replaceAll("\\s+", "");
        /* handle the empty string */
        if(path.length() < 1){
            Log.e(SYNA_TAG, "LogFileManager onCheckLogFilePath() path is empty");
            return false;
        }
        /* if the first character is not '/', insert it */
        if(!path.startsWith("/")){
            path = "/" + path;
        }
        /* if the last character is not '/', append it */
        if(!path.endsWith("/")){
            path = path + "/";
        }
        /* ensure the input path is writable */
        File file = new File(path);
        if (!file.canWrite()) {
            Log.e(SYNA_TAG, "LogFileManager onCheckLogFilePath() path is not writable, " + path);
            return false;
        }
        /* record the valid path */
        str_log_path = path;
        Log.i(SYNA_TAG, "LogFileManager onCheckLogFilePath() done, " + str_log_path );
        return true;
    }
    /********************************************************
     * helper function to return the valid log path
     ********************************************************/
    String onGetValidLogFilePath() {
        return str_log_path;
    }

    /********************************************************
     * create the log file
     ********************************************************/
    abstract boolean onCreateLogFile();

    /********************************************************
     * add the data to the buffer queue
     ********************************************************/
    abstract boolean onAddLogData(String str);
    abstract boolean onAddLogData(int[] data, String str);
    abstract boolean onAddLogData(double[] data, String str);
}

