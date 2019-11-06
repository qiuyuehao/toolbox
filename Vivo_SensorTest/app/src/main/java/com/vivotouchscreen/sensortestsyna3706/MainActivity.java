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

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;


public class MainActivity extends AppCompatActivity {

    private final String SYNA_TAG = "syna-apk";

    private final int PST_DELAY_STARTACTIVITY_MS = 100;

    /********************************************************
     * Supported Functions
     ********************************************************/
    private final int BTN_IDENTIFIER       = 0;
    private final int BTN_TOUCH_EXPLORER   = 1;
    private final int BTN_DELTA_IMG_READER = 2;
    private final int BTN_RAW_IMG_READER   = 3;
    private final int BTN_SNR_CALCULATOR   = 4;
    private final int BTN_TEST             = 5;
    private final int BTN_SYNA_RAW_COMMANDS= 6;

    private final int MAX_BTN_COUNT  = 7;
    private Button[] btn_list = new Button[MAX_BTN_COUNT];
    private int n_btn_idx_current;

    /********************************************************
     * Native Wrapper
     ********************************************************/
    public static NativeWrapper native_lib = null;

    /********************************************************
     * handler for log file access
     ********************************************************/
    public static LogFileManager file_manager_lib = null;

    /********************************************************
     * variables of apk version
     ********************************************************/
    private String version_apk;
    /********************************************************
     * Application Configurations
     ********************************************************/
    private boolean b_log_en;
    private String str_log_en;
    private String str_log_path;
    private String str_test_ini_en;
    private String str_test_ini_file;
    private String str_dev_node;
    private String str_dev_rmi_en;
    private String str_dev_tcm_en;

    /********************************************************
     * Called when the activity is first created.
     * This is where developer should do all of normal static set up:
     * create views, bind data to lists, etc.
     *
     * This method also provides developer with a Bundle containing the activity's previously
     * frozen state, if there was one.
     * Always followed by onStart().
     ********************************************************/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* keep screen always on */
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        /* set main content view */
        setContentView(R.layout.activity_main);
        /* get the apk version and change the app tittle */
        try {
            PackageInfo pInfo = this.getPackageManager().getPackageInfo(getPackageName(), 0);
            version_apk = pInfo.versionName;
        }
        catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        Log.i(SYNA_TAG, "MainActivity onCreate() + " );

        Intent in = getIntent();
        if (in != null) {
            /* get initial settings from the intent
               could be the shell input or come from other activity */
            loadIntent(in);
        }
        else {
            /* get initial settings from the preference */
            loadPreferences();
        }

        /* UI components - toolbar */
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        setTitle("SynaTool APK v" + version_apk);

        /* create the object to handle native methods calling */
        native_lib = new NativeWrapper();

