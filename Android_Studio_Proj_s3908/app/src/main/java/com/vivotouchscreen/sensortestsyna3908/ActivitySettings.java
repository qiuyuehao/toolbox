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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;

public class ActivitySettings extends Activity {

    private final String SYNA_TAG = "syna-apk";


    /********************************************************
     * variables for device nodes
     ********************************************************/
    private String str_dev_node;
    private String str_dev_rmi_en;
    private String str_dev_tcm_en;

    /********************************************************
     * variables for log file saving and external test_cfg file
     ********************************************************/
    private boolean b_log_save;
    private String str_log_path;

    /********************************************************
     * object for native function access
     ********************************************************/
    NativeWrapper native_lib;

    /********************************************************
     * Handler for Log File Access
     ********************************************************/
    LogFileManager file_manager_lib;


    /********************************************************
     * UI Components
     ********************************************************/
    private TextView text_dev_err;
    private EditText edit_dev_rmi;
    private CheckBox cbtn_dev_rmi;
    private CheckBox cbtn_dev_tcm;
    private EditText edit_dev_tcm;

    /********************************************************
     * Called by MainActivity when Button is pressed
     ********************************************************/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* to hide the navigation bar */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                            WindowManager.LayoutParams.FLAG_FULLSCREEN);
        /* set view */
        setContentView(R.layout.page_settings);

        Log.i(SYNA_TAG, "ActivitySetting onCreate() + " );

        Intent intent = this.getIntent();
        str_dev_node = intent.getStringExtra(Common.STR_DEV_NODE);
        str_dev_rmi_en = intent.getStringExtra(Common.STR_DEV_RMI_EN);
        str_dev_tcm_en = intent.getStringExtra(Common.STR_DEV_TCM_EN);
        Log.i(SYNA_TAG, "ActivitySetting onCreate() dev_node: " +
                str_dev_node + " (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");
        b_log_save = intent.getBooleanExtra("CFG_LOG_SAVE", true);
        str_log_path = intent.getStringExtra(Common.STR_CFG_LOG_PATH);
        Log.i(SYNA_TAG, "ActivitySetting onCreate() log saving ( " +
                b_log_save + " ), log path: " + str_log_path );


        /* get assignment from MainActivity */
        native_lib = MainActivity.native_lib;
        file_manager_lib = MainActivity.file_manager_lib;

        /* UI components - button exit */
        Button btn_ok = findViewById(R.id.btn_ok);
        btn_ok.setOnClickListener(_button_listener_exit);

        /* UI components - radio buttons and edit texts, device node */
        cbtn_dev_rmi = findViewById(R.id.setting_cbox_dev_rmi);
        cbtn_dev_rmi.setEnabled(false);
        cbtn_dev_rmi.setOnCheckedChangeListener(_checkbox_listener_dev_rmi);

        edit_dev_rmi = findViewById(R.id.setting_edit_dev_rmi);
        edit_dev_rmi.setEnabled(false);
        edit_dev_rmi.setOnEditorActionListener(_edit_listener_dev_rmi);

        cbtn_dev_tcm = findViewById(R.id.setting_cbox_dev_tcm);
        cbtn_dev_tcm.setEnabled(false);
        cbtn_dev_tcm.setOnCheckedChangeListener(_checkbox_listener_dev_tcm);

        edit_dev_tcm = findViewById(R.id.setting_edit_dev_tcm);
        edit_dev_tcm.setEnabled(false);
        edit_dev_tcm.setOnEditorActionListener(_edit_listener_dev_tcm);

        text_dev_err = findViewById(R.id.setting_dev_err);
        text_dev_err.setVisibility(View.INVISIBLE);

        if (native_lib.isDevRMI()) {
            cbtn_dev_rmi.setChecked(true);
            cbtn_dev_rmi.setEnabled(true);
            edit_dev_rmi.setText(native_lib.getDevNodeStr());
            edit_dev_rmi.setEnabled(true);

            cbtn_dev_tcm.setChecked(false);
            edit_dev_tcm.setText("");
        }
        else if (native_lib.isDevTCM()) {
            cbtn_dev_rmi.setChecked(false);
            edit_dev_rmi.setText("");

            cbtn_dev_tcm.setChecked(true);
            cbtn_dev_tcm.setEnabled(true);
            edit_dev_tcm.setText(native_lib.getDevNodeStr());
            edit_dev_tcm.setEnabled(true);
        }
        else {
            cbtn_dev_rmi.setChecked(false);
            edit_dev_rmi.setText(R.string.syna_dev_rmi);
            cbtn_dev_rmi.setEnabled(true);

            cbtn_dev_tcm.setChecked(false);
            edit_dev_tcm.setText(R.string.syna_dev_tcm);
            cbtn_dev_tcm.setEnabled(true);

            text_dev_err.setText(R.string.syna_dev_unknown);
            text_dev_err.setTextColor(Color.RED);
            text_dev_err.setVisibility(View.VISIBLE);
        }

        /* UI components - edit text, path of log file */
        EditText edit_path = findViewById(R.id.setting_edit_log_path);
        edit_path.setOnEditorActionListener(_edit_listener_logpath);
        TextView text_log_msg = findViewById(R.id.setting_cbox_log_msg);
        if(b_log_save) {
            updateLogFilePath(str_log_path);   /* update the file path */
            edit_path.setEnabled(true);
            text_log_msg.setVisibility(View.VISIBLE);
        }
        else {
            edit_path.setEnabled(false);
            text_log_msg.setVisibility(View.INVISIBLE);
        }
        /* UI components - checkbox, log saving */
        CheckBox cbtn_log_saving = findViewById(R.id.setting_cbox_log_save);
        cbtn_log_saving.setOnCheckedChangeListener(_checkbox_listener_savelog);
        cbtn_log_saving.setChecked(b_log_save);

    } /* end onCreate() */

    /********************************************************
     * implementation if 'EXIT' button is pressed
     *
     * if the prperty of log saving is enabled,
     * check the log path whether it is valid or not in case the
     * user doesn't enter the DONE key
     ********************************************************/
    private View.OnClickListener _button_listener_exit = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            /* save the latest input in case user didn't enter DONE */
            EditText edit_path = findViewById(R.id.setting_edit_log_path);
            CheckBox cbtn_log_saving = findViewById(R.id.setting_cbox_log_save);
            if (cbtn_log_saving.isChecked() && edit_path.getText().toString().length() > 0) {
                str_log_path = edit_path.getText().toString();
                b_log_save = updateLogFilePath(edit_path.getText().toString());
                if (!b_log_save) {
                    new AlertDialog.Builder(ActivitySettings.this)
                            .setTitle("Warning")
                            .setMessage("The log path is invalid, " + str_log_path)
                            .setNeutralButton("Modify", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    // do nothing
                                }})
                            .setPositiveButton("Exit", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    exitFromSettings();
                                }})
                            .show();
                }
                else {
                    Log.i(SYNA_TAG, "ActivitySettings button onClick() exit. " +
                            "( true , " + str_log_path + ")" );
                    exitFromSettings();
                }

            }
            else {
                Log.i(SYNA_TAG, "ActivitySettings button onClick() exit. " +
                        "(" + b_log_save + "," + str_log_path + ")" );
                exitFromSettings();
            }
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the RMI device node is changed
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_dev_rmi =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

            Log.i(SYNA_TAG, "ActivitySetting onCheckedChanged() rmi isChecked = "
                    + isChecked );

            str_dev_rmi_en = (isChecked)? "true" : "false";
            str_dev_tcm_en = "false";

            if(isChecked){
                edit_dev_rmi.setEnabled(true);
            }
            else {
                edit_dev_rmi.setEnabled(false);
            }

            /* to disable the tcm node */
            if (cbtn_dev_tcm.isChecked())
                cbtn_dev_tcm.performClick();
        }
    }; /* end CheckBox.OnCheckedChangeListener() */


    private TextView.OnEditorActionListener _edit_listener_dev_rmi =
            new TextView.OnEditorActionListener() {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {

            if ((actionId == EditorInfo.IME_ACTION_DONE) || (actionId == EditorInfo.IME_ACTION_NEXT)) {
                edit_dev_rmi.clearFocus();
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                assert imm != null;
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);

                String tmp = edit_dev_rmi.getText().toString();
                str_dev_node = tmp.replaceAll("\\s+", "");

                Log.i(SYNA_TAG, "ActivitySetting onEditorAction rmi node = " + str_dev_node);
                Log.i(SYNA_TAG, "ActivitySetting onEditorAction rmi enabled = " + str_dev_rmi_en +
                                    ", tcm enabled = " + str_dev_tcm_en);

                boolean ret = native_lib.onSetupDev(str_dev_node, str_dev_rmi_en, str_dev_tcm_en);
                if (ret) {
                    text_dev_err.setVisibility(View.INVISIBLE);

                    /* disable the tcm cbtn */
                    cbtn_dev_tcm.setEnabled(false);
                    edit_dev_tcm.setText("");
                }
                else {
                    text_dev_err.setText(R.string.syna_dev_unknown);
                    text_dev_err.setTextColor(Color.RED);
                    text_dev_err.setVisibility(View.VISIBLE);

                    str_dev_rmi_en = "false";
                    str_dev_tcm_en = "false";
                }
            }
            return false;
        }
    }; /* end Edit.OnEditorActionListener() */

    /********************************************************
     * implementation if the TCM device node is changed
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_dev_tcm =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

            Log.i(SYNA_TAG, "ActivitySetting onCheckedChanged() tcm isChecked = "
                    + isChecked );

            str_dev_tcm_en = (isChecked)? "true" : "false";
            str_dev_rmi_en = "false";

            if(isChecked){
                edit_dev_tcm.setEnabled(true);
            }
            else {
                edit_dev_tcm.setEnabled(false);
            }

            /* to disable the tcm node */
            if (cbtn_dev_rmi.isChecked())
                cbtn_dev_rmi.performClick();
        }
    }; /* end CheckBox.OnCheckedChangeListener() */


    private TextView.OnEditorActionListener _edit_listener_dev_tcm =
            new TextView.OnEditorActionListener() {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {

            if ((actionId == EditorInfo.IME_ACTION_DONE) || (actionId == EditorInfo.IME_ACTION_NEXT)) {
                edit_dev_tcm.clearFocus();
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                assert imm != null;
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);

                String tmp = edit_dev_tcm.getText().toString();
                str_dev_node = tmp.replaceAll("\\s+", "");

                Log.i(SYNA_TAG, "ActivitySetting onEditorAction tcm node = " + str_dev_node);
                Log.i(SYNA_TAG, "ActivitySetting onEditorAction rmi enabled = " + str_dev_rmi_en +
                        ", tcm enabled = " + str_dev_tcm_en);

                boolean ret = native_lib.onSetupDev(str_dev_node, str_dev_rmi_en, str_dev_tcm_en);
                if (ret) {
                    text_dev_err.setVisibility(View.INVISIBLE);

                    /* disable the rmi cbtn */
                    cbtn_dev_rmi.setEnabled(false);
                    edit_dev_rmi.setText("");
                }
                else {
                    text_dev_err.setText(R.string.syna_dev_unknown);
                    text_dev_err.setTextColor(Color.RED);
                    text_dev_err.setVisibility(View.VISIBLE);

                    str_dev_rmi_en = "false";
                    str_dev_tcm_en = "false";
                }
            }
            return false;
        }
    }; /* end Edit.OnEditorActionListener() */

    /********************************************************
     * implementation if the 'SAVE LOG FILE' is checked
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_savelog =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            b_log_save = isChecked;

            EditText edit_path = findViewById(R.id.setting_edit_log_path);
            TextView text_msg = findViewById(R.id.setting_cbox_log_msg);
            if(b_log_save){
                updateLogFilePath(str_log_path);  /* update the file path */
                edit_path.setEnabled(true);
                text_msg.setVisibility(View.VISIBLE);
            }
            else {
                edit_path.setEnabled(false);
                text_msg.setVisibility(View.INVISIBLE);
            }

        }
    }; /* end CheckBox.OnCheckedChangeListener() */

    /********************************************************
     * implementation if the path of log file is changed
     * normalize the input string, which should be ended with '/'
     ********************************************************/
    private TextView.OnEditorActionListener _edit_listener_logpath =
            new TextView.OnEditorActionListener() {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {

            if ((actionId == EditorInfo.IME_ACTION_DONE) || (actionId == EditorInfo.IME_ACTION_NEXT)) {
                EditText edit_path = findViewById(R.id.setting_edit_log_path);
                edit_path.clearFocus();
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                assert imm != null;
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);

                /* update the file path */
                updateLogFilePath(edit_path.getText().toString());
            }

            return false;
        }
    }; /* end Edit.OnEditorActionListener() */

    /********************************************************
     * helper function to update the path of log file,
     * and confirm the path is writable
     *******************************************************/
    private boolean updateLogFilePath(String str) {
        TextView text_msg = findViewById(R.id.setting_cbox_log_msg);
        EditText edit_path = findViewById(R.id.setting_edit_log_path);
        /* ensure the input path is writable */
        boolean is_valid = file_manager_lib.onCheckLogFilePath(str);
        if (is_valid) {
            str_log_path = file_manager_lib.onGetValidLogFilePath();
            edit_path.setText(str_log_path);
            text_msg.setText(R.string.str_log_path_ok);
            text_msg.setTextColor(0xff006400); // color = dark green

            Log.i(SYNA_TAG, "ActivitySetting: updateLogFilePath() path is ok, "
                    + str_log_path );

            if (!b_log_save)
                b_log_save = true;
        }
        else {
            text_msg.setText(R.string.str_log_path_invalid);
            text_msg.setTextColor(Color.RED);

            Log.e(SYNA_TAG, "ActivitySetting: updateLogFilePath() path is invalid, " + str );

            if (b_log_save)
                b_log_save = false; // to disable the flag of log saving since the path is invalid
        }

        Log.i(SYNA_TAG, "ActivitySetting: updateLogFilePath() log_path = "
                + str_log_path + ", log_save = " + b_log_save );
        return b_log_save;
    }  /* end updateLogFilePath() */


    /********************************************************
     * helper to go back to the main page
     *******************************************************/
    private void exitFromSettings() {
        Intent intent = new Intent();
        intent.setClass(ActivitySettings.this, MainActivity.class);
        intent.putExtra(Common.STR_DEV_NODE, str_dev_node);
        intent.putExtra(Common.STR_DEV_RMI_EN, str_dev_rmi_en);
        intent.putExtra(Common.STR_DEV_TCM_EN, str_dev_tcm_en);
        String str_is_log_save = (b_log_save)? "true":"false";
        intent.putExtra(Common.STR_CFG_LOG_SAVE_EN, str_is_log_save);
        intent.putExtra(Common.STR_CFG_LOG_PATH, str_log_path);
        startActivity(intent);
        ActivitySettings.this.finish();
    }

}
