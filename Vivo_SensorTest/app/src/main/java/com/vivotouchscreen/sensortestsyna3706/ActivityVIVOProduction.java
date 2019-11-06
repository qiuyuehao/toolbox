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

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.WindowManager;
import android.widget.TextView;
import java.io.*;
import java.util.Locale;

import static java.lang.Thread.sleep;

public class ActivityVIVOProduction extends Activity {

    private final String SYNA_TAG = "syna-apk";
    private final String DEFAULT_VIVO_FOLDER = "/sdcard/Android/data/com.touchscreen.tptest/";
    private final String DEFAULT_VIVO_TEST_INI = "/system/etc/syna_test_cfg_3618f.ini";

    /********************************************************
     * testing items listed on UI
     ********************************************************/
    public static final int TEST_NOISE              = 0;
    public static final int TEST_FULL_RAW_OR_DRT    = 1;
    public static final int TEST_EX_HIGH_RESISTANCE = 2;
    public static final int TEST_EX_TRX_SHORT       = 3;
    public static final int TEST_ABS_OPEN           = 4;
    public static final int TEST_ADC_RANGE          = 5;
    public static final int TEST_SENSOR_SPEED       = 6;
    public static final int TEST_TAGSMOISTURE       = 7;
    public static final int TEST_RT133              = 8;
    public static final int TEST_ABS_DELTA          = 9;

    public static final int TOTAL_ITEMS = 10;

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

    boolean test_pass_vivo_production_test;
    boolean[] test_pass = new boolean[TOTAL_ITEMS];

    /********************************************************
     * UI Components
     ********************************************************/
    TextView ui_background;
    TextView ui_messagae;
    TextView ui_information;
    TextView ui_version;
    /********************************************************
     * variables of apk version
     ********************************************************/
    private String version_apk;
    /********************************************************
     * flag to save the status of process
     ********************************************************/
    private boolean b_running;
    /********************************************************
     * variables for log saving and external test_cfg file
     ********************************************************/
    private boolean b_log_save;

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
        setContentView(R.layout.page_tester_factory_vivo);

        Log.i(SYNA_TAG, "ActivityVIVOProduction onCreate() + " );

        /* get the apk version and change the app tittle*/
        try {
            PackageInfo pInfo = this.getPackageManager().getPackageInfo(getPackageName(), 0);
            version_apk = pInfo.versionName;
        }
        catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        /* create an object to handle all production tests */
        for (int i = 0; i < test.length; i++)
            test[i] = new ProductionTest();

        /* create an object to handle native methods calling */
        native_lib = new NativeWrapper();

        /* create an object to handle log file access */
        test_cfg_manager = new TestCfgFileManager();


        /* initialize ui components */
        ui_background = findViewById(R.id.text_vivo_test_background);
        ui_background.setBackgroundColor(Color.WHITE);
        ui_messagae = findViewById(R.id.text_vivo_test_msg);
        ui_information = findViewById(R.id.text_vivo_test_info);
        ui_version = findViewById(R.id.text_vivo_version);
        ui_version.append(version_apk);

        /* get parameters from shell */
        Intent intent = this.getIntent();
        String str_log_path = intent.getStringExtra("CFG_LOG_PATH");
        if (str_log_path == null)
            str_log_path = DEFAULT_VIVO_FOLDER;
        String str_test_cfg_file = intent.getStringExtra("CFG_TEST_INI_FILE");
        if (str_test_cfg_file == null)
            str_test_cfg_file = DEFAULT_VIVO_TEST_INI;

        Log.i(SYNA_TAG, "ActivityVIVOProduction onCreate() str_log_path =  " + str_log_path );
        Log.i(SYNA_TAG, "ActivityVIVOProduction onCreate() str_test_cfg_file =  " + str_test_cfg_file );

        b_log_save = true;

        /* do identification at first in order to get the essential information */
        boolean is_syna_dev_found;
        is_syna_dev_found = native_lib.onCheckDev();
        if (!is_syna_dev_found) {
            Log.e(SYNA_TAG, "ActivityVIVOProduction onCreate() fail to find the device");
        }
        else {
            is_syna_dev_found = native_lib.onIdentifyDev(null);
            if (!is_syna_dev_found) {
                Log.e(SYNA_TAG, "ActivityVIVOProduction onCreate() fail to do identification");
            }
        }