        /* create the object to handle log file access */
        file_manager_lib = new LogFile(native_lib, version_apk);

    }

    /********************************************************
     * Called after onCreate(Bundle)
     * initialize the startup settings
     ********************************************************/
    @Override
    public void onStart() {
        Log.i(SYNA_TAG, "MainActivity onStart() + " );

        /* check the status of device node */
        boolean is_found;
        if (str_dev_node.length() == 0) {
            is_found = native_lib.onCheckDev();
        }
        else {
            is_found = native_lib.onSetupDev(str_dev_node, str_dev_rmi_en, str_dev_tcm_en);
        }

        initializeBtn(is_found);

        if (!is_found){ /* if DUT isn't detected, error out */
            showMessage("Fail to find Synaptics device node.\n\n" +
                    "Please go to \"Setting\" page to setup a valid interface," +
                    " and please confirm its permission is proper as well.");
        }
        else {

            /* update the string of valid device node */
            str_dev_node = native_lib.getDevNodeStr();
            if (native_lib.isDevRMI())
                str_dev_rmi_en = "true";
            else if (native_lib.isDevTCM())
                str_dev_tcm_en = "true";

            Log.i(SYNA_TAG, "MainActivity loadIntent() dev_node : "
                    + str_dev_node + ", (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");

            /* ensure the log path is writable; otherwise, set the flag to false */
            if (b_log_en) {
                if (!file_manager_lib.onCheckLogFilePath(str_log_path)) {
                    Log.e(SYNA_TAG, "MainActivity onStart() " + str_log_path + " is not writable");
                    b_log_en = false;
                    str_log_en = "false";
                }
            }
            /* set the latest button selected */
            setBtnFocus(n_btn_idx_current);
        }

        super.onStart();
    }

    /********************************************************
     * Called when this application is no longer visible to the user.
     ********************************************************/
    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "MainActivity onStop() + " );
        /* save preferences before closing the application*/
        savePreferences();
        super.onStop();
    }

    /********************************************************
     * The final call you receive before your activity is destroyed.
     * This can happen either because the activity is finishing on it
     ********************************************************/
    @Override
    public void onDestroy() {
        Log.i(SYNA_TAG, "MainActivity onDestroy() + " );
        /* save preferences before closing the application*/
        savePreferences();
        super.onDestroy();
    }


    /********************************************************
     * function to listen the KeyEvent
     * VOLUME_UP - perform the button clivk
     * VOLUME_DOWN - move the button selection
     ********************************************************/
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch(keyCode) {
            case KeyEvent.KEYCODE_VOLUME_UP:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();
                    /* execute the implemented function */
                    btn_list[n_btn_idx_current].performClick();
                }
                break;
            case KeyEvent.KEYCODE_VOLUME_DOWN:
                if  (event.getRepeatCount() ==  0 ) {
                    event.startTracking();
                    /* go to next button selection */
                    n_btn_idx_current += 1;
                    /* back to the first one when at the end */
                    if (n_btn_idx_current == (MAX_BTN_COUNT))
                        n_btn_idx_current = 0;
                    /* set focus to the selected button */
                    setBtnFocus(n_btn_idx_current);
                }
                break;
        }
        return false;
    }

    /********************************************************
     * auto-created by wizard
     * onCreateOptionsMenu method is responsible for creating a menu.
     * Menu object is passed to this method as a parameter.
     ********************************************************/
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    /********************************************************
     * onOptionsItemSelected method is an activity listener.
     * It receives menu item which has been clicked as a parameter - MenuItem.
     ********************************************************/
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        /* show the APP version */
        if (id == R.id.menu_info) {
            showMessage(" Version : " + version_apk);
            return true;
        }
        /* show the usage */
        else if (id == R.id.menu_usage) {
            showMessage("This application is a toolbox for Synaptics touch controller.\n\n" +
                    "User is able to run particular function by\n" +
                    "    1. clicking the button\n" +
                    "    2. moving the selection with VOL_DOWN\n" +
                    "        and pressing VOL_UP to perform button clicking");
            return true;
        }
        /* configure the device settings */
        else if (id == R.id.menu_settings) {
            /* launch the page for configuration */
            Intent intent = new Intent();
            intent.setClass(MainActivity.this, ActivitySettings.class);
            intent.putExtra(Common.STR_DEV_NODE, str_dev_node);
            intent.putExtra(Common.STR_DEV_RMI_EN, str_dev_rmi_en);
            intent.putExtra(Common.STR_DEV_TCM_EN, str_dev_tcm_en);
            intent.putExtra("CFG_LOG_SAVE", b_log_en);
            intent.putExtra(Common.STR_CFG_LOG_PATH, str_log_path);
            startActivity(intent);

            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    /********************************************************
     * function to initialize all buttons
     ********************************************************/
    private void initializeBtn(boolean is_enabled) {
        /* UI components - button, do identify */
        btn_list[BTN_IDENTIFIER] = findViewById(R.id.btn_main_identify);
        btn_list[BTN_IDENTIFIER].setOnClickListener(_button_listener_do_identify);
        btn_list[BTN_IDENTIFIER].setEnabled(is_enabled);
        ImageView icon_identify = findViewById(R.id.icon_main_identify);
        if (is_enabled) {
            icon_identify.setImageResource(R.drawable.icon_id);
            btn_list[BTN_IDENTIFIER].setTextColor(Color.WHITE);
        }
        else {
            icon_identify.setImageResource(R.drawable.icon_id_2);
            btn_list[BTN_IDENTIFIER].setTextColor(Color.GRAY);
        }
        /* UI components - button, touch explorer */
        btn_list[BTN_TOUCH_EXPLORER] = findViewById(R.id.btn_main_touch_explorer);
        btn_list[BTN_TOUCH_EXPLORER].setOnClickListener(_button_listener_touch_explorer);
        btn_list[BTN_TOUCH_EXPLORER].setEnabled(is_enabled);
        ImageView icon_touch_explorer = findViewById(R.id.icon_main_touch_explorer);
        if (is_enabled) {
            icon_touch_explorer.setImageResource(R.drawable.icon_touch);
            btn_list[BTN_TOUCH_EXPLORER].setTextColor(Color.WHITE);
        }
        else {
            icon_touch_explorer.setImageResource(R.drawable.icon_touch_2);
            btn_list[BTN_TOUCH_EXPLORER].setTextColor(Color.GRAY);
        }
        /* UI components - button, read delta image */
        btn_list[BTN_DELTA_IMG_READER] = findViewById(R.id.btn_main_img_delta);
        btn_list[BTN_DELTA_IMG_READER].setOnClickListener(_button_listener_read_delta);
        btn_list[BTN_DELTA_IMG_READER].setEnabled(is_enabled);
        ImageView icon_2 = findViewById(R.id.icon_main_img_delta);
        if (is_enabled) {
            icon_2.setImageResource(R.drawable.icon_img);
            btn_list[BTN_DELTA_IMG_READER].setTextColor(Color.WHITE);
        }
        else {
            icon_2.setImageResource(R.drawable.icon_img_2);
            btn_list[BTN_DELTA_IMG_READER].setTextColor(Color.GRAY);
        }
        /* UI components - button, read raw image */
        btn_list[BTN_RAW_IMG_READER] = findViewById(R.id.btn_main_img_raw);
        btn_list[BTN_RAW_IMG_READER].setOnClickListener(_button_listener_read_raw);
        btn_list[BTN_RAW_IMG_READER].setEnabled(is_enabled);
        ImageView icon_3 = findViewById(R.id.icon_main_img_raw);
        if (is_enabled) {
            icon_3.setImageResource(R.drawable.icon_img);
            btn_list[BTN_RAW_IMG_READER].setTextColor(Color.WHITE);
        }
        else {
            icon_3.setImageResource(R.drawable.icon_img_2);
            btn_list[BTN_RAW_IMG_READER].setTextColor(Color.GRAY);
        }
        /* UI components - button, calculate SNR */
        btn_list[BTN_SNR_CALCULATOR] = findViewById(R.id.btn_main_img_snr);
        btn_list[BTN_SNR_CALCULATOR].setOnClickListener(_button_listener_cal_snr);
        btn_list[BTN_SNR_CALCULATOR].setEnabled(is_enabled);
        ImageView icon_5 = findViewById(R.id.icon_main_img_snr);
        if (is_enabled) {
            icon_5.setImageResource(R.drawable.icon_img);
            btn_list[BTN_SNR_CALCULATOR].setTextColor(Color.WHITE);
        }
        else {
            icon_5.setImageResource(R.drawable.icon_img_2);
            btn_list[BTN_SNR_CALCULATOR].setTextColor(Color.GRAY);
        }
        /* UI components - button, do production test */
        btn_list[BTN_TEST] = findViewById(R.id.btn_main_production_test);
        btn_list[BTN_TEST].setOnClickListener(_button_listener_testing);
        btn_list[BTN_TEST].setEnabled(is_enabled);
        ImageView icon_4 = findViewById(R.id.icon_main_production_test);
        if (is_enabled) {
            icon_4.setImageResource(R.drawable.icon_test);
            btn_list[BTN_TEST].setTextColor(Color.WHITE);
        }
        else {
            icon_4.setImageResource(R.drawable.icon_test_2);
            btn_list[BTN_TEST].setTextColor(Color.GRAY);
        }

        /* UI components - button, perform raw command mode */
        btn_list[BTN_SYNA_RAW_COMMANDS] = findViewById(R.id.btn_main_raw_comamnds);
        btn_list[BTN_SYNA_RAW_COMMANDS].setOnClickListener(_button_listener_raw_commands);
        btn_list[BTN_SYNA_RAW_COMMANDS].setEnabled(is_enabled);
        ImageView icon_6 = findViewById(R.id.icon_main_raw_comamnds);
        if (is_enabled) {
            icon_6.setImageResource(R.drawable.icon_syna_page);
            btn_list[BTN_SYNA_RAW_COMMANDS].setTextColor(Color.WHITE);
        }
        else {
            icon_6.setImageResource(R.drawable.icon_syna_page_2);
            btn_list[BTN_SYNA_RAW_COMMANDS].setTextColor(Color.GRAY);
        }

		
        /* hide the following functions if this is not formal release version */
        /* raw command access  */
        String str = getApplicationContext().getPackageName();
        if (!str.contains("synaptics.apk")) {
            icon_6.setVisibility(View.INVISIBLE);
            btn_list[BTN_SYNA_RAW_COMMANDS].setEnabled(false);
            btn_list[BTN_SYNA_RAW_COMMANDS].setVisibility(View.INVISIBLE);
        }

    }

    /********************************************************
     * function to change the text color on specified button
     ********************************************************/
    private void setBtnFocus(int focus_idx) {
        for (int i = 0; i < MAX_BTN_COUNT; i++) {
            if (focus_idx == i) {
                btn_list[focus_idx].setTextColor(0xffb2d0b2);
            }
            else {
                btn_list[i].setTextColor(Color.WHITE);
            }
        }
        n_btn_idx_current = focus_idx;
    }

    /********************************************************
     * implementation if the button 'IDENTIFY' is pressed
     * start the activity, ActivityIdentifier
     ********************************************************/
    private View.OnClickListener _button_listener_do_identify = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* set button focus */
            setBtnFocus(BTN_IDENTIFIER);
            /* launch page to perform device identification */
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent();
                    intent.setClass(MainActivity.this, ActivityIdentifier.class);
                    intent.putExtra(Common.STR_DEV_NODE, str_dev_node);
                    intent.putExtra(Common.STR_DEV_RMI_EN, str_dev_rmi_en);
                    intent.putExtra(Common.STR_DEV_TCM_EN, str_dev_tcm_en);
                    startActivity(intent);
                    MainActivity.this.finish();
                }
            }, PST_DELAY_STARTACTIVITY_MS);

        }
    };

    /********************************************************
     * implementation if the button 'IMAGE DIAGNOSTICS - DELTA' is pressed
     * start the activity, ActivityImageLogger
     ********************************************************/
    private View.OnClickListener _button_listener_read_delta = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* set button focus */
            setBtnFocus(BTN_DELTA_IMG_READER);
            /* launch specific page */
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent();
                    intent.setClass(MainActivity.this, ActivityImageLogger.class);
                    intent.putExtra(Common.STR_DEV_NODE, str_dev_node);
                    intent.putExtra(Common.STR_DEV_RMI_EN, str_dev_rmi_en);
                    intent.putExtra(Common.STR_DEV_TCM_EN, str_dev_tcm_en);
                    intent.putExtra(Common.STR_CFG_REPORT_TYPE, Common.STR_REPORT_TYPE_DELTA);
                    intent.putExtra(Common.STR_CFG_LOG_SAVE_EN, str_log_en);
                    intent.putExtra(Common.STR_CFG_LOG_PATH, str_log_path);
                    startActivity(intent);
                    MainActivity.this.finish();
                }
            }, PST_DELAY_STARTACTIVITY_MS);

        }
    };

    /********************************************************
     * implementation if the button 'IMAGE DIAGNOSTICS - RAW' is pressed
     * start the activity, ActivityImageLogger
     ********************************************************/
    private View.OnClickListener _button_listener_read_raw = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* set button focus */
            setBtnFocus(BTN_RAW_IMG_READER);
            /* launch specific page */
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent();
                    intent.setClass(MainActivity.this, ActivityImageLogger.class);
                    intent.putExtra(Common.STR_DEV_NODE, str_dev_node);
                    intent.putExtra(Common.STR_DEV_RMI_EN, str_dev_rmi_en);
                    intent.putExtra(Common.STR_DEV_TCM_EN, str_dev_tcm_en);
                    intent.putExtra(Common.STR_CFG_REPORT_TYPE, Common.STR_REPORT_TYPE_RAW);
                    intent.putExtra(Common.STR_CFG_LOG_SAVE_EN, str_log_en);
                    intent.putExtra(Common.STR_CFG_LOG_PATH, str_log_path);
                    startActivity(intent);
                    MainActivity.this.finish();
                }
            }, PST_DELAY_STARTACTIVITY_MS);

        }
    };

    /********************************************************
     * implementation if the button 'PRODUCTION TEST' is pressed
     * start the activity, ActivityProductionTester
     ********************************************************/
    private View.OnClickListener _button_listener_testing = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* set button focus */
            setBtnFocus(BTN_TEST);
            /* launch specific page */
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent();
                    intent.setClass(MainActivity.this, ActivityProductionTester.class);
                    intent.putExtra(Common.STR_DEV_NODE, str_dev_node);
                    intent.putExtra(Common.STR_DEV_RMI_EN, str_dev_rmi_en);
                    intent.putExtra(Common.STR_DEV_TCM_EN, str_dev_tcm_en);
                    intent.putExtra(Common.STR_CFG_LOG_SAVE_EN, str_log_en);
                    intent.putExtra(Common.STR_CFG_LOG_PATH, str_log_path);
                    intent.putExtra(Common.STR_CFG_TEST_INI_EN, str_test_ini_en);
                    intent.putExtra(Common.STR_CFG_TEST_INI_FILE, str_test_ini_file);
                    startActivity(intent);
                    MainActivity.this.finish();
                }
            }, PST_DELAY_STARTACTIVITY_MS);

        }
    };

    /********************************************************
     * implementation if the button 'TOUCH EXPLORER' is pressed
     * start the activity, ActivityTouchExplorer
     ********************************************************/
    private View.OnClickListener _button_listener_touch_explorer = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* set button focus */
            setBtnFocus(BTN_TOUCH_EXPLORER);
            /* launch specific page */
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent();
                    intent.setClass(MainActivity.this, ActivityTouchExplorer.class);
                    startActivity(intent);
                    MainActivity.this.finish();
                }
            }, PST_DELAY_STARTACTIVITY_MS);
        }
    };

    /********************************************************
     * implementation if the button 'SNR CALCULATOR' is pressed
     * start the activity, ActivitySNRCalculator
     ********************************************************/
    private View.OnClickListener _button_listener_cal_snr = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* set button focus */
            setBtnFocus(BTN_SNR_CALCULATOR);
            /* launch specific page */
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent();
                    intent.setClass(MainActivity.this, ActivitySNRCalculator.class);
                    intent.putExtra(Common.STR_DEV_NODE, str_dev_node);
                    intent.putExtra(Common.STR_DEV_RMI_EN, str_dev_rmi_en);
                    intent.putExtra(Common.STR_DEV_TCM_EN, str_dev_tcm_en);
                    intent.putExtra(Common.STR_CFG_LOG_SAVE_EN, str_log_en);
                    intent.putExtra(Common.STR_CFG_LOG_PATH, str_log_path);
                    startActivity(intent);
                    MainActivity.this.finish();
                }
            }, PST_DELAY_STARTACTIVITY_MS);
        }
    };

    /********************************************************
     * implementation if the button 'COMMANDS HELPER' is pressed
     * start the activity, ActivityRawCmdHelper
     ********************************************************/
    private View.OnClickListener _button_listener_raw_commands = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            /* set button focus */
            setBtnFocus(BTN_SYNA_RAW_COMMANDS);
            /* launch specific page */
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent();
                    intent.setClass(MainActivity.this, ActivityRawCmdHelper.class);
                    intent.putExtra(Common.STR_DEV_NODE, str_dev_node);
                    intent.putExtra(Common.STR_DEV_RMI_EN, str_dev_rmi_en);
                    intent.putExtra(Common.STR_DEV_TCM_EN, str_dev_tcm_en);
                    intent.putExtra(Common.STR_CFG_LOG_SAVE_EN, str_log_en);
                    intent.putExtra(Common.STR_CFG_LOG_PATH, str_log_path);
                    startActivity(intent);
                    MainActivity.this.finish();
                }
            }, PST_DELAY_STARTACTIVITY_MS);
        }
    };

    /********************************************************
     * helper function to output the message to users
     ********************************************************/
    private void showMessage(String msg) {
        AlertDialog.Builder dlg  = new AlertDialog.Builder(this);
        dlg.setMessage(msg);
        dlg.setPositiveButton("OK", null);
        dlg.create().show();
    }

    /********************************************************
     * function to save preferences
     ********************************************************/
    private void savePreferences() {
        Log.i(SYNA_TAG, "MainActivity savePreferences() + " );

        SharedPreferences preference = getSharedPreferences("SynaAPKPreference",
                                                            Context.MODE_PRIVATE);
        preference.edit()
                .putString(Common.STR_DEV_NODE, str_dev_node)
                .putString(Common.STR_DEV_RMI_EN, str_dev_rmi_en)
                .putString(Common.STR_DEV_TCM_EN, str_dev_tcm_en)
                .putInt(STR_BTN_INDEX, n_btn_idx_current)
                .putString(Common.STR_CFG_LOG_SAVE_EN, str_log_en)
                .putString(Common.STR_CFG_LOG_PATH, str_log_path)
                .apply();
    }

    /********************************************************
     * function to save preferences
     ********************************************************/
    private void loadPreferences() {
        Log.i(SYNA_TAG, "MainActivity loadPreferences() + " );

        String defaul_log_folder = Environment.getExternalStorageDirectory().getPath();

        SharedPreferences preference = getSharedPreferences("SynaAPKPreference",
                                                            Context.MODE_PRIVATE);

        str_dev_node = preference.getString(Common.STR_DEV_NODE, "");
        str_dev_rmi_en = preference.getString(Common.STR_DEV_RMI_EN, "false");
        str_dev_tcm_en = preference.getString(Common.STR_DEV_TCM_EN, "false");

        n_btn_idx_current = preference.getInt(STR_BTN_INDEX, 0);

        str_log_en = preference.getString(Common.STR_CFG_LOG_SAVE_EN, "true");
        str_log_path = preference.getString(Common.STR_CFG_LOG_PATH, defaul_log_folder);
        str_test_ini_en = "";
        str_test_ini_file = "";

        if (str_log_en != null) {
            try {
                b_log_en = Boolean.valueOf(str_log_en);
            } catch (NumberFormatException e) {
                Log.e(SYNA_TAG, "MainActivity onParameterInit() fail to convert str_log_en" );
                b_log_en = false;
            }
        }

        Log.i(SYNA_TAG, "MainActivity loadPreferences() dev_node : "
                + str_dev_node + ", (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");
        Log.i(SYNA_TAG, "MainActivity loadPreferences() log path : "
                + str_log_path + ", (" + str_log_en + ")");
        Log.i(SYNA_TAG, "MainActivity loadPreferences() test ini : "
                + str_test_ini_file + ", (" + str_test_ini_en + ")");

    } /* end loadPreferences() */

    /********************************************************
     * helper function to initialize the parameters via intent
     * it could be the shell input or come from other activity
     ********************************************************/
    public void loadIntent(Intent in) {

        String defaul_log_folder = Environment.getExternalStorageDirectory().getPath();

        SharedPreferences preference = getSharedPreferences("SynaAPKPreference",
                Context.MODE_PRIVATE);


        str_dev_node = in.getStringExtra(Common.STR_DEV_NODE);
        if (str_dev_node == null) {
            str_dev_node = preference.getString(Common.STR_DEV_NODE, "");
        }

        str_dev_rmi_en = in.getStringExtra(Common.STR_DEV_RMI_EN);
        if (str_dev_rmi_en == null) {
            str_dev_rmi_en = preference.getString(Common.STR_DEV_RMI_EN, "false");
        }

        str_dev_tcm_en = in.getStringExtra(Common.STR_DEV_TCM_EN);
        if (str_dev_tcm_en == null) {
            str_dev_tcm_en = preference.getString(Common.STR_DEV_TCM_EN, "false");
        }

        str_log_en = in.getStringExtra(Common.STR_CFG_LOG_SAVE_EN);
        if (str_log_en == null) {
            str_log_en = preference.getString(Common.STR_CFG_LOG_SAVE_EN, "true");
        }

        str_log_path = in.getStringExtra(Common.STR_CFG_LOG_PATH);
        if (str_log_path == null){
            str_log_path = preference.getString(Common.STR_CFG_LOG_PATH, defaul_log_folder);
        }

        /* to get the configuration to assign an external test configuration */
        /* don't need to read from preference because the tester page will handle it */
        str_test_ini_en = in.getStringExtra(Common.STR_CFG_TEST_INI_EN);

        /* to get the configuration of target test configuration file */
        /* don't need to read from preference because the tester page will handle it */
        str_test_ini_file = in.getStringExtra(Common.STR_CFG_TEST_INI_FILE);

        n_btn_idx_current = preference.getInt(STR_BTN_INDEX, 0);

        if (str_log_en != null) {
            try {
                b_log_en = Boolean.valueOf(str_log_en);
            } catch (NumberFormatException e) {
                Log.e(SYNA_TAG, "MainActivity loadIntent() fail to convert str_log_en" );
                b_log_en = false;
            }
        }

        Log.i(SYNA_TAG, "MainActivity loadIntent() dev_node : "
                + str_dev_node + ", (rmi: " + str_dev_rmi_en + ") (tcm: " + str_dev_tcm_en + ")");
        Log.i(SYNA_TAG, "MainActivity loadIntent() log path : "
                + str_log_path + ", (" + str_log_en + ")");
        Log.i(SYNA_TAG, "MainActivity loadIntent() test ini : "
                + str_test_ini_file + ", (" + str_test_ini_en + ")");

        /* save into the preference if it is loaded from shell */
        savePreferences();

    } /* end loadIntent() */

    private final String STR_BTN_INDEX = "BTN_IDEX";

}
