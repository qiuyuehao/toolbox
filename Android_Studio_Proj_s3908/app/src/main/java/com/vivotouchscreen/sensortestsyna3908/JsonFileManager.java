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
import org.json.*;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

class JsonFileManager {

    private final String SYNA_TAG = "syna-apk";

    private JSONObject json_file;

    /********************************************************
     * constructor
     ********************************************************/
    JsonFileManager() {
        json_file = null;
    }

    /********************************************************
     * helper function to ensure appointed .json file is available
     *******************************************************/
    boolean onCheckJsonFile(String str_file) {
        /* replace all blanking in the string */
        str_file = str_file.replaceAll("\\s+", "");
        /* handle the empty string */
        if(str_file.length() < 1){
            return false;
        }
        /* if the last character is not '.json' */
        if(!str_file.endsWith(".json")) {
            Log.e(SYNA_TAG, "JsonFileManager: onCheckJsonFile() file extension is not .json");
            return false;
        }
        File file = new File(str_file);
        if (!file.exists()) {
            Log.e(SYNA_TAG, "JsonFileManager: onCheckJsonFile() file not exist, " + str_file );
            return false;
        }

        StringBuilder sb = new StringBuilder();
        if (!onReadFileContent(str_file, sb)) {
            Log.e(SYNA_TAG, "JsonFileManager: onCheckJsonFile() fail to open file, " + str_file );
            return false;
        }

        try {
            json_file = new JSONObject(sb.toString());

        } catch(Exception e) {
            Log.e(SYNA_TAG, "JsonFileManager: onCheckJsonFile: " +
                    "JSONObject Exception, " + str_file);
            return false;
        }

        Log.i(SYNA_TAG, "JsonFileManager: onCheckJsonFile() completed, " + str_file );
        return true;
    } /* end onCheckJsonFile() */

    /********************************************************
     * helper function to read the file content and save
     * into the given string builder
     *
     * return false if it is failed; otherwise, return true
     *******************************************************/
    private boolean onReadFileContent(String str_file, StringBuilder sb) {

        if (sb.length() != 0) {
            sb.setLength(0);
        }

        try {
            FileReader file_reader = new FileReader(str_file);
            BufferedReader buffered_reader = new BufferedReader(file_reader);
            String str;
            while ((str = buffered_reader.readLine()) != null){
                sb.append(str);
            }

        } catch(Exception e) {
            Log.e(SYNA_TAG, "JsonFileManager: onReadFileContent: " +
                    "FileReader Exception, " + str_file);
            return false;
        }

        return true;
    }

    /********************************************************
     * helper function to query the particular value defined
     * in the json file
     *
     * return -1 if it is failed; otherwise, return the data
     *******************************************************/
    int onGetJsonData(String key) {

        if (json_file == null) {
            Log.e(SYNA_TAG, "JsonFileManager: onGetJsonData: json_file is null");
            return -1;
        }

        Object value;

        try {
            value = json_file.get(key);

            Log.i(SYNA_TAG, "JsonFileManager: onGetJsonData: " +
                    "key = " + key +", value = " + value);

        } catch(Exception e) {
            Log.e(SYNA_TAG, "JsonFileManager: onGetJsonData: " +
                    "JSONObject Exception, key = " + key );
            return -1;
        }
        return (int)value;
    }

    int onGetJsonData(String obj_outer, String obj_inner, String key) {

        if (json_file == null) {
            Log.e(SYNA_TAG, "JsonFileManager: onGetJsonData: json_file is null");
            return -1;
        }

        Object value;

        try {
            value = json_file.getJSONObject(obj_outer)
                                .getJSONObject(obj_inner)
                                .get(key);

            Log.i(SYNA_TAG, "JsonFileManager: onGetJsonData: " +
                    "obj = " + obj_outer + ", and "+ obj_inner + ", key = "
                    + key + ", value = " + value);

        } catch(Exception e) {
            Log.e(SYNA_TAG, "JsonFileManager: onGetJsonData: " +
                    "JSONObject Exception, obj = " + obj_outer + ", and "+ obj_inner + ", key = "
                    + key);
            return -1;
        }
        return (int)value;
    }
}