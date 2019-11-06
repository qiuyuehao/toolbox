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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Locale;


public class ActivityProductionTester extends Activity {

    private final String SYNA_TAG = "syna-apk";

    /********************************************************
     * testing items listed on UI
     ********************************************************/
    public static final int TEST_NOISE              = 0;
    public static final int TEST_FULL_RAW_OR_DRT    = 1;
    public static final int TEST_TRX_TRX_SHORT_PT01 = 2;
    public static final int TEST_TRX_GROUND_PT03    = 3;
    public static final int TEST_ADC_RANGE_PT11     = 4;
    public static final int TEST_ABS_RAW_CAP_PT12   = 5;
    public static final int TEST_HYBRID_ABS_NOISE_PT1D = 6;
    public static final int TEST_EX_HIGH_RESISTANCE_PT05 = 7;

    public static final int TOTAL_ITEMS = 8;

    /********************************************************
     * object for native function access
     ********************************************************/
    private NativeWrapper native_lib;

    /********************************************************
     * handler for log file access
     ********************************************************/
    private LogFileManager log_manager = null;
    private TestCfgFileManager test_cfg_manager = null;

    /********************************************************
     * object to perform production test
     ********************************************************/
    private ProductionTest[] test = new ProductionTest[TOTAL_ITEMS];

    /********************************************************
     * components for Production Test Page
     ********************************************************/
    private LayoutInflater layout_inflater;
    private Button btn_exit;
    private Button btn_start_testing;
    private LinearLayout test_cfg_layout;
    /********************************************************
     * flag to save the status of process
     ********************************************************/
    private boolean b_running;
    /********************************************************
     * variables for log saving and external test_cfg file
     ********************************************************/
    private boolean b_log_save;
    private String str_test_cfg_file;
    private String str_test_cfg_en;

    /********************************************************
     * called from MainActivity if button 'Production Test' is pressed
     * to initial all UI objects
     ********************************************************/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* keep screen always on */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        /* to hide the navigation bar */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        /* set view */
        setContentView(R.layout.page_tester);

        Log.i(SYNA_TAG, "ActivityProductionTester onCreate() + " );

        /* get the apk version and change the app tittle*/
        String version_apk = "";
        try {
            PackageInfo pInfo = this.getPackageManager().getPackageInfo(getPackageName(), 0);
            version_apk = pInfo.versionName;
        }
        catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        Context mContext = ActivityProductionTester.this;
        layout_inflater = (LayoutInflater) mContext.getSystemService(LAYOUT_INFLATER_SERVICE);

        /* create an object to handle all production tests */
        for (int i = 0; i < test.length; i++)
            test[i] = new ProductionTest();

        /* create an object to handle native methods calling */
        native_lib = new NativeWrapper();

        /* create an object to handle log file access */
        test_cfg_manager = new TestCfgFileManager();

        /* get parameters from intent */
        Intent intent = this.getIntent();
        String str_dev_node = intent.getStringExtra(Common.STR_DEV_NODE);
        String str_dev_rmi_en = intent.getStringExtra(Common.STR_DEV_RMI_EN);
        String str_dev_tcm_en = intent.getStringExtra(Common.STR_DEV_TCM_EN);
        Log.i(SYNA_TAG, "ActivityProductionTester onCreate() dev_node: " +
                str_dev_node + " (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");

        String str_log_en = intent.getStringExtra(Common.STR_CFG_LOG_SAVE_EN);
        if (str_log_en != null) {
            try {
                b_log_save = Boolean.valueOf(str_log_en);
            } catch (NumberFormatException e) {
                Log.e(SYNA_TAG, "ActivityProductionTester onCreate() fail to convert str_log_en" );
                b_log_save = false;
            }
        }
        else {
            Log.e(SYNA_TAG, "ActivityProductionTester onCreate() str_log_en is invalid, use false");
            b_log_save = false;
        }

        String str_log_path = intent.getStringExtra(Common.STR_CFG_LOG_PATH);
        Log.i(SYNA_TAG, "ActivityProductionTester onCreate() log saving ( " + b_log_save + " ), " +
                "path: " + str_log_path );

        /* get parameters from intent about test config */
        str_test_cfg_en = intent.getStringExtra(Common.STR_CFG_TEST_INI_EN);
        boolean b_test_cfg;
        if (str_test_cfg_en != null) {
            try {
                b_test_cfg = Boolean.valueOf(str_test_cfg_en);
            } catch (NumberFormatException e) {
                Log.e(SYNA_TAG, "ActivityProductionTester onCreate() fail to convert str_test_cfg_en" );
                b_test_cfg = false;
            }
        }
        else {
            Log.e(SYNA_TAG, "ActivityProductionTester onCreate() str_test_cfg_en is invalid, use false");
            b_test_cfg = false;
        }

        str_test_cfg_file = intent.getStringExtra(Common.STR_CFG_TEST_INI_FILE);
        Log.i(SYNA_TAG, "ActivityProductionTester onCreate() test cfg input ( " + str_test_cfg_en + " )," +
                " file: " + str_test_cfg_file );
        if ((str_test_cfg_en == null) || (str_test_cfg_file == null)) {
            onLoadPreferencesTestCfg();
            if (str_test_cfg_en != null) {
                try {
                    b_test_cfg = Boolean.valueOf(str_test_cfg_en);
                } catch (NumberFormatException e) {
                    Log.e(SYNA_TAG, "ActivityProductionTester onCreate() fail to convert str_test_cfg_en" );
                    b_test_cfg = false;
                }
            }
        }

        /* confirm that the device is available */
        boolean ret = native_lib.onSetupDev(str_dev_node, str_dev_rmi_en, str_dev_tcm_en);
        if (!ret) {
            onShowErrorOut("Fail to find Synaptics device node, " + str_dev_node +
                    "\n\nPlease go to \"Setting\" page to setup a valid interface," +
                    " and please confirm its permission is proper as well.");
        }
        /* at first, perform the device identification */
        ret = native_lib.onIdentifyDev(null);
        if (!ret) {
            onShowErrorOut("Fail to perform device identification\n");
        }

        /* user interface initialization */

        /* UI components - button exit */
        btn_exit = findViewById(R.id.btn_test_exit);
        btn_exit.setOnClickListener(_button_listener_exit);
        /* UI components - button start testing */
        btn_start_testing = findViewById(R.id.btn_test_start);
        btn_start_testing.setOnClickListener(_button_listener_start_testing);
        /* UI components - layout for test configuration file */
        test_cfg_layout = findViewById(R.id.layout_test_cfg);

        /* initialize all test items */
        onInitTestItems();

        /* load preferences to get the property of each test items */
        onLoadPreferences();

        /* UI components - button for test configuration file */
        CheckBox cbtn_test_ini_input = findViewById(R.id.cbtn_test_cfg);
        cbtn_test_ini_input.setOnCheckedChangeListener(_checkbox_listener_usetestcfg);
        cbtn_test_ini_input.setChecked(b_test_cfg);

        /* create an object to handle log file access */
        log_manager = new LogTestData(native_lib, version_apk);

        /* confirm that the path of log file is valid */
        if (b_log_save) {
            boolean is_log_valid = log_manager.onCheckLogFilePath(str_log_path);
            if (!is_log_valid) {
                b_log_save = false; // set flag to false once the log path is invalid
                onShowWarningDialog(str_log_path + " is invalid. Disable the log saving.");
            }
        }

        /* update the status of log saving */
        CheckBox cbtn_property_log_saving = findViewById(R.id.cbtn_test_log_saving);
        TextView text_property_log_saving = findViewById(R.id.text_test_log);
        String str = "Log Saving is ";
        if (b_log_save) {
            str += "Enabled. ";
            cbtn_property_log_saving.setText(str);
            cbtn_property_log_saving.setChecked(true);
            text_property_log_saving.append(str_log_path);
            text_property_log_saving.setVisibility(View.VISIBLE);
        }
        else {
            str += "Disabled. ";
            cbtn_property_log_saving.setText(str);
            cbtn_property_log_saving.setChecked(false);
            text_property_log_saving.setVisibility(View.INVISIBLE);
        }

        /* based on b_test_cfg, enable/disable loading limit from the .ini file */
        onChangeStatusTestIniEdit(b_test_cfg);

        /* enable all test items saved in preference */
        for (ProductionTest t : test) {
            if (t.is_enabled)
                t.ui_cbtn.performClick();
        }

        /* initialize the flag */
        b_running = false;