        /* initialize all test items */
        onInitTestItems();

        /* create an object to handle log file access */
        log_manager = new LogTestData(native_lib, version_apk);

        /* load tes limit from an external configuration file */
        boolean is_test_ini_existed = false;
        if (test_cfg_manager.onCheckTestCfgFile(str_test_cfg_file)) {
            // parse the test limit
            StringBuilder err = new StringBuilder();
            is_test_ini_existed = onLoadTestConfiguration(str_test_cfg_file, err);
            if (!is_test_ini_existed) {
                String str = "Fail to parse " + str_test_cfg_file + ".\n\n" + err.toString();
                Log.e(SYNA_TAG, "ActivityVIVOProduction onCreate() " + str );
                for (int i = 0; i < TOTAL_ITEMS; i++)
                    test[i].onClearFlagFromTestCfg();
            }
        }

        /* check the specific vivo folder is writable */
        boolean is_path_accessible;
        File f = new File(str_log_path);
        if (!f.exists()) {  // if no exist, try to create the folder
            boolean ret = f.mkdirs();
            if (!ret) {
                Log.e(SYNA_TAG, "ActivityVIVOProduction onCreate() fail to " +
                        "create vivo folder, " + str_log_path);
            }
            ret = f.setWritable(true, false);
            if (!ret) {
                Log.e(SYNA_TAG, "ActivityVIVOProduction onCreate() fail to " +
                        "configure vivo folder into writable, " + str_log_path);
            }
            ret = f.setReadable(true, false);
            if (!ret) {
                Log.e(SYNA_TAG, "ActivityVIVOProduction onCreate() fail to " +
                        "configure vivo folder into readable, " + str_log_path);
            }
        }

        Log.i(SYNA_TAG, "ActivityVIVOProduction onCreate() str_log_path = " + str_log_path);
        is_path_accessible = log_manager.onCheckLogFilePath(str_log_path);
        if (!is_path_accessible) {
            String str = "Fail to access " + str_log_path;
            Log.e(SYNA_TAG, "ActivityVIVOProduction onCreate() " + str );
        }

        /* check the status of device node */
        if (!is_syna_dev_found) {
            ui_messagae.setText(getString(R.string.tester_fail));
            ui_background.setBackgroundColor(Color.RED);
            ui_information.append("Error: fail to identify syna device\n");
        }
        if (!is_test_ini_existed) {
            ui_messagae.setText(getString(R.string.tester_fail));
            ui_background.setBackgroundColor(Color.RED);
            ui_information.append("Error: fail to test configuration file is invalid, " +
                    str_test_cfg_file + "\n");
        }
        if (!is_path_accessible) {
            ui_messagae.setText(getString(R.string.tester_fail));
            ui_background.setBackgroundColor(Color.RED);
            ui_information.append("Error: fail to log path is not accessible, " +
                    str_log_path + "\n");
        }

        b_running = false;