        /* set the default selection to the start button */
        onSetBtnFocus(BTN_START);

    } /* end onCreate() */

    /********************************************************
     * called when this application is closed
     ********************************************************/
    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "ActivityProductionTester onStop() + " );
        if (b_running) {
            b_running = false;
        }

        super.onStop();
    } /* end onStop() */

    /********************************************************
     * function to listen the KeyEvent
     * VOLUME_UP - stop the report logger
     ********************************************************/
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch(keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();
                }
                if (b_running) {
                    b_running = false;
                }
                // perform the button click, if the process is not running
                else {
                    if (BTN_EXIT == current_btn_idx)
                        btn_exit.performClick();
                    else if  (BTN_START == current_btn_idx) {
                        btn_start_testing.performClick();
                    }
                }

                break;

            case KeyEvent.KEYCODE_VOLUME_DOWN:
                if (!b_running) {
                    current_btn_idx += 1;
                    if (current_btn_idx > BTN_START)
                        current_btn_idx = BTN_EXIT;
                    onSetBtnFocus(current_btn_idx);
                }
                break;
        }
        return false;
    } /* end onKeyDown() */

    /********************************************************
     * function to change the text color on selected button
     ********************************************************/
    private final int BTN_EXIT  = 0;
    private final int BTN_START = 1;
    private int current_btn_idx;
    private void onSetBtnFocus(int focus_idx) {
        for (int i = 0; i < 2; i++) {
            if (focus_idx == BTN_EXIT) {
                btn_exit.setTextColor(0xffb2d0b2);
                btn_start_testing.setTextColor(Color.WHITE);
            }
            else if (focus_idx == BTN_START)  {
                btn_start_testing.setTextColor(0xffb2d0b2);
                btn_exit.setTextColor(Color.WHITE);
            }
            else {
                Log.i(SYNA_TAG, "ActivityProductionTester setBtnFocus() unknown button index "
                        + focus_idx );
            }
        }
        current_btn_idx = focus_idx;
    }

    /********************************************************
     * implementation if 'START TEST' button is pressed
     * do production test with all selected items
     ********************************************************/
    private View.OnClickListener _button_listener_start_testing = new View.OnClickListener() {
        @Override
        public void onClick(View view) {

            /* reset all test result and its editor box */
            for (ProductionTest t : test) {
                if (t.ui_text_result != null)
                    t.ui_text_result.setText("");
                if (t.ui_edit_limit_min != null)
                    t.ui_edit_limit_min.clearFocus();
                if (t.ui_edit_limit_max != null)
                    t.ui_edit_limit_max.clearFocus();
            }

            /* retrieve test limit from ui */
            for (ProductionTest t : test) {
                if (t.ui_cbtn.isChecked()) {
                    int min = 0;
                    int max = 0;

                    if (t.ui_edit_limit_min != null) {
                        if (t.ui_edit_limit_min.getText().toString().matches("")) {
                            t.ui_edit_limit_min.setText(String.valueOf(t.onGetLimitMin()));
                        }
                        else {
                            min = Integer.parseInt(t.ui_edit_limit_min.getText().toString());
                        }
                    }
                    if (t.ui_edit_limit_max != null) {
                        if (t.ui_edit_limit_max.getText().toString().matches("")) {
                            t.ui_edit_limit_max.setText(String.valueOf(t.onGetLimitMax()));
                        }
                        else {
                            max = Integer.parseInt(t.ui_edit_limit_max.getText().toString());
                        }
                    }

                    /* configure the test limit in case */
                    t.onSetLimit(min, max);
                }
            }
            /* save preference */
            onSavePreferences();

            /* tag the test result into testing */
            for (ProductionTest t : test) {
                if (t.is_enabled) {
                    t.ui_text_result.setText(R.string.tester_going);
                    t.ui_text_result.setTextColor(Color.WHITE);
                }
            }

            /* call function to do production test */
            new Thread(new Runnable() {
                public void run() {

                    doProductionTest();

                } /* end of run() */
            }).start(); /* start the Thread */

        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if 'EXIT' button is pressed
     * go back to the MAIN page
     ********************************************************/
    private View.OnClickListener _button_listener_exit = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* save preference */
            onSavePreferences();

            Intent intent = new Intent();
            intent.setClass(ActivityProductionTester.this, MainActivity.class);
            startActivity(intent);
            ActivityProductionTester.this.finish();
        }
    }; /* end Button.OnClickListener() */

    /********************************************************
     * implementation if the 'USE TEST CFG' is checked
     *
     * if checked, enable the edit text to allow user to input
     * appointed test ini file; otherwise, disable it
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_usetestcfg =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    "Test INI Config isChecked = " + isChecked );

            str_test_cfg_en = (isChecked)?"true":"false";

            onClearTestIniInput();
            onChangeStatusTestIniEdit(isChecked);

        }
    }; /* end CheckBox.onCheckedChanged() */

    /********************************************************
     * implementation if the 'TEST INI FILE' is edited
     ********************************************************/
    private TextView.OnEditorActionListener _edit_listener =
            new TextView.OnEditorActionListener() {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            if ((actionId == EditorInfo.IME_ACTION_DONE) || (actionId == EditorInfo.IME_ACTION_NEXT)) {
                v.clearFocus();
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                assert imm != null;
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);

                onClearTestIniInput();

                /* update the string of test ini file */
                str_test_cfg_file = v.getText().toString();
                Log.i(SYNA_TAG, "ActivityProductionTester onEditorAction() str_test_cfg_file: "
                        + str_test_cfg_file );

                TextView text_cfg_msg = findViewById(R.id.test_cfg_msg);
                boolean ret = test_cfg_manager.onCheckTestCfgFile(str_test_cfg_file);
                if (!ret) {
                    text_cfg_msg.setVisibility(View.VISIBLE);
                }
                else {
                    text_cfg_msg.setVisibility(View.INVISIBLE);
                    onInitTestIniInput();
                }

                /* update button ui if needed */
                onChangeStatusTestItem(TEST_TRX_TRX_SHORT_PT01,
                        test[TEST_TRX_TRX_SHORT_PT01].onHasLimitFromTestCfg());
                onChangeStatusTestItem(TEST_TRX_GROUND_PT03,
                        test[TEST_TRX_GROUND_PT03].onHasLimitFromTestCfg());
                onChangeStatusTestItem(TEST_EX_HIGH_RESISTANCE_PT05,
                        test[TEST_EX_HIGH_RESISTANCE_PT05].onHasLimitFromTestCfg());

                /* update the status whether it loads the limit from test ini file */
                onChangeStatusTestIniSelected();

                return true;
            }
            return false;
        }
    }; /* end Edit.OnEditorActionListener() */

    /********************************************************
     * function to change the status of edit text which allows
     * user to input appointed test ini file
     ********************************************************/
    private void onChangeStatusTestIniEdit(boolean en) {
        Log.i(SYNA_TAG, "ActivityProductionTester onChangeStatusTestIniEdit (" + en + ")" );

        TextView text_cfg_msg = findViewById(R.id.test_cfg_msg);
        text_cfg_msg.setVisibility(View.INVISIBLE);

        /* if enabled, attach the layout containing the edit box */
        if (en) {

            final ViewGroup nullParent = null;
            View layout_parameter = layout_inflater.inflate(R.layout.item_test_general_config_parameter,
                    nullParent);
            if (test_cfg_layout.getChildCount() == 1) {
                test_cfg_layout.addView(layout_parameter);
            }

            EditText edit_test_ini = findViewById(R.id.edit_test_cfg_file);
            edit_test_ini.setOnEditorActionListener(_edit_listener);

            if ((str_test_cfg_file != null) && (str_test_cfg_file.length() > 0)) {
                edit_test_ini.setText(str_test_cfg_file);
                onInitTestIniInput();

                /* update button ui if needed */
                onChangeStatusTestItem(TEST_TRX_TRX_SHORT_PT01,
                        test[TEST_TRX_TRX_SHORT_PT01].onHasLimitFromTestCfg());
                onChangeStatusTestItem(TEST_TRX_GROUND_PT03,
                        test[TEST_TRX_GROUND_PT03].onHasLimitFromTestCfg());
                onChangeStatusTestItem(TEST_EX_HIGH_RESISTANCE_PT05,
                        test[TEST_EX_HIGH_RESISTANCE_PT05].onHasLimitFromTestCfg());
            }
            else {
                test[TEST_TRX_TRX_SHORT_PT01].onClearFlagFromTestCfg();
                onChangeStatusTestItem(TEST_TRX_TRX_SHORT_PT01, false);

                test[TEST_TRX_GROUND_PT03].onClearFlagFromTestCfg();
                onChangeStatusTestItem(TEST_TRX_GROUND_PT03, false);

                test[TEST_EX_HIGH_RESISTANCE_PT05].onClearFlagFromTestCfg();
                onChangeStatusTestItem(TEST_EX_HIGH_RESISTANCE_PT05, false);
            }
        }
        /* otherwise, remove the attached layout */
        else {
            if (test_cfg_layout.getChildCount() > 1)
                test_cfg_layout.removeViewAt(1);

            /* disable the button ui if needed */
            test[TEST_TRX_TRX_SHORT_PT01].onClearFlagFromTestCfg();
            onChangeStatusTestItem(TEST_TRX_TRX_SHORT_PT01, false);

            test[TEST_TRX_GROUND_PT03].onClearFlagFromTestCfg();
            onChangeStatusTestItem(TEST_TRX_GROUND_PT03, false);

            test[TEST_EX_HIGH_RESISTANCE_PT05].onClearFlagFromTestCfg();
            onChangeStatusTestItem(TEST_EX_HIGH_RESISTANCE_PT05, false);
        }

        /* go through all test items,
         * and update its status whether load from the ini file */
        onChangeStatusTestIniSelected();
    }

    /********************************************************
     * function to change the status whether loads the limit
     * from test ini file or not
     *
     * if the item is enabled, click the checkbox button twice
     * to call its listener
     ********************************************************/
    private void onChangeStatusTestIniSelected() {

        for (ProductionTest t : test) {
            if (t.ui_cbtn == null)
                continue;

            if (t.ui_cbtn.isChecked() && t.ui_cbtn.isEnabled()) {
                t.ui_cbtn.performClick(); // close the item
                t.ui_cbtn.performClick(); // re-open it again
            }
        }
    }

    /********************************************************
     * function to change the state of appointed test item
     *
     * this function can enable/disable the testing item
     * if it is running with the test ini file only
     ********************************************************/
    private void onChangeStatusTestItem(int item, boolean en) {

        if (test[item].ui_cbtn == null)
            return;

        if (en) {
            test[item].ui_cbtn.setEnabled(true);
        }
        else {
            /* uncheck the button if it is checked */
            if (test[item].ui_cbtn.isChecked()) {
                test[item].ui_cbtn.setChecked(false);
            }

            test[item].is_enabled = false;  /* set to disable as well */
            test[item].ui_cbtn.setEnabled(false);
        }
    }

    /********************************************************
     * TCM Noise Test (PID0A) - TCM
     * RMI Noise Test (RT02)  - RMI
     * implementation if 'TEST' is selected
     *
     * if enabled, attach the specified user interface from other .xml file
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_test_noise =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            final ViewGroup nullParent = null;
            View layout_parameter =
                    layout_inflater.inflate(R.layout.item_test_general_noise_parameter,
                    nullParent);

            int item = TEST_NOISE;

            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    String.format(Locale.getDefault(), "%s", test[item].getTestName()) +
                    " (" + isChecked + ")" );

            /* if item is enabled, attach the layout and initialize its ui component */
            if(isChecked) {
                test[item].ui_layout.addView(layout_parameter);

                test[item].ui_edit_limit_max = findViewById(R.id.edit_limit_noise);
                test[item].ui_edit_limit_max.setText(String.valueOf(test[item].onGetLimitMax()));

                test[item].ui_edit_limit_min = null;

                test[item].ui_text_result = findViewById(R.id.text_result_noise);
                test[item].ui_rbtn_ini_input = findViewById(R.id.rbtn_cfg_en_noise);

                /* check whether the test ini file is using */
                if (test[item].onHasLimitFromTestCfg()) {
                    test[item].ui_rbtn_ini_input.setChecked(true);
                    test[item].ui_rbtn_ini_input.setVisibility(View.VISIBLE);
                    /* disable the edit because the limit is loaded from the ini file */
                    test[item].ui_edit_limit_max.setEnabled(false);
                }
                else {
                    test[item].ui_rbtn_ini_input.setChecked(false);
                    test[item].ui_rbtn_ini_input.setVisibility(View.INVISIBLE);
                    test[item].ui_edit_limit_max.setEnabled(true);
                }
            }
            /* if item is disabled, remove the attached layout */
            else {
                test[item].ui_layout.removeViewAt(1);
                test[item].ui_edit_limit_max = null;
                test[item].ui_edit_limit_min = null;
                test[item].ui_text_result = null;
            }

            /* update the flag of this test item */
            test[item].is_enabled = isChecked;

        }
    }; /* end CheckBox.OnCheckedChangeListener() */
    /********************************************************
     * Dynamic Range Test (PID07) - TCM TDDI
     * Full Raw Test      (PID05) - TCM
     * TDDI Full Raw Test (RT92)  - RMI TDDI
     * Full Raw Test      (RT20)  - RMI
     * implementation if 'TEST' is selected
     *
     * if enabled, attach the specified user interface from other .xml file
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_test_full_raw_or_drt =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            final ViewGroup nullParent = null;
            View layout_parameter =
                    layout_inflater.inflate(R.layout.item_test_general_full_raw_or_drt_parameter,
                    nullParent);

            int item = TEST_FULL_RAW_OR_DRT;

            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    String.format(Locale.getDefault(), "%s", test[item].getTestName()) +
                    " (" + isChecked + ")" );

            /* if item is enabled, attach the layout and initialize its ui component */
            if(isChecked) {
                test[item].ui_layout.addView(layout_parameter);

                test[item].ui_edit_limit_min =
                        findViewById(R.id.edit_limit_min_full_raw_or_drt);
                test[item].ui_edit_limit_min.setText(
                        String.valueOf(test[item].onGetLimitMin()));

                test[item].ui_edit_limit_max =
                        findViewById(R.id.edit_limit_max_full_raw_or_drt);
                test[item].ui_edit_limit_max.setText(
                        String.valueOf(test[item].onGetLimitMax()));

                test[item].ui_text_result =
                        findViewById(R.id.text_result_full_raw_or_drt);
                test[item].ui_rbtn_ini_input =
                        findViewById(R.id.rbtn_cfg_en_full_raw_or_drt);

                /* check whether the test ini file is using */
                if (test[item].onHasLimitFromTestCfg()) {
                    test[item].ui_rbtn_ini_input.setChecked(true);
                    test[item].ui_rbtn_ini_input.setVisibility(View.VISIBLE);
                    /* disable the edit because the limit is loaded from the ini file */
                    test[item].ui_edit_limit_min.setEnabled(false);
                    test[item].ui_edit_limit_max.setEnabled(false);
                }
                else {
                    test[item].ui_rbtn_ini_input.setChecked(false);
                    test[item].ui_rbtn_ini_input.setVisibility(View.INVISIBLE);
                    test[item].ui_edit_limit_min.setEnabled(true);
                    test[item].ui_edit_limit_max.setEnabled(true);
                }
            }
            /* if item is disabled, remove the attached layout */
            else {
                test[item].ui_layout.removeViewAt(1);
                test[item].ui_edit_limit_min = null;
                test[item].ui_edit_limit_max = null;
                test[item].ui_text_result = null;
            }

            /* update the flag of this test item */
            test[item].is_enabled = isChecked;

        }
    }; /* end CheckBox.OnCheckedChangeListener() */

    /********************************************************
     * TRx TRXx Short Test PT01 - TCM
     * implementation if 'TEST' is selected
     *
     * if enabled, attach the specified user interface from other .xml file
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_test_trx_trx_short_pt01 =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            final ViewGroup nullParent = null;
            View layout_parameter = layout_inflater.inflate(
                    R.layout.item_test_tcm_trx_trx_short_pt01_parameter,
                    nullParent);

            int item = TEST_TRX_TRX_SHORT_PT01;

            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    String.format(Locale.getDefault(), "%s", test[item].getTestName()) +
                    " (" + isChecked + ")" );

            /* if item is enabled, attach the layout and initialize its ui component */
            if(isChecked) {
                test[item].ui_layout.addView(layout_parameter);

                test[item].ui_edit_limit_custom =
                        findViewById(R.id.edit_limit_trx_trx_short_pt01);

                test[item].ui_text_result =
                        findViewById(R.id.text_result_trx_trx_short_pt01);
                test[item].ui_rbtn_ini_input =
                        findViewById(R.id.rbtn_cfg_en_trx_trx_short_pt01);

                /* check whether the test ini file is using */
                if (test[item].onHasLimitFromTestCfg()) {
                    test[item].ui_rbtn_ini_input.setChecked(true);
                    test[item].ui_rbtn_ini_input.setVisibility(View.VISIBLE);

                    test[item].ui_edit_limit_custom.setText(test[item].onGetCustomLimitStr(false));
                    test[item].ui_edit_limit_custom.setEnabled(false);
                }
                else {
                    test[item].ui_rbtn_ini_input.setChecked(false);
                    test[item].ui_rbtn_ini_input.setVisibility(View.INVISIBLE);
                }
            }
            /* if item is disabled, remove the attached layout */
            else {
                test[item].ui_layout.removeViewAt(1);
                test[item].ui_edit_limit_min = null;
                test[item].ui_edit_limit_max = null;
                test[item].ui_text_result = null;
            }

            /* update the flag of this test item */
            test[item].is_enabled = isChecked;
        }
    }; /* end CheckBox.OnCheckedChangeListener() */

    /********************************************************
     * TRx Ground Test PT03 - TCM
     * implementation if 'TEST' is selected
     *
     * if enabled, attach the specified user interface from other .xml file
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_test_trx_ground_pt03 =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            final ViewGroup nullParent = null;
            View layout_parameter = layout_inflater.inflate(
                    R.layout.item_test_tcm_trx_ground_pt03_parameter,
                    nullParent);

            int item = TEST_TRX_GROUND_PT03;

            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    String.format(Locale.getDefault(), "%s", test[item].getTestName()) +
                    " (" + isChecked + ")" );


            /* if item is enabled, attach the layout and initialize its ui component */
            if(isChecked) {
                test[item].ui_layout.addView(layout_parameter);

                test[item].ui_edit_limit_custom =
                        findViewById(R.id.edit_limit_trx_ground_pt03);

                test[item].ui_text_result =
                        findViewById(R.id.text_result_trx_ground_pt03);
                test[item].ui_rbtn_ini_input =
                        findViewById(R.id.rbtn_cfg_en_trx_ground_pt03);

                /* check whether the test ini file is using */
                if (test[item].onHasLimitFromTestCfg()) {
                    test[item].ui_rbtn_ini_input.setChecked(true);
                    test[item].ui_rbtn_ini_input.setVisibility(View.VISIBLE);

                    test[item].ui_edit_limit_custom.setText(test[item].onGetCustomLimitStr(false));
                    test[item].ui_edit_limit_custom.setEnabled(false);
                }
                else {
                    test[item].ui_rbtn_ini_input.setChecked(false);
                    test[item].ui_rbtn_ini_input.setVisibility(View.INVISIBLE);
                }
            }
            /* if item is disabled, remove the attached layout */
            else {
                test[item].ui_layout.removeViewAt(1);
                test[item].ui_edit_limit_min = null;
                test[item].ui_edit_limit_max = null;
                test[item].ui_text_result = null;
            }

            /* update the flag of this test item */
            test[item].is_enabled = isChecked;
        }
    }; /* end CheckBox.OnCheckedChangeListener() */

    /********************************************************
     * ADC Range Test PT11 - TCM
     * implementation if 'TEST' is selected
     *
     * if enabled, attach the specified user interface from other .xml file
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_test_adc_range_pt11 =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            final ViewGroup nullParent = null;
            View layout_parameter = layout_inflater.inflate(
                    R.layout.item_test_tcm_adc_range_pt11_parameter,
                    nullParent);

            int item = TEST_ADC_RANGE_PT11;

            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    String.format(Locale.getDefault(), "%s", test[item].getTestName()) +
                    " (" + isChecked + ")" );

            /* if item is enabled, attach the layout and initialize its ui component */
            if(isChecked) {
                test[item].ui_layout.addView(layout_parameter);

                test[item].ui_edit_limit_min =
                        findViewById(R.id.edit_limit_min_adc_range_pt11);
                test[item].ui_edit_limit_min.setText(
                        String.valueOf(test[item].onGetLimitMin()));

                test[item].ui_edit_limit_max =
                        findViewById(R.id.edit_limit_max_adc_range_pt11);
                test[item].ui_edit_limit_max.setText(
                        String.valueOf(test[item].onGetLimitMax()));

                test[item].ui_text_result =
                        findViewById(R.id.text_result_adc_range_pt11);
                test[item].ui_rbtn_ini_input =
                        findViewById(R.id.rbtn_cfg_en_adc_range_pt11);

                /* check whether the test ini file is using */
                if (test[item].onHasLimitFromTestCfg()) {
                    test[item].ui_rbtn_ini_input.setChecked(true);
                    test[item].ui_rbtn_ini_input.setVisibility(View.VISIBLE);
                    /* disable the edit because the limit is loaded from the ini file */
                    test[item].ui_edit_limit_min.setEnabled(false);
                    test[item].ui_edit_limit_max.setEnabled(false);
                }
                else {
                    test[item].ui_rbtn_ini_input.setChecked(false);
                    test[item].ui_rbtn_ini_input.setVisibility(View.INVISIBLE);
                    test[item].ui_edit_limit_min.setEnabled(true);
                    test[item].ui_edit_limit_max.setEnabled(true);
                }
            }
            /* if item is disabled, remove the attached layout */
            else {
                test[item].ui_layout.removeViewAt(1);
                test[item].ui_edit_limit_min = null;
                test[item].ui_edit_limit_max = null;
                test[item].ui_text_result = null;
            }

            /* update the flag of this test item */
            test[item].is_enabled = isChecked;
        }
    }; /* end CheckBox.OnCheckedChangeListener() */

    /********************************************************
     * Abs Raw Cap Test PT12 - TCM
     * implementation if 'TEST' is selected
     *
     * if enabled, attach the specified user interface from other .xml file
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_test_abs_raw_pt12 =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            final ViewGroup nullParent = null;
            View layout_parameter = layout_inflater.inflate(
                    R.layout.item_test_tcm_abs_raw_cap_pt12_parameter,
                    nullParent);

            int item = TEST_ABS_RAW_CAP_PT12;

            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    String.format(Locale.getDefault(), "%s", test[item].getTestName()) +
                    " (" + isChecked + ")" );

            /* if item is enabled, attach the layout and initialize its ui component */
            if(isChecked) {
                test[item].ui_layout.addView(layout_parameter);

                test[item].ui_edit_limit_min =
                        findViewById(R.id.edit_limit_min_abs_raw_cap_pt12);
                test[item].ui_edit_limit_min.setText(
                        String.valueOf(test[item].onGetLimitMin()));

                test[item].ui_edit_limit_max =
                        findViewById(R.id.edit_limit_max_abs_raw_cap_pt12);
                test[item].ui_edit_limit_max.setText(
                        String.valueOf(test[item].onGetLimitMax()));

                test[item].ui_text_result =
                        findViewById(R.id.text_result_abs_raw_cap_pt12);
                test[item].ui_rbtn_ini_input =
                        findViewById(R.id.rbtn_cfg_en_abs_raw_cap_pt12);

                /* check whether the test ini file is using */
                if (test[item].onHasLimitFromTestCfg()) {
                    test[item].ui_rbtn_ini_input.setChecked(true);
                    test[item].ui_rbtn_ini_input.setVisibility(View.VISIBLE);
                    /* disable the edit because the limit is loaded from the ini file */
                    test[item].ui_edit_limit_min.setEnabled(false);
                    test[item].ui_edit_limit_max.setEnabled(false);
                }
                else {
                    test[item].ui_rbtn_ini_input.setChecked(false);
                    test[item].ui_rbtn_ini_input.setVisibility(View.INVISIBLE);
                    test[item].ui_edit_limit_min.setEnabled(true);
                    test[item].ui_edit_limit_max.setEnabled(true);
                }
            }
            /* if item is disabled, remove the attached layout */
            else {
                test[item].ui_layout.removeViewAt(1);
                test[item].ui_edit_limit_min = null;
                test[item].ui_edit_limit_max = null;
                test[item].ui_text_result = null;
            }

            /* update the flag of this test item */
            test[item].is_enabled = isChecked;
        }
    }; /* end CheckBox.OnCheckedChangeListener() */

    /********************************************************
     * Hybrid Abs Noise Test PT1D - TCM
     * implementation if 'TEST' is selected
     *
     * if enabled, attach the specified user interface from other .xml file
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_test_hybrid_abs_noise_pt1d =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            final ViewGroup nullParent = null;
            View layout_parameter = layout_inflater.inflate(
                    R.layout.item_test_tcm_hybrid_abs_noise_pt1d_parameter,
                    nullParent);

            int item = TEST_HYBRID_ABS_NOISE_PT1D;

            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    String.format(Locale.getDefault(), "%s", test[item].getTestName()) +
                    " (" + isChecked + ")" );

            /* if item is enabled, attach the layout and initialize its ui component */
            if(isChecked) {
                test[item].ui_layout.addView(layout_parameter);

                test[item].ui_edit_limit_min =
                        findViewById(R.id.edit_limit_min_hy_abs_noise_pt1d);
                test[item].ui_edit_limit_min.setText(
                        String.valueOf(test[item].onGetLimitMin()));

                test[item].ui_edit_limit_max =
                        findViewById(R.id.edit_limit_max_hy_abs_noise_pt1d);
                test[item].ui_edit_limit_max.setText(
                        String.valueOf(test[item].onGetLimitMax()));

                test[item].ui_text_result =
                        findViewById(R.id.text_result_hy_abs_noise_pt1d);
                test[item].ui_rbtn_ini_input =
                        findViewById(R.id.rbtn_cfg_en_hy_abs_noise_pt1d);

                /* check whether the test ini file is using */
                if (test[item].onHasLimitFromTestCfg()) {
                    test[item].ui_rbtn_ini_input.setChecked(true);
                    test[item].ui_rbtn_ini_input.setVisibility(View.VISIBLE);
                    /* disable the edit because the limit is loaded from the ini file */
                    test[item].ui_edit_limit_min.setEnabled(false);
                    test[item].ui_edit_limit_max.setEnabled(false);
                }
                else {
                    test[item].ui_rbtn_ini_input.setChecked(false);
                    test[item].ui_rbtn_ini_input.setVisibility(View.INVISIBLE);
                    test[item].ui_edit_limit_min.setEnabled(true);
                    test[item].ui_edit_limit_max.setEnabled(true);
                }
            }
            /* if item is disabled, remove the attached layout */
            else {
                test[item].ui_layout.removeViewAt(1);
                test[item].ui_edit_limit_min = null;
                test[item].ui_edit_limit_max = null;
                test[item].ui_text_result = null;
            }

            /* update the flag of this test item */
            test[item].is_enabled = isChecked;
        }
    }; /* end CheckBox.OnCheckedChangeListener() */

    /********************************************************
     * Extended High Resistance Test PT05 - TCM
     * implementation if 'TEST' is selected
     *
     * if enabled, attach the specified user interface from other .xml file
     ********************************************************/
    private CheckBox.OnCheckedChangeListener _checkbox_listener_test_ex_high_resistance_pt05 =
            new CheckBox.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            final ViewGroup nullParent = null;
            View layout_parameter = layout_inflater.inflate(
                    R.layout.item_test_tcm_ex_high_resistance_pt05_parameter,
                    nullParent);

            int item = TEST_EX_HIGH_RESISTANCE_PT05;

            Log.i(SYNA_TAG, "ActivityProductionTester onCheckedChanged() " +
                    String.format(Locale.getDefault(), "%s", test[item].getTestName()) +
                    " (" + isChecked + ")" );

            /* if item is enabled, attach the layout and initialize its ui component */
            if(isChecked) {
                test[item].ui_layout.addView(layout_parameter);

                test[item].ui_edit_limit_min = null;
                test[item].ui_edit_limit_max = null;

                EditText e_tixel = findViewById(R.id.edit_limit_ex_high_resistance_tixel_pt05);
                EditText e_rxroe = findViewById(R.id.edit_limit_ex_high_resistance_rxroe_pt05);
                EditText e_txroe = findViewById(R.id.edit_limit_ex_high_resistance_txroe_pt05);
                e_tixel.setEnabled(false);
                e_rxroe.setEnabled(false);
                e_txroe.setEnabled(false);

                test[item].ui_text_result =
                        findViewById(R.id.text_result_ex_high_resistance_pt05);
                test[item].ui_rbtn_ini_input =
                        findViewById(R.id.rbtn_cfg_en_ex_high_resistance_pt05);

                /* check whether the test ini file is using */
                if (test[item].onHasLimitFromTestCfg()) {
                    test[item].ui_rbtn_ini_input.setChecked(true);
                    test[item].ui_rbtn_ini_input.setVisibility(View.VISIBLE);

                    e_tixel.setText(String.valueOf(test[item].onGetLimitTixel()));
                    e_rxroe.setText(String.valueOf(test[item].onGetLimitRxRoe()));
                    e_txroe.setText(String.valueOf(test[item].onGetLimitTxRoe()));
                }
                else {
                    test[item].ui_rbtn_ini_input.setChecked(false);
                    test[item].ui_rbtn_ini_input.setVisibility(View.INVISIBLE);
                }
            }
            /* if item is disabled, remove the attached layout */
            else {
                test[item].ui_layout.removeViewAt(1);
                test[item].ui_edit_limit_min = null;
                test[item].ui_edit_limit_max = null;
                test[item].ui_text_result = null;
            }

            /* update the flag of this test item */
            test[item].is_enabled = isChecked;

        }
    }; /* end CheckBox.OnCheckedChangeListener() */

    /********************************************************
     * function to initialize all test items on ui
     ********************************************************/
    private void onInitTestItems() {

        for (int item = 0; item < test.length; item++) {
            switch (item) {

                /* Noise Test */
                /* TCM Noise Test (PID0A), for TCM device */
                /* RMI Noise Test (RT02) , for RMI device */
                case TEST_NOISE:
                    test[item].ui_layout = findViewById(R.id.layout_test_item_1);

                    test[item].ui_cbtn = findViewById(R.id.cbtn_test_item_1);
                    test[item].ui_cbtn.setOnCheckedChangeListener(_checkbox_listener_test_noise);

                    if (native_lib.isDevRMI()) {
                        test[item].ui_cbtn.setText(getString(R.string.tester_rmi_noise_test_rt02));
                        test[item].id = ProductionTestItems.TEST_RMI_NOISE_RT02;
                    }
                    else if (native_lib.isDevTCM()) {
                        test[item].ui_cbtn.setText(getString(R.string.tester_tcm_noise_test_pt0a));
                        test[item].id = ProductionTestItems.TEST_TCM_NOISE_PID0A;
                    }

                    test[item].ui_cbtn.setEnabled(true);
                    test[item].ui_cbtn.setVisibility(View.VISIBLE);

                    break;

                /* Dynamic Range Test (PID07), for TCM TDDI device */
                /* Full Raw Test,     (PID05), for TCM device      */
                /* TDDI Full Raw Test (RT92) , for RMI TDDI device */
                /* Full Raw Test,     (RT20) , for RMI device      */
                case TEST_FULL_RAW_OR_DRT:
                    test[item].ui_layout = findViewById(R.id.layout_test_item_2);

                    test[item].ui_cbtn = findViewById(R.id.cbtn_test_item_2);
                    test[item].ui_cbtn.setOnCheckedChangeListener(_checkbox_listener_test_full_raw_or_drt);

                    if (native_lib.isDevRMI()) {
                        if (native_lib.isDevRMITDDI()) {
                            test[item].ui_cbtn.setText(getString(R.string.tester_rmi_full_raw_test_rt92));
                            test[item].id = ProductionTestItems.TEST_RMI_TDDI_FULL_RAW_RT92;
                        }
                        else {
                            test[item].ui_cbtn.setText(getString(R.string.tester_rmi_full_raw_test_rt20));
                            test[item].id = ProductionTestItems.TEST_RMI_FULL_RAW_RT20;
                        }
                    }
                    else if (native_lib.isDevTCM()) {
                        if (native_lib.isDevTCMTDDI()) {
                            test[item].ui_cbtn.setText(getString(R.string.tester_tcm_drt_test_pt07));
                            test[item].id = ProductionTestItems.TEST_TCM_DRT_PID07;
                        }
                        else {
                            test[item].ui_cbtn.setText(getString(R.string.tester_tcm_full_raw_test_pt05));
                            test[item].id = ProductionTestItems.TEST_TCM_FULL_RAW_PID05;
                        }
                    }

                    test[item].ui_cbtn.setEnabled(true);
                    test[item].ui_cbtn.setVisibility(View.VISIBLE);

                    break;

                /* TRx TRXx Short Test PT01 */
                case TEST_TRX_TRX_SHORT_PT01:
                    test[item].ui_layout = findViewById(R.id.layout_test_item_trx_trx_short_pt01);

                    test[item].ui_cbtn = findViewById(R.id.cbtn_test_item_trx_trx_short_pt01);
                    test[item].ui_cbtn.setOnCheckedChangeListener(_checkbox_listener_test_trx_trx_short_pt01);

                    if (native_lib.isDevTCM() && !native_lib.isDevTCMTDDI()) {
                        test[item].ui_cbtn.setEnabled(true);
                        test[item].ui_cbtn.setText(getString(R.string.tester_tcm_trx_trx_short_test_pt01));
                        test[item].ui_cbtn.setVisibility(View.VISIBLE);

                        test[item].id = ProductionTestItems.TEST_TCM_TRX_TRX_SHORT_PID01;
                    }
                    else {
                        test[item].ui_cbtn.setEnabled(false);
                        test[item].ui_cbtn.setVisibility(View.INVISIBLE);
                    }

                    break;

                /* TRx Ground Short PT03 */
                case TEST_TRX_GROUND_PT03:
                    test[item].ui_layout = findViewById(R.id.layout_test_item_trx_ground_pt03);

                    test[item].ui_cbtn = findViewById(R.id.cbtn_test_item_trx_ground_pt03);
                    test[item].ui_cbtn.setOnCheckedChangeListener(_checkbox_listener_test_trx_ground_pt03);

                    if (native_lib.isDevTCM() && !native_lib.isDevTCMTDDI()) {
                        test[item].ui_cbtn.setEnabled(true);
                        test[item].ui_cbtn.setText(getString(R.string.tester_tcm_trx_ground_test_pt03));
                        test[item].ui_cbtn.setVisibility(View.VISIBLE);

                        test[item].id = ProductionTestItems.TEST_TCM_TRX_GROUND_PID03;
                    }
                    else {
                        test[item].ui_cbtn.setEnabled(false);
                        test[item].ui_cbtn.setVisibility(View.INVISIBLE);
                    }

                    break;

                /* ADC Range Test PT11 */
                case TEST_ADC_RANGE_PT11:
                    test[item].ui_layout = findViewById(R.id.layout_test_item_adc_range_pt11);

                    test[item].ui_cbtn = findViewById(R.id.cbtn_test_item_adc_range_pt11);
                    test[item].ui_cbtn.setOnCheckedChangeListener(_checkbox_listener_test_adc_range_pt11);

                    if (native_lib.isDevTCM() && !native_lib.isDevTCMTDDI()) {
                        test[item].ui_cbtn.setEnabled(true);
                        test[item].ui_cbtn.setText(getString(R.string.tester_tcm_adc_range_test_pt11));
                        test[item].ui_cbtn.setVisibility(View.VISIBLE);

                        test[item].id = ProductionTestItems.TEST_TCM_ADC_RANGE_PID11;
                    }
                    else {
                        test[item].ui_cbtn.setEnabled(false);
                        test[item].ui_cbtn.setVisibility(View.INVISIBLE);
                    }

                    break;

                /* Abs Raw Cap Test PT12 */
                case TEST_ABS_RAW_CAP_PT12:
                    test[item].ui_layout = findViewById(R.id.layout_test_item_abs_raw_cap_pt12);

                    test[item].ui_cbtn = findViewById(R.id.cbtn_test_item_abs_raw_cap_pt12);
                    test[item].ui_cbtn.setOnCheckedChangeListener(_checkbox_listener_test_abs_raw_pt12);

                    if (native_lib.isDevTCM() && !native_lib.isDevTCMTDDI()) {
                        test[item].ui_cbtn.setEnabled(true);
                        test[item].ui_cbtn.setText(getString(R.string.tester_tcm_abs_raw_test_pt12));
                        test[item].ui_cbtn.setVisibility(View.VISIBLE);

                        test[item].id = ProductionTestItems.TEST_TCM_ABS_RAWCAP_PID12;
                    }
                    else {
                        test[item].ui_cbtn.setEnabled(false);
                        test[item].ui_cbtn.setVisibility(View.INVISIBLE);
                    }

                    break;

                /* Hybrid Abs Noise Test PT1D */
                case TEST_HYBRID_ABS_NOISE_PT1D:
                    test[item].ui_layout = findViewById(R.id.layout_test_item_hy_abs_noise_pt1d);

                    test[item].ui_cbtn = findViewById(R.id.cbtn_test_item_hy_abs_noise_pt1d);
                    test[item].ui_cbtn.setOnCheckedChangeListener(_checkbox_listener_test_hybrid_abs_noise_pt1d);

                    if (native_lib.isDevTCM() && !native_lib.isDevTCMTDDI()) {
                        test[item].ui_cbtn.setEnabled(true);
                        test[item].ui_cbtn.setText(getString(R.string.tester_tcm_hybrid_abs_noise_test_pt1d));
                        test[item].ui_cbtn.setVisibility(View.VISIBLE);

                        test[item].id = ProductionTestItems.TEST_TCM_HYBRID_ABS_NOISE_PID1D;
                    }
                    else {
                        test[item].ui_cbtn.setEnabled(false);
                        test[item].ui_cbtn.setVisibility(View.INVISIBLE);
                    }

                    break;

                /* Extended High Resistance Test PT05 */
                case TEST_EX_HIGH_RESISTANCE_PT05:
                    test[item].ui_layout = findViewById(R.id.layout_test_item_ex_high_resistance_pt05);

                    test[item].ui_cbtn = findViewById(R.id.cbtn_test_item_ex_high_resistance_pt05);
                    test[item].ui_cbtn.setOnCheckedChangeListener(_checkbox_listener_test_ex_high_resistance_pt05);

                    if (native_lib.isDevTCM() && !native_lib.isDevTCMTDDI()) {
                        test[item].ui_cbtn.setEnabled(true);
                        test[item].ui_cbtn.setText(getString(R.string.tester_tcm_ex_high_resistance_test_pt1d));
                        test[item].ui_cbtn.setVisibility(View.VISIBLE);

                        test[item].id = ProductionTestItems.TEST_TCM_EX_HIGH_RESISTANCE_PID05;
                    }
                    else {
                        test[item].ui_cbtn.setEnabled(false);
                        test[item].ui_cbtn.setVisibility(View.INVISIBLE);
                    }

                    break;
            } /* end switch */
        } /* end for loop */

    } /* end onInitTestItems() */

    /********************************************************
     * perform the production testing
     * this function will execute every selected testing items
     * and save the result to the log if enabled
     ********************************************************/
    public void doProductionTest() {
        Log.i(SYNA_TAG, "ActivityProductionTester doProductionTest() + ");
        boolean ret;

        /* prepare the log file */
        if (b_log_save) {
            log_manager.onPrepareLogFile("TestLog", log_manager.FILE_CSV);
        }

        ret = native_lib.onStartProductionTest(true, false);
        if (!ret) {
            runOnUiThread(new Runnable() {
                public void run(){
                    onShowErrorDialog("Fail to start production testing\n");
                }
            });
            Log.e(SYNA_TAG, "ActivityProductionTester doProductionTest()" +
                    " fail to start production testing" );

            if (b_log_save) {
                log_manager.onAddErrorMessages("Error: Fail to start production testing",
                        native_lib);
                /* create a log file with errors only */
                ret = log_manager.onCreateLogFile();
                if (!ret) {
                    Log.e(SYNA_TAG, "ActivityProductionTester doProductionTest() " +
                            "fail to create the log file" );
                }
            }
            return;
        }

        b_running = true;

        /* go through every test items perform its test procedure */
        for (int item = 0; item < test.length; item++) {

            if (test[item].is_enabled) {

                StringBuilder sb = new StringBuilder();

                /* if the process is terminated */
                if (!b_running) {
                    /* generate the error message as well */
                    sb.append(String.format(Locale.getDefault(), "test item = %s\n",
                            test[item].getTestName()));
                    sb.append("result = fail, terminated\n\n");

                    if (b_log_save) {
                        log_manager.onAddLogData(sb.toString());
                    }
                    continue;
                }

                /* run the selected production testing */
                if (item == TEST_EX_HIGH_RESISTANCE_PT05)
                    ret = test[item].doTestExHighResistance(native_lib, sb);
                else
                    ret = test[item].doTest(native_lib, sb);

                /* show test result           */
                /* true = pass , false = fail */
                test[item].is_test_succeed = ret;

                /* save testing data into the file_manager */
                if (b_log_save) {
                    log_manager.onAddLogData(sb.toString());
                }

            } // end if (t.is_enabled)
        }

        b_running = false;

        ret = native_lib.onStopProductionTest();
        if (!ret) {
            Log.e(SYNA_TAG, "ActivityProductionTester doProductionTest()" +
                    " fail to stop testing" );
        }

        /* attach the result string */
        for (ProductionTest t : test) {
            if (t.is_enabled) {
                ui_result = t.ui_text_result;

                if (t.is_test_succeed) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            ui_result.setText(R.string.tester_pass);
                            ui_result.setTextColor(Color.GREEN);

                            synchronized(ui_sync){ui_sync.notify();}
                        }
                    });
                }
                else {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            ui_result.setText(R.string.tester_fail);
                            ui_result.setTextColor(Color.RED);

                            synchronized(ui_sync){ui_sync.notify();}
                        }
                    });
                }

                try{
                    synchronized(ui_sync){ui_sync.wait();}
                }catch(InterruptedException ignored){}
            }
        }

        /* create a log file */
        if (b_log_save) {
            log_manager.onCreateLogFile();
            runOnUiThread(new Runnable() {
                public void run(){
                    onShowToastMsg("Log file is saved,\n" + log_manager.onGetLogFileName());
                }
            });
        }

    }/* end doProductionTest() */

    TextView ui_result;
    private final Object ui_sync = new Object();

    /********************************************************
     * function to initialize the test configuration (.ini) file
     * being the entry function to ensure whether the .ini file exist
     * and call function to parse the content as well
     ********************************************************/
    private void onInitTestIniInput() {

        boolean ret = test_cfg_manager.onCheckTestCfgFile(str_test_cfg_file);

        /* if file is valid, parse the test limit */
        if (ret) {
            StringBuilder err = new StringBuilder();
            ret = onLoadTestConfiguration(str_test_cfg_file, err);
            if (!ret) {
                String str = "Fail to parse " + str_test_cfg_file + ".\n\n" + err.toString();
                onShowWarningDialog(str);
                onClearTestIniInput();
            }
        }
        /*  otherwise, show warning message and disable the limit from the .ini file */
        else {
            String str = str_test_cfg_file + " is not found. Disable the test config input.";
            onShowWarningDialog(str);

            TextView text_cfg_msg = findViewById(R.id.test_cfg_msg);
            text_cfg_msg.setVisibility(View.VISIBLE);

            onClearTestIniInput();
        }
    } /* end onInitTestIniInput() */

    /********************************************************
     * function to clear all limits from the external test configuration file
     ********************************************************/
    private void onClearTestIniInput() {
        for (ProductionTest t : test) {
            t.onResetLimit();
        }
    }

    /********************************************************
     * function to load the external test configuration file
     * to skip the line if '#' existed
     * then, parse the value following by the known key word
     ********************************************************/
    private boolean onLoadTestConfiguration(String str_file, StringBuilder err) {
        Log.i(SYNA_TAG, "ActivityProductionTester onLoadTestConfiguration() + ");

        onClearTestIniInput();

        int row = native_lib.getDevImageRow(true);
        int col = native_lib.getDevImageCol(true);

        /* parse the entire configuration file */
        boolean ret = test_cfg_manager.onParseTestConfigurationFile(str_file, row, col, err);
        if (!ret) {
            Log.e(SYNA_TAG, "ActivityProductionTester onLoadTestConfigurationFile() " +
                    "fail to parse cfg file, " + str_file);
            return false;
        }

        /* retrieve the limit for each testing items */
        for (ProductionTest t : test) {
            t.onParseLimitFromTestCfg(test_cfg_manager, row, col, err, native_lib);
        }

        return true;
    } /* end onLoadTestConfiguration() */

    /********************************************************
     * function to save preferences
     ********************************************************/
    public void onSavePreferences() {
        Log.i(SYNA_TAG, "ActivityProductionTester onSavePreferences() + " );

        for (ProductionTest t : test) {
            if (t.ui_cbtn.isChecked()) {

                int min = 0;
                int max = 0;

                if (t.ui_edit_limit_min != null)
                    min = Integer.parseInt(t.ui_edit_limit_min.getText().toString());

                if (t.ui_edit_limit_max != null)
                    max = Integer.parseInt(t.ui_edit_limit_max.getText().toString());

                t.onSetLimit(min, max);
            }
        }

        SharedPreferences preference =
                getSharedPreferences("SynaAPKPreference_Tester", Context.MODE_PRIVATE);
        preference.edit()
                // parameter for test configuration
                .putString(STR_TEST_CFG_EN, str_test_cfg_en)
                .putString(STR_TEST_CFG_FILE, str_test_cfg_file)
                // parameter for Noise test
                .putBoolean(STR_TEST_NOISE_EN, test[TEST_NOISE].is_enabled)
                .putInt(STR_TEST_NOISE_LIMIT_1, test[TEST_NOISE].onGetLimitMax())
                .putInt(STR_TEST_NOISE_LIMIT_2, test[TEST_NOISE].onGetLimitMin())
                // parameter for Full Raw or DRT test
                .putBoolean(STR_TEST_FULL_RAW_OR_DRT_EN, test[TEST_FULL_RAW_OR_DRT].is_enabled)
                .putInt(STR_TEST_FULL_RAW_OR_DRT_LIMIT_1, test[TEST_FULL_RAW_OR_DRT].onGetLimitMin())
                .putInt(STR_TEST_FULL_RAW_OR_DRT_LIMIT_2, test[TEST_FULL_RAW_OR_DRT].onGetLimitMax())
                // parameter for tcm trx trx short test pt01
                .putBoolean(STR_TEST_TRX_TRX_SHORT_PT01_EN, test[TEST_TRX_TRX_SHORT_PT01].is_enabled)
                // parameter for tcm trx ground short pt03
                .putBoolean(STR_TEST_TRX_GROUND_PT03_EN, test[TEST_TRX_GROUND_PT03].is_enabled)
                // parameter for tcm adc range pt11
                .putBoolean(STR_TEST_ADC_RANGE_PT11_EN, test[TEST_ADC_RANGE_PT11].is_enabled)
                .putInt(STR_TEST_ADC_RANGE_PT11_LIMIT_1, test[TEST_ADC_RANGE_PT11].onGetLimitMin())
                .putInt(STR_TEST_ADC_RANGE_PT11_LIMIT_2, test[TEST_ADC_RANGE_PT11].onGetLimitMax())
                // parameter for tcm abs raw cap pt12
                .putBoolean(STR_TEST_ABS_RAW_CAP_PT12_EN, test[TEST_ABS_RAW_CAP_PT12].is_enabled)
                .putInt(STR_TEST_ABS_RAW_CAP_PT12_LIMIT_1, test[TEST_ABS_RAW_CAP_PT12].onGetLimitMin())
                .putInt(STR_TEST_ABS_RAW_CAP_PT12_LIMIT_2, test[TEST_ABS_RAW_CAP_PT12].onGetLimitMax())
                // parameter for tcm hybrid abs noise pt1d
                .putBoolean(STR_TEST_HYBRID_ABS_NOISE_PT1D_EN, test[TEST_HYBRID_ABS_NOISE_PT1D].is_enabled)
                .putInt(STR_TEST_HYBRID_ABS_NOISE_PT1D_LIMIT_1, test[TEST_HYBRID_ABS_NOISE_PT1D].onGetLimitMin())
                .putInt(STR_TEST_HYBRID_ABS_NOISE_PT1D_LIMIT_2, test[TEST_HYBRID_ABS_NOISE_PT1D].onGetLimitMax())
                // parameter for tcm extended high resistance pt05
                .putBoolean(STR_TEST_EX_HIGH_RESISTANCE_PT05_EN, test[TEST_EX_HIGH_RESISTANCE_PT05].is_enabled)
                .putInt(STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_TIXEL,
                        test[TEST_EX_HIGH_RESISTANCE_PT05].onGetLimitTixel())
                .putInt(STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_RXROE,
                        test[TEST_EX_HIGH_RESISTANCE_PT05].onGetLimitRxRoe())
                .putInt(STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_TXROE,
                        test[TEST_EX_HIGH_RESISTANCE_PT05].onGetLimitTxRoe())
                .apply();
    } /* end onSavePreferences() */

    /********************************************************
     * function to load preferences
     ********************************************************/
    private void onLoadPreferences() {
        Log.i(SYNA_TAG, "ActivityProductionTester onLoadPreferences() + " );

        SharedPreferences preference =
                getSharedPreferences("SynaAPKPreference_Tester", Context.MODE_PRIVATE);
        int min = 0;
        int max = 0;
        boolean en = false;

        for (int item = 0; item < test.length; item++) {
            switch (item) {

                case TEST_NOISE:
                    en  = preference.getBoolean(STR_TEST_NOISE_EN, false);
                    min = preference.getInt(STR_TEST_NOISE_LIMIT_2, 0);
                    max = preference.getInt(STR_TEST_NOISE_LIMIT_1,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_NOISE_MAX);
                    break;

                case TEST_FULL_RAW_OR_DRT:
                    en  = preference.getBoolean(STR_TEST_FULL_RAW_OR_DRT_EN, false);
                    min = preference.getInt(STR_TEST_FULL_RAW_OR_DRT_LIMIT_1,
                            (native_lib.isDevTCM())?
                                    ((native_lib.isDevTCMTDDI())?
                                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_DRT_MIN :
                                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_FULL_RAW_PT05_MIN)
                                    : ProductionTestItems.DEFAULT_TEST_LIMIT_RMI_FULL_RAW_MIN);
                    max = preference.getInt(STR_TEST_FULL_RAW_OR_DRT_LIMIT_2,
                            (native_lib.isDevTCM())?
                                    ((native_lib.isDevTCMTDDI())?
                                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_DRT_MAX :
                                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_FULL_RAW_PT05_MAX)
                                    : ProductionTestItems.DEFAULT_TEST_LIMIT_RMI_FULL_RAW_MAX);
                    break;

                case TEST_TRX_TRX_SHORT_PT01:
                    en  = preference.getBoolean(STR_TEST_TRX_TRX_SHORT_PT01_EN, false);
                    min = 0;
                    max = 0;
                    break;

                case TEST_TRX_GROUND_PT03:
                    en  = preference.getBoolean(STR_TEST_TRX_GROUND_PT03_EN, false);
                    min = 0;
                    max = 0;
                    break;

                case TEST_ADC_RANGE_PT11:
                    en  = preference.getBoolean(STR_TEST_ADC_RANGE_PT11_EN, false);
                    min = preference.getInt(STR_TEST_ADC_RANGE_PT11_LIMIT_1,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_ADC_RANGE_PT11_MIN);
                    max = preference.getInt(STR_TEST_ADC_RANGE_PT11_LIMIT_2,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_ADC_RANGE_PT11_MAX);
                    break;

                case TEST_ABS_RAW_CAP_PT12:
                    en  = preference.getBoolean(STR_TEST_ABS_RAW_CAP_PT12_EN, false);
                    min = preference.getInt(STR_TEST_ABS_RAW_CAP_PT12_LIMIT_1,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_ABS_RAW_CAP_PT12_MIN);
                    max = preference.getInt(STR_TEST_ABS_RAW_CAP_PT12_LIMIT_2,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_ABS_RAW_CAP_PT12_MAX);
                    break;

                case TEST_HYBRID_ABS_NOISE_PT1D:
                    en  = preference.getBoolean(STR_TEST_HYBRID_ABS_NOISE_PT1D_EN, false);
                    min = preference.getInt(STR_TEST_HYBRID_ABS_NOISE_PT1D_LIMIT_1,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_HYBRID_ABS_NOISE_PT1D_MIN);
                    max = preference.getInt(STR_TEST_HYBRID_ABS_NOISE_PT1D_LIMIT_2,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_HYBRID_ABS_NOISE_PT1D_MAX);
                    break;

                case TEST_EX_HIGH_RESISTANCE_PT05:
                    en  = preference.getBoolean(STR_TEST_EX_HIGH_RESISTANCE_PT05_EN, false);
                    min = 0;
                    max = 0;

                    int limit_tixel = preference.getInt(STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_TIXEL,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_EX_HIGH_RESISTANCE_TIXEL);
                    int limit_rxroe = preference.getInt(STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_RXROE,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_EX_HIGH_RESISTANCE_RXROE);
                    int limit_txroe = preference.getInt(STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_TXROE,
                            ProductionTestItems.DEFAULT_TEST_LIMIT_TCM_EX_HIGH_RESISTANCE_TXROE);

                    test[item].onSetLimitExHighResistance(limit_tixel, limit_txroe, limit_rxroe);

                    break;
            }

            /* configure the test limit */
            test[item].is_enabled = en;
            test[item].onSetLimit(min, max);
        }

    } /* end onLoadPreferences() */

    private void onLoadPreferencesTestCfg() {
        Log.i(SYNA_TAG, "ActivityProductionTester onLoadPreferencesTestCfg() + ");

        SharedPreferences preference =
                getSharedPreferences("SynaAPKPreference_Tester", Context.MODE_PRIVATE);

        str_test_cfg_en = preference.getString(STR_TEST_CFG_EN, "false");
        str_test_cfg_file = preference.getString(STR_TEST_CFG_FILE, "test_cfg.ini");

        Log.i(SYNA_TAG, "ActivityProductionTester onLoadPreferencesTestCfg() str_test_cfg_en = "
                + str_test_cfg_en + ", str_test_cfg_file = " + str_test_cfg_file);
    }

    /********************************************************
     * function to popup toast message
     ********************************************************/
    private void onShowToastMsg(String msg) {
        Toast toast = Toast.makeText(ActivityProductionTester.this,
                msg, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER, 0, 0);
        toast.show();
    } /* end showToast() */

    /********************************************************
     * create a dialog with warning messages only
     * this function will not terminate the current process
     *******************************************************/
    private void onShowWarningDialog(String msg) {
        new AlertDialog.Builder(ActivityProductionTester.this)
                .setTitle("Warning")
                .setMessage(msg)
                .setPositiveButton("OK", null).show();
    } /* end onShowWarningDialog() */

    /********************************************************
     * create a dialog with message
     *******************************************************/
    private void onShowErrorDialog(String msg) {
        new AlertDialog.Builder(ActivityProductionTester.this)
                .setTitle("Error")
                .setMessage(msg)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        btn_exit.setEnabled(true);
                        btn_start_testing.setEnabled(true);
                    }
                }).show();
    } /* end showErrorDialog() */

    /********************************************************
     * create a dialog with message
     * once clicking 'OK' button, go back to the main page
     *******************************************************/
    private void onShowErrorOut(String msg) {
        new AlertDialog.Builder(ActivityProductionTester.this)
                .setMessage(msg)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Intent intent = new Intent();
                        intent.setClass(ActivityProductionTester.this, MainActivity.class);
                        startActivity(intent);
                        ActivityProductionTester.this.finish();
                    }
                }).show();
    }  /* end onShowErrorOut() */

    private final String STR_TEST_CFG_EN = "TEST_CFG_EN";
    private final String STR_TEST_CFG_FILE = "TEST_CFG_FILE";

    public static final String STR_TEST_NOISE_EN = "TEST_NOISE_EN";
    public static final String STR_TEST_NOISE_LIMIT_1 = "TEST_NOISE_LIMIT_1";  /* max */
    public static final String STR_TEST_NOISE_LIMIT_2 = "TEST_NOISE_LIMIT_2";  /* min */

    public static final String STR_TEST_FULL_RAW_OR_DRT_EN = "TEST_FULL_RAW_OR_DRT_EN";
    public static final String STR_TEST_FULL_RAW_OR_DRT_LIMIT_1 = "TEST_FULL_RAW_OR_DRT_LIMIT_1";  /* min */
    public static final String STR_TEST_FULL_RAW_OR_DRT_LIMIT_2 = "TEST_FULL_RAW_OR_DRT_LIMIT_2";  /* max */

    public static final String STR_TEST_TRX_TRX_SHORT_PT01_EN = "TEST_TRX_TRX_SHORT_PT01_EN";

    public static final String STR_TEST_TRX_GROUND_PT03_EN = "TEST_TRX_GROUND_PT03_EN";

    public static final String STR_TEST_ADC_RANGE_PT11_EN = "TEST_ADC_RANGE_PT11_EN";
    public static final String STR_TEST_ADC_RANGE_PT11_LIMIT_1 = "TEST_ADC_RANGE_PT11_LIMIT_1";  /* min */
    public static final String STR_TEST_ADC_RANGE_PT11_LIMIT_2 = "TEST_ADC_RANGE_PT11_LIMIT_2";  /* max */

    public static final String STR_TEST_ABS_RAW_CAP_PT12_EN = "TEST_ABS_RAW_CAP_PT12_EN";
    public static final String STR_TEST_ABS_RAW_CAP_PT12_LIMIT_1 = "TEST_ABS_RAW_CAP_PT12_LIMIT_1";  /* min */
    public static final String STR_TEST_ABS_RAW_CAP_PT12_LIMIT_2 = "TEST_ABS_RAW_CAP_PT12_LIMIT_2";  /* max */

    public static final String STR_TEST_HYBRID_ABS_NOISE_PT1D_EN = "TEST_HYBRID_ABS_NOISE_PT1D_EN";
    public static final String STR_TEST_HYBRID_ABS_NOISE_PT1D_LIMIT_1 = "TEST_HYBRID_ABS_NOISE_PT1D_LIMIT_1";  /* min */
    public static final String STR_TEST_HYBRID_ABS_NOISE_PT1D_LIMIT_2 = "TEST_HYBRID_ABS_NOISE_PT1D_LIMIT_2";  /* max */

    public static final String STR_TEST_EX_HIGH_RESISTANCE_PT05_EN = "TEST_EX_HIGH_RESISTANCE_PT05_EN";
    public static final String STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_TIXEL = "TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_TIXEL";
    public static final String STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_RXROE = "TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_RXROE";
    public static final String STR_TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_TXROE = "TEST_EX_HIGH_RESISTANCE_PT05_LIMIT_TXROE";

}