        if (is_syna_dev_found && is_test_ini_existed && is_path_accessible) {

            test[TEST_NOISE].is_enabled = true;
            test[TEST_FULL_RAW_OR_DRT].is_enabled = true;
            test[TEST_EX_HIGH_RESISTANCE].is_enabled = true;
            test[TEST_EX_TRX_SHORT].is_enabled = true;
            test[TEST_ABS_OPEN].is_enabled = true;
            test[TEST_ADC_RANGE].is_enabled = false;
            test[TEST_SENSOR_SPEED].is_enabled = false;
            test[TEST_TAGSMOISTURE].is_enabled = false;
            test[TEST_RT133].is_enabled = true;
            test[TEST_ABS_DELTA].is_enabled = false;

            ui_messagae.setText(getString(R.string.vivo_factory_msg));

            final Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {

                    new Thread(new Runnable() {
                        public void run() {

                            doProductionTest();

                        } /* end of run() */
                    }).start(); /* start the Thread */

                }
            }, 500);

        }

    } /* end onCreate() */

    /********************************************************
     * called when this application is closed
     ********************************************************/
    @Override
    public void onStop() {
        Log.i(SYNA_TAG, "ActivityVIVOProduction onStop() + " );
        if (b_running) {
            b_running = false;
        }

        super.onStop();
    } /* end onStop() */

    /********************************************************
     * function to initialize all test items on ui
     ********************************************************/
    private void onInitTestItems() {

        for (int item = 0; item < test.length; item++) {
            switch (item) {

                /* Noise Test */
                case TEST_NOISE:
                    if (native_lib.isDevRMI()) {
                        test[item].id = ProductionTestItems.TEST_RMI_NOISE_RT02;
                    }
                    else if (native_lib.isDevTCM()) {
                        test[item].id = ProductionTestItems.TEST_TCM_NOISE_PID0A;
                    }
                    break;

                /* Dynamic Range Test, for TCM TDDI device */
                /* Full Raw Test,      for TCM device      */
                /* TDDI Full Raw Test, for RMI TDDI device */
                /* Full Raw Test,      for RMI device      */
                case TEST_FULL_RAW_OR_DRT:
                    if (native_lib.isDevRMI()) {
                        if (native_lib.isDevRMITDDI()) {
                            test[item].id = ProductionTestItems.TEST_RMI_TDDI_FULL_RAW_RT92;
                        }
                        else {
                            test[item].id = ProductionTestItems.TEST_RMI_FULL_RAW_RT20;
                        }
                    }
                    else if (native_lib.isDevTCM()) {
                        if (native_lib.isDevTCMTDDI()) {
                            test[item].id = ProductionTestItems.TEST_TCM_DRT_PID07;
                        }
                        else {
                            test[item].id = ProductionTestItems.TEST_TCM_FULL_RAW_PID05;
                        }
                    }
                    break;

                /* Extended High Resistance Test (RT20) */
                case TEST_EX_HIGH_RESISTANCE:
                    if (native_lib.isDevRMI() && !native_lib.isDevRMITDDI()) {

                        test[item].id = ProductionTestItems.TEST_RMI_EX_HIGH_RESISTANCE_RT20;
                    }
                    break;

                /* Extended TRX Short Test (RT26 + RT100) */
                case TEST_EX_TRX_SHORT:
                    if (native_lib.isDevRMI() && !native_lib.isDevRMITDDI()) {

                        test[item].id = ProductionTestItems.TEST_RMI_EX_TRX_SHORT_RT26100;
                    }
                    break;

                /* Abs Sensing Open Test (RT63) */
                case TEST_ABS_OPEN:
                    if (native_lib.isDevRMI() && !native_lib.isDevRMITDDI()) {

                        test[item].id = ProductionTestItems.TEST_RMI_ABS_OPEN_RT63;
                    }

                    break;

                /* ADC Range Test (RT23) */
                case TEST_ADC_RANGE:

                    if (native_lib.isDevRMI() && !native_lib.isDevRMITDDI()) {

                        test[item].id = ProductionTestItems.TEST_RMI_ADC_RANGE_RT23;
                    }
                    break;

                /* Sensor Speed Test (RT22) */
                case TEST_SENSOR_SPEED:

                    if (native_lib.isDevRMI() && !native_lib.isDevRMITDDI()) {

                        test[item].id = ProductionTestItems.TEST_RMI_SENSOR_SPEED_RT22;
                    }
                    break;

                /* TagsMoisture Test (RT76) */
                case TEST_TAGSMOISTURE:

                    if (native_lib.isDevRMI() && !native_lib.isDevRMITDDI()) {

                        test[item].id = ProductionTestItems.TEST_RMI_TAGSMOISTURE_RT76;
                    }
                    break;

                /* RT133 Short Test (RT133) */
                case TEST_RT133:

                    if (native_lib.isDevRMI() && !native_lib.isDevRMITDDI()) {

                        test[item].id = ProductionTestItems.TEST_RMI_RT133;
                    }
                    break;

                /* Abs Sensing Delta Test (RT59) */
                case TEST_ABS_DELTA:
                    if (native_lib.isDevRMI() && !native_lib.isDevRMITDDI()) {

                        test[item].id = ProductionTestItems.TEST_RMI_ABS_DELTA_RT59;
                    }
                    break;
            } /* end switch */
        } /* end for loop */

    } /* end onInitTestItems() */


    /********************************************************
     * function to do production test
     * will run every selected testing items
     * and, save to the log file if it is enabled
     ********************************************************/
    public void doProductionTest() {
        Log.i(SYNA_TAG, "ActivityVIVOProduction doProductionTest() + ");
        boolean ret;
        boolean wrong_ini = false;

        test_pass_vivo_production_test = false;

        for (int item = 0; item < TOTAL_ITEMS; item++)
            test_pass[item] = false;

        for (int item = 0; item< TOTAL_ITEMS; item++) {
            if (!test[item].onHasLimitFromTestCfg() && test[item].is_enabled) {
                wrong_ini = true;
                String err =
                        String.format(Locale.getDefault(), "Error: invalid test limit in %s",
                                test[item].getTestName());
                Log.e(SYNA_TAG, "ActivityVIVOProduction doProductionTest() " + err);
            }
        }

        if ( wrong_ini ) {
            runOnUiThread(new Runnable() {
                public void run() {
                    ui_messagae.setText(getString(R.string.tester_fail));
                    ui_background.setBackgroundColor(Color.RED);
                }
            });
            return;
        }

        /* prepare the log file */
        if (b_log_save) {
            log_manager.onPrepareLogFile("TestLog", log_manager.FILE_CSV);
        }

        ret = native_lib.onStartProductionTest(true, false);
        if (!ret) {
            Log.e(SYNA_TAG, "ActivityVIVOProduction doProductionTest() fail to open syna device" );
            runOnUiThread(new Runnable() {
                public void run() {
                    ui_messagae.setText(getString(R.string.tester_fail));
                    ui_background.setBackgroundColor(Color.RED);
                    ui_information.append("Error: fail to open syna device\n");
                }
            });
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
                if (item == TEST_EX_HIGH_RESISTANCE)
                    ret = test[item].doTestExHighResistance(native_lib, sb);
                else if (item == TEST_EX_TRX_SHORT)
                    ret = test[item].doTestExTRxShort(native_lib, sb);
                else
                    ret = test[item].doTest(native_lib, sb);

                /* show test result           */
                /* true = pass , false = fail */
                if (ret) {
                    result_str = String.format(Locale.getDefault(), "- %s : Pass\n",
                            test[item].getTestName());
                    test_pass[item] = true;
                } else {
                    result_str = String.format(Locale.getDefault(), "- %s : Fail\n",
                            test[item].getTestName());
                    test_pass[item] = false;
                }

                /* show test result */
                runOnUiThread(new Runnable() {
                    public void run() {
                        ui_information.append(result_str);
                    }
                });

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

        /* create a log file */
        if (b_log_save) {
            log_manager.onCreateLogFile();

            runOnUiThread(new Runnable() {
                public void run() {
                    String msg = "\n\nInfo: Log file is saved, " + log_manager.onGetLogFileName();
                    ui_information.append(msg);
                }
            });

        }

        /* output the final result */
        test_pass_vivo_production_test = true;
        for (int item = 0; item < TOTAL_ITEMS; item++) {
            if (test[item].is_enabled) {
                test_pass_vivo_production_test =
                        test_pass_vivo_production_test && test_pass[item];
            }
        }
        Log.i(SYNA_TAG, "ActivityProductionTester doProductionTest() vivo_production_test = " +
                test_pass_vivo_production_test );

        if (test_pass_vivo_production_test) {

            runOnUiThread(new Runnable() {
                public void run() {
                    ui_messagae.setText(getString(R.string.tester_pass));
                    ui_background.setBackgroundColor(Color.GREEN);
                }
            });

        }
        else {

            runOnUiThread(new Runnable() {
                public void run() {
                    ui_messagae.setText(getString(R.string.tester_fail));
                    ui_background.setBackgroundColor(Color.RED);
                }
            });

        }
        try {
            sleep(3000);
        } catch (InterruptedException ignored) {}

        /* close activity  */
        closeVivoFactory();
    }

    String result_str;


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
     * function to close the activity
     * action = com.vivotouchscreen.sensortestsyna3706
     * key = com.vivotouchscreen.sensortestsyna3706
     * value = "0" - pass / "1" - fail
     ********************************************************/
    private void closeVivoFactory() {
        Log.i(SYNA_TAG, "colseVivoFactory" );
        String result;
        Intent intent = new Intent();
        intent.setAction("com.vivotouchscreen.sensortestsyna3706");
        if (test_pass_vivo_production_test) {
            result = "0";
        }
        else {
            result = "1";
        }
        intent.putExtra("com.vivotouchscreen.sensortestsyna3706", result);
        sendBroadcast(intent);
        ActivityVIVOProduction.this.finish();
    }

}
